#pragma once

#include <vector>

#include <Vfw.h>
#include <Gdiplus.h>

#import "progid:BarcodeReader.BarcodeDecoder" no_namespace


#define	DEFAULT_PREVIEW_RATE	100
#define	RGB_BLUE_COLOR			RGB(0, 0, 255)
#define	RGB_WHITE_COLOR			RGB(255, 255, 255)
#define	TIME_FMT_STR			"%H:%M:%S"
#define	DATE_FMT_STR			"%d-%m-%Y"



class CCaptureVFW
{
public:

	CCaptureVFW(void);
	virtual ~CCaptureVFW(void);

	BOOL CreateCaptureWindow( HWND hWndParent, DWORD dwStyle, UINT nID );
	BOOL EnumDriversName( char*** pppDrvName, WORD* pwSize );
	BOOL ConnectToDriver( BOOL bConDisc, int idx=0 );
	BOOL GetCurDriverName( char** ppName );
	BOOL SetVideoSize( int cx, int cy );
	HWND GetCaptureWnd() const { return m_hWndCap; }
	void DestroyCaptureWnd();

protected:

	static LRESULT CALLBACK CallbackProcOnFrame( HWND hWnd, LPVIDEOHDR lpVHdr );

#ifdef _DEBUG
	static LRESULT CALLBACK CallbackProcOnStatus( HWND hWnd, int nID, LPCSTR lpsz );
	static LRESULT CALLBACK CallbackProcOnError( HWND hWnd, int nID, LPCSTR lpsz );
#endif	//_DEBUG

	CComPtr<IBarcodeDecoder> m_pIBarcodeDecoder;

	HWND	m_hWndCap;				//video capture window
	int		m_DrvIdx;				//current driver index
	std::vector<char*>	m_vDrvName;	//driver name vector
	BOOL	m_Result;				//result
	BITMAPINFO*	m_pBI;				//video format info

	Gdiplus::GdiplusStartupInput	m_gdiplusStartupInput;
	ULONG_PTR						m_gdiplusToken;

	BOOL CreateBMPFile( const char* pszFile, PBITMAPINFO pbi, HBITMAP hBMP, HDC hDC );
	PBITMAPINFO CreateBitmapInfoStruct( HBITMAP hBmp );
	BOOL SaveImage( const char* filename, HBITMAP hBmp );
	BOOL SaveImage( const char* filename, const BITMAPINFO* pBI, void* pData );
	void DrawTextToImage( HDC hdc, const char* pDescr );
	BOOL ChangeCapWndSize();
	BOOL ObtainVideoFormat();
	BOOL ObtainCodecCLSID( const char* pFileName, CLSID& codecClsid );

	BOOL CaptureBarcode();
	BOOL CaptureBarcode( const BITMAPINFO* pBI, void* pData );
};
