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

#define _USE_MATH_DEFINES

#include <glad/gl.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtx/rotate_vector.hpp>

#include <cstdlib>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <cmath>
#include <memory>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"


// constants
const static float planetSize = 0.2;
const static float radSize = 0.1;
const static float kSizeSun = 1;
const static float kSizeEarth = 0.5;
const static float kSizeMoon = 0.25;
const static float kRadOrbitEarth = 10;
const static float kRadOrbitMoon = 2;

const static float orbitPeriodEarth = 10;
const static float orbitPeriodMoon = 5;

const static float rotationPeriodEarth = 5;


// Window parameters
GLFWwindow *g_window = nullptr;



// GPU objects
GLuint g_program = 0; // A GPU program contains at least a vertex shader and a fragment shader

//user parameters
bool animationRunning = true;  
float virtualTimeOrigin = 0.0; //date in the animation at the begining
float realTimeOrigin = 0.0;    //real date corresponding to the virtualTimeOrigin
float currentVirtualTime = 0.0;
float currentRealTime = 0.0;

//key input 
bool key_right_pressed = false;
bool key_left_pressed = false;
bool key_up_pressed = false;
bool key_down_pressed = false;
bool key_shift_pressed = false;







//***************** CAMERA *******************


class Camera {
public:

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
    m_pos = m_center + glm::rotate(m_pos-m_center, delta*move_angle_step, m_up); //rotate m_pos around up axis
    //update the directions
    updateForward();
    updateRightFromUp();
  }

  void move_left(float delta){
    //change the position
    m_pos = m_center + glm::rotate(m_pos-m_center, -delta*move_angle_step, m_up); //rotate m_pos around up axis
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
    glm::vec3 pos = m_pos + delta*move_step*m_forward;
    //if pos is too close to the center or is on the other side of the center, 
    //we don't update m_pos
    if(glm::dot(m_center-pos, m_forward) >= min_distance_to_center){
      m_pos = pos;
    }
  }

  void move_backward(float delta){
    m_pos = m_pos - delta*move_step*m_forward;
  }

  void rotate_right(float delta){
    //change the direction
    m_up = glm::rotate(m_up, -delta*move_angle_step, m_forward);
    updateRightFromUp();
  }

  void rotate_left(float delta){
    //change the direction
    m_up = glm::rotate(m_up, delta*move_angle_step, m_forward);
    updateRightFromUp();
  }


private:
  const float move_angle_step = 1;
  const float move_step = 10;
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





//***************** LOAD TEXTURE *******************


GLuint loadTextureFromFileToGPU(const std::string &filename) {
  int width, height, numComponents;
  // Loading the image in CPU memory using stb_image
  unsigned char *data = stbi_load(
    filename.c_str(),
    &width, &height,
    &numComponents, // 1 for a 8 bit grey-scale image, 3 for 24bits RGB image, 4 for 32bits RGBA image
    0);

  GLuint texID;
  // TODO: create a texture and upload the image data in GPU memory using
  // glGenTextures, glBindTexture, glTexParameteri, and glTexImage2D
  glGenTextures(1, &texID);  //generate texture
  glBindTexture(GL_TEXTURE_2D, texID); //activate texture
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); //set parameters...
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  //fill gpu texture with our data
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);

  // Free useless CPU memory
  stbi_image_free(data);
  glBindTexture(GL_TEXTURE_2D, 0); // unbind the texture

  return texID;
}






//***************** MESH *******************


