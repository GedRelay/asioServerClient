#include "../include/ConnectionManager.h"

ConnectionManager::ConnectionManager(){

}

ConnectionManager& ConnectionManager::GetInstance(){
	static ConnectionManager instance;
	return instance;
}

void ConnectionManager::AddConnection(std::shared_ptr<Connection> connection){
	_connections[connection->GetUid()] = connection;
}

void ConnectionManager::RmvConnection(std::string id){
	_connections.erase(id);
}


