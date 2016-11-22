#include "StdAfx.h"
#include "MainDlg.h"

#include "ProcessInfo.h"


LRESULT CMainDlg::OnLbnDblclkListProcs(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	// TODO: 在此添加控件通知处理程序代码
	int id = m_olbProcs.GetCurSel();
	CString strText;
	m_olbProcs.GetText(id , strText);
	
	m_oMonitorList.push_back(strText.GetBuffer(strText.GetLength()));
	m_olbMonitor.AddString(strText);
	
	return 0;
}

LRESULT CMainDlg::OnBnClickedButton1(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	// TODO: 在此添加控件通知处理程序代
			
	CProcessInfo oProcInfo;
	std::wstring &&strPath = oProcInfo.GetModulePathFromPid(23392);
	std::wstring &&strCaption = oProcInfo.GetWndCaption(oProcInfo.GetMainWndFromPid(23392));
	DWORD size = oProcInfo.GetModuleFileSize(strPath.c_str());
	std::wstring &&strMd5 = oProcInfo.GetModuleFileMD5(strPath.c_str());
	
	int i = 0;
	
	return 0;
}