class Mesh {
public:
  Mesh(const size_t resolution=16){
    // init the vertex positions, the normals and the texcoords

    if( resolution<=0){
      std::cerr << "ERROR: Failed to init Mesh because of negative resolution" << std::endl;
      std::exit(EXIT_FAILURE);
    }
    float theta = 0.f;
    float phi = 0.f;
    float theta_step = glm::pi<float>()/resolution; 
    float phi_step = glm::pi<float>()/resolution;
    
      //creating vertices
    for(int i = 0; i<resolution+1; i++){
      phi = i*phi_step;
      for(int j=0; j<2*resolution+1; j++){
        theta = j*theta_step;

        m_vertexPositions.push_back(sin(phi)*sin(theta));
        m_vertexPositions.push_back(cos(phi));
        m_vertexPositions.push_back(sin(phi)*cos(theta));

        //since the sphere is centered in (0,0,0) and its radius is one, 
        //the normal is equal to the position of the vertex
        m_vertexNormals.push_back(sin(phi)*sin(theta));
        m_vertexNormals.push_back(cos(phi));
        m_vertexNormals.push_back(sin(phi)*cos(theta));


        //the texCoord are givent by phi and theta
        m_vertexTexCoords.push_back(((float)j)/(2.0f*resolution));
        m_vertexTexCoords.push_back(((float)i)/(resolution));
        
        
      }
    }

    int stride = 2*resolution+1; //nbr of vertices between 2 different value of phi
      //creating triangles
    for(int i = 0; i<resolution; i++){
      for(int j = 0; j<2*resolution; j++){
        m_triangleIndices.push_back(i*stride + j);
        m_triangleIndices.push_back((i+1)*stride + j ); //phi +theta_step
        m_triangleIndices.push_back((i+1)*stride + (j+1) ); //phi+theta_step, theta+theta_step

        m_triangleIndices.push_back(i*stride + j);
        m_triangleIndices.push_back((i+1)*stride + (j+1) ); //phi+theta_step, theta+theta_step
        m_triangleIndices.push_back(i*stride + (j+1) ); //theta+theta_step

      }
    }
  }


  //INIT

  void initTexture(const std::string &filename){
    //generate a texture from the given file
    texId = loadTextureFromFileToGPU(filename);
  }

