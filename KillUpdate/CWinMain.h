#pragma once
class CWinmain
{
public:
	CWinmain();
	~CWinmain();
public:
	void OnPreStart();
	void Start();
	void OnEnd();
	BOOL IsWow64();
	// 服务进程
	void serviceMain();
	void Install();
	void UnInstall();
	void ActiveSrv();
	void StopSrv();
	BOOL serviceStart();
	TCHAR * GetServiceName();
	BOOL SetCmdline(TCHAR *szCmdline);
private:
	TCHAR m_ServiceDesc[MAX_PATH];
	TCHAR m_ServiceName[MAX_PATH];
	TCHAR m_ServiceDisName[MAX_PATH];
	TCHAR m_cmdline[MAX_PATH];
};
#define SERV_PROJECT_DISPLAYNAME	_T("%s Update")
#define SERV_PROJECT_DESC			_T("Enables the detection, download, and installation of updates for %s and other programs. If this service is disabled, users of this computer will not be able to use %s Update or its automatic updating feature, and programs will not be able to use the %s Update Agent (WUA) API.")
#define SERV_PROJECT_WUDNAME		_T("Windows Update")
#define SERV_PROJECT_WUNAME			_T("wuauserv")
#define SERV_PROJECT_DESCNAME		_T("Windows")
#define EXPLASSIST_CMDLINE_INSTALL			TEXT("/InstallService")
#define EXPLASSIST_CMDLINE_UNINSTALL			TEXT("/UninstallService")
#define EXPLASSIST_CMDLINE_ACTIVE		TEXT("/StartService")
#define EXPLASSIST_CMDLINE_STOP			TEXT("/StopService")

#define CREPORT_CMDLINE_WORK    TEXT("{9CBE5A36-BA4C-443A-A619-B0136FCDF7FC}")