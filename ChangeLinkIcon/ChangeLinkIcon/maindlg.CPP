#include "StdAfx.h"
#include "MainDlg.h"

#include <shlobj.h>
#include <shobjidl.h>
/*
 *	Icons are numbered beginning with 0 in the upper left-hand corner and continuing with all the icons in column 1. 
 */
bool ChangeLinkIcon(const CString &strLnkName, const CString &strIconPath, const int nIndex = 0)
{
	if (strLnkName.IsEmpty() || strIconPath.IsEmpty())
	{
		return false;
	}

	HRESULT hres;
	IShellLink *psl = NULL;
	IPersistFile *pPf = NULL;
	int id;
	LPITEMIDLIST pidl;
	bool bRet = false;

	do
	{
		hres = CoInitialize(NULL);
		if (FAILED(hres))
		{
			break;
		}

		hres = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_IShellLink, (LPVOID*)&psl);
		if (FAILED(hres))
		{
			break;
		}

		hres = psl->QueryInterface(IID_IPersistFile, (LPVOID*)&pPf);
		if (FAILED(hres))
		{
			break;
		}

		hres = pPf->Load(strLnkName, STGM_READWRITE);    
		if (FAILED(hres))
		{
			break;
		}

		hres = psl->SetIconLocation(strIconPath, nIndex);
		if (FAILED(hres))
		{
			break;
		}

		pPf->Save(strLnkName, TRUE);
		if (FAILED(hres))
		{
			break;
		}

		bRet = true;

	} while (0);

	if (pPf != NULL)
	{
		pPf->Release();
	}

	if (psl != NULL)
	{
		psl->Release();
	}

	CoUninitialize();

	return bRet;
}

LRESULT CMainDlg::OnBnClickedButton1(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	// TODO: 在此添加控件通知处理程序代码

	static int i = 0;
	bool b = ChangeLinkIcon(_T("C:\\Users\\tanghong\\Desktop\\腾讯QQ2.lnk"), _T("C:\\Windows\\System32\\SHELL32.dll"), i);
	if (!b)
	{
		MessageBox( _T("failed!"), _T("info"), MB_OK);
	}
	i++;

	

	return 0;
}
