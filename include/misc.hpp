#ifndef __MISC_HPP
#define __MISC_HPP

#include <string>
#include <vector>


namespace eth_interface
{


// https://stackoverflow.com/questions/116038/how-do-i-read-an-entire-file-into-a-stdstring-in-c
std::string readFile2(std::string const& fileName);

bool isHex(std::string const& str);
bool isEthereumAddress(std::string const& str);
std::string escapeSingleQuotes(std::string const& str);
std::vector<char> hexToBytes(std::string const& hex);
std::string hexStr(unsigned char* data, uint16_t len);
bool isInt(std::string& s);


} //namespace


#endif // __MISC_HPP
