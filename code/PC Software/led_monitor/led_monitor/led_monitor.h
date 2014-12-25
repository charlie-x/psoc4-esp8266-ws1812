
// led_monitor.h : main header file for the PROJECT_NAME application
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols


// Cled_monitorApp:
// See led_monitor.cpp for the implementation of this class
//

class Cled_monitorApp : public CWinApp
{
public:
	Cled_monitorApp();

// Overrides
public:
	virtual BOOL InitInstance();

// Implementation

	DECLARE_MESSAGE_MAP()
};

extern Cled_monitorApp theApp;