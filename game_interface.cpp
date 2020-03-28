#include <array>
#include <boost/algorithm/string/trim.hpp>
#include <boost/lexical_cast.hpp>
#include <iostream>
#include <sstream>
#include <string>

#include <fcntl.h>
#include <string.h>
#include <unistd.h>

#include <game_interface.hpp>
#include <ethabi.hpp>
#include <misc.hpp>




using namespace std;
using namespace libconfig;

namespace game_interface
{



GameInterface::GameInterface(void)
	: EthInterface(GAME_SOL, GAME_ABI, GAME_BIN)
{
	string ipcPath;
	string clientAddress;

	cfg.setOptions(Config::OptionFsync
		| Config::OptionSemicolonSeparators
		| Config::OptionColonAssignmentForGroups
		| Config::OptionOpenBraceOnSeparateLine);

	try
	{
		cfg.readFile(CONFIG_F);
	}
	catch (const FileIOException& e)
	{
		cerr << "IO error reading config file!" << endl;
		exit(EXIT_FAILURE);
	}
	catch (const ParseException& e)
	{
		cerr << "Config file error "
			 << e.getFile()
			 << ":"
			 << e.getLine()
			 << " - "
			 << e.getError()
			 << endl;
		exit(EXIT_FAILURE);
	}

	cfgRoot = &cfg.getRoot();

	if (!cfg.exists("ipcPath"))
	{
		cerr << "Configuration file does not contain 'ipcPath'!" << endl;
		exit(EXIT_FAILURE);
	}

	cfg.lookupValue("ipcPath", ipcPath);
	this->ipcPath = ipcPath;

	if (!cfg.exists("clientAddress"))
	{
		vector<string> accounts = this->eth_accounts();

		if (accounts.size() == 0)
		{
			throw ResourceRequestFailedException(
				"eth_accounts was unable to obtain an account address");
		}

		clientAddress = accounts[0];

		if (cfgRoot->exists("clientAddress"))
			cfgRoot->remove("clientAddress");
		cfgRoot->add("clientAddress", Setting::TypeString) = clientAddress;
		cfg.writeFile(CONFIG_F);

	} else {
		cfg.lookupValue("clientAddress", clientAddress);
	}
	this->clientAddress = clientAddress;

	initialize(ipcPath, clientAddress, contractEventSignatures());

	if (!cfg.exists("helperContractAddress"))
	{
		/* We will compile the helper contract with solc, upload it
		 * to the chain and save the address to the config file */
		try
		{
			helperContractAddress = this->create_contract(HELPER_SOL, HELPER_ABI, HELPER_BIN);

			if (cfgRoot->exists("helperContractAddress"))
				cfgRoot->remove("helperContractAddress");
			cfgRoot->add("helperContractAddress", Setting::TypeString) = helperContractAddress;
			cfg.writeFile(CONFIG_F);
		}
		catch (const TransactionFailedException& e)
		{
			cerr << "Failed to create helper contract!" << endl;
			exit(EXIT_FAILURE);
		}
	} else {
		cfg.lookupValue("helperContractAddress", helperContractAddress);
	}
	this->helperContractAddress = boost::to_lower_copy(boost::trim_copy(helperContractAddress));

	if (this->helperContractAddress.substr(0, 2) == "0x")
	{
		this->helperContractAddress = this->helperContractAddress.substr(2);
	}
}


// TODO
vector<tuple<string, string, bool>>
GameInterface::contractEventSignatures(void)
{
	vector<tuple<string, string, bool>> vecLogSigs;
	vecLogSigs.push_back(make_tuple("JoinGame", "", true));
	vecLogSigs.push_back(make_tuple("NextTurn", "", true));
	vecLogSigs.push_back(make_tuple("CreateDeck", "", true));
	vecLogSigs.push_back(make_tuple("DrawHand", "", true));
	vecLogSigs.push_back(make_tuple("LayPath", "", true));
	vecLogSigs.push_back(make_tuple("LayUnit", "", true));
	vecLogSigs.push_back(make_tuple("MoveUnit", "", true));
	vecLogSigs.push_back(make_tuple("Attack", "", true));
	vecLogSigs.push_back(make_tuple("PlayerJoined", "", false));
	vecLogSigs.push_back(make_tuple("DecksReady", "", false));
	vecLogSigs.push_back(make_tuple("NextTurn", "", false));
	return vecLogSigs;
}



// Throws ResourceRequestFailedException from ethabi()
string
GameInterface::getNoArgs(string const& funcName)
{
	return getFrom(funcName, "");
}



// Throws ResourceRequestFailedException from ethabi()
uint64_t
GameInterface::getInt(string const& funcName)
{
	string value;

	value = getNoArgs(funcName);
	return stoul(value, nullptr, 16);
}



string
GameInterface::createGame(void)
{
	string gameContractAddress;

	gameContractAddress = this->create_contract(GAME_SOL, GAME_ABI, GAME_BIN, helperContractAddress);
	setContractAddress(gameContractAddress);
	createEventLogWaitManager();
}



bool
GameInterface::joinGame(string const& gameAddress)
{
	string ethabiEncodeArgs;
	unique_ptr<unordered_map<string, string>> eventLog;

	ethabiEncodeArgs = " -l -p '" + gameAddress + "'";

	return callMutatorContract("join_game", ethabiEncodeArgs, eventLog);
}



bool
GameInterface::createDeck(uint8_t deckSeed[DECK_SIZE])
{
	string ethabiEncodeArgs = " -l -v 'uint8[]' '[";
	unique_ptr<unordered_map<string, string>> eventLog;

	for (uint8_t i = 0; i < DECK_SIZE; i++)
	{
		if (i != 0)
		{
			ethabiEncodeArgs += ",";
		}
		ethabiEncodeArgs += string(deckSeed[i])
	}

	ethabiEncodeArgs += "]'";
	return callMutatorContract("create_deck", ethabiEncodeArgs, eventLog);
}



bool
GameInterface::drawHand(void)
{
	string ethabiEncodeArgs;
	unique_ptr<unordered_map<string, string>> eventLog;

	ethabiEncodeArgs = "";

	return callMutatorContract("join_game", ethabiEncodeArgs, eventLog);
}



// Throws ResourceRequestFailedException from ethabi()
// Throws InvalidArgumentException
bool
GameInterface::hasPlayers(void)
{
	return getIntFromContract("has_players") == 1;
}



// Throws ResourceRequestFailedException from ethabi()
// Throws InvalidArgumentException
bool
GameInterface::hasDeck(void)
{
	return getIntFromContract("has_deck") == 1;
}



vector<uint8_t>
GameInterface::getPlayerSeedHand(uint8_t playerNum)
{
	return ethabi_decode_uint8_array(
		getEthContractABI(),
		"get_player_seed_hand",
		getArrayFromContract("get_player_seed_hand", " -l -p " + string(playerNum)));
}



uint8_t
GameInterface::getPrivateCardFromSeed(uint8_t cardSeed)
{
	return getIntFromContract("get_private_card_from_seed", " -l -p " + string(cardSeed));
}



std::vector<std::vector<std::vector<uint8_t>>>
GameInterface::getBoardState(void)
{
	uint16_t n = 0;
	vector<vector<vector<uint8_t>>> result;
	vector<string> vec =  ethabi_decode_uint8_array(
		getEthContractABI(),
		"get_board_state",
		getArrayFromContract("get_board_state"));

	// TODO
	if (vec.size() != 3 * 10 * 9) {
		throw ResourceRequestFailedException(
			"getBoardState(): Did not contain the expected quantity of elements");
	}

	for (uint8_t i = 0; i < 3; i++)
	{
		result.push_back(vector<vector<uint8_t>>());
		for (uint8_t j = 0; j < 10; i ++)
		{
			result.push_back(vector<uint8_t>());
			for (uint8_t k = 0; k < 9; k++)
			{
				result[i][j].push_back(vec[n]);
				n++;
			}
		}
	}
	return result;
}



// Throws ResourceRequestFailedException from ethabi()
// Throws InvalidArgumentException
uint8_t
GameInterface::getHqHealth(void)
{
	return getIntFromContract("get_hq_health");
}



// Throws ResourceRequestFailedException from ethabi()
// Throws InvalidArgumentException
uint8_t
GameInterface::isGameOver(void)
{
	return getIntFromContract("is_game_over");
}



// Throws ResourceRequestFailedException from ethabi()
// Throws InvalidArgumentException
bool
GameInterface::myTurn(void)
{
	return getIntFromContract("my_turn") == 1;
}



bool
GameInterface::layPath(uint8_t x,
		uint8_t y,
		uint8_t handIndex,
		uint8_t adjacentPathX,
		uint8_t adjacentPathY)
{
	string ethabiEncodeArgs;
	unique_ptr<unordered_map<string, string>> eventLog;

	ethabiEncodeArgs = "-l -p ";
	ethabiEncodeArgs += x;
	ethabiEncodeArgs += " -p ";
	ethabiEncodeArgs += y;
	ethabiEncodeArgs += " -p ";
	ethabiEncodeArgs += handIndex;
	ethabiEncodeArgs += " -p ";
	ethabiEncodeArgs += adjacentPathX;
	ethabiEncodeArgs += " -p ";
	ethabiEncodeArgs += adjacentPathY;

	return callMutatorContract("lay_path", ethabiEncodeArgs, eventLog);
}



bool
GameInterface::layUnit(uint8_t handIndex)
{
	string ethabiEncodeArgs;
	unique_ptr<unordered_map<string, string>> eventLog;

	ethabiEncodeArgs = "-l -p ";
	ethabiEncodeArgs += handIndex;

	return callMutatorContract("lay_unit", ethabiEncodeArgs, eventLog);
}



bool
GameInterface::moveUnit(uint8_t unitX,
		uint8_t unitY,
		uint8_t moveX,
		uint8_t moveY)
{
	string ethabiEncodeArgs;
	unique_ptr<unordered_map<string, string>> eventLog;

	ethabiEncodeArgs = "-l -p ";
	ethabiEncodeArgs += unitX;
	ethabiEncodeArgs += " -p ";
	ethabiEncodeArgs += unitY;
	ethabiEncodeArgs += " -p ";
	ethabiEncodeArgs += moveX;
	ethabiEncodeArgs += " -p ";
	ethabiEncodeArgs += moveY;

	return callMutatorContract("lay_unit", ethabiEncodeArgs, eventLog);
}



bool
GameInterface::attack(uint8_t unitX,
		uint8_t unitY,
		uint8_t attackX,
		uint8_t attackY)
{
	string ethabiEncodeArgs;
	unique_ptr<unordered_map<string, string>> eventLog;

	ethabiEncodeArgs = "-l -p ";
	ethabiEncodeArgs += unitX;
	ethabiEncodeArgs += " -p ";
	ethabiEncodeArgs += unitY;
	ethabiEncodeArgs += " -p ";
	ethabiEncodeArgs += attackX;
	ethabiEncodeArgs += " -p ";
	ethabiEncodeArgs += attackY;

	return callMutatorContract("attack", ethabiEncodeArgs, eventLog);
}



void
GameInterface::waitPlayerJoined(void)
{
	blockForEvent("PlayerJoined");
}



void
GameInterface::waitNextTurn(void)
{
	if (myTurn())
	{
		return;
	}
	blockForEvent("NextTurn");
}



void
GameInterface::endGame(void)
{
	closeEventLogWaitManager();
}



} //namespace
