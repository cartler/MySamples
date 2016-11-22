#pragma once

#include <string>

class CProcessInfo
{
public:
	CProcessInfo(void);
	~CProcessInfo(void);
	HWND GetMainWndFromPid(DWORD dwPid);
	std::wstring GetModulePathFromPid(DWORD dwPid);
	std::wstring GetWndCaption(HWND hWnd);
	std::wstring GetWndClassName(HWND hWnd);
	std::wstring GetModuleFileMD5(LPCWSTR lpszPath);
	DWORD GetModuleFileSize(LPCWSTR lpszPath);
	
//protected:
private:
	static BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam);
	BOOL EnablePrivilege(HANDLE hToken, LPCTSTR szPrivName);
	std::wstring DosDevicePath2LogicalPath(LPCWSTR lpszDosPath);
};

