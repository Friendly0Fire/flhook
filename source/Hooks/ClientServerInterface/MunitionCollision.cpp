#include "PCH.hpp"

#include "API/FLHook/ClientList.hpp"
#include "API/Utils/Logger.hpp"
#include "API/Utils/PerfTimer.hpp"
#include "Core/ClientServerInterface.hpp"

void __stdcall IServerImplHook::SpMunitionCollision(const SSPMunitionCollisionInfo& mci, ClientId client)
{
    Logger::Trace(std::format(L"SPMunitionCollision(\n\tClientId client = {}\n)", client));

    const auto skip = CallPlugins(&Plugin::OnSpMunitionCollision, client, mci);

    CheckForDisconnect;

    if (!skip)
    {
        CallServerPreamble { Server.SPMunitionCollision(mci, client.GetValue()); }
        CallServerPostamble(true, );
    }

    CallPlugins(&Plugin::OnSpMunitionCollisionAfter, client, mci);
}
