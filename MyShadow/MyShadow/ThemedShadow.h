/////////////////////////////////////////////////////////////////////
// 
#ifndef _THEMEDSHADOWWND_H_INCLUDE_
#define _THEMEDSHADOWWND_H_INCLUDE_
#pragma once
#include <map>
#include <math.h>
#include "atlstr.h"
using namespace std;


/////////////////////////////////////////////////////////////////////
// Defined class for GDI Plus Path.
class CPathHelper
{
	// Constructor and destructor.
public:
	CPathHelper() : m_pPath(NULL)
	{
	}

	~CPathHelper()
	{
		DeleteOldPath();
	}

	// Data members.
public:
	GraphicsPath*	m_pPath;

	// Operations.
public:
	// Delete old path.
	void DeleteOldPath()
	{
		if (m_pPath != NULL)
		{
			delete m_pPath;
			m_pPath = NULL;
		}
	}

	// Create round rect path.
	GraphicsPath* CreateRoundRect(Rect rect, int cornerRadius)
	{
		DeleteOldPath();
		m_pPath = new GraphicsPath();
		m_pPath->AddArc(rect.X, rect.Y, cornerRadius * 2, cornerRadius * 2, 180, 90);
		m_pPath->AddLine(rect.X + cornerRadius, rect.Y, rect.GetRight() - cornerRadius * 2, rect.Y);
		m_pPath->AddArc(rect.X + rect.Width - cornerRadius * 2, rect.Y, cornerRadius * 2, cornerRadius * 2, 270, 90);
		m_pPath->AddLine(rect.GetRight(), rect.Y + cornerRadius * 2, rect.GetRight(), rect.Y + rect.Height - cornerRadius * 2);
		m_pPath->AddArc(rect.X + rect.Width - cornerRadius * 2, rect.Y + rect.Height - cornerRadius * 2, cornerRadius * 2, cornerRadius * 2, 0, 90);
		m_pPath->AddLine(rect.GetRight() - cornerRadius * 2, rect.GetBottom(), rect.X + cornerRadius * 2, rect.GetBottom());
		m_pPath->AddArc(rect.X, rect.GetBottom() - cornerRadius * 2, cornerRadius * 2, cornerRadius * 2, 90, 90);
		m_pPath->AddLine(rect.X, rect.GetBottom() - cornerRadius * 2, rect.X, rect.Y + cornerRadius * 2);
		m_pPath->CloseFigure();
		return m_pPath;
	}
};

class CThemedShadowWnd : public CWindowImpl<CThemedShadowWnd, CWindow, CControlWinTraits>
{
	// Data members.
private:
	HWND		m_hParentWnd;		// Parent window.
	LONG		m_OriParentProc;	// Original parent window processing.
	int			m_nShadowSize;		// Set the shadow size.
	CRect		m_rcClient;			// Client rect.
	Bitmap*		m_pBitmap;			// GDI+ bitmap for drawing shadow.
	CBitmap		m_bmpShadow;		// GDI bitmap for drawing shadow.
	CPathHelper	m_ShadowPath;		// Round rect path.
	BOOL		m_bResize;			// Indicate window is resized.
	int			m_nBlankArea;		// Set blank area position.

	// for DrawShadow2
	int			m_nSharpness;
	int			m_nDarkness;
	COLORREF	m_Color;
	int			m_nxOffset;
	int			m_nyOffset;
private:
	// Record all the created shadow windows and its parent window.
	static map<HWND, CThemedShadowWnd*>  m_szShadowWindows;

	// Constructor.
public:
	DECLARE_WND_CLASS(_T("WTL_ThemedShadowWindow"))

	CThemedShadowWnd()
	{
		m_hParentWnd	= NULL;
		m_OriParentProc	= 0;
		m_nShadowSize	= 5;
		m_pBitmap		= NULL;
		m_bResize		= FALSE;
		m_nBlankArea	= 0;

		m_nSharpness	= 5;
		m_nDarkness		= 300;
		m_Color			= RGB(0, 0, 0);
		m_nxOffset		= 0;
		m_nyOffset		= 0;
	}

