pragma solidity >=0.6.0;



contract Game {
	uint32 game_create_time;
	uint32 game_join_time;

	address player1;
	uint8[] player1_deck;
	bool has_player1_deck;

	address player2;
	uint8[] player2_deck;
	bool has_player2_deck;
	bool has_player2;


	// keccak256 signature: 683bd2659be7113b3c0113c3c6d6a2d8a84e09a864bada4a03a67998e041ad24
	event PlayerJoined();

	// keccak256 signature: 71f35e93dba038828dc45a6ae7729335a6419de8bb0eb50318b023ce96b2579d
	event DecksReady();


	modifier _player {
		require(msg.sender == player1 || msg.sender == player2);
		_;
	}

	constructor() public {
		player1 = msg.sender;
		has_player1_deck = false;
		has_player2_deck = false;
		has_player2 = false;
		game_create_time = now;
	}


	function has_players() external view returns(bool) {
		return has_player2;
	}


	function has_decks() external view returns(bool) {
		return has_player1_deck && has_player2_deck;
	}


	function join_game() external returns(bool) {
		require(!hasPlayer2);

		player2 = msg.sender;
		game_join_time = now;

		emit PlayerJoined();

		return true;
	}


	function create_deck(uint8[] calldata my_deck, uint8[] calldata their_deck) external _player {
		require(my_deck.length == 52 && their_deck.length == 52);

		if (player1 == msg.sender) {
			require(!has_player1_deck);

			if (has_player2_deck) {
				for (uint8 i = 0; i < 52; i++) {
					player1_deck[i] = player1_deck[i] ^ my_deck[i];
					player2_deck[i] = player2_deck[i] ^ their_deck[i];
				}

			} else {
				player1_deck = my_deck;
				player2_deck = their_deck;
			}
			has_player1_deck = true;

		} else if (player2 == msg.sender) {
			require(!has_player2_deck);

			if (has_player1_deck) {
				for (uint8 i = 0; i < 52; i++) {
					player2_deck[i] = player2_deck[i] ^ my_deck[i];
					player1_deck[i] = player1_deck[i] ^ their_deck[i];
				}

			} else {
				player2_deck = my_deck;
				player1_deck = their_deck;
			}
			has_player2_deck = true;
		}

		if (has_player1_deck && has_player2_deck) {
			emit DecksReady();
		}
	}

}
