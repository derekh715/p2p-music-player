// CSCI3280 Phase 1
// Thomas + Anthony

#include "wav.h"

struct WavHeaderCDT {
    uint32_t RIFF_ChunkID;
    uint32_t RIFF_ChunkSize;
    uint32_t RIFF_Format;

    uint32_t fmt_Subchunk1ID;
    uint32_t fmt_Subchunk1Size;
    uint16_t fmt_AudioFormat;
    uint16_t fmt_NumChannels;
    uint32_t fmt_SampleRate;
    uint32_t fmt_ByteRate;
    uint16_t fmt_BlockAlign;
    uint16_t fmt_BitsPerSample;

    uint32_t data_Subchunk2ID;
    uint32_t data_Subchink2Size;
};

Wav::Wav() {
    WavHeader = new WavHeaderCDT;
}

Wav::Wav(const char* FilePath) {
    WavHeader = new WavHeaderCDT;
    openWavFile(FilePath);
}

bool Wav::openWavFile(const char* FilePath) {
    std::ifstream fin;
    fin.open(FilePath, std::ios::binary);
    if (!fin.is_open()) {
        std::cerr << "Cannot Open File: " << FilePath << std::endl;
        return false;
    }
    fin.seekg(0);
    if (!fin.read((char*)WavHeader, sizeof(WavHeaderCDT))) {
        std::cerr << "Cannot Read File: " << FilePath << std::endl;
        return false;
    }
    if (
        WavHeader->RIFF_ChunkID != (uint32_t)0x46464952
        || WavHeader->RIFF_Format != (uint32_t)0x45564157
        || WavHeader->fmt_Subchunk1ID != (uint32_t)0x20746d66
        || WavHeader->fmt_AudioFormat != 1
        // || WavHeader->data_Subchunk2ID != (uint32_t)0x61746164
        ) {
        std::cerr << "Not Canonical WAV File: " << FilePath << std::endl;
        return false;
    }

    if (data == nullptr)
        data = (char*)malloc(WavHeader->data_Subchink2Size);
    else
        data = (char*)realloc(data, WavHeader->data_Subchink2Size);
    if (!fin.read((char*)data, WavHeader->data_Subchink2Size)) {
        std::cerr << "Cannot Read File Data: " << FilePath << std::endl;
        return false;
    }

    fin.close();
    return true;
}

unsigned int Wav::getNumChannels() {
    return WavHeader->fmt_NumChannels;
}

unsigned int Wav::getSampleRate() {
    return WavHeader->fmt_SampleRate;
}

unsigned int Wav::getByteRate() {
    return WavHeader->fmt_ByteRate;
}
unsigned int Wav::getBlockAlign() {
    return WavHeader->fmt_BlockAlign;
}
unsigned int Wav::getBitsPerSample() {
    return WavHeader->fmt_BitsPerSample;
}
unsigned int Wav::getSubchunk2Size() {
    return WavHeader->data_Subchink2Size;
}
const char* Wav::getData() {
    return data;
}

unsigned int Wav::getDataPos(unsigned int TimeInMilliseconds) {
    return TimeInMilliseconds / 1000 * WavHeader->fmt_ByteRate;
}

