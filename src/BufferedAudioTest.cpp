#include <gst/gst.h>
#include <gst/app/gstappsrc.h>
#include <string>
#include <iostream>
#include <fstream>
#include "bufferedaudio.h"

int pushfile (const char *path, BufferedAudio *des, bool last = false){      
    int offset = 0, count = 0;
    while(1){
        std::ifstream input_file;
        input_file.open(path, std::ios::binary);

        guint size = 1024;
        char *buffer = new char[size];

        /* move cursor and read size to buffer */
        input_file.seekg(offset, std::ios::beg);
        input_file.read(buffer, size);

        /* wrap and push */
        des->pushBuffer(buffer, input_file.gcount());

        /* record cursor */
        offset = input_file.tellg();
        count++;
        /* check EOS(EOF here) */
        input_file.seekg(0, std::ios::end);
        if(offset==input_file.tellg()){
            offset = 0;
            break;
        }
        input_file.close();
    }
    if(last)
        des->pushEOS();
    return count;
}

int main (){
    std::cout << "Enter file extention. (.mp3 / .m4a / ...)\n";
    std::string s;
    getline(std::cin, s);

    BufferedAudio *bufaudio = new BufferedAudio(s);


    std::cout << "Enter file name.\n";

    getline(std::cin, s);
    std::cout << pushfile(s.c_str(), bufaudio) << " buffers pushed\n";

    getline(std::cin, s);
    std::cout << pushfile(s.c_str(), bufaudio) << " buffers pushed\n";

    getline(std::cin, s);
    std::cout << pushfile(s.c_str(), bufaudio, true) << " buffers pushed\n";


    /* Wait until error or EOS */
    GstElement *pipeline = bufaudio->getPipeline();
    GstMessage *msg = gst_bus_timed_pop_filtered(gst_element_get_bus(pipeline), GST_CLOCK_TIME_NONE, (GstMessageType)(GST_MESSAGE_ERROR | GST_MESSAGE_EOS));

    if (msg != NULL) gst_message_unref(msg);
    delete bufaudio;

    return 0;
}
