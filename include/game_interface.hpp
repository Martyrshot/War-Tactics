#ifndef __GAME_INTERFACE_HPP
#define __GAME_INTERFACE_HPP

#include <libconfig.h++>
#include <mutex>
#include <thread>

#include <eth_interface.hpp>
#include <eth_interface_except.hpp>
#include <subscription_server.hpp>


#define CONFIG_F "game.conf"

#define DECK_SIZE     52


using namespace eth_interface;

namespace game_interface
{



class GameInterface : public EthInterface
{
	public:

	GameInterface(void);

	// throws include/eth_interface_exception.hpp::TransactionFailedException
	// if the contract cannot be created
	std::string
	createGame(void);

	// gameAddress is an ethereum address (hex), without leading "0x"
	bool
	joinGame(std::string gameAddress);

	bool
	createDeck(uint8_t deckSeed[DECK_SIZE]);

	// Only to be called to draw the initial hand
	bool
	drawHand(void);

	// Has player2 joined the game?
	bool
	hasPlayers(void);

	// Have both players successfully createDeck() to seed the deck?
	bool
	hasDeck(void);

	// Get a players hand of cryptographically unsigned (hidden) cards
	std::vector<uint8_t>
	getPlayerSeedHand(uint8_t playerNum);

	uint8_t
	getPrivateCardFromSeed(uint8_t cardSeed);

	std::vector<std::vector<std::vector<uint8_t>>>
	getBoardState(void);

	std::pair<uint8_t, uint8_t>
	getHqHealth(void);

	// 0 = Game not over, 1 = player 1 won, 2 = player 2 won
	uint8_t
	isGameOver(void);

	bool
	layPath(uint8_t x,
			uint8_t y,
			uint8_t handIndex,
			uint8_t adjacentPathX,
			uint8_t adjacentPathY);

	bool
	layUnit(uint8_t handIndex);

	bool
	moveUnit(uint8_t unitX,
			uint8_t unitY,
			uint8_t moveX,
			uint8_t moveY);

	bool
	attack(uint8_t unitX,
			uint8_t unitY,
			uint8_t attackX,
			uint8_t attackY);

	void
	waitPlayerJoined(void);

	void
	waitNextTurn(void);



	private:

	libconfig::Config cfg;
	libconfig::Setting* cfgRoot;

	std::vector<std::pair<std::string, std::string>> contractEventSignatures(void);

	std::string getFromDeviceID(std::string const& funcName, uint32_t deviceID);
	uint64_t getIntFromDeviceID(std::string const& funcName, uint32_t deviceID);
	std::string getStringFromDeviceID(std::string const& funcName, uint32_t deviceID);
	std::vector<std::string> getStringsFromDeviceID(std::string const& funcName, uint32_t deviceID);
};


} //namespace


#endif //__GAME_INTERFACE_HPP
