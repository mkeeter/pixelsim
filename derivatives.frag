#version 330

////////////////////////////////////////////////////////////////////////////////

uniform sampler2D filled;
uniform sampler2D state;

uniform ivec2 ship_size;

uniform float k;     // linear spring constant
uniform float c;     // linear damping

uniform float m;    // point's mass
uniform float I;    // point's inertia

uniform int thrustEnginesOn;
uniform int leftEnginesOn;
uniform int rightEnginesOn;

out vec4 fragColor;

////////////////////////////////////////////////////////////////////////////////

vec2 accel(vec2 a, vec2 a_dot, vec2 d,
           vec2 b, vec2 b_dot)
{
    vec2 v  = b.xy - a.xy;
    vec2 v_ = normalize(v.xy);

    // Force from linear spring
    vec2 F_kL = -k * (length(d.xy) - length(v.xy)) * v_.xy;

    // Force from linear damper (check this!)
    vec2 F_cL = v_.xy * c * dot(b_dot.xy - a_dot.xy, v_.xy);

    return (F_kL + F_cL) / m;
}

////////////////////////////////////////////////////////////////////////////////

void main()
{
    vec2 tex_coord = vec2(gl_FragCoord.x / float(ship_size.x + 1),
                          gl_FragCoord.y / float(ship_size.y + 1));

    if (texture(filled, tex_coord).r == 0)
    {
        fragColor = vec4(0.0f, 0.0f, 0.0f, 0.0f);
        return;
    }

    vec4 near_state = texture(state, tex_coord);
    vec2 near_pos = near_state.rg;
    vec2 near_vel = near_state.ba;

    vec2 total_accel = vec2(0.0f);
    vec2 total_angle = vec2(0.0f);

    // Iterate over the nine neighboring cells, accumulating forces.
    for (int dx=-1; dx <= 1; ++dx) {
        for (int dy=-1; dy <= 1; ++dy) {
            // Find the texture coordinate of the far pixel's data
            vec2 far_tex_coord = tex_coord +
                                 vec2(float(dx) / float(ship_size.x + 2),
                                      float(dy) / float(ship_size.y + 2));
            // If the chosen pixel is within the image (i.e. it has texture
            // coordinates between 0 and 1) and is marked as filled in the
            // pixel occupancy texture, then find and add its acceleration.
            if (far_tex_coord.x > 0.0f && far_tex_coord.x < 1.0f &&
                far_tex_coord.y > 0.0f && far_tex_coord.y < 1.0f &&
                (dx != 0 || dy != 0) &&
                texture(filled, far_tex_coord).r != 0)
            {
                // Get the actual state of the far point from the textures
                vec4 far_state = texture(state, far_tex_coord);
                vec2 far_pos = far_state.rg;
                vec2 far_vel = far_state.ba;

                // Find the nominal offset and angle between the points.
                vec2 delta = vec2(float(dx), float(dy));

                // Accumulate angle between desired and actual positions.
                float angle = atan(far_pos.y - near_pos.y,
                                   far_pos.x - near_pos.x) - atan(dy, dx);
                total_angle += vec2(cos(angle), sin(angle));

                // Find the acceleration caused by this node-neighbor linkage
                total_accel += accel(near_pos, near_vel, delta,
                                     far_pos, far_vel);
            }
        }
    }

    // Accelerate engine pixels upwards
    float type = texture(filled, tex_coord).r;
    if ((type == SHIP_ENGINE_THRUST/255.0f && thrustEnginesOn != 0) ||
        (type == SHIP_ENGINE_RIGHT/255.0f &&  rightEnginesOn != 0) ||
        (type == SHIP_ENGINE_LEFT/255.0f &&   leftEnginesOn != 0))
    {
        float angle = atan(total_angle.y, total_angle.x);
        total_accel += vec2(-sin(angle), cos(angle))*1000.0f;
    }

    // Output the final derivatives:
    fragColor = vec4(near_vel, total_accel);
}
