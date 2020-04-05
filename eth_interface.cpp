#include <array>
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/lexical_cast.hpp>
#include <iostream>
#include <string>

#include <fcntl.h>
#include <string.h>
#include <unistd.h>

#include <eth_interface.hpp>
#include <ethabi.hpp>
#include <misc.hpp>



using namespace std;

namespace eth_interface
{



EthInterface::EthInterface(
	string ethContractSOL,
	string ethContractABI,
	string ethContractBIN)
{
	this->ethContractSOL = ethContractSOL;
	this->ethContractABI = ethContractABI;
	this->ethContractBIN = ethContractBIN;
}



void
EthInterface::setContractAddress(string contractAddress)
{
	this->contractAddress = boost::to_lower_copy(boost::trim_copy(contractAddress));

	if (this->contractAddress.substr(0, 2) == "0x")
	{
		this->contractAddress = this->contractAddress.substr(2);
	}
}



void
EthInterface::initialize(
	string ipcPath,
	string clientAddress,
	vector<tuple<string, string, bool>> contractEventSignatures)
{
	this->ipcPath = boost::trim_copy(ipcPath);

	this->clientAddress = boost::to_lower_copy(boost::trim_copy(clientAddress));
	if (this->clientAddress.substr(0, 2) == "0x")
	{
		this->clientAddress = this->clientAddress.substr(2);
	}

	this->contractEventSignatures = contractEventSignatures;
}

void
EthInterface::createEventLogWaitManager(void)
{
	if (eventLogWaitManager != nullptr)
	{
		delete eventLogWaitManager;
	}
	eventLogWaitManager = new EventLogWaitManager(
		this->clientAddress,
		this->contractAddress,
		this->ipcPath,
		this->contractEventSignatures,
		this->ethContractABI);
}



void
EthInterface::closeEventLogWaitManager(void)
{
	if (eventLogWaitManager != nullptr)
	{
		delete eventLogWaitManager;
	}
	eventLogWaitManager = nullptr;
}



string
EthInterface::getIPCPath(void)
{
	return ipcPath;
}



string
EthInterface::getClientAddress(void)
{
	return clientAddress;
}



string
EthInterface::getContractAddress(void)
{
	return contractAddress;
}



string
EthInterface::getEthContractSOL(void)
{
	return this->ethContractSOL;
}



string
EthInterface::getEthContractABI(void)
{
	return this->ethContractABI;
}



string
EthInterface::getEthContractBIN(void)
{
	return this->ethContractBIN;
}



// Throws ResourceRequestFailedException from ethabi()
string
EthInterface::getFrom(string const& funcName, string const& ethabiEncodeArgs)
{
	string data, result;

	data = ethabi(
		"encode -l function " + this->ethContractABI + " " + funcName + " " + ethabiEncodeArgs);

#ifdef _DEBUG
	cout << "getFrom(): " << funcName << "  " << ethabiEncodeArgs << endl;
#endif //_DEBUG

	Json callJson = call_helper(data);
	result = callJson["result"];

	return result.substr(2);
}



// Throws ResourceRequestFailedException from ethabi()
string
EthInterface::getNoArgs(string const& funcName)
{
	return getFrom(funcName, "");
}



// Throws ResourceRequestFailedException from ethabi()
uint64_t
EthInterface::getInt(string const& funcName)
{
	string value;

	value = getNoArgs(funcName);
	return stoul(value, nullptr, 16);
}



// Throws ResourceRequestFailedException from ethabi()
uint64_t
EthInterface::getIntFromContract(string const& funcName)
{
	return getIntFromContract(funcName, "");
}



// Throws ResourceRequestFailedException from ethabi()
uint64_t
EthInterface::getIntFromContract(string const& funcName, string const& params)
{
	string hex;
	uint64_t dec;

	hex = getFrom(funcName, params);
	dec = strtoull(hex.c_str(), nullptr, 16);

	return dec;
}



Json
EthInterface::call_helper(string const& data)
{
	string callStr = eth_call(data);
	Json callJson = Json::parse(callStr);
	string callResult = callJson["result"];
	if (callResult.compare("0x") == 0)
	{
		// The contract failed to execute (a require statement failed)
		throw CallFailedException("eth_call did not execute successfully!");
	}
	return callJson;
}



void
EthInterface::blockForEvent(string const& event)
{
	eventLogWaitManager->getEventLog(event);
	return;
}



unique_ptr<unordered_map<string, string>>
EthInterface::contract_helper(string const& data)
{
	string transactionHash, transactionReceipt;
	Json transactionJsonData;

#ifdef _DEBUG
	cout << "contract_helper(): Before call_helper()" << endl;
#endif //_DEBUG

	// Make an eth_call with the parameters first to check the contract will not fail
	call_helper(data);

	transactionHash = eth_sendTransaction(data);
	transactionJsonData = Json::parse(transactionHash);
	auto findResult = transactionJsonData.find("result");

	if (findResult == transactionJsonData.end())
	{
		throw TransactionFailedException(
			"Transaction hash was not present in responce to eth_sendTransaction!");
	}

	transactionHash = findResult.value();
	transactionReceipt = getTransactionReceipt(transactionHash);
#ifdef _DEBUG
	cout << "contract_helper(): Calling eventLogWaitManager->getEventLog()..."
		 << endl
		 << endl;
#endif //_DEBUG
	return eventLogWaitManager->getEventLog(transactionHash);
}



// Throws ResourceRequestFailedException from ethabi()
// Throws TransactionFailedException from eth_sendTransaction()
bool
EthInterface::callMutatorContract(
	string const& funcName,
	string const& ethabiEncodeArgs,
	unique_ptr<unordered_map<string, string>>& eventLog)
{
	string data;

	data = ethabi(
		"encode -l function " + this->ethContractABI + " " + funcName + " " + ethabiEncodeArgs);

	try
	{
		eventLog = contract_helper(data);
	}
	catch (const CallFailedException& e)
	{
		return false;
	}
	catch (const TransactionFailedException& e)
	{
		return false;
	}

#ifdef _DEBUG
	string logStr = "{ ";
	for (std::pair<std::string, std::string> kv : *eventLog.get())
	{
		logStr += "\"" + kv.first + "\":\"" + kv.second + "\", ";
	}

	logStr = logStr.substr(0, logStr.length() - 2);
	logStr += " }";
	cout << funcName
		 << "() successful!"
		 << endl
		 << logStr
		 << endl;
#endif //_DEBUG

	return true;
}



string
EthInterface::eth_sign(string const& data)
{
	string sig;
	string jsonResponceStr;
	string jsonRequest = "{\"jsonrpc\":\"2.0\","
						 "\"method\":\"eth_sign\","
						 "\"params\":[\"0x" +
						 clientAddress +
						 "\",\"0x" +
						 data +
						 "\"],\"id\":1}";

	jsonResponceStr = this->eth_ipc_request(jsonRequest);



	Json jsonResponce = Json::parse(jsonResponceStr);
	auto result = jsonResponce.find("result");

#ifdef _DEBUG
	cout << "eth_sign: jsonResponce = \""
		 << jsonResponceStr
		 << "\""
		 << endl;
#endif //_DEBUG

	if (result == jsonResponce.end())
	{
		throw TransactionFailedException(
			"eth_sign(): \"result\" was not "
			"present in responce to eth_sign!");
	}

	sig = result.value();

#ifdef _DEBUG
	cout << "eth_sign(\""
		 << data
		 << "\") = \""
		 << sig
		 << "\""
		 << endl;
#endif //_DEBUG

	return sig;
}



vector<string>
EthInterface::eth_accounts(void)
{
	Json jsonResponce;
	string jsonRequest = "{\"jsonrpc\":\"2.0\","
						 "\"method\":\"eth_accounts\","
						 "\"params\":[],\"id\":1}";

#ifdef _DEBUG
	cout << "eth_accounts()"
		 << endl
		 << "jsonRequest: "
		 << jsonRequest
		 << endl;
#endif //_DEBUG

	string jsonStr;

	try
	{
		jsonStr = this->eth_ipc_request(jsonRequest);
	}
	catch(ResourceRequestFailedException const& e)
	{
		cerr << "Geth IPC file does not exist!"
			 << endl
			 << "Ensure Geth is running and \"ipcPath\" in game.conf is correct!"
			 << endl;
		exit(EXIT_FAILURE);
	}

	jsonResponce = Json::parse(jsonStr);
	auto result = jsonResponce.find("result");

	if (result == jsonResponce.end())
	{
		throw TransactionFailedException(
			"eth_accounts(): \"result\" was not "
			"present in responce to eth_accounts!");
	}

	vector<string> vec;
	for (auto& item : result.value().items())
	{
		vec.push_back(item.value());
	}

	return vec;
}



// Throws TransactionFailedException from eth_sendTransaction() and locally
// Throws ResourceRequestFailedException
string
EthInterface::create_contract(string const& solFile, string const& abiFile, string const& binFile)
{
	return create_contract(solFile, abiFile, binFile, "");
}



void
EthInterface::compile_solidity(string const& solFile, string const& abiFile, string const& binFile)
{
	string shellCall;

	shellCall = "solc --bin '";
	shellCall += solFile;
	shellCall += "' | tail -n +4 > '";
	shellCall += binFile;
	shellCall += "'";

#ifdef _DEBUG
	cout << "compile_solidity(): system(\"" << shellCall << "\")" << endl;
#endif //_DEBUG

	if (system(shellCall.c_str()) != 0)
	{
		throw ResourceRequestFailedException(
			"solc failed to compile contract to binary format!");
	}

	shellCall = "solc --abi '";
	shellCall += solFile;
	shellCall += "' | tail -n +4 > '";
	shellCall += abiFile;
	shellCall += "'";

#ifdef _DEBUG
	cout << "compile_solidity(): system(\"" << shellCall << "\")" << endl;
#endif //_DEBUG

	if (system(shellCall.c_str()) != 0)
	{
		throw ResourceRequestFailedException(
			"solc failed to compile contract to abi format!");
	}
}



// Throws TransactionFailedException from eth_sendTransaction() and locally
// Throws ResourceRequestFailedException
string
EthInterface::create_contract(string const& solFile, string const& abiFile, string const& binFile, string const& params)
{
	string contractBin,
		transactionJsonStr,
		transactionHash,
		transactionReceipt,
		contractAddress;
	Json transactionJsonData, receiptJsonData;

	compile_solidity(solFile, abiFile, binFile);

	contractBin = boost::trim_copy(readFile2(binFile));
	contractBin = contractBin.substr(0, contractBin.find_first_of('\n', 0)) + params;

	transactionJsonStr = this->eth_createContract(contractBin);

	transactionJsonData = Json::parse(transactionJsonStr);
	auto jsonFindResult = transactionJsonData.find("result");

	if (jsonFindResult == transactionJsonData.end())
	{

#ifdef _DEBUG
		cout << "create_contract(): Transaction responce: \"" << transactionJsonStr << "\"" << endl;
#endif //_DEBUG

		// "result" not in JSON responce
		throw TransactionFailedException(
			"create_contract(): Transaction hash was not "
			"present in responce to eth_sendTransaction!");
	}

	transactionHash = jsonFindResult.value();

	cout << "Parsed contract creation transaction hash: "
		 << transactionHash << endl;

	transactionReceipt = this->getTransactionReceipt(transactionHash);

	receiptJsonData = Json::parse(transactionReceipt);
	jsonFindResult = receiptJsonData.find("result");

	if (!jsonFindResult.value().is_object())
	{
		throw TransactionFailedException(
			"create_contract(): \"result\" was not a JSON object!");
	}
	auto subJsonFindResult = jsonFindResult.value().find("contractAddress");

	if (subJsonFindResult == jsonFindResult.value().end())
	{
		// "contractAddress" not in JSON responce
		// TODO: What if "contractAddress" is not a string
		throw TransactionFailedException(
			"create_contract(): \"contractAddress\" was not present in responce!");
	}

	contractAddress = subJsonFindResult.value();

	if (contractAddress.compare("null") == 0)
	{
		throw TransactionFailedException(
			"create_contract(): \"contractAddress\" was null!");
	}

	cout << "Contract Address: " << contractAddress << endl;

	this->contractAddress = boost::to_lower_copy(boost::trim_copy(contractAddress));
	if (this->contractAddress.substr(0, 2) == "0x")
	{
		this->contractAddress = this->contractAddress.substr(2);
	}
	return contractAddress;
}



string
EthInterface::getTransactionReceipt(string const& transactionHash)
{
	int retries = 0;
	string transactionReceipt, result;
	Json jsonData;

	while (retries <= ETH_GET_TRANSRECEIPT_MAX_RETRIES)
	{
		transactionReceipt = this->eth_getTransactionReceipt(transactionHash);
		retries++;

#ifdef _DEBUG
		cout << "Try #" << retries << endl;
		cout << transactionReceipt << endl
			 << endl;
#endif //_DEBUG
		try
		{
			jsonData = Json::parse(transactionReceipt);
		}
		catch(nlohmann::detail::parse_error const& e)
		{
			continue;
		}
		auto jsonFindResult = jsonData.find("result");
		if (jsonFindResult == jsonData.end())
		{
			sleep(ETH_GET_TRANSRECEIPT_RETRY_DELAY);
			continue;
		}
		else if (jsonFindResult.value().is_null())
		{
			sleep(ETH_GET_TRANSRECEIPT_RETRY_DELAY);
			continue;
		}

#ifdef _DEBUG
		cout << "getTransactionReceipt(): returning..." << endl
			 << endl;
#endif //_DEBUG

		return transactionReceipt;
	}

	throw TransactionFailedException(
		"Failed to obtain transaction result in getTransactionReceipt() "
		"for transaction hash \""
		+ transactionHash + "\"; "
							"transaction may or may not have been mined!");
}



// TODO: Should this be ported to Unix Domain Sockets?
// TODO: At the very least, replace cerr/return with exceptions
string
EthInterface::eth_ipc_request(string const& jsonRequest)
{
	int ipcFdFlags, ipcFd;
	string json;
	array<char, IPC_BUFFER_LENGTH> ipcBuffer;

#ifdef _DEBUG
	cout << "eth_ipc_request(): " << jsonRequest << endl;
#endif //_DEBUG

	FILE* ipc = popen(
		("echo '" + jsonRequest + "' | nc -U '" + ipcPath + "'").c_str(),
		"r");
	if (ipc == NULL)
	{
		// Failed to open Unix domain socket for IPC -- Perhaps geth is not running?
		throw ResourceRequestFailedException(
			"eth_ipc_request(): Failed to popen() unix domain "
			"socket for IPC with geth! Is geth running?");
	}

	ipcFd = fileno(ipc);

	if (fgets(ipcBuffer.data(), IPC_BUFFER_LENGTH, ipc) == NULL)
	{
		throw ResourceRequestFailedException(
			"eth_ipc_request(): Error: Failed to read from IPC!");
	}

	json += ipcBuffer.data();

	ipcFdFlags = fcntl(ipcFd, F_GETFL, 0);
	ipcFdFlags |= O_NONBLOCK;
	fcntl(ipcFd, F_SETFL, ipcFdFlags);

	while (json.find_first_of('\n', 0) == string::npos)
	{
		fgets(ipcBuffer.data(), IPC_BUFFER_LENGTH, ipc);
		json += ipcBuffer.data();
	}

#ifdef _DEBUG
	cout << "eth_ipc_request(): Responce: \""
			<< json << "\"" << endl;
#endif //_DEBUG

	if (pclose(ipc) < 0)
	{
		throw ResourceRequestFailedException(
			"eth_ipc_request(): Failed to pclose() unix domain socket for IPC with geth!");
	}

#ifdef _DEBUG
	cout << "eth_ipc_request(): Successfully relayed request" << endl;
#endif //_DEBUG

	return json;
}



string
EthInterface::eth_call(string const& abiData)
{
	string jsonRequest = "{\"jsonrpc\":\"2.0\","
						 "\"method\":\"eth_call\","
						 "\"params\":[{"
						 "\"from\":\"0x" + clientAddress + "\","
						 "\"to\":\"0x"
		+ contractAddress + "\","
							"\"data\":\"0x"
		+ abiData + "\"},\"latest\"],\"id\":1}";

#ifdef _DEBUG
	cout << "eth_call()" << endl;
#endif //_DEBUG

	return this->eth_ipc_request(jsonRequest);
}



string
EthInterface::eth_sendTransaction(string const& abiData)
{
	string jsonRequest = "{\"jsonrpc\":\"2.0\","
						 "\"method\":\"eth_sendTransaction\""
						 ",\"params\":[{"
						 "\"from\":\"0x"
		+ clientAddress + "\","
						  "\"to\":\"0x"
		+ contractAddress + "\","
							"\"gas\":\"" + ETH_DEFAULT_GAS + "\","
							"\"gasPrice\":\"0x0\","
							"\"data\":\"0x"
		+ abiData + "\"}],"
					"\"id\":1}";

#ifdef _DEBUG
	cout << "eth_sendTransaction()" << endl;
#endif //_DEBUG

	return this->eth_ipc_request(jsonRequest);
}



string
EthInterface::eth_createContract(string const& data)
{
	string jsonRequest = "{\"jsonrpc\":\"2.0\","
						 "\"method\":\"eth_sendTransaction\""
						 ",\"params\":[{"
						 "\"from\":\"0x"
						 + clientAddress + "\","
						 "\"gasPrice\":\"0x0\","
						 "\"gas\":\"" + ETH_DEFAULT_GAS + "\","
						 "\"data\":\"0x"
		+ data + "\"}],"
				 "\"id\":1}";

#ifdef _DEBUG
	cout << "eth_createContract()" << endl;
#endif //_DEBUG

	return this->eth_ipc_request(jsonRequest);
}



string
EthInterface::eth_getTransactionReceipt(string const& transactionHash)
{
	string jsonRequest = "{\"jsonrpc\":\"2.0\","
						 "\"method\":\"eth_getTransactionReceipt\","
						 "\"params\":["
						 "\""
		+ transactionHash + "\""
							"],\"id\":1}";

#ifdef _DEBUG
	cout << "eth_getTransactionReceipt()" << endl;
#endif //_DEBUG

	return this->eth_ipc_request(jsonRequest);
}



void
EthInterface::joinThreads(void)
{
	eventLogWaitManager->joinThreads();
}



} //namespace
