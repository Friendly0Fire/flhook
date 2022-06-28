// Anti Jump Disconnect Plugin by Cannon
// Feb 2010 by Cannon
//
// Ported by Raikkonen 2022
//
// This is free software; you can redistribute it and/or modify it as
// you wish without restriction. If you do then I would appreciate
// being notified and/or mentioned somewhere.

// Includes
#include "Main.h"

namespace Plugins::AntiJumpDisconnect
{
	const std::unique_ptr<Global> global = std::make_unique<Global>();

	void ClearClientInfo(uint& iClientID) { global->mapInfo[iClientID].bInWrapGate = false; }

	void KillBan(uint& iClientID)
	{
		if (global->mapInfo[iClientID].bInWrapGate)
		{
			HkKill(iClientID);
			if (global->tempBanCommunicator)
			{
				std::wstring wscCharname = (const wchar_t*)Players.GetActiveCharacterName(iClientID);
				global->tempBanCommunicator->TempBan(wscCharname, 5);
			}
		}
	}

	void DisConnect(uint& iClientID, enum EFLConnection& state) { KillBan(iClientID); }

	void CharacterInfoReq(uint& iClientID, bool& p2) { KillBan(iClientID); }

	void JumpInComplete(uint& iSystem, uint& iShip, uint& iClientID) { global->mapInfo[iClientID].bInWrapGate = false; }

	void SystemSwitchOutComplete(uint& iShip, uint& iClientID) { global->mapInfo[iClientID].bInWrapGate = true; }
} // namespace Plugins::AntiJumpDisconnect

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

using namespace Plugins::AntiJumpDisconnect;

DefaultDllMain()

extern "C" EXPORT void ExportPluginInfo(PluginInfo* pi)
{
	pi->name("Anti Jump Disconnect Plugin by Cannon");
	pi->shortName("anti_jump_disconnect");
	pi->mayPause(true);
	pi->mayUnload(true);
	pi->returnCode(&global->returncode);
	pi->versionMajor(PluginMajorVersion::VERSION_04);
	pi->versionMinor(PluginMinorVersion::VERSION_00);
	pi->emplaceHook(HookedCall::FLHook__ClearClientInfo, &ClearClientInfo);
	pi->emplaceHook(HookedCall::IServerImpl__DisConnect, &DisConnect);
	pi->emplaceHook(HookedCall::IServerImpl__CharacterInfoReq, &CharacterInfoReq);
	pi->emplaceHook(HookedCall::IServerImpl__JumpInComplete, &JumpInComplete);
	pi->emplaceHook(HookedCall::IServerImpl__SystemSwitchOutComplete, &SystemSwitchOutComplete);

	// We import the definitions for TempBan Communicator so we can talk to it
	global->tempBanCommunicator = static_cast<Plugins::Tempban::TempBanCommunicator*>(
	    PluginCommunicator::ImportPluginCommunicator(Plugins::Tempban::TempBanCommunicator::pluginName));
}