#set document(
  title: "IGR Project \n Sunset over an infinite sea",
  author: ("Alice JEANNIN"),
)

#let title(body) = text(size: 20pt, align(center, heading(numbering: none, level: 1, [
  #if body == none {context {document.title}} else {body}
])))

// Multi-page figures
#show figure: set block(breakable: true)
#show rect: set block(breakable: true)

// Paragraph style definition
#set par(justify: true)

#set heading(numbering: "1.", depth: 2)
#set page(numbering: "1")

#let code-figure(caption, body) = figure(
  caption: caption,
  supplement: [Snippet],
  rect(
    width: 90%,
    align(
      left
    )[#body]
  )
)



#title(none)
<title>

#align(center)[ #context { document.author.join(", ") } ]


#linebreak() 
#linebreak() 

#align(center)[
  #set par(justify: false)
  *Introducion* \
  For this project, I wanted to focus more on the aesthetic than trying to reproducing one or more specific methods I would have found in research papers. This project aims to render a sea under a sunset. The camera can be moved in such a way that the sea seems infinite. I used ray tracing for the rendering. My project focuses more on the geometry and simulation part (maybe more on geometry). The simulation consists in imitating waves and water behavior more in a descriptive way than in a physical way. For the geometry part, I created the mesh from scratch using code and I update it each time the camera moves using an algorithm I found myself.
]

#linebreak() 
= Ray tracing


== Concrete implementation

I decided to do the computation of my ray tracer on the GPU. Basically, my vertex shader receives 2 triangles that represents the screen and passes to the fragment shader the direction of a ray to shoot for each fragment. The ray tracing is therefore performed in the fragment shader.

=== Storage buffer

To pass my vertices and triangles to the fragment shader, I used storage buffers. I wanted to interpret the buffer's data as vec3 array in the shader, but the problem was that in glsl, vec3 have to be aligned like vec4. If verticesPosition is a vec3 array, the address of verticesPositions[1] is the address of verticesPositions[0] + 4 \* sizeof(float), while in C the address would be the address of verticesPositions[0] + 3 \* sizeof(float). The solution I found was to add 1 float between each vec3 in the C struct before sending it to the shader.

=== Object Properties 

To differentiate the objects, I simply passed another storage buffer called objectProperties which stored both the objects' properties (color, reflection/refraction ratio, diffuse/specular ratio) and the index of the last triangle. The challenge was to put all this information in a struct where the color would be well aligned (see Storage Buffer part above). Fortunately, I had 4 floats (4 bytes in glsl), 1 int (4 bytes in glsl) and 1 vec3 (3 \* 4 bytes) which makes exactly 8 \* 4 bytes (remember : vec3 has to be aligned on 4 \* 4 bytes). To be sure the int will take 4 bytes in the C struct, I used the type int32_t from the stdint.h library.

Then each time I find the closest triangle intersected by a ray, I have to browse the list of objects to determine from which object it is, but it is not a big problem because I only have a couple of objects (only 1 beging the sea in the final scene).


=== Recursion and trees

Unfortunately, you cannot have recursive functions in glsl. But ray tracing needs recursion to track all the rays obtained by refraction and reflection. I decided to implement a tree representing the cast rays. To do so I used an array of size $2**("height" + 1) - 1$. The nodes are stored in the array's cells. The sons'index of the node of index i are $2*i+1$ and $2*i+2$. For each node I stored a struct containing information about the ray. I first compute the direction and origin of each reflected and refracted ray from root to leaves and then compute the final color from leaves to root. 

== Concrete computations

=== Computing the initial ray

We have one initial ray for each fragment of the quad I display. This ray comes from the position of the screen and points towards the world position of the corresponding point of virtual screen in front of the camera.
The origin of the ray is always the camera position and is passed to the fragment shader as a uniform value.
I used the parameters of the camera (Fov, aspectRatio and up and right direction) to determine the direction of the ray of each fragment. Concretely, I compute the direction of the ray in the vertex shader for each corner of the quad with the following formula ( $(x, y)$ is the position of each vertex with $x$ and $y$ ranging from -1 to 1 ) : 



$ "rayDiection" = "forward" + (x*"halfWidth")."right" + (y*"halfHeight")."up" $

where

 $ "halfHeight" &= "tan"("Fov"/2) \
  "halfWidth" &= "aspectRation"*"halfHeight" $

The direction is then interpolated for each fragment.



=== Computing other rays

To compute the reflected ray, I used the coordinates of the point of intersection and the normal $n$ of the intersected triangle.

$ "reflectedRay" = 2*("dot"(-"ray_direction", n))*n + "ray_direction" $

For the refracted ray, I change the origin of the ray and keep the same direction. I assume I have the same propagation medium everywhere, and thus the same refraction index. Concretely, I assume all my transparent surfaces are like very thin glass surfaces. In any case, I only used reflection for my final scene, which is enough since in the case of a real sea, the depth of the water is large enough for the observer to only see an opaque blue surface.

