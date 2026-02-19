//to make a video : 
//  start simulation and press r
//  frame will be stored as images in the screenshots/videos folder
//  run the folowing commands : 
//    ffmpeg -framerate 30 -i videos/s%04d.tga -vf "pad=ceil(iw/2)*2:ceil(ih/2)*2" -c:v libx264 -pix_fmt yuv420p output.mp4
//    rm videos/*  

#define _USE_MATH_DEFINES


  // MACROS
//this macro tells us where to place the camera
//if it is not define, we'll see the whole sea square afar
#define _CLOSE_VIEW 
//#define _NIGHT



//#include <glad/gl.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <glm/gtx/rotate_vector.hpp>


#include <stdlib.h>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <vector>
#include <string>
#include <cmath>
#include <memory>

#include "scene.h"
#include "camera.h"


// Window parameters
GLFWwindow *g_window = nullptr;
int gWindowWidth = 1024;
int gWindowHeight = 768;
bool gRecordVideo = false;
int gSavedCnt = 0;
bool animationPaused = false;


// GPU objects
GLuint g_program = 0; // A GPU program contains at least a vertex shader and a fragment shader

// OpenGL identifiers
GLuint g_vao = 0;
GLuint g_posVbo = 0;
GLuint g_ibo = 0;

//storage buffer (this will store the scene data)
GLuint g_vertexSbo = 0; //storage buffer object
GLuint g_triangleSbo = 0;
GLuint g_objectPropertiesSbo = 0;

// All verteices packed in one array [x0, y0, z0, x1, y1, z1, ...]
std::vector<float> g_vertexPositions;
// All triangle indices packed in one array [v00, v01, v02, v10, v11, v12, ...] with vij the index of j-th vertex of the i-th triangle
std::vector<unsigned int> g_triangleIndices;

//key input 
bool key_right_pressed = false;
bool key_left_pressed = false;
bool key_up_pressed = false;
bool key_down_pressed = false;
bool key_shift_pressed = false;

// Other global variables
const float fps = 30;
const float move_angle_step = 1; //step to rotate the light
#ifdef _NIGHT
glm::vec3 lightDirection = glm::vec3(0.0, 0.0, -1.0);
#else 
glm::vec3 lightDirection = glm::vec3(0.0, 0.0, 1.0);
#endif

//Scene
Scene g_scene = Scene();

//Camera
Camera g_camera = Camera();



//GLFW FUNCTIONS

// Executed each time the window is resized. Adjust the aspect ratio and the rendering viewport to the current window.
void windowSizeCallback(GLFWwindow* window, int width, int height) {
  gWindowWidth = width;
  gWindowHeight = height;
  g_camera.setAspectRatio(static_cast<float>(width)/static_cast<float>(height));
  glViewport(0, 0, (GLint)width, (GLint)height); // Dimension of the rendering region in the window
}

// Executed each time a key is entered.
void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
  if(action == GLFW_PRESS && key == GLFW_KEY_W) {
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
  } else if(action == GLFW_PRESS && key == GLFW_KEY_F) {
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
  } else if(action == GLFW_PRESS && (key == GLFW_KEY_ESCAPE || key == GLFW_KEY_Q)) {
    glfwSetWindowShouldClose(window, true); // Closes the application if the escape key is pressed
  } else if(action == GLFW_PRESS && key == GLFW_KEY_R) {
    gRecordVideo = !gRecordVideo;
    std::cout<<"toggle video record\n"<<std::endl;
  } else if(action == GLFW_PRESS && key == GLFW_KEY_P) {
    animationPaused = !animationPaused;
  }

  else if(action == GLFW_PRESS && (key == GLFW_KEY_RIGHT)){
    key_right_pressed = true;
  } else if(action == GLFW_PRESS && (key == GLFW_KEY_LEFT)){
    key_left_pressed = true;
  } else if(action == GLFW_PRESS && (key == GLFW_KEY_UP)){
    key_up_pressed = true;
  } else if(action == GLFW_PRESS && (key == GLFW_KEY_DOWN)){
    key_down_pressed = true;
  } else if(action == GLFW_PRESS && (key == GLFW_KEY_RIGHT_SHIFT || key == GLFW_KEY_LEFT_SHIFT)){
    key_shift_pressed = true;
  } 

  else if(action == GLFW_RELEASE && (key == GLFW_KEY_RIGHT)){
    key_right_pressed = false;
  } else if(action == GLFW_RELEASE && (key == GLFW_KEY_LEFT)){
    key_left_pressed = false;
  } else if(action == GLFW_RELEASE && (key == GLFW_KEY_UP)){
    key_up_pressed = false;
  } else if(action == GLFW_RELEASE && (key == GLFW_KEY_DOWN)){
    key_down_pressed = false;
  } else if(action == GLFW_RELEASE && (key == GLFW_KEY_RIGHT_SHIFT || key == GLFW_KEY_LEFT_SHIFT)){
    key_shift_pressed = false;
  } 
}

