# IGR PROJECT - ALICE JEANNIN



## CONTENTS

The code to build the project is in the src folder. Little information about the available functionalities can be found in this document below. You will find also a report (made with typst) in PDF format, a video presenting the project and the the slides I used for my presentation (in PDF and Power Point format). Finally, you'll find some pictures and videos of the project in the screenshot folder.

You can share my project presentation with the other students.


## FUNCTIONALITIES OF THE PROJECT

### To run the project

in the src folder :
```console
cmake -B build 
```  

in the build folder created in the src folder :

```console
make
./tpOpengl
``` 


You should see a sea on a pink sky.

### Commands

Here are the different commands:
- Arrows : move the camera
- Shift + up/down arrows : rotate the camera to look up or down
- Shift + left/right arrows : move the sun 
- R : start/stop saving picture in order to make a video (details are provided at the beginning of main.cpp file)
- P : pause or unpause the animation
- ESC or Q : quit

### Macros


You can change macros at the beginning of some files to modify some properties :

```main.cpp```
* _CLOSE_VIEW : this macro tells us where to place the camera. If it is not define, we'll see the whole sea square afar
* _NIGHT : if it is defined, it will set the sun behind you when the program starts.


```scene.h```
* _HIGH_RESOLUTION : if it is defined, there will be more triangles. The animation should be slower.


```fragmentShaderTriangle.glsl```
* RECURSION_DEPTH : defines the number of recursion performed by the ray tracer. By default, it is set to 3. 
