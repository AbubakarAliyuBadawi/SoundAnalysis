#include <windows.h>
#include <mmsystem.h>
#include <vector>
#include <iostream>

#pragma comment(lib, "winmm.lib")

const int NUM_BUFFERS = 100;
const int BUFFER_SIZE = 44100 * 2; // Half a second buffer

WAVEFORMATEX wfx = {
    WAVE_FORMAT_PCM,   // format type
    1,                 // number of channels (mono)
    44100,             // sample rate
    88200,             // average bytes per second
    2,                 // block align
    16,                // bits per sample
    0                  // size of extra information
};

HWAVEIN hWaveIn;
HWAVEOUT hWaveOut;
std::vector<WAVEHDR> waveHeaders(NUM_BUFFERS);
std::vector<std::vector<char>> waveBuffers(NUM_BUFFERS, std::vector<char>(BUFFER_SIZE));
std::vector<HANDLE> waveOutEvents(NUM_BUFFERS);

void CALLBACK waveInProc(HWAVEIN hwi, UINT uMsg, DWORD_PTR dwInstance, DWORD_PTR dwParam1, DWORD_PTR dwParam2) {
    if (uMsg == WIM_DATA) {
        WAVEHDR* pwh = (WAVEHDR*)dwParam1;
        
        // Find the index of the buffer
        int index = -1;
        for (int i = 0; i < NUM_BUFFERS; i++) {
            if (waveBuffers[i].data() == pwh->lpData) {
                index = i;
                break;
            }
        }

        if (index == -1) {
            std::cerr << "Error: Buffer not found.\n";
            return; // Buffer not found, return to avoid further errors
        }

        // Process the buffer
        if (pwh->dwFlags & WHDR_DONE) {
            std::cout << "Processing buffer - " << pwh->dwBytesRecorded << " bytes\n";
            waveOutWrite(hWaveOut, pwh, sizeof(WAVEHDR));
            waveInAddBuffer(hwi, pwh, sizeof(WAVEHDR));
        }
    }
}


int main() {
    if (waveOutOpen(&hWaveOut, WAVE_MAPPER, &wfx, 0, 0, CALLBACK_NULL) != MMSYSERR_NOERROR) {
        std::cerr << "Failed to open wave output device." << std::endl;
        return -1;
    }

    if (waveInOpen(&hWaveIn, WAVE_MAPPER, &wfx, (DWORD_PTR)waveInProc, 0, CALLBACK_FUNCTION) != MMSYSERR_NOERROR) {
        std::cerr << "Failed to open wave input device." << std::endl;
        waveOutClose(hWaveOut);
        return -1;
    }

    // Prepare headers and buffers
    for (int i = 0; i < NUM_BUFFERS; ++i) {
        waveOutEvents[i] = CreateEvent(NULL, FALSE, FALSE, NULL);
        waveHeaders[i].lpData = waveBuffers[i].data();
        waveHeaders[i].dwBufferLength = BUFFER_SIZE;
        waveHeaders[i].dwFlags = 0;
        waveHeaders[i].dwLoops = 0;
        waveHeaders[i].dwUser = (DWORD_PTR)waveOutEvents[i]; // Storing the event handle in dwUser for later use

        waveInPrepareHeader(hWaveIn, &waveHeaders[i], sizeof(WAVEHDR));
        waveInAddBuffer(hWaveIn, &waveHeaders[i], sizeof(WAVEHDR));
    }

    // Start recording
    waveInStart(hWaveIn);

    std::cout << "Recording and playing back in real-time... Press Enter to stop." << std::endl;
    std::cin.get();

    // Cleanup
    waveInStop(hWaveIn);
    waveInReset(hWaveIn);
    for (int i = 0; i < NUM_BUFFERS; ++i) {
        waveInUnprepareHeader(hWaveIn, &waveHeaders[i], sizeof(WAVEHDR));
        CloseHandle(waveOutEvents[i]);
    }
    waveInClose(hWaveIn);
    waveOutClose(hWaveOut);

    return 0;
}
