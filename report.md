


## Storage Buffer

To pass my vertices and triangles to the fragment shader, I used storage buffer. I wanted to interpret the buffer's data as vec3 array in the shader, but the problem was that in glsl, vec3 have to be aligned like vec4. If verticesPosition is a vec3 array, the address of verticesPosition[1] is the address of verticesPosition[0] + 4 * sizeof(float), while in C the address would be the address of verticesPosition[0] + 3 * sizeof(float). The solution I found was to add 1 float between each vec3 in the C struct before sending it to the shader.


## Ray information

We have one initial ray for each fragment of the quad I display. This ray comes from the position of the screen and points towards the world position of the corresponding point of virtual screen in front of the camera.
The origin of the ray is always the camera position and is passed to the fragment shader as a uniform value.
I used the parameters of the camera (Fov, aspectRatio and up and right direction) to determine the direction of the ray of each fragment. Concretely, I compute the direction of the ray in the vertex shader for each corner of the quad with the following formula ( (x, y) is the position of each vertex with x and y ranging from -1 to 1 ) : 

$$ rayDirection = forward + x*halfWidth*right + y*halfHeight*up $$

where $$halfHeight = tan(Fov/2)$$ $$halfWidth = aspectRation*halfHeight$$

The direction is then interpolated for each fragment.