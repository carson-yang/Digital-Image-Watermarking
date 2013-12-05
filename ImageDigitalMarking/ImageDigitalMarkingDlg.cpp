
// ImageDigitalMarkingDlg.cpp : implementation file
//

#include "stdafx.h"
#include "ImageDigitalMarking.h"
#include "ImageDigitalMarkingDlg.h"
#include "afxdialogex.h"
#include "MD5.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CImageDigitalMarkingDlg dialog




CImageDigitalMarkingDlg::CImageDigitalMarkingDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CImageDigitalMarkingDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CImageDigitalMarkingDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_BUTTON2, selectFile);
	//DDX_Control(pDX, IDC_STATIC2, sourceBitmap);
	//DDX_Control(pDX, IDC_STATIC1, destBitmap);
	DDX_Control(pDX, IDC_BUTTON4, handleImage);
}

BEGIN_MESSAGE_MAP(CImageDigitalMarkingDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON2, &CImageDigitalMarkingDlg::OnBnClickedButton2)
	ON_UPDATE_COMMAND_UI(ID_32771, &CImageDigitalMarkingDlg::OnUpdate32771)
	ON_UPDATE_COMMAND_UI(ID_32772, &CImageDigitalMarkingDlg::OnUpdate32772)
	ON_COMMAND(ID_32773, &CImageDigitalMarkingDlg::On32773)
	ON_COMMAND(ID_32774, &CImageDigitalMarkingDlg::On32774)
END_MESSAGE_MAP()


// CImageDigitalMarkingDlg message handlers

BOOL CImageDigitalMarkingDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

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

void CImageDigitalMarkingDlg::OnPaint()
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
		CDialogEx::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CImageDigitalMarkingDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CImageDigitalMarkingDlg::OnBnClickedButton2()
{
	// TODO: Add your control notification handler code here
	// Create an instance First
	CFileDialog fOpenDlg(TRUE, _T("bmp"), NULL, OFN_HIDEREADONLY|OFN_FILEMUSTEXIST, _T("BMP Files (*.bmp)|*.bmp|All Files (*.*)|*.*||"), this);

	// Initializes m_pOFN structure
	fOpenDlg.m_pOFN->lpstrTitle=_T("BitMap File");

	// Call DoModal

	if(fOpenDlg.DoModal()==IDOK)
	{
		// Get file path
		CString strPathName = fOpenDlg.GetPathName();
		
		HDC MemDC; //device enviroment
		MemDC = CreateCompatibleDC(0); //get DC
		HBITMAP bit;
		bit = (HBITMAP)LoadImage(AfxGetInstanceHandle(),strPathName,IMAGE_BITMAP,0,0,LR_LOADFROMFILE|LR_CREATEDIBSECTION);
		SelectObject(MemDC,bit);
		CClientDC dc(this);
		BitBlt(dc.m_hDC,40,40,256,256,MemDC,0,0,SRCCOPY);
	}
}


void CImageDigitalMarkingDlg::OnUpdate32771(CCmdUI *pCmdUI)
{
	// TODO: Add your command update UI handler code here
	handleImage.SetWindowTextW(_T("嵌入水印"));
}


void CImageDigitalMarkingDlg::OnUpdate32772(CCmdUI *pCmdUI)
{
	// TODO: Add your command update UI handler code here
	handleImage.SetWindowTextW(_T("提取水印"));
}

void generateFile(LPCWSTR title,LPCWSTR result, LPCWSTR fileName,std::string& content)
{
	// TODO: Add your command handler code here
	BROWSEINFO bi; 
	TCHAR pathBuffer[512]; 
	CString dir; 
	//init bi start
	bi.hwndOwner = NULL; 
	bi.pidlRoot = NULL; 
	bi.pszDisplayName = pathBuffer;//this parameter must be NoEmpty 
	bi.lpszTitle = title;
	bi.ulFlags = BIF_RETURNONLYFSDIRS; 
	bi.lpfn = NULL; 
	bi.iImage = 0; 
	//init bi end
	LPITEMIDLIST pIDList = SHBrowseForFolder(&bi);//show select dialog
	if(pIDList)//clicked OK
	{ 
		//get folder path to buffer
		SHGetPathFromIDList(pIDList, pathBuffer); 
		dir = pathBuffer; 

		//write to a disk file
		dir.Append(CString(fileName));
		CFile myFile;
		CFileException fileException;

		if ( !myFile.Open( dir, CFile::modeCreate |   
			CFile::modeWrite, &fileException ) )
		{
			TRACE( _T("Can't open file %s, error = %u\n"),
				dir, fileException.m_cause );
		}	

		myFile.Write(content.c_str(),content.size());
		myFile.Flush();
		myFile.Close();
		AfxMessageBox(CString(result));
	} 
}

//generate password
void CImageDigitalMarkingDlg::On32773()
{
	// TODO: Add your command handler code here
		//generate password
		CString currentTime = CTime::GetCurrentTime().Format("%H, %M, %S, %A, %B %d, %Y");
		std::string stdCurrent(CW2A(currentTime.GetString()));
		MD5 md5;
		md5.reset();
		md5.update(stdCurrent);
		std::string result = md5.toString();

		generateFile(_T("选择密钥输出目录"),_T("生成密钥成功!"),_T("\\password.txt"),result);
	
}

	//generate watermarking
void CImageDigitalMarkingDlg::On32774()
{
	// TODO: Add your command handler code here
	//generate watermarking
	srand((unsigned int) time(NULL));
	byte watermarking[100];
	int count = 96;
	while(count)
	{
		watermarking[--count] = rand() % 256;
	}
	watermarking[96] = '\0';
	std::string result = MD5::bytesToHexString(watermarking,96);

	generateFile(_T("选择水印输入目录"),_T("生成水印成功"),_T("\\watermarking.txt"),result);
}
