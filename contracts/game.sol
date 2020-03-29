pragma solidity >=0.6.0;


// Validating keccak256 signatures in Solidity: https://ethereum.stackexchange.com/questions/710/how-can-i-verify-a-cryptographic-signature-that-was-produced-by-an-ethereum-addr/718

contract Game {

	uint8 constant PLAYER1 = 0;
	uint8 constant PLAYER2 = 1;

	uint8 constant HQ_HITPOINTS = 2;

	uint8 constant DECK_SIZE = 52;
	uint8 constant HAND_SIZE = 5;
	uint8 constant BOARD_WIDTH = 10;
	uint8 constant BOARD_HEIGHT = 9;

	uint8 constant STATE_BLANK = 0;
	uint8 constant STATE_PATH = 1;
	uint8 constant STATE_PATH_AND_UNIT = 2;
	uint8 constant STATE_HQ = 3;
	uint8 constant STATE_HQ_AND_UNIT = 4;

	uint8 constant BOARD_CARD = 0;
	uint8 constant BOARD_STATE = 1;
	uint8 constant BOARD_OWNER = 2;


	uint8[3][BOARD_WIDTH][BOARD_HEIGHT] board;

	string game_create_time;
	string game_join_time;

	uint8[DECK_SIZE] deck;
	bool player1_turn;


	address[2] player;
	uint8[][2] player_hand;
	int8[2] player_deck_top;
	uint8[2] player_hq;
	uint8[2] player_hq_damage;
	bool[2] has_player_deck;
	bool[2] has_player_hand;
	bool[2] has_player_hq;

	bool has_player2;
	bool game_over;
	bool player1_won;



	// keccak256 signature: 9d148569af2a4ae8c34122247102efb7bb91bf1b595c37c539b852954707d482
	event JoinGame(address indexed sender);

	// keccak256 signature: 683bd2659be7113b3c0113c3c6d6a2d8a84e09a864bada4a03a67998e041ad24
	event PlayerJoined();

	// keccak256 signature: 858525375c500ca80978906562ef417241555482ba83de63d3fc7fe8e11a2d93
	event DecksReady();

	// keccak256 signature: c6c2d48c8a994a16a48e9f7d44b32ae365947a6ccdf319f1e1e6cf8565fa56b4
	event NextTurn();

	// keccak256 signature: 505a777520798d10945a146762a340313d69a0d0948ef094010e3acb756bc39a
	event CreateDeck(address indexed sender);

	// keccak256 signature: a35c8ba1ade945124a66883ef6a7f1759c50d504956f47cb07abd61b0d42f641
	event DrawHand(address indexed sender);

	// keccak256 signature: 2688c69b58ee503b249854e32a7292cd26fd8475aff735ea7fa79fb622d1baaa
	event LayPath(address indexed sender);

	// keccak256 signature: 649e3c66552bba57438a370b0196029a5bec44e46c3d54c098f8cb61be7592b6
	event LayUnit(address indexed sender);

	// keccak256 signature: 43f643101e992dd5eb86c3e17afc53dd49cae9982bc31be8d92673ac08374ae4
	event MoveUnit(address indexed sender);

	// keccak256 signature: bc346c9c919fa67d688bb542823110622183cfeb46d3ac3d5119d20c1a52f17d
	event Attack(address indexed sender);



	modifier _player {
		require(msg.sender == player[PLAYER1] || msg.sender == player[PLAYER2]);
		_;
	}


	modifier _players_turn {
		require(msg.sender == player[PLAYER1] || msg.sender == player[PLAYER2]);
		require((msg.sender == player[PLAYER1] && player1_turn) || (msg.sender == player[PLAYER2] && !player1_turn));
		_;
	}


	constructor() public {

		player[PLAYER1] = msg.sender;
		player_deck_top[PLAYER1] = int8(DECK_SIZE - 1);
		has_player_deck[PLAYER1] = false;
		has_player_hand[PLAYER1] = false;
		has_player_hq[PLAYER1] = false;

		player_deck_top[PLAYER2] = int8(DECK_SIZE - 1);
		has_player_deck[PLAYER2] = false;
		has_player_hand[PLAYER2] = false;
		has_player_hq[PLAYER2] = false;
		has_player2 = false;

		game_create_time = uint2str(now);
	}


	function join_game() external returns(bool) {
		require(!has_player2);

		player[PLAYER2] = msg.sender;
		game_join_time = uint2str(now);

		emit JoinGame(msg.sender);
		emit PlayerJoined();

		return true;
	}


	function create_deck(uint8[] calldata _deck) external _player returns (bool) {
		require(_deck.length == uint256(DECK_SIZE));

		if (player[PLAYER1] == msg.sender) {
			require(!has_player_deck[PLAYER1]);

			if (has_player_deck[PLAYER2]) {
				for (uint8 i = 0; i < DECK_SIZE; i++) {
					deck[i] = deck[i] ^ _deck[i];
				}

			} else {
				for (uint8 i = 0; i < DECK_SIZE; i++) {
					deck[i] = _deck[i];
				}
			}
			has_player_deck[PLAYER1] = true;

		} else if (player[PLAYER2] == msg.sender) {
			require(!has_player_deck[PLAYER2]);

			if (has_player_deck[PLAYER1]) {
				for (uint8 i = 0; i < DECK_SIZE; i++) {
					deck[i] = deck[i] ^ _deck[i];
				}

			} else {
				for (uint8 i = 0; i < DECK_SIZE; i++) {
					deck[i] = _deck[i];
				}
			}
			has_player_deck[PLAYER2] = true;
		}

		if (has_player_deck[PLAYER1] && has_player_deck[PLAYER2]) {
			emit DecksReady();
		}

		emit CreateDeck(msg.sender);
		return true;
	}


	function get_private_card_from_seed(uint8 v, bytes32 r, bytes32 s) public pure returns (uint8) {
		uint8 c = v;
		for (uint8 i = 0; i < 32; i++) {
			c = c ^ uint8(r[i]) ^ uint8(s[i]);
		}
		return c % DECK_SIZE;
	}


	function get_card_hash(uint8 cardSeed) public view returns (bytes32) {
		return prefixed(keccak256(abi.encodePacked(game_create_time, game_join_time, cardSeed)));
	}


	function verify_card(uint8 card, uint8 cardSeed, address addr, uint8 v, bytes32 r, bytes32 s) public view returns (bool) {
		bytes32 hash = get_card_hash(cardSeed);
		return ecrecover(hash, v, r, s) == addr && get_private_card_from_seed(v, r, s) == card;
	}


	function draw_hand() external _player returns (bool) {
		require(has_deck());
		if (player[PLAYER1] == msg.sender) {
			require(!has_player_hand[PLAYER1]);
			has_player_hand[PLAYER1] = true;
		} else {
			require(!has_player_hand[PLAYER2]);
			has_player_hand[PLAYER2] = true;
		}
		draw_cards();

		emit DrawHand(msg.sender);
	}


	function draw_cards() internal {
		uint8 p;
		if (msg.sender == player[PLAYER1]) {
			p = PLAYER1;
		} else {
			p = PLAYER2;
		}

		for (uint i = player_hand[p].length; i < HAND_SIZE && player_deck_top[p] > -1; i++) {
			(player_hand[p]).push(deck[uint256(player_deck_top[p])]);
			player_deck_top[p]--;
		}
	}


	function has_players() public view returns (bool) {
		return has_player2;
	}


	function has_deck() public view returns (bool) {
		return has_player_deck[PLAYER1] && has_player_deck[PLAYER2];
	}


	function is_game_over() external view returns (uint8) {
		if (!game_over) {
			return 0;
		}

		if (player1_won) {
			return 1;
		}
		return 2;
	}


	function my_turn() external view returns (bool) {
		if ((player1_turn && player[PLAYER1] == msg.sender) || (!player1_turn && player[PLAYER2] == msg.sender)) {
			return true;
		}
		return false;
	}


	function get_player_seed_hand(uint8 playerNum) external view returns (uint8[] memory) {
		require(playerNum == 1 || playerNum == 2);

		return player_hand[playerNum - 1];
	}


	function get_board_state() public view returns (uint8[3][10][9] memory) {
		return board;
	}


	function get_hq_health() external view returns (uint8, uint8) {
		return (HQ_HITPOINTS - player_hq_damage[0], HQ_HITPOINTS - player_hq_damage[1]);
	}


	function lay_path(uint8 x, uint8 y, uint8 handIndex, uint8 adjacentPathX, uint8 adjacentPathY) external _players_turn returns (bool) {
		uint8 sender;
		uint8 other;

		if (msg.sender == player[PLAYER1]) {
			sender = PLAYER1;
			other = PLAYER2;
		} else {
			sender = PLAYER2;
			other = PLAYER1;
		}

		if (handIndex >= player_hand[sender].length) {
			return false;
		}

		if (board[BOARD_STATE][x][y] != STATE_BLANK) {
			return false;
		}

		if (
			(
				(board[BOARD_OWNER][adjacentPathX][adjacentPathY] != sender + 1) ||
				(board[BOARD_STATE][adjacentPathX][adjacentPathY] != STATE_PATH_AND_UNIT) ||
				!check_neighbouring(x, y, adjacentPathX, adjacentPathY) ||
				(board[BOARD_STATE][adjacentPathX][adjacentPathY] == STATE_BLANK)
			) && !has_player_hq[sender]
		) {
			return false;
		}

		if (!has_player_hq[sender] && ((sender == PLAYER1 && y == 0) || (sender == PLAYER2 && y == BOARD_HEIGHT - 1))) {
			return false;
		}

		if (!has_player_hq[sender]) {
			board[BOARD_STATE][x][y] = STATE_HQ;
			player_hq[sender] = x;
			has_player_hq[sender] = true;

		} else {
			board[BOARD_STATE][x][y] = STATE_PATH;
		}

		// The card value of board is irrelavent so just ignore it
		board[BOARD_OWNER][x][y] = sender + 1;

		player_hand[sender][handIndex] = player_hand[sender][player_hand[sender].length - 1];
		player_hand[sender].pop();

		draw_cards();
		player1_turn = !player1_turn;
		emit LayPath(msg.sender);
		emit NextTurn();
		return true;
	}


	function lay_unit(uint8 handIndex, uint8 card, uint8 v, bytes32 r, bytes32 s) external _players_turn returns (bool) {
		uint8 sender;
		uint8 other;

		if (msg.sender == player[PLAYER1]) {
			sender = PLAYER1;
			other = PLAYER2;
		} else {
			sender = PLAYER2;
			other = PLAYER1;
		}

		if (handIndex >= player_hand[sender].length) {
			return false;
		}

		if (!has_player_hq[sender] || board[BOARD_STATE][player_hq[sender]][0] != STATE_HQ) {
			return false;
		}

		if (!verify_card(card, player_hand[sender][handIndex], msg.sender, v, r, s)) {
			return false;
		}

		player_hand[sender][handIndex] = player_hand[sender][player_hand[sender].length - 1];
		player_hand[sender].pop();

		draw_cards();
		player1_turn = !player1_turn;
		emit LayUnit(msg.sender);
		emit NextTurn();
		return true;
	}


	function move_unit(uint8 unitX, uint8 unitY, uint8 moveX, uint8 moveY) external _players_turn returns (bool) {
		uint8 sender;
		uint8 other;

		if (msg.sender == player[PLAYER1]) {
			sender = PLAYER1;
			other = PLAYER2;
		} else {
			sender = PLAYER2;
			other = PLAYER1;
		}

		if (board[BOARD_OWNER][unitX][unitY] != sender + 1 ||
			board[BOARD_STATE][unitX][unitY] != STATE_PATH_AND_UNIT ||
			!check_neighbouring(unitX, unitY, moveX, moveY) ||
			(
				board[BOARD_STATE][moveX][moveY] != STATE_PATH &&
				(board[BOARD_STATE][moveX][moveY] != STATE_HQ || board[BOARD_OWNER][unitX][unitY] != sender + 1)
			)
		) {
			return false;
		}

		board[BOARD_CARD][moveX][moveY] = board[BOARD_CARD][unitX][unitY];
		if (board[BOARD_STATE][moveX][moveY] == STATE_HQ) {
			board[BOARD_STATE][moveX][moveY] = STATE_HQ_AND_UNIT;
		} else {
			board[BOARD_STATE][moveX][moveY] = STATE_PATH_AND_UNIT;
			board[BOARD_OWNER][moveX][moveY] = sender + 1;
		}

		if (board[BOARD_STATE][unitX][unitY] == STATE_HQ_AND_UNIT) {
			board[BOARD_STATE][unitX][unitY] = STATE_HQ;
		} else {
			board[BOARD_STATE][unitX][unitY] = STATE_PATH;
		}

		player1_turn = !player1_turn;
		emit MoveUnit(msg.sender);
		emit NextTurn();
		return true;
	}


	function attack(uint8 unitX, uint8 unitY, uint8 attackX, uint8 attackY) external _players_turn returns (bool) {
		uint8 sender;
		uint8 other;
		uint8 attackerCard;
		uint8 attackeeCard;

		if (msg.sender == player[PLAYER1]) {
			sender = PLAYER1;
			other = PLAYER2;
		} else {
			sender = PLAYER2;
			other = PLAYER1;
		}

		if (board[BOARD_OWNER][unitX][unitY] != sender + 1 ||
			board[BOARD_STATE][unitX][unitY] != STATE_PATH_AND_UNIT ||
			!check_neighbouring(unitX, unitY, attackX, attackY)
		) {
			return false;
		}

		if (board[BOARD_OWNER][attackX][attackY] != other + 1 ||
			(
				board[BOARD_STATE][attackX][attackY] != STATE_HQ &&
				board[BOARD_STATE][attackX][attackY] != STATE_HQ_AND_UNIT &&
				board[BOARD_STATE][attackX][attackY] != STATE_PATH_AND_UNIT
			)
		){
			return false;
		}

		if (board[BOARD_STATE][attackX][attackY] == STATE_HQ) {
			// Inflict damage on HQ
			player_hq_damage[other]++;

			if (player_hq_damage[other] == HQ_HITPOINTS) {
				game_over = true;
				if (sender == PLAYER1) {
					player1_won = true;
				}

				return true;
			}
		}

		attackerCard = board[BOARD_CARD][unitX][unitY] % 13;
		attackeeCard = board[BOARD_CARD][attackX][attackY] % 13;

		if (attackerCard == attackeeCard) {
			// Tie, both cards are lost
			if (board[BOARD_STATE][unitX][unitY] == STATE_HQ_AND_UNIT) {
				board[BOARD_STATE][unitX][unitY] = STATE_HQ;
			} else {
				board[BOARD_STATE][unitX][unitY] = STATE_PATH;
			}

			if (board[BOARD_STATE][attackX][attackY] == STATE_HQ_AND_UNIT) {
				board[BOARD_STATE][attackX][attackY] = STATE_HQ;
			} else {
				board[BOARD_STATE][attackX][attackY] = STATE_PATH;
			}

		} else if (attackerCard > attackeeCard) {
			// Attacker wins (unitX, unitY)
			if (board[BOARD_STATE][attackX][attackY] == STATE_HQ_AND_UNIT) {
				board[BOARD_STATE][attackX][attackY] = STATE_HQ;
			} else {
				board[BOARD_STATE][attackX][attackY] = STATE_PATH;
			}

		} else {
			// Attackee wins (attackX, attackY)
			if (board[BOARD_STATE][unitX][unitY] == STATE_HQ_AND_UNIT) {
				board[BOARD_STATE][unitX][unitY] = STATE_HQ;
			} else {
				board[BOARD_STATE][unitX][unitY] = STATE_PATH;
			}

		}

		// No cards were spent from the players hand, so no need to draw, correct?
		player1_turn = !player1_turn;
		emit Attack(msg.sender);
		emit NextTurn();
		return true;
	}


	function check_neighbouring(uint8 ax, uint8 ay, uint8 bx, uint8 by) internal pure returns (bool) {
		int8 diffX = int8(ax - bx);
		int8 diffY = int8(ay - by);

		if (((diffX == -1 || diffX == 1) && diffY == 0) || ((diffY == -1 || diffY == 1) && diffX == 0)) {
			return true;
		}
		return false;
	}

	// https://solidity.readthedocs.io/en/v0.6.3/solidity-by-example.html
	/// builds a prefixed hash to mimic the behavior of eth_sign.
	function prefixed(bytes32 hash) internal pure returns (bytes32) {
		return keccak256(abi.encodePacked("\x19ethereum signed message:\n32", hash));
	}


	// https://github.com/provable-things/ethereum-api/blob/master/oraclizeAPI_0.5.sol
	function uint2str(uint _i) public pure returns (string memory _uintAsString) {
		uint i = _i;
		if (i == 0) {
			return "0";
		}
		uint j = i;
		uint len;
		while (j != 0) {
			len++;
			j /= 10;
		}
		bytes memory bstr = new bytes(len);
		uint k = len - 1;
		while (i != 0) {
			bstr[k--] = byte(uint8(48 + i % 10));
			i /= 10;
		}
		return string(bstr);
	}
}