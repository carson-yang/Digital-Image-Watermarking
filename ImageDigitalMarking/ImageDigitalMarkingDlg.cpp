
// ImageDigitalMarkingDlg.cpp : implementation file
//

#include "stdafx.h"
#include <cstddef>
#include <string>
#include "ImageDigitalMarking.h"
#include "ImageDigitalMarkingDlg.h"
#include "afxdialogex.h"
#include "MD5.h"
#include "PasswordInput.h"
#include "WaterMarkingInputDlg.h"
#include "opencv2/core/core.hpp"
#include "schifra/schifra_galois_field.hpp"
#include "schifra/schifra_galois_field_polynomial.hpp"
#include "schifra/schifra_sequential_root_generator_polynomial_creator.hpp"
#include "schifra/schifra_reed_solomon_encoder.hpp"
#include "schifra/schifra_reed_solomon_decoder.hpp"
#include "schifra/schifra_reed_solomon_block.hpp"
#include "schifra/schifra_error_processes.hpp"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CImageDigitalMarkingDlg dialog


struct YCrCbModel 
{
	double Y;
	double Cr;
	double Cb;
}BitMapData[EDGE_LENGTH][EDGE_LENGTH];

int permutation[SUM_MATRIX+10][SUBMATRIX+10];

std::string RS_String; //rs error correct code string

CImageDigitalMarkingDlg::CImageDigitalMarkingDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CImageDigitalMarkingDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	//password = new char[40];
}

