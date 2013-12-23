#define SERVICE

#include "stdio.h"
#include "tchar.h"
#include <iostream> 
#include <fstream>
#include <boost/asio.hpp> 
#include <string>
#include <Windows.h>
#include <shellapi.h>
#include <boost/thread/thread.hpp>
#include <vector>
#include "inifile/inifile.h"
#include <sstream>
#include <boost/date_time/posix_time/posix_time.hpp>

#include "ModelServer.h"
#include "ModelRuntimeConfig.h"

#define DEFAULT_PORT 13184

std::vector<ModelServer> servers;
ModelRuntimeConfig mrc;


bool isStart;

void printServers()
{
	std::vector<ModelServer>::iterator it = servers.begin();
	while(it != servers.end())
	{
		std::cout << "name:" <<  it->m_server_name << std::endl;
		std::cout <<"ip:" << it->m_server_IP << std::endl;
		std::cout<< "start:"<< it->maintence_start << std::endl;
		std::cout << "end:" << it->maintence_end << std::endl;
		std::cout << "allowMaintence:" << it->AllowMaintence << std::endl << std::endl;
		++ it;
	}
}
void printMrc()
{
	using namespace  std;
	cout << "timer:" <<  mrc.timer << endl ;
	cout << "fetion_id:" <<  mrc.fetion_id << endl ;
	cout << "fetion_pswd:" <<  mrc.fetion_pswd << endl ;
	cout << "fetion_recv:" <<  mrc.fetion_recv << endl ;
	cout << "fetion_shell:" <<  mrc.fetion_shell << endl ;
}
void printToFile()
{
	if(1)
		return;
	using namespace  std;

	char des[512];
	GetModuleFileName(NULL, des, 512);  
	//取出文件路径   
	for (int i=strlen(des); i>=0; i--)
	{   
		if (des[i] == '\\')
		{
			des[i] = '\0';
			break;   
		}
	}
	strcat(des , "\\配置文件读取.log");

	stringstream ss;
	vector<ModelServer>::iterator it = servers.begin();
	while(it != servers.end())
	{
		ss << "name:" <<  it->m_server_name << endl;
		ss <<"ip:" << it->m_server_IP << endl;
		ss<< "start:"<< it->maintence_start << endl;
		ss << "end:" << it->maintence_end << endl;
		ss << "allowMaintence:" << it->AllowMaintence << endl << endl;
		++ it;
	}


	ss << "timer:" <<  mrc.timer << endl ;
	ss << "fetion_id:" <<  mrc.fetion_id << endl ;
	ss << "fetion_pswd:" <<  mrc.fetion_pswd << endl ;
	ss << "fetion_recv:" <<  mrc.fetion_recv << endl ;
	ss << "fetion_shell:" <<  mrc.fetion_shell << endl ;


	string temp = ss.str();
	ofstream fout(des);
	fout.write(temp.c_str() , temp.length());
	
}
template <typename T>
std::string anyToString(T input)
{
	std::stringstream ss;
	ss << input;
	return ss.str();
}



void readConfig();

void Init();
BOOL IsInstalled();
BOOL Install();
BOOL Uninstall();
void writeToLog(char * str);
void WINAPI ServiceMain();
void WINAPI ServiceStrl(DWORD dwOpcode);

TCHAR szServiceName[] = _T("wbyServerTest_Client");
TCHAR szServiceDesc[] = _T("此服务用于伍佰亿服务器测试，客户端发送测试");
BOOL bInstall;
SERVICE_STATUS_HANDLE hServiceStatus;
SERVICE_STATUS status;
DWORD dwThreadID;


void ClientStart();


#ifdef SERVICE

int APIENTRY WinMain(HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPSTR     lpCmdLine,
	int       nCmdShow)
{
	Init();

	dwThreadID = ::GetCurrentThreadId();

	SERVICE_TABLE_ENTRY st[] =
	{
		{ szServiceName, (LPSERVICE_MAIN_FUNCTION)ServiceMain },
		{ NULL, NULL }
	};

	if (stricmp(lpCmdLine, "install") == 0)
	{
		Install();
	}
	else if (stricmp(lpCmdLine, "remove") == 0)
	{
		Uninstall();
	}
	else
	{
		if (!::StartServiceCtrlDispatcher(st))
		{
			writeToLog("serviceMain function error");
		}
	}

	return 0;
}

