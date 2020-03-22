pragma solidity >=0.6.0;
pragma experimental ABIEncoderV2;

import "./helper.sol";

// Validating keccak256 signatures in Solidity: https://ethereum.stackexchange.com/questions/710/how-can-i-verify-a-cryptographic-signature-that-was-produced-by-an-ethereum-addr/718

contract Game {

	uint8 constant DECK_SIZE = 52;
	uint8 constant HAND_SIZE = 5;
	uint8 constant BOARD_WIDTH = 10;
	uint8 constant BOARD_HEIGHT = 9;

	uint8 constant STATE_BLANK = 0;
	uint8 constant STATE_PATH  = 1;
	uint8 constant STATE_UNIT  = 2;
	uint8 constant STATE_HQ    = 3;

	struct BoardSpace {
		uint8 card;
		uint8 state;
		uint8 owner;
	}


	BoardSpace[BOARD_WIDTH][BOARD_HEIGHT] board;

	string game_create_time;
	string game_join_time;

	uint8[DECK_SIZE] deck;
	bool player1_turn;


	address player1;
	uint8[] player1_hand;
	int8 player1_deck_top;
	bool has_player1_deck;
	bool has_player1_hand;
	bool has_player1_hq;


	address player2;
	uint8[] player2_hand;
	int8 player2_deck_top;
	bool has_player2_deck;
	bool has_player2_hand;
	bool has_player2_hq;
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


	modifier _players_turn {
		require(msg.sender == player1 || msg.sender == player2);
		require((msg.sender == player1 && player1_turn) || (msg.sender == player2 && !player1_turn));
		_;
	}


	constructor(address _helper) public {
		helper_contract = Helper(_helper);

		player1 = msg.sender;
		player1_deck_top = int8(DECK_SIZE - 1);
		has_player1_deck = false;
		has_player1_hand = false;
		has_player1_hq = false;

		player2_deck_top = int8(DECK_SIZE - 1);
		has_player2_deck = false;
		has_player2_hand = false;
		has_player2_hq = false;
		has_player2 = false;

		game_create_time = helper_contract.uint2str(now);
	}


	function join_game() external returns(bool) {
		require(!has_player2);

		player2 = msg.sender;
		game_join_time = helper_contract.uint2str(now);

		emit PlayerJoined();

		return true;
	}


	function create_deck(uint8[] calldata _deck) external _player {
		require(_deck.length == uint256(DECK_SIZE));

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


	function verify_card(uint8 card, uint8 hidden_card, uint8 modulo, address addr, uint8 v, bytes32 r, bytes32 s) public view returns (bool) {
		bytes32 hash = helper_contract.prefixed(keccak256(abi.encodePacked(game_create_time, game_join_time, hidden_card)));
		return ecrecover(hash, v, r, s) == addr && helper_contract.get_signed_card(v, r, s, modulo) == card;
	}


	function draw_hand() external _player {
		require(has_deck());
		if (player1 == msg.sender) {
			require(!has_player1_hand);
			has_player1_hand = true;
		} else {
			require(!has_player2_hand);
			has_player2_hand = true;
		}
		draw_cards();
	}


	function draw_cards() internal {
		if (msg.sender == player1) {
			for (uint i = player1_hand.length; i < HAND_SIZE && player1_deck_top > -1; i++) {
				player1_hand.push(deck[uint256(player1_deck_top)]);
				player1_deck_top--;
			}
		} else {
			for (uint i = player2_hand.length; i < HAND_SIZE && player2_deck_top > -1; i++) {
				player2_hand.push(deck[uint256(player2_deck_top)]);
				player1_deck_top--;
			}
		}

	}


	function has_players() public view returns (bool) {
		return has_player2;
	}


	function has_deck() public view returns (bool) {
		return has_player1_deck && has_player2_deck;
	}


	function get_player_seed_hand(uint8 player) external view returns (uint8[] memory) {
		require(player == 1 || player == 2);
		if (player == 1) {
			return player1_hand;
		} else {
			return player2_hand;
		}
	}


	function get_board_state() public view returns (BoardSpace[10][9] memory) {
		return board;
	}


	function lay_path(uint8 x, uint8 y, uint8 handIndex, uint8 adjacentUnitX, uint8 adjacentUnitY) external _players_turn returns (bool) {
		uint8 sender;
		int8 diffX = int8(adjacentUnitX - x);
		int8 diffY = int8(adjacentUnitY - y);
		bool has_hq;

		if (msg.sender == player1) {
			if (handIndex >= player1_hand.length) {
				return false;
			}
			sender = 1;
			has_hq = has_player1_hq;
		} else {
			if (handIndex >= player2_hand.length) {
				return false;
			}
			sender = 2;
			has_hq = has_player2_hq;
		}

		if (board[x][y].state != STATE_BLANK) {
			return false;
		}

		if (
			(
				(board[adjacentUnitX][adjacentUnitY].owner != sender) ||
				(board[adjacentUnitX][adjacentUnitY].state == 3) ||
				(diffX < -1 || diffX > 1) ||
				(diffY < -1 || diffY > 1) ||
				(board[adjacentUnitX][adjacentUnitY].state != STATE_HQ) ||
				(board[adjacentUnitX][adjacentUnitY].state != STATE_UNIT)
			) && !has_hq
		) {
			return false;
		}

		if (!has_hq && ((sender == 1 && y == 0) || (sender == 2 && y == BOARD_HEIGHT - 1))) {
			return false;
		}

		board[x][y].owner = sender;
		if (!has_hq) {
			board[x][y].state = STATE_HQ;
			if (sender == 1) {
				has_player1_hq = true;
			} else {
				has_player2_hq = true;
			}
		} else {
			board[x][y].state = STATE_PATH;
		}
		// The card value of board is irrelavent so just ignore it

		if (sender == 1) {
			player1_hand[handIndex] = player1_hand[player1_hand.length - 1];
			player1_hand.pop();
		} else {
			player2_hand[handIndex] = player2_hand[player2_hand.length - 1];
			player2_hand.pop();
		}

		draw_cards();
		player1_turn = !player1_turn;
		return true;
	}


	function lay_unit(uint8 x, uint8 y, uint8 handIndex, uint8 card, uint8 v, bytes32 r, bytes32 s) external _players_turn returns (bool) {
		// TODO

		draw_cards();
		player1_turn = !player1_turn;
		return true;
	}


	function move_unit(uint8 unitX, uint8 unitY, uint8 moveX, uint8 moveY) external _players_turn returns (bool) {
		// TODO

		//draw_cards(); // TODO: No cards were spent from the players hand, so no need to draw, correct?
		player1_turn = !player1_turn;
		return true;
	}


	function attack(uint8 unitX, uint8 unitY, uint8 attackX, uint8 attackY) external _players_turn returns (bool) {
		// TODO

		//draw_cards(); // TODO: No cards were spent from the players hand, so no need to draw, correct?
		player1_turn = !player1_turn;
		return true;
	}

}