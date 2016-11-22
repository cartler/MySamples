
#include <time.h>

static const TCHAR* const_szBmpSaveDir = _T("d:\\BMPS\\");

static void WriteBmpTofile(/*const bool remote, */LPCTSTR pszFile, PBITMAPINFO pbi, HBITMAP hBmp, HDC hDC)
{
	HANDLE hFile;
	BITMAPFILEHEADER hdr;
	PBITMAPINFOHEADER pbih;
	LPBYTE lpBits;
	DWORD dwTemp;

	pbih = (PBITMAPINFOHEADER)pbi;
	lpBits = (LPBYTE) GlobalAlloc(GMEM_FIXED, pbih->biSizeImage);

	if(!lpBits)
	{
		return; // could not allocate bitmap
	}

	GetDIBits(hDC, hBmp, 0, (WORD)pbih->biHeight, lpBits, pbi, DIB_RGB_COLORS);

	hFile = CreateFile(pszFile,
		GENERIC_READ | GENERIC_WRITE,
		0,
		NULL,
		CREATE_ALWAYS,
		FILE_ATTRIBUTE_NORMAL,
		NULL);

	if(hFile == INVALID_HANDLE_VALUE)
	{
		return; // Could not open screenshot file
	}

	// type == BM
	hdr.bfType = 0x4d42;

	hdr.bfSize = (sizeof(BITMAPFILEHEADER) + pbih->biSize + pbih->biClrUsed * sizeof(RGBQUAD) + pbih->biSizeImage);
	hdr.bfReserved1 = 0;
	hdr.bfReserved2 = 0;

	hdr.bfOffBits = sizeof(BITMAPFILEHEADER) + pbih->biSize + pbih->biClrUsed * sizeof(RGBQUAD);

	// write the bitmap file header to file
	WriteFile(hFile, (LPVOID)&hdr, sizeof(BITMAPFILEHEADER), &dwTemp, NULL);

	// write the bitmap header to file
	WriteFile(hFile, (LPVOID)pbih, sizeof(BITMAPINFOHEADER) + pbih->biClrUsed * sizeof(RGBQUAD), &dwTemp, NULL);

	// copy the bitmap colour data into the file
	WriteFile(hFile, (LPSTR)lpBits, pbih->biSizeImage, &dwTemp, NULL);

	CloseHandle(hFile);

	GlobalFree((HGLOBAL)lpBits);
}


//
static HBITMAP DcToHBITMAP(const HDC hDcToSave, const HWND hWndOfDc = NULL)
{
	int nWidth = 0;
	int nHeight = 0;
	if (hWndOfDc != NULL)
	{
		RECT rcClient = { 0 };
		::GetClientRect(hWndOfDc, &rcClient);
		nWidth = rcClient.right-rcClient.left;
		nHeight = rcClient.bottom-rcClient.top;
	}
	else
	{
		HWND hWnd = WindowFromDC(hDcToSave);
		if (hWnd)
		{
			RECT rcClient = { 0 };
			GetClientRect(hWnd, &rcClient);
			nWidth = rcClient.right-rcClient.left;
			nHeight = rcClient.bottom-rcClient.top;
		}
		else
		{
			nWidth = GetDeviceCaps(hDcToSave, HORZRES);
			nHeight = GetDeviceCaps(hDcToSave, VERTRES);
		}
	}
		
	// then you might have: 

	//Create a new DC compatible with the source DC. Call this the memory DC.
	HDC hDcSave = ::CreateCompatibleDC(hDcToSave);
	//Create a new bitmap of the correct size.
	HBITMAP hbmpSave = ::CreateCompatibleBitmap(hDcToSave, nWidth, nHeight); 
	//Select the bitmap into the memory DC.
	HBITMAP hOldBitmap = (HBITMAP)::SelectObject(hDcSave, hbmpSave);
	//BitBlt the source DC into the memory DC.
	::BitBlt(hDcSave, 0, 0, nWidth, nHeight, hDcToSave, 0, 0, SRCCOPY);
	//The bitmap should now contain a copy of the source DC.
	hbmpSave = (HBITMAP)::SelectObject(hDcSave, hOldBitmap);

	//Clean up
	DeleteDC(hDcSave);

	return hbmpSave;
}
//

static PBITMAPINFO CreateBitmapInfo(HBITMAP hBmp)
{
	BITMAP bmp;
	PBITMAPINFO pbmi;

	GetObject(hBmp, sizeof(BITMAP), &bmp);

	pbmi = static_cast<PBITMAPINFO>(LocalAlloc(LPTR, sizeof(BITMAPINFOHEADER)));

	pbmi->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	pbmi->bmiHeader.biWidth = bmp.bmWidth;
	pbmi->bmiHeader.biHeight = bmp.bmHeight;
	pbmi->bmiHeader.biPlanes = bmp.bmPlanes; // we are assuming that there is only one plane
	pbmi->bmiHeader.biBitCount = bmp.bmBitsPixel;

	// no compression this is an rgb bitmap
	pbmi->bmiHeader.biCompression = BI_RGB;

	// calculate size and align to a DWORD (8bit), we are assuming there is only one plane.
	pbmi->bmiHeader.biSizeImage = ((pbmi->bmiHeader.biWidth * bmp.bmBitsPixel +31) & -31) * pbmi->bmiHeader.biHeight;

	// all device colours are important
	pbmi->bmiHeader.biClrImportant = 0;

	return pbmi;
}

static BOOL GenerateFileName(LPTSTR lpstFileName, int nLength)
{
	if (nLength < 256)
	{
		return FALSE;
	}

	TCHAR szTimestamp[256] = {0};
	// get complete time
	time_t	tCurrentTime = time(NULL);
	_tcsftime(szTimestamp, 20, _T("%Y-%m-%d_%H-%M-%S"), localtime(&tCurrentTime));
	TCHAR szTickcount[256] = {0};
	//_stprintf(szTickcount, _T("%ld"), GetTickCount());
	wsprintf(szTickcount, _T("%ld"), GetTickCount());
	//_stprintf(lpstFileName, _T("%sDC_%s_%s.bmp"), const_szBmpSaveDir, szTimestamp, szTickcount);
	wsprintf(lpstFileName, _T("%sDC_%s_%s.bmp"), const_szBmpSaveDir, szTimestamp, szTickcount);

	//_tcsftime(buff, 255, strFormat, localtime(&time));
	return TRUE;
}

static void DumpDCToBMP( const HDC hDcToSave, const HWND hWndOfDc = NULL, LPCTSTR lpstFilePath = NULL )
{
	HBITMAP hBmpSave = DcToHBITMAP(hDcToSave, hWndOfDc);
	PBITMAPINFO pBmpInfo = CreateBitmapInfo(hBmpSave);
	if (lpstFilePath == NULL)
	{
		TCHAR szFilePath[MAX_PATH] = {0};
		GenerateFileName(szFilePath, MAX_PATH-1);
		WriteBmpTofile(szFilePath, pBmpInfo, hBmpSave, hDcToSave);
	}
	else
	{
		WriteBmpTofile(lpstFilePath, pBmpInfo, hBmpSave, hDcToSave);
	}

	return;
}