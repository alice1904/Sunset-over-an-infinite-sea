// ----------------------------------------------------------------------------
// main.cpp
//
//  Created on: 24 Jul 2020
//      Author: Kiwon Um
//        Mail: kiwon.um@telecom-paris.fr
//
// Description: IGR201 Practical; OpenGL and Shaders (DO NOT distribute!)
//
// Copyright 2020-2025 Kiwon Um
//
// The copyright to the computer program(s) herein is the property of Kiwon Um,
// Telecom Paris, France. The program(s) may be used and/or copied only with
// the written permission of Kiwon Um or in accordance with the terms and
// conditions stipulated in the agreement/contract under which the program(s)
// have been supplied.
// ----------------------------------------------------------------------------

//to make video : 
//  start simu and press r
//  frame will be stored as images in the screenshots/videos folder
//  run the folowing commands : 
//    ffmpeg -framerate 30 -i videos/s%04d.tga -vf "pad=ceil(iw/2)*2:ceil(ih/2)*2" -c:v libx264 -pix_fmt yuv420p output.mp4
//    rm videos/*  

#define _USE_MATH_DEFINES

//this macro tells us where to place the camera
//if it is not define, we'll see the whole sea square afar
#define _CLOSE_VIEW 
//this macro tells us where the sun is (in front of you or in your back)
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

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "scene.h"

// constants
// const static float kSizeSun = 1;
// const static float kSizeEarth = 0.5;
// const static float kSizeMoon = 0.25;
// const static float kRadOrbitEarth = 10;
// const static float kRadOrbitMoon = 2;

// Window parameters
GLFWwindow *g_window = nullptr;
int gWindowWidth = 1024;
int gWindowHeight = 768;
bool gRecordVideo = false;
int gSavedCnt = 0;
bool animationPaused = false;
const float fps = 30;

// GPU objects
GLuint g_program = 0; // A GPU program contains at least a vertex shader and a fragment shader

// OpenGL identifiers
GLuint g_vao = 0;
GLuint g_posVbo = 0;
GLuint g_ibo = 0;

//storage buffer (this will store the scene data)
GLuint g_vertexSbo = 0; //storage buffer object
GLuint g_vertexNormalsSbo = 0;
GLuint g_triangleSbo = 0;
GLuint g_viewMatricesSbo = 0;

// All vertex Colors packed in one array [x0, y0, z0, x1, y1, z1, ...]
std::vector<float> g_vertexPositions;
std::vector<float> g_vertexColors;
// All triangle indices packed in one array [v00, v01, v02, v10, v11, v12, ...] with vij the index of j-th vertex of the i-th triangle
std::vector<unsigned int> g_triangleIndices;

//key input 
bool key_right_pressed = false;
bool key_left_pressed = false;
bool key_up_pressed = false;
bool key_down_pressed = false;
bool key_shift_pressed = false;

//Scene
Scene scene = Scene();



class Camera {
public:

  //TODO : impélemente key event to move the camera!

  void init(const glm::vec3 &pos, const glm::vec3 &center, const glm::vec3 &up){
    if(glm::dot(up, up)==0 || glm::dot(center-pos, center-pos)==0){
      std::cerr << "ERROR: Failed to init Camera due to not allowed arguments" << std::endl;
      glfwTerminate();
      std::exit(EXIT_FAILURE);
    }
    
    m_pos = pos;
    m_center = center;

    m_forward = normalize(center - pos);
    m_right = normalize(cross(m_forward, up));
    m_up = normalize(cross(m_right, m_forward));
  }

  inline float getFov() const { return m_fov; }
  inline void setFoV(const float f) { m_fov = f; }
  inline float getAspectRatio() const { return m_aspectRatio; }
  inline void setAspectRatio(const float a) { m_aspectRatio = a; }
  inline float getNear() const { return m_near; }
  inline void setNear(const float n) { m_near = n; }
  inline float getFar() const { return m_far; }
  inline void setFar(const float n) { m_far = n; }
  inline glm::vec3 getPosition() { return m_pos; }
  inline glm::vec3 getCenter() { return m_center; }
  inline glm::vec3 getForward() {return m_forward;}
  inline glm::vec3 getRight() {return m_right;}
  inline glm::vec3 getUp() {return m_up;}

  inline glm::mat4 computeViewMatrix() const {
    return glm::lookAt(m_pos, m_center, m_up);
  }

  // Returns the projection matrix stemming from the camera intrinsic parameter.
  inline glm::mat4 computeProjectionMatrix() const {
    return glm::perspective(glm::radians(m_fov), m_aspectRatio, m_near, m_far);
  }


