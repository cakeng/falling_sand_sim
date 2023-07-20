#include "falling_sand.h"
#include <thread>


physics_property _air = {
    .state = GAS, .material = AIR
    , .diffuse = {(float)146 / 256, (float)144 / 256, (float)255/256}, .reflect = {0.015, 0.015, 0.03}, .radiate = {0.006, 0.005, 0.01}
    , .mass = 0.1f, .drag = 0.01, .flow = 1.0f,
    .apply_displacement = true, .apply_gravity = false,};
physics_property* air = &_air;
physics_property _sand = {
    .state = SOLID,.material = SAND
    , .diffuse = {0.65, 0.5, 0.2}, .reflect = {0.9, 0.9, 0.9}, .radiate = {0.0, 0.0, 0.0}  
    , .mass = 10.0f, .drag = 1.0, .flow = 1.0f,
    .apply_displacement = true, .apply_gravity = true,};
physics_property* sand = &_sand;
physics_property _water = {
    .state = LIQUID,.material = WATER
    , .diffuse = {0.1, 0.1, 0.8}, .reflect = {0.7, 0.7, 0.7}, .radiate = {0.0, 0.0, 0.0}  
    , .mass = 1.0f, .drag = 0.2, .flow = 5.0f,
    .apply_displacement = true, .apply_gravity = true,};
physics_property* water = &_water;
physics_property _steam = {
    .state = GAS,.material = STEAM
    , .diffuse = {(float)220 / 256, (float)220 / 256, (float)220 / 256}, .reflect = {0.2, 0.2, 0.3}, .radiate = {0.0, 0.0, 0.0}
    , .mass = 0.001f, .drag = 0.35, .flow = 50.0f,
    .apply_displacement = true, .apply_gravity = true,};
physics_property* steam = &_steam;
physics_property _rock = {
    .state = SOLID,.material = ROCK
    , .diffuse = {(float)110 / 256, (float)103 / 256, (float)105 / 256}, .reflect = {0.99, 0.99, 0.99}, .radiate = {0.0, 0.0, 0.0}
    , .mass = 15.0f, .drag = 1.0, .flow = 1.0f,
    .apply_displacement = false, .apply_gravity = true,};
physics_property* rock = &_rock;
physics_property _lava = {
    .state = LIQUID,.material = LAVA
    , .diffuse = {1.0, 0.55, 0.3}, .reflect = {1.0, 0.85, 0.6} , .radiate = {0.88, 0.95, 0.3}
    , .mass = 0.14f, .drag = 1.0, .flow = 1.5f,
    .apply_displacement = true, .apply_gravity = true,};
physics_property* lava = &_lava;
physics_property _light = {
    .state = SOLID,.material = LIGHT
    , .diffuse = {1.0, 1.0, 1.0}, .reflect = {1.0, 1.0, 1.0} , .radiate = {1.0, 0.98, 0.95}
    , .mass = 15.0f, .drag = 1.0, .flow = 1.0f,
    .apply_displacement = false, .apply_gravity = false,};
physics_property* light = &_light;
physics_property _mario = {
    .state = SOLID,.material = MARIO
    , .diffuse = {0.7, 0.4, 0.7}, .reflect = {0.8, 0.8, 0.95} , .radiate = {0.0, 0.0, 0.0}
    , .mass = 6.0f, .drag = 1.0, .flow = 1.0f,
    .apply_displacement = false, .apply_gravity = false, };
physics_property* mario = &_mario;

physics_property *phys_map[1024] = {NULL};

inline float frand()
{
    return (float)rand() / RAND_MAX;
}

inline int vtx_w (vertex_obj *vtx)
{
    return (vtx - vtx->world->vertex_list) % vtx->world->width;
}

inline int vtx_h (vertex_obj *vtx)
{
    return (vtx - vtx->world->vertex_list) / vtx->world->width;
}

vertex_obj *get_vtx(world_obj *world, int w, int h)
{
    if (w < 0 || w >= world->width || h < 0 || h >= world->height)
        return NULL;
    return world->vertex_list + h * world->width + w;
}

inline vertex_obj *get_vtx_dg (world_obj *world, int w, int h)
{
    return world->vertex_list + h * world->width + w;
}

