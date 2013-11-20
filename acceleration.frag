#version 330

////////////////////////////////////////////////////////////////////////////////

uniform sampler2D pos;
uniform sampler2D vel;
uniform sampler2D filled;

uniform ivec2 ship_size;

uniform float k;     // linear spring constant
uniform float c;     // linear damping

uniform float m;    // point's mass
uniform float I;    // point's inertia

uniform int boost;

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

    vec2 force = F_kL + F_cL;

    return force / m;
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

    vec2 near_pos = texture(pos, tex_coord).xy;
    vec2 near_vel = texture(vel, tex_coord).xy;

    vec2 total_accel = vec2(0.0f);

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
                vec2 far_pos = texture(pos, far_tex_coord).xy;
                vec2 far_vel = texture(vel, far_tex_coord).xy;

                // Find the nominal offset and angle between the points.
                vec2 delta = vec2(float(dx), float(dy));

                total_accel += accel(near_pos, near_vel, delta,
                                     far_pos, far_vel);
            }
        }
    }

    // Accelerate engine pixels upwards
    if (boost != 0 && texture(filled, tex_coord).r == 1.0f)
    {
        total_accel += vec2(0.0f, 1000.0f);
    }

    fragColor = vec4(total_accel, 0.0f, 1.0f);
}
