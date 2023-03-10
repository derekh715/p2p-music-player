#ifndef WAV_H
#define WAV_H

#include <vector>

class Wav{
    public:
        Wav();
        Wav(const char *path);
        bool openWavFile(const char *path); // to return false if file not open
        unsigned int getNumChannel();
        unsigned int getSampleRate();

        void getDataChunk(unsigned char * a, double startsec, unsigned int bufferSize);
        double getAmplitude(unsigned short int ch, double sec);

    private:
        unsigned int chunkSize;
        unsigned short int numChannel;
        unsigned int sampleRate;
        unsigned int byteRate;  // byteRate/sampleRate = byte per sample
        unsigned short int blockAlign;  // blockAlign = byte per sample?
        unsigned short int bitPerSample;    // per sample *per channel*
        unsigned int dataSize;  // total number of bytes
        std::vector<unsigned char> dataChunk;
        std::vector<std::vector<int>> data;
        unsigned int base;  // for getAmplitude() so that no need to cal each time
};

#endif
