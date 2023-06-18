#include <iostream>
#include <iomanip>
#include <fstream>
#include <vector>

#include <SDL2/SDL.h>

extern uint64_t buffer[];

using namespace std;

const int SCREEN_WIDTH = 224;
const int SCREEN_HEIGHT = 256;
const int FRAME_BUFFER = 7168;

const SDL_Rect FRAME{(SCREEN_WIDTH - SCREEN_HEIGHT), (SCREEN_HEIGHT - SCREEN_WIDTH), SCREEN_HEIGHT * 2, SCREEN_WIDTH * 2};

uint8_t reverse_bits(uint8_t data)
{
    data = (data << 1 & 0x10) | (data >> 1 & 0x08) | (data & 0xE7);
    data = (data << 3 & 0x20) | (data >> 3 & 0x04) | (data & 0xDB);
    data = (data << 5 & 0x40) | (data >> 5 & 0x02) | (data & 0xBD);
    data = (data << 7 & 0x80) | (data >> 7 & 0x01) | (data & 0x7e);
    return data;
}

int main(int argc, char* argv[])
{
    using namespace std;
    SDL_SetMainReady();
    
    //The window we'll be rendering to
    SDL_Window* reference_window = nullptr;
    SDL_Window* surface_window = nullptr;
    SDL_Window* texture_window = nullptr;
    SDL_Renderer* dot_renderer = nullptr;
    SDL_Renderer* tex_renderer = nullptr;
    SDL_Surface* screen = nullptr;
    bool SDL_failed = false;

    //uint8_t* bytes = reinterpret_cast<uint8_t*>(buffer);
    //for (int i = 0; i < FRAME_BUFFER; ++i) {
    //    bytes[i] = reverse_bits(bytes[i]);
    //}

    try {
        //Initialize SDL
        if (SDL_failed = SDL_Init(SDL_INIT_VIDEO) < 0) {
            throw runtime_error{ "SDL could not initialize! SDL_Error: " + string{SDL_GetError()} };
        }
        //Create window
        reference_window = SDL_CreateWindow("Algorithm", 10, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH * 2, SCREEN_HEIGHT * 2, SDL_WINDOW_SHOWN);
        surface_window = SDL_CreateWindow("Surface Blit", 470, SDL_WINDOWPOS_UNDEFINED, SCREEN_HEIGHT * 2, SCREEN_WIDTH * 2, SDL_WINDOW_SHOWN);
        texture_window = SDL_CreateWindow("Texture Render", 1000, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH * 2, SCREEN_HEIGHT * 2, SDL_WINDOW_SHOWN);

        dot_renderer = SDL_CreateRenderer(reference_window, -1, SDL_RENDERER_ACCELERATED);
        tex_renderer = SDL_CreateRenderer(texture_window, -1, SDL_RENDERER_ACCELERATED);

        screen = SDL_CreateRGBSurfaceWithFormatFrom(buffer, SCREEN_HEIGHT, SCREEN_WIDTH, 1, SCREEN_HEIGHT / 8, SDL_PIXELFORMAT_INDEX1SB);
        swap(screen->format->palette->colors[0], screen->format->palette->colors[1]);

        SDL_Surface* win = SDL_GetWindowSurface(surface_window);

        for (bool done = false; !done; ) {
            SDL_Event event;
            while (SDL_PollEvent(&event)) {
                switch (event.type) {
                case SDL_KEYDOWN:
                case SDL_KEYUP:
                    switch (event.key.keysym.sym) {
                    case SDLK_ESCAPE:
                        done = true;
                        break;
                    }
                }
            }

            SDL_RenderClear(dot_renderer);
            uint8_t* bytes = reinterpret_cast<uint8_t*>(buffer);
            for (int i = 0; i < FRAME_BUFFER; ++i) {
                uint8_t state = bytes[i];
                SDL_Rect pixel = { 2 * i / 32, 2 * (SCREEN_HEIGHT - 8 * (i % 32)), 2, 2 };
                for (uint8_t mask = 0xFF; mask; mask >>= 1) {
                    if (mask & 0x01) {
                        auto value = screen->format->palette->colors[state & 0x01];
                        SDL_SetRenderDrawColor(dot_renderer, value.r, value.g, value.b, 255);

                        SDL_RenderDrawRect(dot_renderer, &pixel);
                    }
                    pixel.y -= 2;
                    state >>= 1;
                    
                }
            }
            SDL_RenderPresent(dot_renderer);

            SDL_Surface* transition = SDL_ConvertSurface(screen, win->format, 0);
            auto result = SDL_BlitScaled(transition, nullptr, win, nullptr);
            SDL_UpdateWindowSurface(surface_window);
            SDL_FreeSurface(transition);

            SDL_RenderClear(tex_renderer);
            SDL_Texture* pixels = SDL_CreateTextureFromSurface(tex_renderer, screen);
            SDL_RenderCopyEx(tex_renderer, pixels, nullptr, &FRAME, -90, nullptr, SDL_FLIP_NONE);
            SDL_DestroyTexture(pixels);
            SDL_RenderPresent(tex_renderer);
        }
    }
    catch (std::exception& e) {
        cerr << e.what() << endl;
    }

    //Destroy window
    SDL_DestroyRenderer(tex_renderer);
    SDL_DestroyRenderer(dot_renderer);
    SDL_DestroyWindow(texture_window);
    SDL_DestroyWindow(surface_window);
    SDL_DestroyWindow(reference_window);
    SDL_FreeSurface(screen);

    //Quit SDL subsystems
    SDL_Quit();

    return 0;
}
