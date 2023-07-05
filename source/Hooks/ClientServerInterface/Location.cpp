#include "PCH.hpp"

#include "Global.hpp"
#include "Core/ClientServerInterface.hpp"

namespace IServerImplHook
{
    void __stdcall SetVisitedState(ClientId client, uint objHash, int state)
    {
        Logger::i()->Log(LogLevel::Trace,
                         std::format(L"SetVisitedState(\n\tClientId client = {}\n\tuint objHash = {}\n\tint state = {}\n)", client, objHash, state));

        if (const auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__SetVisitedState, client, objHash, state); !skip)
        {
            CALL_SERVER_PREAMBLE { Server.SetVisitedState(client, (uchar*)objHash, state); }
            CALL_SERVER_POSTAMBLE(true, );
        }

        CallPluginsAfter(HookedCall::IServerImpl__SetVisitedState, client, objHash, state);
    }

    void __stdcall RequestBestPath(ClientId client, uint _genArg1, int _genArg2)
    {
        Logger::i()->Log(
            LogLevel::Trace,
            std::format(L"RequestBestPath(\n\tClientId client = {}\n\tuint _genArg1 = 0x{:08X}\n\tint _genArg2 = {}\n)", client, _genArg1, _genArg2));

        if (const auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__RequestBestPath, client, _genArg1, _genArg2); !skip)
        {
            CALL_SERVER_PREAMBLE { Server.RequestBestPath(client, (uchar*)_genArg1, _genArg2); }
            CALL_SERVER_POSTAMBLE(true, );
        }

        CallPluginsAfter(HookedCall::IServerImpl__RequestBestPath, client, _genArg1, _genArg2);
    }

    void __stdcall LocationInfoRequest(unsigned int _genArg1, unsigned int _genArg2, bool _genArg3)
    {
        Logger::i()->Log(
            LogLevel::Trace,
            std::format(
                L"LocationInfoRequest(\n\tunsigned int _genArg1 = {}\n\tunsigned int _genArg2 = {}\n\tbool _genArg3 = {}\n)", _genArg1, _genArg2, _genArg3));

        if (const auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__LocationInfoRequest, _genArg1, _genArg2, _genArg3); !skip)
        {
            CALL_SERVER_PREAMBLE { Server.LocationInfoRequest(_genArg1, _genArg2, _genArg3); }
            CALL_SERVER_POSTAMBLE(true, );
        }

        CallPluginsAfter(HookedCall::IServerImpl__LocationInfoRequest, _genArg1, _genArg2, _genArg3);
    }

    void __stdcall LocationExit(uint locationId, ClientId client)
    {
        Logger::i()->Log(LogLevel::Trace, std::format(L"LocationExit(\n\tuint locationId = {}\n\tClientId client = {}\n)", locationId, client));

        if (const auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__LocationExit, locationId, client); !skip)
        {
            CALL_SERVER_PREAMBLE { Server.LocationExit(locationId, client); }
            CALL_SERVER_POSTAMBLE(true, );
        }

        CallPluginsAfter(HookedCall::IServerImpl__LocationExit, locationId, client);
    }

    void __stdcall LocationEnter(uint locationId, ClientId client)
    {
        Logger::i()->Log(LogLevel::Trace, std::format(L"LocationEnter(\n\tuint locationId = {}\n\tClientId client = {}\n)", locationId, client));

        if (const auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__LocationEnter, locationId, client); !skip)
        {
            CALL_SERVER_PREAMBLE { Server.LocationEnter(locationId, client); }
            CALL_SERVER_POSTAMBLE(true, );
        }

        CallPluginsAfter(HookedCall::IServerImpl__LocationEnter, locationId, client);
    }
} // namespace IServerImplHook
