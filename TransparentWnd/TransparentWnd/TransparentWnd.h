
// TransparentWnd.h : PROJECT_NAME Ӧ�ó������ͷ�ļ�
//

#pragma once

#ifndef __AFXWIN_H__
	#error "�ڰ������ļ�֮ǰ������stdafx.h�������� PCH �ļ�"
#endif

#include "resource.h"		// ������


// CTransparentWndApp:
// �йش����ʵ�֣������ TransparentWnd.cpp
//

class CTransparentWndApp : public CWinApp
{
public:
	CTransparentWndApp();

// ��д
public:
	virtual BOOL InitInstance();

// ʵ��

	DECLARE_MESSAGE_MAP()
};

extern CTransparentWndApp theApp;