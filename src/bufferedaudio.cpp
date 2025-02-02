#include "bufferedaudio.h"

BufferedAudio::BufferedAudio(std::string ext){
    if(!gst_is_initialized())
        gst_init (NULL, NULL);

    std::string parse = (std::string)"appsrc name=myappsrc max-bytes=0 ! queue name=myqueue min-threshold-buffers=1 ! " + decoder.at(ext) + " ! audioconvert ! audioresample ! spectrum interval=50000000 bands=128 ! autoaudiosink";
    pipeline = gst_parse_launch(parse.c_str(), NULL);
    appsrc = gst_bin_get_by_name(GST_BIN(pipeline), "myappsrc");
    GstElement *queue = gst_bin_get_by_name(GST_BIN(pipeline), "myqueue");

    GstAppSrcCallbacks cbs;
    cbs.need_data = &BufferedAudio::cb_need_data;
    gst_app_src_set_callbacks(GST_APP_SRC_CAST(appsrc), &cbs, this, NULL);

    g_signal_connect(queue, "underrun", G_CALLBACK(&BufferedAudio::on_queue_underrun), this);
    gst_object_unref(GST_OBJECT(queue));

    gst_element_set_state (pipeline, GST_STATE_PLAYING);

    pipeline_paused = false;
    eos = false;
}

BufferedAudio::~BufferedAudio(){
    for(auto buf : data)
        if(buf != NULL)
            gst_buffer_unref(buf);
    data.clear();

    gst_object_unref(GST_OBJECT(appsrc));

    if(pipeline != NULL){
        gst_element_set_state (pipeline, GST_STATE_NULL);
        gst_object_unref (GST_OBJECT (pipeline));
    }
}

GstElement *BufferedAudio::getPipeline(){
    return pipeline;
}

void BufferedAudio::pushBuffer(const char *cbuffer, guint32 size){
    char *buffer = new char[size + 1];
    std::copy_n(cbuffer, size, buffer);
    data.push_back(gst_buffer_new_wrapped(buffer, size));
    if(pipeline_paused){
        gst_element_set_state (pipeline, GST_STATE_PLAYING);
        pipeline_paused = false;
        push_data();
    }
}

void BufferedAudio::pushEOS(){
    eos = true;
}

void BufferedAudio::push_data(){
    if(!data.empty()){
        gst_app_src_push_buffer(GST_APP_SRC(appsrc), data.front());
        data.pop_front();
    }else if(eos)
        gst_app_src_end_of_stream(GST_APP_SRC(appsrc));
}

void BufferedAudio::pause_pipeline(){
    if(data.empty()){
        gst_element_set_state (pipeline, GST_STATE_PAUSED);
        pipeline_paused = true;
    }else
        push_data();
}

const std::map<std::string, std::string> BufferedAudio::decoder = {
    {".mp3",    "decodebin"},               // OK
    {".wav",    "wavparse"},                // OK
    {".m4a",    "decodebin"},           // need whole file
    {".ogg",    "oggdemux ! vorbisdec"},    // OK
    {".flac",   "flacparse ! flacdec"}  // sometimes fail
};
