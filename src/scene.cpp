#include "scene.h"




void Scene::init(){

    printf("init scene\n");
    float r = ((double) rand() / (RAND_MAX)); //random number between 0 and 1
    std::cout<<r<<std::endl;
    

    float width = 4.f;
    float height = 0.0f;//0.1f
    float depth = 4.f;
    int wave_resolution = 10; //the sea is a grid of size wave_resolution x wave_resolution
    float width_step = 2*width/wave_resolution;
    float depth_step = 2*depth/wave_resolution;

    float height_variance = 0.1f; //0.1

    // VERTICES
    for(int i=0; i<=wave_resolution; i++){
        for(int j=0; j<=wave_resolution; j++){
            float x = -width + i*width_step;
            float z = -depth + j*depth_step;

            float r = ((double) rand() / (RAND_MAX)); //random number between 0 and 1
            r = r*2 -1; //random number between -1 and 1
            float y = -height + r*height_variance;
            if(i==1 && j==1){
                y = +height;
            }
            vertexPositions.push_back(glm::vec3(x, y, z));
        }
    }


    //TRIANGLES
    for(int i=0; i<wave_resolution; i++){
        for(int j=0; j<wave_resolution; j++){
            int a, b, c, d; //indices of the corners of the square
            a = i   +      j*(wave_resolution+1);
            b = i+1 +      j*(wave_resolution+1);
            c = i+1 +  (j+1)*(wave_resolution+1);
            d = i   +  (j+1)*(wave_resolution+1);
            triangleIndices.push_back(glm::uvec3(a, b, c));
            triangleIndices.push_back(glm::uvec3(a, c, d));
        }
    }

    //OBJECTS
    int n_triangles = triangleIndices.size();
    objectProperties = {
        {glm::vec3(0, 1, 1),
        n_triangles, 0.33, 0.33, 0.7, 0.0} //0.6, 0.0
    };

    //PHYSICS ATTRIBUTES
    vertexVelocities.resize(vertexPositions.size());

}


void Scene::update(float dt){
    for(int i=0; i<vertexPositions.size(); i++){
        vertexVelocities[i] = vertexPositions[i];
    }
}