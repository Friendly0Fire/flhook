#pragma once

#include "API/Utils/Logger.hpp"

#include "AddressList.hpp"

#include <mongocxx/pool.hpp>

class InternalApi;
class IServerImplHook;
class IEngineHook;
class StartupCache;
class ClientList;
class InfocardManager;
class Logger;
class FlPacket;
class PersonalityHelper;
class Database;
class AccountManager;

class FLHook final
{
        friend IEngineHook;
        friend IServerImplHook;
        friend IClientImpl;
        friend InternalApi;
        friend ClientId;
        friend ShipId;
        friend FlPacket;

        // Static things

        inline static FLHook* instance;

        inline static HMODULE moduleFLHook;
        inline static HMODULE serverDll;
        inline static HMODULE commonDll;
        inline static HMODULE contentDll;
        inline static HMODULE dalibDll;
        inline static HMODULE remoteClient;
        inline static HMODULE flProc;

        inline static RCSendChatMsgT rcSendChatMsg;
        inline static CRCAntiCheatT crcAntiCheat;
        inline static GetShipInspectT getShipInspect;

        inline static bool nonGunHitsBase;
        inline static float lastHitPts;
        inline static ClientId dmgToClient;
        inline static ObjectId dmgToSpaceId;

        inline static CDPServer* cdpServer;
        inline static CDPClientProxy** clientProxyArray;
        inline static IClientImpl* fakeClientImpl;
        inline static IClientImpl* hookClientImpl;
        inline static char* oldClientImpl;

        inline static FARPROC oldUpdateLoop;
        static void Shutdown() { delete instance; }
        static void Startup() { instance = new FLHook(); }

        static void LoadSettings();
        static void ClearClientInfo(ClientId client);
        static void ProcessPendingCommands();
        static void ProcessPendingAsyncTasks();

        // Timers
        static void PublishServerStats();
        static void TimerCheckKick();
        static void TimerNpcAndF1Check();

        // Non-Static things

        ClientList* clientList;
        Database* database;
        InfocardManager* infocardManager;
        PersonalityHelper* personalityHelper;
        AccountManager* accountManager;
        FLHookConfig* flhookConfig;

        bool OnServerStart();
        void InitHookExports();
        static void PatchClientImpl();
        void UnloadHookExports();

        void LoadUserSettings(ClientId client);

        uint damageToClientId;

        bool messagePrivate;
        bool messageSystem;
        bool messageUniverse;

        std::wstring accPath;

        uint serverLoadInMs;
        uint playerCount;
        bool disableNpcs;

        bool flhookReady;

    public:
        FLHook();
        ~FLHook();
        FLHook(const FLHook&) = delete;
        FLHook& operator=(FLHook) = delete;
        FLHook(FLHook&&) = delete;
        FLHook& operator=(FLHook&&) = delete;

        static void SetupEventLoop();

        enum class BinaryType
        {
            Exe,
            Server,
            Common,
            DaLib,
            Content,
            RemoteClient,
        };

        struct LastHitInformation
        {
                bool nonGunHitsBase;
                float lastHitPts;
                ClientId lastHitClient;
                ObjectId lastHitObject;
        };

        static DWORD __stdcall Offset(BinaryType type, AddressList address);

        static bool IsReady() { return instance != nullptr && instance->flhookReady; }
        static std::wstring_view GetAccountPath() { return instance->accPath; }
        static bool GetShipInspect(uint& ship, IObjInspectImpl*& inspect, uint& dunno) { return getShipInspect(ship, inspect, dunno); }

        static ClientList& Clients() { return *instance->clientList; }
        static ClientData& GetClient(ClientId client);
        static Database& GetDatabase() { return *instance->database; }
        static mongocxx::pool::entry GetDbClient();
        static InfocardManager& GetInfocardManager() { return *instance->infocardManager; }
        static LastHitInformation GetLastHitInformation() { return { nonGunHitsBase, lastHitPts, dmgToClient, dmgToSpaceId }; }
        static Action<pub::AI::Personality, Error> GetPersonality(const std::wstring& pilotNickname);
        static AccountManager& GetAccountManager() { return *instance->accountManager; }
        static FLHookConfig& GetConfig() { return *instance->flhookConfig; }
        static IClientImpl* GetPacketInterface(){ return hookClientImpl; }

        static Action<void, Error> MessageUniverse(std::wstring_view message);
};
