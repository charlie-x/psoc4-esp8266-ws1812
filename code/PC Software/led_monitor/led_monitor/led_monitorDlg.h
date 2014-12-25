
// led_monitorDlg.h : header file
//

#pragma once
#include "afxbutton.h"


// Cled_monitorDlg dialog
class Cled_monitorDlg : public CDialogEx
{
// Construction
public:
	Cled_monitorDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_LED_MONITOR_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	afx_msg void OnTimer( UINT_PTR nIDEvent );

// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	CMFCButton *m_List[150];
	int sock, n;
	int length;
	struct sockaddr_in recvaddr;
	struct sockaddr_in sendaddr;
	unsigned short serverPort ;

	unsigned char led_array[150 * 3];
};
