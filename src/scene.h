#ifndef SCENE_H
#define SCENE_H

#include <glm/glm.hpp>

#include <stdio.h>

#include <vector>

#include <stdint.h>
#include <iostream>
#include <cstdlib> //for random values



typedef struct  {
    bool shouldBeDisplayed;
    bool hasChanged;
} SquareInfo; 

struct ObjectProperties_struct {
    //alignement set to 4 float for the vec3 in glsl
    glm::vec3 color;
    //float shininess;
    int32_t endIndex; //it works without uint32_t, but just to be sure it is 4 bytes...

    float diffuseRatio;
    float specularRatio;
    float reflectionRatio;
    float refractionRatio;

    //eventually : shininess, refraction indice ?
}; typedef struct ObjectProperties_struct ObjectProperties;

class Scene {

    public:
        Scene(){};
        ~Scene(){};

        //init the geometry
        void init(glm::vec3 cameraCenter);


        void printHelp(glm::vec3 cameraCenter);

        //compute the forces, 
        //the new state vector 
        //and update the scene
        void update(float dt, glm::vec3 cameraCenter);

        std::vector<glm::vec3> vertexPositions;
        std::vector<glm::uvec3> triangleIndices;
        std::vector<glm::vec3> vertexNormals;

        std::vector<ObjectProperties> objectProperties;

    private:
        float currentTime = 0.0;
        const float seaWidth = 8.f; //the sea is a square of side seaWidth
        std::vector<glm::vec3> vertexVelocities;
        std::vector<glm::vec3> vertexRelativePositions; //position relative to the point on the cosinus
        
        int getVertexIndex(int i ,int j);

        //infinite wave
        std::vector<SquareInfo> squareInfos; //tells wether the couple of triangle 
                            //that represent the square whose left corer is vertex[i]
                            //should be display or not
        
        //return (i,j) s.a. x = cameraCenter.x - seaWidth/2 + i*seaWidth/wave_resolution
        //and z = cameraCenter.z - seaWidth/2 + j*seaWidth/wave_resolution
        void getGridPositionRelativeToCamera(glm::vec3 pos, glm::vec3 cameraCenter, int* i, int*j);
        void updateTriangles(glm::vec3 cameraCenter);

        //wave parameters
        const int wave_resolution = 10; 
        const float width_step = seaWidth/wave_resolution;
        const float height_variance = 0.15f;//0.15f;//0.15f; 
        const float waveOmega = 1.5f; //pulse
        const glm::vec2 waveVector = -glm::vec2(1.0, 1.0); //wave vector
        const float waveAmplitude = 0.18f;
        float waveFunction(float x, float y, float t);

        //spring properties
        const float springConstantMean = 2.0;//2.0;
        const float springConstantVariance = 1.0;//1.0;//1.0;
        std::vector<float> springConstants; 
        float l0 = 0.0f;


      
};

#endif