  //move the camera

  void move_right(float delta){
    //change the position
    //m_pos = m_center + glm::rotate(m_pos-m_center, delta*move_angle_step, m_up); //rotate m_pos around up axis
    
    glm::vec3 right = glm::vec3(-1.0, 0.0, 0.0);
    m_pos =  m_pos + delta*move_step*right;
    m_center = m_center + delta*move_step*right;
    //update the directions
    updateForward();
    updateRightFromUp();
  }

  void move_left(float delta){
    //change the position
    //m_pos = m_center + glm::rotate(m_pos-m_center, -delta*move_angle_step, m_up); //rotate m_pos around up axis
    
    glm::vec3 right = glm::vec3(-1.0, 0.0, 0.0);
    m_pos =  m_pos - delta*move_step*right;
    m_center = m_center - delta*move_step*right;
    //update the directions
    updateForward();
    updateRightFromUp();
  }

  void move_up(float delta){
    //change the position
    m_pos = m_center + glm::rotate(m_pos-m_center, -delta*move_angle_step, m_right); //rotate m_pos around up axis
    //update the directions
    updateForward();
    updateUpFromRight();
  }

  void move_down(float delta){
    //change the position
    m_pos = m_center + glm::rotate(m_pos-m_center, delta*move_angle_step, m_right); //rotate m_pos around up axis
    //update the directions
    updateForward();
    updateUpFromRight();
  }

  void move_forward(float delta){
    //glm::vec3 pos = m_pos + delta*move_step*m_forward;
    //if pos is too close to the center or is on the other side of the center, 
    //we don't update m_pos
    // if(glm::dot(m_center-pos, m_forward) >= min_distance_to_center){
    //   m_pos = pos;
    // }
    glm::vec3 forward = glm::vec3(0.0, 0.0, 1.0);
    m_pos =  m_pos + delta*move_step*forward;
    m_center =  m_center + delta*move_step*forward;
  }

  void move_backward(float delta){
    glm::vec3 forward = glm::vec3(0.0, 0.0, 1.0);
    m_pos = m_pos - delta*move_step*forward;
    m_center =  m_center - delta*move_step*forward;
  }

  void rotate_right(float delta){
    //change the direction
    //we use (0, 1, 0) instead of up to look down
    printf("%f, %f, %f, \n", m_forward.x, m_forward.y, m_forward.z);
    m_forward = glm::rotate(m_forward, -delta*move_angle_step, glm::vec3(0, 1, 0));
    m_center = m_pos + m_forward;
    updateRightFromUp();
  }

  void rotate_left(float delta){
    //change the direction
    m_forward = glm::rotate(m_forward, delta*move_angle_step, glm::vec3(0, 1, 0));
    m_center = m_pos + m_forward;
    updateRightFromUp();
  }


private:
  const float move_angle_step = 1;
  const float move_step = 3;
  const float min_distance_to_center = 1;

  glm::vec3 m_pos = glm::vec3(0, 0, -1);
  glm::vec3 m_center = glm::vec3(0, 0, 0);
  glm::vec3 m_up = glm::vec3(0, 1, 0);
  glm::vec3 m_right = glm::vec3(1, 0, 0);
  glm::vec3 m_forward = glm::vec3(0, 0, 1);


  inline void setPosition(const glm::vec3 &p) { m_pos = p; }

  inline void updateForward(){
    //update the forward vec3
    m_forward = glm::normalize(m_center-m_pos);
  }

  inline void updateRightFromUp(){
    //update the right vec3 from forward and up vec3
    m_right = glm::normalize(glm::cross(m_forward, m_up));
  }

  inline void updateUpFromRight(){
    //update the up vec3 from forward and right vec3
    m_up = glm::normalize(glm::cross(m_right, m_forward));
  }


  float m_fov = 45.f;        // Field of view, in degrees
  float m_aspectRatio = 1.f; // Ratio between the width and the height of the image
  float m_near = 0.1f; // Distance before which geometry is excluded from the rasterization process
  float m_far = 10.f; // Distance after which the geometry is excluded from the rasterization process
};
Camera g_camera;




