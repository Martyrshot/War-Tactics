#ifndef __ETH_INTERFACE_EXCEPT_HPP
#define __ETH_INTERFACE_EXCEPT_HPP

#include <stdexcept>
#include <string>


namespace eth_interface
{


class EthException : public std::runtime_error
{
	public:
	EthException(const char* msg)
		: std::runtime_error(msg)
	{
	}
	EthException(const std::string& msg)
		: std::runtime_error(msg)
	{
	}
};



class TransactionException : public EthException
{
	public:
	TransactionException(const char* msg)
		: EthException(msg)
	{
	}
	TransactionException(const std::string& msg)
		: EthException(msg)
	{
	}
};

class CallFailedException : public TransactionException
{
	public:
	CallFailedException(const char* msg)
		: TransactionException(msg)
	{
	}
	CallFailedException(const std::string& msg)
		: TransactionException(msg)
	{
	}
};

class TransactionFailedException : public TransactionException
{
	public:
	TransactionFailedException(const char* msg)
		: TransactionException(msg)
	{
	}
	TransactionFailedException(const std::string& msg)
		: TransactionException(msg)
	{
	}
};



class InvalidArgumentException : public EthException
{
	public:
	InvalidArgumentException(const char* msg)
		: EthException(msg)
	{
	}
	InvalidArgumentException(const std::string& msg)
		: EthException(msg)
	{
	}
};



class DeviceNotAssignedException : public EthException
{
	public:
	DeviceNotAssignedException(const char* msg)
		: EthException(msg)
	{
	}
	DeviceNotAssignedException(const std::string& msg)
		: EthException(msg)
	{
	}
};

class ResourceRequestFailedException : public EthException
{
	public:
	ResourceRequestFailedException(const char* msg)
		: EthException(msg)
	{
	}
	ResourceRequestFailedException(const std::string& msg)
		: EthException(msg)
	{
	}
};

class ThreadExistsException : public EthException
{
	public:
	ThreadExistsException(const char* msg)
		: EthException(msg)
	{
	}
	ThreadExistsException(const std::string& msg)
		: EthException(msg)
	{
	}
};



class CryptographicException : public EthException
{
	public:
	CryptographicException(const char* msg)
		: EthException(msg)
	{
	}
	CryptographicException(const std::string& msg)
		: EthException(msg)
	{
	}
};

class CryptographicFailureException : public CryptographicException
{
	public:
	CryptographicFailureException(const char* msg)
		: CryptographicException(msg)
	{
	}
	CryptographicFailureException(const std::string& msg)
		: CryptographicException(msg)
	{
	}
};

class CryptographicKeyMissmatchException : public CryptographicException
{
	public:
	CryptographicKeyMissmatchException(const char* msg)
		: CryptographicException(msg)
	{
	}
	CryptographicKeyMissmatchException(const std::string& msg)
		: CryptographicException(msg)
	{
	}
};


} //namespace


#endif //__ETH_INTERFACE_EXCEPT_HPP
