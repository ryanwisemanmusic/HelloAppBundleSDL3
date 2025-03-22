#include <iostream>
#include <string>
#include <cstdint>
#include <SDL3/SDL.h>
#include <SDL3/SDL_audio.h>
#include <SDL3/SDL.h>
#include <SDL3/SDL_render.h>
#include <SDL3/SDL_video.h>
#include <SDL3/SDL_audio.h>

#include "videoRendering.h"

extern "C"
{
    #include <libavcodec/avcodec.h>
    #include <libavformat/avformat.h>
    #include <libswscale/swscale.h>
    #include <libswresample/swresample.h>
    #include <libavutil/channel_layout.h>
    #include <libswresample/swresample.h>
#include <SDL3/SDL_oldnames.h>
}

#ifndef av_get_default_channel_layout
static inline __attribute__((unused)) uint64_t av_get_default_channel_layout(int nb_channels) {
    switch(nb_channels) {
        case 1: return AV_CH_LAYOUT_MONO;
        case 2: return AV_CH_LAYOUT_STEREO;
        case 3: return AV_CH_LAYOUT_2_1;
        case 4: return AV_CH_LAYOUT_QUAD;
        case 5: return AV_CH_LAYOUT_5POINT0_BACK;
        case 6: return AV_CH_LAYOUT_5POINT1;
        default: return 0;
    }
}
#endif

// Global variable for audio device
SDL_AudioDeviceID audioDevice;
SDL_AudioStream* audioStream = nullptr;
Uint8* audioBuffer = nullptr;
Uint32 audioLength = 0;

bool loadMP4(const std::string &filename, VideoState &video)
{
    avformat_network_init();
    
    if (avformat_open_input(
        &video.pFormatCtx, filename.c_str(), nullptr, nullptr) !=0)
    {
        std::cout << "Error: Could not open video file " << 
        filename << std::endl;
        return false;
    }

    if (avformat_find_stream_info(video.pFormatCtx, nullptr) < 0)
    {
        std::cout << "Error: Could not find stream information\n";
        return false;
    }

    for (unsigned i = 0; i < video.pFormatCtx->nb_streams; i++)
    {
        if (
            video.pFormatCtx->streams[i]->codecpar->codec_type 
            == AVMEDIA_TYPE_VIDEO)
        {
            video.videoStream = i;
            break;
        } 
    }

    if (video.videoStream == -1)
    {
        std::cout << "Error: No video stream found\n";
        return false;
    }
    
    AVCodecParameters *pCodecParams = 
    video.pFormatCtx->streams[video.videoStream]->codecpar;
    video.pCodec = 
    avcodec_find_decoder(pCodecParams->codec_id);
    if (!video.pCodec)
    {
        std::cout << "Error: Unsupported codec\n";
        return false;
    }

    video.pCodecCtx = avcodec_alloc_context3(video.pCodec);
    if (avcodec_parameters_to_context(video.pCodecCtx, pCodecParams) < 0)
    {
        std::cout << "Error: Could not copy codec parameters\n";
        return false;
    }
    
    if (avcodec_open2(video.pCodecCtx, video.pCodec, nullptr) < 0)
    {
        std::cout << "Error: Could not open codec\n";
        return false;
    }
    
    std::cout << "Sucessfully loaded MP4: " << filename << std::endl;
    return true;
}

bool loadAudioFile(const std::string &filename) {
    SDL_AudioSpec wavSpec;

    cleanupAudio();
    
    SDL_Log("Attempting to load: %s", filename.c_str());

    if (!SDL_LoadWAV(filename.c_str(), &wavSpec, &audioBuffer, &audioLength)) {
        SDL_Log("Failed to load WAV file: %s", SDL_GetError());
        return false;
    }
    
    SDL_Log("WAV loaded - Format: %u, Channels: %u, Freq: %d, Size: %u bytes", 
           wavSpec.format, wavSpec.channels, wavSpec.freq, audioLength);
    
    SDL_AudioSpec deviceSpec;
    SDL_zero(deviceSpec);
    deviceSpec.freq = wavSpec.freq;
    deviceSpec.format = wavSpec.format;
    deviceSpec.channels = wavSpec.channels;
    
    audioDevice = SDL_OpenAudioDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &deviceSpec);
    if (!audioDevice) {
        SDL_Log("Failed to open audio device: %s", SDL_GetError());
        SDL_free(audioBuffer);
        audioBuffer = nullptr;
        return false;
    }
    
    audioStream = SDL_CreateAudioStream(&wavSpec, &deviceSpec);
    if (!audioStream) {
        SDL_Log("Failed to create audio stream: %s", SDL_GetError());
        SDL_CloseAudioDevice(audioDevice);
        SDL_free(audioBuffer);
        audioBuffer = nullptr;
        return false;
    }
    
    Uint32 MAX_CHUNK_SIZE = 4096;
    Uint32 processedBytes = 0;
    
    while (processedBytes < audioLength) {
        int chunkSize = (audioLength - processedBytes < MAX_CHUNK_SIZE) ? 
                         audioLength - processedBytes : MAX_CHUNK_SIZE;
        
        int result = SDL_PutAudioStreamData(audioStream, 
                                           audioBuffer + processedBytes, 
                                           chunkSize);
        
        if (result < 0) {
            SDL_Log("Error queueing audio chunk: %s", SDL_GetError());
            SDL_DestroyAudioStream(audioStream);
            audioStream = nullptr;
            SDL_CloseAudioDevice(audioDevice);
            SDL_free(audioBuffer);
            audioBuffer = nullptr;
            return false;
        }
        
        processedBytes += chunkSize;
    }
    
    SDL_FlushAudioStream(audioStream);
    return true;
}


