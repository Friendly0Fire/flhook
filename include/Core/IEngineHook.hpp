#pragma once
#include "Core/VTables.hpp"
#include "FLHook.hpp"
#include "Utils/Detour.hpp"

class FLHook;
class IEngineHook
{
        friend FLHook;

        struct LoadRepData
        {
                uint repId;
                float attitude;
        };

        struct RepDataList
        {
                uint dunno;
                LoadRepData* begin;
                LoadRepData* end;
        };

        inline static FARPROC oldLoadReputationFromCharacterFile =
            reinterpret_cast<FARPROC>(FLHook::Offset(FLHook::BinaryType::Server, AddressList::SaveFileHouseEntrySaveAndLoadPatch) + 7);
        inline static FARPROC oldLaunchPosition;
        inline static FARPROC oldDisconnectPacketSent;
        inline static uint lastTicks;

        static int FreeReputationVibe(const int& p1);
        static void UpdateTime(double interval);
        static void __stdcall ElapseTime(float interval);
        static int DockCall(const uint& shipId, const uint& spaceId, int dockPortIndex, DOCK_HOST_RESPONSE response);
        static bool __stdcall LaunchPosition(uint spaceId, CEqObj& obj, Vector& position, Matrix& orientation, int dock);
        static void __fastcall DaLibDisconnect(void* ecx);

#define VTablePtr(x) static_cast<DWORD>(x)

        inline static VTableHook<VTablePtr(CShipVTable::Start), VTablePtr(CShipVTable::End)> cShipVTable{ "common" };
        inline static VTableHook<VTablePtr(CLootVTable::Start), VTablePtr(CLootVTable::End)> cLootVTable{ "common" };
        inline static VTableHook<VTablePtr(CSolarVTable::Start), VTablePtr(CSolarVTable::End)> cSolarVtable{ "common" };

        inline static VTableHook<VTablePtr(IShipInspectVTable::Start), VTablePtr(IShipInspectVTable::End)> iShipVTable{ "server" };
        inline static VTableHook<VTablePtr(ISolarInspectVTable::Start), VTablePtr(ISolarInspectVTable::End)> iSolarVTable{ "server" };
        inline static VTableHook<VTablePtr(ILootInspectVTable::Start), VTablePtr(ILootInspectVTable::End)> iLootVTable{ "server" };

#undef VTablePtr

        static void __fastcall ShipDestroy(Ship* ship, DamageList* dmgList, bool isKill, Id killerId);
        static void __fastcall LootDestroy(Loot* loot, void* edx, bool isKill, uint killerId);
        static void __fastcall SolarDestroy(Solar* solar, void* edx, bool isKill, uint killerId);

        static void __fastcall ShipHullDamage(Ship* ship, void* edx, float damage, DamageList* dmgList);
        static void __fastcall SolarHullDamage(Solar* ship, void* edx, float damage, DamageList* dmgList);

        static void __fastcall ShipExplosionHit(Ship* ship, void* edx, ExplosionDamageEvent* explosion, DamageList* dmgList);

        static void __fastcall CShipInit(CShip* ship, void* edx, CShip::CreateParms* creationParams);
        static void __fastcall CLootInit(CLoot* loot, void* edx, CLoot::CreateParms* createParams);
        static void __fastcall CSolarInit(CSolar* solar, void* edx, CSolar::CreateParms* createParms);

        struct CallAndRet final : Xbyak::CodeGenerator
        {
                CallAndRet(void* toCall, void* ret);
        };

        struct DisconnectPacketSentAssembly final : Xbyak::CodeGenerator
        {
                DisconnectPacketSentAssembly();
        };

        struct LaunchPositionAssembly final : Xbyak::CodeGenerator
        {
                LaunchPositionAssembly();
        };

        struct LoadReputationFromCharacterFileAssembly final : Xbyak::CodeGenerator
        {
                LoadReputationFromCharacterFileAssembly();
        };

        inline static DisconnectPacketSentAssembly* disconnectPacketSentAssembly;
        inline static LoadReputationFromCharacterFileAssembly* loadReputationFromCharacterFileAssembly;
        inline static LaunchPositionAssembly launchPositionAssembly;

        static bool AllowPlayerDamage(ClientId client, ClientId clientTarget);

        static bool __stdcall LoadReputationFromCharacterFile(const RepDataList* savedReps, const LoadRepData* repToSave);
        static bool __stdcall DisconnectPacketSent(ClientId client);
        static void SendDeathMessage(const std::wstring& msg, SystemId systemId, ClientId clientVictim, ClientId clientKiller);

        using CGunWrapperShutdownFunc = void(__fastcall*)(void*);
        inline static std::unique_ptr<FunctionDetour<CGunWrapperShutdownFunc>> disconnectPacketDetour = nullptr;
};
