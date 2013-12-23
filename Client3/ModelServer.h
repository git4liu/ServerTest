//#pragma once
//#include <string>
//
//class ModelServer
//{
//public:
//	ModelServer(void);
//	~ModelServer(void);
//
//public:
//	std::string server_name;
//	std::string server_ip;
//};
//
#pragma once
#include <string>


enum SERVER_STATE{
	ACCESSIBLE = 10000,
	BADIP = 10001,
	TIMEOUT = 10002,
	UNDETECTED = 10003,
	MAINTENCE = 10004 ,
	UNRETURN = 10005
};

class ModelServer
{
public:
	static std::string parseState(SERVER_STATE server_state);

public:
	ModelServer(const ModelServer& _ms);
	ModelServer(void);
	~ModelServer(void);
	bool isMaintence();

public:
	std::string m_server_IP;
	std::string m_server_name;
	SERVER_STATE m_server_state;
	std::string m_server_remark;
	int m_item_id;
	int maintence_start;
	int maintence_end;
	int AllowMaintence;
};

