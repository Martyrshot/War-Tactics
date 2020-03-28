#ifndef __ETH_INTERFACE_HPP
#define __ETH_INTERFACE_HPP

#include <thread>
#include <mutex>

#include <nlohmann/json.hpp>
#include <eth_interface_except.hpp>
#include <event_wait_mgr.hpp>

#define ETH_GET_TRANSRECEIPT_MAX_RETRIES 25
#define ETH_GET_TRANSRECEIPT_RETRY_DELAY 1

#define IPC_BUFFER_LENGTH 128

#define ETH_DEFAULT_GAS "0xFFFFFF"


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
		std::vector<std::tuple<std::string, std::string, bool>> contractEventSignatures);

	void setContractAddress(std::string contractAddress);
	void blockForEvent(std::string const& event);

	std::string getIPCPath(void);
	std::string getClientAddress(void);
	std::string getContractAddress(void);
	std::string getEthContractSOL(void);
	std::string getEthContractABI(void);
	std::string getEthContractBIN(void);

	void createEventLogWaitManager(void);
	void closeEventLogWaitManager(void);

	void joinThreads(void);

	protected:
	std::vector<std::tuple<std::string, std::string, bool>> contractEventSignatures;
	std::string ipcPath;
	std::string clientAddress;
	std::string contractAddress;
	std::string ethContractSOL;
	std::string ethContractABI;
	std::string ethContractBIN;

	EventLogWaitManager* eventLogWaitManager = nullptr;

	std::string getFrom(std::string const& funcName, std::string const& ethabiEncodeArgs);
	std::string getNoArgs(std::string const& funcName);
	uint64_t getInt(std::string const& funcName);
	uint64_t getIntFromContract(std::string const& funcName);
	uint64_t getIntFromContract(std::string const& funcName, std::string const& params);
	std::string getArrayFromContract(std::string const& funcName);
	std::string getArrayFromContract(std::string const& funcName, std::string const& params);

	Json call_helper(std::string const& data);
	std::unique_ptr<std::unordered_map<std::string, std::string>> contract_helper(std::string const& data);
	bool callMutatorContract(
		std::string const& funcName,
		std::string const& ethabiEncodeArgs,
		std::unique_ptr<std::unordered_map<std::string, std::string>>& eventLog);

	std::string eth_sign(std::string const& data);
	std::vector<std::string> eth_accounts(void);

	std::string create_contract(std::string const& solFile,
		std::string const& abiFile,
		std::string const& binFile);

	std::string create_contract(std::string const& solFile,
		std::string const& abiFile,
		std::string const& binFile,
		std::string const& params);

	std::string getTransactionReceipt(std::string const& transactionHash);

	std::string eth_ipc_request(std::string const& jsonRequest);
	std::string eth_call(std::string const& abiData);
	std::string eth_sendTransaction(std::string const& abiData);
	std::string eth_createContract(std::string const& data);
	std::string eth_getTransactionReceipt(std::string const& transactionHash);
};


} //namespace


#endif //__ETH_INTERFACE_HPP