	// Operations.
public:
	// Create shadow window.
	HWND Create(const HWND wndParent)
	{
		ATLASSERT( ::IsWindow(wndParent) );
		m_hParentWnd = wndParent;
		CRect rc(1, 1, 1, 1);
		return CWindowImpl<CThemedShadowWnd, CWindow, CControlWinTraits>::Create(0, rc, NULL, WS_VISIBLE, NULL);
	}

	// Set shadow window's parent.
	void SetThemeParent(HWND wndParent)
	{
		ATLASSERT( ::IsWindow(wndParent) );
		m_hParentWnd = wndParent;
	}

	// Adjust shadow window position via parent.
	void AdjustWindowPos()
	{
		CRect rcParent;

		if (::IsIconic(m_hParentWnd))
		{
			return;
		}

		::GetWindowRect(m_hParentWnd, &rcParent);

		rcParent.InflateRect(m_nShadowSize, m_nShadowSize, m_nShadowSize, m_nShadowSize);
		rcParent.MoveToY(rcParent.top);
		rcParent.MoveToX(rcParent.left);
		SetWindowPos(m_hParentWnd, rcParent, SWP_NOACTIVATE);

		GetClientRect(m_rcClient);

		// If window was resized, redraw the shadow.
		if (m_bResize == TRUE)
		{
			m_bResize = FALSE;
			//DrawShadow();
			DrawShadow2();
		}
	}

	// Set the shadow size.
	void SetShadowSize(const int nSize)
	{	
		if (nSize <= 0 || nSize > 255)
		{
			m_nShadowSize = 5;
			m_nSharpness = 5;
		}
		else
		{
			m_nShadowSize = nSize;	
			m_nSharpness = nSize;
		}
	}

	// Set blank area right position.
	void SetRightOffsetArea(const int nRightPos)
	{
		m_nBlankArea = nRightPos;
		if (nRightPos < 0)
		{
			m_nBlankArea = 1;
		}
	}

	void SetDrawShadow2(const int nSharpness = 5, 
		const int nDarkness = 300,
		const int nxOffset = 0,
		const int nyOffset = 0,
		const COLORREF stColor = RGB(0,0,0))
	{
		m_nSharpness	= nSharpness;
		m_nDarkness		= nDarkness;
		m_Color			= stColor;
		m_nxOffset		= nxOffset;
		m_nyOffset		= nyOffset;
	}

public:
	// Get parent message.
	static LRESULT CALLBACK ParentProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		// Find the shadow window pointer via parent window handle.
		ATLASSERT( m_szShadowWindows.find(hwnd) != m_szShadowWindows.end() );
		CThemedShadowWnd *pThis	= m_szShadowWindows[hwnd];
		WNDPROC pDefProc		= (WNDPROC)pThis->m_OriParentProc;

		switch(uMsg)
		{
		case WM_ERASEBKGND:
		case WM_PAINT:
		case WM_MOVE:
		case WM_ACTIVATE:
		case WM_NCACTIVATE:
		case WM_SIZE:
			{
				//pThis->DrawShadow2();
				if (::IsWindowVisible(hwnd)) 
				{ 
					pThis->AdjustWindowPos(); 
				}
				break;
			}
		case WM_DESTROY:
			{
				// Destroy the shadow window.
				pThis->DestroyWindow();	
				break;
			}
		case WM_NCDESTROY:
			{
				// Remove shadow window from map.
				m_szShadowWindows.erase(hwnd);	
				break;
			}
		case WM_SHOWWINDOW:
			{
				// the window is being hidden
				if (!wParam)	
				{
					pThis->ShowWindow(SW_HIDE);
				}
				else
				{
					pThis->ShowWindow(SW_SHOW);
				}
				break;
			}
		default:
			{
				break;
			}
		}
 
