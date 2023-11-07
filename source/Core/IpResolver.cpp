#include "PCH.hpp"

#include <ws2tcpip.h>

#include "IpResolver.hpp"

#include "API/FLServer/Player.hpp"

void IpResolver::ThreadResolver()
{
    TryHook
    {
        while (true)
        {
            mutex.lock();
            std::vector<ResolvedIp> ips = resolveIPs;
            resolveIPs.clear();
            mutex.unlock();

            for (auto& [client, connects, IP, hostname] : ips)
            {
                SOCKADDR_IN addr{ AF_INET, 2302, {}, { 0 } };
                InetPtonW(AF_INET, IP.c_str(), &addr.sin_addr);

                static std::wstring hostBuffer;
                hostBuffer.resize(255);

                GetNameInfoW(reinterpret_cast<const SOCKADDR*>(&addr), sizeof addr, hostBuffer.data(), hostBuffer.size(), nullptr, 0, 0);

                hostname = hostBuffer;
            }

            mutex.lock();
            for (auto& ip : ips)
            {
                if (ip.hostname.length())
                {
                    resolveIPsResult.push_back(ip);
                }
            }

            mutex.unlock();

            Sleep(50);
        }
    }
    CatchHook({});

    return 0;
}

void IpResolver::TimerCheckResolveResults()
{
    TryHook
    {
        mutex.lock();
        for (const auto& [client, connects, IP, hostname] : resolveIPsResult)
        {
            if (connects != ClientInfo::At(client).connects)
            {
                continue; // outdated
            }

            // check if banned
            for (const auto* config = FLHookConfig::c(); const auto& ban : config->bans.banWildcardsAndIPs)
            {
                if (Wildcard::Fit(StringUtils::wstos(ban).c_str(), StringUtils::wstos(hostname).c_str()))
                {
                    // AddKickLog(ip.client, StringUtils::wstos(std::format(L"IP/hostname ban({} matches {})", ip.hostname.c_str(), ban.c_str())));
                    if (config->bans.banAccountOnMatch)
                    {
                        Hk::Player::Ban(client, true);
                    }

                    Hk::Player::Kick(client);
                }
            }
            ClientInfo::At(client).hostname = hostname;
        }

        resolveIPsResult.clear();
        mutex.unlock();
    }
    CatchHook({})
}
