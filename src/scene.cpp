#include "scene.h"

void Scene::init(){

    printf("init scene\n");
    vertexPositions = { // the array of vertex Colors [x0, y0, z0, x1, y1, z1, ...]
        glm::vec3(1.f, 0.f, 0.f),
        glm::vec3(0.f, 1.f, 0.f),
        glm::vec3(0.f, 0.f, 1.f),

        glm::vec3(0.5f, 0.5f, 0.f),
        glm::vec3(-1.f, 1.f, 0.f),
        glm::vec3(-1.f, 0.f, 10.f)

    };

    triangleIndices = {
        glm::uvec3(0, 1, 2),
        glm::uvec3(3, 4, 5)

    };
}