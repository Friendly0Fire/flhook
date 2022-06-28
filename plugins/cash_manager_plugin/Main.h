#pragma once

#include <FLHook.hpp>
#include <plugin.h>

namespace Plugins::CashManager
{
	struct Config final : Reflectable
	{
		std::string File() override { return "flhook_plugins/cash_manager.json"; }

		// The minimum transfer amount.
		int minimumTransfer = 0;

		// Transfers are not allowed to/from chars in this system.
		uint blockedSystem = 0;

		// Enable in dock cash cheat detection
		bool cheatDetection = false;

		// Prohibit transfers if the character has not been online for at least this
		// time
		int minimumTime = 0;
	};

	//! Global data for this plugin
	struct Global final
	{
		std::unique_ptr<Config> config = nullptr;

		// Other fields
		ReturnCode returnCode = ReturnCode::Default;
	};
}

