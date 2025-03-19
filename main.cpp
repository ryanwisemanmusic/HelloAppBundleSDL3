/*
Author: Ryan Wiseman

This is a barebones approach to windowing via SDL3.
*/

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3/SDL_render.h>

#include <SDL3_ttf/SDL_ttf.h>
#include <SDL3_image/SDL_image.h>

#include <unistd.h>
#include <stdio.h>

#include <iostream>
#include <array>
#include <stdexcept>

SDL_Window *window = nullptr;
SDL_Renderer *renderer = nullptr;
constexpr int ScreenWidth = 600;
constexpr int ScreenHeight = 600;

bool init();
void render();
void handleEvents(bool& done);
void close();

int main(int argc, char* argv[])
{
    (void)argc;
    (void)argv;
    
    if (!init())
    {
        SDL_Log("Unable to initialize program!\n");
        return 1;
    }
    
    bool done = false;
    // Main loop for event handling – a blank window will be shown until you quit.
    while (!done)
    {
        
        handleEvents(done);
        render();
    }
    
    close();

    return 0;
}

bool init()
{
    SDL_Init(SDL_INIT_VIDEO);
    
    window = SDL_CreateWindow(
        "AtaraxiaSDK", ScreenWidth,
        ScreenHeight, SDL_WINDOW_OPENGL);
    
    if (window == NULL)
    {
        SDL_Log("Window can't be created! SDL error: %s\n",
        SDL_GetError());
        SDL_Quit();
        return false;
    }

    renderer = SDL_CreateRenderer(window, 0);
    if (!renderer)
    {
        SDL_Log("Renderer could not be created! SDL error: %s\n",
        SDL_GetError());
        SDL_Quit();
        return false;
    }
    
    return true;
}

void render()
{
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderClear(renderer); 
    SDL_RenderPresent(renderer);
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

void close()
{
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}
