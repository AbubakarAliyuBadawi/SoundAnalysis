// HMI.cpp : implementation file
//

#include "stdafx.h"
#include "SignalAnalysis.h"
#include "HMI.h"
#include <iostream>
#include <sstream>
#include <algorithm>
#include <vector>
#include <cstring> // For memset

#pragma comment(lib, "winmm.lib")

// CHMI

IMPLEMENT_DYNCREATE(CHMI, CFormView)

CHMI::CHMI()
    : CFormView(CHMI::IDD),
    m_hWaveIn(NULL),
    m_hWaveOut(NULL),
    m_waveHeaders(NUM_BUFFERS),
    m_waveBuffers(NUM_BUFFERS, std::vector<char>(BUFFER_SIZE)),
    m_waveOutEvents(NUM_BUFFERS)
{
    // Initialize WAVEFORMATEX structure
    memset(&m_wfx, 0, sizeof(WAVEFORMATEX));
    m_wfx.wFormatTag = WAVE_FORMAT_PCM;       // Format type
    m_wfx.nChannels = 1;                      // Number of channels (mono)
    m_wfx.nSamplesPerSec = 44100;             // Sample rate
    m_wfx.nAvgBytesPerSec = 88200;            // Average bytes per second
    m_wfx.nBlockAlign = 2;                    // Block align
    m_wfx.wBitsPerSample = 16;                // Bits per sample
    m_wfx.cbSize = 0;                         // Size of extra information
}

CHMI::~CHMI()
{
    CleanupAudio();  // Ensure all audio resources are released
}

void CALLBACK CHMI::StaticWaveInProc(HWAVEIN hwi, UINT uMsg, DWORD_PTR dwInstance, DWORD_PTR dwParam1, DWORD_PTR dwParam2)
{
    // Retrieve the CHMI instance from dwInstance
    CHMI* pThis = reinterpret_cast<CHMI*>(dwInstance);
    if (pThis)
    {
        if (uMsg == WIM_DATA)
        {
            WAVEHDR* pwh = reinterpret_cast<WAVEHDR*>(dwParam1);
            if (pwh)
            {
                pThis->WaveInProcHandler(pwh);
            }
            else
            {
                std::cerr << "Error: WAVEHDR pointer is null in callback.\n";
            }
        }
    }
    else
    {
        std::cerr << "Error: CHMI instance is null in callback.\n";
    }
}

