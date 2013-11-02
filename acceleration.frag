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

// near and far are x,y,a coordinates.
// delta is the nominal vector from near to far.  In all likelihood, far
// is somewhere else, which exerts a force.
vec3 accel(vec3 near, vec2 delta, vec3 far)
{
    // A pixel can't influence itself.
    if (delta.x == 0.0f && delta.y == 0.0f)     return vec3(0.0f, 0.0f, 0.0f);

    // Vectors pointing from far to near
    vec3 d = near - far;

    // Start with the force contribution due to linear spring
    float magnitude = k_linear * (length(delta.xy) - length(d.xy));
    vec3 force = vec3(magnitude * normalize(d.xy), 0.0f);

    // Find the force from the far point's angular spring torquing
    // being exerted on the near point.
    if (true) {
        // Find the angle between our desired beam and the actual beam, from
        // the perspective of the far point (which is exerting this force).
        float d_angle = atan(d.y, d.x) - atan(-delta.y, -delta.x) + far.z;
        while (d_angle < -M_PI)    d_angle += 2*M_PI;
        while (d_angle >  M_PI)    d_angle -= 2*M_PI;

        // Force direction is 90 degrees from moment arm
        vec2 force_direction = normalize(vec2(-d.y, d.x));

        // Acceleration from torsional spring at far point:
        // direction vector * (angle * k * lever arm length) / mass
        force.xy += force_direction *
            (-d_angle * k_torsional * length(d.xy));
    }

    // Torque due to the near point's angular spring
    if (true) {
        // Desired angle from the perspective of the near point
        float d_angle = atan(-d.y, -d.x) - atan(delta.y, delta.x) - near.z;
        while (d_angle < -M_PI)    d_angle += 2*M_PI;
        while (d_angle >  M_PI)    d_angle -= 2*M_PI;

        force.z = d_angle * k_torsional;
    }

    return vec3(force.xy / m, force.z / I);
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
                vec3 far = texture2D(pos, far_tex_coord).xyz;

                // Calculate and accumulate acceleration
                total_accel += accel(near, delta, far);
            }
        }
    }

    // Apply damping based on velocity
    vec3 near_vel = texture2D(vel, tex_coord).xyz;
    total_accel += vec3(-c_linear    * near_vel.xy / m,
                        -c_torsional * near_vel.z  / I);

    gl_FragColor = vec4(total_accel, 1.0f);
}
