#include "PCH.hpp"

#include "Global.hpp"
#include "Core/ClientServerInterface.hpp"

namespace IServerImplHook
{
    void JumpInComplete__InnerAfter(uint systemId, uint shipId)
    {
        TRY_HOOK
        {
            const auto client = Hk::Client::GetClientIdByShip(shipId).Raw();
            if (client.has_error())
            {
                return;
            }

            // TODO: Implement event for jump in
        }
        CATCH_HOOK({})
    }

    void __stdcall JumpInComplete(uint systemId, uint shipId)
    {
        Logger::i()->Log(LogLevel::Trace, std::format(L"JumpInComplete(\n\tuint systemId = {}\n\tuint shipId = {}\n)", systemId, shipId));

        if (const auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__JumpInComplete, systemId, shipId); !skip)
        {
            CALL_SERVER_PREAMBLE { Server.JumpInComplete(systemId, shipId); }
            CALL_SERVER_POSTAMBLE(true, );
        }
        JumpInComplete__InnerAfter(systemId, shipId);

        CallPluginsAfter(HookedCall::IServerImpl__JumpInComplete, systemId, shipId);
    }
}