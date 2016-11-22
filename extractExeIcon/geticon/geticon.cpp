// geticon.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "atlimage.h"

// int _tmain(int argc, _TCHAR* argv[])
// {
// 	return 0;
// }

#include <windows.h>
#include <Shellapi.h>
#include <stdio.h>
#include <tchar.h>


//
// ICONS (.ICO type 1) are structured like this:
//
// ICONHEADER (just 1)
// ICONDIR [1...n] (an array, 1 for each image)
// [BITMAPINFOHEADER+COLOR_BITS+MASK_BITS] [1...n] (1 after the other, for each image)
//
// CURSORS (.ICO type 2) are identical in structure, but use
// two monochrome bitmaps (real XOR and AND masks, this time).
//

typedef struct
{
	WORD idReserved; // must be 0
	WORD idType; // 1 = ICON, 2 = CURSOR
	WORD idCount; // number of images (and ICONDIRs)

	// ICONDIR [1...n]
	// ICONIMAGE [1...n]

} ICONHEADER;

//
// An array of ICONDIRs immediately follow the ICONHEADER
//
typedef struct
{
	BYTE bWidth;
	BYTE bHeight;
	BYTE bColorCount;
	BYTE bReserved;
	WORD wPlanes; // for cursors, this field = wXHotSpot
	WORD wBitCount; // for cursors, this field = wYHotSpot
	DWORD dwBytesInRes;
	DWORD dwImageOffset; // file-offset to the start of ICONIMAGE

} ICONDIR;

//
// After the ICONDIRs follow the ICONIMAGE structures -
// consisting of a BITMAPINFOHEADER, (optional) RGBQUAD array, then
// the color and mask bitmap bits (all packed together
//
typedef struct
{
	BITMAPINFOHEADER biHeader; // header for color bitmap (no mask header)
	//RGBQUAD rgbColors[1...n];
	//BYTE bXOR[1]; // DIB bits for color bitmap
	//BYTE bAND[1]; // DIB bits for mask bitmap

} ICONIMAGE;

//
// Write the ICO header to disk
//
static UINT WriteIconHeader(HANDLE hFile, int nImages)
{
	ICONHEADER iconheader;
	DWORD nWritten;

	// Setup the icon header
	iconheader.idReserved = 0; // Must be 0
	iconheader.idType = 1; // Type 1 = ICON (type 2 = CURSOR)
	iconheader.idCount = nImages; // number of ICONDIRs

	// Write the header to disk
	WriteFile( hFile, &iconheader, sizeof(iconheader), &nWritten, 0);

	// following ICONHEADER is a series of ICONDIR structures (idCount of them, in fact)
	return nWritten;
}

//
// Return the number of BYTES the bitmap will take ON DISK
//
static UINT NumBitmapBytes(BITMAP *pBitmap)
{
	int nWidthBytes = pBitmap->bmWidthBytes;

	// bitmap scanlines MUST be a multiple of 4 bytes when stored
	// inside a bitmap resource, so round up if necessary
	if(nWidthBytes & 3)
		nWidthBytes = (nWidthBytes + 4) & ~3;

	return nWidthBytes * pBitmap->bmHeight;
}

//
// Return number of bytes written
//
static UINT WriteIconImageHeader(HANDLE hFile, BITMAP *pbmpColor, BITMAP *pbmpMask)
{
	BITMAPINFOHEADER biHeader;
	DWORD nWritten;
	UINT nImageBytes;

	// calculate how much space the COLOR and MASK bitmaps take
	nImageBytes = NumBitmapBytes(pbmpColor) + NumBitmapBytes(pbmpMask);

	// write the ICONIMAGE to disk (first the BITMAPINFOHEADER)
	ZeroMemory(&biHeader, sizeof(biHeader));

	// Fill in only those fields that are necessary
	biHeader.biSize = sizeof(biHeader);
	biHeader.biWidth = pbmpColor->bmWidth;
	biHeader.biHeight = pbmpColor->bmHeight * 2; // height of color+mono
	biHeader.biPlanes = pbmpColor->bmPlanes;
	biHeader.biBitCount = pbmpColor->bmBitsPixel;
	biHeader.biSizeImage = nImageBytes;

	// write the BITMAPINFOHEADER
	WriteFile(hFile, &biHeader, sizeof(biHeader), &nWritten, 0);

	// write the RGBQUAD color table (for 16 and 256 colour icons)
	if(pbmpColor->bmBitsPixel == 2 || pbmpColor->bmBitsPixel == 8)
	{

	}

	return nWritten;
}

