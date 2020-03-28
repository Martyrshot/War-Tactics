pragma solidity >=0.6.0;

contract Helper {

	// Game related functions

	function get_signed_card(uint8 v, bytes32 r, bytes32 s, uint8 modulo) public pure returns (uint8) {
		uint8 c = v;
		for (uint8 i = 0; i < 32; i++) {
			c = c ^ uint8(r[i]) ^ uint8(s[i]);
		}
		return c % modulo;
	}


	function check_neighbouring(uint8 ax, uint8 ay, uint8 bx, uint8 by) public pure returns (bool) {
		int8 diffX = int8(ax - bx);
		int8 diffY = int8(ay - by);

		if ((diffX == -1 || diffX == 1) && diffY == 0) {
			return true;
		}
		if ((diffY == -1 || diffY == 1) && diffX == 0) {
			return true;
		}
		return false;
	}





	// Functions not specific to the game


	// https://github.com/provable-things/ethereum-api/blob/master/oraclizeAPI_0.5.sol
	function strConcat(string memory _a, string memory _b, string memory _c, string memory _d, string memory _e) public pure returns (string memory){
		bytes memory _ba = bytes(_a);
		bytes memory _bb = bytes(_b);
		bytes memory _bc = bytes(_c);
		bytes memory _bd = bytes(_d);
		bytes memory _be = bytes(_e);
		string memory abcde = new string(_ba.length + _bb.length + _bc.length + _bd.length + _be.length);
		bytes memory babcde = bytes(abcde);
		uint k = 0;
		uint i;
		for (i = 0; i < _ba.length; i++) babcde[k++] = _ba[i];
		for (i = 0; i < _bb.length; i++) babcde[k++] = _bb[i];
		for (i = 0; i < _bc.length; i++) babcde[k++] = _bc[i];
		for (i = 0; i < _bd.length; i++) babcde[k++] = _bd[i];
		for (i = 0; i < _be.length; i++) babcde[k++] = _be[i];
		return string(babcde);
	}

	function strConcat(string memory _a, string memory _b, string memory _c, string memory _d) public pure returns (string memory) {
		return strConcat(_a, _b, _c, _d, "");
	}

	function strConcat(string memory _a, string memory _b, string memory _c) public pure returns (string memory) {
		return strConcat(_a, _b, _c, "", "");
	}

	function strConcat(string memory _a, string memory _b) public pure returns (string memory) {
		return strConcat(_a, _b, "", "", "");
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


	// https://solidity.readthedocs.io/en/v0.6.3/solidity-by-example.html
	/// builds a prefixed hash to mimic the behavior of eth_sign.
	function prefixed(bytes32 hash) public pure returns (bytes32) {
		return keccak256(abi.encodePacked("\x19ethereum signed message:\n32", hash));
	}
}