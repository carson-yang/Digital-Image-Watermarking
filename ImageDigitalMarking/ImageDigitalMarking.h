
// ImageDigitalMarking.h : main header file for the PROJECT_NAME application
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols


// CImageDigitalMarkingApp:
// See ImageDigitalMarking.cpp for the implementation of this class
//

class CImageDigitalMarkingApp : public CWinApp
{
public:
	CImageDigitalMarkingApp();

// Overrides
public:
	virtual BOOL InitInstance();

// Implementation

	DECLARE_MESSAGE_MAP()
	//afx_msg void On32773();
};

extern CImageDigitalMarkingApp theApp;