#else

int main()
{
	//readConfig();
	//printServers();
	//printMrc();

	ClientStart();
	getchar();
	return 0;
}

#endif
void Init()
{
	writeToLog("初始化");
	hServiceStatus = NULL;
	status.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
	status.dwCurrentState = SERVICE_STOPPED;
	status.dwControlsAccepted = SERVICE_ACCEPT_STOP;
	status.dwWin32ExitCode = 0;
	status.dwServiceSpecificExitCode = 0;
	status.dwCheckPoint = 0;
	status.dwWaitHint = 0;
}


void WINAPI ServiceMain()
{

	writeToLog("服务开启");
	// Register the control request handler
	status.dwCurrentState = SERVICE_START_PENDING;
	status.dwControlsAccepted = SERVICE_ACCEPT_STOP;

	//注册服务控制
	hServiceStatus = RegisterServiceCtrlHandler(szServiceName, ServiceStrl);
	if (hServiceStatus == NULL)
	{
		writeToLog("not install");
		return;
	}
	SetServiceStatus(hServiceStatus, &status);

	status.dwWin32ExitCode = S_OK;
	status.dwCheckPoint = 0;
	status.dwWaitHint = 0;
	status.dwCurrentState = SERVICE_RUNNING;
	SetServiceStatus(hServiceStatus, &status);

	//模拟服务的运行，10后自动退出。应用时将主要任务放于此即可
	ClientStart();
	//

	status.dwCurrentState = SERVICE_STOPPED;
	SetServiceStatus(hServiceStatus, &status);
	writeToLog("service stoped");
}


void WINAPI ServiceStrl(DWORD dwOpcode)
{
	switch (dwOpcode)
	{
	case SERVICE_CONTROL_STOP:
		isStart = 0;
		writeToLog("客户端停止");
		status.dwCurrentState = SERVICE_STOP_PENDING;
		SetServiceStatus(hServiceStatus, &status);
		PostThreadMessage(dwThreadID, WM_CLOSE, 0, 0);
		break;
	case SERVICE_CONTROL_PAUSE:
		break;
	case SERVICE_CONTROL_CONTINUE:
		break;
	case SERVICE_CONTROL_INTERROGATE:
		break;
	case SERVICE_CONTROL_SHUTDOWN:
		isStart = 0;
		writeToLog("客户端停止");
		status.dwCurrentState = SERVICE_STOP_PENDING;
		SetServiceStatus(hServiceStatus, &status);
		PostThreadMessage(dwThreadID, WM_CLOSE, 0, 0);
		break;
	default:
		writeToLog("bad request");
	}
}

BOOL IsInstalled()
{
	BOOL bResult = FALSE;

	//打开服务控制管理器
	SC_HANDLE hSCM = ::OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);

	if (hSCM != NULL)
	{
		//打开服务
		SC_HANDLE hService = ::OpenService(hSCM, szServiceName, SERVICE_QUERY_CONFIG);
		if (hService != NULL)
		{
			bResult = TRUE;
			::CloseServiceHandle(hService);
		}
		::CloseServiceHandle(hSCM);
	}
	return bResult;
}