void errorCallback(int error, const char *desc) {
  std::cout <<  "Error " << error << ": " << desc << std::endl;
}

void initGLFW() {
  glfwSetErrorCallback(errorCallback);

  // Initialize GLFW, the library responsible for window management
  if(!glfwInit()) {
    std::cerr << "ERROR: Failed to init GLFW" << std::endl;
    std::exit(EXIT_FAILURE);
  }

  // Before creating the window, set some option flags
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_RESIZABLE, GL_TRUE);

  // Create the window
  g_window = glfwCreateWindow(
    gWindowWidth, gWindowHeight,
    "IGR Project Alice Jeannin - Sunset over an infinite sea",
    nullptr, nullptr);
  if(!g_window) {
    std::cerr << "ERROR: Failed to open window" << std::endl;
    glfwTerminate();
    std::exit(EXIT_FAILURE);
  }

  // Load the OpenGL context in the GLFW window using GLAD OpenGL wrangler
  glfwMakeContextCurrent(g_window);
  glfwSetWindowSizeCallback(g_window, windowSizeCallback);
  glfwSetKeyCallback(g_window, keyCallback);
}



// OPENGL FUNCTIONS

void initOpenGL() {
  // Load extensions for modern OpenGL
  if(!gladLoadGL()) { //!gladLoadGL(glfwGetProcAddress)
    std::cerr << "ERROR: Failed to initialize OpenGL context" << std::endl;
    glfwTerminate();
    std::exit(EXIT_FAILURE);
  }

  glCullFace(GL_BACK); // Specifies the faces to cull (here the ones pointing away from the camera)
  glEnable(GL_CULL_FACE); // Enables face culling (based on the orientation defined by the CW/CCW enumeration).
  glDepthFunc(GL_LESS);   // Specify the depth test for the z-buffer
  glEnable(GL_DEPTH_TEST);      // Enable the z-buffer test in the rasterization
  glClearColor(0.7f, 0.7f, 0.7f, 1.0f); // specify the background color, used any time the framebuffer is cleared
}

// Loads the content of an ASCII file in a standard C++ string
std::string file2String(const std::string &filename) {
  std::ifstream t(filename.c_str());
  std::stringstream buffer;
  buffer << t.rdbuf();
  return buffer.str();
}

// Loads and compile a shader, before attaching it to a program
void loadShader(GLuint program, GLenum type, const std::string &shaderFilename) {
  GLuint shader = glCreateShader(type); // Create the shader, e.g., a vertex shader to be applied to every single vertex of a mesh
  std::string shaderSourceString = file2String(shaderFilename); // Loads the shader source from a file to a C++ string
  const GLchar *shaderSource = (const GLchar *)shaderSourceString.c_str(); // Interface the C++ string through a C pointer
  glShaderSource(shader, 1, &shaderSource, NULL); // load the vertex shader code
  glCompileShader(shader);
  GLint success;
  GLchar infoLog[512];
  glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
  if(!success) {
    glGetShaderInfoLog(shader, 512, NULL, infoLog);
    std::cout << "ERROR in compiling " << shaderFilename << "\n\t" << infoLog << std::endl;
  }
  glAttachShader(program, shader);
  glDeleteShader(shader);
}

