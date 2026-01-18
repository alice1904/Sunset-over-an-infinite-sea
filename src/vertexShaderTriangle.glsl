#version 430
//#version 330 core            // Minimal GL version support expected from the GPU

layout(location=0) in vec3 vPosition;
layout(location=1) in vec3 vColor;

//uniform mat4 viewMat, projMat;

//camera parameters
uniform vec3 forward; 
uniform vec3 up;
uniform vec3 right;
uniform float half_height;
uniform float half_width;


out vec3 fPosition;
out vec3 fColor;

//ray information
out vec3 non_normalized_ray_direction; //direction from the origin of the camera to the 'pixel' of the virtual screen.
                                       //It gives us one ray for each fragment.

void main() {
        vec4 worldPosition = vec4(vPosition, 1.0);
        fPosition = vec3(worldPosition.x, worldPosition.y, worldPosition.z)/worldPosition.a;
        fColor = vColor;
        gl_Position = vec4(vPosition, 1.0);

        non_normalized_ray_direction = forward 
                + vPosition.y * half_height * up 
                + vPosition.x * half_width * right ;


}