vertex_obj *l_vtx(vertex_obj *vtx)
{
    world_obj *world = vtx->world;
    if (vtx_w(vtx) == 0)
        return NULL;
    return vtx - 1;
}
vertex_obj *r_vtx(vertex_obj *vtx)
{
    world_obj *world = vtx->world;
    if (vtx_w(vtx)== world->width-1)
        return NULL;
    return vtx + 1;
}
vertex_obj *u_vtx(vertex_obj *vtx)
{
    world_obj *world = vtx->world;
    if (vtx_h(vtx) == 0)
        return NULL;
    return vtx - vtx->world->width;
}
vertex_obj *d_vtx(vertex_obj *vtx)
{
    world_obj *world = vtx->world;
    if (vtx_h(vtx) == world->height-1)
        return NULL;
    return vtx + vtx->world->width;
}
vertex_obj *ul_vtx(vertex_obj *vtx)
{
    world_obj *world = vtx->world;
    if (vtx_w(vtx) == 0 || vtx_h(vtx) == 0)
        return NULL;
    return vtx - 1 - vtx->world->width;
}
vertex_obj *ur_vtx(vertex_obj *vtx)
{
    world_obj *world = vtx->world;
    if (vtx_w(vtx)== world->width-1 || vtx_h(vtx) == 0)
        return NULL;
    return vtx + 1 - vtx->world->width;
}
vertex_obj *dl_vtx(vertex_obj *vtx)
{
    world_obj *world = vtx->world;
    if (vtx_h(vtx) == world->height-1 || vtx_w(vtx) == 0)
        return NULL;
    return vtx - 1 + vtx->world->width;
}
vertex_obj *dr_vtx(vertex_obj *vtx)
{
    world_obj *world = vtx->world;
    if (vtx_h(vtx) == world->height-1 || vtx_w(vtx)== world->width-1)
        return NULL;
    return vtx + 1 + vtx->world->width;
}

glm::vec3 dir_vtx (vertex_obj *org, vertex_obj* dir)
{
    int o_w = vtx_w(org), o_h = vtx_h(org), d_w = vtx_w(dir), d_h = vtx_h(dir);
    return glm::normalize(glm::vec3(d_w - o_w, d_h - o_h, 0.0));
}

bool swap_vertex (vertex_obj *vtx1, vertex_obj* vtx2)
{
    if (vtx1->phys_prop == vtx2->phys_prop)
        return false;
    if (!(vtx1->apply_displacement && vtx2->apply_displacement))
        return false;
    vertex_obj temp;
    temp = *vtx1;
    *vtx1 = *vtx2;
    *vtx2 = temp;
    return true;
}
void update_vtx_phys(vertex_obj* vtx, physics_property* phys)
{
    vtx->phys_prop = phys;
    vtx->apply_displacement = vtx->phys_prop->apply_displacement;
    vtx->apply_gravity = vtx->phys_prop->apply_gravity;
    vtx->col = phys->diffuse;
}


void reset_vertex (vertex_obj *vtx)
{
    vtx->updated = false;
    update_vtx_phys(vtx,  air);
    vtx->force = glm::vec3(0.0);
    vtx->vel = glm::vec3(0.0);
    vtx->mov = glm::vec3(0.0);
    vtx->col = vtx->phys_prop->diffuse;
    vtx->apply_displacement = vtx->phys_prop->apply_displacement;
    vtx->apply_gravity = vtx->phys_prop->apply_gravity;
}


void print_vertex (vertex_obj *vtx)
{
    printf ("Material: %d, force: (%2.2f, %2.2f), vel: (%2.2f, %2.2f), mov: (%2.2f, %2.2f)\n"
        , vtx->phys_prop->material, vtx->force.x, vtx->force.y, vtx->vel.x, vtx->vel.y
        , vtx->mov.x, vtx->mov.y);
}


void generate_vertex(world_obj *world, int w, int h, physics_property *mat)
{
    vertex_obj obj;
    if (!get_vtx(world, w, h) || get_vtx(world, w, h)->phys_prop == mat)
        return;
    reset_vertex (&obj);
    obj.col = mat->diffuse;
    obj.world = world;
    update_vtx_phys(&obj, mat);
    *get_vtx(world, w, h) = obj;
}

void generate_vertex(vertex_obj *vtx, physics_property *mat)
{
    vertex_obj obj;
    if (!vtx || vtx->phys_prop == mat)
        return;
    reset_vertex (&obj);
    obj.col = mat->diffuse;
    obj.world = vtx->world;
    update_vtx_phys(&obj, mat);
    *vtx = obj;
}

void generate_circle(world_obj *world, int w, int h, int r, physics_property *mat)
{
    int lasth;
    for (int widx = -r; widx < r; widx++)
    {
        int hlen = (int)sqrt((float)r*r - widx*widx);
        if (widx == 0)
        {
            hlen--;
        }
        for (int hidx = -hlen; hidx < hlen; hidx++)
        {
            generate_vertex (world, w + widx, h + hidx, mat);
        }
    }
}

