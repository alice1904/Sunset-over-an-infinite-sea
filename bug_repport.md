


## Storage Buffer

To pass my vertices and triangles to the fragment shader, I used storage buffer. I wanted to interpret the buffer's data as vec3 array in the shader, but the problem was that in glsl, vec3 have to be aligned like vec4. If verticesPosition is a vec3 array, the address of verticesPosition[1] is the address of verticesPosition[0] + 4 * sizeof(float), while in C the address would be the address of verticesPosition[0] + 3 * sizeof(float). The solution I found was to add 1 float between each vec3 in the C struct before sending it to the shader.


## Ray information

We have one initial ray for each fragment of the quad I display. This ray comes from the position of the screen and points towards the world position of the corresponding point of virtual screen in front of the camera.
The origin of the ray is always the camera position and is passed to the fragment shader as a uniform value.
I used the parameters of the camera (Fov, aspectRatio and up and right direction) to determine the direction of the ray of each fragment. Concretely, I compute the direction of the ray in the vertex shader for each corner of the quad with the following formula ( (x, y) is the position of each vertex with x and y ranging from -1 to 1 ) : 

$$ rayDirection = forward + x*halfWidth*right + y*halfHeight*up $$

where $$halfHeight = tan(Fov/2)$$ $$halfWidth = aspectRation*halfHeight$$

The direction is then interpolated for each fragment.

## Object Properties

To differentiate the objects, I simply passed another storage buffer called objectProperties which stored both the objects' properties (color, reflection/refraction ratio, diffuse/specular ratio) and the index of the last triangle. The challenge was to put all this information in a struct where the color would be well aligned (see Storage Buffer part above). Fortunately, I had 4 floats (4 bytes in glsl), 1 int (4 bytes in glsl) and 1 vec3 (3 * 4 bytes) which makes exactly 8 * 4 bytes (remember : vec3 has to be aligned on 4 * 4 bytes). To be sure the int will take 4 bytes in the C struct, I used the type int32_t from the stdint.h library.

Then each time I find the closest triangle intersected by a ray, I have to browse the list of objects to determine from which object it is, but it is not a big problem because I only have a couple of objects.

## Recursion and trees

Unfortunately, you cannot have recursive functions in glsl. But ray tracing needs recursion to track all the rays obtained by refraction and reflection. I decided to implement a tree representing the cast rays. To do so I used an array of size $2**(height + 1) - 1$. The nodes are stored in the array's cells. The sons'index of the node of index i are $2*i+1$ and $2*i+2$. For each node I stored a struct containing information about the ray. I first compute the direction and origin of each ray from root to leaves and then compute the final color from leaves to root. 

## sun and gradiennt

blabla

## Waves

For the sea, I took inspiration from the video game Monument Valley. 
I split a rectangle into triangles (a regular grid, each cell being split into 2 triangles). The height of each vertex is random in a certain range, which gives us a nice blocky sea. Then I attached a vertical spring to each vertex. I had to put a different spring constant to each vertex, otherwise, the vertices would be at the height 0 at the same time, and then we have an ugly flat sea for a moment and a brutal change of triangle colors when the triangles' orientation change at the same time.

Unfortunately, with this animation, there is no coherence between the wave crests while in the sea you usually have huge waves in addition to small oscillation of the water. That's why I added a vertical offset to each vertex which is obtained with the plane wave formula : 

$ offset = cos(w*t-\vec{k}.\vec{x}) $ 

In our simulation, $\vec{x}$ is the position in the horizontal (x, z) plane.

# Bugs

ray tracing : La couleur renvoyée par les rayons refléchis et réfractés n'étaient pas logiques. J'avais une fonction censée écrire les informations des nouveaux rayons (et notamment leur position et leur direction) dans des variables passées à la fonction par référence. Je redéclarais une variable déclaré comme 'out' (variable de sortie) ce qui fait qu'une fois la fonction terminée, les informations n'étaient pas initialisées. 