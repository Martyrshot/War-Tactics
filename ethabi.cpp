#include <array>
#include <vector>

#ifdef _DEBUG
#include <iostream>
#endif //_DEBUG

#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/trim.hpp>

#include <blockchainsec.hpp>


using namespace std;

namespace eth_interface
{



string
ethabi(string const& args)
{
	string result;
	array<char, IPC_BUFFER_LENGTH> pipe_buffer;

#ifdef _DEBUG
	cout << "ethabi(): Requested '" << args << "'" << endl;
#endif //_DEBUG

	FILE* ethabi_pipe = popen(("ethabi " + args).c_str(), "r");
	if (ethabi_pipe == NULL)
	{
		// Failed to open pipe to ethabi -- is the binary installed and in $PATH?
		throw ResourceRequestFailedException(
			"ethabi(): Failed to popen() pipe to ethabi binary. "
			"Is the binary installed and in the $PATH environment variable?");
	}
	while (fgets(pipe_buffer.data(), IPC_BUFFER_LENGTH, ethabi_pipe) != NULL)
	{
		result += pipe_buffer.data();
	}
	if (pclose(ethabi_pipe) != 0)
	{
		throw ResourceRequestFailedException(
			"ethabi(): ethabi binary exited with a failure status!\n"
			"Args: "
			+ args + "\n"
					 "Output: "
			+ result + "\n");
	}

#ifdef _DEBUG
	cout << "ethabi(): Call successful!" << endl;
	cout << "ethabi(): Returning: " << boost::trim_copy(result) << endl;
#endif //_DEBUG

	return boost::trim_copy(result);
	;
}



//TODO: Should I be using unique_ptr<unordered_map<...>> ?
unordered_map<string, string>
ethabi_decode_log(
	string const& abiFile,
	string const& eventName,
	vector<string>& topics,
	string const& data)
{
	string query, responce;
	vector<string> lines;
	unordered_map<string, string> parsedLog;
	string topic_query;

	for (vector<string>::iterator iter = topics.begin(); iter != topics.end(); ++iter)
	{
		topic_query += "-l " + *iter + " ";
	}

	query = "decode log " + abiFile + " " + eventName + " " + topic_query + data;
	responce = ethabi(query);

	boost::split(lines, responce, boost::is_any_of("\n"));

	for (vector<string>::iterator iter = lines.begin(); iter != lines.end(); ++iter)
	{
		if (boost::trim_copy(*iter).compare("") == 0)
		{
			continue;
		}
		//TODO: What if there is no space to sepatate key and value?
		string key = (*iter).substr(0, (*iter).find_first_of(" "));
		string value = (*iter).substr((*iter).find_first_of(" ") + 1);
		parsedLog[key] = value;
	}
	return parsedLog;
}



string
ethabi_decode_result(string const& abiFile, string const& eventName, string const& data)
{
	string query, responce;

	query = "decode function " + abiFile + " " + eventName + " " + data; //TODO: Check data does not have "0x"...
	responce = ethabi(query);

#ifdef _DEBUG
	cout << "ethabi_decode_result() responce:" << responce << endl;
#endif //_DEBUG

	if (responce.find_first_of(" ") == string::npos)
	{
		return "";
	}
	return responce.substr(responce.find_first_of(" ") + 1);
}



vector<string>
ethabi_decode_results(string const& abiFile, string const& eventName, string const& data)
{
	vector<string> array;
	string query, responce;

	query = "decode function " + abiFile + " " + eventName + " " + data; //TODO: Check data does not have "0x"...
	responce = ethabi(query);

#ifdef _DEBUG
	cout << "ethabi_decode_results() responce:" << responce << endl;
#endif //_DEBUG

	boost::split(array, responce, boost::is_any_of("\n"));

	for (vector<string>::iterator it = array.begin(); it != array.end(); ++it)
	{
		if (responce.find_first_of(" ") == string::npos)
		{
			*it = "";
		}
		else
		{
			*it = (*it).substr((*it).find_first_of(" ") + 1);
		}
	}
	return array;
}



vector<string>
ethabi_decode_string_array(string const& abiFile, string const& eventName, string const& data)
{
	vector<string> array;
	string responce;

	responce = boost::trim_copy(ethabi_decode_result(abiFile, eventName, data));
	responce = responce.substr(1, responce.length() - 2);
	boost::split(array, responce, boost::is_any_of(","));

	return array;
}



vector<uint32_t>
ethabi_decode_uint32_array(string const& abiFile, string const& eventName, string const& data)
{
	vector<string> arrayStr;
	vector<uint32_t> arrayUInt32;
	string responce;

	responce = boost::trim_copy(ethabi_decode_result(abiFile, eventName, data));
	responce = responce.substr(1, responce.length() - 2);
	boost::split(arrayStr, responce, boost::is_any_of(","));

	for (vector<string>::iterator it = arrayStr.begin(); it != arrayStr.end(); ++it)
	{
		arrayUInt32.push_back(strtoul(((*it).c_str()), nullptr, 16));
	}
	return arrayUInt32;
}


} //namespace