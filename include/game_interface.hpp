#ifndef __GAME_INTERFACE_HPP
#define __GAME_INTERFACE_HPP

#include <boost/multi_array.hpp>
#include <libconfig.h++>
#include <mutex>
#include <thread>

#include <eth_interface.hpp>
#include <eth_interface_except.hpp>
#include <event_wait_mgr.hpp>


#define CONFIG_F "game.conf"

#define GAME_SOL "contracts/game.sol"
#define GAME_ABI "contracts/game.abi"
#define GAME_BIN "contracts/game.bin"

#define DECK_SIZE 52

//Number of choices in the main menu
#define MENU_ITEMS 3


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
	joinGame(std::string const &gameAddress);

	bool
	createDeck(uint8_t deckSeed[DECK_SIZE]);

	// Only to be called to draw the initial hand
	bool
	drawHand(void);

	bool
	placeHq(uint8_t x);

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

	// boost::multi_array<uint8_t, 3> (boost::extents[3][10][9])
	std::vector<std::vector<std::vector<uint8_t>>>
	getBoardState(void);

	std::vector<uint8_t>
	getHqHealth(void);

	// 0 = Game not over, 1 = player 1 won, 2 = player 2 won
	uint8_t
	isGameOver(void);

	bool
	myTurn(void);

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
	waitDecksReady(void);

	void
	waitGameStart(void);

	void
	waitNextTurn(void);

	void
	endGame(void);

	private:
	libconfig::Config cfg;
	libconfig::Setting *cfgRoot;

	std::vector<std::tuple<std::string, std::string, bool>>
	contractEventSignatures(void);

	std::string
	getCardHash(uint8_t cardSeed);

	std::string
	getHandCardHash(uint8_t handIndex);
};


} //namespace


#endif //__GAME_INTERFACE_HPP
