// BarcodeVideoDlg.cpp : implementation file
//

#include "stdafx.h"
#include "BarcodeVideo.h"
#include "BarcodeVideoDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CBarcodeVideoDlg dialog




CBarcodeVideoDlg::CBarcodeVideoDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CBarcodeVideoDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CBarcodeVideoDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CBarcodeVideoDlg, CDialog)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_BUTTON1, &CBarcodeVideoDlg::OnBnClickedButton1)
END_MESSAGE_MAP()


// CBarcodeVideoDlg message handlers

BOOL CBarcodeVideoDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here

	return TRUE;  // return TRUE  unless you set the focus to a control
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CBarcodeVideoDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CBarcodeVideoDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


void CBarcodeVideoDlg::OnBnClickedButton1()
{
	//enumerate drivers
	char** ppDrvName;
	WORD wSize=0;
	if( !m_vfw.EnumDriversName(&ppDrvName, &wSize) )
	{
		AfxMessageBox("Error: Cant enumerate video drivers");
		return;
	}

	//create window
	BOOL res = m_vfw.CreateCaptureWindow( GetSafeHwnd(), WS_CHILD|WS_VISIBLE, 1000 );
	if( !res )
	{
		AfxMessageBox("Error: Cant create a video capture window");
		return;
	}

	//connect to the driver number 0
	res = m_vfw.ConnectToDriver( TRUE, 0 );
	if( !res )
	{
		::MessageBox( this->GetSafeHwnd(), "Error: Cant connect to a video driver 0", _T("Error"), MB_OK );
		return;
	}

	//window refreash rate, ms
	res = capPreviewRate( m_vfw.GetCaptureWnd(), 2 );

	//start preview
	res = capPreview( m_vfw.GetCaptureWnd(), TRUE );
	if( !res )
	{
		AfxMessageBox("Error: Cant start preview");
		return;
	}

	/*
	if( m_RightDlg.GetDlgItem(IDC_BUTTON1) )
		m_RightDlg.GetDlgItem(IDC_BUTTON1)->EnableWindow(FALSE);
	if( m_RightDlg.GetDlgItem(IDC_BUTTON2) )
		m_RightDlg.GetDlgItem(IDC_BUTTON2)->EnableWindow(FALSE);

	if( m_RightDlg.GetDlgItem(IDC_BUTTON3) )
		m_RightDlg.GetDlgItem(IDC_BUTTON3)->EnableWindow(FALSE);
	if( m_RightDlg.GetDlgItem(IDC_BUTTON4) )
		m_RightDlg.GetDlgItem(IDC_BUTTON4)->EnableWindow(TRUE);
	*/
}
