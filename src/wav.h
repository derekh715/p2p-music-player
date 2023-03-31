// CSCI3280 Phase 1
// Thomas + Anthony

#include <fstream>
#include <iostream>
struct WavHeaderCDT;
typedef WavHeaderCDT* WavHeaderADT;

class Wav {
public:
    Wav();
    Wav(const char* FilePath);
    bool openWavFile(const char* FilePath);
    unsigned int getAudioFormat();
    unsigned int getNumChannels();
    unsigned int getSampleRate();
    unsigned int getByteRate();
    unsigned int getBlockAlign();
    unsigned int getBitsPerSample();
    unsigned int getSubchunk2Size();
    const char* getData();
    unsigned int getDataPos(unsigned int TimeInMilliseconds);
private:
    WavHeaderADT WavHeader;
    char* data = nullptr;
};