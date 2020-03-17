#ifndef __ETH_INTERFACE_HPP
#define __ETH_INTERFACE_HPP

#include <thread>
#include <mutex>

#include "json/include/nlohmann/json.hpp"
#include <sodium.h>

#include <eth_interface_except.hpp>
#include <subscription_server.hpp>

#define ETH_GET_TRANSRECEIPT_MAX_RETRIES 25
#define ETH_GET_TRANSRECEIPT_RETRY_DELAY 1

#define IPC_BUFFER_LENGTH 128

#define ETH_DEFAULT_GAS "0x7A120"


using Json = nlohmann::json;

namespace eth_interface
{



class EthInterface
{
	public:
	EthInterface(
		std::string ethContractSOL,
		std::string ethContractABI,
		std::string ethContractBIN);

	void initialize(
		std::string ipcPath,
		std::string clientAddress,
		std::string contractAddress,
		std::vector<std::pair<std::string, std::string>> contractEventSignatures);

	std::string getClientAddress(void);
	std::string getContractAddress(void);
	std::string getEthContractSOL(void);
	std::string getEthContractABI(void);
	std::string getEthContractBIN(void);

	void joinThreads(void);

	protected:
	std::string ipcPath;
	std::string clientAddress;
	std::string contractAddress;
	std::string ethContractSOL;
	std::string ethContractABI;
	std::string ethContractBIN;

	EventLogWaitManager* eventLogWaitManager;

	std::string getFrom(std::string const& funcName, std::string const& ethabiEncodeArgs);
	uint64_t getIntFromContract(std::string const& funcName);
	std::string getArrayFromContract(std::string const& funcName);
	Json call_helper(std::string const& data);
	std::unique_ptr<std::unordered_map<std::string, std::string>> contract_helper(std::string const& data);
	bool callMutatorContract(
		std::string const& funcName,
		std::string const& ethabiEncodeArgs,
		std::unique_ptr<std::unordered_map<std::string, std::string>>& eventLog);

	std::string create_contract(void);
	std::string getTransactionReceipt(std::string const& transactionHash);

	std::string eth_ipc_request(std::string const& jsonRequest);
	std::string eth_call(std::string const& abiData);
	std::string eth_sendTransaction(std::string const& abiData);
	std::string eth_createContract(std::string const& data);
	std::string eth_getTransactionReceipt(std::string const& transactionHash);
};


} //namespace


#endif //__ETH_INTERFACE_HPP