void kinetic_engine (vertex_obj *vtx)
{
    float dt = vtx->world->delta_time;
    int w = vtx_w(vtx), h = vtx_h(vtx);
    physics_property *v_phys = vtx->phys_prop;

    if (v_phys->material == LAVA)
    {
        vertex_obj * targ = vtx;
        int count = 0;
        bool rockd = false;
        while (count < 6)
        {
            vertex_obj* rock_targ = u_vtx(targ);
            if (rock_targ && rock_targ->phys_prop->material == WATER)
            {
                rockd = true;
                generate_vertex(rock_targ, steam);
                targ = rock_targ;
                goto LOOP;
            }
            else if (rock_targ && rock_targ->phys_prop->material == ROCK)
            {
                targ = rock_targ;
                goto LOOP;
            }
            rock_targ = d_vtx(targ);
            if (rock_targ && rock_targ->phys_prop->material == WATER)
            {
                rockd = true;
                generate_vertex(rock_targ, steam);
                targ = rock_targ;
                goto LOOP;
            }
            else if (rock_targ && rock_targ->phys_prop->material == ROCK)
            {
                targ = rock_targ;
                goto LOOP;
            }
            rock_targ = l_vtx(targ);
            if (rock_targ && rock_targ->phys_prop->material == WATER)
            {
                rockd = true;
                generate_vertex(rock_targ, steam);
                targ = rock_targ;
                goto LOOP;
            }
            else if (rock_targ && rock_targ->phys_prop->material == ROCK)
            {
                targ = rock_targ;
                goto LOOP;
            }
            rock_targ = r_vtx(targ);
            if (rock_targ && rock_targ->phys_prop->material == WATER)
            {
                rockd = true;
                generate_vertex(rock_targ, steam);
                targ = rock_targ;
                goto LOOP;
            }
            else if (rock_targ && rock_targ->phys_prop->material == ROCK)
            {
                targ = rock_targ;
                goto LOOP;
            }
        LOOP:
            count++;
        }
        if (rockd)
        {
            generate_vertex(vtx, rock);
        }
    }
    else if (v_phys->material == STEAM)
    {
        vertex_obj *d_obj = d_vtx(vtx);
        vertex_obj* l_obj = l_vtx(vtx);
        vertex_obj* r_obj = r_vtx(vtx);
        if (d_obj && d_obj->phys_prop->material == STEAM &&
            l_obj && l_obj->phys_prop->material == STEAM &&
            r_obj && r_obj->phys_prop->material == STEAM)
        {
            reset_vertex(d_obj);
            reset_vertex(r_obj);
            reset_vertex(l_obj);
            generate_vertex(vtx, water);
        }
    }
    else if (v_phys->material == MARIO)
    {
        if (vtx->apply_gravity == true)
        {
            vertex_obj* l_obj = l_vtx(vtx);
            vertex_obj* r_obj = r_vtx(vtx);
            if (l_obj && l_obj->phys_prop->material == MARIO && (ul_vtx(vtx) && ul_vtx(vtx)->phys_prop->material != MARIO))
            {
                l_obj->apply_displacement = true;
                l_obj->apply_gravity = true;
            }
            if (r_obj && r_obj->phys_prop->material == MARIO && (ur_vtx(vtx) && ur_vtx(vtx)->phys_prop->material != MARIO))
            {
                r_obj->apply_displacement = true;
                r_obj->apply_gravity = true;
            }
        }
        else
        {
            float total_weight = 0;
            vertex_obj* u_obj = u_vtx(vtx);
            while (u_obj != NULL && u_obj->phys_prop->state == SOLID)
            {
                if (u_obj->apply_gravity == false)
                    total_weight -= INFINITY;
                else
                    total_weight += u_obj->phys_prop->mass;
                u_obj = u_vtx(u_obj);
            }
            if (total_weight > 50)
            {
                vtx->apply_displacement = true;
                vtx->apply_gravity = true;
            }
        }
    }

    if (!vtx->apply_displacement)
    {
        vtx->vel = glm::vec3(0.0f);
        vtx->mov - glm::vec3(0.0f);
        return;
    }
    if (!vtx->apply_gravity)
        return;
    
    // Gravity
    vertex_obj *grav_targ = NULL;
    glm::vec3 ratio = glm::vec3 (1.0f);
    if (v_phys->state == SOLID)
    {
        if (!grav_targ && d_vtx(vtx) && (d_vtx(vtx)->phys_prop != v_phys) &&
            (d_vtx(vtx)->phys_prop->state == GAS || d_vtx(vtx)->phys_prop->state == LIQUID))
        {
            grav_targ = d_vtx(vtx);
        }
        if (frand() < 0.5)
        {
            if (!grav_targ&& dl_vtx(vtx) && (dl_vtx(vtx)->phys_prop != v_phys) &&
                (dl_vtx(vtx)->phys_prop->state == GAS || dl_vtx(vtx)->phys_prop->state == LIQUID))
            {
                grav_targ = dl_vtx(vtx);
            }
            else if (!grav_targ&& dr_vtx(vtx) && (dr_vtx(vtx)->phys_prop != v_phys) &&
                (dr_vtx(vtx)->phys_prop->state == GAS || dr_vtx(vtx)->phys_prop->state == LIQUID))
            {
                grav_targ = dr_vtx(vtx);
            }
        }
        else
        {
            if (!grav_targ&& dr_vtx(vtx) && (dr_vtx(vtx)->phys_prop != v_phys) &&
                (dr_vtx(vtx)->phys_prop->state == GAS || dr_vtx(vtx)->phys_prop->state == LIQUID))
            {
                grav_targ = dr_vtx(vtx);
            }
            else if (!grav_targ&& dl_vtx(vtx) && (dl_vtx(vtx)->phys_prop != v_phys) &&
                (dl_vtx(vtx)->phys_prop->state == GAS || dl_vtx(vtx)->phys_prop->state == LIQUID))
            {
                grav_targ = dl_vtx(vtx);
            }
        }
    }
    else if (v_phys->state == LIQUID)
    {
        if (!grav_targ && d_vtx(vtx) && (d_vtx(vtx)->phys_prop != v_phys) &&
            (d_vtx(vtx)->phys_prop->state == GAS || d_vtx(vtx)->phys_prop->state == LIQUID))
        {
            grav_targ = d_vtx(vtx);
        }
        if (vtx->vel.x > 0.0 || (vtx->vel.x == 0.0 && frand() < 0.5))
        {
            if (!grav_targ&& dr_vtx(vtx) && (dr_vtx(vtx)->phys_prop != v_phys) &&
                (dr_vtx(vtx)->phys_prop->state == GAS || dr_vtx(vtx)->phys_prop->state == LIQUID))
            {
                grav_targ = dr_vtx(vtx);
            }
            else if (!grav_targ && r_vtx(vtx) && (r_vtx(vtx)->phys_prop != v_phys) &&
                (r_vtx(vtx)->phys_prop->state == GAS || r_vtx(vtx)->phys_prop->state == LIQUID))
            {
                grav_targ = r_vtx(vtx);
                vtx->vel = glm::vec3(0.0f);
            }
        }
        else
        {
            if (!grav_targ&& dl_vtx(vtx) && (dl_vtx(vtx)->phys_prop != v_phys) &&
                (dl_vtx(vtx)->phys_prop->state == GAS || dl_vtx(vtx)->phys_prop->state == LIQUID))
            {
                grav_targ = dl_vtx(vtx);
            }
            else if (!grav_targ && (vtx->vel.x <= 0.0) && l_vtx(vtx) && (l_vtx(vtx)->phys_prop != v_phys) &&
                (l_vtx(vtx)->phys_prop->state == GAS || l_vtx(vtx)->phys_prop->state == LIQUID))
            {
                grav_targ = l_vtx(vtx);
                vtx->vel = glm::vec3(0.0f);
            }
        }
    }
    else // if (v_phys->state == GAS)
    {
        if (!grav_targ && u_vtx(vtx) && (u_vtx(vtx)->phys_prop != v_phys) &&
            (u_vtx(vtx)->phys_prop->state == GAS || u_vtx(vtx)->phys_prop->state == LIQUID))
        {
            grav_targ = u_vtx(vtx);
        }
        if (frand() < 0.5)
        {
            if (!grav_targ&& ul_vtx(vtx) && (ul_vtx(vtx)->phys_prop != v_phys) &&
                (ul_vtx(vtx)->phys_prop->state == GAS || ul_vtx(vtx)->phys_prop->state == LIQUID))
            {
                grav_targ = ul_vtx(vtx);
            }
            else if (!grav_targ&& ur_vtx(vtx) && (ur_vtx(vtx)->phys_prop != v_phys) &&
                (ur_vtx(vtx)->phys_prop->state == GAS || ur_vtx(vtx)->phys_prop->state == LIQUID))
            {
                grav_targ = ur_vtx(vtx);
            }
        }
        else
        {
            if (!grav_targ&& ur_vtx(vtx) && (ur_vtx(vtx)->phys_prop != v_phys) &&
                (ur_vtx(vtx)->phys_prop->state == GAS || ur_vtx(vtx)->phys_prop->state == LIQUID))
            {
                grav_targ = ur_vtx(vtx);
            }
            else if (!grav_targ&& ul_vtx(vtx) && (ul_vtx(vtx)->phys_prop != v_phys) &&
                (ul_vtx(vtx)->phys_prop->state == GAS || ul_vtx(vtx)->phys_prop->state == LIQUID))
            {
                grav_targ = ul_vtx(vtx);
            }
        }
    }
    if (!grav_targ)
    {
        vtx->mov = glm::vec3(0.0f);
        vtx->vel = glm::vec3(0.0f);
    }
    else
    {
        ratio *= glm::vec3 (v_phys->flow, 1.0, 1.0);
        if (v_phys->state != GAS)
            vtx->vel += ratio * dir_vtx(vtx, grav_targ) * _GRAVITY * (v_phys->mass - grav_targ->phys_prop->mass) / v_phys->mass;
        else
            vtx->vel += ratio * dir_vtx(vtx, grav_targ) * _GRAVITY * (grav_targ->phys_prop->mass - v_phys->mass) / v_phys->mass;
    }

    glm::vec3 dir = glm::normalize (glm::vec3 (vtx->vel.x, vtx->vel.y, 0.0));
    int n_w = w + 1.42 * dir.x, n_h = h + 1.42 * dir.y;
    vertex_obj *drag_targ = get_vtx (vtx->world, n_w, n_h);
    if (drag_targ)
        vtx->vel -= vtx->vel * drag_targ->phys_prop->drag;

    vtx->mov += (dt * MOV_SCALE) * vtx->vel; 
}

