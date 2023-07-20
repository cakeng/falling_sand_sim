#ifndef _FALLING_SAND_H
#define _FALLING_SAND_H

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "shader.h"

#define SCR_WIDTH 1280
#define SCR_RATIO 9/16
#define SCR_HEIGHT 720

#define WRD_WIDTH 288
#define WRD_HEIGHT (WRD_WIDTH*SCR_RATIO)

#define VTX_SCALE 1.0f
#define MOV_SCALE 0.2f
#define FLOW_SCALE 3.0f

#define MOUSE_BRUSH_SIZE 6

#define _GRAVITY 10.0f

typedef struct physics_property physics_property;
typedef struct vertex_obj vertex_obj;
typedef struct render_obj render_obj;
typedef struct int3 int3;
typedef struct world_obj world_obj;

typedef enum MOUSE_BUTTON {NONE, LEFT, RIGHT} MOUSE_BUTTON;
typedef enum MATERIAL_TYPE {AIR, SAND, WATER, STEAM, LAVA, ROCK, LIGHT, MARIO} MATERIAL_TYPE;
typedef enum _TYPE {GAS, LIQUID, SOLID} _TYPE;

struct physics_property 
{
    
    _TYPE state;
    MATERIAL_TYPE material;
    // Lighting
    glm::vec3 diffuse;
    glm::vec3 reflect;
    glm::vec3 radiate;

    // Kinetics
    float mass;
    float drag;
    float flow;

    bool apply_displacement;
    bool apply_gravity;
};

struct vertex_obj
{
    bool updated;
    glm::vec3 col;
    physics_property *phys_prop;
    world_obj *world;
    bool apply_displacement;
    bool apply_gravity;
    glm::vec3 force;
    glm::vec3 vel; 
    glm::vec3 mov;
};

struct render_obj
{
    glm::vec3 pos;
    glm::vec3 vpos;
    glm::vec3 reflect; 
    glm::vec3 radiation;
    glm::vec3 col;
};

struct world_obj
{
    bool event_flag;
    float event_time;
    float current_time;
    float delta_time;
    int width;
    int height;
    int brush_size;
    int fall_on;

    vertex_obj *vertex_list;
    render_obj *render_list;
    
    unsigned int TID, FBO, VAO, SSBO;
    unsigned int QUAD_VAO, QUAD_VBO;

    vertex_obj* extern_obj;
    int extern_obj_w, extern_obj_h;
};

extern physics_property* air;
extern physics_property* sand;
extern physics_property* water;
extern physics_property* steam;
extern physics_property* rock;
extern physics_property* lava;
extern physics_property* light;
extern physics_property* mario;

void mouse_event (world_obj *world, MATERIAL_TYPE material, MOUSE_BUTTON button, int xmax, int ymax, float xoffset, float yoffset);
void update_world_physics (world_obj *world);

void update_word_render_list (world_obj *world);

world_obj* make_world (int width, int height, int load_world);

void free_world (world_obj *world);

void update_world (world_obj *world);

void render_world (world_obj *world, Shader *shader, int rtx_on);

void draw_world (world_obj *world, Shader *shader, int scr_w, int scr_h);

#endif