// pngDropCircleTest.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "pngDropCircleTest.h"


// #include <comdef.h>  
// #include <gdiplus.h>
// #pragma comment(lib, "gdiplus.lib")
// using namespace Gdiplus;

#include <windows.h>
#include <gdiplus.h>
//#include "GdiplusHeaders.h"
using namespace Gdiplus;
#pragma comment (lib, "gdiplus.lib")

ULONG_PTR m_gdiplusToken;

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;								// current instance
TCHAR szTitle[MAX_LOADSTRING];					// The title bar text
TCHAR szWindowClass[MAX_LOADSTRING];			// the main window class name

// Forward declarations of functions included in this code module:
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);

int APIENTRY _tWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPTSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

 	// TODO: Place code here.
	MSG msg;
	HACCEL hAccelTable;

	// Initialize global strings
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_PNGDROPCIRCLETEST, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// Perform application initialization:
	if (!InitInstance (hInstance, nCmdShow))
	{
		return FALSE;
	}

	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_PNGDROPCIRCLETEST));

	// Main message loop:
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
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
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
	wcex.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_PNGDROPCIRCLETEST));
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= MAKEINTRESOURCE(IDC_PNGDROPCIRCLETEST);
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassEx(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   HWND hWnd;

   hInst = hInstance; // Store instance handle in our global variable

   hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);

   if (!hWnd)
   {
      return FALSE;
   }

   Gdiplus::GdiplusStartupInput gdiplusStartupInput;
   Gdiplus::GdiplusStartup(&m_gdiplusToken, &gdiplusStartupInput, NULL);

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

int GetEncoderClsid(const WCHAR* format, CLSID* pClsid)
{
	UINT  num = 0;          // number of image encoders
	UINT  size = 0;         // size of the image encoder array
	// in bytes

	ImageCodecInfo* pImageCodecInfo = NULL;

	GetImageEncodersSize(&num, &size);
	if (size == 0)
		return -1;

	pImageCodecInfo = (ImageCodecInfo*)(malloc(size));
	if (pImageCodecInfo == NULL)
		return -1;

	GetImageEncoders(num, size, pImageCodecInfo);
	for (UINT j = 0; j < num; ++j)
	{
		if (wcscmp(pImageCodecInfo[j].MimeType, format) == 0)
		{
			*pClsid = pImageCodecInfo[j].Clsid;
			free(pImageCodecInfo);
			return j;  // Success
		}
	}

	free(pImageCodecInfo);
	return -1;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND	- process the application menu
//  WM_PAINT	- Paint the main window
//  WM_DESTROY	- post a quit message and return
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
		// Parse the menu selections:
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
		// TODO: Add any drawing code here...

		{
			//Bitmap *pOldBmp = Bitmap::FromFile(L"d:\\透明图片\\62708510_12.png");
			Bitmap *pOldBmp = Bitmap::FromFile(L"d:\\透明图片\\mini.jpg");
			int newWidth = pOldBmp->GetWidth() - 100;
			int newHeight = pOldBmp->GetHeight() - 100;
			RectF bmpRect(0.0f, 0.0f, newWidth, newHeight);

			//Bitmap *pCloneBmp = pOldBmp->Clone(0, 0, newWidth, newHeight, pOldBmp->GetPixelFormat());
			//////////////////////////////////////////////////////////////////////////
			// PixelFormat32bppARGB 设置任何图片格式拷贝为32为带Alpha channel的透明图片格式
			//
			//////////////////////////////////////////////////////////////////////////
			Bitmap *pCloneBmp = pOldBmp->Clone(0, 0, newWidth, newHeight, PixelFormat32bppARGB);	
// 			{
// 				INT iWidth = pCloneBmp->GetWidth();
// 				INT iHeight = pCloneBmp->GetHeight();
// 				Color color, colorTemp;
// 				for (INT iRow = 0; iRow < iHeight; iRow++)
// 				{
// 					for (INT iColumn = 0; iColumn < iWidth; iColumn++)
// 					{
// 						pCloneBmp->GetPixel(iColumn, iRow, &color);
// 						colorTemp.SetValue(color.MakeARGB(
// 							(BYTE)(255 * iColumn / iWidth),
// 							color.GetRed(),
// 							color.GetGreen(),
// 							color.GetBlue()));
// 						pCloneBmp->SetPixel(iColumn, iRow, colorTemp);
// 					}
// 				}
// 			}
			Graphics *pGraphics = Graphics::FromImage(pCloneBmp);

			Gdiplus::Font GdiPlusFont(L"Times new roman", 16);
			Gdiplus::SolidBrush blackBrush(Gdiplus::Color(255, 0, 0, 0));
			StringFormat format;
			format.SetAlignment(StringAlignmentNear);
			pGraphics->DrawString(L"Test String", 15, &GdiPlusFont, PointF(9, 4), &format, &blackBrush);
			UINT num, size;
			ImageCodecInfo * pImageCodecInfo;
			GetImageEncodersSize(&num, &size);
			pImageCodecInfo = (ImageCodecInfo*)(malloc(size));
			GetImageEncoders(num, size, pImageCodecInfo);
			CLSID encoderClsid;
			GetEncoderClsid(L"image/png", &encoderClsid);	// 需要自己定义

			GraphicsPath clipPath;
			clipPath.AddEllipse(0, 0, 200, 100);

			// Set the clipping region with hRegion.
			pGraphics->SetClip(&clipPath, CombineModeExclude);

			//pGraphics->SetCompositingMode(CompositingModeSourceOver);		// 默认方式
			pGraphics->SetCompositingMode(CompositingModeSourceCopy); //设置组合方式
			pGraphics->Clear(Color::Transparent);
			pGraphics->FillRectangle(&SolidBrush(Color(0, 255, 255, 255)), 0, 0, 500, 500);


			//
			// DrawImage时添加此属性，可以将固定点变为透明色，类似TransparentBlt
			// 如： pDrawGraphics->DrawImage(pCloneBmp, bmpRect, 0, 0, newWidth,newHeight, UnitPixel, &imAtt);
			//

			//ImageAttributes imAtt;
			//imAtt.SetColorKey(Color(255, 255, 255), Color(255, 255, 255), ColorAdjustTypeBitmap);

			// Initialize the color matrix.
			// Notice the value 0.8 in row 4, column 4.
// 			ColorMatrix colorMatrix = { 1.0f, 0.0f, 0.0f, 0.0f, 0.0f,
// 				0.0f, 1.0f, 0.0f, 0.0f, 0.0f,
// 				0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
// 				0.0f, 0.0f, 0.0f, 0.8f, 0.0f,
// 				0.0f, 0.0f, 0.0f, 0.0f, 1.0f };
// 			// Create an ImageAttributes object and set its color matrix.
// 			ImageAttributes imageAtt;
// 			imageAtt.SetColorMatrix(&colorMatrix, ColorMatrixFlagsDefault,
// 				ColorAdjustTypeBitmap);

			Graphics *pDrawGraphics = new Graphics(hdc);
			pDrawGraphics->DrawImage(pCloneBmp, bmpRect, 0, 0, newWidth,
				newHeight, UnitPixel);


			pCloneBmp->Save(L"CroppedBmp.png", &encoderClsid);

			delete pCloneBmp;
			delete pOldBmp;
			delete pGraphics;
			free(pImageCodecInfo);
		}

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

// Message handler for about box.
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
