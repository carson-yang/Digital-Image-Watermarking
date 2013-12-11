// WaterMarkingInputDlg.cpp : implementation file
//

#include "stdafx.h"
#include "ImageDigitalMarking.h"
#include "WaterMarkingInputDlg.h"
#include "afxdialogex.h"


// WaterMarkingInputDlg dialog

IMPLEMENT_DYNAMIC(WaterMarkingInputDlg, CDialogEx)

WaterMarkingInputDlg::WaterMarkingInputDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(WaterMarkingInputDlg::IDD, pParent)
{

}

WaterMarkingInputDlg::~WaterMarkingInputDlg()
{
}

void WaterMarkingInputDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDITWaterMarkingInput, waterMarkingInputEdit);
}


BEGIN_MESSAGE_MAP(WaterMarkingInputDlg, CDialogEx)
	ON_BN_CLICKED(IDOK, &WaterMarkingInputDlg::OnBnClickedOk)
END_MESSAGE_MAP()


// WaterMarkingInputDlg message handlers

void WaterMarkingInputDlg::setMainDialog(CImageDigitalMarkingDlg* pMain)
{
	this->pDialog = pMain;
}

void WaterMarkingInputDlg::OnBnClickedOk()
{
	// TODO: Add your control notification handler code here
	CString waterMark;
	int sumLen = 0;
	int lineIndex, nLineCount = waterMarkingInputEdit.GetLineCount();
	if (nLineCount == 0)
	{
		AfxMessageBox(_T("请输入水印"));
		return;
	}
	CString tempStr;
	for (lineIndex = 0; lineIndex < nLineCount; ++ lineIndex)
	{
		int len = waterMarkingInputEdit.LineLength(waterMarkingInputEdit.LineIndex(lineIndex));
		sumLen += len;
		waterMarkingInputEdit.GetLine(lineIndex,tempStr.GetBuffer(len),len);
		tempStr.ReleaseBuffer(len);
		waterMark.Append(tempStr);
		AFXDUMP(tempStr);
	}

	if (sumLen != 192)
	{
		AfxMessageBox(_T("请输入192个十六进制字符"));
		return;
	}
	
	waterMark.MakeLower();
	for (int i = 0; i< 192; ++i){
		TCHAR pass = waterMark.GetAt(i);
		if (!isdigit(pass) && (pass < 'a' || pass > 'f'))
		{
			AfxMessageBox(_T("密钥不正确"));
			return;
		}
	}
	pDialog->setWaterMarking(waterMark);

	CDialogEx::OnOK();
}
