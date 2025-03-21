#pragma once
#include "Connection.h"
#include "boost/unordered_map.hpp"
class ConnectionManager{
public:
	static ConnectionManager& GetInstance();
	void AddConnection(std::shared_ptr<Connection> conptr);
	void RmvConnection(std::string );
private:
	ConnectionManager(const ConnectionManager&) = delete;
	ConnectionManager& operator = (const ConnectionManager&) = delete;
	ConnectionManager();
	boost::unordered_map<std::string, std::shared_ptr<Connection>>  _connections;
};

