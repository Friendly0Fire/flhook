// Purchase Restrictions plugin for FLHookPlugin
// Feb 2010 by Cannon
//
// Ported by Raikkonen 2022
//
// This is free software; you can redistribute it and/or modify it as
// you wish without restriction. If you do then I would appreciate
// being notified and/or mentioned somewhere.

#include "Main.h"

namespace Plugins::PurchaseRestrictions
{
	const auto global = std::make_unique<Global>();

	//! Log items of interest so we can see what cargo cheats people are using.
	static void LogItemsOfInterest(ClientId& client, uint iGoodId, const std::string& details)
	{
		const auto iter = global->itemsOfInterestHashed.find(iGoodId);
		if (iter != global->itemsOfInterestHashed.end())
		{
			const auto charName = Hk::Client::GetCharacterNameByID(client);
			AddLog(LogType::Normal,
			    LogLevel::Info,
			    fmt::format("Item '{}' found in cargo of {} - {}", iter->second.c_str(), wstos(charName.value()), details.c_str()));
		}
	}

	void LoadSettings()
	{
		auto config = Serializer::JsonToObject<Config>();

		for (const auto& str : config.itemsOfInterest)
		{
			global->itemsOfInterestHashed[CreateID(str.c_str())] = str;
		}

		for (const auto& str : config.unbuyableItems)
		{
			global->unbuyableItemsHashed.emplace_back(CreateID(str.c_str()));
		}

		for (const auto& [key, value] : config.goodItemRestrictions)
		{
			auto& map = global->goodItemRestrictionsHashed[CreateID(key.c_str())] = {};
			for (const auto& i : value)
				map.emplace_back(CreateID(i.c_str()));
		}

		for (const auto& [key, value] : config.shipItemRestrictions)
		{
			auto& map = global->shipItemRestrictionsHashed[CreateID(key.c_str())] = {};
			for (const auto& i : value)
				map.emplace_back(CreateID(i.c_str()));
		}
		global->config = std::make_unique<Config>(config);
	}

	/// Check that this client is allowed to buy/mount this piece of equipment or
	/// ship Return true if the equipment is mounted to allow this good.
	bool CheckIdEquipRestrictions(ClientId client, uint iGoodId, bool isShip)
	{
		const auto list = isShip ? global->shipItemRestrictionsHashed : global->goodItemRestrictionsHashed;

		const auto validItem = list.find(iGoodId);
		if (validItem == list.end())
			return true;

		int remainingHoldSize;
		const auto cargo = Hk::Player::EnumCargo(client, remainingHoldSize);
		return std::any_of(cargo.value().begin(), cargo.value().end(), [validItem](const CARGO_INFO& cargo) {
			return cargo.bMounted && std::find(validItem->second.begin(), validItem->second.end(), cargo.iArchId) != validItem->second.end();
		});
	}

	void ClearClientInfo(ClientId& client)
	{
		global->clientSuppressBuy[client] = false;
	}

	void PlayerLaunch(uint& ship, ClientId& client)
	{
		global->clientSuppressBuy[client] = false;
	}

	void BaseEnter(uint& iBaseId, ClientId& client)
	{
		global->clientSuppressBuy[client] = false;
	}

	/// Suppress the buying of goods.
	void GFGoodBuy(struct SGFGoodBuyInfo const& gbi, ClientId& client)
	{
		auto& suppress = global->clientSuppressBuy[client] = false;
		LogItemsOfInterest(client, gbi.iGoodId, "good-buy");

		if (std::find(global->unbuyableItemsHashed.begin(), global->unbuyableItemsHashed.end(), gbi.iGoodId) != global->unbuyableItemsHashed.end())
		{
			suppress = true;
			pub::Player::SendNNMessage(client, pub::GetNicknameId("info_access_denied"));
			PrintUserCmdText(client, global->config->goodPurchaseDenied);
			global->returnCode = ReturnCode::SkipAll;
		}

		/// Check restrictions for the Id that a player has.
		if (global->config->checkItemRestrictions)
		{
			// Check Item
			if (global->goodItemRestrictionsHashed.find(gbi.iGoodId) != global->goodItemRestrictionsHashed.end())
			{
				if (!CheckIdEquipRestrictions(client, gbi.iGoodId, false))
				{
					const auto charName = Hk::Client::GetCharacterNameByID(client);
					AddLog(LogType::Normal, LogLevel::Info, fmt::format("{} attempting to buy {} without correct Id", wstos(charName.value()), gbi.iGoodId));
					if (global->config->enforceItemRestrictions)
					{
						PrintUserCmdText(client, global->config->goodPurchaseDenied);
						pub::Player::SendNNMessage(client, pub::GetNicknameId("info_access_denied"));
						suppress = true;
						global->returnCode = ReturnCode::SkipAll;
					}
				}
			}
			else
			{
				// Check Ship
				const GoodInfo* packageInfo = GoodList::find_by_id(gbi.iGoodId);
				if (packageInfo->iType != 3)
				{
					return;
				}

				const GoodInfo* hullInfo = GoodList::find_by_id(packageInfo->iHullGoodId);
				if (hullInfo->iType != 2)
				{
					return;
				}

				if (global->shipItemRestrictionsHashed.find(hullInfo->shipGoodId) != global->shipItemRestrictionsHashed.end() &&
				    !CheckIdEquipRestrictions(client, hullInfo->shipGoodId, true))
				{
					const auto charName = Hk::Client::GetCharacterNameByID(client);
					AddLog(LogType::Normal,
					    LogLevel::Info,
					    fmt::format("{} attempting to buy {} without correct Id", wstos(charName.value()), hullInfo->shipGoodId));
					if (global->config->enforceItemRestrictions)
					{
						PrintUserCmdText(client, global->config->shipPurchaseDenied);
						pub::Player::SendNNMessage(client, pub::GetNicknameId("info_access_denied"));
						suppress = true;
						global->returnCode = ReturnCode::SkipAll;
					}
				}
			}
		}
	}