void CHMI::WaveInProcHandler(WAVEHDR* pwh)
{
    if (pwh->dwFlags & WHDR_DONE)
    {
        // Find the index of the buffer
        int index = -1;
        for (int i = 0; i < NUM_BUFFERS; ++i)
        {
            if (m_waveBuffers[i].data() == pwh->lpData)
            {
                index = i;
                break;
            }
        }

        if (index == -1)
        {
            std::cerr << "Error: Buffer not found.\n";
            return; // Buffer not found, avoid further processing
        }

        // Process the buffer (e.g., send to output device)
        MMRESULT writeResult = waveOutWrite(m_hWaveOut, pwh, sizeof(WAVEHDR));
        if (writeResult != MMSYSERR_NOERROR)
        {
            std::cerr << "Error: waveOutWrite failed with error code " << writeResult << ".\n";
        }

        // Re-add the buffer to the input queue for continuous recording
        MMRESULT addResult = waveInAddBuffer(m_hWaveIn, pwh, sizeof(WAVEHDR));
        if (addResult != MMSYSERR_NOERROR)
        {
            std::cerr << "Error: waveInAddBuffer failed with error code " << addResult << ".\n";
        }
    }
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

BEGIN_MESSAGE_MAP(CHMI, CFormView)
    ON_BN_CLICKED(IDC_BUTTON_RT_START, &CHMI::OnButtonStartCapture)
    ON_BN_CLICKED(IDC_BUTTON_RT_STOP, &CHMI::OnButtonStopCapture)
    ON_BN_CLICKED(IDC_BUTTON_REALTIME_AUDIO_ON, &CHMI::OnButtonRealTimeAudioOn)
    ON_BN_CLICKED(IDC_BUTTON_REALTIME_AUDIO_OFF, &CHMI::OnButtonRealTimeAudioOff)
    ON_CBN_SELCHANGE(IDC_COMBO_FFT_LEN, &CHMI::OnComboFFTLen)
    ON_CBN_SELCHANGE(IDC_COMBO_SAMPLING_RATE, &CHMI::OnComboSamplingRate)
END_MESSAGE_MAP()

// Initialize Audio Devices and Buffers
void CHMI::InitializeAudio()
{
    MMRESULT result;

    // Open wave output device
    result = waveOutOpen(&m_hWaveOut, WAVE_MAPPER, &m_wfx, 0, 0, CALLBACK_NULL);
    if (result != MMSYSERR_NOERROR)
    {
        AfxMessageBox(_T("Failed to open wave output device."));
        return;
    }

    // Open wave input device with callback
    result = waveInOpen(&m_hWaveIn, WAVE_MAPPER, &m_wfx, (DWORD_PTR)StaticWaveInProc, (DWORD_PTR)this, CALLBACK_FUNCTION);
    if (result != MMSYSERR_NOERROR)
    {
        AfxMessageBox(_T("Failed to open wave input device."));
        waveOutClose(m_hWaveOut); // Cleanup waveOut if waveIn fails
        return;
    }

    // Prepare headers and buffers
    for (int i = 0; i < NUM_BUFFERS; ++i)
    {
        // Create event for synchronization (optional, can be used for more advanced handling)
        m_waveOutEvents[i] = CreateEvent(NULL, FALSE, FALSE, NULL);
        if (m_waveOutEvents[i] == NULL)
        {
            std::cerr << "Failed to create event for buffer " << i << ".\n";
            continue; // Proceed to the next buffer
        }

        // Initialize WAVEHDR structure
        m_waveHeaders[i].lpData = m_waveBuffers[i].data();               // Pointer to buffer data
        m_waveHeaders[i].dwBufferLength = BUFFER_SIZE;                   // Size of the buffer
        m_waveHeaders[i].dwFlags = 0;                                    // No flags initially
        m_waveHeaders[i].dwLoops = 0;                                    // No looping
        m_waveHeaders[i].dwUser = reinterpret_cast<DWORD_PTR>(m_waveOutEvents[i]); // Store event handle

        // Prepare the header for input device
        result = waveInPrepareHeader(m_hWaveIn, &m_waveHeaders[i], sizeof(WAVEHDR));
        if (result != MMSYSERR_NOERROR)
        {
            std::cerr << "Failed to prepare wave header " << i << " with error: " << result << ".\n";
            CloseHandle(m_waveOutEvents[i]); // Clean up the event handle
            m_waveOutEvents[i] = NULL;
            continue; // Try the next buffer
        }

        // Add buffer to the input queue
        result = waveInAddBuffer(m_hWaveIn, &m_waveHeaders[i], sizeof(WAVEHDR));
        if (result != MMSYSERR_NOERROR)
        {
            std::cerr << "Failed to add buffer " << i << " to wave input with error: " << result << ".\n";
            waveInUnprepareHeader(m_hWaveIn, &m_waveHeaders[i], sizeof(WAVEHDR)); // Unprepare header
            CloseHandle(m_waveOutEvents[i]); // Clean up the event handle
            m_waveOutEvents[i] = NULL;
            continue; // Try the next buffer
        }
    }

    // Start recording
    result = waveInStart(m_hWaveIn);
    if (result != MMSYSERR_NOERROR)
    {
        AfxMessageBox(_T("Failed to start wave input device."));
    }
}

// Stop Audio Capture and Playback in GUI
void CHMI::StopAudioCapture()
{
    if (m_hWaveIn)
    {
        // Stop recording
        MMRESULT result = waveInStop(m_hWaveIn);
        if (result != MMSYSERR_NOERROR)
        {
            std::cerr << "Failed to stop wave input device with error: " << result << ".\n";
        }

        // Reset the input device to flush the buffers
        result = waveInReset(m_hWaveIn);
        if (result != MMSYSERR_NOERROR)
        {
            std::cerr << "Failed to reset wave input device with error: " << result << ".\n";
        }

        // Unprepare headers and close handles
        for (int i = 0; i < NUM_BUFFERS; ++i)
        {
            if (m_waveHeaders[i].lpData)
            {
                waveInUnprepareHeader(m_hWaveIn, &m_waveHeaders[i], sizeof(WAVEHDR));
                m_waveHeaders[i].lpData = NULL;
            }

            if (m_waveOutEvents[i])
            {
                CloseHandle(m_waveOutEvents[i]);
                m_waveOutEvents[i] = NULL;
            }
        }

        // Close wave input device
        result = waveInClose(m_hWaveIn);
        if (result != MMSYSERR_NOERROR)
        {
            std::cerr << "Failed to close wave input device with error: " << result << ".\n";
        }
        m_hWaveIn = NULL;
    }

    if (m_hWaveOut)
    {
        // Stop playback immediately and flush all pending buffers
        MMRESULT result = waveOutReset(m_hWaveOut);
        if (result != MMSYSERR_NOERROR)
        {
            std::cerr << "Failed to reset wave output device with error: " << result << ".\n";
        }

        // Close wave output device
        result = waveOutClose(m_hWaveOut);
        if (result != MMSYSERR_NOERROR)
        {
            std::cerr << "Failed to close wave output device with error: " << result << ".\n";
        }
        m_hWaveOut = NULL;
    }
}


// Cleanup Audio Resources
void CHMI::CleanupAudio()
{
    StopAudioCapture(); // Ensure capture is stopped and resources are released
}

// Start Audio Capture (Triggered by Button)
void CHMI::OnButtonStartCapture()
{
    InitializeAudio();
    // waveInStart(m_hWaveIn); // Already started in InitializeAudio()
}

// Stop Audio Capture (Triggered by Button)
void CHMI::OnButtonStopCapture()
{
    StopAudioCapture();
}

// Additional Methods (Real-time Audio, Replay, etc.)
void CHMI::OnButtonRealTimeAudioOn()
{
    // Implementation for starting real-time audio (if different from capture)
}

void CHMI::OnButtonRealTimeAudioOff()
{
    // Implementation for stopping real-time audio
}

// Combo Handlers for FFT Length and Sampling Rate
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
    default:
        m_fft_length = 1024; // Default value
        break;
    }
}

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
    default:
        m_sampling_rate = 1024; // Default value
        break;
    }
}

void CHMI::OnInitialUpdate()
{
    CFormView::OnInitialUpdate();
    // Additional initialization code
}

void CHMI::UserMessage(CString user_message, int message_type)
{
    // Implementation for showing a user message
}
