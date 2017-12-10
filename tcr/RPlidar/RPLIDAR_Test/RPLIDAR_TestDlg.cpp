
// RPLIDAR_TestDlg.cpp : implementation file
//

#include "stdafx.h"
#include "RPLIDAR_Test.h"
#include "RPLIDAR_TestDlg.h"
#include "afxdialogex.h"

#include "rplidar.h" //RPLIDAR standard sdk, all-in-one header

#ifdef _DEBUG
#define new DEBUG_NEW
#endif



/***************************************************************************************/

#include <stdio.h>
#include <stdlib.h>



#ifndef _countof
#define _countof(_Array) (int)(sizeof(_Array) / sizeof(_Array[0]))
#endif

#ifdef _WIN32
#include <Windows.h>
#define delay(x)   ::Sleep(x)
#else
#include <unistd.h>
static inline void delay(_word_size_t ms){
	while (ms >= 1000){
		usleep(1000 * 1000);
		ms -= 1000;
	};
	if (ms != 0)
		usleep(ms * 1000);
}
#endif

using namespace rp::standalone::rplidar;



RPlidarDriver * drv;
bool StopDataThread = false;

int PortNumber;

CString Result;

double  thetta, distance, q;

UINT DataThread(LPVOID)
{
	CString Measurement;

	long index = 0;
	u_result     op_result;
	// fetech result and print it out...
	while (!StopDataThread) {
		index++;
		rplidar_response_measurement_node_t nodes[360 * 2];
		size_t   count = _countof(nodes);

		op_result = drv->grabScanData(nodes, count);

		if (IS_OK(op_result)) {
			drv->ascendScanData(nodes, count);
			for (int pos = 0; pos < (int)count; ++pos) {

				if (nodes[pos].sync_quality & RPLIDAR_RESP_MEASUREMENT_SYNCBIT)
			    Measurement.Format(L"S ");
				else
				Measurement.Format(L" ");

			    Result.Format(L"%s theta: %03.2f Dist: %08.2f Q: %d",
				//TRACE(L"%s theta: %03.2f Dist: %08.2f Q: %d \n",
				Measurement,
					(nodes[pos].angle_q6_checkbit >> RPLIDAR_RESP_MEASUREMENT_ANGLE_SHIFT) / 64.0f,
					nodes[pos].distance_q2 / 4.0f,
					nodes[pos].sync_quality >> RPLIDAR_RESP_MEASUREMENT_QUALITY_SHIFT);

				thetta = (nodes[pos].angle_q6_checkbit >> RPLIDAR_RESP_MEASUREMENT_ANGLE_SHIFT) / 64.0f;
				distance = nodes[pos].distance_q2 / 4.0f;
				q = nodes[pos].sync_quality >> RPLIDAR_RESP_MEASUREMENT_QUALITY_SHIFT;

				//TRACE("%ld %s\n", index, Result);
			}
		}

	}
	return 0;
}

void OnFinished()
{
	RPlidarDriver::DisposeDriver(drv);
}

bool checkRPLIDARHealth(RPlidarDriver * drv)
{
	u_result     op_result;
	rplidar_response_device_health_t healthinfo;


	op_result = drv->getHealth(healthinfo);
	if (IS_OK(op_result)) { // the macro IS_OK is the preperred way to judge whether the operation is succeed.
		TRACE("RPLidar health status : %d\n", healthinfo.status);
		if (healthinfo.status == RPLIDAR_STATUS_ERROR) {
			::AfxMessageBox(_T("Error, rplidar internal error detected. Please reboot the device to retry."));
			// enable the following code if you want rplidar to be reboot by software
			// drv->reset();
			return false;
		}
		else {
			return true;
		}

	}
	else {
		::AfxMessageBox(_T("Error, cannot retrieve the lidar health code."));// : %x\n", op_result);
		return false;
	}
}

#include <signal.h>
bool ctrl_c_pressed;
void ctrlc(int)
{
	ctrl_c_pressed = true;
}







/***************************************************************************************/

// CAboutDlg dialog used for App About

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// Dialog Data
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CRPLIDAR_TestDlg dialog



CRPLIDAR_TestDlg::CRPLIDAR_TestDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CRPLIDAR_TestDlg::IDD, pParent)
	, m_Thetta(0)
	, m_Distance(0)
	, m_Q(0)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CRPLIDAR_TestDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_COMBO_COM_PORT, m_ComPort);
	DDX_Text(pDX, IDC_EDIT1, m_Thetta);
	DDX_Text(pDX, IDC_EDIT2, m_Distance);
	DDX_Text(pDX, IDC_EDIT3, m_Q);
}

