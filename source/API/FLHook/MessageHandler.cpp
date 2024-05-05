#include "PCH.hpp"

#include "Core/MessageHandler.hpp"
#include "Defs/FLHookConfig.hpp"
#include <API/Utils/Logger.hpp>

MessageHandler::MessageHandler()
{
    auto& config = FLHook::GetConfig();
    if (!config.messageQueue.enableQueues)
    {
        return;
    }

    Logger::Log(LogLevel::Info, L"Attempting connection to RabbitMQ");

    loop = uvw::loop::get_default();
    connectHandle = loop->resource<uvw::tcp_handle>();

    connectHandle->on<uvw::error_event>(
        [this](const uvw::error_event& event, uvw::tcp_handle&)
        {
            Logger::Log(LogLevel::Err, std::format(L"Socket error: {}", StringUtils::stows(event.what())));
            FLHook::GetConfig().messageQueue.enableQueues = false;
            runner.request_stop(); // Terminate thread runner
            isInitalizing = false;
        });

    connectHandle->on<uvw::connect_event>(
        [this, config](const uvw::connect_event&, uvw::tcp_handle& tcp)
        {
            Logger::Log(LogLevel::Info, L"Connected to RabbitMQ, attempting authentication");
            // Authenticate with the RabbitMQ cluster.
            connection = std::make_unique<AMQP::Connection>(this, AMQP::Login(StringUtils::wstos(config.messageQueue.username), StringUtils::wstos(config.messageQueue.password)), "/");

            Logger::Log(LogLevel::Info, L"Authenticated to RabbitMQ");
            // Start reading from the socket.
            connectHandle->read();
        });

    connectHandle->on<uvw::data_event>([this](const uvw::data_event& event, const uvw::tcp_handle&) { connection->parse(event.data.get(), event.length); });

    connectHandle->connect(StringUtils::wstos(config.messageQueue.hostName), config.messageQueue.port);
    runner = std::jthread([this] { loop->run(); });

    while (isInitalizing)
    {
        static int i = 0;
        i++;
        Sleep(1000);

        if (i >= 5)
        {
            connectHandle->close_reset();
            connectHandle = nullptr;
            Logger::Log(LogLevel::Warn, L"Failed to connect to RabbitMQ, but queues are enabled");
            break;
        }
    }
}

void MessageHandler::onData(AMQP::Connection* conn, const char* data, size_t size) { connectHandle->write((char*)data, size); }

void MessageHandler::onReady(AMQP::Connection* conn)
{
    Logger::Log(LogLevel::Info, L"Connected to RabbitMQ!");
    isInitalizing = false;

    channel = std::make_unique<AMQP::Channel>(conn);
}

void MessageHandler::onError(AMQP::Connection* conn, const char* message)
{
    isInitalizing = false;
    Logger::Log(LogLevel::Err, std::format(L"AMQP error: {}", StringUtils::stows(message)));
}

void MessageHandler::onClosed(AMQP::Connection* conn) { std::cout << "closed" << std::endl; }

MessageHandler::~MessageHandler() = default;

void MessageHandler::Subscribe(const std::wstring& queue, QueueOnData callback, std::optional<QueueOnFail> onFail)
{
    if (!FLHook::GetConfig().messageQueue.enableQueues || !connectHandle)
    {
        return;
    }

    if (!onMessageCallbacks.contains(queue))
    {
        onMessageCallbacks[queue] = { callback };
        if (onFail.has_value())
        {
            onFailCallbacks[queue] = { onFail.value() };
        }
        else
        {
            onFailCallbacks[queue] = {};
        }

        channel->consume(StringUtils::wstos(queue))
            .onSuccess([queue]() { Logger::Log(LogLevel::Info, std::format(L"successfully subscribed to {}", queue)); })
            .onReceived(
                [this, queue](const AMQP::Message& message, const uint64_t deliveryTag, bool)
                {
                    for (const auto callbacks = onMessageCallbacks.find(queue); const auto& cb : callbacks->second)
                    {
                        std::shared_ptr<BsonWrapper> responseBody;
                        if (!cb(message, responseBody))
                        {
                            continue;
                        }

                        if (message.headers().contains("reply_to"))
                        {
                            std::stringstream ss;
                            message.headers()["reply_to"].output(ss);

                            if (responseBody)
                            {
                                const auto bytes = responseBody->GetBytes();
                                channel->publish("", ss.str(), bytes);
                            }
                            else
                            {
                                channel->publish("", ss.str(), "");
                            }
                        }

                        channel->reject(deliveryTag);
                        return;
                    }

                    channel->reject(deliveryTag);
                })
            .onError(
                [this, queue](const char* msg)
                {
                    Logger::Log(LogLevel::Warn, std::format(L"connection terminated with {} - {}", queue, StringUtils::stows(std::string(msg))));
                    const auto callbacks = onFailCallbacks.find(queue);
                    for (const auto& cb : callbacks->second)
                    {
                        cb(msg);
                    }
                });

        return;
    }

    onMessageCallbacks[queue].emplace_back(callback);
    if (onFail.has_value())
    {
        onFailCallbacks[queue].emplace_back(onFail.value());
    }
}

void MessageHandler::DeclareQueue(const std::wstring& queue, const int flags) const
{
    if (!channel)
    {
        return;
    }

    channel->declareQueue(StringUtils::wstos(queue), flags);
}

void MessageHandler::DeclareExchange(const std::wstring& exchange, const AMQP::ExchangeType type, const int flags) const
{
    if (!channel)
    {
        return;
    }

    channel->declareExchange(StringUtils::wstos(exchange), type, flags);
}

void MessageHandler::Publish(const std::wstring& jsonData, const std::wstring& exchange, const std::wstring& queue) const
{
    if (!channel)
    {
        return;
    }

    channel->publish(StringUtils::wstos(exchange), StringUtils::wstos(queue), StringUtils::wstos(jsonData));
}
