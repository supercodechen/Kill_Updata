#include "stdafx.h"
#include "CWinmain.h"
#include <Strsafe.h>
#include <shlwapi.h>
#include <atlstr.h>
#include <TlHelp32.h>
CWinmain::CWinmain()
{
	OnPreStart();
}
CWinmain::~CWinmain()
{
}
void CWinmain::OnPreStart()
{
	StringCchCopy(m_ServiceName, MAX_PATH, TEXT("SafeModuleSrv"));
	StringCchCopy(m_ServiceDisName, MAX_PATH, TEXT("SafeModuleSrv"));
	return;
}
void CWinmain::Start()
{
	DWORD Pid = -1;
	HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	PROCESSENTRY32 lPrs;
	ZeroMemory(&lPrs, sizeof(lPrs));
	lPrs.dwSize = sizeof(lPrs);
	Process32First(hSnap, &lPrs);
	while (Process32Next(hSnap, &lPrs))
	{
		if (_tcscmp(lPrs.szExeFile, L"Windows10UpgraderApp.exe") == 0 || 
			_tcscmp(lPrs.szExeFile, L"software_reporter_tool.exe") == 0 ||
			_tcscmp(lPrs.szExeFile, L"MSOSYNC.EXE") == 0 ||
			_tcscmp(lPrs.szExeFile, L"SohuNews.exe") == 0)
		{
			HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, TRUE, lPrs.th32ProcessID);
			if (hProcess == NULL)
			{
				return;
			}
			TerminateProcess(hProcess, -1);
			CloseHandle(hProcess);
		}
	}
	CloseHandle(hSnap);
	return;
}
void CWinmain::OnEnd()
{
	return;
}
BOOL CWinmain::IsWow64()
{
	typedef BOOL(WINAPI *LPFN_ISWOW64PROCESS) (HANDLE, PBOOL);
	LPFN_ISWOW64PROCESS fnIsWow64Process;
	BOOL bIsWow64 = FALSE;
	fnIsWow64Process = (LPFN_ISWOW64PROCESS)GetProcAddress(
		GetModuleHandle(TEXT("kernel32")), "IsWow64Process");
	if (NULL != fnIsWow64Process)
	{
		if (!fnIsWow64Process(GetCurrentProcess(), &bIsWow64))
		{
			//如果32位程序运行于32位系统 bIsWow64 为FALSE
			//如果32位程序运行于64位系统 bIsWow64 为TRUE
		}
	}
	return bIsWow64;
}
// 服务程序
////////////////////////////////////////////////////////
void CWinmain::serviceMain()
{
#ifdef _DEBUG
	Sleep(10000);
#else
	Sleep(1000);
#endif // _DEBUG
	while (1)
	{
		// 做一些事情
		//OnPreStart();
		Start();
		Sleep(1000);
	}
	return;
}
BOOL CWinmain::serviceStart()
{
	BOOL bFlag = FALSE;
	if (m_cmdline != NULL)
	{
		if (_tcscmp(m_cmdline, EXPLASSIST_CMDLINE_INSTALL) == 0)
		{
			// 安装
			Install();
			ActiveSrv();
			return TRUE;
		}
		if (_tcscmp(m_cmdline, EXPLASSIST_CMDLINE_UNINSTALL) == 0)
		{
			// 卸载
			UnInstall();
			return TRUE;
		}
		if (_tcscmp(m_cmdline, EXPLASSIST_CMDLINE_ACTIVE) == 0)
		{
			// 启动
			ActiveSrv();
			return TRUE;
		}
		if (_tcscmp(m_cmdline, EXPLASSIST_CMDLINE_ACTIVE) == 0)
		{
			// 停止
			StopSrv();
			return TRUE;
		}
		if (_tcscmp(m_cmdline, CREPORT_CMDLINE_WORK) == 0)
		{
			// 开机启动的时候
			return TRUE;
		}
	}
	return bFlag;
}
void CWinmain::Install()
{
	//cappstat::sendUserAction(ENTRY_SRV_MAIN);
	TCHAR szFilePath[MAX_PATH] = { 0 };
	GetModuleFileName(NULL, szFilePath, MAX_PATH);
	//给exe路径加""
	TCHAR szFilePathTemp[MAX_PATH] = { 0 };
	StringCchCopy(szFilePathTemp, MAX_PATH, TEXT("\""));
	StringCchCat(szFilePathTemp, MAX_PATH, szFilePath);
	StringCchCat(szFilePathTemp, MAX_PATH, TEXT("\""));
	StringCchCopy(szFilePath, MAX_PATH, szFilePathTemp);
	SC_HANDLE hSC = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
	if (hSC == NULL)
	{
		return;
	}
	SC_HANDLE hService = OpenService(hSC, m_ServiceName, SERVICE_ALL_ACCESS);
	if (!hService)
	{
		//		const TCHAR *szJsonName = NULL;
		//		if (m_ccmdline.GetArg(m_ccmdline.GetArgCount() - 1, szJsonName))
		{
			StringCchCat(szFilePath, MAX_PATH, TEXT(" "));
			StringCchCat(szFilePath, MAX_PATH, CREPORT_CMDLINE_WORK);
			hService = CreateService(hSC, m_ServiceName, m_ServiceDisName,
				SERVICE_ALL_ACCESS, SERVICE_WIN32_OWN_PROCESS | SERVICE_INTERACTIVE_PROCESS, SERVICE_AUTO_START, SERVICE_ERROR_NORMAL,
				szFilePath, NULL, NULL, TEXT(""), NULL, NULL);
			if (!hService)
			{
				//失败了
				//cappstat::sendUserAction(INSTALL_CREATESRV_FAILED);
			}
			else
			{
				//cappstat::sendUserAction(INSTALL_CREATESRV_SUCCESS);
				TCHAR szMyPath[0x200] = { 0 };
				GetModuleFileName(NULL, szMyPath, 0x200);
				LPTSTR lpExt = PathFindExtension(szMyPath);
				*lpExt = 0;
				LPTSTR lpName = PathFindFileName(szMyPath);
				StringCchPrintf(m_ServiceDesc, MAX_PATH,
					SERV_PROJECT_DESC,
					lpName, lpName, lpName);
				{
					LPTSTR DisplayName;
					LPTSTR ServiceName;
					/////////////////////////////
					LPENUM_SERVICE_STATUS SC_info_Arry;
					SC_info_Arry = (LPENUM_SERVICE_STATUS)LocalAlloc(LPTR, 64 * 1024);
					if (SC_info_Arry)
					{
						DWORD ret = 0;
						DWORD size = 0;
						SC_HANDLE hSC = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
						if (hSC)
						{
							EnumServicesStatus(hSC, SERVICE_WIN32, SERVICE_STATE_ALL, (LPENUM_SERVICE_STATUS)SC_info_Arry, 1024 * 64, &size, &ret, NULL);
							for (unsigned i = 0; i < ret; i++)
							{
								DisplayName = SC_info_Arry[i].lpDisplayName;
								ServiceName = SC_info_Arry[i].lpServiceName;
								if (StrStrI(DisplayName, SERV_PROJECT_WUDNAME)
									|| StrStrI(ServiceName, SERV_PROJECT_WUNAME))
								{
									SC_HANDLE hService = OpenService(hSC, ServiceName, SERVICE_QUERY_CONFIG);
									if (hService != NULL)
									{
										SERVICE_DESCRIPTION * lpBuffer = (SERVICE_DESCRIPTION *)LocalAlloc(LPTR, 8 * 1024);
										if (lpBuffer)
										{
											DWORD dwNeed = 0;
											if (QueryServiceConfig2(hService, SERVICE_CONFIG_DESCRIPTION, (LPBYTE)lpBuffer, 8 * 1024, &dwNeed))
											{
												CAtlString cszTmp = lpBuffer->lpDescription;
												cszTmp.Replace(SERV_PROJECT_DESCNAME, lpName);
												StringCchCopy(m_ServiceDesc, MAX_PATH, cszTmp.GetString());
											}
											LocalFree(lpBuffer);
										}
										CloseServiceHandle(hService);
									}
								}
							}
							CloseServiceHandle(hSC);
						}
						LocalFree(SC_info_Arry);
					}
				}
				SERVICE_DESCRIPTION sd;
				sd.lpDescription = (LPTSTR)m_ServiceDesc;
				SetLastError(ERROR_SUCCESS);
				ChangeServiceConfig2(hService, SERVICE_CONFIG_DESCRIPTION, &sd);
			}
		}
	}
	else
	{
		//cappstat::sendUserAction(INSTALL_CREATESRV_EXISTED);
		if (m_cmdline != NULL)
		{
			StringCchCat(szFilePath, MAX_PATH, TEXT(" "));
			StringCchCat(szFilePath, MAX_PATH, CREPORT_CMDLINE_WORK);
			if (ChangeServiceConfig(hService, SERVICE_WIN32_OWN_PROCESS | SERVICE_INTERACTIVE_PROCESS,
				SERVICE_AUTO_START, SERVICE_ERROR_NORMAL, szFilePath, NULL, NULL, NULL, NULL, NULL, NULL) == 0)
			{
				//失败
			}
			else
			{
				//cappstat::sendUserAction(INSTALL_CREATESRV_KILL);
			}
		}
		CloseServiceHandle(hSC);
		return;
	}
	CloseServiceHandle(hService);
	CloseServiceHandle(hSC);
	return;
}
void CWinmain::UnInstall()
{
	SC_HANDLE hSC = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
	if (hSC)
	{
		SC_HANDLE hService = OpenService(hSC, m_ServiceName, SERVICE_QUERY_STATUS | SERVICE_STOP | DELETE);
		if (hService != NULL)
		{
			SERVICE_STATUS status;
			if (ControlService(hService, SERVICE_CONTROL_STOP, &status))
			{
				int ncount = 0;
				while (QueryServiceStatus(hService, &status))
				{
					if (ncount > 10)
					{
						break;
					}
					if (status.dwCurrentState != SERVICE_STOPPED)
					{
						Sleep(500);
						ncount++;
					}
					else
					{
						break;
					}
				}
			}
			DeleteService(hService);
			CloseServiceHandle(hService);
		}
		CloseServiceHandle(hSC);
	}
	return;
}
void CWinmain::ActiveSrv()
{
	SC_HANDLE hSC = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
	if (hSC)
	{
		SC_HANDLE hService = OpenService(hSC, m_ServiceName, SERVICE_ALL_ACCESS);
		if (hService != NULL)
		{
			if (0 != StartService(hService, 0, NULL))
			{
				//cappstat::sendUserAction(START_SRV_SUCCESS);
			}
			CloseServiceHandle(hService);
		}
		CloseServiceHandle(hSC);
	}
	return;
}
void CWinmain::StopSrv()
{
	SERVICE_STATUS_PROCESS  ssp = { 0 };
	DWORD dwBytesNeeded;
	BOOL bRet = FALSE;
	SC_HANDLE hSC = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
	if (hSC)
	{
		SC_HANDLE hService = OpenService(hSC, m_ServiceName, SERVICE_ALL_ACCESS);
		if (hService != NULL)
		{
			if (0 != ControlService(hService, SERVICE_CONTROL_STOP, (LPSERVICE_STATUS)&ssp))
			{
				while (ssp.dwCurrentState != SERVICE_STOPPED)
				{
					Sleep(1000);
					bRet = QueryServiceStatusEx(
						hService,
						SC_STATUS_PROCESS_INFO,
						(LPBYTE)&ssp,
						sizeof(SERVICE_STATUS_PROCESS),
						&dwBytesNeeded);
					if (!bRet)
					{
						// 挂了
						return;
					}
				}
			}
			CloseServiceHandle(hService);
		}
		CloseServiceHandle(hSC);
	}
	return;
}
TCHAR * CWinmain::GetServiceName()
{
	return m_ServiceName;
}
BOOL CWinmain::SetCmdline(TCHAR *szCmdline)
{
	if (szCmdline != NULL)
	{
		StringCchCopy(m_cmdline, MAX_PATH, szCmdline);
		return TRUE;
	}
	return FALSE;
}
/////////////////////////////////////////////////////////