		return pDefProc(hwnd, uMsg, wParam, lParam);
	}

	void DrawShadow()
	{
		DeleteOldBrush();

		// Create shadow path.
		Rect rcShadow(m_rcClient.left, m_rcClient.top, m_rcClient.Width(), m_rcClient.Height());
		m_ShadowPath.CreateRoundRect(rcShadow, 10);

		m_pBitmap = new Bitmap(rcShadow.Width, rcShadow.Height);
		Graphics graphics(m_pBitmap);

		// Create shadow brush.
		PathGradientBrush brShadow(m_ShadowPath.m_pPath);
		Color clrShadow[3] = {Color::Transparent, Color(255, 0, 0, 0), Color(255, 0, 0, 0)};
		int nCount = 3;

		REAL szPos[3] = {0.0F, 0.05F, 1.0F};
		brShadow.SetInterpolationColors(clrShadow, szPos, nCount);

		// Draw shadow.
		rcShadow.Width	= rcShadow.Width - m_nShadowSize*2 - m_nBlankArea;
		rcShadow.Height	= rcShadow.Height - m_nShadowSize*2 - m_nBlankArea;

		rcShadow.X = rcShadow.X + m_nShadowSize + m_nBlankArea;
		rcShadow.Y = rcShadow.Y + m_nShadowSize + m_nBlankArea;

		graphics.ExcludeClip(rcShadow);
		graphics.FillPath(&brShadow, m_ShadowPath.m_pPath);

		// Update layered window.
		m_pBitmap->GetHBITMAP((ARGB)Color::Black, &m_bmpShadow.m_hBitmap);

		CClientDC dc(m_hWnd);

		CDC dcCompat;
		dcCompat.CreateCompatibleDC();
		dcCompat.SelectBitmap(m_bmpShadow);

		BITMAP bmpInfo = {0};
		m_bmpShadow.GetBitmap(&bmpInfo);
		CSize size(bmpInfo.bmWidth, bmpInfo.bmHeight);
		CRect rcWindow;
		GetWindowRect(&rcWindow);

		BLENDFUNCTION bf = { AC_SRC_OVER, 0, 128, AC_SRC_ALPHA };

		CPoint ptWnd(rcWindow.left, rcWindow.top);
		CPoint ptSource(0, 0);

		UpdateLayeredWindow(m_hWnd, dc.m_hDC, &ptWnd, &size,
			dcCompat.m_hDC, &ptSource, 0, &bf, ULW_ALPHA);
	}

