#include "StdAfx.h"

#include "ProcessMonitor.h"
#include <algorithm>

CProcessMonitor::CProcessMonitor(void):
	m_hThread(NULL),
	m_bEndThread(false)
{
}


CProcessMonitor::~CProcessMonitor(void)
{
}

unsigned CProcessMonitor::DoThread( void )
{
//	DWORD dw = GetTickCount();

	m_oCProcessList.Update();
	CProcessList::PROCLIST mapLastProcList = m_oCProcessList.GetProcessList();
	vector<DWORD> listLastProcPid;
	CProcessList::ExtractPidList(mapLastProcList, listLastProcPid);
	Sleep(2000);
	while(!m_bEndThread)
	{
		// get current process list
		m_oCProcessList.Update();
		CProcessList::PROCLIST mapCurrProcList = m_oCProcessList.GetProcessList(); 
		vector<DWORD> listCurrProcPid;
		CProcessList::ExtractPidList(mapCurrProcList, listCurrProcPid);
		// compare with old process list
		for (auto it = listCurrProcPid.begin(); it != listCurrProcPid.end(); it++)
		{
			auto iterFind = std::find(listLastProcPid.begin(),
				listLastProcPid.end(),
				*it);

			if (!listLastProcPid.empty() && iterFind == listLastProcPid.end())
			{
				// found new program start
				Notify(*it, mapCurrProcList[*it].c_str(), true);
			}
		} // for
		
		for (auto it = listLastProcPid.begin(); it != listLastProcPid.end(); it++)
		{
			auto iterFind = std::find(listCurrProcPid.begin(),
				listCurrProcPid.end(),
				*it);

			if (!listCurrProcPid.empty() && iterFind == listCurrProcPid.end())
			{
				// found program already closed
				Notify(*it, mapLastProcList[*it].c_str(), false);
			}
		} // for

		Sleep(2000);

		mapLastProcList.clear();
		listLastProcPid.clear();
		mapLastProcList = mapCurrProcList;
		CProcessList::ExtractPidList(mapLastProcList, listLastProcPid);
	} // while

// 	WTL::CString strOutput;
// 	strOutput.Format(_T("%d"), GetTickCount()-dw);
// 	OutputDebugString(strOutput);

	return 1;
}

unsigned CProcessMonitor::DoThread2( void )
{
//	DWORD dw = GetTickCount();
	
	m_oCProcessList.Update();
	CProcessList::PROCLIST mapLastProcList = m_oCProcessList.GetProcessList();
	vector<DWORD> listLastProcPid;
	CProcessList::ExtractPidList(mapLastProcList, listLastProcPid);
	std::sort(listLastProcPid.begin(), listLastProcPid.end());
	Sleep(2000);
	while(!m_bEndThread)
	{
		// get current process list
		m_oCProcessList.Update();
		CProcessList::PROCLIST mapCurrProcList = m_oCProcessList.GetProcessList(); 
		vector<DWORD> listCurrProcPid;
		CProcessList::ExtractPidList(mapCurrProcList, listCurrProcPid);
		std::sort(listCurrProcPid.begin(), listCurrProcPid.end());
		// compare with old process list
		auto itLast = listLastProcPid.begin();
		auto itCurr = listCurrProcPid.begin();
		while(itCurr != listCurrProcPid.end() || itLast != listLastProcPid.end())
		{
			// new start program
			if (itLast == listLastProcPid.end())
			{
				Notify(*itCurr, mapCurrProcList[*itCurr].c_str(), true);
				itCurr++;
				continue;
			}
			// program already closed
			if (itCurr == listCurrProcPid.end())
			{
				Notify(*itLast, mapLastProcList[*itLast].c_str(), false);
				itLast++;
				continue;
			}

			if (*itLast == *itCurr)
			{
				itCurr++;
				itLast++;
				continue;
			}

			if (*itCurr < *itLast)
			{
				Notify(*itCurr, mapCurrProcList[*itCurr].c_str(), true);
				itCurr++;
				continue;
			}

			if (*itCurr > *itLast)
			{
				Notify(*itLast, mapLastProcList[*itLast].c_str(), false);
				itLast++;
				continue;
			}
		} // while

		Sleep(2000);

		mapLastProcList.clear();
		listLastProcPid.clear();
		mapLastProcList = mapCurrProcList;
		CProcessList::ExtractPidList(mapLastProcList, listLastProcPid);
		std::sort(listLastProcPid.begin(), listLastProcPid.end());
	} // while

// 	WTL::CString strOutput;
// 	strOutput.Format(_T("%d"), GetTickCount()-dw);
// 	OutputDebugString(strOutput);

	return 1;
}

unsigned __stdcall CProcessMonitor::ThreadProc( void* pArguments )
{
	if (!pArguments) return 0;
	CProcessMonitor *pCProcessMonitor = (CProcessMonitor *)pArguments;
	return pCProcessMonitor->DoThread2();
}

bool CProcessMonitor::StartMonitoring( void )
{
	m_bEndThread = false;
	m_hThread = (HANDLE)_beginthreadex(NULL, 0, ThreadProc, this, 0, NULL);
	return m_hThread?true:false;
}

bool CProcessMonitor::StopMonitoring( void )
{
	m_bEndThread = true;
	DWORD dwExit = NULL;
	while(GetExitCodeThread(m_hThread, &dwExit)==TRUE && dwExit==STILL_ACTIVE)
	{
		printf("Still Checking...\n"); 
		Sleep(100); 
	}  // polling code(bad)
	::CloseHandle(m_hThread);
	m_hThread = NULL;

	return true;
}

bool CProcessMonitor::Notify( DWORD pid, LPCWSTR lpwszProcName, bool bStart /*= true*/ )
{
	WTL::CString strOutput;
	strOutput.Format(_T("程序 %s，pid：%d 已经 %s !\r\n"), lpwszProcName, pid, bStart?_T("启动"):_T("结束"));
	OutputDebugString(strOutput);

	return true;
}