//
// Wrapper around GetIconInfo and GetObject(BITMAP)
//
static BOOL GetIconBitmapInfo(HICON hIcon, ICONINFO *pIconInfo, BITMAP *pbmpColor, BITMAP *pbmpMask)
{
	if(!GetIconInfo(hIcon, pIconInfo))
		return FALSE;

	if(!GetObject(pIconInfo->hbmColor, sizeof(BITMAP), pbmpColor))
		return FALSE;

	if(!GetObject(pIconInfo->hbmMask, sizeof(BITMAP), pbmpMask))
		return FALSE;

	return TRUE;
}

//
// Write one icon directory entry - specify the index of the image
//
static UINT WriteIconDirectoryEntry(HANDLE hFile, int nIdx, HICON hIcon, UINT nImageOffset)
{
	ICONINFO iconInfo;
	ICONDIR iconDir;

	BITMAP bmpColor;
	BITMAP bmpMask;

	DWORD nWritten;
	UINT nColorCount;
	UINT nImageBytes;

	GetIconBitmapInfo(hIcon, &iconInfo, &bmpColor, &bmpMask);

	nImageBytes = NumBitmapBytes(&bmpColor) + NumBitmapBytes(&bmpMask);

	if(bmpColor.bmBitsPixel >= 8)
		nColorCount = 0;
	else
		nColorCount = 1 << (bmpColor.bmBitsPixel * bmpColor.bmPlanes);

	// Create the ICONDIR structure
	iconDir.bWidth = (BYTE)bmpColor.bmWidth;
	iconDir.bHeight = (BYTE)bmpColor.bmHeight;
	iconDir.bColorCount = nColorCount;
	iconDir.bReserved = 0;
	iconDir.wPlanes = bmpColor.bmPlanes;
	iconDir.wBitCount = bmpColor.bmBitsPixel;
	iconDir.dwBytesInRes = sizeof(BITMAPINFOHEADER) + nImageBytes;
	iconDir.dwImageOffset = nImageOffset;

	// Write to disk
	WriteFile(hFile, &iconDir, sizeof(iconDir), &nWritten, 0);

	// Free resources
	DeleteObject(iconInfo.hbmColor);
	DeleteObject(iconInfo.hbmMask);

	return nWritten;
}

static UINT WriteIconData(HANDLE hFile, HBITMAP hBitmap)
{
	BITMAP bmp;
	int i;
	BYTE * pIconData;

	UINT nBitmapBytes;
	DWORD nWritten;

	GetObject(hBitmap, sizeof(BITMAP), &bmp);

	nBitmapBytes = NumBitmapBytes(&bmp);

	pIconData = (BYTE *)malloc(nBitmapBytes);

	GetBitmapBits(hBitmap, nBitmapBytes, pIconData);

	// bitmaps are stored inverted (vertically) when on disk..
	// so write out each line in turn, starting at the bottom + working
	// towards the top of the bitmap. Also, the bitmaps are stored in packed
	// in memory - scanlines are NOT 32bit aligned, just 1-after-the-other
	for(i = bmp.bmHeight - 1; i >= 0; i--)
	{
		// Write the bitmap scanline
		WriteFile(
			hFile,
			pIconData + (i * bmp.bmWidthBytes), // calculate offset to the line
			bmp.bmWidthBytes, // 1 line of BYTES
			&nWritten,
			0);

		// extend to a 32bit boundary (in the file) if necessary
		if(bmp.bmWidthBytes & 3)
		{
			DWORD padding = 0;
			WriteFile(hFile, &padding, 4 - bmp.bmWidthBytes, &nWritten, 0);
		}
	}

	free(pIconData);

	return nBitmapBytes;
}

