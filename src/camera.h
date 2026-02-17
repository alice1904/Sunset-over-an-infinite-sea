
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