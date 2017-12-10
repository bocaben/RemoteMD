
// RPLIDAR_Test.h : main header file for the PROJECT_NAME application
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols


// CRPLIDAR_TestApp:
// See RPLIDAR_Test.cpp for the implementation of this class
//

class CRPLIDAR_TestApp : public CWinApp
{
public:
	CRPLIDAR_TestApp();

// Overrides
public:
	virtual BOOL InitInstance();

// Implementation

	DECLARE_MESSAGE_MAP()
};

extern CRPLIDAR_TestApp theApp;