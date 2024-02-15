#include "PCH.hpp"

#include "API/FLHook/ClientList.hpp"
#include "API/InternalApi.hpp"
#include "Core/IEngineHook.hpp"

std::wstring SetSizeToSmall(const std::wstring& dataFormat) { return dataFormat.substr(0, 8) + L"90"; }

void IEngineHook::SendDeathMessage(const std::wstring& msg, SystemId systemId, ClientId clientVictim, ClientId clientKiller)
{
    CallPlugins(&Plugin::OnSendDeathMessage, clientKiller, clientVictim, systemId, std::wstring_view(msg));

    // encode xml std::wstring(default and small) non-system
    const auto xmlMsg =
        std::format(L"<TRA data=\"{}\" mask=\"-1\"/> <TEXT>{}</TEXT>", FLHook::GetConfig().chatConfig.msgStyle.deathMsgStyle, StringUtils::XmlText(msg));

    static std::array<char, 0xFFFF> xmlBuf;
    uint ret;
    if (InternalApi::FMsgEncodeXml(xmlMsg, xmlBuf.data(), xmlBuf.size(), ret).Raw().has_error())
    {
        return;
    }

    const auto xmlMsgSmall = std::format(
        L"<TRA data=\"{}\" mask=\"-1\"/> <TEXT>{}</TEXT>", SetSizeToSmall(FLHook::GetConfig().chatConfig.msgStyle.deathMsgStyle), StringUtils::XmlText(msg));

    static std::array<char, 0xFFFF> bufSmall;
    uint retSmall;
    if (InternalApi::FMsgEncodeXml(xmlMsgSmall, bufSmall.data(), bufSmall.size(), retSmall).Raw().has_error())
    {
        return;
    }

    const auto xmlMsgSystem =
        std::format(L"<TRA data=\"{}\" mask=\"-1\"/> <TEXT>{}</TEXT>", FLHook::GetConfig().chatConfig.msgStyle.deathMsgStyleSys, StringUtils::XmlText(msg));

    static std::array<char, 0xFFFF> bufSys;
    uint retSys;
    if (InternalApi::FMsgEncodeXml(xmlMsgSystem, bufSys.data(), bufSys.size(), retSys).Raw().has_error())
    {
        return;
    }

    const auto xmlMsgSmallSys =
        std::format(L"<TRA data=\"{}\" mask=\"-1\"/> <TEXT>{}", FLHook::GetConfig().chatConfig.msgStyle.deathMsgStyleSys, StringUtils::XmlText(msg));

    static std::array<char, 0xFFFF> BufSmallSys;
    uint retSmallSys;
    if (InternalApi::FMsgEncodeXml(xmlMsgSmallSys, BufSmallSys.data(), BufSmallSys.size(), retSmallSys).Raw().has_error())
    {
        return;
    }

    for (auto& client : FLHook::Clients())
    {
        const auto system = client.id.GetSystemId().Unwrap();

        const auto& data = client.id.GetData();

        char* sendXmlBuf;
        int sendXmlRet;
        char* sendXmlBufSys;
        int sendXmlSysRet;
        if (FLHook::GetConfig().userCommands.userCmdSetDieMsgSize && data.dieMsgSize == ChatSize::Small)
        {
            sendXmlBuf = bufSmall.data();
            sendXmlRet = retSmall;
            sendXmlBufSys = BufSmallSys.data();
            sendXmlSysRet = retSmallSys;
        }
        else
        {
            sendXmlBuf = xmlBuf.data();
            sendXmlRet = ret;
            sendXmlBufSys = bufSys.data();
            sendXmlSysRet = retSys;
        }

        if (!FLHook::GetConfig().userCommands.userCmdSetDieMsg)
        {
            // /set diemsg disabled, thus send to all
            if (systemId == system)
            {
                InternalApi::FMsgSendChat(client.id, sendXmlBufSys, sendXmlSysRet);
            }
            else
            {
                InternalApi::FMsgSendChat(client.id, sendXmlBuf, sendXmlRet);
            }
            continue;
        }

        if (data.dieMsg == DieMsgType::None)
        {
            continue;
        }

        if (data.dieMsg == DieMsgType::System && systemId == system)
        {
            InternalApi::FMsgSendChat(client.id, sendXmlBufSys, sendXmlSysRet);
        }
        else if (data.dieMsg == DieMsgType::Self && (client.id == clientVictim || client.id == clientKiller))
        {
            InternalApi::FMsgSendChat(client.id, sendXmlBufSys, sendXmlSysRet);
        }
        else if (data.dieMsg == DieMsgType::All)
        {
            if (systemId == system)
            {
                InternalApi::FMsgSendChat(client.id, sendXmlBufSys, sendXmlSysRet);
            }
            else
            {
                InternalApi::FMsgSendChat(client.id, sendXmlBuf, sendXmlRet);
            }
        }

        std::ranges::fill(xmlBuf, 0);
        std::ranges::fill(bufSmall, 0);
        std::ranges::fill(bufSys, 0);
        std::ranges::fill(BufSmallSys, 0);
    }

    const std::wstring formattedMsg = StringUtils::stows(BufSmallSys.data());
    CallPlugins(&Plugin::OnSendDeathMessageAfter, clientKiller, clientVictim, systemId, std::wstring_view(formattedMsg));
}

