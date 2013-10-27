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

// near, far, and far_ are x,y,a coordinates.
// far is the desired (0-force/torque) position of the far point, and far_ is
// the actual position of the far point.
vec3 accel(vec3 near, vec2 delta, vec3 far)
{
    // A pixel can't influence itself.
    if (delta.x == 0.0f && delta.y == 0.0f)     return vec3(0.0f);

    // Vectors pointing from far to near
    vec3 d = near - far;

    // Start with the force contribution due to linear spring
    float magnitude = k_linear * (length(delta) - length(d.xy));
    vec3 force = vec3(magnitude * normalize(d.xy), 0.0f);

    // Find the force from the far point's angular spring torquing
    // being exerted on the near point.
    {
        float angle = atan(d.y, d.x);

        // Find the angle between our desired beam and the actual beam, from
        // the perspective of the far point (which is exerting this force).
        float d_angle = angle - (atan(-delta.y, -delta.x) + far.z);
        while (d_angle < -M_PI)    d_angle += 2*M_PI;
        while (d_angle >  M_PI)    d_angle -= 2*M_PI;

        vec2 force_direction = vec2(cos(angle + M_PI/2),
                                    sin(angle + M_PI/2));

        // Force from torsional spring at far point:
        // direction vector * (angle * k * lever arm length)
        force.xy += force_direction * (-d_angle * k_torsional * length(d.xy));
    }
    force /= m;

    // Torque due to the near point's angular spring
    {
        // Desired angle from the perspective of the near point
        float d_angle = atan(-d.y, -d.x) - (atan(delta.y, delta.x) + near.z);
        while (d_angle < -M_PI)    d_angle += 2*M_PI;
        while (d_angle >  M_PI)    d_angle -= 2*M_PI;

        force.z += d_angle * k_torsional / I;
    }

    return force;
}

void main()
{
    vec3 near = texture2D(pos, tex_coord).xyz;
    vec3 total_accel = vec3(0.0f);

    // Iterate over the nine neighboring cells, accumulating forces.
    for (int dx=-1; dx <= 1; ++dx) {
        for (int dy=-1; dy <= 1; ++dy) {
            // Pick an offset that will give us the next pixel
            // in the desired direction.
            vec2 delta = vec2(dx, dy);
            float tex_dx = float(dx) / float(ship_size.x);
            float tex_dy = float(dy) / float(ship_size.y);
            vec2 tex_delta = vec2(tex_dx, tex_dy);

            // If the chosen pixel is within the image (i.e. it has texture
            // coordinates between 0 and 1) and is marked as filled in the
            // pixel occupancy texture, then find and add its acceleration.
            if (tex_coord.x + tex_dx >= 0.0f && tex_coord.x + tex_dx <= 1.0f &&
                tex_coord.y + tex_dy >= 0.0f && tex_coord.y + tex_dy <= 1.0f &&
                texture2D(filled, tex_coord + tex_delta).r != 0)
            {
                // Get the actual location of the far point from the texture
                vec3 far = texture2D(pos, tex_coord + tex_delta).xyz;

                // Calculate and accumulate acceleration
                total_accel += accel(near, delta, far);
            }
        }
    }

    // Apply damping based on velocity
    vec3 near_vel = texture2D(vel, tex_coord).xyz;
    total_accel += vec3(-c_linear * near_vel.xy / m,
                        -c_torsional * near_vel.z / I);

    gl_FragColor = vec4(total_accel, 1.0f);
}
