
// led_monitorDlg.cpp : implementation file
//

#include "stdafx.h"
#include "led_monitor.h"
#include "led_monitorDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// Cled_monitorDlg dialog



Cled_monitorDlg::Cled_monitorDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(Cled_monitorDlg::IDD, pParent)
{
	for (int i = 0; i < 150; i++) {
		m_List[i] = new CMFCButton;

	}


	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void Cled_monitorDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(Cled_monitorDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_TIMER()
	ON_WM_QUERYDRAGICON()
END_MESSAGE_MAP()


// Cled_monitorDlg message handlers

BOOL Cled_monitorDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon
	DWORD broadcast = 1;
	int iBsockTimeout = 5000;

	serverPort = 40002;
	sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
	setsockopt(sock, SOL_SOCKET, SO_BROADCAST,
	           (char*)&broadcast, sizeof broadcast);

	setsockopt(sock, IPPROTO_IP, IP_RECEIVE_BROADCAST, (char *)&broadcast, sizeof broadcast);
	setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (char *)&broadcast, sizeof broadcast);
	setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (char *)&iBsockTimeout, sizeof iBsockTimeout);

	memset(&sendaddr, 0, sizeof sendaddr);
	sendaddr.sin_family = PF_INET;
	sendaddr.sin_port = htons(serverPort);
	sendaddr.sin_addr.s_addr = INADDR_BROADCAST;


	memset(&recvaddr, 0, sizeof recvaddr);
	recvaddr.sin_family = PF_INET;
	recvaddr.sin_port = htons(serverPort);
	recvaddr.sin_addr.s_addr = INADDR_ANY;
	if (bind(sock, (struct sockaddr*)&recvaddr, sizeof recvaddr) == -1) {
		perror("bind");

	}


	for (int i = 0; i < 150; i++) {

		m_List[i]->Create(_T(""), WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
		                  CRect(20 + ((i - 1) * 10), 10, 20 + ((i - 1) * 10)+10, 40), this, IDC_BUTTON1 + i);
	}

	SetTimer(1,20,NULL);

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void Cled_monitorDlg::OnTimer(UINT_PTR nIDEvent)
{
	length = sizeof sendaddr;;
	n = recvfrom(sock, (char*)&led_array[0], sizeof(led_array), 0, (struct sockaddr *)&sendaddr, &length);
	if (n != -1) {
		for (int i = 0; i < 150; i++) {
			m_List[i]->SetFaceColor(RGB(led_array[i * 3], led_array[1 + (i * 3)], led_array[2 + (i * 3)]));
		}
		Invalidate(FALSE);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void Cled_monitorDlg::OnPaint()
{
	if (IsIconic()) {
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
	} else {
		CDialogEx::OnPaint();
	}

}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR Cled_monitorDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

