#version 120

#define M_PI 3.1415926535

////////////////////////////////////////////////////////////////////////////////

varying vec2 tex_coord;

uniform sampler2D pos;
uniform sampler2D vel;
uniform sampler2D filled;

uniform ivec2 ship_size;

uniform float k_linear;     // linear spring constant
uniform float k_torsional;  // angular spring constant

uniform float c_linear;     // linear damping
uniform float c_torsional;  // torsional damping

uniform float m;    // point's mass
uniform float I;    // point's inertia

////////////////////////////////////////////////////////////////////////////////

vec3 accel(const vec3 a, const vec3 a_dot, const vec3 delta,
           const vec3 b, const vec3 b_dot)
{
    const vec3 v = vec3(b.xy - a.xy, atan(b.y - a.y, b.x - a.x));
    const vec3 v_ = vec3(normalize(v.xy), v.z);
    const vec3 p_ = vec3(v_.x, -v_.y, v_.z + M_PI/2);

    // Force from linear spring
    const vec3 F_kL = vec3(
            -k_linear * (length(d.xy) - length(v.xy)) * v_.xy,
            0.0f);

    // Torque from near torsional spring
    const vec3 T_k = vec3(0.0f, 0.0f,
            -k_torsional * (d.z + a.z - v.z));

    // Force from far torsional spring
    const vec3 F_kT = vec3(
            -p_.xy * k_torsional * length(v.xy) * (d.z + b.z - v.z),
            0.0f);

    // Force from linear damper (check this!)
    const vec3 F_cL = vec3(
            v_.xy * c_linear * dot(b_dot.xy - a_dot.xy, v_.xy),
            0.0f);

    // Torque from near torsional damper
    const vec3 T_c = vec3(0.0f, 0.0f,
            -c_torsional * (a_dot.z - dot(b_dot.xy - a_dot.xy, p_.xy) / length(v.xy)));

    // Force from far torsional damper
    const vec3 F_cT = vec3(
            -p_.xy * c_torsional * length(v.xy) * (b_dot.z - dot(b_dot.xy - a_dot.xy, p_.xy) / length(v.xy)),
            0.0f);

    return F_kL + T_k + F_kT + F_cL + T_c + F_cT;
}

////////////////////////////////////////////////////////////////////////////////

void main()
{
    vec3 near_pos = texture2D(pos, tex_coord).xyz;
    vec3 near_vel = texture2D(vel, tex_coord).xyz;

    vec3 total_accel = vec3(0.0f);

    // Iterate over the nine neighboring cells, accumulating forces.
    for (int dx=-1; dx <= 1; ++dx) {
        for (int dy=-1; dy <= 1; ++dy) {
            // Pick an offset that will give us the next pixel
            // in the desired direction.
            vec3 delta = vec3(dx, dy, atan(dy, dx));
            vec2 far_tex_coord = tex_coord + vec2(delta.x / ship_size.x,
                                                  delta.y / ship_size.y);
            // If the chosen pixel is within the image (i.e. it has texture
            // coordinates between 0 and 1) and is marked as filled in the
            // pixel occupancy texture, then find and add its acceleration.
            if (far_tex_coord.x > 0.0f && far_tex_coord.x < 1.0f &&
                far_tex_coord.y > 0.0f && far_tex_coord.y < 1.0f &&
                texture2D(filled, far_tex_coord).r != 0)
            {
                // Get the actual location of the far point from the texture
                vec3 far_pos = texture2D(pos, far_tex_coord).xyz;
                vec3 far_vel = texture2D(vel, far_tex_coord).xyz;

                total_accel += accel(near, near_vel, delta,
                                     far, far_vel);
            }
        }
    }

    // Accelerate engine pixels upwards
    //if (texture2D(filled, tex_coord).r == 1.0f)   total_accel += vec3(0.0f, 1000.0f, 0.0f);

    gl_FragColor = vec4(total_accel, 1.0f);
}
