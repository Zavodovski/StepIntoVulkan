#include <iostream>
#include "renderer.hpp"
#include "SDL.h"
#include "SDL_vulkan.h"
int main(int, char**)
{
    SDL_Init(SDL_INIT_EVERYTHING);
    SDL_Window* window = SDL_CreateWindow("hello world",
                                          SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                          800, 600,
                                          SDL_WINDOW_SHOWN | SDL_WINDOW_VULKAN);

    Renderer::Init(window);
    
    bool isquit = false;
    SDL_Event event;
    while(!isquit)
    {
        while(SDL_PollEvent(&event))
        {
            if(event.type == SDL_QUIT)
                isquit = true;
        }
    }
    
    
    Renderer::Quit();
    std::cout << "hello test" << std::endl;
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}