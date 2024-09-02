#include <windows.h>
#include <mmsystem.h>
#include <fstream>
#include <iostream>

void writeWAVHeader(std::ofstream& outFile, unsigned int dataSize) {
    unsigned int file_size = 36 + dataSize; // 36 + Subchunk2Size
    unsigned int subchunk2_size = dataSize;
    unsigned short audio_format = 1; // PCM
    unsigned short num_channels = 1;
    unsigned int sample_rate = 44100;
    unsigned int byte_rate = sample_rate * num_channels * 2; // 16-bit audio
    unsigned short block_align = num_channels * 2;
    unsigned short bits_per_sample = 16;

    outFile.write("RIFF", 4);
    outFile.write(reinterpret_cast<char*>(&file_size), 4);
    outFile.write("WAVE", 4);
    outFile.write("fmt ", 4);

    unsigned int subchunk1_size = 16; // for PCM
    outFile.write(reinterpret_cast<char*>(&subchunk1_size), 4);
    outFile.write(reinterpret_cast<char*>(&audio_format), 2);
    outFile.write(reinterpret_cast<char*>(&num_channels), 2);
    outFile.write(reinterpret_cast<char*>(&sample_rate), 4);
    outFile.write(reinterpret_cast<char*>(&byte_rate), 4);
    outFile.write(reinterpret_cast<char*>(&block_align), 2);
    outFile.write(reinterpret_cast<char*>(&bits_per_sample), 2);

    outFile.write("data", 4);
    outFile.write(reinterpret_cast<char*>(&subchunk2_size), 4);
}



#pragma comment(lib, "winmm.lib")

// Waveform audio format specification
WAVEFORMATEX wfx = {
    WAVE_FORMAT_PCM, // format type
    1,               // number of channels (1 for mono)
    44100,           // sample rate
    44100 * 2,       // for 16-bit samples, average bytes per second
    2,               // block align; number of bytes per sample
    16,              // bits per sample
    0                // size of extra information (none)
};

// Buffer parameters
const int NUM_BUFFERS = 2;
const int BUFFER_SIZE = 44100 * 2; // Buffer for 1 second of audio

WAVEHDR headers[NUM_BUFFERS];
char buffer[NUM_BUFFERS][BUFFER_SIZE];

HWAVEIN hWaveIn;
HWAVEOUT hWaveOut;

// Callback function for handling waveIn messages
void CALLBACK waveInProc(
    HWAVEIN hwi,
    UINT uMsg,
    DWORD_PTR dwInstance,
    DWORD_PTR dwParam1,
    DWORD_PTR dwParam2
) {
    switch (uMsg) {
    case WIM_DATA: {
        PWAVEHDR pwh = (PWAVEHDR)dwParam1;
        std::ofstream outFile("output.wav", std::ios::binary | std::ios::app);
        outFile.write(pwh->lpData, pwh->dwBytesRecorded);
        outFile.close();
        waveInAddBuffer(hwi, pwh, sizeof(WAVEHDR));
        break;
    }
    }
}

int main() {
    // Open the waveform audio for input
    if (waveInOpen(&hWaveIn, WAVE_MAPPER, &wfx, (DWORD_PTR)waveInProc, 0, CALLBACK_FUNCTION) != MMSYSERR_NOERROR) {
        std::cerr << "Failed to open waveform input device." << std::endl;
        return -1;
    }

    // Prepare headers and add buffers
    std::ofstream outFile("output.wav", std::ios::binary);
    if (!outFile) {
        std::cerr << "Failed to create output file." << std::endl;
        return -1;
    }

    // Write header with a placeholder for data size
    writeWAVHeader(outFile, 0); // Placeholder, will be updated later

    for (int i = 0; i < NUM_BUFFERS; ++i) {
        headers[i].lpData = buffer[i];
        headers[i].dwBufferLength = BUFFER_SIZE;
        headers[i].dwFlags = 0;
        headers[i].dwLoops = 0;
        waveInPrepareHeader(hWaveIn, &headers[i], sizeof(WAVEHDR));
        waveInAddBuffer(hWaveIn, &headers[i], sizeof(WAVEHDR));
    }

    // Start recording
    waveInStart(hWaveIn);

    // Let it record for a few seconds
    std::cout << "Recording... Press Enter to stop." << std::endl;
    std::cin.get();

    // Stop recording and update header
    waveInStop(hWaveIn);
    for (int i = 0; i < NUM_BUFFERS; ++i) {
        waveInUnprepareHeader(hWaveIn, &headers[i], sizeof(WAVEHDR));
    }
    waveInClose(hWaveIn);

    // Move to the beginning of the file to update the data size in header
    unsigned int fileSize = static_cast<unsigned int>(outFile.tellp());
    writeWAVHeader(outFile, fileSize - 8); // Exclude RIFF chunk size

    outFile.close();

    std::cout << "Audio recorded and saved to 'output.wav'" << std::endl;

    return 0;
}
