// testPrintWindow.cpp : 定义应用程序的入口点。
//

#include "stdafx.h"
#include "testPrintWindow.h"

#define MAX_LOADSTRING 100

// 全局变量:
HINSTANCE hInst;								// 当前实例
TCHAR szTitle[MAX_LOADSTRING];					// 标题栏文本
TCHAR szWindowClass[MAX_LOADSTRING];			// 主窗口类名







#include <vector>

#ifdef UNICODE  
#ifndef TSTRING  
#define TSTRING std::wstring  
#endif  
#else  
#ifndef TSTRING  
#define TSTRING std::string  
#endif  
#endif  

BOOL WriteBmp(const TSTRING &strFile,const std::vector<BYTE> &vtData,const SIZE &sizeImg);  
BOOL WriteBmp(const TSTRING &strFile,HDC hdc);  
BOOL WriteBmp(const TSTRING &strFile,HDC hdc,const RECT &rcDC);  

BOOL WriteBmp(const TSTRING &strFile,const std::vector<BYTE> &vtData,const SIZE &sizeImg)   
{     

	BITMAPINFOHEADER bmInfoHeader = {0};  
	bmInfoHeader.biSize = sizeof(BITMAPINFOHEADER);  
	bmInfoHeader.biWidth = sizeImg.cx;  
	bmInfoHeader.biHeight = sizeImg.cy;  
	bmInfoHeader.biPlanes = 1;  
	bmInfoHeader.biBitCount = 24;  

	//Bimap file header in order to write bmp file  
	BITMAPFILEHEADER bmFileHeader = {0};  
	bmFileHeader.bfType = 0x4d42;  //bmp    
	bmFileHeader.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);  
	bmFileHeader.bfSize = bmFileHeader.bfOffBits + ((bmInfoHeader.biWidth * bmInfoHeader.biHeight) * 3); ///3=(24 / 8)  

	HANDLE hFile = CreateFile(strFile.c_str(),GENERIC_WRITE,0,NULL,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL);  
	if(hFile == INVALID_HANDLE_VALUE)  
	{  
		return FALSE;  
	}  

	DWORD dwWrite = 0;  
	WriteFile(hFile,&bmFileHeader,sizeof(BITMAPFILEHEADER),&dwWrite,NULL);  
	WriteFile(hFile,&bmInfoHeader, sizeof(BITMAPINFOHEADER),&dwWrite,NULL);  
	WriteFile(hFile,&vtData[0], vtData.size(),&dwWrite,NULL);  


	CloseHandle(hFile);  

	return TRUE;  
}   


BOOL WriteBmp(const TSTRING &strFile,HDC hdc)  
{  
	int iWidth = GetDeviceCaps(hdc,HORZRES);  
	int iHeight = GetDeviceCaps(hdc,VERTRES);  
	RECT rcDC = {0,0,iWidth,iHeight};  

	return WriteBmp(strFile,hdc,rcDC);    
}  

BOOL WriteBmp(const TSTRING &strFile,HDC hdc,const RECT &rcDC)  
{  
	BOOL bRes = FALSE;  
	BITMAPINFO bmpInfo = {0};  
	BYTE *pData = NULL;  
	SIZE sizeImg = {0};  
	HBITMAP hBmp = NULL;  
	std::vector<BYTE> vtData;  
	HGDIOBJ hOldObj = NULL;  
	HDC hdcMem = NULL;  

	//Initilaize the bitmap information   
	bmpInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);  
	bmpInfo.bmiHeader.biWidth = rcDC.right - rcDC.left;  
	bmpInfo.bmiHeader.biHeight = rcDC.bottom - rcDC.top;  
	bmpInfo.bmiHeader.biPlanes = 1;  
	bmpInfo.bmiHeader.biBitCount = 24;  

	//Create the compatible DC to get the data  
	hdcMem = CreateCompatibleDC(hdc);  
	if(hdcMem == NULL)  
	{  
		goto EXIT;  
	}  

	//Get the data from the memory DC     
	hBmp = CreateDIBSection(hdcMem,&bmpInfo,DIB_RGB_COLORS,reinterpret_cast<VOID **>(&pData),NULL,0);  
	if(hBmp == NULL)  
	{  
		goto EXIT;  
	}  
	hOldObj = SelectObject(hdcMem, hBmp);  

	//Draw to the memory DC  
	sizeImg.cx = bmpInfo.bmiHeader.biWidth;  
	sizeImg.cy = bmpInfo.bmiHeader.biHeight;  
	StretchBlt(hdcMem,  
		0,  
		0,  
		sizeImg.cx,  
		sizeImg.cy,  
		hdc,  
		rcDC.left,  
		rcDC.top,  
		rcDC.right - rcDC.left + 1,  
		rcDC.bottom - rcDC.top + 1,  
		SRCCOPY);  


	vtData.resize(sizeImg.cx * sizeImg.cy * 3);  
	memcpy(&vtData[0],pData,vtData.size());  
	bRes = WriteBmp(strFile,vtData,sizeImg);  

	SelectObject(hdcMem, hOldObj);  


