#include "StdAfx.h"
#include "CaptureVFW.h"
#include <algorithm>


#pragma comment( lib, "Vfw32.lib" )
#pragma comment(lib, "GdiPlus.lib")


#define TRACE	ATLTRACE



#ifndef _VERIFY
	#ifdef _DEBUG
	  #define _VERIFY(f)	_ASSERTE(f)
	#else   // _DEBUG
	  #define _VERIFY(f)	((void)(f))
	#endif
#endif


#define STRL2(x) #x
#define STRL(x) STRL2(x)
#define warnMSG(desc) message(__FILE__ "(" STRL(__LINE__) "):" " !!! *** [" desc "] ***")


BOOL ConvertBmpToRawPgm( BYTE* pData, Gdiplus::Bitmap* pBMP )
{
	_ASSERTE( pBMP );

	//lock bits
	Gdiplus::BitmapData bitmapData;
	int width = pBMP->GetWidth();
	int height = pBMP->GetHeight();
	Gdiplus::Rect rect( 0, 0, width, height );
	Gdiplus::Status err = pBMP->LockBits( &rect, Gdiplus::ImageLockModeRead, PixelFormat32bppRGB, &bitmapData );
	if( err != Gdiplus::Ok ) return FALSE;

	BYTE* pixels = (BYTE*)bitmapData.Scan0;
	int i=0;
	for( int row = 0; row < height; ++row )
	{
		for( int col = 0; col < width; ++col)
		{
			pData[i++] = (BYTE)((299*pixels[(row * bitmapData.Stride/4 + col)*4 + 2] + 587*pixels[(row * bitmapData.Stride/4 + col)*4 + 1] + 114*pixels[(row * bitmapData.Stride/4 + col)*4 + 0])/1000);
		}
	}

	pBMP->UnlockBits( &bitmapData );

	return TRUE;
}

void DrawFoundBarcodes( Gdiplus::Graphics * pgr, IBarcodeDecoder* pIBarcodeDecoder )
{
	_ASSERTE( pgr );

	_ASSERTE( pIBarcodeDecoder );

	CComPtr<IBarcodeList> pIBarcodeList;
	HRESULT hr = pIBarcodeDecoder->get_Barcodes( &pIBarcodeList );
	_ASSERTE( pIBarcodeList );

	long len;
	hr = pIBarcodeList->get_length( &len );
	_ASSERTE( pIBarcodeList );

	if( len == 0 )
		return;

	// Create objects
	Gdiplus::Pen greenPen(Gdiplus::Color(0, 200, 0), 3);
	Gdiplus::Pen redPen(Gdiplus::Color(255, 0, 0), 3);
	Gdiplus::Font myFont(L"Arial", 11, Gdiplus::FontStyleBold, Gdiplus::UnitPoint );
	Gdiplus::SolidBrush fontBrush(Gdiplus::Color(0, 200, 0));

	for( long i=0; i < len; ++i )
	{
		IBarcodePtr pBarcode;
		pBarcode = pIBarcodeList->item( i );
		_ASSERTE( pBarcode );

		CString sResult, sBarcode;

		EBarcodeTypes bt = pBarcode->BarcodeType;
		switch( bt )
		{
		case Code128:				sBarcode = "Code128";				break;
		case Code39:				sBarcode = "Code39";				break;
		case Interl25:				sBarcode = "Interl25";				break;
		case EAN13:					sBarcode = "EAN13";					break;
		case EAN8:					sBarcode = "EAN8";					break;
		case Codabar:				sBarcode = "Codabar";				break;
		case Code11:				sBarcode = "Code11";				break;
		case UPCA:					sBarcode = "UPCA";					break;
		case UPCE:					sBarcode = "UPCE";					break;
		case Code93:				sBarcode = "Code93";				break;
		case DataBarOmni:			sBarcode = "DataBarOmni";			break;
		case DataBarLim:			sBarcode = "DataBarLim";			break;

		case LinearUnrecognized:	sBarcode = "Linear Unrecognized";	break;
		case PDF417Unrecognized:	sBarcode = "PDF417 Unrecognized";	break;
		case DataMatrixUnrecognized:sBarcode = "DataMatrix Unrecognized";	break;

		case PDF417:				sBarcode = "PDF417";				break;
		case DataMatrix:			sBarcode = "DataMatrix";			break;
		}

		pgr->SetSmoothingMode( Gdiplus::SmoothingModeAntiAlias );

		//outline barcode
		Gdiplus::Pen* pPen;
		if( bt == LinearUnrecognized || bt == PDF417Unrecognized || bt == DataMatrixUnrecognized )
			pPen = &redPen;
		else
			pPen = &greenPen;
		pgr->DrawLine( pPen, pBarcode->x1, pBarcode->y1, pBarcode->x2, pBarcode->y2 );
		pgr->DrawLine( pPen, pBarcode->x2, pBarcode->y2, pBarcode->x3, pBarcode->y3 );
		pgr->DrawLine( pPen, pBarcode->x3, pBarcode->y3, pBarcode->x4, pBarcode->y4 );
		pgr->DrawLine( pPen, pBarcode->x4, pBarcode->y4, pBarcode->x1, pBarcode->y1 );

		sResult.Format( "%s (%s)", (LPCTSTR)pBarcode->Text, sBarcode );
		Gdiplus::PointF point(0, 0);
		BSTR bstrResult = sResult.AllocSysString() ;
		pgr->DrawString( bstrResult, wcslen(bstrResult), &myFont, point, &fontBrush );
	}

	::Sleep(1000);
}



