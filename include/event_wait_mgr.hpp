#ifndef __EVENT_WAIT_MGR_HPP
#define __EVENT_WAIT_MGR_HPP

#include <boost/asio.hpp>
#include <condition_variable>
#include <unordered_map>
#include <mutex>
#include <string>
#include <thread>



namespace eth_interface
{


class EventLogWaitManager
{
	public:
	EventLogWaitManager(
		std::string const& clientAddress,
		std::string const& contractAddress,
		std::string const& ipcPath,
		std::vector<std::tuple<std::string, std::string, bool>> const& contractLogSignatures,
		std::string const& contractABI);

	std::unique_ptr<std::unordered_map<std::string, std::string>> getEventLog(std::string const& logID);
	void setEventLog(std::string const& logID, std::unordered_map<std::string, std::string> const& eventLog);
	void joinThreads(void);

	// Thread main function for geth log monitor thread
	void ipc_subscription_listener_thread(void);

	private:
	std::thread* subscriptionListener = NULL;

	std::vector<std::tuple<std::string, std::string, bool>> contractLogSignatures;
	std::map<std::string, std::tuple<std::string, std::string, bool>> subscriptionToEventName;
	std::string receiveParse;

	std::string contractAddress;
	std::string clientAddress;
	std::string ipcPath;
	std::string contractABI;

	struct EventLogWaitElement
	{
		std::mutex cvLockMtx;
		std::unique_lock<std::mutex> cvLock;
		std::condition_variable cv;
		bool hasWaitingThread = false;
		bool hasEventLog = false;
		std::unique_ptr<std::unordered_map<std::string, std::string>> eventLog;

		EventLogWaitElement()
		{
			cvLock = std::unique_lock<std::mutex>(cvLockMtx);
		}

		std::string
		toString(void)
		{
			std::string str = "{ ";
			for (std::pair<std::string, std::string> kv : *eventLog.get())
			{
				str += "\"" + kv.first + "\":\"" + kv.second + "\", ";
			}
			str = str.substr(0, str.length() - 2);
			str += " }";
			return str;
		}
	};

	std::mutex mtx;
	std::unordered_map<std::string, std::unique_ptr<EventLogWaitElement>> eventLogMap;

	void ipc_subscription_listener_setup(
		boost::asio::local::stream_protocol::socket& socket,
		boost::asio::local::stream_protocol::endpoint& ep);
};



} //namespace


#endif //__EVENT_WAIT_MGR_HPP