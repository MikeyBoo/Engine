#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <SDL.h>
#include "array.h"
#include "display.h"
#include "vector.h"
#include "mesh.h"
#include "light.h"
#include "matrix.h"

triangle_t *triangles_to_render = NULL;

vec3_t camera_position = {0, 0, 0};
//vec3_t cube_rotation = {.x = 0, .y = 0, .z = 0};

//float fov_factor = 640;

bool is_running = false;
int previous_frame_time = 0;
mat4_t proj_matrix;

void setup(void) {
    render_method = RENDER_FILL_TRIANGLE_WIRE;
    cull_method = CULL_BACKFACE;

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

    // init persp proj mat
    float fov = M_PI / 3.0; //60 degrees
    float aspect = (float)window_height / (float)window_width;
    float znear = 0.1;
    float zfar = 100.0;
    proj_matrix = mat4_make_perspective(fov, aspect, znear, zfar);


    //load_cube_mesh_data();
    load_obj_file_data("./assets/cult2.obj");
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
            if (event.key.keysym.sym == SDLK_1)
                render_method = RENDER_WIRE_VERTEX;
            if (event.key.keysym.sym == SDLK_2)
                render_method = RENDER_WIRE;
            if (event.key.keysym.sym == SDLK_3)
                render_method = RENDER_FILL_TRIANGLE;
            if (event.key.keysym.sym == SDLK_4)
                render_method = RENDER_FILL_TRIANGLE_WIRE;
            if (event.key.keysym.sym == SDLK_c)
                cull_method = CULL_BACKFACE;
            if (event.key.keysym.sym == SDLK_d)
                cull_method = CULL_NONE;
            break;
    }
}