	/// Suppress the buying of goods.
	void ReqAddItem(uint& goodId, char const* hardpoint, int& count, float& status, bool& mounted, ClientId& client)
	{
		LogItemsOfInterest(client, goodId, "add-item");
		if (global->clientSuppressBuy[client])
		{
			global->returnCode = ReturnCode::SkipAll;
		}
	}

	/// Suppress the buying of goods.
	void ReqChangeCash(int& iMoneyDiff, ClientId& client)
	{
		if (global->clientSuppressBuy[client])
		{
			global->clientSuppressBuy[client] = false;
			global->returnCode = ReturnCode::SkipAll;
		}
	}

	/// Suppress ship purchases
	void ReqSetCash(int& iMoney, ClientId& client)
	{
		if (global->clientSuppressBuy[client])
		{
			global->returnCode = ReturnCode::SkipAll;
		}
	}

	/// Suppress ship purchases
	void ReqEquipment(class EquipDescList const& eqDesc, ClientId& client)
	{
		if (global->clientSuppressBuy[client])
		{
			global->returnCode = ReturnCode::SkipAll;
		}
	}

	/// Suppress ship purchases
	void ReqShipArch(uint& iArchId, ClientId& client)
	{
		if (global->clientSuppressBuy[client])
		{
			global->returnCode = ReturnCode::SkipAll;
		}
	}

	/// Suppress ship purchases
	void ReqHullStatus(float& fStatus, ClientId& client)
	{
		if (global->clientSuppressBuy[client])
		{
			global->clientSuppressBuy[client] = false;
			global->returnCode = ReturnCode::SkipAll;
		}
	}
} // namespace Plugins::PurchaseRestrictions

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
// FLHOOK STUFF
///////////////////////////////////////////////////////////////////////////////////////////////////////////////

using namespace Plugins::PurchaseRestrictions;
REFL_AUTO(type(Config), field(checkItemRestrictions), field(enforceItemRestrictions), field(shipPurchaseDenied), field(goodPurchaseDenied),
    field(itemsOfInterest), field(unbuyableItems), field(goodItemRestrictions), field(shipItemRestrictions));

DefaultDllMainSettings(LoadSettings)

    // Functions to hook
    extern "C" EXPORT void ExportPluginInfo(PluginInfo* pi)
{
	pi->name("Purchase Restrictions");
	pi->shortName("PurchaseRestrictions");
	pi->mayUnload(true);
	pi->returnCode(&global->returnCode);
	pi->versionMajor(PluginMajorVersion::VERSION_04);
	pi->versionMinor(PluginMinorVersion::VERSION_00);
	pi->emplaceHook(HookedCall::IServerImpl__BaseEnter, &BaseEnter);
	pi->emplaceHook(HookedCall::FLHook__ClearClientInfo, &ClearClientInfo);
	pi->emplaceHook(HookedCall::IServerImpl__GFGoodBuy, &GFGoodBuy);
	pi->emplaceHook(HookedCall::FLHook__LoadSettings, &LoadSettings, HookStep::After);
	pi->emplaceHook(HookedCall::IServerImpl__PlayerLaunch, &PlayerLaunch);
	pi->emplaceHook(HookedCall::IServerImpl__ReqAddItem, &ReqAddItem);
	pi->emplaceHook(HookedCall::IServerImpl__ReqChangeCash, &ReqChangeCash);
	pi->emplaceHook(HookedCall::IServerImpl__ReqEquipment, &ReqEquipment);
	pi->emplaceHook(HookedCall::IServerImpl__ReqHullStatus, &ReqHullStatus);
	pi->emplaceHook(HookedCall::IServerImpl__ReqSetCash, &ReqSetCash);
	pi->emplaceHook(HookedCall::IServerImpl__ReqShipArch, &ReqShipArch);
}