  void init(){
    // Create a single handle, vertex array object that contains attributes,
    // vertex buffer objects (e.g., vertex's position, normal, and color)

      // VAO
    #ifdef _MY_OPENGL_IS_33_
      glGenVertexArrays(1, &m_vao); // If your system doesn't support OpenGL 4.5, you should use this instead of glCreateVertexArrays.
    #else
      glCreateVertexArrays(1, &m_vao);
    #endif
      glBindVertexArray(m_vao);


      // VBOs
      size_t vertexBufferSize = sizeof(float)*m_vertexPositions.size(); // Gather the size of the buffer from the CPU-side vector

    #ifdef _MY_OPENGL_IS_33_
      glGenBuffers(1, &m_posVbo);
      glBindBuffer(GL_ARRAY_BUFFER, m_posVbo);
      glBufferData(GL_ARRAY_BUFFER, vertexBufferSize, m_vertexPositions.data(), GL_DYNAMIC_READ);
      glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3*sizeof(GLfloat), 0);
      glEnableVertexAttribArray(0);
    #else
      glCreateBuffers(1, &m_posVbo);
      glBindBuffer(GL_ARRAY_BUFFER, m_posVbo);
      glNamedBufferStorage(m_posVbo, vertexBufferSize, m_vertexPositions.data(), GL_DYNAMIC_STORAGE_BIT); // Create a data storage on the GPU and fill it from a CPU array
      glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3*sizeof(GLfloat), 0);
      glEnableVertexAttribArray(0);
    #endif

      size_t normalBufferSize = sizeof(float)*m_vertexNormals.size();

    #ifdef _MY_OPENGL_IS_33_
      glGenBuffers(1, &m_normalVbo);
      glBindBuffer(GL_ARRAY_BUFFER, m_normalVbo);
      glBufferData(GL_ARRAY_BUFFER, normalBufferSize, m_vertexNormals.data(), GL_DYNAMIC_READ);
      glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3*sizeof(GLfloat), 0);
      glEnableVertexAttribArray(1);
    #else
      glCreateBuffers(1, &m_normalVbo);
      glBindBuffer(GL_ARRAY_BUFFER, m_normalVbo);
      glNamedBufferStorage(m_normalVbo, normalBufferSize, m_vertexNormals.data(), GL_DYNAMIC_STORAGE_BIT); // Create a data storage on the GPU and fill it from a CPU array
      glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3*sizeof(GLfloat), 0);
      glEnableVertexAttribArray(1);
    #endif

      size_t texCoordBufferSize = sizeof(float)*m_vertexTexCoords.size();

    #ifdef _MY_OPENGL_IS_33_
      glGenBuffers(1, &m_texCoordVbo); 
      glBindBuffer(GL_ARRAY_BUFFER, m_texCoordVbo);
      glBufferData(GL_ARRAY_BUFFER, texCoordBufferSize, m_vertexTexCoords.data(), GL_DYNAMIC_READ);
      glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 2*sizeof(GLfloat), 0);
      glEnableVertexAttribArray(2);
    #else
      glCreateBuffers(1, &m_texCoordVbo);
      glBindBuffer(GL_ARRAY_BUFFER, m_texCoordVbo);
      glNamedBufferStorage(m_texCoordVbo, texCoordBufferSize, m_vertexTexCoords.data(), GL_DYNAMIC_STORAGE_BIT); // Create a data storage on the GPU and fill it from a CPU array
      glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 2*sizeof(GLfloat), 0);
      glEnableVertexAttribArray(2);
    #endif


      // IBO
      size_t indexBufferSize = sizeof(unsigned int)*m_triangleIndices.size();
    #ifdef _MY_OPENGL_IS_33_
      glGenBuffers(1, &m_ibo);
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ibo);
      glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexBufferSize, m_triangleIndices.data(), GL_DYNAMIC_READ);
    #else
      glCreateBuffers(1, &m_ibo);
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ibo);
      glNamedBufferStorage(m_ibo, indexBufferSize, m_triangleIndices.data(), GL_DYNAMIC_STORAGE_BIT);
    #endif

      glBindVertexArray(0); // deactivate the VAO for now, will be activated again when rendering

  }


  //RENDER

  void render(){
    // should be called in the main rendering loop
    // we assume the shader progams have been already linked
    // and the basic uniform attributes too

    
    glUniformMatrix4fv(glGetUniformLocation(g_program, "modelMat"), 1, GL_FALSE, glm::value_ptr(modelMatrix));
    glUniformMatrix3fv(glGetUniformLocation(g_program, "normalMat"), 1, GL_FALSE, glm::value_ptr(normalMatrix));
    glUniform3f(glGetUniformLocation(g_program, "meshColor"), color[0], color [1], color[2]);
    glUniform3f(glGetUniformLocation(g_program, "phongLightingRatio"), phongLightingRatio[0], phongLightingRatio[1], phongLightingRatio[2]);
    
    //texture
    if(texId == 0){
      glUniform1i(glGetUniformLocation(g_program, "hasTexture"), 0);
    }
    else{
      glUniform1i(glGetUniformLocation(g_program, "hasTexture"), 1);
      glActiveTexture(GL_TEXTURE0); //tex unit 0
      glBindTexture(GL_TEXTURE_2D, texId); //bind our texture to the tex unit 0
      glUniform1i(glGetUniformLocation(g_program, "texUnit"), 0); //we use tex Unit 0
    }

    glBindVertexArray(m_vao);     // activate the VAO storing geometry data
    glDrawElements(GL_TRIANGLES, m_triangleIndices.size(), GL_UNSIGNED_INT, 0); // Call for rendering: stream the current GPU geometry through the current GPU program

    glActiveTexture(GL_TEXTURE0); //tex unit 0
    glBindTexture(GL_TEXTURE_2D, 0); //unbind our texture 

  } 

  static std::shared_ptr<Mesh> genSphere(const size_t resolution=16){ // should generate a unit sphere
    std::shared_ptr<Mesh> sphere ( new Mesh(resolution));
    return sphere;
  }


  //GETTER

  glm::vec3 getPosition(){
    return position;
  }


  //SETTER

  void setColor(float r, float g, float b){
    color.clear();
    color.push_back(r);
    color.push_back(g);
    color.push_back(b);
  }

  void setPhongLightingRatio(float ambient, float diffuse, float specular){
    phongLightingRatio.clear();
    phongLightingRatio.push_back(ambient);
    phongLightingRatio.push_back(diffuse);
    phongLightingRatio.push_back(specular);
  }

  void setSize(float i_size){
    size = i_size;
    generateModelMatrix();
  }

  void setPosition(float x, float y, float z){
    position = glm::vec3(x, y, z);
    generateModelMatrix();
  }

  void setPosition(const glm::vec3& i_position){
    position = i_position;
    generateModelMatrix();
  }

  void setRotation(const glm::vec3& axis, float angle){
    rotationMatrix = glm::rotate(glm::mat4(1.0f), angle, axis);
    generateModelMatrix();
  }

  void addRotation(const glm::vec3& axis, float angle){
    //add a rotation to the current rotation matrix
    rotationMatrix = glm::rotate(rotationMatrix, angle, axis);
    generateModelMatrix();
  }

   
  private:

    //geometry
    std::vector<float> m_vertexPositions;
    std::vector<float> m_vertexNormals;
    std::vector<unsigned int> m_triangleIndices;
    std::vector<float> m_vertexTexCoords;
    GLuint m_vao = 0;
    GLuint m_posVbo = 0;
    GLuint m_normalVbo = 0;
    GLuint m_ibo = 0;
    GLuint m_texCoordVbo = 0;


    //texture
    GLuint texId=0;

    //attributes
    std::vector<float> color = {1.0, 1.0, 0.0}; //of size 3
    std::vector<float> phongLightingRatio = {1.0, 0.0, 0.0};
    glm::mat4 modelMatrix = glm::mat4(1.0f);
    float size = 1.0;
    glm::vec3 position = glm::vec3(0.);
    glm::mat4 rotationMatrix = glm::mat4(1.0f);
    glm::mat3 normalMatrix = glm::mat3(1.0f);


    void generateModelMatrix(){
      modelMatrix = glm::mat4(1.0f);
      modelMatrix = glm::translate(modelMatrix, position);
      modelMatrix = modelMatrix * rotationMatrix;
      modelMatrix = glm::scale( modelMatrix, glm::vec3( size ) );

      normalMatrix = glm::transpose(glm::inverse(glm::mat3(modelMatrix)));
    }
};