/* format 取值为：
image/bmp 
image/jpeg 
image/gif 
image/tiff 
image/png 
*/
int GetEncoderClsid( const WCHAR* format, CLSID* pClsid )
{
    UINT  num = 0;          // number of image encoders
    UINT  size = 0;         // size of the image encoder array in bytes
 
    ImageCodecInfo* pImageCodecInfo = NULL;
 
    GetImageEncodersSize(&num, &size);
    if(size == 0)
        return -1;  // Failure
 
    pImageCodecInfo = (ImageCodecInfo*)(malloc(size));
    if(pImageCodecInfo == NULL)
        return -1;  // Failure
 
    GetImageEncoders(num, size, pImageCodecInfo);
 
    for(UINT j = 0; j < num; ++j)
    {
        if( wcscmp(pImageCodecInfo[j].MimeType, format) == 0 )
        {
            *pClsid = pImageCodecInfo[j].Clsid;
            free(pImageCodecInfo);
            return j;  // Success
        }    
    }
 
    free(pImageCodecInfo);
    return -1;  // Failure
 
}


	void DrawShadow2()
	{
		DeleteOldBrush();

		RECT WndRect;
		::GetWindowRect(m_hParentWnd, &WndRect);
		int nShadWndWid = WndRect.right - WndRect.left + m_nShadowSize*2;
		int nShadWndHei = WndRect.bottom - WndRect.top + m_nShadowSize*2;

		if( nShadWndWid <=0 ||  nShadWndHei<=0){
			return ; 
		}

		ATL::CString str;
		str.Format(_T("width %d, height %d \r\n"), nShadWndWid, nShadWndHei);
		OutputDebugString(str);

		// Create the alpha blending bitmap
		BITMAPINFO bmi;        // bitmap header

		ZeroMemory(&bmi, sizeof(BITMAPINFO));
		bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
		bmi.bmiHeader.biWidth = nShadWndWid;
		bmi.bmiHeader.biHeight = nShadWndHei;
		bmi.bmiHeader.biPlanes = 1;
		bmi.bmiHeader.biBitCount = 32;         // four 8-bit components
		bmi.bmiHeader.biCompression = BI_RGB;
		bmi.bmiHeader.biSizeImage = nShadWndWid * nShadWndHei * 4;

		BYTE *pvBits = 0;          // pointer to DIB section
		HBITMAP hbitmap = CreateDIBSection(NULL, &bmi, DIB_RGB_COLORS, (void **)&pvBits, NULL, 0);
		if( !hbitmap || !pvBits ||bmi.bmiHeader.biSizeImage ==0 ) {//Modify By Fanxiushu
			return ; 
		}

		ZeroMemory(pvBits, bmi.bmiHeader.biSizeImage);
		MakeShadow((UINT32 *)pvBits, m_hParentWnd, &WndRect);

// 		CLSID bmpClsid;
// 		GetEncoderClsid(L"image/bmp", &bmpClsid);
// 		//保存到磁盘上的bmp文件
// 		Gdiplus::Bitmap bitmap(hbitmap,NULL);
// 		bitmap.Save(L"d:\\1.bmp", &bmpClsid, NULL);

		CClientDC dc(m_hWnd);

		CDC dcCompat;
		dcCompat.CreateCompatibleDC();
		dcCompat.SelectBitmap(hbitmap);

		m_bmpShadow.Attach(hbitmap);
		BITMAP bmpInfo = {0};
		m_bmpShadow.GetBitmap(&bmpInfo);
		CSize size(bmpInfo.bmWidth, bmpInfo.bmHeight);



		BLENDFUNCTION bf = { AC_SRC_OVER, 0, 128, AC_SRC_ALPHA };

		CPoint ptWnd(WndRect.left + m_nxOffset - m_nShadowSize, WndRect.top + m_nyOffset - m_nShadowSize);
		CPoint ptSource(0, 0);

		::MoveWindow(m_hWnd, ptWnd.x, ptWnd.y, nShadWndWid, nShadWndHei, FALSE);

		BOOL bRet = UpdateLayeredWindow(m_hWnd, dc.m_hDC, &ptWnd, &size,
			dcCompat.m_hDC, &ptSource, 0, &bf, ULW_ALPHA);

		str.Format(_T("bRet %d \r\n"), bRet);
		OutputDebugString(str);

		DeleteObject(hbitmap);
	}

	// Helper to calculate the alpha-premultiled value for a pixel
	inline DWORD PreMultiply(COLORREF cl, unsigned char nAlpha)
	{
		// It's strange that the byte order of RGB in 32b BMP is reverse to in COLORREF
		return (GetRValue(cl) * (DWORD)nAlpha / 255) << 16 |
			(GetGValue(cl) * (DWORD)nAlpha / 255) << 8 |
			(GetBValue(cl) * (DWORD)nAlpha / 255);
	}

	// Fill in the shadow window alpha blend bitmap with shadow image pixels
	void MakeShadow(UINT32 *pShadBits, HWND hParent, RECT *rcParent)
	{
		// The shadow algorithm:
		// Get the region of parent window,
		// Apply morphologic erosion to shrink it into the size (ShadowWndSize - Sharpness)
		// Apply modified (with blur effect) morphologic dilation to make the blurred border
		// The algorithm is optimized by assuming parent window is just "one piece" and without "wholes" on it

		// Get the region of parent window,
		// Create a full rectangle region in case of the window region is not defined
		HRGN hParentRgn = CreateRectRgn(0, 0, rcParent->right - rcParent->left, rcParent->bottom - rcParent->top);
		::GetWindowRgn(hParent, hParentRgn);

		// Determine the Start and end point of each horizontal scan line
		SIZE szParent = {rcParent->right - rcParent->left, rcParent->bottom - rcParent->top};
		SIZE szShadow = {szParent.cx + 2 * m_nShadowSize, szParent.cy + 2 * m_nShadowSize};

		// Extra 2 lines (set to be empty) in ptAnchors are used in dilation
		int nAnchors = max(szParent.cy, szShadow.cy);	// # of anchor points pares
		int (*ptAnchors)[2] = new int[nAnchors + 2][2];
		int (*ptAnchorsOri)[2] = new int[szParent.cy][2];	// anchor points, will not modify during erosion
		ptAnchors[0][0] = szParent.cx;
		ptAnchors[0][1] = 0;
		ptAnchors[nAnchors + 1][0] = szParent.cx;
		ptAnchors[nAnchors + 1][1] = 0;
		if(m_nShadowSize > 0)
		{
			// Put the parent window anchors at the center
			for(int i = 0; i < m_nShadowSize; i++)
			{
				ptAnchors[i + 1][0] = szParent.cx;
				ptAnchors[i + 1][1] = 0;
				ptAnchors[szShadow.cy - i][0] = szParent.cx;
				ptAnchors[szShadow.cy - i][1] = 0;
			}
			ptAnchors += m_nShadowSize;
		}
		for(int i = 0; i < szParent.cy; i++)
		{
			// find start point
			int j;
			for(j = 0; j < szParent.cx; j++)
			{
				if(PtInRegion(hParentRgn, j, i))
				{
					ptAnchors[i + 1][0] = j + m_nShadowSize;
					ptAnchorsOri[i][0] = j;
					break;
				}
			}

			if(j >= szParent.cx)	// Start point not found
			{
				ptAnchors[i + 1][0] = szParent.cx;
				ptAnchorsOri[i][1] = 0;
				ptAnchors[i + 1][0] = szParent.cx;
				ptAnchorsOri[i][1] = 0;
			}
			else
			{
				// find end point
				for(j = szParent.cx - 1; j >= ptAnchors[i + 1][0]; j--)
				{
					if(PtInRegion(hParentRgn, j, i))
					{
						ptAnchors[i + 1][1] = j + 1 + m_nShadowSize;
						ptAnchorsOri[i][1] = j + 1;
						break;
					}
				}
			}
			// 		if(0 != ptAnchorsOri[i][1])
			// 			_RPTF2(_CRT_WARN, "%d %d\n", ptAnchorsOri[i][0], ptAnchorsOri[i][1]);
		}

		if(m_nShadowSize > 0)
			ptAnchors -= m_nShadowSize;	// Restore pos of ptAnchors for erosion
		int (*ptAnchorsTmp)[2] = new int[nAnchors + 2][2];	// Store the result of erosion
		// First and last line should be empty
		ptAnchorsTmp[0][0] = szParent.cx;
		ptAnchorsTmp[0][1] = 0;
		ptAnchorsTmp[nAnchors + 1][0] = szParent.cx;
		ptAnchorsTmp[nAnchors + 1][1] = 0;
		int nEroTimes = 0;
		// morphologic erosion
		for(int i = 0; i < m_nSharpness - m_nShadowSize; i++)
		{
			nEroTimes++;
			//ptAnchorsTmp[1][0] = szParent.cx;
			//ptAnchorsTmp[1][1] = 0;
			//ptAnchorsTmp[szParent.cy + 1][0] = szParent.cx;
			//ptAnchorsTmp[szParent.cy + 1][1] = 0;
			for(int j = 1; j < nAnchors + 1; j++)
			{
				ptAnchorsTmp[j][0] = max(ptAnchors[j - 1][0], max(ptAnchors[j][0], ptAnchors[j + 1][0])) + 1;
				ptAnchorsTmp[j][1] = min(ptAnchors[j - 1][1], min(ptAnchors[j][1], ptAnchors[j + 1][1])) - 1;
			}
			// Exchange ptAnchors and ptAnchorsTmp;
			int (*ptAnchorsXange)[2] = ptAnchorsTmp;
			ptAnchorsTmp = ptAnchors;
			ptAnchors = ptAnchorsXange;
		}

		// morphologic dilation
		ptAnchors += (m_nShadowSize < 0 ? -m_nShadowSize : 0) + 1;	// now coordinates in ptAnchors are same as in shadow window
		// Generate the kernel
		int nKernelSize = m_nShadowSize > m_nSharpness ? m_nShadowSize : m_nSharpness;
		int nCenterSize = m_nShadowSize > m_nSharpness ? (m_nShadowSize - m_nSharpness) : 0;
		UINT32 *pKernel = new UINT32[(2 * nKernelSize + 1) * (2 * nKernelSize + 1)];
		UINT32 *pKernelIter = pKernel;
		for(int i = 0; i <= 2 * nKernelSize; i++)
		{
			for(int j = 0; j <= 2 * nKernelSize; j++)
			{
				double dLength = sqrt((i - nKernelSize) * (i - nKernelSize) + (j - nKernelSize) * (double)(j - nKernelSize));
				if(dLength < nCenterSize)
					*pKernelIter = m_nDarkness << 24 | PreMultiply(m_Color, m_nDarkness);
				else if(dLength <= nKernelSize)
				{
					UINT32 nFactor = ((UINT32)((1 - (dLength - nCenterSize) / (m_nSharpness + 1)) * m_nDarkness));
					*pKernelIter = nFactor << 24 | PreMultiply(m_Color, nFactor);
				}
				else
					*pKernelIter = 0;
				//TRACE("%d ", *pKernelIter >> 24);
				pKernelIter ++;
			}
			//TRACE("\n");
		}
		// Generate blurred border
		for(int i = nKernelSize; i < szShadow.cy - nKernelSize; i++)
		{
			int j;
			if(ptAnchors[i][0] < ptAnchors[i][1])
			{

				// Start of line
				for(j = ptAnchors[i][0];
					j < min(max(ptAnchors[i - 1][0], ptAnchors[i + 1][0]) + 1, ptAnchors[i][1]);
					j++)
				{
					for(int k = 0; k <= 2 * nKernelSize; k++)
					{
						UINT32 *pPixel = pShadBits +
							(szShadow.cy - i - 1 + nKernelSize - k) * szShadow.cx + j - nKernelSize;
						UINT32 *pKernelPixel = pKernel + k * (2 * nKernelSize + 1);
						for(int l = 0; l <= 2 * nKernelSize; l++)
						{
							if(*pPixel < *pKernelPixel)
								*pPixel = *pKernelPixel;
							pPixel++;
							pKernelPixel++;
						}
					}
				}	// for() start of line

				// End of line
				for(j = max(j, min(ptAnchors[i - 1][1], ptAnchors[i + 1][1]) - 1);
					j < ptAnchors[i][1];
					j++)
				{
					for(int k = 0; k <= 2 * nKernelSize; k++)
					{
						UINT32 *pPixel = pShadBits +
							(szShadow.cy - i - 1 + nKernelSize - k) * szShadow.cx + j - nKernelSize;
						UINT32 *pKernelPixel = pKernel + k * (2 * nKernelSize + 1);
						for(int l = 0; l <= 2 * nKernelSize; l++)
						{
							if(*pPixel < *pKernelPixel)
								*pPixel = *pKernelPixel;
							pPixel++;
							pKernelPixel++;
						}
					}
				}	// for() end of line

			}
		}	// for() Generate blurred border

		// Erase unwanted parts and complement missing
		UINT32 clCenter = m_nDarkness << 24 | PreMultiply(m_Color, m_nDarkness);
		for(int i = min(nKernelSize, max(m_nShadowSize - m_nyOffset, 0));
			i < max(szShadow.cy - nKernelSize, min(szParent.cy + m_nShadowSize - m_nyOffset, szParent.cy + 2 * m_nShadowSize));
			i++)
		{
			UINT32 *pLine = pShadBits + (szShadow.cy - i - 1) * szShadow.cx;
			if(i - m_nShadowSize + m_nyOffset < 0 || i - m_nShadowSize + m_nyOffset >= szParent.cy)	// Line is not covered by parent window
			{
				for(int j = ptAnchors[i][0]; j < ptAnchors[i][1]; j++)
				{
					*(pLine + j) = clCenter;
				}
			}
			else
			{
				for(int j = ptAnchors[i][0];
					j < min(ptAnchorsOri[i - m_nShadowSize + m_nyOffset][0] + m_nShadowSize - m_nxOffset, ptAnchors[i][1]);
					j++)
					*(pLine + j) = clCenter;
				for(int j = max(ptAnchorsOri[i - m_nShadowSize + m_nyOffset][0] + m_nShadowSize - m_nxOffset, 0);
					j < min(ptAnchorsOri[i - m_nShadowSize + m_nyOffset][1] + m_nShadowSize - m_nxOffset, szShadow.cx);
					j++)
					*(pLine + j) = 0;
				for(int j = max(ptAnchorsOri[i - m_nShadowSize + m_nyOffset][1] + m_nShadowSize - m_nxOffset, ptAnchors[i][0]);
					j < ptAnchors[i][1];
					j++)
					*(pLine + j) = clCenter;
			}
		}

		// Delete used resources
		delete[] (ptAnchors - (m_nShadowSize < 0 ? -m_nShadowSize : 0) - 1);
		delete[] ptAnchorsTmp;
		delete[] ptAnchorsOri;
		delete[] pKernel;
		DeleteObject(hParentRgn);
	}

