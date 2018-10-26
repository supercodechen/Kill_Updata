// KillUpdate.cpp : 定义应用程序的入口点。
//

#include "stdafx.h"
#include "KillUpdate.h"
#include "CWinMain.h"

void WINAPI ServiceMain(DWORD dwArgc, PWSTR *pszArgv);
void WINAPI ServiceCtrlHandler(DWORD dwCtrl);
CWinmain cMain;
SERVICE_STATUS_HANDLE hStatus = NULL;
void WINAPI ServiceCtrlHandler(DWORD dwCtrl)
{
	switch (dwCtrl)
	{
	case SERVICE_CONTROL_STOP:
	{
		SERVICE_STATUS ss = { 0 };
		ss.dwCurrentState = SERVICE_STOPPED;
		ss.dwWin32ExitCode = NO_ERROR;
		ss.dwWaitHint = 0;
		ss.dwServiceType = SERVICE_WIN32;
		ss.dwControlsAccepted = SERVICE_ACCEPT_STOP;
		ss.dwCheckPoint = 1;
		if (hStatus)
		{
			SetServiceStatus(hStatus, &ss);
		}
	}
	break;
	case SERVICE_CONTROL_PAUSE:  break;
	case SERVICE_CONTROL_CONTINUE:  break;
	case SERVICE_CONTROL_SHUTDOWN:  break;
	case SERVICE_CONTROL_INTERROGATE: break;
	default: break;
	}
}
void WINAPI ServiceMain(DWORD dwArgc, PWSTR *pszArgv)
{
	hStatus = RegisterServiceCtrlHandler(/*Main.getServName()*/TEXT("SafeModuleSrv"), ServiceCtrlHandler);
	if (hStatus == NULL)
		return;
	{
		SERVICE_STATUS ss = { 0 };
		ss.dwCurrentState = SERVICE_RUNNING;
		ss.dwWin32ExitCode = NO_ERROR;
		ss.dwWaitHint = 0;
		ss.dwServiceType = SERVICE_WIN32;
		ss.dwControlsAccepted = SERVICE_ACCEPT_STOP;
		ss.dwCheckPoint = 1;
		SetServiceStatus(hStatus, &ss);
	}
	cMain.serviceMain();
	{
		SERVICE_STATUS ss = { 0 };
		ss.dwCurrentState = SERVICE_STOPPED;
		ss.dwWin32ExitCode = NO_ERROR;
		ss.dwWaitHint = 0;
		ss.dwServiceType = SERVICE_WIN32;
		ss.dwControlsAccepted = SERVICE_ACCEPT_STOP;
		ss.dwCheckPoint = 1;
		SetServiceStatus(hStatus, &ss);
	}
}
int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR    lpCmdLine,
	_In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	// TODO: 在此放置代码。
	// 注册服务，并启动服务
	cMain.SetCmdline(lpCmdLine);
	if (!cMain.serviceStart())
	{
		exit(0);
	}
	SERVICE_TABLE_ENTRY serviceTable[] =
	{
		{ cMain.GetServiceName(), (LPSERVICE_MAIN_FUNCTION)ServiceMain },
		{ NULL, NULL }
	};
	StartServiceCtrlDispatcher(serviceTable);

	return 1;
}