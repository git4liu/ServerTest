#include "stdio.h"
#include "tchar.h"
#include <iostream> 
#include <boost/asio.hpp> 
#include <string>
#include <Windows.h>
#include <boost/thread/thread.hpp>

#define DEFAULT_PORT 13184

BOOL isStart;
 


void Init();
BOOL IsInstalled();
BOOL Install();
BOOL Uninstall();
void writeToLog(char * str);
void WINAPI ServiceMain();
void WINAPI ServiceStrl(DWORD dwOpcode);

int killThread();

void ServerStart();

TCHAR szServiceName[] = _T("wbyServerTest_Server");
TCHAR szServiceDesc[] = _T("此服务用于伍佰亿服务器测试，服务端监听");
BOOL bInstall;
SERVICE_STATUS_HANDLE hServiceStatus;
SERVICE_STATUS status;
DWORD dwThreadID;

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
	ServerStart();
	//boost::thread th (&ServerStart);
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
		killThread();
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
		killThread();
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
	strcat(des , "\\Server.log");

	FILE * pfile;
	fopen_s( &pfile , des , "a+");
	if (pfile == NULL)
	{
		return ;
	}
	fprintf_s( pfile , "%s\n" , str);
	fclose( pfile );
}

void ServerStart()
{
	writeToLog("开始服务程序");
	isStart = TRUE;
	using namespace boost::asio;
	io_service iosev; 
	ip::tcp::acceptor acceptor(iosev,  ip::tcp::endpoint(ip::tcp::v4(), DEFAULT_PORT)); 
	std::string backMsg;
	while(isStart) 
	{ 
		ip::tcp::socket socket(iosev); 
		acceptor.accept(socket); 
		boost::system::error_code ec;

		char buf[20] = "";
		char bufOk[] = "OK";
		char bufError[] = "ERROR";
		int len = socket.read_some(buffer(buf), ec);
		std::string comeMsg = buf;
		if(comeMsg.substr(0 , 16) != "!@#$%^&*(){}<>?X")
		{ 
			socket.write_some(buffer(bufError, 6), ec); 
			continue;
		}
		if (ec)
		{
			break; 
		} 
		socket.write_some(buffer(bufOk, 2), ec); 
		if(ec) 
		{ 
			break; 
		} 
	} 
}



int killThread()
{
	writeToLog("关闭服务进程");
	using namespace boost::asio; 

	isStart = FALSE; 
	io_service iosev; 
	ip::tcp::socket socket(iosev); 
	ip::tcp::endpoint ep(ip::address_v4::from_string("127.0.0.1"), DEFAULT_PORT); 
	boost::system::error_code ec; 
	socket.connect(ep,ec); 
	if(ec) 
	{ 
		return 1;
	} 
	// 发送数据
	char buf[20] = "!@#$%^&*(){}<>?X"; 
	socket.write_some(buffer(buf), ec);
	return 0;
}