#version 430
struct render_obj
{
    float pos[3];
    float vpos[3];
    float reflect[3]; 
    float radiation[3];
    float col[3];
};

out vec3 vtxCol;

layout(std430, binding = 1) buffer Points{
    render_obj data[];
};

uniform int world_w;
uniform int world_h;
uniform int rtx_on;

render_obj get_obj (vec3 loc)
{
    return data[(int(loc.y)*world_w + int(loc.x))];
}

vec3 raytrace (vec3 targ, vec3 org)
{
    vec3 out_col = vec3(0.0);
    vec3 delta = targ - org;
    vec3 trace = org;
    int iter = 0;
    while (iter < world_w && length(delta) > 1.0)
    {
        int w = int(trace.x), h = int(trace.y);
        vec3 dir = normalize (vec3(delta.x, delta.y, 0.0));
        if (iter > world_w/3 && length(out_col) < 0.1)
            return out_col;

        render_obj trace_obj = get_obj(trace);
        vec3 ref_factor = vec3 (trace_obj.reflect[0], trace_obj.reflect[1], trace_obj.reflect[2]);
        vec3 rad_factor = vec3 (trace_obj.radiation[0], trace_obj.radiation[1], trace_obj.radiation[2]);
        vec3 col_factor = vec3 (trace_obj.col[0], trace_obj.col[1], trace_obj.col[2]);
        out_col = out_col*(1-ref_factor) + (out_col + rad_factor)*col_factor*ref_factor;

        // out_col += rad_factor;

        trace = trace + dir*1.414;
        delta = targ - trace;
        iter++;
    }
    return out_col;
}

void main()
{
    render_obj obj = data[gl_VertexID];

    vec3 pos = vec3 (obj.vpos[0], obj.vpos[1], obj.vpos[2]);
    vec3 ref = vec3 (obj.reflect[0], obj.reflect[1], obj.reflect[2]);
    vec3 rad = vec3 (obj.radiation[0], obj.radiation[1], obj.radiation[2]);
    vec3 col = vec3 (obj.col[0], obj.col[1], obj.col[2]);

    vec3 light = vec3 (0.0);
    if (rtx_on == 1)
    {
        int trace_num = 1;
        for (int w = 0; w <= world_w; w += world_w/4)
        {
            light += raytrace (pos, vec3 (w, 0, 0));
            light += raytrace (pos, vec3 (w, world_h, 0));
            trace_num += 2;
        }
        for (int h = world_h/3; h <= world_h*2/3; h += world_h/3)
        {
            light += raytrace (pos, vec3 (0, h, 0));
            light += raytrace (pos, vec3 (world_w, h, 0));
            trace_num += 2;
        }
        
        light /= float(trace_num * 5/8);

        light = (light + rad + vec3(0.1))*col*ref;

        if (length(light) < 0.1)
        {
            light += vec3 (0.01, 0.01, 0.025);
        }
    }
    else
    {
        light = col;
    }

    gl_Position = vec4 (obj.pos[0], obj.pos[1], obj.pos[2], 1.0);
    vtxCol = light;
    
}