vertex_obj *move_vertex (vertex_obj *vtx)
{
    while (abs(vtx->mov.x) > 1.0 || abs(vtx->mov.y) > 1.0)
    {
        int w = vtx_w(vtx), h = vtx_h(vtx);
        glm::vec3 dir = glm::normalize (glm::vec3 (vtx->mov.x, vtx->mov.y, 0.0));
        int n_w = w + 1.42 * dir.x, n_h = h + 1.42 * dir.y;
        vertex_obj *targ = get_vtx (vtx->world, n_w, n_h);
        if (!targ)
        {
            reset_vertex (vtx);
            return NULL;
        }
        if (!swap_vertex(vtx, targ))
        {
            float vel_norm = glm::length (vtx->vel);
            vtx->vel = -0.1f * vtx->vel;
            float fr = frand();
            if (frand() < 0.5)
                vtx->vel += 0.2f * glm::vec3 (vtx->vel.x, 0.0, 0.0);
            if (frand() < 0.5)
                vtx->vel += 0.2f * glm::vec3 (-vtx->vel.x, 0.0, 0.0);
            if (frand() < 0.5)
                vtx->vel += 0.2f * glm::vec3 (0.0, vtx->vel.y, 0.0);
            if (frand() < 0.5)
                vtx->vel += 0.2f * glm::vec3 (0.0, -vtx->vel.y, 0.0);
            vtx->mov = glm::vec3(0.0);
            return vtx;
        }
        vtx = targ;
        // printf ("mx %2.2f, my %2.2f, w %d, h %d, n_w %d, n_h %d, n_mx %2.2f, n_my %2.2f\n",
        //     vtx->mov.x, vtx->mov.y, w, h, n_w, n_h, vtx->mov.x - (n_w - w), vtx->mov.y - (n_h - h));
        vtx->mov.x -= (n_w - w);
        vtx->mov.y -= (n_h - h);
    }
    return vtx;
}


