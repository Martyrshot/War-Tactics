pragma solidity >=0.6.0;

// Validating keccak256 signatures in Solidity: https://ethereum.stackexchange.com/questions/710/how-can-i-verify-a-cryptographic-signature-that-was-produced-by-an-ethereum-addr/718

contract Game {

	enum owner_e {unowned, player1, player2}

	struct BoardSpace {
		uint8 card;
		uint8 state;
		owner_e owner;
	}

	BoardSpace[10][9] board;

	uint8 constant DECK_SIZE = 52;

	string game_create_time;
	string game_join_time;

	address player1;
	uint8[] player1_deck;
	uint8[] player1_revealed_deck;
	bool player1_cheated;
	bool has_player1_deck;
	bool has_player1_decksigs;

	address player2;
	uint8[] player2_deck;
	uint8[] player2_revealed_deck;
	bool player2_cheated;
	bool has_player2_deck;
	bool has_player2_decksigs;
	bool has_player2;


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

	constructor() public {
		player1 = msg.sender;
		has_player1_deck = false;
		has_player1_decksigs = false;
		player1_cheated = false;

		has_player2_deck = false;
		has_player2_decksigs = false;
		has_player2 = false;
		player2_cheated = false;

		game_create_time = uint2str(now);
	}


	function signature_to_card(uint8 v, bytes32 r, bytes32 s) internal returns(uint8) {
		uint8 memory c = v;
		for (uint8 i = 0; i < 32; i++) {
			c = c ^ r ^ s;
		}
		return c % DECK_SIZE;
	}


	function verify_card(uint8 card, uint8 hidden_card, uint8 modulo, address addr, uint8 v, bytes32 r, bytes32 s) internal returns(bool) {
		bytes32 memory hash = strConcat(game_create_time, game_join_time, hidden_card);
		return ecrecover(hash, v, r, s) == addr && signature_to_card(v, r, s) == card;
	}


	function get_timestamps() external view _player returns(uint32, uint32) {
		require(has_player2);
		return (game_creation_time, game_join_time);
	}


	function has_players() public view returns(bool) {
		return has_player2;
	}


	function has_decks() public view returns(bool) {
		return has_player1_deck && has_player2_deck;
	}


	function get_player1_deck() external view returns(uint8[] memory) {
		require(has_player1_deck && has_player2_deck);
		return player1_deck;
	}


	function get_player2_deck() external view returns(uint8[] memory) {
		require(has_player1_deck && has_player2_deck);
		return player2_deck;
	}


	// WORK IN PROGRESS
	function submit_deck_signatures(uint8[] calldata v, bytes32[] calldata r, bytes32[] calldata s) external _player {
		require(v.length == DECK_SIZE && r.length == DECK_SIZE && r.length == DECK_SIZE);
		uint8 memory hidden_card;

		if (player1 == msg.sender) {
			require(!has_player1_decksigs);

			for (uint8 i = 0; i < DECK_SIZE; i++) {
				if (msg.sender == player1) {
					hidden_card = player1_deck[i];
				} else {
					hidden_card = player2_deck[i];
				}

				if (!verify_card(card, hidden_card, i, addr, v[i], r[i], s[i])) {
					if (msg.sender == player1) {
						player1_cheated = true;
					} else {
						player2_cheated = true;
					}
					break;
				}
				player1_revealed_deck[i] = player1_deck[i];
			}

			has_player1_decksigs = true;

		} else if (player2 == msg.sender) {
			require(!has_player2_decksigs);

		}

		emit SubmitDeckSignatures(msg.sender);

		if (has_player1_decksigs && has_player2_decksigs) {
			emit SignaturesVerified();
		}
	}


	function join_game() external returns(bool) {
		require(!hasPlayer2);

		player2 = msg.sender;
		game_join_time = uint2str(now);

		emit PlayerJoined();

		return true;
	}


	function create_deck(uint8[] calldata my_deck, uint8[] calldata their_deck) external _player {
		require(my_deck.length == DECK_SIZE && their_deck.length == DECK_SIZE);

		if (player1 == msg.sender) {
			require(!has_player1_deck);

			if (has_player2_deck) {
				for (uint8 i = 0; i < DECK_SIZE; i++) {
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
				for (uint8 i = 0; i < DECK_SIZE; i++) {
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


	// https://github.com/provable-things/ethereum-api/blob/master/oraclizeAPI_0.5.sol
	function strConcat(string _a, string _b, string _c, string _d, string _e) internal returns (string){
		bytes memory _ba = bytes(_a);
		bytes memory _bb = bytes(_b);
		bytes memory _bc = bytes(_c);
		bytes memory _bd = bytes(_d);
		bytes memory _be = bytes(_e);
		string memory abcde = new string(_ba.length + _bb.length + _bc.length + _bd.length + _be.length);
		bytes memory babcde = bytes(abcde);
		uint k = 0;
		for (uint i = 0; i < _ba.length; i++) babcde[k++] = _ba[i];
		for (i = 0; i < _bb.length; i++) babcde[k++] = _bb[i];
		for (i = 0; i < _bc.length; i++) babcde[k++] = _bc[i];
		for (i = 0; i < _bd.length; i++) babcde[k++] = _bd[i];
		for (i = 0; i < _be.length; i++) babcde[k++] = _be[i];
		return string(babcde);
	}

	function strConcat(string _a, string _b, string _c, string _d) internal returns (string) {
		return strConcat(_a, _b, _c, _d, "");
	}

	function strConcat(string _a, string _b, string _c) internal returns (string) {
		return strConcat(_a, _b, _c, "", "");
	}

	function strConcat(string _a, string _b) internal returns (string) {
		return strConcat(_a, _b, "", "", "");
	}

	// https://github.com/provable-things/ethereum-api/blob/master/oraclizeAPI_0.5.sol
	function uint2str(uint _i) internal pure returns (string memory _uintAsString) {
		if (_i == 0) {
			return "0";
		}
		uint j = _i;
		uint len;
		while (j != 0) {
			len++;
			j /= 10;
		}
		bytes memory bstr = new bytes(len);
		uint k = len - 1;
		while (_i != 0) {
			bstr[k--] = byte(uint8(48 + _i % 10));
			_i /= 10;
		}
		return string(bstr);
	}

}