GLuint loadTextureFromFileToGPU(const std::string &filename) {
  // Loading the image in CPU memory using stb_image
  int width, height, numComponents;
  unsigned char *data = stbi_load(filename.c_str(), &width, &height, &numComponents, 0);
  GLuint texID; // OpenGL texture identifier
  glGenTextures(1, &texID); // generate an OpenGL texture container
  glBindTexture(GL_TEXTURE_2D, texID); // activate the texture
  // Setup the texture filtering option and repeat mode; check www.opengl.org for details.
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  // Fill the GPU texture with the data stored in the CPU image
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
  // Free useless CPU memory
  stbi_image_free(data);
  glBindTexture(GL_TEXTURE_2D, 0); // unbind the texture
  return texID;
}

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
    scene.printHelp(g_camera.getCenter());
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
    "IGR Project Alice Jeannin - Sunset over sea",
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


void initCamera() {
  int width, height;
  glfwGetWindowSize(g_window, &width, &height);//0.5

  #ifdef _CLOSE_VIEW
  g_camera.init(glm::vec3(0.0, 1.0, -3.0), glm::vec3(0.0, 0.5, 0.0), glm::vec3(0.0, 1.0, 0.0)); 
  #else
  g_camera.init(glm::vec3(0.0, 5.5, -10.5), glm::vec3(0.0, 0.5, 0.0), glm::vec3(0.0, 1.0, 0.0)); 
  #endif
  
  g_camera.setAspectRatio(static_cast<float>(width)/static_cast<float>(height));
  g_camera.setNear(0.1);
  g_camera.setFar(80.1);
}

void initScene(){
  scene.init(g_camera.getCenter());
}

// Define your mesh(es) in the CPU memory
void initCPUgeometry() {
  // TODO: add vertices and indices for your mesh(es)
  g_vertexPositions = { // the array of vertex Colors [x0, y0, z0, x1, y1, z1, ...]
    -1.f, -1.f, 0.f,
    -1.f, 1.f, 0.f,
    1.f, -1.f, 0.f,
    1.f, 1.f, 0.f
  };
  g_vertexColors = { // the array of vertex Colors [x0, y0, z0, x1, y1, z1, ...]
    1.f, 0.f, 0.f,
    0.f, 0.f, 1.f,
    0.f, 1.f, 0.f,
    1.f, 0.f, 0.f
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

  getDataFromVec3Vector(scene.vertexPositions, bufferData);
  bufferSize = sizeof(float)*bufferData.size();
  glCreateBuffers(1, &g_vertexSbo);
  glBindBuffer(GL_SHADER_STORAGE_BUFFER, g_vertexSbo);
  // glBufferStorage(
  //           GL_SHADER_STORAGE_BUFFER, bufferSize, 
  //           scene.vertexPositions.data(), GL_DYNAMIC_STORAGE_BIT);
  glBufferData(
            GL_SHADER_STORAGE_BUFFER, bufferSize, 
            bufferData.data(), GL_DYNAMIC_READ);
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, g_vertexSbo);


  



  
  bufferSize = sizeof(ObjectProperties)*scene.objectProperties.size();
  glCreateBuffers(1, &g_viewMatricesSbo);
  glBindBuffer(GL_SHADER_STORAGE_BUFFER, g_viewMatricesSbo);
  glBufferData(
            GL_SHADER_STORAGE_BUFFER, bufferSize, 
            scene.objectProperties.data(), GL_DYNAMIC_READ);
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, g_viewMatricesSbo);



  getDataFromUvec3Vector(scene.triangleIndices, uintBufferData);
  bufferSize = sizeof(uint)*uintBufferData.size();
  glCreateBuffers(1, &g_triangleSbo);
  glBindBuffer(GL_SHADER_STORAGE_BUFFER, g_triangleSbo);
  // glBufferStorage(
  //           GL_SHADER_STORAGE_BUFFER, bufferSize, 
  //           scene.triangleIndices.data(), GL_DYNAMIC_STORAGE_BIT);
  glBufferData(
            GL_SHADER_STORAGE_BUFFER, bufferSize, 
            uintBufferData.data(), GL_DYNAMIC_READ);
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, g_triangleSbo);

  // getDataFromVec3Vector(scene.vertexNormals, bufferData);
  // bufferSize = sizeof(float)*bufferData.size();
  // std::cout<<"buffer size : "<<bufferSize<<std::endl;
  // glCreateBuffers(1, &g_vertexNormalsSbo);
  // glBindBuffer(GL_SHADER_STORAGE_BUFFER, g_vertexNormalsSbo);
  // // glBufferStorage(
  // //           GL_SHADER_STORAGE_BUFFER, bufferSize, 
  // //           scene.vertexPositions.data(), GL_DYNAMIC_STORAGE_BIT);
  // glBufferData(
  //           GL_SHADER_STORAGE_BUFFER, bufferSize, 
  //           bufferData.data(), GL_DYNAMIC_READ);
  // glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, g_vertexNormalsSbo);


}

