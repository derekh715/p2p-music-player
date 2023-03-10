#include "wav.h"
#include <fstream>


Wav::Wav(){
    chunkSize = 0;
    numChannel = 0;
    sampleRate = 0;
    byteRate = 0;
    blockAlign = 0;
    bitPerSample = 0;
    dataSize = 0;
}

Wav::Wav(const char *path){
    chunkSize = 0;
    numChannel = 0;
    sampleRate = 0;
    byteRate = 0;
    blockAlign = 0;
    bitPerSample = 0;
    dataSize = 0;
    openWavFile(path);
}

bool Wav::openWavFile(const char *path){
    std::ifstream fin(path, std::ios::binary);

    if(!fin.is_open())
        goto fail;  // file not exist
    
    {// read RIFF chunk descriptor, Subchunk1ID
        const char descriptor[] = "RIFF....WAVEfmt ";
        for(int field=0; field<4; field++){
            for(int i=0; i<4; i++){
                unsigned char c;
                fin.read(reinterpret_cast<char*>(&c),1);
                if(field == 1){
                    chunkSize += (unsigned int)c << (i*8);
                }else{
                    if(c != descriptor[field*4+i])
                        goto fail;  // not wav file
                }
            }
        }
    }
    {// read fmt sub-chunk
        // read Subchunk1Size
        unsigned int fmtSize = 0;
        for(int i = 0; i<4; i++){
            unsigned char c;
            fin.read(reinterpret_cast<char*>(&c),1);
            fmtSize += (unsigned int)c << (i*8);
        }

        // read AudioFormat (2 byte)
        unsigned short int Af = 0;
        for(int i = 0; i<2; i++){
            unsigned char c;
            fin.read(reinterpret_cast<char*>(&c),1);
            Af += (unsigned short int)c << (i*8);
            fmtSize--;
        }
        if(Af != 1)
            goto fail;  // not integer PCM
        
        // read NumChannels (2 byte)
        for(int i = 0; i<2; i++){
            unsigned char c;
            fin.read(reinterpret_cast<char*>(&c),1);
            numChannel += (unsigned short int)c << (i*8);
            fmtSize--;
        }

        // read SampleRate (4 byte)
        for(int i = 0; i<4; i++){
            unsigned char c;
            fin.read(reinterpret_cast<char*>(&c),1);
            sampleRate += (unsigned int)c << (i*8);
            fmtSize--;
        }

        // read ByteRate (4 byte)
        for(int i = 0; i<4; i++){
            unsigned char c;
            fin.read(reinterpret_cast<char*>(&c),1);
            byteRate += (unsigned int)c << (i*8);
            fmtSize--;
        }

        // read BlockAlign (2 byte)
        for(int i = 0; i<2; i++){
            unsigned char c;
            fin.read(reinterpret_cast<char*>(&c),1);
            blockAlign += (unsigned short int)c << (i*8);
            fmtSize--;
        }

        // read BitsPerSample (2 byte)
        for(int i = 0; i<2; i++){
            unsigned char c;
            fin.read(reinterpret_cast<char*>(&c),1);
            bitPerSample += (unsigned short int)c << (i*8);
            fmtSize--;
        }
        
        // jump, in case fmtSize != 16
        fin.seekg(fmtSize,std::ios::cur);
    }
    {// read Subchunk2ID, Size
        const char id[] = "data";
        for(int i=0; i<4; i++){
            unsigned char c;
            fin.read(reinterpret_cast<char*>(&c),1);
            if(c != id[i])
                goto fail;  // cannot find data
        }
        // read Subchunk2Size
        for(int i = 0; i<4; i++){
            unsigned char c;
            fin.read(reinterpret_cast<char*>(&c),1);
            dataSize += (unsigned int)c << (i*8);
        }
    }
    const unsigned int numSample = dataSize/blockAlign;
    const unsigned int bytePerChannel = blockAlign/numChannel;
    dataChunk.resize(dataSize, 0);
    data.resize(numChannel, std::vector<int>(numSample, 0));
    for(unsigned int sp=0; sp<numSample; sp++){
        for(unsigned int ch=0; ch<numChannel; ch++){
            for(int i = 0; i<bytePerChannel; i++){
                unsigned char c;
                fin.read(reinterpret_cast<char*>(&c),1);
                dataChunk[sp*numChannel*bytePerChannel+ch*bytePerChannel+i] = c;
                data[ch][sp] += (unsigned int)c << (i*8);
                if((data[ch][sp]>>(bitPerSample-1))%2==1)
                    data[ch][sp] += int(-1)<<bitPerSample;  // fill ones
            }
        }
    }
    
    // calculate the base for getAmplitude()
    base = 1;
    for(int i = 0; i<bitPerSample-1; i++)
        base *= 2;
    
    fin.close();
    return true;
    fail:   // fail to open wav file
        fin.close();
        return false;
}

unsigned int Wav::getNumChannel(){
    return numChannel;
}

unsigned int Wav::getSampleRate(){
    return sampleRate;
}

void Wav::getDataChunk(unsigned char * a, double startsec, unsigned int bufferSize){
    for(unsigned int i = 0; i<bufferSize; i++){
        unsigned int pos = unsigned int(sampleRate*blockAlign*startsec) + i;
        a[i] = (pos < dataChunk.size()) ? dataChunk[pos] : 0;
    }
}

double Wav::getAmplitude(unsigned short int ch, double sec){
    return (double)data[ch][unsigned int(sampleRate*sec)]/base;
}
