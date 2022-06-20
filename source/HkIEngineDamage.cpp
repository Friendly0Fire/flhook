﻿#include "Global.hpp"

EXPORT uint g_DmgTo = 0;
EXPORT uint g_DmgToSpaceID = 0;
DamageList g_LastDmgList;

bool g_NonGunHitsBase = false;
float g_LastHitPts;

/**************************************************************************************************************
Called when a torp/missile/mine/wasp hits a ship
return 0 -> pass on to server.dll
return 1 -> suppress
**************************************************************************************************************/

FARPROC g_OldGuidedHit;

int __stdcall GuidedHit(char* ecx, char* p1, DamageList* dmgList)
{
	auto [rval, skip] = CallPluginsBefore<int>(HookedCall::IEngine__GuidedHit, ecx, p1, dmgList);
	if (skip)
		return rval;

	TRY_HOOK
	{
		char* p;
		memcpy(&p, ecx + 0x10, 4);
		uint clientID;
		memcpy(&clientID, p + 0xB4, 4);
		uint spaceID;
		memcpy(&spaceID, p + 0xB0, 4);

		g_DmgTo = clientID;
		g_DmgToSpaceID = spaceID;
		if (clientID)
		{ // a player was hit
			uint inflictorShip;
			memcpy(&inflictorShip, p1 + 4, 4);
			uint clientIDInflictor = HkGetClientIDByShip(inflictorShip);
			if (!clientIDInflictor)
				return 0; // hit by npc

			if (!AllowPlayerDamage(clientIDInflictor, clientID))
				return 1;

			if (FLHookConfig::i()->general.changeCruiseDisruptorBehaviour)
			{
				if (((dmgList->get_cause() == 6) || (dmgList->get_cause() == 0x15)) &&
				    !ClientInfo[clientID].bCruiseActivated)
					dmgList->set_cause(static_cast<enum DamageCause>(0xC0)); // change to sth else, so
					                                                         // client won't recognize it as
					                                                         // a disruptor
			}
		}
	}
	CATCH_HOOK({})

	return 0;
}

__declspec(naked) void Naked__GuidedHit()
{
	__asm {
        mov eax, [esp+4]
        mov edx, [esp+8]
        push ecx
        push edx
        push eax
        push ecx
        call GuidedHit
        pop ecx
        cmp eax, 1
        jnz go_ahead
        mov edx, [esp] ; suppress
        add esp, 0Ch
        jmp edx
go_ahead:
        jmp [g_OldGuidedHit]
	}
}

/**************************************************************************************************************
Called when ship was damaged
however you can't figure out here, which ship is being damaged, that's why i use
the g_DmgTo variable...
**************************************************************************************************************/

void __stdcall AddDamageEntry(
    DamageList* dmgList, unsigned short subObjID, float hitPts, enum DamageEntry::SubObjFate fate)
{
	if (CallPluginsBefore(HookedCall::IEngine__AddDamageEntry, dmgList, subObjID, hitPts, fate))
		return;

	// check if we got damaged by a cd with changed behaviour
	if (dmgList->get_cause() == 0xC0)
	{
		// check if player should be protected (f.e. in a docking cut scene)
		bool unk1 = false;
		bool unk2 = false;
		float unk;
		pub::SpaceObj::GetInvincible(ClientInfo[g_DmgTo].iShip, unk1, unk2, unk);
		// if so, suppress the damage
		if (unk1 && unk2)
			return;
	}

	if (g_NonGunHitsBase && (dmgList->get_cause() == 5))
	{
		const float damage = g_LastHitPts - hitPts;
		hitPts = g_LastHitPts - damage * FLHookConfig::i()->general.torpMissileBaseDamageMultiplier;
		if (hitPts < 0)
			hitPts = 0;
	}

	if (!dmgList->is_inflictor_a_player()) // npcs always do damage
		dmgList->add_damage_entry(subObjID, hitPts, fate);
	else if (g_DmgTo)
	{
		// lets see if player should do damage to other player
		uint dmgFrom = HkGetClientIDByShip(dmgList->get_inflictor_id());
		if (dmgFrom && AllowPlayerDamage(dmgFrom, g_DmgTo))
			dmgList->add_damage_entry(subObjID, hitPts, fate);
	}
	else
		dmgList->add_damage_entry(subObjID, hitPts, fate);

	TRY_HOOK
	{
		g_LastDmgList = *dmgList; // save

		// check for base kill (when hull health = 0)
		if (hitPts == 0 && subObjID == 1)
		{
			uint type;
			pub::SpaceObj::GetType(g_DmgToSpaceID, type);
			uint clientIDKiller = HkGetClientIDByShip(dmgList->get_inflictor_id());
			if (clientIDKiller && type & (OBJ_DOCKING_RING | OBJ_STATION | OBJ_WEAPONS_PLATFORM))
				BaseDestroyed(g_DmgToSpaceID, clientIDKiller);
		}

		if (g_DmgTo && subObjID == 1) // only save hits on the hull (subObjID=1)
		{
			ClientInfo[g_DmgTo].dmgLast = *dmgList;
		}
	}
	CATCH_HOOK({})

	CallPluginsAfter(HookedCall::IEngine__AddDamageEntry, dmgList, subObjID, hitPts, fate);

	g_DmgTo = 0;
	g_DmgToSpaceID = 0;
}