void mouse_event (world_obj *world, MATERIAL_TYPE material, MOUSE_BUTTON button, int xmax, int ymax, float xoffset, float yoffset)
{
    static float last_callback_time = 0.0f;
    if (button != NONE && (world->current_time - last_callback_time > 0.005))
    {
        int w = (float)world->width * xoffset / xmax;
        int h = (float)world->height * yoffset / ymax;  
        last_callback_time = world->current_time;

        int lasth;
        int r = world->brush_size;

        if (material != MARIO)
        {
            for (int widx = -r; widx < r; widx++)
            {
                int hlen = (int)sqrt((float)r * r - widx * widx);
                if (widx == 0)
                {
                    hlen--;
                }
                for (int hidx = -hlen; hidx < hlen; hidx++)
                {
                    vertex_obj* targ =
                        get_vtx(world, w + widx, h + hidx);
                    if (targ)
                    {
                        if (button == RIGHT)
                        {
                            reset_vertex(targ);
                        }
                        else if (button == LEFT)
                        {
                            generate_vertex(targ, phys_map[material]);
                        }
                    }
                }
            }
        }
        else
        {
            for (int hidx = -4; hidx < world->extern_obj_h + 4; hidx++)
            {
                for (int widx = -4; widx < world->extern_obj_w + 4; widx++)
                {
                    vertex_obj* targ = get_vtx(world, w + widx, h + hidx);
                    if ((hidx >= 0 && hidx < world->extern_obj_h) && (widx >= 0 && widx < world->extern_obj_w))
                    {
                        vertex_obj* ex_obj = world->extern_obj + hidx * world->extern_obj_w + widx;
                        if (targ && world->extern_obj)
                        {
                            generate_vertex(targ, ex_obj->phys_prop);
                            targ->col = ex_obj->col;
                        }
                    }
                    else if (targ)
                    {
                        reset_vertex(targ);
                    }
                }
            }
        }
    }
}