CImageDigitalMarkingDlg::~CImageDigitalMarkingDlg()
{
	//delete password;
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
	ON_BN_CLICKED(IDC_BUTTON4, &CImageDigitalMarkingDlg::OnBnClickedButton4)
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
		
		myImage.Destroy();
		myImage.Load((LPCTSTR)strPathName);
		if (myImage.IsNull())
			return;
		CClientDC dc(this);
		myImage.Draw(dc.m_hDC,40,40,256,256);
		//int w = myImage.GetWidth();
		//int h = myImage.GetHeight();
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

static void generateFile(LPCWSTR title,LPCWSTR result, LPCWSTR fileName,std::string& content)
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
		stdCurrent = md5.toString();

		generateFile(_T("选择密钥输出目录"),_T("生成密钥成功!"),_T("\\password.txt"),stdCurrent);
	
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

void CImageDigitalMarkingDlg::setPassword(CString& pass)
{
	this->password = pass;
}

void CImageDigitalMarkingDlg::setWaterMarking(CString& watermark)
{
	this->waterMark = watermark;
}

void computePertumation(CString& initPassword)
{
	std::string temp = CStringA(initPassword);
	int count = SUM_MATRIX - 1;
	MD5 md5;

	do 
	{
		unsigned int pass = 0;
		//sscanf_s(temp.c_str(),"%x",&pass);
		sscanf_s(temp.c_str(),"%x",&pass);
		srand(pass);
		for (int i = 0; i< SUBMATRIX; ++i)
		{
			permutation[SUM_MATRIX - count - 1][i] = rand() % SUBMATRIX;
		}
		md5.reset();
		md5.update(temp);
		temp = md5.toString();
	} while (count --);
}

void pertumate(int height, int width)
{
	int count = 0;
	for (int x = 0; x < height; x += 8){
		for (int y = 0; y < width; y += 8)
		{
			int pert = 0;
			for (int i = 0; i< 8; ++i)
				for (int j = 0; j< 8; ++j)
				{
					pert = permutation[count][i*8+j];
					std::swap(BitMapData[x+i][y+j].Cr,BitMapData[x+pert/8][y+pert%8].Cr);
				}
		}
		count++;
	}
}

void dePertumate(int height, int width)
{
	int count = 0;
	for (int x = 0; x < height; x += 8){
		for (int y = 0; y < width; y += 8)
		{
			int pert = 0;
			for (int i = 7; i>=0; --i)
				for (int j = 7; j>=0; --j)
				{
					pert = permutation[count][i*8+j];
					std::swap(BitMapData[x+i][y+j].Cr,BitMapData[x+pert/8][y+pert%8].Cr);
				}
		}
		count++;
	}
}

void DCT(int height, int width,int flag)
{
	for (int x = 0; x < height; x += 8){
		for (int y = 0; y < width; y += 8)
		{
			cv::Mat_<double> cvData(8,8);

			for (int i = 0; i< 8; ++i)
				for (int j = 0; j< 8; ++j)
				{
					cvData(i,j) = BitMapData[x+i][y+j].Cr;
				}
			
			//DCT
			if (flag == 0)
			{
				int flags = 0;
				cv::dct(cvData,cvData,flags);
			}
			else//IDCT
			{
				int flags = cv::DCT_INVERSE;
				cv::dct(cvData,cvData,flags);
			}

			//data store to bitmapdata
			for (int i = 0; i< 8; ++i)
				for (int j = 0; j< 8; ++j)
				{
					BitMapData[x+i][y+j].Cr = cvData(i,j);
				}
		}
	}
}

bool CImageDigitalMarkingDlg::commonBehaviorOfHandleImage()
{
	if (myImage.IsNull())
	{
		AfxMessageBox(_T("请先选择图片"));
		return false;
	}

	//initialize password, watermark
	password.Empty();
	ASSERT(password.GetLength() == 0);
	waterMark.Empty();
	ASSERT(waterMark.GetLength() == 0);

	//input password
	PasswordInput inputDialog;
	inputDialog.setMainDialog(this);
	inputDialog.DoModal();

	if (password.GetLength() == 0)
	{
		return false;
	}
	int imageWidth = myImage.GetWidth();
	int imageHeight = myImage.GetHeight();

	memset(BitMapData,0,sizeof(BitMapData));
	memset(permutation,0,sizeof(permutation));

	byte r, g, b;
	COLORREF pixel;
	//RGB convert to YCrCb
	for (int x = 0; x< imageHeight;++x)
		for (int y = 0; y< imageWidth; ++y)
		{
			pixel = myImage.GetPixel(x,y);
			r = GetRValue(pixel);
			g = GetGValue(pixel);
			b = GetBValue(pixel);
			BitMapData[x][y].Y = (77.0 * r  + 150 * g  + 29 * b) / 256;
			BitMapData[x][y].Cb = (-44.0 * r  - 87 * g  + 131 * b )/ 256 + 128;
			BitMapData[x][y].Cr = (131.0 * r  - 110 * g  - 21 * b) / 256 + 128;
		}
	//compute pertumation
	computePertumation(password);

	//complete pertumation
	pertumate(imageHeight,imageWidth);

	//every block DCT
	DCT(imageHeight,imageWidth,0); 

	return true;
}

//Extract watermark to Show(or output to a *.txt file)
static void fromRS_StringToWaterMark()
{
	std::string additionString = RS_String.substr(96,14);
	int count = 0;
	std::string result;
	for (int i = 1; i <= 96;++i )
	{
		additionString[count] <<= 1;
		RS_String[i-1] |=  (additionString[count] & 0x80);
		if (0 == i % 7)
		{
			count++;
		}
		char temp[10];
		sprintf_s(temp,"%x",RS_String[i-1]);
		if (RS_String[i-1] < 0)
		{
			result.push_back(temp[6]);
			result.push_back(temp[7]);
		}
		else
		{
			result.push_back(temp[0]);
			result.push_back(temp[1]);
		}
	}
	generateFile(_T("请选择水印输出目录"),_T("水印提取成功"),_T("\\ExtractWaterMark.txt"),result);
}

//Extract watermark to RS_String
static void fromImageToRS_String()
{
	RS_String.clear();
	ASSERT(RS_String.length() == 0);
}

static void RS_Deconde()
{
	//not test
	//fromImageToRS_String();

	std::string checkString = RS_String.substr(117);
	RS_String.erase(RS_String.begin()+117,RS_String.end());

	/* Finite Field Parameters */
	const std::size_t field_descriptor                 =   7;
	const std::size_t generator_polynommial_index      =   0;
	const std::size_t generator_polynommial_root_count =  10;

	/* Reed Solomon Code Parameters */
	const std::size_t code_length = 127;
	const std::size_t fec_length  =  10;
	const std::size_t data_length = code_length - fec_length;

	/* Instantiate Finite Field and Generator Polynomials */
	schifra::galois::field field(field_descriptor,
		schifra::galois::primitive_polynomial_size04,
		schifra::galois::primitive_polynomial04);

	schifra::galois::field_polynomial generator_polynomial(field);

	schifra::sequential_root_generator_polynomial_creator(field,
		generator_polynommial_index,
		generator_polynommial_root_count,
		generator_polynomial);

	/* Instantiate Decoder (Codec) */
	schifra::reed_solomon::decoder<code_length,fec_length> decoder(field,generator_polynommial_index);

	/* Instantiate RS Block For Codec */
	schifra::reed_solomon::block<code_length,fec_length> block(RS_String,checkString);

	if (!decoder.decode(block))
	{
		//std::cout << "Error - Critical decoding failure!" << std::endl;
		AfxMessageBox(_T("Error - Critical decoding failure!"));
		return;
	}
	// decode successfully

	//117 bytes
	block.data_to_string(RS_String); 

	fromRS_StringToWaterMark();
}

static void fromWaterMarkToRS_String(CString& watermark)
{
	RS_String.clear();
	ASSERT(RS_String.length() == 0);
	std::string additionString;
	unsigned char additionStringElement = 0;
	int count = 0;

	for (int i = 0;i <watermark.GetLength(); i+= 2)
	{
		//hex string to byte;
		int tem = 0;
		std::string subTemp;
		subTemp.push_back(static_cast<char>(watermark[i]));
		subTemp.push_back(static_cast<char>(watermark[i+1]));
		sscanf_s(subTemp.c_str(),"%x",&tem);
		RS_String.push_back(static_cast<char>(tem));

		//rs(127,117) only with 7 valid bit symbols
		//so every byte's head(first) bit stored in addition string
		if (1 == (tem >> 7))
		{
			additionStringElement |= 0x01;
		}
		additionStringElement <<= 1;
		count++;
		if (0 == count % 7)
		{
			additionStringElement >>= 1;
			additionString.push_back(additionStringElement);
			additionStringElement = 0;
		}
	}

	if (0 != additionStringElement)
	{
		while (0 != count % 7)
		{
			additionStringElement <<= 1;
			count++;
		}
		additionStringElement >>= 1;
		additionString.push_back(additionStringElement);
	}
	//addition string
	RS_String += additionString;
}

static void RS_Encode(CString& watermark)
{
	fromWaterMarkToRS_String(watermark);
	/* Finite Field Parameters */
	const std::size_t field_descriptor                 =   7;
	const std::size_t generator_polynommial_index      =   0;
	const std::size_t generator_polynommial_root_count =  10;

	/* Reed Solomon Code Parameters */
	const std::size_t code_length = 127;
	const std::size_t fec_length  =  10;
	const std::size_t data_length = code_length - fec_length;

	/* Instantiate Finite Field and Generator Polynomials */
	schifra::galois::field field(field_descriptor,
		schifra::galois::primitive_polynomial_size04,
		schifra::galois::primitive_polynomial04);

	schifra::galois::field_polynomial generator_polynomial(field);

	schifra::sequential_root_generator_polynomial_creator(field,
		generator_polynommial_index,
		generator_polynommial_root_count,
		generator_polynomial);

	/* Instantiate Encoder and Decoder (Codec) */
	schifra::reed_solomon::encoder<code_length,fec_length> encoder(field,generator_polynomial);

	 RS_String += std::string(data_length - RS_String.length(),static_cast<unsigned char>(0x00));

	 /* Instantiate RS Block For Codec */
	 schifra::reed_solomon::block<code_length,fec_length> block;

	 /* Transform message into Reed-Solomon encoded codeword */
	 if (!encoder.encode(RS_String,block))
	 {
		 //std::cout << "Error - Critical encoding failure!" << std::endl;
		 AfxMessageBox(_T("Error - Critical encoding failure!"));
		 return;
	 }

	 block.data_to_string(RS_String);
	 std::string checkString(fec_length,' ');
	 block.fec_to_string(checkString);
	 //result'size is 127 bytes
	 RS_String += checkString;

	 //RS_Deconde();
}

void CImageDigitalMarkingDlg::OnBnClickedButton4()
{
	// TODO: Add your control notification handler code here

	//common handle
	if (!commonBehaviorOfHandleImage())
	{
		return;
	}
	
	//different handle
	CString textContent;
	handleImage.GetWindowText(textContent);
	CString constContent(_T("嵌入水印"));
	if (textContent == constContent)
	{
		WaterMarkingInputDlg waterMarkInput;
		waterMarkInput.setMainDialog(this);
		waterMarkInput.DoModal();

		if (waterMark.GetLength() == 0)
		{
			return;
		}
		
		// rs error correct code
		RS_Encode(waterMark);

	} 
	else //提取水印
	{
		
	}

}
