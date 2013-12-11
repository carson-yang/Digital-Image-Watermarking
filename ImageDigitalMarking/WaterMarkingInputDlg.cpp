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


	CDialogEx::OnOK();
}