std::vector< std::shared_ptr<Mesh> > meshes;



//***************** MAIN *******************


// Executed each time the window is resized. Adjust the aspect ratio and the rendering viewport to the current window.
void windowSizeCallback(GLFWwindow* window, int width, int height) {
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
  } else if(action == GLFW_PRESS && (key == GLFW_KEY_SPACE)){
    virtualTimeOrigin = currentVirtualTime;
    realTimeOrigin = currentRealTime;
    animationRunning = !animationRunning;
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
    1024, 768,
    "Interactive 3D Applications (OpenGL) - Simple Solar System",
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
  if(!gladLoadGL(glfwGetProcAddress)) {
    std::cerr << "ERROR: Failed to initialize OpenGL context" << std::endl;
    glfwTerminate();
    std::exit(EXIT_FAILURE);
  }

  glCullFace(GL_BACK); // Specifies the faces to cull (here the ones pointing away from the camera)
  glEnable(GL_CULL_FACE); // Enables face culling (based on the orientation defined by the CW/CCW enumeration).
  glDepthFunc(GL_LESS);   // Specify the depth test for the z-buffer
  glEnable(GL_DEPTH_TEST);      // Enable the z-buffer test in the rasterization
  glClearColor(0.3f, 0.2f, 0.5f, 1.0f); // specify the background color, used any time the framebuffer is cleared
  glClearColor(0.2f, 0.05f, 0.3f, 1.0f);
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
  loadShader(g_program, GL_VERTEX_SHADER, "../vertexShader.glsl");
  loadShader(g_program, GL_FRAGMENT_SHADER, "../fragmentShader.glsl");
  glLinkProgram(g_program); // The main GPU program is ready to be handle streams of polygons

  glUseProgram(g_program);
}


void initMesh(){

  //sun
  std::shared_ptr<Mesh> sun = Mesh::genSphere(21); //since the sun is bigger, i set the resolution hiher
  sun->init();
  sun->setColor(1., 1., 0.);
  sun->setPhongLightingRatio(1.0, 0.0, 0.0);
  sun->setSize(planetSize*kSizeSun);
  sun->setPosition(0., 0., 0.);
  meshes.push_back(sun);

  //earth
  std::shared_ptr<Mesh>  earth = Mesh::genSphere(16);
  earth->init();
  earth->initTexture("../media/earth.jpg");
  earth->setColor(0., 1., 0.5);
  earth->setPhongLightingRatio(0.1, 0.7, 0.2);
  earth->setSize(planetSize*kSizeEarth);
  earth->setPosition(sun->getPosition() + radSize*kRadOrbitEarth*glm::vec3(1., 0., 0.));
  meshes.push_back(earth);

  //moon
  std::shared_ptr<Mesh>  moon = Mesh::genSphere(16);
  moon->init();
  moon->initTexture("../media/moon.jpg");
  moon->setColor(0., 0., 1.);
  moon->setPhongLightingRatio(0.1, 0.6, 0.3);
  moon->setSize(planetSize*kSizeMoon);
  moon->setPosition(earth->getPosition() + radSize*kRadOrbitMoon*glm::vec3(1., 0., 0.));
  meshes.push_back(moon);

}

