/**
 * @date July, 2022
 * @author Nekura Mew
 * @defgroup Tax Tax
 * @brief
 * The Tax plugin allows players to issue 'formally' make credit demands and declare hostilities.
 *
 * @paragraph cmds Player Commands
 * -tax <amount> - demand listed amount from the player, for amount equal zero, it declares hostilities.
 * -pay - submits the demanded payment to the issuing player
 *
 * @paragraph adminCmds Admin Commands
 * None
 *
 * @paragraph configuration Configuration
 * @code
 * {
 *     "cannotPay": "This rogue isn't interested in money. Run for cover, they want to kill you!",
 *     "customColor": 9498256,
 *     "customFormat": 144,
 *     "huntingMessage": "You are being hunted by %s. Run for cover, they want to kill you!",
 *     "huntingMessageOriginator": "Good luck hunting %s !",
 *     "killDisconnectingPlayers": true,
 *     "maxTax": 300,
 *     "minplaytimeSec": 0,
 *     "taxRequestReceived": "You have received a tax request: Pay %d credits to %s! type \"/pay\" to pay the tax."
 * }
 * @endcode
 *
 * @paragraph ipc IPC Interfaces Exposed
 * This plugin does not expose any functionality.
 */

#include "PCH.hpp"

#include "Tax.hpp"

void TaxPlugin::RemoveTax(const Tax& toRemove)
{
    const auto taxToRemove =
        std::ranges::find_if(taxes, [&toRemove](const Tax& tax) { return tax.targetId == toRemove.targetId && tax.initiatorId == toRemove.initiatorId; });
    taxes.erase(taxToRemove);
}

void TaxPlugin::UserCmdTax(const std::wstring_view taxAmount)
{
    const auto& noPvpSystems = FLHook::GetConfig()->general.noPVPSystemsHashed;
    // no-pvp check
    if (SystemId system = Hk::Player::GetSystem(client).Unwrap(); std::ranges::find(noPvpSystems, system) == noPvpSystems.end())
    {
        client.Message(L"Error: You cannot tax in a No-PvP system.");
        return;
    }

    if (taxAmount.empty())
    {
        client.Message(L"Usage:");
        client.Message(L"/tax <credits>");
        return;
    }

    const uint taxValue = StringUtils::MultiplyUIntBySuffix(taxAmount);

    if (taxValue > config->maxTax)
    {
        client.Message(std::format(L"Error: Maximum tax value is {} credits.", config->maxTax));
        return;
    }

    const auto clientTargetObject = Hk::Player::GetTargetClientID(client).Unwrap();

    if (!clientTargetObject)
    {
        client.Message(L"Error: You are not targeting a player.");
        return;
    }

    const auto clientTarget = clientTargetObject;
    if (const auto secs = Hk::Player::GetOnlineTime(client).Unwrap(); secs < config->minplaytimeSec)
    {
        client.Message(L"Error: This player doesn't have enough playtime.");
        return;
    }

    for (const auto& [targetId, initiatorId, target, initiator, cash, f1] : taxes)
    {
        if (targetId == clientTarget)
        {
            client.Message(L"Error: There already is a tax request pending for this player.");
            return;
        }
    }

    Tax tax;
    tax.initiatorId = client;
    tax.targetId = clientTarget;
    tax.cash = taxValue;
    taxes.push_back(tax);

    std::wstring msg;
    const auto characterName = client.GetCharacterName().Handle();

    if (taxValue == 0)
    {
        msg = Hk::Chat::FormatMsg(config->customColor, config->customFormat, std::vformat(config->huntingMessage, std::make_wformat_args(characterName)));
    }
    else
    {
        msg = Hk::Chat::FormatMsg(
            config->customColor, config->customFormat, std::vformat(config->taxRequestReceived, std::make_wformat_args(taxValue, characterName)));
    }

    Hk::Chat::FMsg(clientTarget, msg);

    const auto targetCharacterName = clientTarget.GetCharacterName().Handle();

    // send confirmation msg
    if (taxValue > 0)
    {
        client.Message(std::format(L"Tax request of {} credits sent to {}!", taxValue, targetCharacterName));
    }
    else
    {
        client.Message(std::vformat(config->huntingMessageOriginator, std::make_wformat_args(targetCharacterName)));
    }
}

