#pragma once

#include "ProcessList.h"

class CProcessMonitor
{
public:
	CProcessMonitor(void);
	virtual ~CProcessMonitor(void);	
	bool StartMonitoring(void);
	bool StopMonitoring(void);
protected:
	virtual unsigned DoThread(void);
	virtual unsigned DoThread2(void);
	virtual bool Notify(DWORD pid, LPCWSTR lpsz, bool bStart = true);
private:
	static unsigned __stdcall ThreadProc(void* pArguments);
	HANDLE m_hThread;
	volatile bool m_bEndThread;
private:
	CProcessList m_oCProcessList;
};