void initCamera() {
  int width, height;
  glfwGetWindowSize(g_window, &width, &height);
  g_camera.init(glm::vec3(0.0, 0.0, -3.0), glm::vec3(0.0, 0.0, 0.0), glm::vec3(0.0, 1.0, 0.0));
  g_camera.setAspectRatio(static_cast<float>(width)/static_cast<float>(height));
  g_camera.setNear(0.1);
  g_camera.setFar(80.1);
}

void init() {
  initGLFW();
  initOpenGL();
  initMesh();
  initGPUprogram();
  initCamera();
}

void clear() {
  glDeleteProgram(g_program);
  glfwDestroyWindow(g_window);
  glfwTerminate();
}

// The main rendering call
void render() {
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // Erase the color and z buffers.

  const glm::mat4 viewMatrix = g_camera.computeViewMatrix();
  const glm::mat4 projMatrix = g_camera.computeProjectionMatrix();
  const glm::mat4 identity = glm::mat4( 1.0f );

  const glm::vec3 camPosition = g_camera.getPosition();
  glUniform3f(glGetUniformLocation(g_program, "camPos"), camPosition[0], camPosition[1], camPosition[2]);
  glUniformMatrix4fv(glGetUniformLocation(g_program, "viewMat"), 1, GL_FALSE, glm::value_ptr(viewMatrix)); // compute the view matrix of the camera and pass it to the GPU program
  glUniformMatrix4fv(glGetUniformLocation(g_program, "projMat"), 1, GL_FALSE, glm::value_ptr(projMatrix)); // compute the projection matrix of the camera and pass it to the GPU program
  glUniform3f(glGetUniformLocation(g_program, "lightSource"), 0.0, 0.0, 0.0);

  for(std::shared_ptr<Mesh> mesh : meshes){
    mesh->render();
  }
}

// Update any accessible variable based on the current time
void update(const float currentTimeInSec) {

  float delta = currentTimeInSec - currentRealTime; //time before the previous call to update

  //CAMERA
  

  if(key_shift_pressed){
    if(key_right_pressed){
      g_camera.rotate_right(delta);
    }
    else if(key_left_pressed){
      g_camera.rotate_left(delta);
    }
    else if(key_up_pressed){
      g_camera.move_forward(delta);
    }
    else if(key_down_pressed){
      g_camera.move_backward(delta);
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
      g_camera.move_up(delta);
    }
    else if(key_down_pressed){
      g_camera.move_down(delta);
    }
  }


  currentRealTime = currentTimeInSec; //global variable to freeze the animation when
                                      //the space key is pressed

  
  if(animationRunning){
    currentVirtualTime = virtualTimeOrigin + currentRealTime - realTimeOrigin;

      //orbital roatations are in the xz plane

    //position
    float thetaEarth = 2*glm::pi<float>()*currentVirtualTime/orbitPeriodEarth;
    float thetaMoon = 2*glm::pi<float>()*currentVirtualTime/orbitPeriodMoon;
    glm::vec3 earthRelativePosition = glm::vec3(cos(thetaEarth), 0., sin(thetaEarth));
    meshes[1]->setPosition(radSize*kRadOrbitEarth*earthRelativePosition);

    glm::vec3 moonRelativePosition = glm::vec3(cos(thetaMoon), 0., sin(thetaMoon));
    meshes[2]->setPosition(meshes[1]->getPosition() + radSize*kRadOrbitMoon*moonRelativePosition);


    //rotation

    float alphaEarth = 2*glm::pi<float>()*currentVirtualTime/rotationPeriodEarth;
    float axisAngle = glm::pi<float>()*23.5/180;
    glm::vec3 rotationAxis = glm::vec3(sin(axisAngle), cos(axisAngle), 0.0);

    float alphaMoon = -thetaMoon ;
    meshes[1]->setRotation(glm::vec3(0.0, 0.0, 1.0), axisAngle); //the earth is inclined
    meshes[1]->addRotation(glm::vec3(0.0, 1.0, 0.0), alphaEarth); //the earth rotates on itself
    meshes[2]->setRotation(glm::vec3(0.0, 1.0, 0.0), alphaMoon); //the moon rotates on itself
  }
  
}

int main(int argc, char ** argv) {
  init(); // Your initialization code (user interface, OpenGL states, scene with geometry, material, lights, etc)

  while(!glfwWindowShouldClose(g_window)) {
    update(static_cast<float>(glfwGetTime()));
    render();
    glfwSwapBuffers(g_window);
    glfwPollEvents();
  }
  clear();
  return EXIT_SUCCESS;
}
