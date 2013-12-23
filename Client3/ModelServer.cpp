//#include "ModelServer.h"
//
//
//ModelServer::ModelServer(void)
//{
//}
//
//
//ModelServer::~ModelServer(void)
//{
//}

#include "ModelServer.h"
#include <time.h>


ModelServer::ModelServer(void)
{
	this->m_item_id = -1;
	this->m_server_IP = "0.0.0.0";
	this->m_server_name = "none";
	this->m_server_remark = "δ���";
	this->m_server_state = UNDETECTED;
	this->AllowMaintence = 0;
	this->maintence_end = this->maintence_start = 0;
}

ModelServer::ModelServer(const ModelServer& _ms )
{
	*this = _ms;
}


ModelServer::~ModelServer(void)
{
}



std::string ModelServer::parseState( SERVER_STATE server_state )
{
	switch (server_state)
	{
	case UNDETECTED:
		return "δ���";
		break;
	case ACCESSIBLE:
		return "״̬����";
		break;
	case TIMEOUT:
		return "��ʱ";
		break;
	case BADIP:
		return "IP��Ч!";
		break;
	case MAINTENCE:
		return "ϵͳά����";
		break;
	case UNRETURN:
		return "�޷���������";
		break;
	default:
		return "δ���壡";
	}

}

bool ModelServer::isMaintence()
{

	//������ά����Ȼ��û������ά����^_^
	if(!AllowMaintence)
		return false;

	int start = this->maintence_start;
	int end = this->maintence_end; 

	time_t t = time(0);
	char tmp_h[64];
	strftime( tmp_h, sizeof(tmp_h), "%H",localtime(&t) ); 
	int time_h = atoi(tmp_h);


	char tmp_m[64];
	strftime( tmp_m, sizeof(tmp_m), "%M",localtime(&t) ); 
	int time_m = atoi(tmp_m);

	int time = time_h * 60 + time_m;
	if(end > start)
	{
		if(time <= end && time >= start)
			return true;
		return false;
	}
	else if(end < start)
	{
		if(time <= start && time >= end )
			return false;
		return true;
	}
	return true;
}