//////////////////////////////////////////////////////////////////////////
HBITMAP BitmapFromIcon(HICON hIcon);
bool SaveBitmapToFile(HBITMAP hBitmap);
BOOL SaveToFile(HBITMAP hBitmap, LPCTSTR lpszFileName);
HBITMAP icon_to_bitmap(HICON Icon_Handle);
//////////////////////////////////////////////////////////////////////////

//
// Create a .ICO file, using the specified array of HICON images
//
BOOL SaveIcon3(TCHAR *szIconFile, HICON hIcon[], int nNumIcons)
{
	HANDLE hFile;
	int i;
	int * pImageOffset;

	if(hIcon == 0 || nNumIcons < 1)
		return FALSE;

	// Save icon to disk:
	hFile = CreateFile(szIconFile, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, 0, 0);

	if(hFile == INVALID_HANDLE_VALUE)
		return FALSE;

	//
	// Write the iconheader first of all
	//
	WriteIconHeader(hFile, nNumIcons);

	//
	// Leave space for the IconDir entries
	//
	SetFilePointer(hFile, sizeof(ICONDIR) * nNumIcons, 0, FILE_CURRENT);

	pImageOffset = (int *)malloc(nNumIcons * sizeof(int));

	//
	// Now write the actual icon images!
	//
	for(i = 0; i < nNumIcons; i++)
	{
		ICONINFO iconInfo;
		BITMAP bmpColor, bmpMask;

		GetIconBitmapInfo(hIcon[i], &iconInfo, &bmpColor, &bmpMask);

		// record the file-offset of the icon image for when we write the icon directories
		pImageOffset[i] = SetFilePointer(hFile, 0, 0, FILE_CURRENT);

		// bitmapinfoheader + colortable
		WriteIconImageHeader(hFile, &bmpColor, &bmpMask);

		// color and mask bitmaps
		WriteIconData(hFile, iconInfo.hbmColor);
		WriteIconData(hFile, iconInfo.hbmMask);

		DeleteObject(iconInfo.hbmColor);
		DeleteObject(iconInfo.hbmMask);
	}

	//
	// Lastly, skip back and write the icon directories.
	//
	SetFilePointer(hFile, sizeof(ICONHEADER), 0, FILE_BEGIN);

	for(i = 0; i < nNumIcons; i++)
	{
		WriteIconDirectoryEntry(hFile, i, hIcon[i], pImageOffset[i]);
	}

	free(pImageOffset);

	// finished!
	CloseHandle(hFile);

	return TRUE;
}

HBITMAP TransparentImage(HBITMAP hBitmap)
{
	CImage Image;
	Image.Attach(hBitmap);
	int nPitch = Image.GetPitch( ) , nBPP = Image.GetBPP( );
	LPBYTE lpBits = reinterpret_cast< LPBYTE>( Image.GetBits( ) );

	for( int yPos = 0 ; yPos < Image.GetHeight( ) ; yPos ++ )
	{
		LPBYTE lpBytes = lpBits + ( yPos * nPitch );
		PDWORD lpLines = reinterpret_cast< PDWORD >( lpBytes );
		for( int xPos = 0 ; xPos < Image.GetWidth( ) ; xPos ++ )
		{
			if( nBPP == 32 && lpLines[ xPos ] >> 24 != 0x000000FF)
			{
				lpLines[ xPos ] |= 0xFFFFFFFF ;
			}
		}
	}

	return Image.Detach();
}

