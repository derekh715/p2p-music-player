#ifndef BUFFEREDAUDIO_H
#define BUFFEREDAUDIO_H

#include <gst/gst.h>
#include <gst/app/gstappsrc.h>
#include <algorithm>
#include <string>
#include <deque>
#include <map>

class BufferedAudio;

class BufferedAudio{
    public:
        BufferedAudio(std::string ext);
        ~BufferedAudio();
        GstElement *getPipeline();
        void pushBuffer(const char *buffer, guint32 size);
        void pushEOS();

    private:
        const static std::map<std::string, std::string> decoder;

        std::deque<GstBuffer *> data;
        GstElement *pipeline, *appsrc;
        bool pipeline_paused, eos;
        
        void push_data();
        static void cb_need_data(GstAppSrc *unused_ptr, guint unused_size, gpointer user_data) {
            BufferedAudio *self = static_cast<BufferedAudio*>(user_data);
            self->push_data();
        }

        void pause_pipeline();
        static void on_queue_underrun(GstElement *queue, gpointer user_data){
            BufferedAudio *self = static_cast<BufferedAudio*>(user_data);
            self->pause_pipeline();
        }

};

#endif
