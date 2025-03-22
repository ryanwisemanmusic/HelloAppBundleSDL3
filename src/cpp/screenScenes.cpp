#include "screenScenes.h"
#include "SDLColors.h"

void handleMainMenu(SDL_Renderer* renderer, SceneState& currentScene)
{
    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
    SDL_RenderClear(renderer);
    SDL_RenderPresent(renderer);
}

void handleGame(SDL_Renderer* renderer, SceneState& currentScene)
{
    SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255);
    SDL_RenderClear(renderer);
    SDL_RenderPresent(renderer);
}

void handleEndScreen(SDL_Renderer* renderer, SceneState& currentScene)
{
    SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
    SDL_RenderClear(renderer);
    SDL_RenderPresent(renderer);
}

void handleLeaderboardScreen(SDL_Renderer* renderer, SceneState& currentScene)
{
    SDL_SetRenderDrawColor(renderer, 245, 245, 245, 255);
    SDL_RenderClear(renderer);
    SDL_RenderPresent(renderer);
}