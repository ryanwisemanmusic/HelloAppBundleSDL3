#ifndef VIDEO_RENDERING_H
#define VIDEO_RENDERING_H

#include <string>
#include <SDL3/SDL.h>

extern "C"
{
    #include <libavcodec/avcodec.h>
    #include <libavformat/avformat.h>
    #include <libswscale/swscale.h>
    #include <libavutil/imgutils.h>
}

extern SDL_AudioDeviceID audioDevice;
extern SDL_AudioStream* audioStream;
extern Uint8* audioBuffer;
extern Uint32 audioLength;

struct VideoState
{
    //Video Component
    AVFormatContext *pFormatCtx = nullptr;
    AVCodecContext *pCodecCtx = nullptr;
    const AVCodec *pCodec = nullptr;
    AVFrame *pFrame = nullptr;
    AVFrame *pFrameRGB = nullptr;
    SwsContext *swsCtx = nullptr;
    uint8_t *buffer = nullptr;
    int videoStream = -1;
    
    //Audio Component
    int audioStreamIndex = -1;
    SDL_AudioStream* audioStream = nullptr;
    AVCodecContext *pAudioCodecCtx = nullptr;
    const AVCodec *pAudioCodec = nullptr;
    SDL_AudioDeviceID audioDevice = 0;
    Uint8* audioBuffer = nullptr;
    Uint32 audioLength = 0;
    SDL_AudioSpec audioSpec;

    VideoState() : pFormatCtx(nullptr), pCodecCtx(nullptr), pCodec(nullptr),
                   pFrame(nullptr), pFrameRGB(nullptr), swsCtx(nullptr),
                   buffer(nullptr), videoStream(-1) {}

    ~VideoState()
    {
        if (buffer) av_free(buffer);
        if (pFrameRGB) av_frame_free(&pFrameRGB);
        if (pFrame) av_frame_free(&pFrame);
        if (pCodecCtx) avcodec_free_context(&pCodecCtx);
        if (pFormatCtx) avformat_close_input(&pFormatCtx);
        if (swsCtx) sws_freeContext(swsCtx);
        if (audioStream) SDL_DestroyAudioStream(audioStream);
        if (pAudioCodecCtx) avcodec_free_context(&pAudioCodecCtx);
    }
};


bool loadMP4(const std::string &filename, VideoState &video);
bool loadAudioFile(const std::string &filename);
void playAudio();
void cleanupAudio();
void playSFX();

#endif