void initGPUprogram() {
  g_program = glCreateProgram(); // Create a GPU program, i.e., two central shaders of the graphics pipeline
  loadShader(g_program, GL_VERTEX_SHADER, "../vertexShaderTriangle.glsl");
  loadShader(g_program, GL_FRAGMENT_SHADER, "../fragmentShaderTriangle.glsl");
  glLinkProgram(g_program); // The main GPU program is ready to be handle streams of polygons
  glUseProgram(g_program);
}



// INIT CAMERA AND SCENE

void initCamera() {
  int width, height;
  glfwGetWindowSize(g_window, &width, &height);
  #ifdef _CLOSE_VIEW
  g_camera.init(glm::vec3(0.0, 1.0, -3.0), glm::vec3(0.0, 0.5, 0.0), glm::vec3(0.0, 1.0, 0.0)); 
  #else
  g_camera.init(glm::vec3(0.0, 5.5, -10.5), glm::vec3(0.0, 0.5, 0.0), glm::vec3(0.0, 1.0, 0.0)); 
  #endif
  g_camera.setAspectRatio(static_cast<float>(width)/static_cast<float>(height));
}

void initScene(){
  g_scene.init(g_camera.getCenter());
}



// INIT GPU MEMORY

void initCPUgeometry() {
  //2 triangles for the screen
  g_vertexPositions = { // the array of vertex Colors [x0, y0, z0, x1, y1, z1, ...]
    -1.f, -1.f, 0.f,
    -1.f, 1.f, 0.f,
    1.f, -1.f, 0.f,
    1.f, 1.f, 0.f
  };
  g_triangleIndices = { 0, 2, 1, 3, 1, 2};
}

void initGPUgeometry() {
  // Create a single handle, vertex array object that contains attributes,
  // vertex buffer objects (e.g., vertex's Color, normal, and color)
#ifdef _MY_OPENGL_IS_33_
  glGenVertexArrays(1, &g_vao); // If your system doesn't support OpenGL 4.5, you should use this instead of glCreateVertexArrays.
#else
  glCreateVertexArrays(1, &g_vao);
#endif

  glBindVertexArray(g_vao);

  // Generate a GPU buffer to store the Colors of the vertices
  size_t vertexBufferSize = sizeof(float)*g_vertexPositions.size(); // Gather the size of the buffer from the CPU-side vector
#ifdef _MY_OPENGL_IS_33_
  glGenBuffers(1, &g_posVbo);
  glBindBuffer(GL_ARRAY_BUFFER, g_posVbo);
  glBufferData(GL_ARRAY_BUFFER, vertexBufferSize, g_vertexPositions.data(), GL_DYNAMIC_READ);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3*sizeof(GLfloat), 0);
  glEnableVertexAttribArray(0);
#else
  glCreateBuffers(1, &g_posVbo);
  glBindBuffer(GL_ARRAY_BUFFER, g_posVbo);
  glNamedBufferStorage(g_posVbo, vertexBufferSize, g_vertexPositions.data(), GL_DYNAMIC_STORAGE_BIT); // Create a data storage on the GPU and fill it from a CPU array
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3*sizeof(GLfloat), 0); //index, (vertex) size, type, normalized, stride, (offset) pointer
  glEnableVertexAttribArray(0); //layout
#endif


  // Same for an index buffer object that stores the list of indices of the
  // triangles forming the mesh
  size_t indexBufferSize = sizeof(unsigned int)*g_triangleIndices.size();
#ifdef _MY_OPENGL_IS_33_
  glGenBuffers(1, &g_ibo);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_ibo);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexBufferSize, g_triangleIndices.data(), GL_DYNAMIC_READ);
  
#else
  glCreateBuffers(1, &g_ibo);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_ibo);
  glNamedBufferStorage(g_ibo, indexBufferSize, g_triangleIndices.data(), GL_DYNAMIC_STORAGE_BIT);
#endif

  glBindVertexArray(0); // deactivate the VAO for now, will be activated again when rendering
}

void getDataFromVec3Vector(std::vector<glm::vec3>& vin, std::vector<float>& vout){
  //transform the vector by adding 1 float between each 
  //vec3 in order to fit with glsl alignment
  int n = vin.size();
  vout.clear();
  for(int i=0; i<n; i++){
    vout.push_back(vin[i][0]);
    vout.push_back(vin[i][1]);
    vout.push_back(vin[i][2]);
    vout.push_back(0.f);
  }
}

