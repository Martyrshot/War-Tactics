#include <boost/algorithm/string/replace.hpp>
#include <boost/lexical_cast.hpp>
#include <cstdint>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>

#include <misc.hpp>


using namespace std;

namespace eth_interface
{



// https://stackoverflow.com/questions/116038/how-do-i-read-an-entire-file-into-a-stdstring-in-c
string
readFile2(string const& fileName)
{
	ifstream ifs(fileName.c_str(), ios::in | ios::binary | ios::ate);

	ifstream::pos_type fileSize = ifs.tellg();
	ifs.seekg(0, ios::beg);

	vector<char> bytes(fileSize);
	ifs.read(bytes.data(), fileSize);

	return string(bytes.data(), fileSize);
}



bool
isHex(string const& str)
{
	for (char* s = (char*)str.c_str(); *s != 0; s++)
	{
		if ((*s < 48) || (*s > 70 && *s < 97) || (*s > 102))
			return false;
	}
	return true;
}



bool
isEthereumAddress(string const& str)
{
	return isHex(str) && str.length() > 0 && str.length() <= 40;
}



string
escapeSingleQuotes(string const& str)
{
	return boost::replace_all_copy(str, "'", "'\\''");
}



// https://stackoverflow.com/questions/17261798/converting-a-hex-string-to-a-byte-array
vector<char>
hexToBytes(string const& hex)
{
	vector<char> bytes;

	for (unsigned int i = 0; i < hex.length(); i += 2)
	{
		string byteString = hex.substr(i, 2);
		char byte = (char)strtol(byteString.c_str(), NULL, 16);
		bytes.push_back(byte);
	}
	return bytes;
}



// https://stackoverflow.com/questions/14050452/how-to-convert-byte-array-to-hex-string-in-visual-c#14051107
string
hexStr(unsigned char* data, uint16_t len)
{
	std::stringstream ss;
	ss << std::hex;

	for (int i(0); i < len; ++i)
	{
		ss << std::setw(2) << std::setfill('0') << (int)data[i];
	}
	return ss.str();
}



bool
isInt(string& s)
{
	try
	{
		boost::lexical_cast<uint64_t>(s);
	}
	catch (...)
	{
		return false;
	}
	return true;
}


} //namespace
