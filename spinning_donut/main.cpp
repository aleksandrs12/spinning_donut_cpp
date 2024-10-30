#include <SDL.h>
#include <iostream>
#include <cmath>
#include <algorithm>
#include <map>

const int SCREEN_WIDTH = 300;
const int SCREEN_HEIGHT = 300;

SDL_Window* window = nullptr;
SDL_Renderer* renderer = nullptr;

bool init() {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cout << "SDL could not initialize! SDL_Error: " << SDL_GetError() << std::endl;
        return false;
    }

    window = SDL_CreateWindow("Spinning Donut", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    if (window == nullptr) {
        std::cout << "Window could not be created! SDL_Error: " << SDL_GetError() << std::endl;
        return false;
    }

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (renderer == nullptr) {
        std::cout << "Renderer could not be created! SDL_Error: " << SDL_GetError() << std::endl;
        return false;
    }

    return true;
}

void close() {
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

void renderPixel(int x, int y, Uint8 r, Uint8 g, Uint8 b, Uint8 a) {
    SDL_SetRenderDrawColor(renderer, r, g, b, a);
    SDL_RenderDrawPoint(renderer, x, y);
}

struct Pos {
    int x;
    int y;
    bool operator<(const Pos& other) const {
        if (x != other.x)
            return x < other.x;
        return y < other.y;
    }
};

struct Color {
    uint8_t r, g, b;
};

Color distanceToColor(double distance) {
    // Ensure distance is in the range [0, 1]
    distance = 1 - distance;
    distance = std::max(0.0, std::min(1.0, distance));

    Color color;

    if (distance < 0.25) {
        // Blue to Cyan
        color.r = 0;
        color.g = static_cast<uint8_t>(255 * (distance / 0.25));
        color.b = 255;
    }
    else if (distance < 0.5) {
        // Cyan to Green
        color.r = 0;
        color.g = 255;
        color.b = static_cast<uint8_t>(255 * (1 - (distance - 0.25) / 0.25));
    }
    else if (distance < 0.75) {
        // Green to Yellow
        color.r = static_cast<uint8_t>(255 * ((distance - 0.5) / 0.25));
        color.g = 255;
        color.b = 0;
    }
    else {
        // Yellow to Red
        color.r = 255;
        color.g = static_cast<uint8_t>(255 * (1 - (distance - 0.75) / 0.25));
        color.b = 0;
    }

    return color;
}

Pos calculate_ring(const Pos& pos, int r, double angle) { // radians
    Pos output;
    output.y = sin(angle) * r + pos.y;
    output.x = cos(angle) * r + pos.x;
    return output;
}

int main(int argc, char* args[]) {
    const int INNER_RADIUS = 30;
    const int OUTER_RADIUS = 80;
    Pos donut_centre = { 150, 150 };
    double x_rot = 0;

    typedef std::map<Pos, double> pixels_seen_hash;
    pixels_seen_hash pixels_seen;

    double distance_matrix[SCREEN_WIDTH][SCREEN_HEIGHT];



    if (!init()) {
        std::cout << "Failed to initialize!" << std::endl;
        return -1;
    }

    bool quit = false;
    SDL_Event e;

    while (!quit) {
        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT) {
                quit = true;
            }
        }

        SDL_SetRenderDrawColor(renderer, 20, 20, 54, 0xFF);
        SDL_RenderClear(renderer);

        // Example: Render a red pixel at (320, 240)
        //renderPixel(320, 240, 0xFF, 0x00, 0x00, 0xFF);

        x_rot += 3.14 / 64.0;
        //pixels_seen.clear();
        for (int i = donut_centre.x - OUTER_RADIUS; i < donut_centre.x + OUTER_RADIUS; i++) {
            for (int j = donut_centre.y - OUTER_RADIUS; j < donut_centre.y + OUTER_RADIUS; j++) {
                distance_matrix[i][j] = 0; // Set the value to 0
            }
        }

        for (double radians = 0; radians < 6.28; radians += 6.28 / (3.14 * 2 * OUTER_RADIUS)) {
            Pos inner_pixel = calculate_ring(donut_centre, INNER_RADIUS, radians);
            Pos outer_pixel = calculate_ring(donut_centre, OUTER_RADIUS, radians);

            inner_pixel.x = (inner_pixel.x - donut_centre.x) * cos(x_rot) + donut_centre.x;
            outer_pixel.x = (outer_pixel.x - donut_centre.x) * cos(x_rot) + donut_centre.x;


            for (double radians2 = 0; radians2 < 6.28; radians2 += 6.28 / (3.14 * (OUTER_RADIUS - INNER_RADIUS))) {
                Pos ring_pos_adj = calculate_ring(Pos{ 0, 0 }, (OUTER_RADIUS - INNER_RADIUS) / 2, radians2);
                Pos ring_pos;
                ring_pos.x = (OUTER_RADIUS - INNER_RADIUS) / 2 * cos(radians2) + (inner_pixel.x + outer_pixel.x) / 2;
                ring_pos.y = (OUTER_RADIUS - INNER_RADIUS) / 2 * sin(radians2) + (inner_pixel.y + outer_pixel.y) / 2;

                double distance = cos(radians2) * (OUTER_RADIUS - INNER_RADIUS) / 2 + sin(x_rot) * cos(radians) * ((OUTER_RADIUS + INNER_RADIUS) / 2) + ((OUTER_RADIUS + INNER_RADIUS) / 2);

                Color pixel_color = distanceToColor(distance / static_cast<float>((static_cast<float>(OUTER_RADIUS - INNER_RADIUS) / 2) * 0.7 + (0.7 * static_cast<float>(OUTER_RADIUS + INNER_RADIUS) / 2) + (OUTER_RADIUS + INNER_RADIUS) / 2));
                //renderPixel(ring_pos.x, ring_pos.y, pixel_color.r, pixel_color.g, pixel_color.b, 0xFF);

                /*
                if (pixels_seen.find(ring_pos) == pixels_seen.end()) {
                    pixels_seen[ring_pos] = distance;
                    renderPixel(ring_pos.x, ring_pos.y, pixel_color.r, pixel_color.g, pixel_color.b, 0xFF);
                }
                else if (pixels_seen.at(ring_pos) > distance) {
                    pixels_seen[ring_pos] = distance;
                    renderPixel(ring_pos.x, ring_pos.y, pixel_color.r, pixel_color.g, pixel_color.b, 0xFF);
                }
                */
                if (distance_matrix[ring_pos.x][ring_pos.y] == 0.0) {
                    distance_matrix[ring_pos.x][ring_pos.y] = distance;
                    renderPixel(ring_pos.x, ring_pos.y, pixel_color.r, pixel_color.g, pixel_color.b, 0xFF);
                }
                else if (distance_matrix[ring_pos.x][ring_pos.y] > distance) {
                    distance_matrix[ring_pos.x][ring_pos.y] = distance;
                    renderPixel(ring_pos.x, ring_pos.y, pixel_color.r, pixel_color.g, pixel_color.b, 0xFF);
                }

            }
            //renderPixel(outer_pixel.x, outer_pixel.y, 0xFF, 0xFF, 0xFF, 0xFF);
            //renderPixel(inner_pixel.x, inner_pixel.y, 0xFF, 0xFF, 0xFF, 0xFF);
        }



        SDL_RenderPresent(renderer);
    }

    close();
    return 0;
}