void getDataFromUvec3Vector(std::vector<glm::uvec3>& vin, std::vector<uint>& vout){
  //transform the vector by adding 1 uint between each 
  //vec3 in order to fit with glsl alignment
  int n = vin.size();
  vout.clear();
  for(int i=0; i<n; i++){
    vout.push_back(vin[i][0]);
    vout.push_back(vin[i][1]);
    vout.push_back(vin[i][2]);
    vout.push_back(0);
  }
}

void initGPUstorageBuffer(){
  //create buffer

  size_t bufferSize;
  std::vector<float> bufferData;
  std::vector<uint> uintBufferData;

  //vertices
  getDataFromVec3Vector(g_scene.vertexPositions, bufferData);
  bufferSize = sizeof(float)*bufferData.size();
  glCreateBuffers(1, &g_vertexSbo);
  glBindBuffer(GL_SHADER_STORAGE_BUFFER, g_vertexSbo);
  glBufferData(
            GL_SHADER_STORAGE_BUFFER, bufferSize, 
            bufferData.data(), GL_DYNAMIC_READ);
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, g_vertexSbo);

  //objectProperties
  bufferSize = sizeof(ObjectProperties)*g_scene.objectProperties.size();
  glCreateBuffers(1, &g_objectPropertiesSbo);
  glBindBuffer(GL_SHADER_STORAGE_BUFFER, g_objectPropertiesSbo);
  glBufferData(
            GL_SHADER_STORAGE_BUFFER, bufferSize, 
            g_scene.objectProperties.data(), GL_DYNAMIC_READ);
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, g_objectPropertiesSbo);

  //triangles
  getDataFromUvec3Vector(g_scene.triangleIndices, uintBufferData);
  bufferSize = sizeof(uint)*uintBufferData.size();
  glCreateBuffers(1, &g_triangleSbo);
  glBindBuffer(GL_SHADER_STORAGE_BUFFER, g_triangleSbo);
  glBufferData(
            GL_SHADER_STORAGE_BUFFER, bufferSize, 
            uintBufferData.data(), GL_DYNAMIC_READ);
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, g_triangleSbo);

}

void initGPU(){
  initGPUgeometry();
  initGPUstorageBuffer();
  initGPUprogram();
}



// INIT ALL

void init() {
  initGLFW();
  initOpenGL();
  initCamera();
  initScene();
  initCPUgeometry();
  initGPU();
}

void clear() {
  glDeleteProgram(g_program);
  glfwDestroyWindow(g_window);
  glfwTerminate();
}



// LIGHT

void rotate_light_left(float delta){
  //rotate light to left
  glm::vec3 up = glm::vec3(0.0, 1.0, 0.0);
  lightDirection = glm::rotate(lightDirection, delta*move_angle_step, up);
}

void rotate_light_right(float delta){
  //rotate light to right
  glm::vec3 up = glm::vec3(0.0, 1.0, 0.0);
  lightDirection = glm::rotate(lightDirection, -delta*move_angle_step, up);
}



// RENDER

void savePicture(){
  std::stringstream fpath;
  fpath <<"../../screenshots/videos/"<< "s" << std::setw(4) << std::setfill('0') << gSavedCnt++ << ".tga";
  const short int w = gWindowWidth;
  const short int h = gWindowHeight;
  std::vector<int> buf(w*h*3, 0);
  glReadPixels(0, 0, w, h, GL_BGR, GL_UNSIGNED_BYTE, &(buf[0]));

  FILE *out = fopen(fpath.str().c_str(), "wb");
  short TGAhead[] = {0, 2, 0, 0, 0, 0, w, h, 24};
  fwrite(&TGAhead, sizeof(TGAhead), 1, out);
  fwrite(&(buf[0]), 3*w*h, 1, out);
  fclose(out);
}

