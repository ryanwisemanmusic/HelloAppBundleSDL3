/*
Author: Ryan Wiseman

This is a barebones approach to windowing via SDL3.
*/

#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <SDL3/SDL_main.h>
#include <SDL3_image/SDL_image.h>
#include <SDL3/SDL_video.h>
#include <SDL3/SDL_surface.h>
#include <SDL3/SDL_render.h>
#include <SDL3/SDL_audio.h>
#include <sqlite3.h>
#include <iostream>
#include <array>

//App headers
#include "SDLColors.h"
#include "videoRendering.h"

extern "C" {
    #include <libavcodec/avcodec.h>
    #include <libavformat/avformat.h>
    #include <libswscale/swscale.h>
}

SDL_Window *window;
SDL_Renderer *renderer;
constexpr int ScreenWidth = 600;
constexpr int ScreenHeight = 600;

TTF_Font* font = nullptr;

bool audioInitialized = false;

extern "C" void cocoaBaseMenuBar();
extern "C" void openSDLWindowAboutMenu();

bool init();
bool initAudio(VideoState &video);
void render();
void renderText(const char* message, int x, int y, SDL_Color color);
void handleEvents(bool& done);
void close();

//External functions

int main(int argc, char* argv[]) {
    (void)argc; 
    (void)argv; 

    if (!init()) {
        SDL_Log("Unable to initialize program!\n");
        return 1;
    }

    cocoaBaseMenuBar();

    bool done = false;

    // Main loop for window event handling
    while (!done) 
    {
        handleEvents(done);
        render();
    }

    close();

    return 0;
}

bool init() {
    // Initialize SDL and TTF
    TTF_Init();
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
    
    window = SDL_CreateWindow(
        "AtaraxiaSDK", ScreenWidth,
        ScreenHeight, SDL_WINDOW_OPENGL);
    
    if (window == NULL) {
        SDL_Log("Window can't be created! SDL error: %s\n",
        SDL_GetError());
        SDL_Quit();
        return false;
    }

    renderer = SDL_CreateRenderer(window, 0);
    if (!renderer) {
        SDL_Log("Renderer could not be created! SDL error: %s\n",
        SDL_GetError());
        SDL_Quit();
        return false;
    }
    
    // Load font
    std::string fontPath = "assets/fonts/ArianaVioleta.ttf";
    font = TTF_OpenFont(fontPath.c_str(), 50);
    if (!font) {
        SDL_Log("Cannot load font!");
    }
    return true;
}

bool initAudio(VideoState &video) {
    SDL_AudioSpec wantedSpec, obtainedSpec;
    SDL_zero(wantedSpec); 
    wantedSpec.freq = 44100;
    wantedSpec.format = SDL_AUDIO_S16;
    wantedSpec.channels = 2;
    
    video.audioDevice = SDL_OpenAudioDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &wantedSpec);
    if (video.audioDevice == 0) {
        SDL_Log("Error: Could not open audio device: %s", SDL_GetError());
        return false;
    }
    
    video.audioSpec = obtainedSpec;
    SDL_ResumeAudioDevice(video.audioDevice);
    return true;
}

void render() {
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderClear(renderer); 
    renderText("Hello World", 180, 250, cMagenta);
    SDL_RenderPresent(renderer);

    // Only try to initialize audio once
    static bool attemptedAudioInit = false;
    
    if (!audioInitialized && !attemptedAudioInit) {
        attemptedAudioInit = true; // Mark that we've tried, regardless of success
        std::string audioPath = "assets/video/CatSpin.wav";
        SDL_Log("Attempting to load audio from: %s", audioPath.c_str());
        if (loadAudioFile(audioPath)) {
            SDL_Log("Audio file loaded successfully, attempting playback...");
            playAudio();
            audioInitialized = true;
            SDL_Log("Audio initialized and playing!");
        } else {
            SDL_Log("Failed to initialize audio device. Will not retry.");
        }
    }
}

void renderText(const char* message, int x, int y, SDL_Color color) {
    if (!font) {
        SDL_Log("Cannot load font!");
        return;
    }
    size_t messageLength = strlen(message);
    SDL_Surface* textSurface = TTF_RenderText_Solid(font, message, messageLength, color);
    SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
    int textW = textSurface->w; 
    int textH = textSurface->h;
    SDL_DestroySurface(textSurface);
    if (!textTexture) {
        SDL_Log("Texture creation failed!");
        return;
    }
    SDL_FRect destRect = { static_cast<float>(x), static_cast<float>(y), static_cast<float>(textW), static_cast<float>(textH) };
    SDL_RenderTexture(renderer, textTexture, nullptr, &destRect);
    SDL_DestroyTexture(textTexture);
}
void handleEvents(bool& done)
{
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        if (event.type == SDL_EVENT_QUIT)
        {
            done = true;
            cleanupAudio();
            audioInitialized = false;
        }
        // Don't clean up audio for other events
    }
}


void close() {
    if (font) {
        TTF_CloseFont(font);
        font = nullptr;
    }
    
    TTF_Quit();
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}
