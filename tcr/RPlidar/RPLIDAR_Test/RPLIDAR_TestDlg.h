
// RPLIDAR_TestDlg.h : header file
//

#pragma once
#include "afxwin.h"


// CRPLIDAR_TestDlg dialog
class CRPLIDAR_TestDlg : public CDialogEx
{
// Construction
public:
	CRPLIDAR_TestDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_RPLIDAR_TEST_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCancel();
	afx_msg void OnBnClickedButtonStart();
	afx_msg void OnBnClickedButtonConnect();
	afx_msg void OnBnClickedButtonStop();
	afx_msg void OnCbnSelchangeComboComPort();
	CComboBox m_ComPort;
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	double m_Thetta;
	double m_Distance;
	double m_Q;
};