void TaxPlugin::UserCmdPay()
{
    for (auto& it : taxes)
    {
        if (it.targetId == client)
        {
            if (it.cash == 0)
            {
                client.Message(config->cannotPay);
                return;
            }

            if (const auto cash = Hk::Player::GetCash(client).Unwrap(); cash < it.cash)
            {
                client.Message(L"You have not enough money to pay the tax.");
                PrintUserCmdText(it.initiatorId, L"The player does not have enough money to pay the tax.");
                return;
            }
            Hk::Player::RemoveCash(client, it.cash).Handle();
            client.Message(L"You paid the tax.");

            Hk::Player::AddCash(it.initiatorId, it.cash).Handle();

            const auto characterName = client.GetCharacterName().Handle();
            PrintUserCmdText(it.initiatorId, std::format(L"{} paid the tax!", characterName));
            RemoveTax(it);

            Hk::Player::SaveChar(client);
            Hk::Player::SaveChar(it.initiatorId);
            return;
        }
    }

    client.Message(L"Error: No tax request was found that could be accepted!");
}

void TaxPlugin::TimerF1Check()
{
    PlayerData* playerData = nullptr;
    while ((playerData = Players.traverse_active(playerData)))
    {
        ClientId client = playerData->onlineId;

        if (ClientInfo::At(client).tmF1TimeDisconnect)
        {
            continue;
        }

        if (ClientInfo::At(client).tmF1Time && (TimeUtils::UnixMilliseconds() >= ClientInfo::At(client).tmF1Time)) // f1
        {
            // tax
            for (const auto& it : taxes)
            {
                if (it.targetId == client)
                {
                    if (uint ship = Hk::Player::GetShip(client).Unwrap(); ship && config->killDisconnectingPlayers)
                    {
                        // F1 -> Kill
                        pub::SpaceObj::SetRelativeHealth(ship, 0.0);
                    }
                    const auto characterName = it.targetId.GetCharacterName().Handle();
                    PrintUserCmdText(it.initiatorId, std::format(L"Tax request to {} aborted.", characterName));
                    RemoveTax(it);
                    break;
                }
            }
        }
        else if (ClientInfo::At(client).tmF1TimeDisconnect && (TimeUtils::UnixMilliseconds() >= ClientInfo::At(client).tmF1TimeDisconnect))
        {
            for (const auto& it : taxes)
            {
                if (it.targetId == client)
                {
                    if (uint ship = Hk::Player::GetShip(client).Unwrap())
                    {
                        // F1 -> Kill
                        pub::SpaceObj::SetRelativeHealth(ship, 0.0);
                    }
                    const auto characterName = it.targetId.GetCharacterName().Handle();
                    PrintUserCmdText(it.initiatorId, std::format(L"Tax request to {} aborted.", characterName));
                    RemoveTax(it);
                    break;
                }
            }
        }
    }
}

void TaxPlugin::DisConnect([[maybe_unused]] ClientId& client, [[maybe_unused]] const EFLConnection& state) { TimerF1Check(); }

void TaxPlugin::LoadSettings() { config = Serializer::LoadFromJson<Config>(L"config/tax.json"); }

DefaultDllMain();

const PluginInfo Info(TaxPlugin::pluginName, TaxPlugin::pluginShortName, PluginMajorVersion::V04, PluginMinorVersion::V01);
SetupPlugin(TaxPlugin, Info);

TaxPlugin::TaxPlugin(const PluginInfo& info) : Plugin(info)
{
    AddPluginTimer(&TaxPlugin::TimerF1Check, 1);

    EmplaceHook(HookedCall::IServerImpl__DisConnect, &TaxPlugin::DisConnect);
    EmplaceHook(HookedCall::FLHook__LoadSettings, &TaxPlugin::LoadSettings, HookStep::After);
}
