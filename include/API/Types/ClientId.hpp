#pragma once

#include "API/Types/BaseId.hpp"
#include "API/Types/ShipId.hpp"
#include "API/Types/SystemId.hpp"

class ShipId;
class ClientData;

class ClientId
{
        uint value = 0;

        [[nodiscard]]
        Action<void, Error> AdjustCash(int amount) const;

        [[nodiscard]]
        bool IsValidClientId() const;

        [[nodiscard]]
        static uint GetClientIdFromCharacterName(std::wstring_view name);

    public:
        explicit ClientId(const uint val) : value(val) {}
        explicit ClientId(const SpecialChatIds id) { value = static_cast<const uint>(id); }
        explicit ClientId(const std::wstring_view str) : value(GetClientIdFromCharacterName(str)){};
        explicit ClientId() = default;

        explicit operator uint() const noexcept { return value; }
        bool operator==(const ClientId next) const { return value == next.value; }
        explicit operator bool() const;

        // Returns the underlying value of the ClientId, it is generally recommended to not use this.

        [[nodiscard]]
        uint GetValue() const
        {
            return value;
        }

        // Type Conversions

        [[nodiscard]]
        Action<std::wstring_view, Error> GetCharacterName() const;
        [[nodiscard]]
        Action<BaseId, Error> GetCurrentBase() const;
        [[nodiscard]]
        Action<SystemId, Error> GetSystemId() const;
        [[nodiscard]]
        Action<CAccount *, Error> GetAccount() const;
        [[nodiscard]]
        Action<const Archetype::Ship *, Error> GetShipArch();
        [[nodiscard]]
        Action<ShipId, Error> GetShipId();
        [[nodiscard]]
        Action<CPlayerGroup *, Error> GetGroup();
        [[nodiscard]]
        Action<RepId, Error> GetReputation() const;
        [[nodiscard]]
        Action<CShip *, Error> GetShip();
        [[nodiscard]]
        Action<uint, Error> GetRank();
        [[nodiscard]]
        Action<uint, Error> GetWealth();
        [[nodiscard]]
        Action<int, Error> GetPvpKills();
        [[nodiscard]]
        Action<uint, Error> GetCash();
        [[nodiscard]]
        Action<std::list<CargoInfo>, Error> EnumCargo(int &remainingHoldSize) const;
        [[nodiscard]]
        ClientData &GetData() const;

        // State Checks

        [[nodiscard]]
        bool InSpace() const;
        [[nodiscard]]
        bool IsDocked() const;
        [[nodiscard]]
        bool InCharacterSelect() const;
        [[nodiscard]]
        bool IsAlive() const;

        // Manipulation

        Action<void, Error> AddToGroup(uint group);
        Action<void, Error> Kick(const std::optional<std::wstring_view> &reason = {}, std::optional<uint> delay = {});
        Action<void, Error> MessageAndKick(std::wstring_view reason, uint delay = 10);
        Action<void, Error> SaveChar();
        Action<void, Error> SetPvpKills(uint killAmount);
        Action<void, Error> AddCash(uint amount);
        Action<void, Error> RemoveCash(uint amount);
        Action<void, Error> Beam(std::variant<BaseId, std::wstring_view> base);
        Action<void, Error> Rename(std::wstring_view);
        void MarkObject(uint objId, int markStatus);

        // Chat

        Action<void, Error> Message(const std::wstring_view message, const MessageFormat format = MessageFormat::Normal,
                                    const MessageColor color = MessageColor::Default) const;
        Action<void, Error> MessageFrom(ClientId destinationClient, std::wstring message) const;

        void Save();
};
