#pragma once

// TODO: Rename, and find more appropriate file/location
class CLIENT_INFO
{
    public:
        void SaveAccountData();

        // JSON object of various keys and custom data. Some of these will map directly on to members of this class, others are by accessor only.
        nlohmann::json accountData;

        uint client;
        std::wstring characterName;
        std::wstring characterFile;

        // kill msgs
        uint ship;
        uint shipOld;
        mstime tmSpawnTime;

        DamageList dmgLast;

        // money cmd
        std::list<MONEY_FIX> moneyFix;

        // anticheat
        uint tradePartner;

        // change cruise disruptor behaviour
        bool cruiseActivated;
        bool thrusterActivated;
        bool engineKilled;
        bool tradelane;

        // idle kicks
        uint baseEnterTime;
        uint charMenuEnterTime;

        // msg, wait and kick
        mstime tmKickTime;

        // eventmode
        uint lastExitedBaseId;
        bool disconnected;

        // f1 laming
        mstime tmF1Time;
        mstime tmF1TimeDisconnect;

        // ignore usercommand
        std::list<IgnoreInfo> ignoreInfoList;

        // user settings
        DIEMSGTYPE dieMsg;
        CHATSIZE dieMsgSize;
        CHATSTYLE dieMsgStyle;
        CHATSIZE chatSize;
        CHATSTYLE chatStyle;

        // MultiKillMessages
        uint killsInARow;

        // bans
        uint connects; // incremented when player connects

        // Group
        uint groupId;

        // other
        std::wstring hostname;

        bool spawnProtected;

        // Your randomly assigned formation tag, e.g. Navy Lambda 1-6
        uint formationNumber1;
        uint formationNumber2;
        uint formationTag;
};

// Magic Enum Extensions
using namespace magic_enum::bitwise_operators; // NOLINT

// variables
extern DLL HANDLE hProcFL;
extern DLL HMODULE server;
extern DLL HMODULE common;
extern DLL HMODULE remoteClient;
extern DLL HMODULE hModDPNet;
extern DLL HMODULE hModDaLib;
extern DLL HMODULE content;
extern DLL FARPROC fpOldUpdate;

DLL void UserCmd_SetDieMsg(ClientId& client, const std::wstring& param);
DLL void UserCmd_SetChatFont(ClientId& client, const std::wstring& param);
DLL void PrintUserCmdText(ClientId client, std::wstring_view text);
DLL void PrintLocalUserCmdText(ClientId client, std::wstring_view msg, float distance);

DLL extern bool g_NonGunHitsBase;
DLL extern float g_LastHitPts;

// namespaces
namespace IServerImplHook
{
    struct SubmitData
    {
            bool inSubmitChat;
            std::wstring characterName;
    };

    // If the namespace is not set, resharper specifically will fail to determine types correctly. I do not know why.

    const DLL extern std::unique_ptr<IServerImplHook::SubmitData> chatData;
} // namespace IServerImplHook

extern DLL CDPServer* cdpSrv;
extern DLL _GetShipInspect GetShipInspect;
extern DLL char* g_FLServerDataPtr;
extern DLL CDPClientProxy** clientProxyArray;
extern DLL _RCSendChatMsg RCSendChatMsg;
extern DLL _CRCAntiCheat CRCAntiCheat;
extern DLL IClientImpl* FakeClient;
extern DLL IClientImpl* HookClient;
extern DLL char* OldClient;

extern DLL std::array<CLIENT_INFO, MaxClientId + 1> ClientInfo;
