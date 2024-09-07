// HMI.h : header file
//

#pragma once
#include <vector>
#include <windows.h>
#include <mmsystem.h>
#include "afxwin.h" // Include for MFC support

class CHMI : public CFormView
{
    DECLARE_DYNCREATE(CHMI)

protected:
    // Constructor and destructor
    CHMI(); // Protected constructor for dynamic creation
    virtual ~CHMI();

public:
    // Form ID
    enum { IDD = IDD_FORMVIEW };

#ifdef _DEBUG
    virtual void AssertValid() const;
#ifndef _WIN32_WCE
    virtual void Dump(CDumpContext& dc) const;
#endif
#endif

protected:
    // MFC DDX/DDV support for control data exchange
    virtual void DoDataExchange(CDataExchange* pDX); // DDX/DDV support

    DECLARE_MESSAGE_MAP()

public:
    // Public member functions
    void UserMessage(CString user_message, int message_type);
    void CreateConsole();
    virtual void OnInitialUpdate(); // Called for initial form setup

    // Strings for file handling
    CString m_strOpenFile;
    CStringA file_name_exception;

    // FFT length and sampling rate
    int m_fft_length;
    int m_sampling_rate;

private:
    // Audio-related member variables
    static const int NUM_BUFFERS = 100;
    static const int BUFFER_SIZE = 44100 * 2; // Half a second buffer

    WAVEFORMATEX m_wfx;                  // Audio format specification
    HWAVEIN m_hWaveIn;                   // Handle to wave input device
    HWAVEOUT m_hWaveOut;                 // Handle to wave output device
    std::vector<WAVEHDR> m_waveHeaders;  // Vector to hold wave headers
    std::vector<std::vector<char>> m_waveBuffers; // Vector to hold audio buffers
    std::vector<HANDLE> m_waveOutEvents; // Event handles for synchronization

    // Private member functions for handling audio
    void InitializeAudio();    // Initialize audio input
    void StartAudioCapture();  // Start capturing audio
    void StopAudioCapture();   // Stop capturing audio
    void CleanupAudio();       // Clean up audio resources

    // Static callback function for audio processing
    static void CALLBACK StaticWaveInProc(HWAVEIN hwi, UINT uMsg, DWORD_PTR dwInstance, DWORD_PTR dwParam1, DWORD_PTR dwParam2);

    // Instance callback handler
    void WaveInProcHandler(WAVEHDR* pwh);

    // Private member functions for button actions
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
    afx_msg void OnComboFFTLen();
    afx_msg void OnComboSamplingRate();

    // UI elements
    CStatic m_static_user_message, m_static_file_info; // Static control elements for the UI
    CBrush brush_black;                                // Black brush for drawing

    CComboBox m_combo_fft_len;      // ComboBox for FFT length selection
    CComboBox m_combo_sampling_rate; // ComboBox for sampling rate selection
    CButton m_button_rt_audio_on;   // Button for turning real-time audio on
    CButton m_button_rt_audio_off;  // Button for turning real-time audio off
    CButton m_button_replay_audio_on;  // Button for turning replay audio on
    CButton m_button_replay_audio_off; // Button for turning replay audio off
};
