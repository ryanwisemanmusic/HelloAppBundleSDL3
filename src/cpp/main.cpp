/*
Author: Ryan Wiseman

This is a barebones approach to windowing via SDL3.
*/

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3/SDL_render.h>

/*Third party image library for SDL3*/
#include <SDL3_ttf/SDL_ttf.h>
#include <SDL3_image/SDL_image.h>
/*Header for SQLite, if you aren't using local
databases, this can be deleted.*/
#include <sqlite3.h>

//Debug headers
#include <unistd.h>
#include <stdio.h>

//Universal libraries
#include <iostream>
#include <array>
#include <stdexcept>

//App headers
#include "SDLColors.h"

SDL_Window *window;
SDL_Renderer *renderer;
constexpr int ScreenWidth = 600;
constexpr int ScreenHeight = 600;

TTF_Font* font = nullptr;

extern "C" void cocoaBaseMenuBar();
extern "C" void openSDLWindowAboutMenu();

bool init();
void render();
void renderText(const char* message, int x, int y, SDL_Color color);
void handleEvents(bool& done);
void close();




int main(int argc, char* argv[]) {
    (void)argc; 
    (void)argv; 

    if (!init()) {
        SDL_Log("Unable to initialize program!\n");
        return 1;
    }

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
    SDL_Init(SDL_INIT_VIDEO);
    
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

void render() {
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderClear(renderer); 
    renderText("Hello World", 180, 250, cMagenta);
    SDL_RenderPresent(renderer);
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
        }
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
