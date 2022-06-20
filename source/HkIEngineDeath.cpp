﻿#include "Global.hpp"

std::wstring SetSizeToSmall(const std::wstring& wscDataFormat)
{
	return wscDataFormat.substr(0, 8) + L"90";
}

/**************************************************************************************************************
Send "Death: ..." chat-message
**************************************************************************************************************/

void SendDeathMessage(const std::wstring& msg, uint systemID, uint clientIDVictim, uint clientIDKiller)
{
	CallPluginsBefore(HookedCall::IEngine__SendDeathMessage, msg, systemID, clientIDVictim, clientIDKiller);

	// encode xml std::string(default and small)
	// non-sys
	std::wstring xmlMsg = L"<TRA data=\"" + FLHookConfig::i()->msgStyle.deathMsgStyle + L"\" mask=\"-1\"/> <TEXT>";
	xmlMsg += XMLText(msg);
	xmlMsg += L"</TEXT>";

	char xmlBuf[0xFFFF];
	uint ret;
	if (!HKHKSUCCESS(HkFMsgEncodeXML(xmlMsg, xmlBuf, sizeof(xmlBuf), ret)))
		return;

	std::wstring styleSmall = SetSizeToSmall(FLHookConfig::i()->msgStyle.deathMsgStyle);
	std::wstring xmlMsgSmall = std::wstring(L"<TRA data=\"") + styleSmall + L"\" mask=\"-1\"/> <TEXT>";
	xmlMsgSmall += XMLText(msg);
	xmlMsgSmall += L"</TEXT>";
	char bufSmall[0xFFFF];
	uint retSmall;
	if (!HKHKSUCCESS(HkFMsgEncodeXML(xmlMsgSmall, bufSmall, sizeof(bufSmall), retSmall)))
		return;

	// sys
	std::wstring xmlMsgSys = L"<TRA data=\"" + FLHookConfig::i()->msgStyle.deathMsgStyleSys + L"\" mask=\"-1\"/> <TEXT>";
	xmlMsgSys += XMLText(msg);
	xmlMsgSys += L"</TEXT>";
	char bufSys[0xFFFF];
	uint retSys;
	if (!HKHKSUCCESS(HkFMsgEncodeXML(xmlMsgSys, bufSys, sizeof(bufSys), retSys)))
		return;

	std::wstring styleSmallSys = SetSizeToSmall(FLHookConfig::i()->msgStyle.deathMsgStyleSys);
	std::wstring xmlMsgSmallSys = L"<TRA data=\"" + styleSmallSys + L"\" mask=\"-1\"/> <TEXT>";
	xmlMsgSmallSys += XMLText(msg);
	xmlMsgSmallSys += L"</TEXT>";
	char szBufSmallSys[0xFFFF];
	uint retSmallSys;
	if (!HKHKSUCCESS(HkFMsgEncodeXML(xmlMsgSmallSys, szBufSmallSys, sizeof(szBufSmallSys), retSmallSys)))
		return;

	// send
	// for all players
	struct PlayerData* playerData = nullptr;
	while (playerData = Players.traverse_active(playerData))
	{
		uint clientID = HkGetClientIdFromPD(playerData);
		uint clientSystemID = 0;
		pub::Player::GetSystem(clientID, clientSystemID);

		char* sendXmlBuf;
		int sendXmlRet;
		char* sendXmlBufSys;
		int sendXmlSysRet;
		if (FLHookConfig::i()->userCommands.userCmdSetDieMsgSize && (ClientInfo[clientID].dieMsgSize == CS_SMALL))
		{
			sendXmlBuf = bufSmall;
			sendXmlRet = retSmall;
			sendXmlBufSys = szBufSmallSys;
			sendXmlSysRet = retSmallSys;
		}
		else
		{
			sendXmlBuf = xmlBuf;
			sendXmlRet = ret;
			sendXmlBufSys = bufSys;
			sendXmlSysRet = retSys;
		}

		if (!FLHookConfig::i()->userCommands.userCmdSetDieMsg)
		{ // /set diemsg disabled, thus send to all
			if (systemID == clientSystemID)
				HkFMsgSendChat(clientID, sendXmlBufSys, sendXmlSysRet);
			else
				HkFMsgSendChat(clientID, sendXmlBuf, sendXmlRet);
			continue;
		}

		if (ClientInfo[clientID].dieMsg == DIEMSG_NONE)
			continue;
		else if ((ClientInfo[clientID].dieMsg == DIEMSG_SYSTEM) && (systemID == clientSystemID))
			HkFMsgSendChat(clientID, sendXmlBufSys, sendXmlSysRet);
		else if (
		    (ClientInfo[clientID].dieMsg == DIEMSG_SELF) &&
		    ((clientID == clientIDVictim) || (clientID == clientIDKiller)))
			HkFMsgSendChat(clientID, sendXmlBufSys, sendXmlSysRet);
		else if (ClientInfo[clientID].dieMsg == DIEMSG_ALL)
		{
			if (systemID == clientSystemID)
				HkFMsgSendChat(clientID, sendXmlBufSys, sendXmlSysRet);
			else
				HkFMsgSendChat(clientID, sendXmlBuf, sendXmlRet);
		}
	}
}

