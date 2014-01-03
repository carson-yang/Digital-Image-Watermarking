
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
struct RGBModel 
{
	double R;
	double G;
	double B;
}BitMapData[EDGE_LENGTH][EDGE_LENGTH];

int permutation[SUM_MATRIX+10][SUBMATRIX+10];

std::string BinaryString; //rs error correct code string

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
	ON_UPDATE_COMMAND_UI(ID_UpdateEmbeded, &CImageDigitalMarkingDlg::OnUpdateEmbeded)
	ON_UPDATE_COMMAND_UI(ID_UpdateExtract, &CImageDigitalMarkingDlg::OnUpdateExtract)
	ON_COMMAND(ID_PasswordGenrate, &CImageDigitalMarkingDlg::OnPasswordGenrate)
	ON_COMMAND(ID_WaterMarkGenerate, &CImageDigitalMarkingDlg::OnWaterMarkGenerate)
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
	// Create an instance First
	CFileDialog fOpenDlg(TRUE, _T("bmp"), NULL, OFN_HIDEREADONLY|OFN_FILEMUSTEXIST, _T("BMP Files (*.bmp)|*.bmp|All Files (*.*)|*.*||"), this);

	// Initializes m_pOFN structure
	fOpenDlg.m_pOFN->lpstrTitle=_T("BitMap File");

	// Call DoModal

	if(fOpenDlg.DoModal()==IDOK)
	{
		// Get file path
		strPathName = fOpenDlg.GetPathName();
		
		myImage.Destroy();
		myImage.Load((LPCTSTR)strPathName);
		if (myImage.IsNull())
			return;
		CClientDC dc(this);
		myImage.Draw(dc.m_hDC,40,40,256,256);
	}
}


void CImageDigitalMarkingDlg::OnUpdateEmbeded(CCmdUI *pCmdUI)
{
	handleImage.SetWindowTextW(_T("嵌入水印"));
}


void CImageDigitalMarkingDlg::OnUpdateExtract(CCmdUI *pCmdUI)
{
	handleImage.SetWindowTextW(_T("提取水印"));
}

static void generateFile(LPCWSTR title,LPCWSTR result, LPCWSTR fileName,std::string& content)
{
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
void CImageDigitalMarkingDlg::OnPasswordGenrate()
{
		CString currentTime = CTime::GetCurrentTime().Format("%H, %M, %S, %A, %B %d, %Y");
		std::string stdCurrent(CW2A(currentTime.GetString()));
		MD5 md5;
		md5.reset();
		md5.update(stdCurrent);
		stdCurrent = md5.toString();

		generateFile(_T("选择密钥输出目录"),_T("生成密钥成功!"),_T("\\password.txt"),stdCurrent);
}

	//generate watermarking
void CImageDigitalMarkingDlg::OnWaterMarkGenerate()
{
	srand((unsigned int) time(NULL));
	byte watermarking[100];
	int count = 96;
	while(count)
	{
		watermarking[--count] = rand() % 256;
	}
	watermarking[96] = '\0';
	std::string result = MD5::bytesToHexString(watermarking,96);

	generateFile(_T("选择水印输出目录"),_T("生成水印成功"),_T("\\watermarking.txt"),result);
}

void CImageDigitalMarkingDlg::setPassword(CString& pass)
{
	this->password = pass;
}

void CImageDigitalMarkingDlg::setWaterMarking(CString& watermark)
{
	this->waterMark = watermark;
}

static void computePertumation(CString& initPassword)
{
	std::string temp = CStringA(initPassword);
	int count = SUM_MATRIX - 1;
	MD5 md5;

	do 
	{
		unsigned int pass = 0;
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

static void pertumate(int height, int width)
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
					std::swap(BitMapData[x+i][y+j].B,BitMapData[x+pert/8][y+pert%8].B);
				}
				count++;
		}
	}
}

static void dePertumate(int height, int width)
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
					std::swap(BitMapData[x+i][y+j].B,BitMapData[x+pert/8][y+pert%8].B);
				}
				count++;
		}
	}
}

