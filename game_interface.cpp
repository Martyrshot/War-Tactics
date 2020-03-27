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
#include <cpp-base64/base64.h>
#include <ethabi.hpp>
#include <misc.hpp>




using namespace std;

namespace game_interface
{



GameInterface::GameInterface(void)
	: EthInterface(GAME_SOL, GAME_ABI, GAME_BIN)
{
	std::string ipcPath;
	std::string clientAddress;

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


	cfg.lookupValue("helperContractAddress", helperContractAddress);

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
			cfg.writeFile(BLOCKCHAINSEC_CONFIG_F);
		}
		catch (const TransactionFailedException& e)
		{
			cerr << "Failed to create helper contract!" << endl;
			exit(EXIT_FAILURE);
		}
	}

	// TODO
	initialize(ipcPath, clientAddress, contractEventSignatures());
}


// TODO
vector<pair<string, string>>
GameInterface::contractEventSignatures(void)
{
	vector<pair<string, string>> vecLogSigs;
	vecLogSigs.push_back(pair<string, string>("JoinGame", ""));
	vecLogSigs.push_back(pair<string, string>("NextTurn", ""));
	vecLogSigs.push_back(pair<string, string>("CreateDeck", ""));
	vecLogSigs.push_back(pair<string, string>("DrawHand", ""));
	vecLogSigs.push_back(pair<string, string>("LayPath", ""));
	vecLogSigs.push_back(pair<string, string>("LayUnit", ""));
	vecLogSigs.push_back(pair<string, string>("MoveUnit", ""));
	vecLogSigs.push_back(pair<string, string>("Attack", ""));
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
	// TODO
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
	// TODO
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
		getArrayFromContract("get_player_seed_hand", " -l -p " + static_cast<string>(playerNum)));
}



uint8_t
GameInterface::getPrivateCardFromSeed(uint8_t cardSeed)
{
	return getIntFromContract("get_private_card_from_seed", " -l -p " + static_cast<string>(cardSeed));
}



std::vector<std::vector<std::vector<uint8_t>>>
GameInterface::getBoardState(void)
{
	// TODO
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



bool
GameInterface::layPath(uint8_t x,
		uint8_t y,
		uint8_t handIndex,
		uint8_t adjacentPathX,
		uint8_t adjacentPathY)
{
	// TODO
}



bool
GameInterface::layUnit(uint8_t handIndex)
{
	// TODO
}



bool
GameInterface::moveUnit(uint8_t unitX,
		uint8_t unitY,
		uint8_t moveX,
		uint8_t moveY)
{
	// TODO
}



bool
GameInterface::attack(uint8_t unitX,
		uint8_t unitY,
		uint8_t attackX,
		uint8_t attackY)
{
	// TODO
}



void
GameInterface::waitPlayerJoined(void)
{
	// TODO
}



void
GameInterface::waitNextTurn(void)
{
	// TODO
}



} //namespace
