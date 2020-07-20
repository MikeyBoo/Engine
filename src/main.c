#include <stdio.h>
#include <stdbool.h>
#include <SDL2/SDL.h>

bool is_running = false;
SDL_Window *window = NULL;
SDL_Renderer *renderer = NULL;

bool initialize_window(void) {
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
        fprintf(stderr, "Error initializing SDL.\n");
        return false;
    }
    //create window
    SDL_Window *window = SDL_CreateWindow(
        NULL,
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        800,
        600,
        SDL_WINDOW_BORDERLESS
    );
    if (!window) {
        fprintf(stderr, "Error creating SDL window.\n");
        return false;
    }
    //create render
    renderer = SDL_CreateRenderer(window, -1, 0);
    if (!renderer) {
        fprintf(stderr, "Error creating SDL Render.\n");
        return false;
    }

    return true;
}

int main(void) {
    
    is_running = initialize_window();

    return 0;
}