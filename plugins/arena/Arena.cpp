﻿/**
 * @date August, 2022
 * @author MadHunter (Ported by Raikkonen 2022)
 * @defgroup Arena Arena
 * @brief
 * This plugin is used to beam players to/from an arena system for the purpose of pvp.
 *
 * @paragraph cmds Player Commands
 * All commands are prefixed with '/' unless explicitly specified.
 * - arena (configurable) - This beams the player to the pvp system.
 * - return - This returns the player to their last docked base.
 *
 * @paragraph adminCmds Admin Commands
 * There are no admin commands in this plugin.
 *
 * @paragraph configuration Configuration
 * @code
 * {
 *     "command": "arena",
 *     "restrictedSystem": "Li01",
 *     "targetBase": "Li02_01_Base",
 *     "targetSystem": "Li02"
 * }
 * @endcode
 *
 * @paragraph ipc IPC Interfaces Exposed
 * This plugin does not expose any functionality.
 *
 * @paragraph optional Optional Plugin Dependencies
 * This plugin uses the "Base" plugin.
 */
#include "PCH.hpp"

#include "Arena.hpp"

#include "API/FLHook/ClientList.hpp"

namespace Plugins
{
    ArenaPlugin::ArenaPlugin(const PluginInfo& info) : Plugin(info) {}

    /// Clear client info when a client connects.
    void ArenaPlugin::OnClearClientInfo(const ClientId client) { clientData[client.GetValue()].flag = TransferFlag::None; }

    /// Load the configuration
    void ArenaPlugin::OnLoadSettings()
    {
        if (const auto conf = Json::Load<Config>("config/arena.json"); !conf.has_value())
        {
            Json::Save(config, "config/arena.json");
        }
        else
        {
            config = conf.value();
        }
    }

    /** @ingroup Arena
     * @brief Returns true if the client doesn't hold any commodities, returns false otherwise. This is to prevent people using the arena system as a trade
     * shortcut.
     */
    bool ArenaPlugin::ValidateCargo(ClientId client)
    {
        int remainingHoldSize;
        for (const auto cargo = client.EnumCargo(remainingHoldSize).Handle(); const auto& item : cargo)
        {
            bool flag = false;
            pub::IsCommodity(item.archId, flag);

            // Some commodity present.
            if (flag)
            {
                return false;
            }
        }

        return true;
    }

    /** @ingroup Arena
     * @brief This returns the return base id that is stored in the client's save file.
     */
    BaseId ArenaPlugin::ReadReturnPointForClient(ClientId client)
    {
        const auto view = client.GetData().GetCharacterData();
        if (auto returnBase = view.find("arenaReturnBase"); returnBase != view.end())
        {
            return BaseId{ static_cast<uint>(returnBase->get_int32()) };
        }

        return BaseId();
    }

    /** @ingroup Arena
     * @brief Hook on CharacterSelect. Sets their transfer flag to "None".
     */
    void ArenaPlugin::OnCharacterSelect(const ClientId client, std::wstring_view charFilename) { clientData[client.GetValue()].flag = TransferFlag::None; }

    /** @ingroup Arena
     * @brief Hook on PlayerLaunch. If their transfer flags are set appropriately, redirect the undock to either the arena base or the return point
     */
    void ArenaPlugin::OnPlayerLaunchAfter(ClientId client, ShipId ship)
    {
        const auto state = clientData[client.GetValue()].flag;
        if (state == TransferFlag::Transfer)
        {
            if (!ValidateCargo(client))
            {
                (void)client.Message(cargoErrorText);
                return;
            }

            clientData[client.GetValue()].flag = TransferFlag::None;
            (void)client.Beam(targetBaseId);
            return;
        }

        if (state == TransferFlag::Return)
        {
            if (!ValidateCargo(client))
            {
                (void)client.Message(cargoErrorText);
                return;
            }

            clientData[client.GetValue()].flag = TransferFlag::None;
            const BaseId returnPoint = ReadReturnPointForClient(client);

            if (!returnPoint)
            {
                return;
            }

            (void)client.Beam(returnPoint);
        }
    }
    void ArenaPlugin::OnCharacterSave(ClientId client, std::wstring_view charName, bsoncxx::builder::basic::document& document)
    {
        int value = 0;
        if (const auto data = clientData.find(client.GetValue()); data != clientData.end())
        {
            value = static_cast<int>(data->second.returnBase.GetValue());
        }
        document.append(bsoncxx::builder::basic::kvp("arenaReturnBase", value));
    }

    /** @ingroup Arena
     * @brief Used to switch to the arena system
     */
    void ArenaPlugin::UserCmdArena()
    {
        // Prohibit jump if in a restricted system or in the target system
        if (const SystemId system = userCmdClient.GetSystemId().Unwrap();
            std::ranges::find(restrictedSystems, system) != restrictedSystems.end() || system == targetSystemId)
        {
            (void)userCmdClient.Message(L"ERR Cannot use command in this system or base");
            return;
        }

        if (!userCmdClient.IsDocked())
        {
            (void)userCmdClient.Message(dockErrorText);
            return;
        }

        if (!ValidateCargo(userCmdClient))
        {
            (void)userCmdClient.Message(cargoErrorText);
            return;
        }

        (void)userCmdClient.Message(L"Redirecting undock to Arena.");
        clientData[userCmdClient.GetValue()].flag = TransferFlag::Transfer;
    }

    /** @ingroup Arena
     * @brief Used to return from the arena system.
     */
    void ArenaPlugin::UserCmdReturn()
    {
        if (!ReadReturnPointForClient(userCmdClient))
        {
            (void)userCmdClient.Message(L"No return possible");
            return;
        }

        if (!userCmdClient.IsDocked())
        {
            (void)userCmdClient.Message(dockErrorText);
            return;
        }

        if (userCmdClient.GetCurrentBase().Unwrap() != targetBaseId)
        {
            (void)userCmdClient.Message(L"Not in correct base");
            return;
        }

        if (!ValidateCargo(userCmdClient))
        {
            (void)userCmdClient.Message(cargoErrorText);
            return;
        }

        (void)userCmdClient.Message(L"Redirecting undock to previous base");
        clientData[userCmdClient.GetValue()].flag = TransferFlag::Return;
    }
} // namespace Plugins

using namespace Plugins;

DefaultDllMain();

const PluginInfo Info(L"Arena", L"arena", PluginMajorVersion::V04, PluginMinorVersion::V01);
__declspec(dllexport) std::shared_ptr<ArenaPlugin> PluginFactory()
{
    __pragma(clang diagnostic push);
    __pragma(clang diagnostic ignored "-Wunknown-pragmas");
    __pragma("comment(linker, \"/ EXPORT : __FUNCTION__ = __FUNCDNAME__\")");
    __pragma(clang diagnostic pop);
    return std::make_shared<ArenaPlugin>(Info);
};
