// PasswordInput.cpp : implementation file
//

#include "stdafx.h"
#include "ImageDigitalMarking.h"
#include "PasswordInput.h"
#include "afxdialogex.h"


// PasswordInput dialog

IMPLEMENT_DYNAMIC(PasswordInput, CDialogEx)

PasswordInput::PasswordInput(CWnd* pParent /*=NULL*/)
	: CDialogEx(PasswordInput::IDD, pParent)
{

}

PasswordInput::~PasswordInput()
{
}

void PasswordInput::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT1, passwordInputText);
}


BEGIN_MESSAGE_MAP(PasswordInput, CDialogEx)
	ON_BN_CLICKED(IDOK, &PasswordInput::OnBnClickedOk)
END_MESSAGE_MAP()


// PasswordInput message handlers


void PasswordInput::OnBnClickedOk()
{
	// TODO: Add your control notification handler code here
	int len = passwordInputText.LineLength();
	if (len == 0)
	{
		AfxMessageBox(_T("请输入密钥"));
		return;
	}
	if (len != 32)
	{
		AfxMessageBox(_T("密钥不正确"));
		return;
	}
	//TCHAR password[40];
	CString password;
	passwordInputText.GetLine(0,password.GetBuffer(len),len);
	password.ReleaseBuffer(len);
	password.MakeLower();
	for (int i = 0; i< 32; ++i){
		TCHAR pass = password.GetAt(i);
		if (!isdigit(pass) && (pass < 'a' || pass > 'f'))
		{
			AfxMessageBox(_T("密钥不正确"));
			return;
		}
	}
	pDialog->setPassword(password);
	CDialogEx::OnOK();
}