void update_world_physics (world_obj *world)
{
    static float event_time[100] = {-10.0f};

    if (world->fall_on)
    {
        if (world->current_time > event_time[0] + 0.1)
        {
            generate_vertex(world, world->width / 2 - world->width / 12, 10, sand);
            generate_vertex(world, world->width / 2 + world->width / 12, 10, sand);
            generate_vertex(world, world->width / 2, 10, water);
            generate_vertex(world, world->width / 2 + 4, 10, water);
            generate_vertex(world, world->width / 2 - 4, 10, water);
            event_time[0] = world->current_time;
        }
    }

    int num_threads = std::thread::hardware_concurrency();
    num_threads = num_threads > 0 ? num_threads : 1;
    int w_section = world->width / (num_threads*2);
    for (int s = 0; s < 2; s++)
    {
        #pragma omp parallel for
        for (int t = 0; t < num_threads; t++)
        {
            for (int h = world->height - 1; h >= 0 ; h--)
            {
                for (int w = 0; w < w_section; w++)
                {
                    vertex_obj *vtx = get_vtx (world, s*w_section + (w_section*2)*t + w, h);
                    if (vtx && !vtx->updated)
                    {
                        kinetic_engine (vtx);
                        vtx = move_vertex (vtx);
                        if (vtx)
                            vtx->updated = true;
                    }
                }
            }
        }
    }

}

void update_render_obj(render_obj* r_obj, vertex_obj* v_obj, int h, int w)
{
    r_obj->col = v_obj->col;
    r_obj->reflect = v_obj->phys_prop->reflect;
    r_obj->radiation = v_obj->phys_prop->radiate;
    r_obj->vpos = glm::vec3(w, h, 0.0);
}

void update_word_render_list(world_obj* world)
{
#pragma omp parallel for collapse(2)
    for (int h = 0; h < world->height; h++)
    {
        for (int w = 0; w < world->width; w++)
        {
            vertex_obj* v_obj = get_vtx(world, w, h);
            v_obj->updated = false;
            render_obj* r_obj = world->render_list + (v_obj - world->vertex_list);
            r_obj->pos.x = ((float)w * 2.0 - world->width) / world->width;
            r_obj->pos.y = (-(float)h * 2.0 + world->height) / world->height;
            update_render_obj(r_obj, v_obj, h, w);
        }
    }
}

