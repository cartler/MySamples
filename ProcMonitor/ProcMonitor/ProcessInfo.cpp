#include "StdAfx.h"
#include "ProcessInfo.h"

#include <Psapi.h>
#pragma comment(lib, "psapi.lib")
#include <Wincrypt.h>

#include <atlstr.h>

CProcessInfo::CProcessInfo(void)
{
}

CProcessInfo::~CProcessInfo(void)
{
}

BOOL CALLBACK CProcessInfo::EnumWindowsProc(HWND hwnd, LPARAM lParam) 
{ 
	DWORD dwCurProcessId = *((DWORD*)lParam); 
	DWORD dwProcessId = 0; 

	GetWindowThreadProcessId(hwnd, &dwProcessId); 
	if(dwProcessId == dwCurProcessId && GetParent(hwnd) == NULL)
	{ 
		*((HWND *)lParam) = hwnd;
		return FALSE; 
	} 
	return TRUE;
}

HWND CProcessInfo::GetMainWndFromPid( DWORD dwPid )
{
	DWORD dwCurrentProcessId = dwPid;
	if(!EnumWindows(CProcessInfo::EnumWindowsProc, (LPARAM)&dwCurrentProcessId)) 
	{     
		return (HWND)dwCurrentProcessId; 
	} 
	return NULL; 
}

BOOL CProcessInfo::EnablePrivilege( HANDLE hToken, LPCTSTR szPrivName )
{
	TOKEN_PRIVILEGES tkp;  

	LookupPrivilegeValue( NULL,szPrivName,&tkp.Privileges[0].Luid );//修改进程权限  
	tkp.PrivilegeCount=1;  
	tkp.Privileges[0].Attributes=SE_PRIVILEGE_ENABLED;  
	AdjustTokenPrivileges( hToken,FALSE,&tkp,sizeof tkp,NULL,NULL );//通知系统修改进程权限  

	return( (GetLastError()==ERROR_SUCCESS) );
}

std::wstring CProcessInfo::GetWndCaption( HWND hWnd )
{
	int nLen = ::GetWindowTextLength(hWnd);
	ATL::CString strCaption;
	if (nLen > 0)
	{
		::GetWindowText(hWnd, strCaption.GetBufferSetLength(nLen+1), nLen+1);
	}
	return strCaption;
}

std::wstring CProcessInfo::GetWndClassName( HWND hWnd )
{
	ATL::CString strClassName;
	::GetClassName(hWnd, strClassName.GetBufferSetLength(MAX_PATH+1), MAX_PATH);
	return strClassName;
}

std::wstring CProcessInfo::GetModulePathFromPid( DWORD dwPid )
{
	HANDLE hProcess=OpenProcess(PROCESS_QUERY_INFORMATION|PROCESS_VM_READ,FALSE, dwPid); 
	ATL::CString strPath;
	if (hProcess)
	{
		//::GetModuleFileNameEx(hProcess,NULL,strPath.GetBufferSetLength(MAX_PATH+1),MAX_PATH);
		::GetProcessImageFileName(hProcess, strPath.GetBufferSetLength(MAX_PATH+1), MAX_PATH);
		return DosDevicePath2LogicalPath(strPath);
	}
	if ( NULL != hProcess )  
	{  
		::CloseHandle( hProcess );  
		hProcess = NULL;  
	}  
	return strPath;
}