void playAudio() {
    SDL_Log("Inside playAudio() function");

    if (!audioDevice) {
        SDL_Log("ERROR: No audio device available");
        return;
    }
    
    if (!audioStream) {
        SDL_Log("ERROR: No audio stream available");
        return;
    }

    SDL_Log("Flushing audio stream before binding...");
    SDL_FlushAudioStream(audioStream);

    SDL_Log("Binding audio stream to device %u", audioDevice);
    
    int result = SDL_BindAudioStream(audioDevice, audioStream);
    if (result != 0) {
        SDL_Log("ERROR: Failed to bind audio stream: %s (Error code: %d)", SDL_GetError(), result);
        return;
    }
    
}

//This can be used to explicitly call each SFX. Less modular than what I want
//But it works
void playSFX()
{
    SDL_Log("Inside playAudio() function");

    std::string audioPath = "assets/audio/blip.wav";
    SDL_Log("Reloading audio file: %s", audioPath.c_str());

    if (!loadAudioFile(audioPath)) {
        SDL_Log("ERROR: Failed to reload audio file.");
        return;
    }

    SDL_Log("Audio file reloaded successfully, playing now...");

    if (!audioDevice) {
        SDL_Log("ERROR: No audio device available");
        return;
    }
    
    if (!audioStream) {
        SDL_Log("ERROR: No audio stream available");
        return;
    }

    SDL_Log("Flushing audio stream before binding...");
    SDL_FlushAudioStream(audioStream);

    SDL_Log("Binding audio stream to device %u", audioDevice);
    
    int result = SDL_BindAudioStream(audioDevice, audioStream);
    if (result != 0) {
        SDL_Log("ERROR: Failed to bind audio stream: %s (Error code: %d)", SDL_GetError(), result);
        return;
    }

    SDL_Log("Audio should now be playing.");
    cleanupAudio();
}

void cleanupAudio() {
    SDL_Log("Cleaning up audio resources");
    
    if (audioStream) {
        SDL_Log("Destroying audio stream");
        SDL_DestroyAudioStream(audioStream);
        audioStream = nullptr;
    }

    if (audioDevice) {
        SDL_Log("Closing audio device %u", audioDevice);
        SDL_CloseAudioDevice(audioDevice);
        audioDevice = 0;
    }

    if (audioBuffer) {
        SDL_Log("Freeing audio buffer");
        SDL_free(audioBuffer);
        audioBuffer = nullptr;
    }
    
    SDL_Log("Audio cleanup complete");
}

bool testAudioPlayback() {
    SDL_Log("===== TESTING AUDIO PLAYBACK =====");
    
    std::string audioPath = "assets/video/NeverGonna.wav";
    SDL_Log("Testing audio file: %s", audioPath.c_str());
    
    SDL_IOStream* file = SDL_IOFromFile(audioPath.c_str(), "rb");
    if (!file) {
        SDL_Log("TEST FAILED: Audio file not found at: %s", audioPath.c_str());
        return false;
    }
    SDL_CloseIO(file);
    SDL_Log("TEST: File exists");
    
    if (!loadAudioFile(audioPath)) {
        SDL_Log("TEST FAILED: Could not load audio file");
        return false;
    }
    SDL_Log("TEST: Audio file loaded successfully");
    
    playAudio();
    SDL_Log("TEST: Audio playback initiated");
    
    SDL_Log("TEST: Waiting for 5 seconds to let audio play...");
    SDL_Delay(5000);
    
    cleanupAudio();
    SDL_Log("TEST: Audio resources cleaned up");
    
    return true;
}