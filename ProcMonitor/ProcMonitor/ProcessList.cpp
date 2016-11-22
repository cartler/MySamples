/********************************************************************
	created:	2015/05/11
	created:	11:5:2015   17:21
	file base:	ProcessList
	file ext:	cpp
	author:		tanghong
	
	purpose:	封装获取进程列表的实现细节
*********************************************************************/

#include "StdAfx.h"
#include "ProcessList.h"
#include <atlutil.h>
#include <algorithm>

CProcessList::CProcessList(void)
{
}

CProcessList::~CProcessList(void)
{
}

/*
 *	update process list
 */
bool CProcessList::Update( void )
{
	m_mapProcessList.clear();

	CProcessApi oCProcessApi;

	if (!oCProcessApi.Init(false))
	{
		printf("Failed to load either of process api libraries!");
		return false;
	}

	DWORD pl = oCProcessApi.ProcessesGetList();
	int i, j;
	if (pl)
	{
		CProcessApi::tProcessInfo pi;
		for (i=oCProcessApi.ProcessesCount(pl)-1;i>=0;i--)
		{
			if (!oCProcessApi.ProcessesWalk(pl, &pi, i))
			{
				printf("failed to process walk @ %d\n", i);
				continue;
			}
			printf("Process[%d]: %s\n", pi.pid, pi.FileName);
			// ---
			USES_CONVERSION;
			std::wstring strFileName = A2W((LPCSTR)pi.FileName);
			m_mapProcessList.insert(std::pair<DWORD, std::wstring> (pi.pid, strFileName.c_str()));
// #ifdef _DEBUG
// 			WTL::CString strText;
// 			strText.Format(_T("%s\r\n"), strFileName.c_str());
// 			OutputDebugString(strText);
// #endif
			// ---
			//std::wstring strFileName = PathFindFileName(pi.FileName);
		} // for
	} // if
	else
	{
		return false;
	}
	oCProcessApi.ProcessesFreeList(pl);

	return true;
}

bool CProcessList::ExtractPidList( const PROCLIST &mapProcList, vector<DWORD> &listProcPid )
{
	listProcPid.clear();
	std::for_each(mapProcList.begin(), mapProcList.end(), 
		[&listProcPid](PROCLIST::value_type it)
		{
			listProcPid.push_back(it.first);
	});
	return true;
}

bool CProcessList::ExtractProcNameList( const PROCLIST &mapProcList, vector<std::wstring> &listProcName )
{
	listProcName.clear();
	std::for_each(mapProcList.begin(), mapProcList.end(),
		[&listProcName](PROCLIST::value_type it)
		{
			listProcName.push_back(it.second);
	});
	return true;
}