std::wstring CProcessInfo::DosDevicePath2LogicalPath( LPCWSTR lpszDosPath )
{
	std::wstring strLogicalPath;

	// Translate path with device name to drive letters.
	TCHAR szTemp[MAX_PATH] = {0};
	szTemp[0] = '\0';

	if ( lpszDosPath==NULL || !GetLogicalDriveStrings(_countof(szTemp)-1, szTemp) ){
		return strLogicalPath;
	}


	TCHAR szName[MAX_PATH] = {0};
	TCHAR szDrive[3] = TEXT(" :");
	BOOL bFound = FALSE;
	TCHAR* p = szTemp;

	do{
		// Copy the drive letter to the template string
		*szDrive = *p;

		// Look up each device name
		if ( QueryDosDevice(szDrive, szName, _countof(szName)) ){
			UINT uNameLen = (UINT)_tcslen(szName);

			if (uNameLen < MAX_PATH) 
			{
				bFound = _tcsnicmp(lpszDosPath, szName, uNameLen) == 0;

				if ( bFound ){
					// Reconstruct pszFilename using szTemp
					// Replace device path with DOS path
					TCHAR szTempFile[MAX_PATH];
					_stprintf_s(szTempFile, TEXT("%s%s"), szDrive, lpszDosPath+uNameLen);
					strLogicalPath = szTempFile;
				}
			}
		}

		// Go to the next NULL character.
		while (*p++);
	} while (!bFound && *p); // end of string

	return strLogicalPath;
}

DWORD CProcessInfo::GetModuleFileSize( LPCWSTR lpszPath )
{
	DWORD dwSize = 0;
	HANDLE hFile = ::CreateFile(lpszPath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL, NULL);
	do 
	{
		if (hFile == INVALID_HANDLE_VALUE)
		{
			break;
		}
		dwSize = ::GetFileSize(hFile, NULL);
		if (dwSize == INVALID_FILE_SIZE)
		{
			break;
		}
	} while (0);

	CloseHandle(hFile);
	return dwSize;
}

std::wstring CProcessInfo::GetModuleFileMD5( LPCWSTR lpszPath )
{
	BOOL bResult = FALSE;
	std::wstring strMD5;

	HCRYPTPROV hProv = 0;
	HCRYPTHASH hHash = 0;
	HANDLE hFile = NULL;
	do 
	{
		bResult = CryptAcquireContext(&hProv,
			NULL,
			NULL,
			PROV_RSA_FULL,
			CRYPT_VERIFYCONTEXT);
		if (!bResult) break;

		bResult = CryptCreateHash(hProv, CALG_MD5, 0, 0, &hHash);
		if (!bResult) break;

		// read file
		hFile = CreateFile((TCHAR *)lpszPath,
			GENERIC_READ,
			FILE_SHARE_READ,
			NULL,
			OPEN_EXISTING,
			FILE_FLAG_SEQUENTIAL_SCAN,
			NULL);
		if (!bResult) break;

		const int BUFSIZE = 1024;
		BYTE rgbFile[BUFSIZE] = {0};
		DWORD cbRead = 0;
		while (bResult = ReadFile(hFile, rgbFile, BUFSIZE, 
			&cbRead, NULL))
		{
			if (0 == cbRead)
			{
				break;
			}

			bResult = CryptHashData(hHash, rgbFile, cbRead, 0);
			//
			if (!bResult) break;
		}
		if (!bResult) break;

		const int MD5LEN = 32;
		DWORD cbHash = 0;
		cbHash = MD5LEN;
		BYTE rgbHash[MD5LEN] = {0};
		CHAR rgbDigits[] = "0123456789abcdef";
		ATL::CString strHash;
		if (bResult = CryptGetHashParam(hHash, HP_HASHVAL, rgbHash, &cbHash, 0))
		{
			TCHAR szTmpBuff[3] = {0};
			for (DWORD i = 0; i < cbHash; i++)
			{
				swprintf(szTmpBuff, TEXT("%c%c"), rgbDigits[rgbHash[i] >> 4],
					rgbDigits[rgbHash[i] & 0xf]);
				lstrcat(strHash.GetBufferSetLength(MD5LEN+1), szTmpBuff);
			}
			strMD5 = strHash;
			bResult = TRUE;
		}
		
	} while (0);

	DWORD dwStatus = GetLastError();
	CryptDestroyHash(hHash);
	CryptReleaseContext(hProv, 0);
	CloseHandle(hFile);

	return strMD5;
}
