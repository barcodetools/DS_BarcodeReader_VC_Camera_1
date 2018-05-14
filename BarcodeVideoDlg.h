// BarcodeVideoDlg.h : header file
//

#pragma once

#include "CaptureVFW.h"


// CBarcodeVideoDlg dialog
class CBarcodeVideoDlg : public CDialog
{
// Construction
public:
	CBarcodeVideoDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_BARCODEVIDEO_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;
	CCaptureVFW			m_vfw;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedButton1();
};
