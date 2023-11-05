#include "PCH.hpp"

#include "API/FLServer/Client.hpp"
#include "API/FLServer/Player.hpp"
#include "Core/ClientServerInterface.hpp"
#include "Global.hpp"

namespace IServerImplHook
{
    void ActivateEquip__Inner(ClientId client, const XActivateEquip& aq)
    {
        TRY_HOOK
        {
            int _;

            for (const auto cargoList = Hk::Player::EnumCargo(client, _).Raw(); auto& cargo : cargoList.value())
            {
                if (cargo.id == aq.id)
                {
                    Archetype::Equipment* eq = Archetype::GetEquipment(cargo.archId);
                    const EquipmentType eqType = Hk::Client::GetEqType(eq);

                    if (eqType == ET_ENGINE)
                    {
                        ClientInfo[client].engineKilled = !aq.activate;
                        if (!aq.activate)
                        {
                            ClientInfo[client].cruiseActivated = false; // enginekill enabled
                        }
                    }
                }
            }
        }
        CATCH_HOOK({})
    }
    void __stdcall ActivateEquip(ClientId client, const XActivateEquip& aq)
    {
        Logger::i()->Log(LogLevel::Trace, std::format(L"ActivateEquip(\n\tClientId client = {}\n)", client));

        const auto skip = CallPlugins(&Plugin::OnActivateEquip, client, aq);

        CHECK_FOR_DISCONNECT;

        ActivateEquip__Inner(client, aq);

        if (!skip)
        {
            CALL_SERVER_PREAMBLE { Server.ActivateEquip(client, aq); }
            CALL_SERVER_POSTAMBLE(true, );
        }

        CallPlugins(&Plugin::OnActivateEquipAfter, client, aq);
    }

    void __stdcall ReqEquipment(const EquipDescList& edl, ClientId client)
    {
        Logger::i()->Log(LogLevel::Trace, std::format(L"ReqEquipment(\n\tClientId client = {}\n)", client));

        if (const auto skip = CallPlugins(&Plugin::OnRequestEquipment, client, edl); !skip)
        {
            CALL_SERVER_PREAMBLE { Server.ReqEquipment(edl, client); }
            CALL_SERVER_POSTAMBLE(true, );
        }

        CallPlugins(&Plugin::OnRequestEquipmentAfter, client, edl);
    }
    void __stdcall FireWeapon(ClientId client, const XFireWeaponInfo& fwi)
    {
        Logger::i()->Log(LogLevel::Trace, std::format(L"FireWeapon(\n\tClientId client = {}\n)", client));

        const auto skip = CallPlugins(&Plugin::OnFireWeapon, client, fwi);

        CHECK_FOR_DISCONNECT;

        if (!skip)
        {
            CALL_SERVER_PREAMBLE { Server.FireWeapon(client, fwi); }
            CALL_SERVER_POSTAMBLE(true, );
        }

        CallPlugins(&Plugin::OnFireWeaponAfter, client, fwi);
    }

    void __stdcall SetWeaponGroup(ClientId client, uint _genArg1, int _genArg2)
    {
        Logger::i()->Log(
            LogLevel::Trace,
            std::format(L"SetWeaponGroup(\n\tClientId client = {}\n\tuint _genArg1 = 0x{:08X}\n\tint _genArg2 = {}\n)", client, _genArg1, _genArg2));

        if (const auto skip = CallPlugins(&Plugin::OnSetWeaponGroup, client, _genArg1, _genArg2); !skip)
        {
            CALL_SERVER_PREAMBLE { Server.SetWeaponGroup(client, (uchar*)_genArg1, _genArg2); }
            CALL_SERVER_POSTAMBLE(true, );
        }

        CallPlugins(&Plugin::OnSetWeaponGroupAfter, client, _genArg1, _genArg2);
    }

    // We think this is hook involving usage of nanobots and shield batteries but not sure.
    void __stdcall SPRequestUseItem(const SSPUseItem& ui, ClientId client)
    {
        Logger::i()->Log(LogLevel::Trace, std::format(L"SPRequestUseItem(\n\tClientId client = {}\n)", client));

        if (const auto skip = CallPlugins(&Plugin::OnSpRequestUseItem, client, ui); !skip)
        {
            CALL_SERVER_PREAMBLE { Server.SPRequestUseItem(ui, client); }
            CALL_SERVER_POSTAMBLE(true, );
        }

        CallPlugins(&Plugin::OnSpRequestUseItemAfter, client, ui);
    }

    void ActivateThrusters__Inner(ClientId client, const XActivateThrusters& at)
    {
        TRY_HOOK { ClientInfo[client].thrusterActivated = at.activate; }
        CATCH_HOOK({})
    }

    void __stdcall ActivateThrusters(ClientId client, const XActivateThrusters& at)
    {
        Logger::i()->Log(LogLevel::Trace, std::format(L"ActivateThrusters(\n\tClientId client = {}\n)", client));

        const auto skip = CallPlugins(&Plugin::OnActivateThrusters, client, at);

        CHECK_FOR_DISCONNECT;

        ActivateThrusters__Inner(client, at);

        if (!skip)
        {
            CALL_SERVER_PREAMBLE { Server.ActivateThrusters(client, at); }
            CALL_SERVER_POSTAMBLE(true, );
        }

        CallPlugins(&Plugin::OnActivateThrustersAfter, client, at);
    }

    void ActivateCruise__Inner(ClientId client, const XActivateCruise& ac)
    {
        TRY_HOOK { ClientInfo[client].cruiseActivated = ac.activate; }
        CATCH_HOOK({})
    }

    void __stdcall ActivateCruise(ClientId client, const XActivateCruise& ac)
    {
        Logger::i()->Log(LogLevel::Trace, std::format(L"ActivateCruise(\n\tClientId client = {}\n)", client));

        const auto skip = CallPlugins(&Plugin::OnActivateCruise, client, ac);

        CHECK_FOR_DISCONNECT;

        ActivateCruise__Inner(client, ac);

        if (!skip)
        {
            CALL_SERVER_PREAMBLE { Server.ActivateCruise(client, ac); }
            CALL_SERVER_POSTAMBLE(true, );
        }

        CallPlugins(&Plugin::OnActivateCruiseAfter, client, ac);
    }

} // namespace IServerImplHook