void __stdcall IEngineHook::ShipDestroyed(DamageList* dmgList, DWORD* ecx, uint kill)
{
    if (!kill)
    {
        return;
    }

    TryHook
    {
        auto cship = (CShip*)ecx[4];
        auto client = ClientId(cship->GetOwnerPlayer());

        CallPlugins(&Plugin::OnShipDestroyed, client, dmgList, cship);

        if (!client)
        {
            return;
        }

        // a player was killed
        DamageList dmg;
        try
        {
            dmg = *dmgList;
        }
        catch (...)
        {
            return;
        }

        auto& data = client.GetData();
        auto systemId = client.GetSystemId().Unwrap();
        std::wstring_view name = systemId.GetName().Unwrap();

        if (!magic_enum::enum_integer(dmg.get_cause()))
        {
            dmg = data.dmgLast;
        }

        DamageCause cause = dmg.get_cause();
        const auto clientKiller = ShipId(dmg.get_inflictor_id()).GetPlayer();

        std::wstring_view victimName = client.GetCharacterName().Unwrap();
        if (clientKiller.has_value())
        {
            std::wstring killType;
            switch (cause)
            {
                case DamageCause::Collision: killType = L"Collision"; break;
                case DamageCause::Gun: killType = L"Gun"; break;
                case DamageCause::MissileTorpedo: killType = L"Missile/Torpedo"; break;
                case DamageCause::CruiseDisrupter:
                case DamageCause::DummyDisrupter:
                case DamageCause::UnkDisrupter: killType = L"Cruise Disruptor"; break;
                case DamageCause::Mine: killType = L"Mine"; break;
                case DamageCause::Suicide: killType = L"Suicide"; break;
                default: killType = L"Somehow";
            }

            std::wstring deathMessage;
            if (client == clientKiller.value() || cause == DamageCause::Suicide)
            {
                deathMessage = StringUtils::ReplaceStr(FLHook::GetConfig().chatConfig.msgStyle.deathMsgTextSelfKill, std::wstring_view(L"%victim"), victimName);
            }
            else if (cause == DamageCause::Admin)
            {
                deathMessage = StringUtils::ReplaceStr(FLHook::GetConfig().chatConfig.msgStyle.deathMsgTextAdminKill, std::wstring_view(L"%victim"), victimName);
            }
            else
            {
                std::wstring_view killer = client.GetCharacterName().Unwrap();

                deathMessage =
                    StringUtils::ReplaceStr(FLHook::GetConfig().chatConfig.msgStyle.deathMsgTextPlayerKill, std::wstring_view(L"%victim"), victimName);
                deathMessage = StringUtils::ReplaceStr(deathMessage, std::wstring_view(L"%killer"), killer);
            }

            deathMessage = StringUtils::ReplaceStr(deathMessage, std::wstring_view(L"%type"), std::wstring_view(killType));
            if (FLHook::GetConfig().chatConfig.dieMsg && deathMessage.length())
            {
                SendDeathMessage(deathMessage, systemId, client, clientKiller.value());
            }
        }
        else if (dmg.get_inflictor_id())
        {
            std::wstring killType;
            switch (cause)
            {
                case DamageCause::Collision: killType = L"Collision"; break;
                case DamageCause::Gun: break;
                case DamageCause::MissileTorpedo: killType = L"Missile/Torpedo"; break;
                case DamageCause::CruiseDisrupter:
                case DamageCause::DummyDisrupter:
                case DamageCause::UnkDisrupter: killType = L"Cruise Disruptor"; break;
                case DamageCause::Mine: killType = L"Mine"; break;
                default: killType = L"Gun";
            }

            std::wstring deathMessage =
                StringUtils::ReplaceStr(FLHook::GetConfig().chatConfig.msgStyle.deathMsgTextNPC, std::wstring_view(L"%victim"), victimName);
            deathMessage = StringUtils::ReplaceStr(deathMessage, std::wstring_view(L"%type"), std::wstring_view(killType));

            if (FLHook::GetConfig().chatConfig.dieMsg && deathMessage.length())
            {
                SendDeathMessage(deathMessage, systemId, client, ClientId());
            }
        }
        else if (cause == DamageCause::Suicide)
        {
            if (std::wstring deathMessage =
                    StringUtils::ReplaceStr(FLHook::GetConfig().chatConfig.msgStyle.deathMsgTextSuicide, std::wstring_view(L"%victim"), victimName);
                FLHook::GetConfig().chatConfig.dieMsg && !deathMessage.empty())
            {
                SendDeathMessage(deathMessage, systemId, client, ClientId());
            }
        }
        else if (cause == DamageCause::Admin)
        {
            if (std::wstring deathMessage =
                    StringUtils::ReplaceStr(FLHook::GetConfig().chatConfig.msgStyle.deathMsgTextAdminKill, std::wstring_view(L"%victim"), victimName);
                FLHook::GetConfig().chatConfig.dieMsg && deathMessage.length())
            {
                SendDeathMessage(deathMessage, systemId, client, ClientId());
            }
        }
        else
        {
            std::wstring deathMessage = std::format(L"Death: {} has died", victimName);
            if (FLHook::GetConfig().chatConfig.dieMsg && deathMessage.length())
            {
                SendDeathMessage(deathMessage, systemId, client, ClientId());
            }
        }

        data.shipOld = data.ship;
        data.ship = ShipId();
    }

    CatchHook({})
}

IEngineHook::ShipDestroyAssembly::ShipDestroyAssembly()
{
    mov(eax, dword[esp+0xC]);
    mov(eax, dword[esp+0x4]);
    push(ecx);
    push(edx);
    push(ecx);
    push(eax);
    call(IEngineHook::ShipDestroyed);
    pop(ecx);
    mov(eax, dword[IEngineHook::oldShipDestroyed]);
    jmp(eax);
}

void IEngineHook::BaseDestroyed(ObjectId objectId, ClientId clientBy)
{
    CallPlugins(&Plugin::OnBaseDestroyed, clientBy, objectId);

    uint baseId;
    pub::SpaceObj::GetDockingTarget(objectId.GetValue(), baseId);
    Universe::IBase* base = Universe::get_base(baseId);

    auto baseName = "";
    if (base)
    {
        __asm {
			pushad
			mov ecx, [base]
			mov eax, [base]
			mov eax, [eax]
			call [eax+4]
			mov [baseName], eax
			popad
        }
    }
}