private:
	// Delete old brush.
	void DeleteOldBrush()
	{
		if (m_pBitmap)
		{
			delete m_pBitmap;
			m_pBitmap = NULL;
		}
	}

	// Message map and handlers.
public:
	BEGIN_MSG_MAP(CThemedShadowWnd)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MESSAGE_HANDLER(WM_SIZE, OnSize)
		DEFAULT_REFLECTION_HANDLER()
	END_MSG_MAP()

	LRESULT OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/)
	{
		// Record shadow window and its parent window.
		m_szShadowWindows.insert( make_pair(m_hParentWnd, this) );

		// Reset window styles.
		ModifyStyle(WS_MAXIMIZEBOX | WS_SIZEBOX | WS_CAPTION | WS_DLGFRAME, 0, 0);

		// The window style should be WS_EX_LAYERED.
		SetWindowLong(GWL_EXSTYLE, GetWindowLong(GWL_EXSTYLE) | WS_EX_LAYERED | WS_EX_TRANSPARENT);
		ModifyStyleEx(WS_EX_TOPMOST, WS_EX_NOACTIVATE);

		// Set parent window original processing.
		m_OriParentProc = ::GetWindowLong(m_hParentWnd, GWL_WNDPROC);
		::SetWindowLong(m_hParentWnd, GWL_WNDPROC, (LONG)ParentProc);

		LPCREATESTRUCT lpcs = ( LPCREATESTRUCT ) lParam;
		LRESULT res = DefWindowProc( uMsg, wParam, ( LPARAM ) lpcs );
		return res;
	}

	LRESULT OnSize(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
	{
		m_bResize = TRUE;
		bHandled = FALSE;
		return 0;
	}
};

#endif // _THEMEDSHADOWWND_H_INCLUDE_