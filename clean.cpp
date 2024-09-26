// HMI.cpp : implementation file
//

#include "stdafx.h"
#include "SignalAnalysis.h"
#include "HMI.h"
#include <iostream>
#include <sstream>
#include <algorithm>
#pragma comment(lib, "winmm.lib")

using namespace std;

// CHMI

IMPLEMENT_DYNCREATE(CHMI, CFormView)

CHMI::CHMI()
	: CFormView(CHMI::IDD)
{

}

CHMI::~CHMI()
{
}


void CHMI::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_STATIC_USER_MESSAGE, m_static_user_message);
	DDX_Control(pDX, IDC_STATIC_FILE, m_static_file_info);
	DDX_Control(pDX, IDC_COMBO_FFT_LEN, m_combo_fft_len);	
	DDX_Control(pDX, IDC_COMBO_SAMPLING_RATE, m_combo_sampling_rate);
	DDX_Control(pDX, IDC_BUTTON_REALTIME_AUDIO_ON, m_button_rt_audio_on);
	DDX_Control(pDX, IDC_BUTTON_REALTIME_AUDIO_OFF, m_button_rt_audio_off);
	DDX_Control(pDX, IDC_BUTTON_REPLAY_AUDIO_ON, m_button_replay_audio_on);
	DDX_Control(pDX, IDC_BUTTON_REPLAY_AUDIO_OFF, m_button_replay_audio_off);	
}

//*******************************************************************************************

BEGIN_MESSAGE_MAP(CHMI, CFormView)
	ON_BN_CLICKED(IDC_BUTTON1, &CHMI::OnButtonSet)
	ON_BN_CLICKED(IDC_BUTTON3, &CHMI::OnButtonLoad)
	ON_BN_CLICKED(IDC_BUTTON_START_REPLAY, &CHMI::OnButtonStartReplay)
	ON_BN_CLICKED(IDC_BUTTON_PAUSE_REPLAY, &CHMI::OnButtonPauseReplay)
	ON_BN_CLICKED(IDC_BUTTON_STOP_REPLAY, &CHMI::OnButtonStopReplay)
	ON_BN_CLICKED(IDC_BUTTON_RT_START, &CHMI::OnButtonStartCapture)	
	ON_BN_CLICKED(IDC_BUTTON_RT_STOP, &CHMI::OnButtonStopCapture)	
	ON_BN_CLICKED(IDC_BUTTON_REPLAY_AUDIO_ON, &CHMI::OnButtonReplayAudioOn)
	ON_BN_CLICKED(IDC_BUTTON_REPLAY_AUDIO_OFF, &CHMI::OnButtonReplayAudioOff)
	ON_BN_CLICKED(IDC_BUTTON_REALTIME_AUDIO_ON, &CHMI::OnButtonRealTimeAudioOn)
	ON_BN_CLICKED(IDC_BUTTON_REALTIME_AUDIO_OFF, &CHMI::OnButtonRealTimeAudioOff)
	ON_BN_CLICKED(IDC_BUTTON_START_REC, &CHMI::OnButtonStartRecording)
	ON_BN_CLICKED(IDC_BUTTON_STOP_REC, &CHMI::OnButtonStopRecording)	

	ON_CBN_SELCHANGE(IDC_COMBO_FFT_LEN, &CHMI::OnComboFFTLen)
	ON_CBN_SELCHANGE(IDC_COMBO_SAMPLING_RATE, &CHMI::OnComboSamplingRate)

END_MESSAGE_MAP()

//*******************************************************************************************


// CHMI diagnostics

#ifdef _DEBUG
void CHMI::AssertValid() const
{
	CFormView::AssertValid();
}

#ifndef _WIN32_WCE
void CHMI::Dump(CDumpContext& dc) const
{
	CFormView::Dump(dc);
}
#endif
#endif //_DEBUG


// CHMI message handlers

//*******************************************************************************************

void CHMI::OnComboFFTLen()
{
	switch (m_combo_fft_len.GetCurSel())
	{
		case 0:
			m_fft_length = 1024;
		break;
		case 1:
			m_fft_length = 2048;
			break;
		case 2:
			m_fft_length = 4096;
			break;
		case 3:
			m_fft_length = 8192;
			break;
		case 4:
			m_fft_length = 16384;
			break;
		case 5:
			m_fft_length = 32768;
			break;
	}

	//printf("m_fft_length %d\n", m_fft_length);
}

//*******************************************************************************************

void CHMI::OnComboSamplingRate()
{
	switch (m_combo_sampling_rate.GetCurSel())
	{
	case 0:
		m_sampling_rate = 1024;
		break;
	case 1:
		m_sampling_rate = 2048;
		break;
	}
}

