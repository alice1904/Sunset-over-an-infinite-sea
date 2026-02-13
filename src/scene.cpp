#include "scene.h"




void Scene::init(){

    printf("init scene\n");
    

    float width = 4.f;
    float height = 0.0f;//0.1f
    float depth = 4.f;//the sea is a grid of size wave_resolution x wave_resolution
    float width_step = 2*width/wave_resolution;
    float depth_step = 2*depth/wave_resolution;


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

            vertexRelativePositions.push_back(glm::vec3(x, y, z));
            vertexPositions.push_back(vertexRelativePositions[i]);
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
    vertexVelocities = std::vector<glm::vec3>(vertexPositions.size(), glm::vec3(0.0, 0.0, 0.0));
    springConstants.resize(vertexPositions.size());
    for(int i=0; i<springConstants.size(); i++){
        float r = ((double) rand() / (RAND_MAX)); //random number between 0 and 1
        r = r*2 -1; //random number between -1 and 1
        springConstants[i] = springConstantMean + r*springConstantVariance;
    }

}

float Scene::waveFunction(float x, float y, float t){
    float phase = waveOmega*t - glm::dot(waveVector, glm::vec2(x, y));
    return waveAmplitude*(cos(phase));
}

void Scene::update(float dt){
    currentTime+=dt; //time since simulation started
    for(int i=0; i<vertexPositions.size(); i++){
        glm::vec3 direction = glm::vec3(0.0, vertexRelativePositions[i].y, 0.0);
        float length = glm::length(direction);
        if(length!=0.0){
            direction = glm::normalize(direction);
        }
        float k = springConstants[i];
        glm::vec3 acc = -k*(length - l0)*direction; //we ignore the mass which is included in k
        vertexVelocities[i] += dt*acc;
        vertexRelativePositions[i] += dt*vertexVelocities[i];

        float x = vertexRelativePositions[i].x;
        float y = vertexRelativePositions[i].y;
        float z = vertexRelativePositions[i].z;
        float offset = waveFunction(x, z, currentTime);

        vertexPositions[i] = glm::vec3(0, offset, 0) + vertexRelativePositions[i];
        vertexPositions[i] = glm::vec3(0, offset, 0) + vertexRelativePositions[i];
    }
}