int saveIcon(TCHAR* filename, TCHAR* iconFile) {
	HICON hIconLarge;
	HICON hIconSmall;
	BOOL ret;


	if ( ExtractIconEx(filename, 0, &hIconLarge, &hIconSmall, 1) == 0 ) {
		return 1;
	}

	//////////////////////////////////////////////////////////////////////////
	//HBITMAP  hBmp = BitmapFromIcon(hIconLarge);
	//HBITMAP  hBmp = icon_to_bitmap(hIconLarge);

// 	{
// 		CImage cImage;
// 		//..
// 		HBITMAP hBmp = cImage; // cImage is still the data owner
// 		//..
// 		hBmp = NULL;
// 		//..
// 		hBmp = cImage.Detach(); // cImage is "empty" now
// 		//..
// 		::DeleteObject(hBmp);
// 	}

	CImage Image;
	//..
	HBITMAP hBmp = Image; // cImage is still the data owner
	//..
	Image.Load(_T("d:\\ss.ico"));

	int nPitch = Image.GetPitch( ) , nBPP = Image.GetBPP( );
	LPBYTE lpBits = reinterpret_cast< LPBYTE>( Image.GetBits( ) );

	for( int yPos = 0 ; yPos < Image.GetHeight( ) ; yPos ++ )
	{
		LPBYTE lpBytes = lpBits + ( yPos * nPitch );
		PDWORD lpLines = reinterpret_cast< PDWORD >( lpBytes );
		for( int xPos = 0 ; xPos < Image.GetWidth( ) ; xPos ++ )
		{
			if( nBPP == 32 && lpLines[ xPos ] >> 24 != 0x000000FF)
			{
				lpLines[ xPos ] |= 0xFFFFFFFF ;
			}
		}
	}

	Image.Save(_T("D:\\sss.bmp"));
	hBmp = NULL;
	//..
	hBmp = Image.Detach(); // cImage is "empty" now
	//..
	SaveToFile(hBmp, _T("D:\\ss.bmp"));
	::DeleteObject(hBmp);


	
	//////////////////////////////////////////////////////////////////////////

	ret = SaveIcon3(iconFile, &hIconLarge, 1);
	if ( ret ) {
		return 0;
	}
	return -1;
}

/////////////////////////

HBITMAP icon_to_bitmap(HICON Icon_Handle) {
	HDC Screen_Handle = GetDC(NULL);
	HDC Device_Handle = CreateCompatibleDC(Screen_Handle);

	HBITMAP Bitmap_Handle = 
		CreateCompatibleBitmap(Device_Handle,GetSystemMetrics(SM_CXICON),
		GetSystemMetrics(SM_CYICON));

	HBITMAP Old_Bitmap = (HBITMAP)SelectObject(Device_Handle,Bitmap_Handle);
	DrawIcon(Device_Handle, 0,0, Icon_Handle);
	SelectObject(Device_Handle,Old_Bitmap);

	DeleteDC(Device_Handle);
	ReleaseDC(NULL,Screen_Handle);
	return Bitmap_Handle;

// 	int x= 32;
// 	int y = 32;
// 
// 	HDC hDC = GetDC(NULL);
// 	HDC hMemDC = CreateCompatibleDC(hDC);
// 	HBITMAP hMemBmp = CreateCompatibleBitmap(hDC, x, y);
// 	HBITMAP hResultBmp = NULL;
// 	HGDIOBJ hOrgBMP = SelectObject(hMemDC, hMemBmp);
// 
// 	DrawIconEx(hMemDC, 0, 0, hIcon, x, y, 0, NULL, DI_NORMAL);
// 
// 	hResultBmp = hMemBmp;
// 	hMemBmp = NULL;
// 
// 	SelectObject(hMemDC, hOrgBMP);
// 	DeleteDC(hMemDC);
// 	ReleaseDC(NULL, hDC);
// 	DestroyIcon(hIcon);
// 	return hResultBmp;
}

HBITMAP BitmapFromIcon(HICON hIcon)
{
	HDC hDC = CreateCompatibleDC(NULL);
	HBITMAP hBitmap = CreateCompatibleBitmap(hDC, GetSystemMetrics(SM_CXICON), GetSystemMetrics(SM_CYICON));
	HBITMAP hOldBitmap = (HBITMAP)SelectObject(hDC, hBitmap);
	DrawIcon(hDC, 0, 0, hIcon);
	SelectObject(hDC, hOldBitmap);
	DeleteDC(hDC);

	return hBitmap;
}

