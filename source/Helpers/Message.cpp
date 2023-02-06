#include <Global.hpp>

// Very hacky way to call non-header function
namespace IServerImplHook
{
	void __stdcall SubmitChat(CHAT_ID cidFrom, ulong size, void const* rdlReader, CHAT_ID cidTo, int _genArg1);
}

bool g_bMsg;
bool g_bMsgU;
bool g_bMsgS;

_RCSendChatMsg RCSendChatMsg;

namespace Hk::Message
{
	cpp::result<void, Error> Msg(const std::variant<uint, std::wstring>& player, const std::wstring& wscMessage)
	{
		ClientId client = Hk::Client::ExtractClientID(player);

		if (client == UINT_MAX)
			return cpp::fail(Error::PlayerNotLoggedIn);

		const CHAT_ID ci = {0};
		const CHAT_ID ciClient = {client};

		const std::wstring wscXML = L"<TRA data=\"0x19BD3A00\" mask=\"-1\"/><TEXT>" + XMLText(wscMessage) + L"</TEXT>";
		uint iRet;
		char szBuf[1024];
		if (const auto err = FMsgEncodeXML(wscXML, szBuf, sizeof(szBuf), iRet); err.has_error())
			return cpp::fail(err.error());

		IServerImplHook::SubmitChat(ci, iRet, szBuf, ciClient, -1);
		return {};
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////

	cpp::result<void, Error> MsgS(const std::wstring& wscSystemname, const std::wstring& wscMessage)
	{
		uint systemId = 0;
		if (!((systemId = ToInt(wscSystemname))))
		{
			pub::GetSystemID(systemId, wstos(wscSystemname).c_str());
			if (!systemId)
				return cpp::fail(Error::InvalidSystem);
		}

		// prepare xml
		const std::wstring wscXML = L"<TRA data=\"0xE6C68400\" mask=\"-1\"/><TEXT>" + XMLText(wscMessage) + L"</TEXT>";
		uint iRet;
		char szBuf[1024];
		if (const auto err = FMsgEncodeXML(wscXML, szBuf, sizeof(szBuf), iRet); err.has_error())
			return cpp::fail(err.error());

		const CHAT_ID ci = {0};

		// for all players in system...
		PlayerData* playerDb = nullptr;
		while ((playerDb = Players.traverse_active(playerDb)))
		{
			ClientId client = playerDb->iOnlineId;
			uint iClientSystemId = 0;
			pub::Player::GetSystem(client, iClientSystemId);
			if (systemId == iClientSystemId)
			{
				const CHAT_ID ciClient = {client};
				IServerImplHook::SubmitChat(ci, iRet, szBuf, ciClient, -1);
			}
		}

		return {};
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////

	cpp::result<void, Error> MsgU(const std::wstring& wscMessage)
	{
		const CHAT_ID ci = {0};
		const CHAT_ID ciClient = {0x00010000};

		const std::wstring xml = L"<TRA font=\"1\" color=\"#FFFFFF\"/><TEXT>" + XMLText(wscMessage) + L"</TEXT>";
		uint iRet;
		char szBuf[1024];
		if (const auto err = FMsgEncodeXML(xml, szBuf, sizeof(szBuf), iRet); err.has_error())
			return cpp::fail(err.error());

		IServerImplHook::SubmitChat(ci, iRet, szBuf, ciClient, -1);
		return {};
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////

	cpp::result<void, Error> FMsgEncodeXML(const std::wstring& wscXml, char* szBuf, uint iSize, uint& iRet)
	{
		XMLReader rdr;
		RenderDisplayList rdl;
		std::wstring wscMsg = L"<?xml version=\"1.0\" encoding=\"UTF-16\"?><RDL><PUSH/>";
		wscMsg += wscXml;
		wscMsg += L"<PARA/><POP/></RDL>\x000A\x000A";
		if (!rdr.read_buffer(rdl, (const char*)wscMsg.c_str(), wscMsg.length() * 2))
			return cpp::fail(Error::WrongXmlSyntax);

		BinaryRDLWriter rdlwrite;
		rdlwrite.write_buffer(rdl, szBuf, iSize, iRet);
		return {};
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void FMsgSendChat(ClientId client, char* szBuf, uint iSize)
	{
		uint p4 = (uint)szBuf;
		uint p3 = iSize;
		uint p2 = 0x00010000;
		uint p1 = client;

		__asm {
        push [p4]
        push [p3]
        push [p2]
        push [p1]
        mov ecx, [HookClient]
        add ecx, 4
        call [RCSendChatMsg]
		}
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////

	cpp::result<void, Error> FMsg(ClientId client, const std::wstring& wscXML)
	{
		char szBuf[0xFFFF];
		uint iRet;
		if (const auto err = FMsgEncodeXML(wscXML, szBuf, sizeof(szBuf), iRet); err.has_error())
			return cpp::fail(err.error());

		FMsgSendChat(client, szBuf, iRet);
		return {};
	}

	cpp::result<void, Error> FMsg(const std::variant<uint, std::wstring>& player, const std::wstring& wscXML)
	{
		ClientId client = Hk::Client::ExtractClientID(player);

		if (client == UINT_MAX)
			return cpp::fail(Error::PlayerNotLoggedIn);

		return FMsg(client, wscXML);
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////

	cpp::result<void, Error> FMsgS(const SystemId id, const std::wstring& wscXML)
	{
		// encode xml std::string
		char szBuf[0xFFFF];
		uint iRet;
		if (const auto err = FMsgEncodeXML(wscXML, szBuf, sizeof(szBuf), iRet); err.has_error())
			return cpp::fail(err.error());

		// for all players in system...
		PlayerData* playerDb = nullptr;
		while ((playerDb = Players.traverse_active(playerDb)))
		{
			ClientId client = playerDb->iOnlineId;
			uint iClientSystemId = 0;
			pub::Player::GetSystem(client, iClientSystemId);
			if (id == iClientSystemId)
				FMsgSendChat(client, szBuf, iRet);
		}

		return {};
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////

	cpp::result<void, Error> FMsgU(const std::wstring& wscXML)
	{
		// encode xml std::string
		char szBuf[0xFFFF];
		uint iRet;
		const auto err = FMsgEncodeXML(wscXML, szBuf, sizeof(szBuf), iRet);
		if (err.has_error())
			return cpp::fail(err.error());

		// for all players
		PlayerData* playerDb = nullptr;
		while ((playerDb = Players.traverse_active(playerDb)))
		{
			ClientId client = playerDb->iOnlineId;
			FMsgSendChat(client, szBuf, iRet);
		}

		return {};
	}

	/** Format a chat std::string in accordance with the receiver's preferences and
	send it. Will check that the receiver accepts messages from wscSender and
	refuses to send if necessary. */
	cpp::result<void, Error> FormatSendChat(uint iToClientId, const std::wstring& wscSender, const std::wstring& text, const std::wstring& textColor)
	{
#define HAS_FLAG(a, b) ((a).wscFlags.find(b) != -1)

		if (FLHookConfig::i()->userCommands.userCmdIgnore)
		{
			for (auto& ignore : ClientInfo[iToClientId].lstIgnore)
			{
				if (!HAS_FLAG(ignore, L"i") && !(ToLower(wscSender).compare(ToLower(ignore.character))))
					return {}; // ignored
				else if (HAS_FLAG(ignore, L"i") && (ToLower(wscSender).find(ToLower(ignore.character)) != -1))
					return {}; // ignored
			}
		}

		uchar cFormat;
		// adjust chatsize
		switch (ClientInfo[iToClientId].chatSize)
		{
			case CS_SMALL:
				cFormat = 0x90;
				break;
			case CS_BIG:
				cFormat = 0x10;
				break;
			default:
				cFormat = 0x00;
				break;
		}

		// adjust chatstyle
		switch (ClientInfo[iToClientId].chatStyle)
		{
			case CST_BOLD:
				cFormat += 0x01;
				break;
			case CST_ITALIC:
				cFormat += 0x02;
				break;
			case CST_UNDERLINE:
				cFormat += 0x04;
				break;
			default:
				cFormat += 0x00;
				break;
		}

		wchar_t wszFormatBuf[8];
		swprintf(wszFormatBuf, _countof(wszFormatBuf), L"%02X", (long)cFormat);
		const std::wstring wscTRADataFormat = wszFormatBuf;
		const std::wstring wscTRADataSenderColor = L"FFFFFF"; // white

		const std::wstring wscXML = L"<TRA data=\"0x" + wscTRADataSenderColor + wscTRADataFormat + L"\" mask=\"-1\"/><TEXT>" + XMLText(wscSender) +
		    L": </TEXT>" + L"<TRA data=\"0x" + textColor + wscTRADataFormat + L"\" mask=\"-1\"/><TEXT>" + XMLText(text) + L"</TEXT>";

		if (const auto err = FMsg(iToClientId, wscXML); err.has_error())
			return cpp::fail(err.error());

		return {};
	}

	/** Send a player to player message */
	cpp::result<void, Error> SendPrivateChat(uint iFromClientId, uint iToClientId, const std::wstring& text)
	{
		const auto wscSender = Client::GetCharacterNameByID(iFromClientId);
		if (wscSender.has_error())
		{
			Console::ConErr(std::format("Unable to send private chat message from client {}", iFromClientId));
			return {};
		}

		if (FLHookConfig::i()->userCommands.userCmdIgnore)
		{
			for (auto const& ignore : ClientInfo[iToClientId].lstIgnore)
			{
				if (HAS_FLAG(ignore, L"p"))
					return {};
			}
		}

		// Send the message to both the sender and receiver.
		auto err = FormatSendChat(iToClientId, wscSender.value(), text, L"19BD3A");
		if (err.has_error())
			return cpp::fail(err.error());

		err = FormatSendChat(iFromClientId, wscSender.value(), text, L"19BD3A");
		if (err.has_error())
			return cpp::fail(err.error());

		return {};
	}

	/** Send a player to system message */
	void SendSystemChat(uint iFromClientId, const std::wstring& text)
	{
		const std::wstring wscSender = (const wchar_t*)Players.GetActiveCharacterName(iFromClientId);

		// Get the player's current system.
		uint systemId;
		pub::Player::GetSystem(iFromClientId, systemId);

		// For all players in system...
		PlayerData* playerDb = nullptr;
		while ((playerDb = Players.traverse_active(playerDb)))
		{
			ClientId client = playerDb->iOnlineId;
			uint iClientSystemId = 0;
			pub::Player::GetSystem(client, iClientSystemId);
			if (systemId == iClientSystemId)
			{
				// Send the message a player in this system.
				FormatSendChat(client, wscSender, text, L"E6C684");
			}
		}
	}

	/** Send a player to local system message */
	void SendLocalSystemChat(uint iFromClientId, const std::wstring& text)
	{
		const auto wscSender = Client::GetCharacterNameByID(iFromClientId);
		if (wscSender.has_error())
		{
			Console::ConErr(std::format("Unable to send local system chat message from client {}", iFromClientId));
			return;
		}

		// Get the player's current system and location in the system.
		uint systemId;
		pub::Player::GetSystem(iFromClientId, systemId);

		uint iFromShip;
		pub::Player::GetShip(iFromClientId, iFromShip);

		Vector vFromShipLoc;
		Matrix mFromShipDir;
		pub::SpaceObj::GetLocation(iFromShip, vFromShipLoc, mFromShipDir);

		// For all players in system...
		PlayerData* playerDb = nullptr;
		while ((playerDb = Players.traverse_active(playerDb)))
		{
			// Get the this player's current system and location in the system.
			ClientId client = playerDb->iOnlineId;
			uint iClientSystemId = 0;
			pub::Player::GetSystem(client, iClientSystemId);
			if (systemId != iClientSystemId)
				continue;

			uint ship;
			pub::Player::GetShip(client, ship);

			Vector vShipLoc;
			Matrix mShipDir;
			pub::SpaceObj::GetLocation(ship, vShipLoc, mShipDir);

			// Cheat in the distance calculation. Ignore the y-axis
			// Is player within scanner range (15K) of the sending char.
			if (static_cast<float>(sqrt(pow(vShipLoc.x - vFromShipLoc.x, 2) + pow(vShipLoc.z - vFromShipLoc.z, 2))) > 14999.0f)
				continue;

			// Send the message a player in this system.
			FormatSendChat(client, wscSender.value(), text, L"FF8F40");
		}
	}

	/** Send a player to group message */
	void SendGroupChat(uint iFromClientId, const std::wstring& text)
	{
		const wchar_t* wscSender = (const wchar_t*)Players.GetActiveCharacterName(iFromClientId);
		// Format and send the message a player in this group.
		auto lstMembers = Hk::Player::GetGroupMembers(wscSender);
		if (lstMembers.has_error())
		{
			return;
		}

		for (const auto& gm : lstMembers.value())
		{
			FormatSendChat(gm.client, wscSender, text, L"FF7BFF");
		}
	}

	std::vector<HINSTANCE> vDLLs;

	void UnloadStringDLLs()
	{
		for (uint i = 0; i < vDLLs.size(); i++)
			FreeLibrary(vDLLs[i]);
		vDLLs.clear();
	}

	void LoadStringDLLs()
	{
		UnloadStringDLLs();

		HINSTANCE hDLL = LoadLibraryEx("resources.dll", nullptr,
		    LOAD_LIBRARY_AS_DATAFILE); // typically resources.dll
		if (hDLL)
			vDLLs.push_back(hDLL);

		INI_Reader ini;
		if (ini.open("freelancer.ini", false))
		{
			while (ini.read_header())
			{
				if (ini.is_header("Resources"))
				{
					while (ini.read_value())
					{
						if (ini.is_value("DLL"))
						{
							hDLL = LoadLibraryEx(ini.get_value_string(0), nullptr, LOAD_LIBRARY_AS_DATAFILE);
							if (hDLL)
								vDLLs.push_back(hDLL);
						}
					}
				}
			}
			ini.close();
		}
	}

	std::wstring GetWStringFromIdS(uint iIdS)
	{
		if (wchar_t wszBuf[1024]; LoadStringW(vDLLs[iIdS >> 16], iIdS & 0xFFFF, wszBuf, 1024))
			return wszBuf;
		return L"";
	}

	std::wstring FormatMsg(MessageColor color, MessageFormat format, const std::wstring& msg)
	{
		const uint bgrColor = Math::RgbToBgr(static_cast<uint>(color));
		const std::wstring tra = Math::UintToHexString(bgrColor, 6, true) + Math::UintToHexString(static_cast<uint>(format), 2);

		return L"<TRA data=\"" + tra + L"\" mask=\"-1\"/><TEXT>" + XMLText(msg) + L"</TEXT>";
	}
} // namespace Hk::Message