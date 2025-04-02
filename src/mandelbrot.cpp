/**
 * Mandelbrot Set
 * Compilation: g++ -std=c++20 mandelbrot.cpp -o mandelbrot.o `sdl2-config --cflags --libs` 
 * SDL2 Version: libsdl2-dev:amd64 | 2.30.0+dfsg-1build3
 */

#include <SDL2/SDL.h>
#include <numeric>
#include <complex>
#include <iostream>

int is_in_set(std::complex<double> c) {
    std::complex<double> z(0.0,0.0);
    for(int i=0; i < 255; ++i) {
        z = std::pow(z,2) + c;
        if(std::norm(z) > 4) {
            return i;
        }
    }
    return 0; // Inside the set
}

int julia_is_in_set(std::complex<double> z) {
    std::complex<double> c(-1.0,0.47);
    for(int i=0; i < 255; ++i) {
        z = std::pow(z,2) + c;
        if(std::norm(z) > 10) {
            return i;
        }
    }
    return 0; // Inside the set
}

int main() {
    if(SDL_Init(SDL_INIT_EVERYTHING) != 0) {
        std::cerr << "Could not init SDL2: " << SDL_GetError() << std::endl;
        return 1;
    }
    
    SDL_Window* window = nullptr;
    SDL_Renderer* renderer = nullptr;
    SDL_CreateWindowAndRenderer(800,800,0,&window,&renderer);
    
    if(window == nullptr) {
        std::cerr << "Could not create window: " << SDL_GetError() << std::endl;
        return 1;
    }

    if(renderer == nullptr) {
        std::cerr << "Could not create renderer: " << SDL_GetError() << std::endl;
        return 1;
    }
    
    SDL_RenderSetScale(renderer,2,2);

    //std::cout << "Reached here\n";

    for(double i=0.0; i < 1.0; i+=0.001) {
        for(double j=0.0; j < 1.0; j+=0.001) {

            double x = std::lerp(-2.0, 2.0, i);
            double y = std::lerp(-2.0, 2.0, j);

            int it = is_in_set(std::complex<double> (x,y));

            if(it == 0) {
                SDL_SetRenderDrawColor(renderer,0,0,0,255);
                SDL_RenderDrawPointF(renderer, i * 400, j * 400);
            }
            else {
                SDL_SetRenderDrawColor(renderer, 
                (5 * it) % 255, 
                (0 * it) % 255, 
                (5 * it) % 255, 
                255);
                SDL_RenderDrawPointF(renderer, i * 400, j * 400);                
            }
        }
    }

    SDL_RenderPresent(renderer);

    bool running = true;
    SDL_Event event;
    while(running) {
        while(SDL_PollEvent(&event)) {
            if(event.type == SDL_QUIT) {
                running = false;
            }
        }
        SDL_Delay(10);        
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}