bool SaveBitmapToFile(HBITMAP hBitmap)
{
	HDC hDC;         
	//设备描述表
	int iBits;
	//当前显示分辨率下每个像素所占字节数
	WORD wBitCount;   
	//位图中每个像素所占字节数
	//定义调色板大小， 位图中像素字节大小 ，  位图文件大小 ， 写入文件字节数
	DWORD  dwPaletteSize=0,dwBmBitsSize,dwDIBSize, dwWritten;
	BITMAP  Bitmap;
	//位图属性结构
	BITMAPFILEHEADER   bmfHdr;
	//位图文件头结构
	BITMAPINFOHEADER   bi;
	//位图信息头结构
	LPBITMAPINFOHEADER lpbi;
	//指向位图信息头结构
	HANDLE  fh, hDib, hPal;
	HPALETTE  hOldPal=NULL;
	//定义文件，分配内存句柄，调色板句柄
	//计算位图文件每个像素所占字节数
	hDC = CreateDC(_T("DISPLAY"),NULL,NULL,NULL);
	iBits = GetDeviceCaps(hDC, BITSPIXEL) * GetDeviceCaps(hDC, PLANES);
	DeleteDC(hDC);
	if (iBits <= 1)
		wBitCount = 1;
	else if (iBits <= 4)
		wBitCount = 4;
	else if (iBits <= 8)
		wBitCount = 8;
	else if (iBits <= 24)
		wBitCount = 24;
	else
		wBitCount = 32;
	//计算调色板大小
	if (wBitCount <= 8)
		dwPaletteSize=(1<<wBitCount)*sizeof(RGBQUAD);
	//设置位图信息头结构
	GetObject(hBitmap, sizeof(BITMAP), (LPSTR)&Bitmap);
	bi.biSize              = sizeof(BITMAPINFOHEADER);
	bi.biWidth             = Bitmap.bmWidth;
	bi.biHeight            = Bitmap.bmHeight;
	bi.biPlanes            = 1;
	bi.biBitCount          = wBitCount;
	bi.biCompression       = BI_RGB;
	bi.biSizeImage         = 0;
	bi.biXPelsPerMeter     = 0;
	bi.biYPelsPerMeter     = 0;
	bi.biClrUsed           = 0;
	bi.biClrImportant      = 0;
	dwBmBitsSize = ((Bitmap.bmWidth*wBitCount+31)/32)*4*Bitmap.bmHeight;

	//为位图内容分配内存
	hDib  = GlobalAlloc(GHND,dwBmBitsSize+dwPaletteSize+sizeof(BITMAPINFOHEADER));
	lpbi = (LPBITMAPINFOHEADER)GlobalLock(hDib);
	*lpbi = bi;
	// 处理调色板
	hPal = GetStockObject(DEFAULT_PALETTE);
	if (hPal)
	{
		hDC = ::GetDC(NULL);
		hOldPal=SelectPalette(hDC,(HPALETTE)hPal,FALSE);
		RealizePalette(hDC);
	}
	// 获取该调色板下新的像素值
	GetDIBits(hDC,hBitmap,0,(UINT)Bitmap.bmHeight,(LPSTR)lpbi+sizeof(BITMAPINFOHEADER)+dwPaletteSize, (BITMAPINFO *)lpbi,DIB_RGB_COLORS);
	//恢复调色板   
	if (hOldPal)
	{
		SelectPalette(hDC, hOldPal, TRUE);
		RealizePalette(hDC);
		::ReleaseDC(NULL, hDC);
	}

	//创建位图文件    
	fh=CreateFile(_T("test2012.bmp"), GENERIC_WRITE,0, NULL, CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL);
	if (fh==INVALID_HANDLE_VALUE)
		return FALSE;

	//设置位图文件头
	bmfHdr.bfType = 0x4D42;  // "BM"
	dwDIBSize=sizeof(BITMAPFILEHEADER)+sizeof(BITMAPINFOHEADER)+dwPaletteSize+dwBmBitsSize;  
	bmfHdr.bfSize = dwDIBSize;
	bmfHdr.bfReserved1 = 0;
	bmfHdr.bfReserved2 = 0;
	bmfHdr.bfOffBits = (DWORD)sizeof(BITMAPFILEHEADER)+(DWORD)sizeof(BITMAPINFOHEADER)+dwPaletteSize;

	WriteFile(fh, (LPSTR)&bmfHdr, sizeof(BITMAPFILEHEADER), &dwWritten, NULL);
	WriteFile(fh, (LPSTR)lpbi, sizeof(BITMAPINFOHEADER)+dwPaletteSize+dwBmBitsSize , &dwWritten, NULL);

	GlobalUnlock(hDib);
	GlobalFree(hDib);

	CloseHandle(fh);
	return false;
}

