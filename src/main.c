#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <SDL2/SDL.h>
#include "display.h"
#include "vector.h"


#define N_POINTS (9*9*9)
vec3_t cube_points[N_POINTS];
vec2_t projected_points[N_POINTS];

vec3_t camera_position = {.x = 0, .y = 0, .z = -5};
vec3_t cube_rotation = {.x = 0, .y = 0, .z = 0};

float fov_factor = 640;

bool is_running = false;

void setup(void) {
    // allocate memory for color buffer
    color_buffer = (uint32_t*) malloc(sizeof(uint32_t) * window_width * window_height);

    // create SDL texture to display color buffer
    color_buffer_texture = SDL_CreateTexture(
        renderer,
        SDL_PIXELFORMAT_ARGB8888,
        SDL_TEXTUREACCESS_STREAMING,
        window_width,
        window_height
    );

    int point_count = 0;
    //load objects
    for (float x = -1; x <= 1; x += 0.25) {
        for (float y = -1; y <= 1; y += 0.25) {
            for (float z = -1; z <= 1; z += 0.25) {
                vec3_t new_point = {.x=x,.y=y,.z=z};
                cube_points[point_count++] = new_point;
            }
        }
    }

}

void process_input(void) {
    SDL_Event event;
    SDL_PollEvent(&event);

    switch (event.type) {
        case SDL_QUIT:
            is_running = false;
            break;
        case SDL_KEYDOWN:
            if (event.key.keysym.sym == SDLK_ESCAPE)
                is_running = false;
            break;
    }
}

vec2_t project(vec3_t point) {
    vec2_t projected_point = {
        .x = (fov_factor * point.x) / point.z,
        .y = (fov_factor * point.y) / point.z
    };
    return projected_point;
}

void update(void) {
    cube_rotation.x += 0.01;
    cube_rotation.y += 0.01;
    cube_rotation.z += 0.01;

    for (int i = 0; i < N_POINTS; i++) {
        vec3_t point = cube_points[i];

        vec3_t transformed_point = vec3_rotate_x(point, cube_rotation.x);
        transformed_point = vec3_rotate_y(transformed_point, cube_rotation.y);
        transformed_point = vec3_rotate_z(transformed_point, cube_rotation.z);

        transformed_point.z -= camera_position.z;

        //projection
        vec2_t projected_point = project(transformed_point);

        //save projected points
        projected_points[i] = projected_point;
    }
}

void render(void) {
    /*
    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
    SDL_RenderClear(renderer);
    */ //no longer in use

    //draw_rect(10,10,10,10, 0xFFFF0000);
    //draw_rect(100,100,10,10, 0xFFFF0000);
    //draw_grid();

    //draw_pixel(20, 20, 0xffffff00);

    for (int i = 0; i < N_POINTS; i++) {
        vec2_t projected_point = projected_points[i];
        draw_rect(
            projected_point.x + (window_width / 2),
            projected_point.y + (window_width / 2),
            4,
            4,
            0xffffff00
        );
    }

    render_color_buffer();

    clear_color_buffer(0xFF8080FF);

    SDL_RenderPresent(renderer);
}

int main(void) {
    
    is_running = initialize_window();

    setup();

    while (is_running) {
        process_input();
        update();
        render();
    }

    destroy_window();

    return 0;
}