void load_extern_obj(world_obj* world, const char* img_loc)
{
    FILE* f = fopen(img_loc, "rb");
    if (f == NULL)
    {
        printf("External image does not exist.\n");
        world->extern_obj = NULL;
        world->extern_obj_h = 0;
        world->extern_obj_w = 0;
        return;
    }

    int32_t file_size, px_arr_offset, dib_header_size;
    uint16_t bits_per_pixel;
    char* file_dump = NULL, * pixel_ptr = NULL;
    bool is_height_reversed = false;


    char header[14] = { 0 };
    fread(header, sizeof(char), 14, f);
    if (!(*header == 'B' && *(header + 1) == 'M'))
    {
        printf("External image not in BMP format. %c, %c\n", *header, *(header + 1));
        return;
    }

    file_size = *(uint32_t*)(header + 2);
    px_arr_offset = *(uint32_t*)(header + 10);

    file_dump = (char*)calloc(sizeof(char), file_size + 32);
    memcpy(file_dump, header, 14);
    fread(file_dump + 14, sizeof(char), file_size - 14, f);

    world->extern_obj_w = *(int32_t*)(file_dump + 14 + 4);
    world->extern_obj_h = *(int32_t*)(file_dump + 14 + 8);
    bits_per_pixel = *(int16_t*)(file_dump + 14 + 14);
    if (bits_per_pixel != 24 && bits_per_pixel != 32)
    {
        if (file_dump)
            free(file_dump);
        printf("External image not in 24-bit or 32-bit format. %d\n", bits_per_pixel);
        return;
    }
    world->extern_obj = (vertex_obj*)calloc(sizeof(vertex_obj), world->extern_obj_h * world->extern_obj_w);
    for (int h = 0; h < world->extern_obj_h; h++)
    {
        for (int w = 0; w < world->extern_obj_w; w++)
        {
            vertex_obj* v_obj = world->extern_obj + h * world->extern_obj_w + w;
            v_obj->world = world;
            reset_vertex(v_obj);
        }
    }
    if (world->extern_obj_h < 0)
    {
        world->extern_obj_h = -world->extern_obj_h;
        is_height_reversed = true;
    }

    //printf("External image info: Height %d, Width: %d, Bits per pixel: %d, is_height_reversed: %d\n",
    //    world->extern_obj_h, world->extern_obj_w, bits_per_pixel, is_height_reversed);

    pixel_ptr = file_dump + px_arr_offset;
    int row_size = ((world->extern_obj_w * (bits_per_pixel / 8) + 3) & (~3));
    for (int h = 0; h < world->extern_obj_h; h++)
    {
        for (int w = 0; w < world->extern_obj_w; w++)
        {
            vertex_obj* v_obj = world->extern_obj + (world->extern_obj_h - 1 - h) * world->extern_obj_w + w;
            if (is_height_reversed)
                v_obj = world->extern_obj + h * world->extern_obj_w + w;
            uint32_t col = *((uint32_t*)(pixel_ptr + w * (bits_per_pixel / 8)));
            glm::vec3 col_v = glm::vec3((float)((col >> 16) & 0xff) / 256, (float)((col >> 8) & 0xff) / 256, (float)((col >> 0) & 0xff) / 256);
            bool generated = false;
            for (physics_property** phys_iter = &(phys_map[0]); *phys_iter != NULL; phys_iter++)
            {
                if (glm::length (col_v - (*phys_iter)->diffuse) < 0.001)
                {
                    generate_vertex(v_obj, *phys_iter);
                    generated = true;
                    break;
                }
            }
            if (generated == false)
            {
                generate_vertex(v_obj, mario);
                v_obj->col = col_v;
            }
        }
        pixel_ptr += row_size;
    }


    if (file_dump)
        free (file_dump);
    fclose(f);
}

