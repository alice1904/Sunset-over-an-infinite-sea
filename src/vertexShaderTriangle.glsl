#version 430

layout(location=0) in vec3 vPosition;

//camera parameters
uniform vec3 forward; 
uniform vec3 up;
uniform vec3 right;
uniform float half_height;
uniform float half_width;

//ray information
out vec3 non_normalized_ray_direction; //direction from the origin of the camera to the 'pixel' of the virtual screen.
                                       //It gives us one ray for each fragment.

void main() {
        gl_Position = vec4(vPosition, 1.0);

        non_normalized_ray_direction = forward 
                + vPosition.y * half_height * up 
                + vPosition.x * half_width * right ;


}