BEGIN_MESSAGE_MAP(CRPLIDAR_TestDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDOK, &CRPLIDAR_TestDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &CRPLIDAR_TestDlg::OnBnClickedCancel)
	ON_BN_CLICKED(IDC_BUTTON_START, &CRPLIDAR_TestDlg::OnBnClickedButtonStart)
	ON_BN_CLICKED(IDC_BUTTON_CONNECT, &CRPLIDAR_TestDlg::OnBnClickedButtonConnect)
	ON_BN_CLICKED(IDC_BUTTON_STOP, &CRPLIDAR_TestDlg::OnBnClickedButtonStop)
	ON_CBN_SELCHANGE(IDC_COMBO_COM_PORT, &CRPLIDAR_TestDlg::OnCbnSelchangeComboComPort)
	ON_WM_TIMER()
END_MESSAGE_MAP()


// CRPLIDAR_TestDlg message handlers

BOOL CRPLIDAR_TestDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	


	m_ComPort.SetCurSel(6);
	PortNumber = 7;


	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CRPLIDAR_TestDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CRPLIDAR_TestDlg::OnPaint()
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
HCURSOR CRPLIDAR_TestDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CRPLIDAR_TestDlg::OnBnClickedOk()
{
	// TODO: Add your control notification handler code here
	CDialogEx::OnOK();
}


void CRPLIDAR_TestDlg::OnBnClickedCancel()
{
	OnFinished();
	CDialogEx::OnCancel();
}


void CRPLIDAR_TestDlg::OnBnClickedButtonStart()
{
	SetTimer(1000, 1, NULL);

	drv->startMotor();
	// start scan...
	drv->startScan();

	StopDataThread = false();
	AfxBeginThread(DataThread, NULL);

}


void CRPLIDAR_TestDlg::OnBnClickedButtonConnect()
{

	const char * opt_com_path = NULL;
	_u32         opt_com_baudrate = 115200;
	u_result     op_result;

	/*// read serial port from the command line...
	if (argc>1) opt_com_path = argv[1]; // or set to a fixed value: e.g. "com3" 

	// read baud rate from the command line if specified...
	if (argc>2) opt_com_baudrate = strtoul(argv[2], NULL, 10);
	*/

	if (!opt_com_path) {
#ifdef _WIN32
		// use default com port
		opt_com_path = "\\\\.\\com7";
#else
		opt_com_path = "/dev/ttyUSB0";
#endif
	}
	
	switch (PortNumber)
	{
	case 1: opt_com_path = "\\\\.\\com1"; break;
	case 2: opt_com_path = "\\\\.\\com2"; break;
	case 3: opt_com_path = "\\\\.\\com3"; break;
	case 4: opt_com_path = "\\\\.\\com4"; break;
	case 5: opt_com_path = "\\\\.\\com5"; break;
	case 6: opt_com_path = "\\\\.\\com6"; break;
	case 7: opt_com_path = "\\\\.\\com7"; break;
	case 8: opt_com_path = "\\\\.\\com8"; break;
	case 9: opt_com_path = "\\\\.\\com9"; break;
	case 10: opt_com_path = "\\\\.\\com10"; break;

	}

	// create the driver instance
    drv = RPlidarDriver::CreateDriver(RPlidarDriver::DRIVER_TYPE_SERIALPORT);

	if (!drv) {
		MessageBox(_T("insufficent memory, exit"));
		exit(-2);
	}


	// make connection...
	if (IS_FAIL(drv->connect(opt_com_path, opt_com_baudrate))) {
		MessageBox(_T("Error, cannot bind to the specified serial port"));
		OnFinished();
	}

	rplidar_response_device_info_t devinfo;

	// retrieving the device info
	////////////////////////////////////////
	op_result = drv->getDeviceInfo(devinfo);

	if (IS_FAIL(op_result)) {
		MessageBox(_T("Error, cannot get device info."));
		OnFinished();
	}

	CString SN,s1;
	SN.Format(L"");

	// print out the device serial number, firmware and hardware version number..
	
	for (int pos = 0; pos < 16; ++pos) {
		s1.Format(L"%02X", devinfo.serialnum[pos]);
		SN.Append(s1);
	}
	(GetDlgItem(IDC_SERIAL_NUMBER))->SetWindowText(SN);

	printf("\n"
		"Firmware Ver: %d.%02d\n"
		"Hardware Rev: %d\n"
		, devinfo.firmware_version >> 8
		, devinfo.firmware_version & 0xFF
		, (int)devinfo.hardware_version);



	// check health...
	if (!checkRPLIDARHealth(drv)) {
		OnFinished();
	}

	//signal(SIGINT, ctrlc);

	

	


	
}


void CRPLIDAR_TestDlg::OnBnClickedButtonStop()
{
	drv->stop();
	drv->stopMotor();

	StopDataThread = true;

	
}


void CRPLIDAR_TestDlg::OnCbnSelchangeComboComPort()
{
	PortNumber = m_ComPort.GetCurSel() + 1;
}


void CRPLIDAR_TestDlg::OnTimer(UINT_PTR nIDEvent)
{
	//(GetDlgItem(IDC_DATA))->SetWindowText(Result);
	//Sleep(5);
	//Result.Format(L"%s", " ");
	m_Thetta = thetta; 
	m_Distance = distance;
	m_Q = q;
	UpdateData(FALSE);

	CDialogEx::OnTimer(nIDEvent);
}