//vec2_t project(vec3_t point) {
//    vec2_t projected_point = {
//        .x = (fov_factor * point.x) / point.z,
//        .y = (fov_factor * point.y) / point.z
//    };
//    return projected_point;
//}

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

    //mesh.scale.x += 0.002;
    mesh.translation.z = 5.0;
    //mesh.translation.x += 0.001;

    mat4_t scale_matrix = mat4_make_scale(mesh.scale.x, mesh.scale.y, mesh.scale.z);
    mat4_t translation_matrix = mat4_make_translation(mesh.translation.x, mesh.translation.y, mesh.translation.z);
    mat4_t rotation_matrix_x = mat4_make_rotation_x(mesh.rotation.x);
    mat4_t rotation_matrix_y = mat4_make_rotation_y(mesh.rotation.y);
    mat4_t rotation_matrix_z = mat4_make_rotation_z(mesh.rotation.z);

    int num_faces = array_length(mesh.faces);
    for (int i = 0; i < num_faces; i++) {
        face_t mesh_face = mesh.faces[i];

        vec3_t face_vertices[3];
        face_vertices[0] = mesh.vertices[mesh_face.a - 1];
        face_vertices[1] = mesh.vertices[mesh_face.b - 1];
        face_vertices[2] = mesh.vertices[mesh_face.c - 1];

        //triangle_t projected_triangle;

        vec4_t transformed_vertices[3];

        // loop all 3 vertices
        for (int j = 0; j < 3; j++) {
            vec4_t transformed_vertex = vec4_from_vec3(face_vertices[j]);

            //transformed_vertex = vec3_rotate_x(transformed_vertex, mesh.rotation.x);
            //transformed_vertex = vec3_rotate_y(transformed_vertex, mesh.rotation.y);
            //transformed_vertex = vec3_rotate_z(transformed_vertex, mesh.rotation.z);

            //transformed_vertex = mat4_mul_vec4(scale_matrix, transformed_vertex);
            //transformed_vertex = mat4_mul_vec4(rotation_matrix_x, transformed_vertex);
            //transformed_vertex = mat4_mul_vec4(rotation_matrix_y, transformed_vertex);
            //transformed_vertex = mat4_mul_vec4(rotation_matrix_z, transformed_vertex);
            //transformed_vertex = mat4_mul_vec4(translation_matrix, transformed_vertex);

            mat4_t world_matrix = mat4_identity();
            world_matrix = mat4_mul_mat4(scale_matrix, world_matrix);
            world_matrix = mat4_mul_mat4(rotation_matrix_z, world_matrix);
            world_matrix = mat4_mul_mat4(rotation_matrix_x, world_matrix);
            world_matrix = mat4_mul_mat4(rotation_matrix_y, world_matrix);
            world_matrix = mat4_mul_mat4(translation_matrix, world_matrix);

            transformed_vertex = mat4_mul_vec4(world_matrix, transformed_vertex);

            transformed_vertices[j] = transformed_vertex;
        }

        // backface culling
        vec3_t vector_a = vec3_from_vec4(transformed_vertices[0]);
        vec3_t vector_b = vec3_from_vec4(transformed_vertices[1]);
        vec3_t vector_c = vec3_from_vec4(transformed_vertices[2]);

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

        if (cull_method == CULL_BACKFACE) {
            //skip triangles not facing camera
            if (dot_normal_camera < 0) {
                continue;
            }
        }

        vec4_t projected_points[3];

        for (int j = 0; j < 3; j++) {
            projected_points[j] = mat4_mul_vec4_project(proj_matrix, transformed_vertices[j]);

            //scale first
            projected_points[j].x *= (window_width / 2.0);
            projected_points[j].y *= (window_height / 2.0);

            // translate
            projected_points[j].x += (window_width / 2.0);
            projected_points[j].y += (window_height / 2.0);
        }

        // calculate average depth for each face
        float avg_depth = (transformed_vertices[0].z + transformed_vertices[1].z + transformed_vertices[2].z) / 3.0;

        // calc shading intensity on normal
        float light_intensity_factor = -vec3_dot(normal, light.direction);

        // calc light angle
        uint32_t triangle_color = light_apply_intensity(mesh_face.color, light_intensity_factor);

        triangle_t projected_triangle = {
            .points = {
                {projected_points[0].x, projected_points[0].y },
                {projected_points[1].x, projected_points[1].y },
                {projected_points[2].x, projected_points[2].y }
            },
            .color = triangle_color,
            .avg_depth = avg_depth
        };

        // save projected triangle in array of triangles to render
        //triangles_to_render[i] = Projected_triangle;
        array_push(triangles_to_render, projected_triangle);
    }

    //bubble sort
    int num_triangles = array_length(triangles_to_render);
    for (int i = 0; i < num_triangles; i++) {
        for (int j = i; j < num_triangles; j++) {
            if (triangles_to_render[i].avg_depth < triangles_to_render[j].avg_depth) {
                triangle_t temp = triangles_to_render[i];
                triangles_to_render[i] = triangles_to_render[j];
                triangles_to_render[j] = temp;
            }
        }
    }
}

void render(void) {

    // loop all porjected triangles and render them



    int num_triangles = array_length(triangles_to_render);
    for (int i = 0; i < num_triangles; i++) {
        triangle_t triangle = triangles_to_render[i];
        
        if (render_method == RENDER_WIRE_VERTEX) {
            draw_rect(
                triangle.points[0].x - 2,
                triangle.points[0].y - 2,
                4,
                4,
                0xffffff00
            );
            draw_rect(
                triangle.points[1].x - 2,
                triangle.points[1].y - 2,
                4,
                4,
                0xffffff00
            );
            draw_rect(
                triangle.points[2].x - 2,
                triangle.points[2].y - 2,
                4,
                4,
                0xffffff00
            );
        }

        if (render_method == RENDER_FILL_TRIANGLE || render_method == RENDER_FILL_TRIANGLE_WIRE) {
            draw_filled_triangle(
                triangle.points[0].x, 
                triangle.points[0].y, 
                triangle.points[1].x,
                triangle.points[1].y,
                triangle.points[2].x,
                triangle.points[2].y,
                triangle.color
            );
        }

        if (render_method == RENDER_WIRE || render_method == RENDER_WIRE_VERTEX || render_method == RENDER_FILL_TRIANGLE_WIRE) {
            draw_triangle(
                triangle.points[0].x, 
                triangle.points[0].y, 
                triangle.points[1].x,
                triangle.points[1].y,
                triangle.points[2].x,
                triangle.points[2].y,
                0xffffffff
            );
        }
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

int main(int argc, char* args[]) {
    
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