#ifndef SCENE_H
#define SCENE_H

#include <glm/glm.hpp>

#include <stdio.h>

#include <vector>


class Scene {

    public:
        Scene(){};
        ~Scene(){};

        void init();

        std::vector<glm::vec3> vertexPositions;
        std::vector<glm::uvec3> triangleIndices;
        std::vector<glm::vec3> vertexNormals;

};

#endif