static void DCT(int height, int width,int flag)
{
	for (int x = 0; x < height; x += 8){
		for (int y = 0; y < width; y += 8)
		{
			cv::Mat_<double> cvData(8,8);

			for (int i = 0; i< 8; ++i)
				for (int j = 0; j< 8; ++j)
				{
					cvData(i,j) = BitMapData[x+i][y+j].B;
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
					BitMapData[x+i][y+j].B = cvData(i,j);
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
	
	for (int x = 0; x< imageHeight;++x)
		for (int y = 0; y< imageWidth; ++y)
		{
			pixel = myImage.GetPixel(x,y);
			r = GetRValue(pixel);
			g = GetGValue(pixel);
			b = GetBValue(pixel);
			BitMapData[x][y].R = r;
			BitMapData[x][y].G = g;
			BitMapData[x][y].B = b;
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
static void fromBinaryStringToWaterMark()
{
	std::string additionString = BinaryString.substr(96,14);
	int count = 0;
	byte watermarking[100];
	for (int i = 1; i <= 96;++i )
	{
		additionString[count] <<= 1;
		BinaryString[i-1] |=  (additionString[count] & 0x80);
		if (0 == i % 7)
		{
			count++;
		}
		watermarking[i-1] = static_cast<byte>(BinaryString[i-1]);
	}
	
	watermarking[96] = '\0';
	std::string result = MD5::bytesToHexString(watermarking,96);

	generateFile(_T("请选择水印输出目录"),_T("水印提取成功"),_T("\\ExtractWaterMark.txt"),result);
}

//Extract watermark to BinaryString
static void fromImageToBinaryString(int height, int width)
{
	BinaryString.clear();
	ASSERT(BinaryString.length() == 0);
	char temp = 0;
	int count = 1;
	char singleBit[10]={0x00,0x80,0x40,0x20,0x10,0x08,0x04,0x02,0x01};
	int numStatics[3];

	for (int x = 0; x < height; x+= 8)
	{
		for (int y = 0; y < width; y+= 8)
		{
			bool bit = false;
			memset(numStatics,0,sizeof(numStatics));

			if (BitMapData[x][y+1].B > BitMapData[x+1][y].B)
			{
				numStatics[1]++;
			}
			else
				numStatics[0]++;

			if (BitMapData[x][y+2].B > BitMapData[x+2][y].B)
			{
				numStatics[1]++;
			}
			else
				numStatics[0]++;

			if (BitMapData[x+1][y+1].B > 0)
			{
				numStatics[1]++;
			}
			else
				numStatics[0]++;

			if (numStatics[1] > numStatics[0])
			{
				bit = true;
			}

			if (bit)
			{
				temp |= singleBit[count];
			}
	
			if (count == 8)
			{
				BinaryString.push_back(temp);
				count = 0;
				temp = 0;
			}
			count++;
			if (BinaryString.size() == 127) {
				return;
			}
		}
	}
}

static void RS_Deconde()
{
	std::string checkString = BinaryString.substr(117);
	BinaryString.erase(BinaryString.begin()+117,BinaryString.end());

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
	schifra::reed_solomon::block<code_length,fec_length> block(BinaryString,checkString);

	if (!decoder.decode(block))
	{
		AfxMessageBox(_T("Error - Critical decoding failure!"));
		return;
	}
	// decode successfully

	//117 bytes
	block.data_to_string(BinaryString); 

	fromBinaryStringToWaterMark();
}

static void fromWaterMarkToBinaryString(CString& watermark)
{
	BinaryString.clear();
	ASSERT(BinaryString.length() == 0);
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
		BinaryString.push_back(static_cast<char>(tem));

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
	//additional string
	BinaryString += additionString;
}

static void RS_Encode(CString& watermark)
{
	fromWaterMarkToBinaryString(watermark);

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

	 BinaryString += std::string(data_length - BinaryString.length(),static_cast<unsigned char>(0x00));

	 /* Instantiate RS Block For Codec */
	 schifra::reed_solomon::block<code_length,fec_length> block;

	 /* Transform message into Reed-Solomon encoded codeword */
	 if (!encoder.encode(BinaryString,block))
	 {
		 AfxMessageBox(_T("Error - Critical encoding failure!"));
		 return;
	 }

	 block.data_to_string(BinaryString);
	 std::string checkString(fec_length,' ');
	 block.fec_to_string(checkString);
	 //result's len is 127 bytes
	 BinaryString += checkString;
}

static void fromBinaryStringToImage(int height, int width)
{
	int count = 1;
	int index = 0;
	const double CC = 5;
	char singleBit[10]={0x00,0x80,0x40,0x20,0x10,0x08,0x04,0x02,0x01};

	for (int x = 0; x < height; x+= 8)
	{
		for (int y = 0; y < width; y+= 8)
		{
			bool bit = ( (BinaryString[index] & singleBit[count]) == singleBit[count] ) ? true : false;

			//Ensure that the difference is greater than CC
			if (fabs(BitMapData[x][y+2].B - BitMapData[x+2][y].B) < CC)
			{
				BitMapData[x][y+2].B < BitMapData[x+2][y].B ? BitMapData[x][y+2].B -= CC : BitMapData[x+2][y].B -= CC;
			}

			if (fabs(BitMapData[x][y+1].B - BitMapData[x+1][y].B) < CC)
			{
				BitMapData[x][y+1].B < BitMapData[x+1][y].B ? BitMapData[x][y+1].B -= CC : BitMapData[x+1][y].B -= CC;
			}

			if (bit)
			{
				if (BitMapData[x][y+2].B < BitMapData[x+2][y].B)
				{
					std::swap(BitMapData[x][y+2].B,BitMapData[x+2][y].B);
				}
				if (BitMapData[x][y+1].B < BitMapData[x+1][y].B)
				{
					std::swap(BitMapData[x][y+1].B,BitMapData[x+1][y].B);
				}
				//positive number stand for 1
				BitMapData[x+1][y+1].B = fabs(BitMapData[x+1][y+1].B)+ 0.0001;
			}
			else
			{
				if (BitMapData[x][y+2].B > BitMapData[x+2][y].B)
				{
					std::swap(BitMapData[x][y+2].B,BitMapData[x+2][y].B);
				}
				if (BitMapData[x][y+1].B > BitMapData[x+1][y].B)
				{
					std::swap(BitMapData[x][y+1].B,BitMapData[x+1][y].B);
				}
				BitMapData[x+1][y+1].B = fabs(BitMapData[x+1][y+1].B) * (-1.0);
			}

			if (0 == count % 8)
			{
				index++;
				count = 0;
			}
			count++;
			if (index == 127)   
				return;
		}
	}
}

void CImageDigitalMarkingDlg::generateEmbededWaterMarkImage()
{
	int height = myImage.GetHeight();
	int width = myImage.GetWidth();
	for (int x = 0; x < height; ++x)
	{
		for (int y = 0 ;y < width; ++ y)
		{
			BYTE B = 0;
			if (BitMapData[x][y].B > 255)
			{
				B = 255;
			}
			else if (BitMapData[x][y].B < 0)
			{
				B = 0;
			}else
				B = static_cast<BYTE>(BitMapData[x][y].B);

			myImage.SetPixelRGB(x,y,static_cast<BYTE>(BitMapData[x][y].R),static_cast<BYTE>(BitMapData[x][y].G),B);
		}
	}

	//save as an another image
	strPathName.Insert((strPathName.GetLength()-4),_T("_EmbededWaterMark"));

	myImage.Save(strPathName);

	CImage embededWaterMarkImage;
	embededWaterMarkImage.Load(strPathName);
	
	//show image
	CClientDC dc(this);
	embededWaterMarkImage.Draw(dc.m_hDC,450,40,256,256);
}

void CImageDigitalMarkingDlg::OnBnClickedButton4()
{
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
		
		fromBinaryStringToImage(myImage.GetHeight(),myImage.GetWidth());
		
		//IDCT
		DCT(myImage.GetHeight(),myImage.GetWidth (),1);

		dePertumate(myImage.GetHeight(),myImage.GetWidth());

		generateEmbededWaterMarkImage();

	} 
	else //Extract watermark
	{
		//what if Image don't contain waterMark
		fromImageToBinaryString(myImage.GetHeight(),myImage.GetWidth());

		RS_Deconde();
	}
}
