#include <boost/algorithm/string/trim.hpp>
#include <iostream>

#include <ethabi.hpp>
#include <event_wait_mgr.hpp>
#include <nlohmann/json.hpp>


using namespace std;
using Json = nlohmann::json;

namespace eth_interface
{



EventLogWaitManager::EventLogWaitManager(
	string const &clientAddress,
	string const &contractAddress,
	string const &ipcPath,
	vector<tuple<string, string, bool>> const &contractLogSignatures,
	string const &contractABI)
{
	if (contractLogSignatures.size() < 1)
	{
		throw InvalidArgumentException(
			"contractLogSignatures has no log signatures to subscribe to");
	}

	this->clientAddress = clientAddress;
	this->contractAddress = contractAddress;
	this->ipcPath = ipcPath;
	this->contractLogSignatures = contractLogSignatures;
	this->contractABI = contractABI;
	subscriptionListener = new thread(&EventLogWaitManager::ipc_subscription_listener_thread, this);
}



void
EventLogWaitManager::joinThreads(void)
{
	if (subscriptionListener != NULL)
	{
		subscriptionListener->join();
	}
}



unique_ptr<unordered_map<string, string>>
EventLogWaitManager::getEventLog(string const &logID)
{
	unordered_map<string, string> *element;

#ifdef _DEBUG
	cout << "EventLogWaitManager::getEventLog(): Acquiring mutex..." << endl
		 << endl;
#endif //_DEBUG

	mtx.lock();

#ifdef _DEBUG
	cout << "EventLogWaitManager::getEventLog(): Mutex acquired..." << endl
		 << endl;
#endif //_DEBUG

	if (eventLogMap.count(logID) == 0)
	{
		eventLogMap.emplace(logID, make_unique<EventLogWaitElement>());
	}

	if (eventLogMap[logID]->hasWaitingThread)
	{
		mtx.unlock();
		throw ResourceRequestFailedException(
			"Cannot request log which already has a thread waiting on it.");
	}
	eventLogMap[logID].get()->hasWaitingThread = true;

	if (!eventLogMap[logID].get()->hasEventLog)
	{

#ifdef _DEBUG
		cout << "EventLogWaitManager::getEventLog(): Releasing mutex and waiting on condition variable \""
			 << logID << "\"..." << endl
			 << endl;
#endif //_DEBUG

		mtx.unlock();
		while (!eventLogMap[logID].get()->hasEventLog)
		{
			eventLogMap[logID].get()->cv.wait(eventLogMap[logID].get()->cvLock);
		}
		mtx.lock();
	}

	element = new unordered_map<string, string>(*eventLogMap[logID].get()->eventLog.get());
	eventLogMap.erase(logID);
	mtx.unlock();

#ifdef _DEBUG
	cout << "EventLogWaitManager::getEventLog(): Mutex released and returning..." << endl
		 << endl;
#endif //_DEBUG

	return unique_ptr<unordered_map<string, string>>(element);
}



void
EventLogWaitManager::setEventLog(string const &logID, unordered_map<string, string> const &eventLog)
{
	mtx.lock();

	if (eventLogMap.count(logID) == 0)
	{

#ifdef _DEBUG
		cout << "EventLogWaitManager::setEventLog(): Create EventLogWaitElement \""
			 << logID << "\"..." << endl
			 << endl;
#endif //_DEBUG

		eventLogMap.emplace(logID, make_unique<EventLogWaitElement>());
	}
#ifdef _DEBUG
	else
	{
		cout << "EventLogWaitManager::setEventLog(): EventLogWaitElement \""
			 << logID << "\" already exists..." << endl
			 << endl;
	}
#endif //_DEBUG

	eventLogMap[logID].get()->eventLog = unique_ptr<unordered_map<string, string>>(new unordered_map<string, string>(eventLog));
	eventLogMap[logID].get()->hasEventLog = true;

	if (eventLogMap[logID].get()->hasWaitingThread)
	{

#ifdef _DEBUG
		cout << "EventLogWaitManager::setEventLog(): Notfiy condition variable \""
			 << logID << "\"..." << endl
			 << endl;
#endif //_DEBUG

		eventLogMap[logID].get()->cv.notify_all();
	}

	mtx.unlock();
}



void
EventLogWaitManager::ipc_subscription_listener_setup(
	boost::asio::local::stream_protocol::socket &socket,
	boost::asio::local::stream_protocol::endpoint &ep)
{
	char receiveBuffer[IPC_BUFFER_LENGTH];
	string subscribeParse, data, message;
	Json jsonResponce;
	int receiveLength;
	uint16_t i;

	subscriptionToEventName.clear();

restart: // TODO: Get rid of this

	try
	{
		socket.connect(ep);
	}
	catch (...)
	{
		throw ResourceRequestFailedException(
			"Failed to open Unix Domain Socket with Geth Ethereum client via "
			"\""
			+ ipcPath + "\"");
	}

	for (i = 0; i < contractLogSignatures.size(); i++)
	{
		data = "{\"id\":1,"
			   "\"method\":\"eth_subscribe\","
			   "\"params\":["
			   "\"logs\",{"
			   "\"address\":\"0x"
			+ contractAddress + "\","
								"\"topics\":[\""
			+ "0x";
		data += get<1>(contractLogSignatures[i]);
		data += "\"";
		if (get<2>(contractLogSignatures[i]))
		{
			data += ",\"0x000000000000000000000000" + clientAddress + "\"";
		}
		data += "]}]}";

		socket.send(boost::asio::buffer(data.c_str(), data.length()));

	subscribeReceive: // TODO: Get rid of this

		while (subscribeParse.find_first_of('\n', 0) == string::npos)
		{
			receiveLength = socket.receive(boost::asio::buffer(receiveBuffer, IPC_BUFFER_LENGTH - 1));

			if (receiveLength == 0)
			{
				// Socket was closed by other end
				goto restart;
			}
			receiveBuffer[receiveLength] = 0;
			subscribeParse += receiveBuffer;
		}

	subParse: // TODO: Get rid of this

		message = subscribeParse.substr(0, subscribeParse.find_first_of('\n', 0));
		subscribeParse = subscribeParse.substr(subscribeParse.find_first_of('\n', 0) + 1);

		try
		{
			jsonResponce = Json::parse(message);
		}
		catch (const Json::exception &e)
		{
			cerr << "ipc_subscription_listener_setup(): JSON response error in response while subscribing:"
				 << endl
				 << "\t"
				 << message
				 << endl
				 << e.what()
				 << endl;
			goto restart; // TODO: Remove this
		}

		if (jsonResponce.count("error") > 0)
		{
			throw ResourceRequestFailedException(
				"ipc_subscription_listener_setup(): Got an error response to eth_subscribe!\n"
				"Signature: "
				+ get<1>(contractLogSignatures[i]) + "\n"
													 "Request: "
				+ data + "\n"
						 "Responce: "
				+ message + "\n");
		}

		if (jsonResponce.contains("method") > 0)
		{
			receiveParse += message + "\n";
			goto subscribeReceive;
		}

		if (jsonResponce.count("result") == 0 || !jsonResponce["result"].is_string())
		{
			throw ResourceRequestFailedException(
				"ipc_subscription_listener_setup(): Unexpected response to eth_subscribe received!");
		}
		string result = jsonResponce["result"];

		subscriptionToEventName[result] = contractLogSignatures[i];
	}

	boost::trim(subscribeParse);
	if (subscribeParse.length() > 0)
		goto subParse;
}



void
EventLogWaitManager::ipc_subscription_listener_thread(void)
{
	boost::asio::io_service io_service;
	boost::asio::local::stream_protocol::endpoint ep(ipcPath);
	boost::asio::local::stream_protocol::socket socket(io_service);
	char receiveBuffer[IPC_BUFFER_LENGTH];
	int receiveLength;
	Json jsonData, resultJsonObject;
	string data, message, method, subscription, transactionHash;
	vector<string> topics;

#ifdef _DEBUG
	cout << "ipc_subscription_listener_thread()" << endl;
#endif //_DEBUG

begin:
	ipc_subscription_listener_setup(socket, ep);

	for (;;)
	{
		while (receiveParse.find_first_of('\n', 0) == string::npos)
		{
			try
			{
				receiveLength = socket.receive(boost::asio::buffer(receiveBuffer, IPC_BUFFER_LENGTH - 1));
			}
			catch (...)
			{
				socket.close();
				goto begin;
			}

			if (receiveLength == 0)
			{
				// Socket was closed by other end
				socket.close();
				goto begin;
			}
			receiveBuffer[receiveLength] = 0;
			receiveParse += receiveBuffer;
		}

		message = receiveParse.substr(0, receiveParse.find_first_of('\n', 0));
		receiveParse = receiveParse.substr(receiveParse.find_first_of('\n', 0) + 1);

		try
		{
			jsonData = Json::parse(message);

			method = jsonData["method"];
			subscription = jsonData["params"]["subscription"];
			resultJsonObject = jsonData["params"]["result"];

			topics.clear();

			for (Json::iterator iter = resultJsonObject["topics"].begin(); iter != resultJsonObject["topics"].end(); ++iter)
			{
				string iterStr = *iter;
				iterStr = iterStr.substr(2);
				topics.push_back(iterStr);
			}

			data = resultJsonObject["data"];
			transactionHash = resultJsonObject["transactionHash"];
		}
		catch (const Json::exception &e)
		{
			cerr << "ipc_subscription_listener_thread(): JSON response error in response:"
				 << endl
				 << "\t"
				 << message
				 << endl
				 << e.what()
				 << endl;
			continue;
		}

		if (method.compare("eth_subscription") != 0)
		{
			// "method" field of the JSON data is not "eth_subscription"
			cerr << "ipc_subscription_listener_thread(): \"method\" field of JSON message is \"eth_subscription\"!"
				 << endl
				 << "\t"
				 << message
				 << endl
				 << endl;
			continue;
		}

		if (subscriptionToEventName.count(subscription) <= 0)
		{
			//TODO: The subscription does not exist! Something is out of sync! -- Unsubscribe and start again?
			cerr << "ipc_subscription_listener_thread(): Received a subscription hash that does not exist internally!"
				 << endl
				 << "\t"
				 << message
				 << endl
				 << endl;

			socket.close();
			ipc_subscription_listener_setup(socket, ep);
		}

#ifdef _DEBUG
		cout << "ipc_subscription_listener_thread():"
			 << endl
			 << "\tcalling ethabi_decode_log(\""
			 << contractABI
			 << "\", \""
			 << get<0>(subscriptionToEventName[subscription])
			 << "\", [";
		uint16_t i = 0;
		for (auto &x : topics)
		{
			if (i > 0)
			{
				cout << ", ";
			}
			cout << x;
		}
		cout << "], \""
			 << data.substr(2)
			 << "\")"
			 << endl;
#endif //_DEBUG

		unordered_map<string, string> log;

		if (topics.size() > 1 || data.length() > 2)
		{
			// log = ethabi_decode_log(contractABI, get<0>(subscriptionToEventName[subscription]), topics, data.substr(2));
			log["transactionHash"] = transactionHash;
			log["sender"] = topics[1].substr(topics[1].length() - 40);
		}
		else
		{
			log["transactionHash"] = transactionHash;
		}


		log["EventName"] = get<0>(subscriptionToEventName[subscription]);

		if (get<2>(subscriptionToEventName[subscription]))
		{

#ifdef _DEBUG
			cout << "ipc_subscription_listener_thread(): setEventLog() by transactionHash \""
				 << transactionHash
				 << "\""
				 << endl;
#endif
			setEventLog(transactionHash, log);
		}
		else
		{

#ifdef _DEBUG
			cout << "ipc_subscription_listener_thread(): setEventLog() by event name \""
				 << get<0>(subscriptionToEventName[subscription])
				 << "\"" << endl;
#endif

			setEventLog(get<0>(subscriptionToEventName[subscription]), log);
		}

#ifdef _DEBUG
		mtx.lock();
		cout << "\t"
			 << "Event log received:"
			 << endl
			 << "[\"" << transactionHash
			 << "\" (\""
			 << get<0>(subscriptionToEventName[subscription])
			 << "\")] = ";
		if (topics.size() > 1 || data.length() > 2)
		{
			cout << eventLogMap[transactionHash].get()->toString();
		}
		else
		{
			cout << eventLogMap[get<0>(subscriptionToEventName[subscription])].get()->toString();
		}
		cout << endl
			 << endl;
		mtx.unlock();
#endif //_DEBUG
	}
	socket.close();
}


} //namespace
