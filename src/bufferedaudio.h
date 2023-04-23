#ifndef BUFFEREDAUDIO_H
#define BUFFEREDAUDIO_H

#include <gst/gst.h>
#include <gst/app/gstappsrc.h>
#include <deque>

class BufferedAudio;

class BufferedAudio{
    public:
        BufferedAudio();
        ~BufferedAudio();
        GstElement *getPipeline();
        void pushBuffer(char *buffer, guint32 size);
        void pushEOS();

    private:
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