void render() {
  //clear
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // Erase the color and z buffers.


  //update vertex buffer
  size_t bufferSize;
  std::vector<float> bufferData;
  getDataFromVec3Vector(g_scene.vertexPositions, bufferData);
  bufferSize = sizeof(float)*bufferData.size();
  glBindBuffer(GL_SHADER_STORAGE_BUFFER, g_vertexSbo);
  glBufferData(
            GL_SHADER_STORAGE_BUFFER, bufferSize, 
            bufferData.data(), GL_DYNAMIC_READ);
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, g_vertexSbo);

  //update triangle buffer
  std::vector<uint> uintBufferData;
  getDataFromUvec3Vector(g_scene.triangleIndices, uintBufferData);
  bufferSize = sizeof(uint)*uintBufferData.size();
  glBindBuffer(GL_SHADER_STORAGE_BUFFER, g_triangleSbo);
  glBufferData(
            GL_SHADER_STORAGE_BUFFER, bufferSize, 
            uintBufferData.data(), GL_DYNAMIC_READ);
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, g_triangleSbo);


  //Scene informations
  glUniform1i(glGetUniformLocation(g_program, "n_triangles"), g_scene.triangleIndices.size());
  glUniform1i(glGetUniformLocation(g_program, "n_objects"), g_scene.objectProperties.size());
  #ifdef _NIGHT
  glUniform3fv(glGetUniformLocation(g_program, "lightDirection"),1,  glm::value_ptr(glm::normalize(lightDirection)));
  #else 
  glUniform3fv(glGetUniformLocation(g_program, "lightDirection"),1,  glm::value_ptr(glm::normalize(lightDirection)));
  #endif
  
  //camera informations
  float half_height = tan(glm::radians(g_camera.getFov())/2.0f);
  float half_width = half_height*g_camera.getAspectRatio();
  glUniform3fv(glGetUniformLocation(g_program, "camera_position"), 1, glm::value_ptr(g_camera.getPosition()));
  glUniform3fv(glGetUniformLocation(g_program, "forward"), 1, glm::value_ptr(g_camera.getForward()));
  glUniform3fv(glGetUniformLocation(g_program, "up"), 1, glm::value_ptr(g_camera.getUp()));
  glUniform3fv(glGetUniformLocation(g_program, "right"), 1, glm::value_ptr(g_camera.getRight()));
  glUniform1f(glGetUniformLocation(g_program, "half_height"), half_height);
  glUniform1f(glGetUniformLocation(g_program, "half_width"), half_width);

  //call GPU
  glBindVertexArray(g_vao);     // activate the VAO storing geometry data
  glDrawElements(GL_TRIANGLES, g_triangleIndices.size(), GL_UNSIGNED_INT, 0); // Call for rendering: stream the current GPU geometry through the current GPU program


  //save the frame
  if(gRecordVideo) {
    savePicture();//save the current frame in the disk
  }
}



// UPDATE

void update(const float delta) {

  //SCENE
  if(!animationPaused){
    float dt = 1/fps; //fix dt to avoid strange physics bug if the computer is to slow and dt is too large
    g_scene.update(dt, g_camera.getCenter());
  }


  //LIGHT
  if(key_shift_pressed){
    if(key_right_pressed){
      rotate_light_right(delta);
    }
    else if (key_left_pressed){
      rotate_light_left(delta);
    }
  }


  //CAMERA
  if(key_shift_pressed){
    if(key_up_pressed){
      g_camera.move_up(delta);
    }
    else if(key_down_pressed){
      g_camera.move_down(delta);
    }
  }
  else{
    if(key_right_pressed){
      g_camera.move_right(delta);
    }
    else if(key_left_pressed){
      g_camera.move_left(delta);
    }
    else if(key_up_pressed){
      g_camera.move_forward(delta);
    }
    else if(key_down_pressed){
      g_camera.move_backward(delta);
    }
  }
}



// MAIN

int main(int argc, char ** argv) {
  srand((unsigned int)time(0));
  init(); // Your initialization code (user interface, OpenGL states, scene with geometry, material, lights, etc)

  printf("number of triangles : %ld\n", g_scene.triangleIndices.size());
  float currentTime;
  float lastCurrentTime=static_cast<float>(glfwGetTime());
  while(!glfwWindowShouldClose(g_window)) {
    currentTime = static_cast<float>(glfwGetTime());
    update(currentTime-lastCurrentTime);
    lastCurrentTime = currentTime;
    render();
    glfwSwapBuffers(g_window);
    glfwPollEvents();
  }
  clear();
  return EXIT_SUCCESS;
}
