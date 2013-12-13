
// ImageDigitalMarkingDlg.h : header file
//

#pragma once
#include "afxwin.h"
#include <string>


// CImageDigitalMarkingDlg dialog
class CImageDigitalMarkingDlg : public CDialogEx
{
// Construction
public:
	CImageDigitalMarkingDlg(CWnd* pParent = NULL);	// standard constructor
	~CImageDigitalMarkingDlg();
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
	//char* m_pBMPBuffer;
public:
	afx_msg void OnBnClickedButton2();
//private:
	//void generateFile(LPCWSTR title,LPCWSTR result, LPCWSTR fileName,std::string& content);
	//BOOL LoadShowBMPFile(const char *pPathname);
	//HBITMAP BufferToHBITMAP();
	//void DrawImage(int x, int y, CDC *pDC);
public:
	afx_msg void OnUpdate32771(CCmdUI *pCmdUI);
	afx_msg void OnUpdate32772(CCmdUI *pCmdUI);
private:
	CButton handleImage;
	CImage myImage;
	bool commonBehaviorOfHandleImage();
	void generateEmbededWaterMarkImage();
	//char* password;
	CString password;
	CString waterMark;
	CString strPathName;
public:
	afx_msg void On32773();
	afx_msg void On32774();
	afx_msg void OnBnClickedButton4();
	void setPassword(CString&);
	void setWaterMarking(CString&);
};