/**************************************************************************************************************
Called when ship was destroyed
**************************************************************************************************************/

void __stdcall ShipDestroyed(DamageList* dmgList, DWORD* ecx, uint kill)
{
	CallPluginsBefore(HookedCall::IEngine__ShipDestroyed, dmgList, ecx, kill);

	TRY_HOOK
	{
		if (kill == 1)
		{
			CShip* cship = (CShip*)ecx[4];
			uint clientID = cship->GetOwnerPlayer();

			if (clientID)
			{ // a player was killed
				DamageList dmg;
				try
				{
					dmg = *dmgList;
				}
				catch (...)
				{
					return;
				}

				std::wstring eventStr;
				eventStr.reserve(256);
				eventStr = L"kill";

				uint systemID;
				pub::Player::GetSystem(clientID, systemID);
				wchar_t systemName[64];
				swprintf_s(systemName, L"%u", systemID);

				if (!dmg.get_cause())
					dmg = ClientInfo[clientID].dmgLast;

				uint cause = dmg.get_cause();
				uint clientIDKiller = HkGetClientIDByShip(dmg.get_inflictor_id());

				std::wstring victimName = ToWChar(Players.GetActiveCharacterName(clientID));
				eventStr += L" victim=" + victimName;
				if (clientIDKiller)
				{
					std::wstring killType = L"";
					if (cause == 0x05)
						killType = L"Missile/Torpedo";
					else if (cause == 0x07)
						killType = L"Mine";
					else if ((cause == 0x06) || (cause == 0xC0) || (cause == 0x15))
						killType = L"Cruise Disruptor";
					else if (cause == 0x01)
						killType = L"Collision";
					else
						killType = L"Gun";

					std::wstring deathMessage;
					if (clientID == clientIDKiller)
					{
						eventStr += L" type=selfkill";
						deathMessage = ReplaceStr(FLHookConfig::i()->msgStyle.deathMsgTextSelfKill, L"%victim", victimName);
					}
					else
					{
						eventStr += L" type=player";
						std::wstring wscKiller = ToWChar(Players.GetActiveCharacterName(clientIDKiller));
						eventStr += L" by=" + wscKiller;

						deathMessage = ReplaceStr(FLHookConfig::i()->msgStyle.deathMsgTextPlayerKill, L"%victim", victimName);
						deathMessage = ReplaceStr(deathMessage, L"%killer", wscKiller);
					}

					deathMessage = ReplaceStr(deathMessage, L"%type", killType);
					if (FLHookConfig::i()->general.dieMsg && deathMessage.length())
						SendDeathMessage(deathMessage, systemID, clientID, clientIDKiller);
					ProcessEvent(L"%s", eventStr.c_str());

					// MultiKillMessages
					if ((FLHookConfig::i()->multiKillMessages.active) && (clientID != clientIDKiller))
					{
						std::wstring killerName = ToWChar(Players.GetActiveCharacterName(clientIDKiller));

						ClientInfo[clientIDKiller].iKillsInARow++;
						for (auto& msg : FLHookConfig::i()->multiKillMessages.multiKillMessages)
						{
							if (msg.second == ClientInfo[clientIDKiller].iKillsInARow)
							{
								std::wstring xmlMsg = L"<TRA data=\"" + FLHookConfig::i()->multiKillMessages.multiKillMessageStyle + L"\" mask=\"-1\"/> <TEXT>";
								xmlMsg += XMLText(ReplaceStr(msg.first, L"%player", killerName));
								xmlMsg += L"</TEXT>";

								char encodeBuf[0xFFFF];
								uint rval;
								if (!HKHKSUCCESS(HkFMsgEncodeXML(xmlMsg, encodeBuf, sizeof(encodeBuf), rval)))
									break;

								// for all players in system...
								struct PlayerData* playerData = nullptr;
								while (playerData = Players.traverse_active(playerData))
								{
									uint clientID = HkGetClientIdFromPD(playerData);
									uint clientSystemID = 0;
									pub::Player::GetSystem(clientID, clientSystemID);
									if (clientID == clientIDKiller ||
									    ((systemID == clientSystemID) &&
									     (((ClientInfo[clientID].dieMsg == DIEMSG_ALL) ||
									       (ClientInfo[clientID].dieMsg == DIEMSG_SYSTEM)) ||
									            !FLHookConfig::i()->general.dieMsg)))
										HkFMsgSendChat(clientID, encodeBuf, rval);
								}
							}
						}
					}
				}
				else if (dmg.get_inflictor_id())
				{
					std::wstring killType = L"";
					if (cause == 0x05)
						killType = L"Missile/Torpedo";
					else if (cause == 0x07)
						killType = L"Mine";
					else if ((cause == 0x06) || (cause == 0xC0) || (cause == 0x15))
						killType = L"Cruise Disruptor";
					else if (cause == 0x01)
						killType = L"Collision";
					else
						killType = L"Gun";

					eventStr += L" type=npc";
					std::wstring deathMessage = ReplaceStr(FLHookConfig::i()->msgStyle.deathMsgTextNPC, L"%victim", victimName);
					deathMessage = ReplaceStr(deathMessage, L"%type", killType);

					if (FLHookConfig::i()->general.dieMsg && deathMessage.length())
						SendDeathMessage(deathMessage, systemID, clientID, 0);
					ProcessEvent(L"%s", eventStr.c_str());
				}
				else if (cause == 0x08)
				{
					eventStr += L" type=suicide";
					std::wstring deathMessage = ReplaceStr(FLHookConfig::i()->msgStyle.deathMsgTextSuicide, L"%victim", victimName);

					if (FLHookConfig::i()->general.dieMsg && deathMessage.length())
						SendDeathMessage(deathMessage, systemID, clientID, 0);
					ProcessEvent(L"%s", eventStr.c_str());
				}
				else if (cause == 0x18)
				{
					std::wstring deathMessage = ReplaceStr(FLHookConfig::i()->msgStyle.deathMsgTextAdminKill, L"%victim", victimName);

					if (FLHookConfig::i()->general.dieMsg && deathMessage.length())
						SendDeathMessage(deathMessage, systemID, clientID, 0);
				}
				else
				{
					std::wstring deathMessage = L"Death: " + victimName + L" has died";
					if (FLHookConfig::i()->general.dieMsg && deathMessage.length())
						SendDeathMessage(deathMessage, systemID, clientID, 0);
				}
			}

			ClientInfo[clientID].iShipOld = ClientInfo[clientID].iShip;
			ClientInfo[clientID].iShip = 0;
		}
	}
	CATCH_HOOK({})
}

FARPROC g_OldShipDestroyed;

__declspec(naked) void Naked__ShipDestroyed()
{
	__asm {
        mov eax, [esp+0Ch] ; +4
        mov edx, [esp+4]
        push ecx
        push edx
        push ecx
        push eax
        call ShipDestroyed
        pop ecx
        mov eax, [g_OldShipDestroyed]
        jmp eax
	}
}
/**************************************************************************************************************
Called when base was destroyed
**************************************************************************************************************/

void BaseDestroyed(uint objectID, uint clientIDBy)
{
	CallPluginsBefore(HookedCall::IEngine__BaseDestroyed, objectID, clientIDBy);

	uint baseID;
	pub::SpaceObj::GetDockingTarget(objectID, baseID);
	Universe::IBase* base = Universe::get_base(baseID);

	char* baseName = "";
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

	ProcessEvent(
	    L"basedestroy basename=%s basehash=%u solarhash=%u by=%s", stows(baseName).c_str(), objectID, baseID,
	    ToWChar(Players.GetActiveCharacterName(clientIDBy)));
}