//*******************************************************************************************

void CHMI::UserMessage(CString user_message, int message_type)
{
	try
	{		
		CClientDC dc (this);
		CFont font_symbol;
		font_symbol.CreateFont(20,18,0,0,FW_THIN,0,0,0,SYMBOL_CHARSET,0,0,PROOF_QUALITY,FF_DONTCARE,_T("Wingdings"));
		
		dc.SelectObject(&font_symbol);
		dc.SetBkColor(MENU);
		
		switch (message_type)
		{
		case 0://user message
			if (user_message=="erase")
			{
				//m_static_user_message.SetWindowText("");
				//dc.SetTextColor(GRAY);
				//dc.TextOut(1,1168,42);//Mail
			}//if
			else
			{
				//m_static_user_message.SetWindowText(user_message);
				//dc.SetTextColor(DARK_YELLOW);
				//dc.TextOut(1,1168,42);//Mail
			}//else
			break;
			
		case 1://System message //e.g. something is out of order
			if (user_message=="erase")
			{
				//m_static_system_message.SetWindowText("");
				//dc.SetTextColor(GRAY);
				//dc.TextOut(1,1168,42);//Mail
			}//if
			else 
			{
				//m_static_system_message.SetWindowText(user_message);
				//dc.SetTextColor(ORANGE);
				//dc.TextOut(1,1168,42);//Mail
			}//else
			break;
			
		case 2://Error
			if (user_message=="erase")
			{
				//m_static_user_message.SetWindowText("");
			}//if
			else 
			{
				//user_message.MakeUpper();
				//m_static_user_message.SetWindowText(user_message);
			}//else
			break;
			
		}//switch	
		
		//SetTimer(6,10000,NULL);
	}
	catch(CException* ex )
	{
		TCHAR  szCause[255];
		ex->GetErrorMessage(szCause, 255);
		((CSignalAnalysisApp*)AfxGetApp())->LogException(_T("HMI.cpp"),_T("UserMessage"),szCause,_T("CException"),this);
	}
	catch(std::exception* e) 
	{
		((CSignalAnalysisApp*)AfxGetApp())->LogException(_T("HMI.cpp"),_T("UserMessage"),(LPCWSTR)(e->what()),_T("std::exception"),this);
	}
	catch(...)
	{
		((CSignalAnalysisApp*)AfxGetApp())->LogException(_T("HMI.cpp"),_T("UserMessage"),_T(""),_T("..."),this);
	}		
}

//*******************************************************************************************
//void CALLBACK myWaveInProc(HWAVEIN hwi, UINT uMsg, DWORD dwInstance, DWORD dwParam1, DWORD dwParam2)
//{
//	static int _iBuf;
//	waveOutWrite(hWaveOut, &_header[_iBuf], sizeof(WAVEHDR));   // play audio
//	++_iBuf;
//	if (_iBuf == NUM_BUF)   _iBuf = 0;
//	waveInAddBuffer(hWaveIn, &_header[_iBuf], sizeof(WAVEHDR));
//}

//******************************************************************************************

void CHMI::OnInitialUpdate()
{
	CFormView::OnInitialUpdate();

	// TODO: Add your specialized code here and/or call the base class

	//Exception File
	file_name_exception = m_date_time.s_year + _T("_") + m_date_time.s_month + _T("_") + m_date_time.s_day+ _T("__") + m_date_time.s_hour + _T("_") + m_date_time.s_minute+ _T("_") + m_date_time.s_second;
	file_name_exception = file_name_exception + ".txt";
	file_name_exception.Insert(0,"Exception_data_");
			
	if (! SetCurrentDirectory(_T("C:\\config\\exception_data_record"))) 
		CreateDirectory(_T("C:\\config\\exception_data_record"),NULL);
		
	SetCurrentDirectory(_T("C:\\config\\exception_data_record"));
		
	((CSignalAnalysisApp*)AfxGetApp())->exceptionLogFile = fopen(file_name_exception,"w+t");//on console
		
	fprintf(((CSignalAnalysisApp*)AfxGetApp())->exceptionLogFile,"%s\n\n",file_name_exception);
	fflush(((CSignalAnalysisApp*)AfxGetApp())->exceptionLogFile);
	CreateConsole();

	SetInitialParameters();

	SetGUITools();	
}

//*******************************************************************************************

void CHMI::SetInitialParameters()
{
	m_fft_length = 2048;
	m_combo_fft_len.SetCurSel(m_fft_length % 1024);

	m_sampling_rate = 44100;
	m_combo_sampling_rate.SetCurSel(m_sampling_rate % 22050);
}

//*******************************************************************************************

void CHMI::SetGUITools()
{
	brush_black.CreateSolidBrush(RGB(0, 0, 0));
}

