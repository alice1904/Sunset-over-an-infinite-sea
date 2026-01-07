


## Storage Buffer

To pass my vertices and triangles to the fragment shader, I used storage buffer. I wanted to interpret the buffer's data as vec3 array in the shader, but the problem was that in glsl, vec3 have to be aligned like vec4. If verticesPosition is a vec3 array, the address of verticesPosition[1] is the address of verticesPosition[0] + 4 * sizeof(float), while in C the address would be the address of verticesPosition[0] + 3 * sizeof(float). The solution I found was to add 1 float between each vec3 in the C struct before sending it to the shader.
