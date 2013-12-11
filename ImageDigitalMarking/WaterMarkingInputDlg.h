#pragma once
#include "afxwin.h"
#include "ImageDigitalMarkingDlg.h"

// WaterMarkingInputDlg dialog

class WaterMarkingInputDlg : public CDialogEx
{
	DECLARE_DYNAMIC(WaterMarkingInputDlg)

public:
	WaterMarkingInputDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~WaterMarkingInputDlg();

// Dialog Data
	enum { IDD = IDD_DIALOGWaterMarkInput };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
private:
	CEdit waterMarkingInputEdit;
	CImageDigitalMarkingDlg* pDialog;
public:
	afx_msg void OnBnClickedOk();
	void setMainDialog(CImageDigitalMarkingDlg*);
};