void initGPU(){
  initGPUgeometry();
  initGPUstorageBuffer();
  initGPUprogram();
}


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

void savePicture(){
  std::stringstream fpath;
  fpath <<"../../screenshots/videos/"<< "s" << std::setw(4) << std::setfill('0') << gSavedCnt++ << ".tga";

  //std::cout << "Saving file " << fpath.str() << " ... " << std::flush;
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

// The main rendering call
void render() {
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // Erase the color and z buffers.

  //update vertex buffer
  size_t bufferSize;
  std::vector<float> bufferData;
  getDataFromVec3Vector(scene.vertexPositions, bufferData);
  bufferSize = sizeof(float)*bufferData.size();
  glBindBuffer(GL_SHADER_STORAGE_BUFFER, g_vertexSbo);
  glBufferData(
            GL_SHADER_STORAGE_BUFFER, bufferSize, 
            bufferData.data(), GL_DYNAMIC_READ);
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, g_vertexSbo);

  //update triangle buffer
  std::vector<uint> uintBufferData;
  getDataFromUvec3Vector(scene.triangleIndices, uintBufferData);
  bufferSize = sizeof(uint)*uintBufferData.size();
  glBindBuffer(GL_SHADER_STORAGE_BUFFER, g_triangleSbo);
  // glBufferStorage(
  //           GL_SHADER_STORAGE_BUFFER, bufferSize, 
  //           scene.triangleIndices.data(), GL_DYNAMIC_STORAGE_BIT);
  glBufferData(
            GL_SHADER_STORAGE_BUFFER, bufferSize, 
            uintBufferData.data(), GL_DYNAMIC_READ);
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, g_triangleSbo);

  //Camera informations
  float half_height = tan(glm::radians(g_camera.getFov())/2.0f);
  float half_width = half_height*g_camera.getAspectRatio();

  //scene
  glUniform1i(glGetUniformLocation(g_program, "n_triangles"), scene.triangleIndices.size());
  glUniform1i(glGetUniformLocation(g_program, "n_objects"), scene.objectProperties.size());
  #ifdef _NIGHT
  glUniform3fv(glGetUniformLocation(g_program, "lightDirection"),1,  glm::value_ptr(glm::normalize(glm::vec3(0.0, 0.0, -1.0))));
  #else 
  glUniform3fv(glGetUniformLocation(g_program, "lightDirection"),1,  glm::value_ptr(glm::normalize(glm::vec3(0.0, 0.0, -1.0))));//0.9, 0.0, -1.0

  #endif
  
  //camera
  glUniform3fv(glGetUniformLocation(g_program, "camera_position"), 1, glm::value_ptr(g_camera.getPosition()));
  glUniform3fv(glGetUniformLocation(g_program, "forward"), 1, glm::value_ptr(g_camera.getForward()));
  glUniform3fv(glGetUniformLocation(g_program, "up"), 1, glm::value_ptr(g_camera.getUp()));
  glUniform3fv(glGetUniformLocation(g_program, "right"), 1, glm::value_ptr(g_camera.getRight()));
  glUniform1f(glGetUniformLocation(g_program, "half_height"), half_height);
  glUniform1f(glGetUniformLocation(g_program, "half_width"), half_width);


  glBindVertexArray(g_vao);     // activate the VAO storing geometry data
  glDrawElements(GL_TRIANGLES, g_triangleIndices.size(), GL_UNSIGNED_INT, 0); // Call for rendering: stream the current GPU geometry through the current GPU program


  if(gRecordVideo) {
    savePicture();//save the current frame in the disk
    
  }


}

// Update any accessible variable based on the current time
void update(const float delta) {

  //SCENE
  if(!animationPaused){
    float dt = 1/fps; //fix dt to avoid strange physics
    scene.update(dt, g_camera.getCenter());
  }

  //CAMERA
  
  if(key_shift_pressed){
    if(key_right_pressed){
      //g_camera.rotate_right(delta);
    }
    else if(key_left_pressed){
      //g_camera.rotate_left(delta);
    }
    else if(key_up_pressed){
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

int main(int argc, char ** argv) {
  srand((unsigned int)time(0));
  init(); // Your initialization code (user interface, OpenGL states, scene with geometry, material, lights, etc)

  printf("number of triangles : %ld\n", scene.triangleIndices.size());
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