//******************************************************************************************

void CHMI::CreateConsole()
{
	AllocConsole();
	freopen("CONOUT$", "w", stdout);
	//BOOL CtrlHandler(DWORD fdwCtrlType);
	//SetConsoleCtrlHandler((PHANDLER_ROUTINE)CtrlHandler, TRUE);
}

//*******************************************************************************************

void CHMI::OnButtonSet()
{
	// TODO: Add your control notification handler code here
	m_dialog_sound.DoModal();
}

//*******************************************************************************************

void CHMI::OnButtonLoad()
{
	// TODO: Add your control notification handler code here
}

//*******************************************************************************************

void CHMI::OnButtonStartReplay()
{
	// TODO: Add your control notification handler code here
}

//*******************************************************************************************

void CHMI::OnButtonPauseReplay()
{
	// TODO: Add your control notification handler code here
}

//*******************************************************************************************

void CHMI::OnButtonStopReplay()
{
	// TODO: Add your control notification handler code here
}

//*******************************************************************************************

void CHMI::OnButtonStartCapture()
{

}

//*******************************************************************************************
void CHMI::OnButtonStopCapture()
{

}

//*******************************************************************************************

void CHMI::OnButtonReplayAudioOn()
{

}

//*******************************************************************************************

void CHMI::OnButtonReplayAudioOff()
{

}

//*******************************************************************************************

void CHMI::OnButtonRealTimeAudioOn()
{

}

//*******************************************************************************************

void CHMI::OnButtonRealTimeAudioOff()
{

}

//*******************************************************************************************

void CHMI::OnButtonStartRecording()
{

}

//*******************************************************************************************

void CHMI::OnButtonStopRecording()
{

}

//*******************************************************************************************

void CHMI::PlotData(float *data)
{
	CClientDC dc(this);
	CPen pen;
	pen.CreatePen(PS_SOLID, 1, RGB(255, 200, 200));
	dc.SelectObject(&pen);

	float max, min;

	dc.SelectObject(&brush_black);
	dc.FillRect(CRect(0, 200, 1000, 1000), &brush_black);

	for (int i = 0; i < m_fft_length / 2; i++)//or sizeof data
	{
		dc.MoveTo(i, data[i]);
		dc.LineTo(i + 1, data[i + 1]);
	}
}

//*******************************************************************************************

// HMI.h

#pragma once

#include "DateTime.h"
#include "Colors.h"
#include "Dialog_SoundDevices.h"
#include "afxwin.h"

//#include <mmsystem.h>
//#pragma comment(lib,"winmm.lib")

struct DataHolder
{
	void* pData_left_channel;
	void* pData_right_channel;
};

class CHMI : public CFormView
{
	DECLARE_DYNCREATE(CHMI)

protected:
	CHMI();           // protected constructor used by dynamic creation
	virtual ~CHMI();

public:
	enum { IDD = IDD_FORMVIEW };
#ifdef _DEBUG
	virtual void AssertValid() const;
#ifndef _WIN32_WCE
	virtual void Dump(CDumpContext& dc) const;
#endif
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//Ta2d_card m_sound_card;
	DataHolder m_data;

	DECLARE_MESSAGE_MAP()
public:
	void UserMessage(CString user_message, int message_type);
	void CreateConsole();
	afx_msg void OnComboSamplingRate();

	CDateTime m_date_time;
	CDialog_SoundDevices m_dialog_sound;
	
	CString m_strOpenFile;
	CStringA  file_name_exception;
	
	
	virtual void OnInitialUpdate();
		
	int m_fft_length, m_sampling_rate;

private:
	afx_msg void OnButtonLoad();
	afx_msg void OnButtonStartReplay();
	afx_msg void OnButtonPauseReplay();
	afx_msg void OnButtonStopReplay(); 
	afx_msg void OnButtonSet();
	afx_msg void OnButtonStartCapture();
	afx_msg void OnButtonStopCapture();
	afx_msg void OnButtonReplayAudioOn();
	afx_msg void OnButtonReplayAudioOff();
	afx_msg void OnButtonRealTimeAudioOn();
	afx_msg void OnButtonRealTimeAudioOff();
	afx_msg void OnButtonStartRecording();
	afx_msg void OnButtonStopRecording();

	void PlotData(float* data);
	void SetGUITools();
	void SetInitialParameters();
	afx_msg void OnComboFFTLen();

	CStatic m_static_user_message, m_static_file_info;
	CBrush brush_black;

	CComboBox m_combo_fft_len, m_combo_sampling_rate;
	CButton m_button_rt_audio_on, m_button_rt_audio_off, m_button_replay_audio_on, m_button_replay_audio_off;
};