BOOL Install()
{
	if (IsInstalled())
		return TRUE;

	//打开服务控制管理器
	SC_HANDLE hSCM = ::OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
	if (hSCM == NULL)
	{
		MessageBox(NULL, _T("Couldn't open service manager"), szServiceName, MB_OK);
		return FALSE;
	}

	// Get the executable file path
	TCHAR szFilePath[MAX_PATH];
	::GetModuleFileName(NULL, szFilePath, MAX_PATH);

	//创建服务
	SC_HANDLE hService = ::CreateService(
		hSCM, szServiceName, szServiceName,
		SERVICE_ALL_ACCESS, SERVICE_WIN32_OWN_PROCESS,
		SERVICE_DEMAND_START, SERVICE_ERROR_NORMAL,
		szFilePath, NULL, NULL, _T(""), NULL, NULL);

	SERVICE_DESCRIPTION sdBuf;
	sdBuf.lpDescription = szServiceDesc;
	ChangeServiceConfig2 (hService, SERVICE_CONFIG_DESCRIPTION, &sdBuf);

	if (hService == NULL)
	{
		::CloseServiceHandle(hSCM);
		MessageBox(NULL, _T("Couldn't create service"), szServiceName, MB_OK);
		return FALSE;
	}

	::CloseServiceHandle(hService);
	::CloseServiceHandle(hSCM);
	return TRUE;
}


BOOL Uninstall()
{
	if (!IsInstalled())
		return TRUE;

	SC_HANDLE hSCM = ::OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);

	if (hSCM == NULL)
	{
		MessageBox(NULL, _T("Couldn't open service manager"), szServiceName, MB_OK);
		return FALSE;
	}

	SC_HANDLE hService = ::OpenService(hSCM, szServiceName, SERVICE_STOP | DELETE);

	if (hService == NULL)
	{
		::CloseServiceHandle(hSCM);
		MessageBox(NULL, _T("Couldn't open service"), szServiceName, MB_OK);
		return FALSE;
	}
	SERVICE_STATUS status;
	::ControlService(hService, SERVICE_CONTROL_STOP, &status);

	//删除服务
	BOOL bDelete = ::DeleteService(hService);
	::CloseServiceHandle(hService);
	::CloseServiceHandle(hSCM);

	if (bDelete)
		return TRUE;

	writeToLog("service could not be detected");
	return FALSE;
}


void writeToLog(char * str)
{
	if(1)
		return;
	char des[512];
	GetModuleFileName(NULL, des, 512);  
	//取出文件路径   
	for (int i=strlen(des); i>=0; i--)
	{   
		if (des[i] == '\\')
		{
			des[i] = '\0';
			break;   
		}
	}
	strcat(des , "\\Client.log");

	FILE * pfile;
	fopen_s( &pfile , des , "a+");
	if (pfile == NULL)
	{
		return ;
	}
	fprintf_s( pfile , "%s\n" , str);
	fclose( pfile );
}
void informAdmin(std::vector<ModelServer>::iterator ms)
{
	writeToLog("开始发送飞信");
	std::string fetion_shell = mrc.fetion_shell;
	std::string open = "open";
	int pos = fetion_shell.find_first_of(' ');
	std::string exe = fetion_shell.substr( 0 , pos);
	std::string param = fetion_shell.substr(pos + 1 , fetion_shell.size());
	param.append(" ");
	param.append(ms->m_server_IP);

	::ShellExecute( NULL , open.c_str() , exe.c_str() , param.c_str() , NULL , SW_HIDE );
	writeToLog("结束发送飞信");

}

void accessModel(std::vector<ModelServer>::iterator ms)
{
	writeToLog("开始访问服务器");
	if(ms->isMaintence())
	{
		ms->m_server_state = MAINTENCE;
		ms->m_server_remark = "服务器维护期间不允许检测";
		return;
	}
	using namespace boost::asio; 

	io_service iosev; 
	ip::tcp::socket socket(iosev); 
	ip::tcp::endpoint ep(ip::address_v4::from_string(ms->m_server_IP), DEFAULT_PORT); 
	boost::system::error_code ec; 
	socket.connect(ep,ec); 
	if(ec) 
	{ 
		ms->m_server_state = BADIP;
		ms->m_server_remark = ec.message();
		informAdmin(ms);
		return;
	} 
	// 发送数据
	char buf[20] = "!@#$%^&*(){}<>?X"; 
	size_t len = socket.write_some(buffer(buf), ec);
	memset(buf, 0, 20);
	len=socket.read_some(buffer(buf), ec); 
	if(ec)
	{
		ms->m_server_state = UNRETURN;
		ms->m_server_remark = ec.message();
		informAdmin(ms);
		return ;
	}
	std::string back = buf;
	ms->m_server_state = ACCESSIBLE;
	ms->m_server_remark = back;
	
	writeToLog("结束访问服务器");
}