__declspec(naked) void Naked__AddDamageEntry()
{
	__asm {
        push [esp+0Ch]
        push [esp+0Ch]
        push [esp+0Ch]
        push ecx
        call AddDamageEntry
        mov eax, [esp]
        add esp, 10h
        jmp eax
	}
}

/**************************************************************************************************************
Called when ship was damaged
**************************************************************************************************************/

FARPROC g_OldDamageHit, g_OldDamageHit2;

void __stdcall DamageHit(char* ecx)
{
	CallPluginsBefore(HookedCall::IEngine__DamageHit, ecx);

	TRY_HOOK
	{
		char* p;
		memcpy(&p, ecx + 0x10, 4);
		uint clientID;
		memcpy(&clientID, p + 0xB4, 4);
		uint spaceID;
		memcpy(&spaceID, p + 0xB0, 4);

		g_DmgTo = clientID;
		g_DmgToSpaceID = spaceID;
	}
	CATCH_HOOK({})
}

__declspec(naked) void Naked__DamageHit()
{
	__asm {
        push ecx
        push ecx
        call DamageHit
        pop ecx
        jmp [g_OldDamageHit]
	}
}

__declspec(naked) void Naked__DamageHit2()
{
	__asm {
        push ecx
        push ecx
        call DamageHit
        pop ecx
        jmp [g_OldDamageHit2]
	}
}

/**************************************************************************************************************
Called when ship was damaged
**************************************************************************************************************/

bool AllowPlayerDamage(uint iClientID, uint iClientIDTarget)
{
	auto [rval, skip] = CallPluginsBefore<bool>(HookedCall::IEngine__AllowPlayerDamage, iClientID, iClientIDTarget);
	if (skip)
		return rval;

	const auto* config = FLHookConfig::c();

	if (iClientIDTarget)
	{
		// anti-dockkill check
		if (ClientInfo[iClientIDTarget].bSpawnProtected)
		{
			if ((timeInMS() - ClientInfo[iClientIDTarget].tmSpawnTime) <= config->general.antiDockKill)
				return false; // target is protected
			else
				ClientInfo[iClientIDTarget].bSpawnProtected = false;
		}
		if (ClientInfo[iClientID].bSpawnProtected)
		{
			if ((timeInMS() - ClientInfo[iClientID].tmSpawnTime) <= config->general.antiDockKill)
				return false; // target may not shoot
			else
				ClientInfo[iClientID].bSpawnProtected = false;
		}

		// no-pvp check
		uint systemID;
		pub::Player::GetSystem(iClientID, systemID);
		if (std::find(config->general.noPVPSystemsHashed.begin(), config->general.noPVPSystemsHashed.end(), systemID) != config->general.noPVPSystemsHashed.end())
			return false;
	}

	return true;
}

/**************************************************************************************************************
**************************************************************************************************************/

FARPROC g_OldNonGunWeaponHitsBase;

void __stdcall NonGunWeaponHitsBaseBefore(char* ECX, char* p1, DamageList* dmg)
{
	CSimple* simple;
	memcpy(&simple, ECX + 0x10, 4);
	g_LastHitPts = simple->get_hit_pts();

	g_NonGunHitsBase = true;
}

void NonGunWeaponHitsBaseAfter()
{
	g_NonGunHitsBase = false;
}

ulong g_NonGunWeaponHitsBaseRetAddress;

__declspec(naked) void Naked__NonGunWeaponHitsBase()
{
	__asm {
        mov eax, [esp+4]
        mov edx, [esp+8]
        push ecx
        push edx
        push eax
        push ecx
        call NonGunWeaponHitsBaseBefore
        pop ecx

        mov eax, [esp]
        mov [g_NonGunWeaponHitsBaseRetAddress], eax
        lea eax, return_here
        mov [esp], eax
        jmp [g_OldNonGunWeaponHitsBase]
return_here:
        pushad
        call NonGunWeaponHitsBaseAfter
        popad
        jmp [g_NonGunWeaponHitsBaseRetAddress]
	}
}

///////////////////////////