BOOL SaveToFile(HBITMAP hBitmap, LPCTSTR lpszFileName)
{
	HDC hDC;

	int iBits;

	WORD wBitCount;

	DWORD dwPaletteSize=0, dwBmBitsSize=0, dwDIBSize=0, dwWritten=0;

	BITMAP Bitmap;

	BITMAPFILEHEADER bmfHdr;

	BITMAPINFOHEADER bi;

	LPBITMAPINFOHEADER lpbi;

	HANDLE fh, hDib, hPal,hOldPal=NULL;

	hDC = CreateDC(_T("DISPLAY"), NULL, NULL, NULL);
	iBits = GetDeviceCaps(hDC, BITSPIXEL) * GetDeviceCaps(hDC, PLANES);
	DeleteDC(hDC);
	if (iBits <= 1)
		wBitCount = 1;
	else if (iBits <= 4)
		wBitCount = 4;
	else if (iBits <= 8)
		wBitCount = 8;
	else
		wBitCount = 24;
	GetObject(hBitmap, sizeof(Bitmap), (LPSTR)&Bitmap);
	bi.biSize = sizeof(BITMAPINFOHEADER);
	bi.biWidth = Bitmap.bmWidth;
	bi.biHeight = Bitmap.bmHeight;
	bi.biPlanes = 1;
	bi.biBitCount = wBitCount;
	bi.biCompression = BI_RGB;
	bi.biSizeImage = 0;
	bi.biXPelsPerMeter = 0;
	bi.biYPelsPerMeter = 0;
	bi.biClrImportant = 0;
	bi.biClrUsed = 0;
	dwBmBitsSize = ((Bitmap.bmWidth * wBitCount + 31) / 32) * 4 * Bitmap.bmHeight;

	hDib = GlobalAlloc(GHND,dwBmBitsSize + dwPaletteSize + sizeof(BITMAPINFOHEADER));
	lpbi = (LPBITMAPINFOHEADER)GlobalLock(hDib);
	*lpbi = bi;

	//hPal = GetStockObject(DEFAULT_PALETTE);
	hPal = GetStockObject(WHITE_BRUSH);
	if (hPal)
	{ 
		hDC = GetDC(NULL);
		hOldPal = SelectPalette(hDC, (HPALETTE)hPal, FALSE);
		RealizePalette(hDC);
	}


	GetDIBits(hDC, hBitmap, 0, (UINT) Bitmap.bmHeight, (LPSTR)lpbi + sizeof(BITMAPINFOHEADER) 
		+dwPaletteSize, (BITMAPINFO *)lpbi, DIB_RGB_COLORS);

	if (hOldPal)
	{
		SelectPalette(hDC, (HPALETTE)hOldPal, TRUE);
		RealizePalette(hDC);
		ReleaseDC(NULL, hDC);
	}

	fh = CreateFile(lpszFileName, GENERIC_WRITE,0, NULL, CREATE_ALWAYS, 
		FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL); 

	if (fh == INVALID_HANDLE_VALUE)
		return FALSE; 

	bmfHdr.bfType = 0x4D42; // "BM"
	dwDIBSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + dwPaletteSize + dwBmBitsSize;
	bmfHdr.bfSize = dwDIBSize;
	bmfHdr.bfReserved1 = 0;
	bmfHdr.bfReserved2 = 0;
	bmfHdr.bfOffBits = (DWORD)sizeof(BITMAPFILEHEADER) + (DWORD)sizeof(BITMAPINFOHEADER) + dwPaletteSize;

	WriteFile(fh, (LPSTR)&bmfHdr, sizeof(BITMAPFILEHEADER), &dwWritten, NULL);

	WriteFile(fh, (LPSTR)lpbi, dwDIBSize, &dwWritten, NULL);
	GlobalUnlock(hDib);
	GlobalFree(hDib);
	CloseHandle(fh);
	return TRUE;
}

int _tmain(int argc, TCHAR* argv[]) {
	if ( argc < 3 ) {
		printf("Usage: <exe/dll file> <output ico file>");
		return EXIT_FAILURE;
	}
	_tprintf(_T("src = %s\n"), argv[1]);
	_tprintf(_T("dest = %s\n"), argv[2]);
	saveIcon(argv[1], argv[2]);

	return 0;
}