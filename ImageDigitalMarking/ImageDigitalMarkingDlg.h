
// ImageDigitalMarkingDlg.h : header file
//

#pragma once
#include "afxwin.h"


// CImageDigitalMarkingDlg dialog
class CImageDigitalMarkingDlg : public CDialogEx
{
// Construction
public:
	CImageDigitalMarkingDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_IMAGEDIGITALMARKING_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
private:
	CButton selectFile;
public:
	afx_msg void OnBnClickedButton2();
};