struct delarrObject
{
	template<class T>
	void operator () (T ptr)
	{
		delete [] ptr;
	}
};

int GetEncoderClsid(const WCHAR* format, CLSID* pClsid)
{
   using namespace Gdiplus;

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


CCaptureVFW::CCaptureVFW(void)
{
	m_hWndCap = NULL;
	m_DrvIdx = -1;
	m_vDrvName.reserve( 5 );
	m_pBI = NULL;
	m_gdiplusToken = 0;

	//initialize GDI+
	Gdiplus::Status status = Gdiplus::GdiplusStartup( &m_gdiplusToken, &m_gdiplusStartupInput, NULL );
	if( status != Gdiplus::Ok )
	{
		//error
	}

	HRESULT hr = ::CoInitialize( NULL );

	hr = m_pIBarcodeDecoder.CoCreateInstance( __uuidof(BarcodeDecoder) );
	if( FAILED(hr) )
	{
		AfxMessageBox( _T("Cant create Barcode Reader object.") );
	}
}

CCaptureVFW::~CCaptureVFW(void)
{
	ConnectToDriver( FALSE );

	std::for_each( m_vDrvName.begin(), m_vDrvName.end(), delarrObject() );

	if( m_gdiplusToken )
		Gdiplus::GdiplusShutdown( m_gdiplusToken );
}

BOOL CCaptureVFW::CreateCaptureWindow( HWND hWndParent, DWORD dwStyle, UINT nID )
{
	if( m_hWndCap ) return FALSE;

	m_hWndCap = capCreateCaptureWindow(
		(LPSTR) "My Capture Window for VFW",		//window name if pop-up 
		dwStyle,									//window style 
		0, 0, 0, 0,									//window position and dimensions
		(HWND)hWndParent,							//parent HWND
		(int)nID									//child ID
		);

	if( !m_hWndCap )
		return FALSE;

	if( !capSetUserData(m_hWndCap, this) )
		return FALSE;

	EnumDriversName( NULL, NULL );

	return( m_hWndCap ? TRUE : FALSE );
}

void CCaptureVFW::DestroyCaptureWnd()
{
	if( !m_hWndCap ) return;
	if( ::DestroyWindow(m_hWndCap) )
		m_hWndCap = NULL;
}

BOOL CCaptureVFW::EnumDriversName( char*** pppDrvName, WORD* pwSize )
{
	if( pppDrvName && pwSize )
	{
		*pppDrvName = NULL;
		*pwSize = 0;
	}

	char szDeviceName[80];
	char szDeviceVersion[80];

	std::for_each( m_vDrvName.begin(), m_vDrvName.end(), delarrObject() );
	m_vDrvName.clear();

	for( WORD wIndex = 0; wIndex < 10; wIndex++ ) 
	{
		if( capGetDriverDescription(wIndex, szDeviceName, 
			sizeof(szDeviceName), szDeviceVersion, sizeof(szDeviceVersion) ) )
		{
			char* szValue = new char[strlen(szDeviceName)+1];
			strcpy( szValue, szDeviceName );
			m_vDrvName.push_back( szValue );
		}
	}

	if( pppDrvName && pwSize )
	{
		*pwSize = (WORD)m_vDrvName.size();
		*pppDrvName = &m_vDrvName[0];
	}

	return TRUE;
}

BOOL CCaptureVFW::ChangeCapWndSize()
{
	//change window size
	_VERIFY( ObtainVideoFormat() );
	if( !m_pBI ) return FALSE;

	return ::SetWindowPos( 
		m_hWndCap, NULL, 0, 0, 
		m_pBI->bmiHeader.biWidth, m_pBI->bmiHeader.biHeight,
		SWP_NOMOVE|SWP_NOZORDER );
}

/*static*/
LRESULT CALLBACK CCaptureVFW::CallbackProcOnFrame( HWND hWnd, LPVIDEOHDR lpVHdr )
{ 
	TRACE( "CallbackProcOnFrame\n" );

	CCaptureVFW* pThis = reinterpret_cast<CCaptureVFW*>(capGetUserData(hWnd));
	_ASSERTE( _CrtIsValidPointer(pThis, sizeof(CCaptureVFW), TRUE) );
	if( !pThis ) return 1;

	if( pThis->m_pBI->bmiHeader.biCompression != BI_RGB )
	{
		//need decompression
		HANDLE hDec;
		__try
		{
			hDec = ICImageDecompress( NULL, 0, pThis->m_pBI, lpVHdr->lpData, NULL );

			LPBITMAPINFO pDecBI = (BITMAPINFO*)::GlobalLock( hDec );
			if( !pDecBI ) __leave;

			pThis->m_Result = pThis->CaptureBarcode( pDecBI, (BYTE*)pDecBI + pDecBI->bmiHeader.biSize );
		}
		__finally
		{
			_VERIFY( GlobalFree(hDec) == NULL );
		}
	}
	else
	{
		//do not need decompression
		pThis->m_Result = pThis->CaptureBarcode( pThis->m_pBI, lpVHdr->lpData );
	}

    return 1;
} 

#ifdef _DEBUG
/*static*/
LRESULT CALLBACK CCaptureVFW::CallbackProcOnStatus( HWND hWnd, int nID, LPCSTR lpsz )
{
	TRACE( "capStatusCallback - [%d] %s\n", nID, lpsz );
	return 1;
}

/*static*/
LRESULT CALLBACK CCaptureVFW::CallbackProcOnError( HWND hWnd, int nID, LPCSTR lpsz )
{
	TRACE( "CallbackProcOnError - [%d] %s\n", nID, lpsz );
	return 1;
}
#endif	//_DEBUG

BOOL CCaptureVFW::ConnectToDriver( BOOL bConDisc, int idx/*=0*/ )
{
	BOOL res = FALSE;
	if( bConDisc )
	{
		if( m_DrvIdx != -1 ) return FALSE;

		res = capDriverConnect( m_hWndCap, idx ); 

		if( res )
		{
			m_DrvIdx = idx;

#ifdef _DEBUG
		_VERIFY( capSetCallbackOnStatus(m_hWndCap, &CallbackProcOnStatus) );
		_VERIFY( capSetCallbackOnError(m_hWndCap, &CallbackProcOnError) );
#endif	//_DEBUG

			if( !capSetCallbackOnFrame(m_hWndCap, &CallbackProcOnFrame) )
			{
				ConnectToDriver( FALSE );
				return FALSE;
			}

			res = ChangeCapWndSize();
		}
	}
	else
	{
		if( m_DrvIdx == -1 ) return FALSE;

		res = capSetCallbackOnFrame( m_hWndCap, NULL );

#ifdef _DEBUG
		capSetCallbackOnStatus( m_hWndCap, NULL );
		capSetCallbackOnError( m_hWndCap, NULL );
#endif	//_DEBUG

		res = capDriverDisconnect( m_hWndCap );
		if( res )
		{
			if( m_pBI )
			{
				m_pBI = NULL;
				::GlobalFree( (HGLOBAL)m_pBI );
			}
			m_DrvIdx = -1;
		}
	}

#ifdef _DEBUG
	if( res )
	{
		if( bConDisc )
			TRACE( "connect to driver %d - Ok\n", idx );
		if( !bConDisc )
			TRACE( "dicconnect from driver - Ok\n" );
	}
#endif	//_DEBUG

	return res;
}

BOOL CCaptureVFW::GetCurDriverName( char** ppName )
{
	if( !ppName )
		return FALSE;
	*ppName = NULL;

	if( m_DrvIdx < 0 || m_DrvIdx >= (int)m_vDrvName.size() )
		return FALSE;

	*ppName = m_vDrvName[m_DrvIdx];

	return TRUE;
}

BOOL CCaptureVFW::CaptureBarcode( const BITMAPINFO* pBI, void* pData )
{
	if( !m_hWndCap || m_pIBarcodeDecoder==NULL )
		return FALSE;

	HDC hDC;
	HBITMAP hBitmap;
	HBITMAP holdBitmap;
	BOOL res = FALSE;
	HDC hdcCompatible = 0;
	SIZE szBmp={0};
	HRESULT hr;

	try
	{
		hDC = ::GetDC( m_hWndCap );
		if( !hDC ) throw 0;

		hdcCompatible = ::CreateCompatibleDC( hDC );
		if( !hdcCompatible ) throw 0;

		//calc. bitmap size
		szBmp.cx = pBI->bmiHeader.biWidth;
		szBmp.cy = pBI->bmiHeader.biHeight;

		//create bitmap
		hBitmap = ::CreateCompatibleBitmap( hDC, szBmp.cx, szBmp.cy );
		if( !hBitmap ) throw 0;

		//select bitmap
		holdBitmap = (HBITMAP)::SelectObject( hdcCompatible, hBitmap );

		int resscan = ::StretchDIBits( hdcCompatible,
						0, 0, szBmp.cx, szBmp.cy,
						0, 0, pBI->bmiHeader.biWidth, pBI->bmiHeader.biHeight,
						pData, pBI,
						DIB_RGB_COLORS,
						SRCCOPY );

		if( resscan == 0 ) throw 0;

		::SelectObject( hdcCompatible, holdBitmap );

		Gdiplus::Bitmap bmpGDI( hBitmap, NULL );
		if( bmpGDI.GetLastStatus() != Gdiplus::Ok )
		{
			::MessageBox( AfxGetMainWnd()->GetSafeHwnd(), "Error: Cant create the bmp object", _T("Error"), MB_OK );
			throw 0;
		}

		int width = bmpGDI.GetWidth();
		int height = bmpGDI.GetHeight();

		//create safe array
		CComVariant var;
		var.vt = VT_ARRAY | VT_UI1;
		var.parray = SafeArrayCreateVector( VT_UI1, 0, width*height );
		if( !var.parray ) throw 0;

		//lock safe array
		BYTE* pSaBuf=NULL;
		if( SUCCEEDED(SafeArrayAccessData(var.parray, (void HUGEP**)&pSaBuf)) )
		{
			//convert&copy image into the safe array
			if( !ConvertBmpToRawPgm(pSaBuf, &bmpGDI) )
				throw 0;
			//unlock safe array
			SafeArrayUnaccessData( var.parray );
		}

		//hr = pIBarcodeDecoder->put_LinearFindBarcodes( 7 );

		//find barcodes in the image stream
		hr = m_pIBarcodeDecoder->DecodeGrayMap( var, width, height );

		if( !FAILED(hr) )
		{
			Gdiplus::Graphics gr( hDC );
			DrawFoundBarcodes( &gr, m_pIBarcodeDecoder );
		}
	}
	catch(...)
	{
		hr = E_FAIL;
	}

	if(hDC)
		_VERIFY( ::DeleteDC(hDC) );

	if( hBitmap )
		_VERIFY( ::DeleteObject(hBitmap) );

	if( hdcCompatible )
		_VERIFY( ::DeleteDC(hdcCompatible) );

	return !FAILED(hr);
}

//valid NTSC formats : 640x480, 320x240, 160x120 etc.
//valid PAL  formats : 768x576, 384x288, 196x144 etc.
BOOL CCaptureVFW::SetVideoSize( int cx, int cy )
{
	if( m_DrvIdx == -1 ) return FALSE;
	if( cx == 0 || cy == 0 ) return FALSE;
	if( !ObtainVideoFormat() ) return FALSE;
	if( cx == m_pBI->bmiHeader.biWidth && cy == m_pBI->bmiHeader.biHeight ) return TRUE;

	DWORD dwSize = capGetVideoFormatSize( m_hWndCap );
	m_pBI->bmiHeader.biWidth = cx;
	m_pBI->bmiHeader.biHeight = cy;

	BOOL res = FALSE;

	try{ res = capSetVideoFormat( m_hWndCap, m_pBI, dwSize ); }
	catch(...){ _VERIFY(ObtainVideoFormat()); return FALSE; }

	if( res )
	{
		//reconnect
		int idx = m_DrvIdx;
		res = ConnectToDriver( FALSE );
		res = ConnectToDriver( TRUE, idx );
	}
	else
	{
		_VERIFY( ObtainVideoFormat() );
	}

	return res;
}

BOOL CCaptureVFW::ObtainVideoFormat()
{
	DWORD dwSize = capGetVideoFormatSize( m_hWndCap );
	if( dwSize == 0 )
		return FALSE;

	if( m_pBI )
		::GlobalFree( (HGLOBAL)m_pBI );

	m_pBI = (LPBITMAPINFO)::GlobalAlloc( GHND, dwSize );
	return capGetVideoFormat(m_hWndCap, m_pBI, dwSize) == 0 ? FALSE : TRUE;
}

