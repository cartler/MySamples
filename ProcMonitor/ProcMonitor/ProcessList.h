/********************************************************************
	created:	2015/05/11
	created:	11:5:2015   17:21
	file base:	ProcessList
	file ext:	h
	author:		tanghong
	
	purpose:	��װ��ȡ�����б��ʵ��ϸ��
*********************************************************************/

#pragma once

#include "processapi.h"
#include <map>
#include <vector>

class CProcessList
{
public:
	typedef std::map<DWORD, std::wstring> PROCLIST;
private:
	PROCLIST m_mapProcessList;
public:
	CProcessList(void);
	~CProcessList(void);
	bool Update(void);
	const PROCLIST & GetProcessList() {return m_mapProcessList;}
	static bool ExtractPidList(const PROCLIST &mapProcList, vector<DWORD> &listProcPid);
	static bool ExtractProcNameList(const PROCLIST &mapProcList, vector<std::wstring> &listProcName);
};