void sub_function_thread()
{
	boost::asio::io_service is;
	boost::asio::deadline_timer t( is , boost::posix_time::seconds(1));
	while(isStart)
	{
		for(int i = mrc.timer * 60 ; i > 0 ; i --)
		{
			if(!isStart)
				return;
			t.wait();
		}
		std::vector<ModelServer>::iterator it = servers.begin();
		while(it != servers.end())
		{
			accessModel(it);
			++ it;
		}
	}
}

void ClientStart()
{
	writeToLog("进入服务程序");
	isStart = 1;
	readConfig();
	//printToFile();
	/*boost::thread th(&sub_function_thread);
	while(isStart);*/
	boost::asio::io_service is;
	boost::asio::deadline_timer t( is , boost::posix_time::seconds(1));
	while(isStart)
	{


		for(int i = mrc.timer * 60 ; i > 0 ; i --)
		{
			if(!isStart)
				return;
			t.wait();
			t.expires_from_now(boost::posix_time::seconds(1));
		}
		std::vector<ModelServer>::iterator it = servers.begin();
		while(it != servers.end())
		{
			accessModel(it);
			++ it;
		}
	}
}



void readConfig()
{
	writeToLog("读取配置文件");
	char IP[20] = "";
	char NAME[255] = "";
	char START[10] = "";
	char END[10] = "";
	char AM[5]= "";
	int i = 1;

	char des[512];
	GetModuleFileName(NULL, des, 512);  
	//取出文件路径   
	for (int i=strlen(des); i>=0; i--)
	{   
		if (des[i] == '\\')
		{
			des[i] = '\0';
			break;   
		}
	}
	strcat(des , "\\config.ini");
	
	while(
		read_profile_string("IP" , ("ip" + anyToString(i)).c_str() , IP , 20 , "" , des )
		&& read_profile_string("NAME" , ("name" + anyToString(i)).c_str() , NAME , 255 , "" ,  des )
		&& read_profile_string("START" , ("start" + anyToString(i)).c_str() , START , 10 , "0" , des )
		&& read_profile_string("END" , ("end" + anyToString(i)).c_str() , END , 10 , "0" , des )
		&& read_profile_string("AM" , ("am" + anyToString(i)).c_str() , AM , 5 , "0" , des )
		)
	{
		ModelServer ms;
		ms.m_server_IP = IP;
		ms.m_server_name = NAME;
		
		const char split = ':';

		int start_H = atoi(strtok( START , &split));
		int start_M = atoi(strtok(NULL , &split));

		int end_H = atoi(strtok(END , &split));
		int end_M = atoi(strtok(NULL , &split));

		ms.maintence_start = start_H*60 + start_M;
		ms.maintence_end = end_H*60 + end_M;


		//printf("%d:%d" , ms.maintence_start , ms.maintence_end);
		//ms.maintence_start = atoi(START);
		//ms.maintence_end = atoi(END);


		ms.AllowMaintence = atoi(AM);
		servers.push_back(ms);
		++ i;
	}

	char timer [5] = "";
	char fetion_id[15] = "";
	char fetion_pswd[255] = "";
	char fetion_recv[15] = "";
	char fetion_shell[255] = "";

	read_profile_string("MRC" , "timer" , timer , 5 , "" , des );
	read_profile_string("MRC" , "fetion_id" , fetion_id , 15 , "" , des );
	read_profile_string("MRC" , "fetion_pswd" , fetion_pswd , 255 , "" , des );
	read_profile_string("MRC" , "fetion_recv" , fetion_recv , 15 , "" , des );
	read_profile_string("MRC" , "fetion_shell" , fetion_shell , 255 , "" , des );

	mrc.fetion_id = fetion_id;
	mrc.fetion_pswd = fetion_pswd;
	mrc.fetion_recv = fetion_recv;
	mrc.fetion_shell = fetion_shell;
	mrc.timer = atoi(timer);

	printToFile();
}

