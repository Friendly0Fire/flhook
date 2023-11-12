#pragma once

struct CargoInfo
{
        uint id;
        int count;
        uint archId;
        float status;
        bool mission;
        bool mounted;
        CacheString hardpoint;
};

struct LaunchData
{
        CEqObj* obj;
        Vector pos;
        Matrix orientation;
        int dock;
};

// money stuff
struct MoneyFix
{
        std::wstring character;
        uint amount;

        bool operator==(const MoneyFix mf1) const
        {
            if (character != mf1.character)
            {
                return true;
            }

            return false;
        };
};

// ignore
struct IgnoreInfo
{
        std::wstring character;
        std::wstring flags;
};

// taken from directplay
typedef struct _DPN_CONNECTION_INFO
{
        DWORD size;
        DWORD roundTripLatencyMS;
        DWORD throughputBPS;
        DWORD peakThroughputBPS;
        DWORD bytesSentGuaranteed;
        DWORD packetsSentGuaranteed;
        DWORD bytesSentNonGuaranteed;
        DWORD packetsSentNonGuaranteed;
        DWORD bytesRetried;
        DWORD packetsRetried;
        DWORD bytesDropped;
        DWORD packetsDropped;
        DWORD messagesTransmittedHighPriority;
        DWORD messagesTimedOutHighPriority;
        DWORD messagesTransmittedNormalPriority;
        DWORD messagesTimedOutNormalPriority;
        DWORD messagesTransmittedLowPriority;
        DWORD messagesTimedOutLowPriority;
        DWORD bytesReceivedGuaranteed;
        DWORD packetsReceivedGuaranteed;
        DWORD bytesReceivedNonGuaranteed;
        DWORD packetsReceivedNonGuaranteed;
        DWORD messagesReceived;
} DPN_CONNECTION_INFO, *PDPN_CONNECTION_INFO;

struct PlayerInfo
{
        uint client;
        std::wstring character;
        std::wstring baseName;
        std::wstring systemName;
        uint system;
        uint ship;
        DPN_CONNECTION_INFO connectionInfo;
        std::wstring IP;
        std::wstring hostname;
};

struct BaseHealth
{
        float currentHealth;
        float maxHealth;
};

struct DataMarketItem
{
        uint archId;
        float rep;
};

struct BaseInfo
{
        uint baseId;
        std::wstring baseName;
        uint objectId;
        bool destroyed;
        std::list<DataMarketItem> MarketMisc;
};

struct GroupMember
{
        uint client;
        std::wstring character;
};

struct SystemInfo
{
        /** The system nickname */
        std::wstring sysNick;

        /** The system id */
        uint systemId;

        /** The system scale */
        float scale;
};

struct TransformMatrix
{
        float d[4][4];
};

struct Zone
{
        /** The system nickname */
        std::wstring sysNick;

        /** The zone nickname */
        std::wstring zoneNick;

        /** The id of the system for this zone */
        uint systemId;

        /** The zone transformation matrix */
        TransformMatrix transform;

        /** The zone ellipsoid size */
        Vector size;

        /** The zone position */
        Vector pos;

        /** The damage this zone causes per second */
        int damage;

        /** Is this an encounter zone */
        bool encounter;
};

class JumpPoint
{
    public:
        /** The system nickname */
        std::wstring sysNick;

        /** The jump point nickname */
        std::wstring jumpNick;

        /** The jump point destination system nickname */
        std::wstring jumpDestSysNick;

        /** The id of the system for this jump point. */
        uint system;

        /** The id of the jump point. */
        uint jumpId;

        /** The jump point destination system id */
        uint jumpDestSysId;
};

struct LootableZone
{
        /** The zone nickname */
        std::wstring zoneNick;

        /** The id of the system for this lootable zone */
        uint systemId;

        /** The nickname and arch id of the loot dropped by the asteroids */
        std::wstring lootNick;
        uint lootId;

        /** The arch id of the crate the loot is dropped in */
        uint crateId;

        /** The minimum number of loot items to drop */
        uint minLoot;

        /** The maximum number of loot items to drop */
        uint maxLoot;

        /** The drop difficultly */
        uint lootDifficulty;

        /** The lootable zone ellipsoid size */
        Vector size;

        /** The lootable zone position */
        Vector pos;
};

struct Light
{
        std::wstring nickname;
        uint archId;
        float bulbSize;
        float glowSize;
        float intensity;
        Vector glowColor;
        Vector color;
        Vector minColor;
        bool dockingLight;
        bool alwaysOn;
        float flareConeMin;
        float flareConeMax;
        int lightSourceCone;

        bool blinks;
        float delay;
        float blinkDuration;
};

template <class Processor>
struct CommandInfo
{
        std::wstring_view cmd;
        void (*func)(Processor* cl, std::vector<std::wstring>& params);
        std::wstring_view usage;
        std::wstring_view description;
};
