#include "wav.h"

struct WavHeaderCDT {
    uint16_t AudioFormat;
    uint16_t NumChannels;
    uint32_t SampleRate;
    uint32_t ByteRate;
    uint16_t BlockAlign;
    uint16_t BitsPerSample;

    uint32_t dataSize;
};

Wav::Wav() {
    WavHeader = new WavHeaderCDT;
}

Wav::Wav(const char *path) {
    WavHeader = new WavHeaderCDT;
    openWavFile(path);
}

bool Wav::openWavFile(const char *path){
    // create an empty one to init.
    WavHeaderCDT init = WavHeaderCDT();
    *WavHeader = init;
    // delete if the Wav is reused (size is different)
    if(data != nullptr)
        delete data;

    std::ifstream fin(path, std::ios::binary);

    if(!fin.is_open()){
        std::cerr << "Cannot Open File: " << path << std::endl;
        goto fail;  // file not exist
    }

    fin.seekg(0);
    {// read RIFF chunk descriptor, Subchunk1ID
        const char descriptor[] = "RIFF....WAVEfmt ";   // .... is chunkSize
        for(int field=0; field<4; field++){
            for(int i=0; i<4; i++){
                unsigned char c;
                fin.read(reinterpret_cast<char*>(&c),1);
                if(field == 1){
                    // chunkSize += (uint32_t)c << (i*8);  // not reading as it's not related to the audio
                }else{
                    if(c != descriptor[field*4+i]){ // not match with "RIFF" / "WAVE" / "fmt "
                        std::cerr << "Not Canonical WAV File: " << path << std::endl;
                        goto fail;
                    }
                }
            }
        }
    }
    {// read fmt sub-chunk (subchunk1)
        // read Subchunk1Size
        uint32_t Subchunk1Size = 0;
        for(int i = 0; i<4; i++){
            unsigned char c;
            fin.read(reinterpret_cast<char*>(&c),1);
            Subchunk1Size += (uint32_t)c << (i*8);
        }
        uint32_t readCount = 0;

        // read AudioFormat (2 byte)
        for(int i = 0; i<2; i++){
            unsigned char c;
            fin.read(reinterpret_cast<char*>(&c),1);
            WavHeader->AudioFormat += (uint16_t)c << (i*8);
            readCount++;
        }
        if(WavHeader->AudioFormat != 1){    // not integer PCM
            std::cerr << "Not Canonical WAV File: " << path << std::endl;
            goto fail;
        }
        
        // read NumChannels (2 byte)
        for(int i = 0; i<2; i++){
            unsigned char c;
            fin.read(reinterpret_cast<char*>(&c),1);
            WavHeader->NumChannels += (uint16_t)c << (i*8);
            readCount++;
        }

        // read SampleRate (4 byte)
        for(int i = 0; i<4; i++){
            unsigned char c;
            fin.read(reinterpret_cast<char*>(&c),1);
            WavHeader->SampleRate += (uint32_t)c << (i*8);
            readCount++;
        }

        // read ByteRate (4 byte)
        for(int i = 0; i<4; i++){
            unsigned char c;
            fin.read(reinterpret_cast<char*>(&c),1);
            WavHeader->ByteRate += (uint32_t)c << (i*8);
            readCount++;
        }

        // read BlockAlign (2 byte)
        for(int i = 0; i<2; i++){
            unsigned char c;
            fin.read(reinterpret_cast<char*>(&c),1);
            WavHeader->BlockAlign += (uint16_t)c << (i*8);
            readCount++;
        }

        // read BitsPerSample (2 byte)
        for(int i = 0; i<2; i++){
            unsigned char c;
            fin.read(reinterpret_cast<char*>(&c),1);
            WavHeader->BitsPerSample += (uint16_t)c << (i*8);
            readCount++;
        }
        
        // jump to subchunk2 in case (Subchunk1Size != 16)
        fin.seekg(Subchunk1Size - readCount,std::ios::cur);
    }// end of fmt sub-chunk (subchunk1)
    
    {// read Subchunk2ID, Size
        // read ID
        const char id[] = "data";
        for(int i=0; i<4; i++){
            unsigned char c;
            fin.read(reinterpret_cast<char*>(&c),1);
            if(c != id[i]){ // cannot find "data"
                std::cerr << "Not Canonical WAV File: " << path << std::endl;
                goto fail;
            }
        }

        // read Subchunk2Size
        for(int i = 0; i<4; i++){
            unsigned char c;
            fin.read(reinterpret_cast<char*>(&c),1);
            WavHeader->dataSize += (uint32_t)c << (i*8);
        }
    }
    
    // read data
    data = new char[WavHeader->dataSize];
    if (!fin.read((char*)data, WavHeader->dataSize)) {
        std::cerr << "Cannot Read File Data: " << path << std::endl;
        goto fail;
    }
    
    // fin.close();    // fin will auto destruct after return (should be)
    return true;

    fail:   // fail to open wav file
        // fin.close();    // fin will auto destruct after return (should be) :)
        return false;
}

unsigned int Wav::getNumChannels() {
    return WavHeader->NumChannels;
}

unsigned int Wav::getSampleRate() {
    return WavHeader->SampleRate;
}

unsigned int Wav::getByteRate() {
    return WavHeader->ByteRate;
}
unsigned int Wav::getBlockAlign() {
    return WavHeader->BlockAlign;
}
unsigned int Wav::getBitsPerSample() {
    return WavHeader->BitsPerSample;
}
unsigned int Wav::getSubchunk2Size() {
    return WavHeader->dataSize;
}
const char* Wav::getData() {
    return data;
}

unsigned int Wav::getDataPos(unsigned int TimeInMilliseconds) {
    return TimeInMilliseconds / 1000 * WavHeader->ByteRate;
}
