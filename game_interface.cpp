#include <array>
#include <boost/algorithm/string/trim.hpp>
#include <boost/algorithm/string.hpp>
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
}



vector<tuple<string, string, bool>>
GameInterface::contractEventSignatures(void)
{
	vector<tuple<string, string, bool>> vecLogSigs;
	vecLogSigs.push_back(make_tuple("JoinGame", "9d148569af2a4ae8c34122247102efb7bb91bf1b595c37c539b852954707d482", true));
	vecLogSigs.push_back(make_tuple("PlayerJoined", "683bd2659be7113b3c0113c3c6d6a2d8a84e09a864bada4a03a67998e041ad24", false));
	vecLogSigs.push_back(make_tuple("DecksReady", "858525375c500ca80978906562ef417241555482ba83de63d3fc7fe8e11a2d93", false));
	vecLogSigs.push_back(make_tuple("NextTurn", "c6c2d48c8a994a16a48e9f7d44b32ae365947a6ccdf319f1e1e6cf8565fa56b4", true));
	vecLogSigs.push_back(make_tuple("CreateDeck", "505a777520798d10945a146762a340313d69a0d0948ef094010e3acb756bc39a", true));
	vecLogSigs.push_back(make_tuple("DrawHand", "a35c8ba1ade945124a66883ef6a7f1759c50d504956f47cb07abd61b0d42f641", true));
	vecLogSigs.push_back(make_tuple("LayPath", "2688c69b58ee503b249854e32a7292cd26fd8475aff735ea7fa79fb622d1baaa", true));
	vecLogSigs.push_back(make_tuple("LayUnit", "649e3c66552bba57438a370b0196029a5bec44e46c3d54c098f8cb61be7592b6", true));
	vecLogSigs.push_back(make_tuple("MoveUnit", "43f643101e992dd5eb86c3e17afc53dd49cae9982bc31be8d92673ac08374ae4", true));
	vecLogSigs.push_back(make_tuple("Attack", "bc346c9c919fa67d688bb542823110622183cfeb46d3ac3d5119d20c1a52f17d", true));
	return vecLogSigs;
}



string
GameInterface::createGame(void)
{
	string gameContractAddress;

	gameContractAddress = this->create_contract(GAME_SOL, GAME_ABI, GAME_BIN, "");
	setContractAddress(gameContractAddress);
	createEventLogWaitManager();
	return gameContractAddress;
}



bool
GameInterface::joinGame(string const& gameAddress)
{
	string ethabiEncodeArgs;
	unique_ptr<unordered_map<string, string>> eventLog;

	setContractAddress(gameAddress);
	createEventLogWaitManager();
	ethabiEncodeArgs = "";

	return callMutatorContract("join_game", ethabiEncodeArgs, eventLog);
}



bool
GameInterface::createDeck(uint8_t deckSeed[DECK_SIZE])
{
	string ethabiEncodeArgs = " -p '[";
	unique_ptr<unordered_map<string, string>> eventLog;

	for (uint8_t i = 0; i < DECK_SIZE; i++)
	{
		if (i != 0)
		{
			ethabiEncodeArgs += ",";
		}
		ethabiEncodeArgs += to_string(deckSeed[i]);
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

	return callMutatorContract("draw_hand", ethabiEncodeArgs, eventLog);
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
		getArrayFromContract("get_player_seed_hand", " -p " + playerNum));
}



string
GameInterface::getCardHash(uint8_t cardSeed)
{
	return ethabi_decode_result(
		getEthContractABI(),
		"get_card_hash",
		getFrom("get_card_hash", " -p '" + boost::lexical_cast<string>(cardSeed) + "'"));
}



uint8_t
GameInterface::getPrivateCardFromSeed(uint8_t cardSeed)
{
	string hash = getCardHash(cardSeed);
	while (hash.length() < 130) {
		hash = "0" + hash;
	}

	return getIntFromContract(
		"get_private_card_from_seed",
		" -p " + hash.substr(0, 2) +
		" -p " + hash.substr(2, 64) +
		" -p " + hash.substr(66, 64));
}



std::vector<std::vector<std::vector<uint8_t>>>
GameInterface::getBoardState(void)
{
	uint16_t n = 0;
	vector<vector<vector<uint8_t>>> result(3, vector<vector<uint8_t>> (10, vector<uint8_t> (9, 0)));
	vector<uint8_t> vec =  ethabi_decode_uint8_array(
		getEthContractABI(),
		"get_board_state",
		getArrayFromContract("get_board_state"));

	if (vec.size() != 3 * 10 * 9) {
		throw ResourceRequestFailedException(
			"getBoardState(): Did not contain the expected quantity of elements");
	}

	for (uint8_t i = 0; i < 3; i++)
	{
		for (uint8_t j = 0; j < 10; i ++)
		{
			for (uint8_t k = 0; k < 9; k++)
			{
				result[i][j][k] = vec[n];
				n++;
			}
		}
	}
	return result;
}



// Throws ResourceRequestFailedException from ethabi()
// Throws InvalidArgumentException
vector<uint8_t>
GameInterface::getHqHealth(void)
{
	return ethabi_decode_uint8_array(
		getEthContractABI(),
		"get_hq_health",
		getArrayFromContract("get_hq_health", ""));
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

	ethabiEncodeArgs = " -p ";
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

	ethabiEncodeArgs = " -p ";
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

	ethabiEncodeArgs = "-p ";
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

	ethabiEncodeArgs = "-p ";
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