EXIT:  
	if(hBmp != NULL)  
	{  
		DeleteObject(hBmp);  
	}  

	if(hdcMem != NULL)  
	{  
		DeleteDC(hdcMem);  
	}  

	return bRes;  
}  









// 此代码模块中包含的函数的前向声明:
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);

int APIENTRY _tWinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR    lpCmdLine,
                     int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

 	// TODO: 在此放置代码。
	MSG msg;
	HACCEL hAccelTable;

	// 初始化全局字符串
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_TESTPRINTWINDOW, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// 执行应用程序初始化:
	if (!InitInstance (hInstance, nCmdShow))
	{
		return FALSE;
	}

	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_TESTPRINTWINDOW));

	// 主消息循环:
	while (GetMessage(&msg, NULL, 0, 0))
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return (int) msg.wParam;
}



//
//  函数: MyRegisterClass()
//
//  目的: 注册窗口类。
//
//  注释:
//
//    仅当希望
//    此代码与添加到 Windows 95 中的“RegisterClassEx”
//    函数之前的 Win32 系统兼容时，才需要此函数及其用法。调用此函数十分重要，
//    这样应用程序就可以获得关联的
//    “格式正确的”小图标。
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_TESTPRINTWINDOW));
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= MAKEINTRESOURCE(IDC_TESTPRINTWINDOW);
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassEx(&wcex);
}

//
//   函数: InitInstance(HINSTANCE, int)
//
//   目的: 保存实例句柄并创建主窗口
//
//   注释:
//
//        在此函数中，我们在全局变量中保存实例句柄并
//        创建和显示主程序窗口。
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   HWND hWnd;

   hInst = hInstance; // 将实例句柄存储在全局变量中

   hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);



   if (!hWnd)
   {
      return FALSE;
   }

//    ShowWindow(hWnd, SW_HIDE);
//    UpdateWindow(hWnd);
   //ShowWindow(hWnd, SW_HIDE);

   // Takes a snapshot of the window hwnd, stored in the memory device context hdcMem
   HDC hdc = GetWindowDC(hWnd);
   if (hdc)
   {
	   HDC hdcMem = CreateCompatibleDC(hdc);
	   if (hdcMem)
	   {
		   RECT rc;
		   GetWindowRect(hWnd, &rc);

		   HBITMAP hbitmap = CreateCompatibleBitmap(hdc, rc.right-rc.left, rc.bottom-rc.top);
		   if (hbitmap)
		   {
			   SelectObject(hdcMem, hbitmap);

			   PrintWindow(hWnd, hdcMem, 0);
			   
			   DeleteObject(hbitmap);
		   }
		   DeleteObject(hdcMem);
	   }
	   WriteBmp(TEXT("c:\\test\\DCSave.bmp"), hdc);
	   ReleaseDC(hWnd, hdc);

   }

   
   
   //ShowWindow(hWnd, SW_MINIMIZE);



   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

//
//  函数: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  目的: 处理主窗口的消息。
//
//  WM_COMMAND	- 处理应用程序菜单
//  WM_PAINT	- 绘制主窗口
//  WM_DESTROY	- 发送退出消息并返回
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC hdc;

	switch (message)
	{
	case WM_COMMAND:
		wmId    = LOWORD(wParam);
		wmEvent = HIWORD(wParam);
		// 分析菜单选择:
		switch (wmId)
		{
		case IDM_ABOUT:
			DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
			break;
		case IDM_EXIT:
			DestroyWindow(hWnd);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		break;
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		// TODO: 在此添加任意绘图代码...
		EndPaint(hWnd, &ps);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

// “关于”框的消息处理程序。
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}
