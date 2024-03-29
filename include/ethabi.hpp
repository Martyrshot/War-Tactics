#ifndef __ETHABI_HPP
#define __ETHABI_HPP

#include <string>
#include <unordered_map>
#include <vector>
#include <eth_interface_except.hpp>

#ifndef IPC_BUFFER_LENGTH
#define IPC_BUFFER_LENGTH 256
#endif


namespace eth_interface
{


std::string
ethabi(std::string const &args);

std::unordered_map<std::string, std::string>
ethabi_decode_log(
	std::string const &abiFile,
	std::string const &eventName,
	std::vector<std::string> &topics,
	std::string const &data);

std::string
ethabi_decode_result(
	std::string const &abiFile,
	std::string const &eventName,
	std::string const &data);

std::vector<std::string>
ethabi_decode_results(
	std::string const &abiFile,
	std::string const &eventName,
	std::string const &data);

std::vector<std::string>
ethabi_decode_sting_array(
	std::string const &abiFile,
	std::string const &eventName,
	std::string const &data);

std::vector<uint32_t>
ethabi_decode_uint32_array(
	std::string const &abiFile,
	std::string const &eventName,
	std::string const &data);

std::vector<uint8_t>
ethabi_decode_uint8_array(
	std::string const &abiFile,
	std::string const &eventName,
	std::string const &data);

std::vector<uint8_t>
ethabi_decode_3d_uint8_array(
	std::string const &abiFile,
	std::string const &eventName,
	std::string const &data);


} //namespace


#endif //__ETHABI_HPP