The last ray that is created is the shadow ray. It is shot toward the light source and is used to detect whether the point of interection is in shadow.


=== Mixing color

To obtain the final color of a fragment, I mix the color obtained by the phong light model in the point of intersection and the colors computed recursively by shooting the reflected and refracted ray. As for the shadow ray, if it intersected a triangle, I divide the phong light model color by 2. 

If no triangle is intersected, I simply return the color of the sky whose calculation is explained in the next paragraph.

=== Background

To find the color of the sky, I simply compute the dot producti between the ray direction and the light direction and return the corresponding color in a color gradient ranging from yellow to blue.



#linebreak() 

```C
float sun_closeness = dot(rayDirection, normalize(lightDirection));
if(sun_closeness>sunThreshold){
		return yellow;
	}
	else if(sun_closeness>haloThreshold){
		float x = (sun_closeness-haloThreshold)/(sunThreshold-haloThreshold);
		return getGradient(orange, yellow, x);
	}
	else if(sun_closeness>pinkSkyThreshold){
		float x = (sun_closeness - pinkSkyThreshold)/(haloThreshold - pinkSkyThreshold);
		return getGradient(pink, orange, x);
	}
	else{
		float x = (sun_closeness - -1)/(pinkSkyThreshold - -1);
		return getGradient(blue, pink, x);
	}
 ```

 = Sea geometry and simulation

== Initial mesh

 For the sea, I took inspiration from the video game Monument Valley. 
I split a rectangle into triangles (a regular grid, each cell being split into 2 triangles). The height of each vertex is random in a certain range, which gives us a nice blocky sea as you can see in figure 1. 

#figure(
  image("screenshots\4\sunset_over_sea.png", width: 60%),
  caption: [
    grid of vertices with random height.
  ],
)

As you can see in the figure, the water looks pink while the mesh color is cyan. It is because of the reflection which represents 70% of the final mixed color.

== Simulation

Then I attached a vertical spring to each vertex. I had to put a different spring constant to each vertex, otherwise, the vertices would be at the height 0 at the same time, and then we would have an ugly flat sea for a moment and a brutal change of triangle colors when the triangles' orientation change at the same time.

$ arrow(F) = -k*y.arrow(u)_y $

where $k$ is the spring constant.

Unfortunately, with this animation, there is no coherence between the wave crests while in the sea you usually have huge waves in addition to small oscillation of the water. That's why I added a vertical offset to each vertex which is obtained with the plane wave formula :

$ "offset" = cos(w*t-arrow(k).arrow(x)) $ 

where $w$ is the pulsation and $arrow(k)$ is the wave vector

In our simulation, $arrow(x)$ is the position in the horizontal $(x, z)$ plane.


 = Changing the mesh to simulate infinite surface

 == Conventions 

For the next calculations in this document, I define the % operator for integers as described below :

$ a % b = c <=>  cases(
  a = k * b + c ,
  k in ZZ ,
  c in \[0 comma b\[
) $

Note that it is not the same definion than in C. In C, the result of % for a negative number will be a negative number, which is not the behavior expected for my formulas. That means I had to create a special function in my code to implement this operator.

== Structures used for vertices and triangle indices

In order to understand the formulas to update the mesh, you should remember how my vertices and triangles are stored in the memory.

=== Vertices

My vertices and triangle indices are both stored in arrays. The vertices are stored in 1D array which should be interpreted as a 2D array. The indices $(i, j)$  of each vertex represent its position in the initial grid. The acutal index to access the vertex in the array computed this way : 
$ "vertexIndex" = j + i*(N+1) $

where $N$ is the number of columns in the grid.

You will notice that $i$ and $j$ range from 0 to $N$ included and that the number of vertices is therefore $2^(N+1)$

=== Triangles

To render the sea, I need $2*N^2$ triangles. However, as I will explain later in this document, the vertices are going to move from one side of the grid to the other, and the set of triangles that should exists or not will change over time.
The way I will move the vertices (when i say "move", I mean changing the $(x,z)$ position, the vertex remains at the same index in the array), ensures that the only triangles I will have to render will be composed of neighboring vertex in the 2D array if we consider the $(i,j)$ indices modulo N. 
You can think of my array as a grid in a snake game where you can go from the left wall to right wall directly. Therefore I decide to associate each vertex to 2 triangles, even if the triangles should not be displayed at a time t.

 To simplify the access and to order the strucutre, 
I wanted to associate a square composed of 2 triangles to each vertex 
(you can consider that each vertex is the left corner of the square even if this association is done regardless to any space position). 
But there should be no triangles associated to the vertices that are in the last row or last column of my 2D array.
However, as I will explain later in this document, the vertices are going to move from one side of the grid to the other, and the set of triangles that should exists or not will change over time. 

