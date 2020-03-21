pragma solidity >=0.6.0;

import "./helper.sol";

// Validating keccak256 signatures in Solidity: https://ethereum.stackexchange.com/questions/710/how-can-i-verify-a-cryptographic-signature-that-was-produced-by-an-ethereum-addr/718

contract Game {

	uint8 constant DECK_SIZE = 52;
	uint8 constant HAND_SIZE = 5;
	uint8 constant BOARD_WIDTH = 10;
	uint8 constant BOARD_HEIGHT = 9;

	enum owner_e {unowned, player1, player2}

	struct BoardSpace {
		uint8 card;
		uint8 state;
		owner_e owner;
	}


	BoardSpace[BOARD_WIDTH][BOARD_HEIGHT] board;

	string game_create_time;
	string game_join_time;

	uint8[DECK_SIZE] deck;


	address player1;
	uint8[HAND_SIZE] player1_hand;
	bool has_player1_deck;


	address player2;
	uint8[HAND_SIZE] player2_hand;
	bool has_player2_deck;
	bool has_player2;


	Helper helper_contract;



	// keccak256 signature: 683bd2659be7113b3c0113c3c6d6a2d8a84e09a864bada4a03a67998e041ad24
	event PlayerJoined();

	// keccak256 signature: 71f35e93dba038828dc45a6ae7729335a6419de8bb0eb50318b023ce96b2579d
	event DecksReady();

	// keccak256 signature: f3a2f88f7c3271d9fa4567afa6b0a0b4b791579b3b6c5eeacc1de20699269562
	event SubmitDeckSignatures(address indexed sender);

	// keccak256 signature: 9ca5593cfed860b5d72bc51135525cbf7cf071276e2c1501d8618e9b8e6e6179
	event SignaturesVerified();



	modifier _player {
		require(msg.sender == player1 || msg.sender == player2);
		_;
	}

	constructor(address _helper) public {
		helper_contract = Helper(_helper);

		player1 = msg.sender;
		has_player1_deck = false;

		has_player2_deck = false;
		has_player2 = false;

		game_create_time = helper_contract.uint2str(now);
	}


	function join_game() external returns(bool) {
		require(!hasPlayer2);

		player2 = msg.sender;
		game_join_time = helper_contract.uint2str(now);

		emit PlayerJoined();

		return true;
	}


	function create_deck(uint8[] calldata _deck) external _player {
		require(_deck.length == DECK_SIZE);

		if (player1 == msg.sender) {
			require(!has_player1_deck);

			if (has_player2_deck) {
				for (uint8 i = 0; i < DECK_SIZE; i++) {
					deck[i] = deck[i] ^ _deck[i];
				}

			} else {
				for (uint8 i = 0; i < DECK_SIZE; i++) {
					deck[i] = _deck[i];
				}
			}
			has_player1_deck = true;

		} else if (player2 == msg.sender) {
			require(!has_player2_deck);

			if (has_player1_deck) {
				for (uint8 i = 0; i < DECK_SIZE; i++) {
					deck[i] = deck[i] ^ _deck[i];
				}

			} else {
				for (uint8 i = 0; i < DECK_SIZE; i++) {
					deck[i] = _deck[i];
				}
			}
			has_player2_deck = true;
		}

		if (has_player1_deck && has_player2_deck) {
			emit DecksReady();
		}
	}


	function signature_to_card(uint8 v, bytes32 r, bytes32 s) internal returns (uint8) {
		uint8 memory c = v;
		for (uint8 i = 0; i < 32; i++) {
			c = c ^ r ^ s;
		}
		return c % DECK_SIZE;
	}


	function verify_card(uint8 card, uint8 hidden_card, uint8 modulo, address addr, uint8 v, bytes32 r, bytes32 s) internal returns (bool) {
		bytes32 memory hash = helper_contract.strConcat(game_create_time, game_join_time, hidden_card);
		return ecrecover(hash, v, r, s) == addr && signature_to_card(v, r, s) == card;
	}


	function has_players() public view returns (bool) {
		return has_player2;
	}


	function has_deck() public view returns (bool) {
		return has_player1_deck && has_player2_deck;
	}


	function get_board_state() public view returns (board[10][9]) {
		return board;
	}


}