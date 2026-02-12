#ifndef SCENE_H
#define SCENE_H

#include <glm/glm.hpp>

#include <stdio.h>

#include <vector>

#include <stdint.h>
#include <iostream>
#include <cstdlib> //for random values


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
        void init();

        //compute the forces, 
        //the new state vector 
        //and update the scene
        void update(float dt);

        std::vector<glm::vec3> vertexPositions;
        std::vector<glm::uvec3> triangleIndices;
        std::vector<glm::vec3> vertexNormals;

        std::vector<ObjectProperties> objectProperties;

    private:
        std::vector<glm::vec3> vertexVelocities;
        //spring properties
        const float springConstantMean = 2.0;
        const float springConstantVariance = 0.0;//1.0;
        std::vector<float> springConstants; 
        float l0 = 0.0f;
        
};

#endif