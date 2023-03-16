#pragma once

namespace Hk::Admin
{
	DLL std::wstring GetPlayerIP(ClientId client);
	DLL cpp::result<PlayerInfo, Error> GetPlayerInfo(const std::variant<uint, std::wstring>& player, bool alsoCharmenu);
	DLL std::list<PlayerInfo> GetPlayers();
	DLL cpp::result<DPN_CONNECTION_INFO, Error> GetConnectionStats(ClientId client);
	DLL cpp::result<void, Error> SetAdmin(const std::variant<uint, std::wstring>& player, const std::wstring& rights);
	DLL cpp::result<std::wstring, Error> GetAdmin(const std::variant<uint, std::wstring>& player);
	DLL cpp::result<void, Error> DelAdmin(const std::variant<uint, std::wstring>& player);
	DLL cpp::result<void, Error> ChangeNPCSpawn(bool disable);
	DLL cpp::result<BaseHealth, Error> GetBaseStatus(const std::wstring& basename);
	DLL Fuse* GetFuseFromID(uint fuseId);
	DLL bool LightFuse(IObjRW* ship, uint fuseId, float delay, float lifetime, float skip);
	DLL bool UnLightFuse(IObjRW* ship, uint fuseId);
	DLL CEqObj* GetEqObjFromObjRW(IObjRW* objRW);
}