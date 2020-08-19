#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <SDL2/SDL.h>
#include "array.h"
#include "display.h"
#include "vector.h"
#include "mesh.h"

triangle_t *triangles_to_render = NULL;

vec3_t camera_position = {0, 0, 0};
//vec3_t cube_rotation = {.x = 0, .y = 0, .z = 0};

float fov_factor = 640;

bool is_running = false;
int previous_frame_time = 0;

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

    //load_cube_mesh_data();
    load_obj_file_data("./assets/f22.obj");
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
    //while (!SDL_TICKS_PASSED(SDL_GetTicks(),previous_frame_time + FRAME_TARGET_TIME));
    //previous_frame_time = SDL_GetTicks();
    int time_to_wait = FRAME_TARGET_TIME - (SDL_GetTicks() - previous_frame_time);

    if (time_to_wait > 0 && time_to_wait <= FRAME_TARGET_TIME) {
        SDL_Delay(time_to_wait);
    }

    triangles_to_render = NULL;

    mesh.rotation.x += 0.01;
    mesh.rotation.y += 0.01;
    mesh.rotation.z += 0.01;

    int num_faces = array_length(mesh.faces);
    for (int i = 0; i < num_faces; i++) {
        face_t mesh_face = mesh.faces[i];

        vec3_t face_vertices[3];
        face_vertices[0] = mesh.vertices[mesh_face.a - 1];
        face_vertices[1] = mesh.vertices[mesh_face.b - 1];
        face_vertices[2] = mesh.vertices[mesh_face.c - 1];

        //triangle_t projected_triangle;

        vec3_t transformed_vertices[3];

        // loop all 3 vertices
        for (int j = 0; j < 3; j++) {
            vec3_t transformed_vertex = face_vertices[j];

            transformed_vertex = vec3_rotate_x(transformed_vertex, mesh.rotation.x);
            transformed_vertex = vec3_rotate_y(transformed_vertex, mesh.rotation.y);
            transformed_vertex = vec3_rotate_z(transformed_vertex, mesh.rotation.z);

            //translate camera
            transformed_vertex.z += 5;
            transformed_vertices[j] = transformed_vertex;
        }

        // backface culling
        vec3_t vector_a = transformed_vertices[0];
        vec3_t vector_b = transformed_vertices[1];
        vec3_t vector_c = transformed_vertices[2];

        vec3_t vector_ab = vec3_sub(vector_b, vector_a);
        vec3_t vector_ac = vec3_sub(vector_c, vector_a);
        vec3_normalize(&vector_ab);
        vec3_normalize(&vector_ac);

        //computer normal
        vec3_t normal = vec3_cross(vector_ab, vector_ac);

        //normalize normal in place
        vec3_normalize(&normal);

        vec3_t camera_ray = vec3_sub(camera_position, vector_a);

        //calculate dot product for culling
        float dot_normal_camera = vec3_dot(normal, camera_ray);

        //skip triangles not facing camera
        if (dot_normal_camera < 0) {
            continue;
        }

        triangle_t projected_triangle;

        for (int j = 0; j < 3; j++) {
            vec2_t projected_point = project(transformed_vertices[j]);

            //scale and translate
            projected_point.x += (window_width / 2);
            projected_point.y += (window_height / 2);

            projected_triangle.points[j] = projected_point;
        }

        // save projected triangle in array of triangles to render
        //triangles_to_render[i] = Projected_triangle;
        array_push(triangles_to_render, projected_triangle);
    }
}

void render(void) {

    // loop all porjected triangles and render them



    int num_triangles = array_length(triangles_to_render);
    for (int i = 0; i < num_triangles; i++) {
        triangle_t triangle = triangles_to_render[i];
        /*
        draw_rect(
            triangle.points[0].x,
            triangle.points[0].y,
            4,
            4,
            0xffffff00
        );
        draw_rect(
            triangle.points[1].x,
            triangle.points[1].y,
            4,
            4,
            0xffffff00
        );
        draw_rect(
            triangle.points[2].x,
            triangle.points[2].y,
            4,
            4,
            0xffffff00
        );
        */

        draw_filled_triangle(
            triangle.points[0].x, 
            triangle.points[0].y, 
            triangle.points[1].x,
            triangle.points[1].y,
            triangle.points[2].x,
            triangle.points[2].y,
            0xff00ff00
        );
    }



    //draw_filled_triangle(300, 100, 50, 400, 500, 700, 0xff00ff00);

    // clear array
    array_free(triangles_to_render);

    render_color_buffer();

    clear_color_buffer(0xFF08088f);

    SDL_RenderPresent(renderer);
}

void free_resources(void) {
    free(color_buffer);
    array_free(mesh.faces);
    array_free(mesh.vertices);
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
    free_resources();

    return 0;
}