world_obj* make_world (int width, int height, int load_world)
{
    // printf ("Building world of width %d, height %d\n", width, height);
    phys_map[AIR] = air;
    phys_map[SAND] = sand;
    phys_map[WATER] = water;
    phys_map[STEAM] = steam;
    phys_map[ROCK] = rock;
    phys_map[LAVA] = lava;
    phys_map[LIGHT] = light;
    phys_map[MARIO] = mario;

    world_obj *world_out = (world_obj*)calloc (1, sizeof(world_obj));
    world_out->event_flag = true;
    world_out->event_time = 0.0f;
    world_out->delta_time = 0.0f;
    world_out->current_time = 0.0f;
    world_out->height = height;
    world_out->width = width;
    world_out->brush_size = 5;
    world_out->fall_on = 1;
    world_out->vertex_list = (vertex_obj*)calloc (width*height, sizeof(vertex_obj));
    world_out->render_list = (render_obj*)calloc (width*height, sizeof(render_obj));
    int idx = 0;
    for (int h = 0; h < height; h++)
    {
        for (int w = 0; w < width; w++)
        {
            vertex_obj *v_obj = world_out->vertex_list + idx;
            idx++;
            v_obj->world = world_out;
            reset_vertex (v_obj);
        }
    }
   
    if (load_world == 1)
    {
        load_extern_obj(world_out, "world1-1.bmp");
        for (int h = 0; h < height; h++)
        {
            for (int w = 0; w < width; w++)
            {
                vertex_obj* targ = get_vtx(world_out, w, h);
                if ((h >= 0 && h < world_out->extern_obj_h) && (w >= 0 && w < world_out->extern_obj_w))
                {
                    vertex_obj* ex_obj = world_out->extern_obj + h * world_out->extern_obj_w + w;
                    if (targ && world_out->extern_obj)
                    {
                        generate_vertex(targ, ex_obj->phys_prop);
                        targ->col = ex_obj->col;
                    }
                }
                else if (targ)
                {
                    reset_vertex(targ);
                }
            }
        }
        free(world_out->extern_obj);
        for (int h = 0; h < 2; h++)
        {
            for (int w = 0; w < width / 48; w++)
            {
                generate_circle(world_out, width * 15 / 16  - w, height *1/ 9 + 2 + h, width / 36, light);
                generate_circle(world_out, width * 1 / 16  + w, height *1/ 9 + 2 + h, width / 36, light);
                //generate_circle(world_out, width / 2 - width/32 + w, height/ 9 + 2 + h, width / 36, light);
            }
        }
    }
    else
    {
        for (int h = 0; h < 3; h++)
        {
            for (int w = 0; w < world_out->width / 4; w++)
                generate_vertex(world_out, world_out->width / 2 - world_out->width / 8 + w, world_out->height * 6 / 8 - h, rock);
        }
        generate_circle(world_out, width * 15 / 16 - 10, height / 9 + 10, width / 24, light);
    }

    load_extern_obj(world_out, "extern_img.bmp");
    

    update_word_render_list (world_out);

    glGenVertexArrays(1, &world_out->VAO);
    glBindVertexArray(world_out->VAO);
    // SSBO for rendering data
    glGenBuffers(1, &world_out->SSBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, world_out->SSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(render_obj)*width*height, world_out->render_list, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, world_out->SSBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0); // unbind
    // Texture to store render output
    glGenFramebuffers(1, &world_out->FBO);
    glBindFramebuffer(GL_FRAMEBUFFER, world_out->FBO);
    glGenTextures(1, &world_out->TID);
    glBindTexture(GL_TEXTURE_2D, world_out->TID);
    glTexImage2D(GL_TEXTURE_2D, 0,GL_RGB, world_out->width, world_out->height, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, world_out->TID, 0);
    GLenum DrawBuffers[1] = {GL_COLOR_ATTACHMENT0};
    glDrawBuffers(1, DrawBuffers); // "1" is the size of DrawBuffers
    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        printf ("Framebuffer creation failed!\n");
    
    // Simple Quad VAO for final screen output
    static const float quadVertices[] = {
        // positions        // texture Coords
        -1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
        -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
        1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
        1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
    };
    glGenVertexArrays(1, &world_out->QUAD_VAO);
    glGenBuffers(1, &world_out->QUAD_VBO);
    glBindVertexArray(world_out->QUAD_VAO);
    glBindBuffer(GL_ARRAY_BUFFER, world_out->QUAD_VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    return world_out;
}

void free_world (world_obj *world)
{
    if (world->vertex_list)
        free (world->vertex_list);
    if (world->render_list)
        free (world->render_list);
    if (world->extern_obj)
        free(world->extern_obj);
}

void update_world (world_obj *world)
{
    world->delta_time = glfwGetTime() - world->current_time;
    world->current_time = world->current_time + world->delta_time;
    update_world_physics (world);
    update_word_render_list (world);
}

void render_world (world_obj *world, Shader *shader, int rtx_on)
{
    shader->use();
    glBindVertexArray(world->VAO);
    glViewport(0, 0, world->width, world->height);
    glBindFramebuffer(GL_FRAMEBUFFER, world->FBO);
    glClear(GL_COLOR_BUFFER_BIT);
    glUniform1i(glGetUniformLocation(shader->ID, "world_w"), world->width);
    glUniform1i(glGetUniformLocation(shader->ID, "world_h"), world->height);
    glUniform1i(glGetUniformLocation(shader->ID, "rtx_on"), rtx_on);
    
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, world->SSBO);
    GLvoid* p = glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_WRITE_ONLY);
    memcpy(p, world->render_list, sizeof(render_obj)*world->width*world->height);
    glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
    
    glDrawArrays(GL_POINTS, 0, world->height*world->width);
}

void draw_world (world_obj *world, Shader *shader, int scr_w, int scr_h)
{
    shader->use();
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glUniform1i(glGetUniformLocation(shader->ID, "colorMap"), 1);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, world->TID);
    glBindVertexArray(world->QUAD_VAO);
    glViewport(0, 0, scr_w, scr_h);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
}