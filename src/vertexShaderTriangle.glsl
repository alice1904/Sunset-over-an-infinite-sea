#version 330 core            // Minimal GL version support expected from the GPU

layout(location=0) in vec3 vPosition;
layout(location=1) in vec3 vColor;

uniform mat4 viewMat, projMat;


out vec3 fPosition;
out vec3 fColor;

void main() {
        vec4 worldPosition = vec4(vPosition, 1.0);
        fPosition = vec3(worldPosition.x, worldPosition.y, worldPosition.z)/worldPosition.a;
        fColor = vColor;
        //gl_Position = projMat * viewMat * worldPosition; // mandatory to rasterize properly
        gl_Position = vec4(vPosition, 1.0);
}

