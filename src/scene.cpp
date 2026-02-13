#include "scene.h"

int getModulo(int a, int b){
    // return c s.a. a = c[b]
    // and c€[0, b-1]
    if(b<=0){
        std::cout<< "ERROR [Scene::getModulo] : b should be greater than 0";
        assert(b>0);
    }
    if(a>=0){
        return a%b;
    }
    return (a + (-a/b + 1)*b)%b;
}

void testGetModulo(){
    assert(getModulo(0, 1)==0);
    assert(getModulo(6, 1)==0);
    assert(getModulo(-6, 1)==0);
    assert(getModulo(5, 3)==2);
    assert(getModulo(-1, 5)==4);
    assert(getModulo(-48, 20)==12);
}

int Scene::getVertexIndex(int i ,int j){
    //return the index of the vertex in vertexPosition and vertexRelativePOsitions
    //note : i and j can be <0 or >SIZE
    //in this case, we compute the result of==for i%SIZE and j%SIZE
    i = getModulo(i, wave_resolution+1);
    j = getModulo(j, wave_resolution+1);
    return i + j*(wave_resolution+1);;
}


void Scene::init(glm::vec3 cameraCenter){
    //the sea is set around the point the camera is looking at
    testGetModulo();
    printf("init scene\n");
    
    float halfWidth = seaWidth/2;
    float height = 0.0f;
    float halfDepth = seaWidth/2; //the sea is a grid of size wave_resolution x wave_resolution
    float width_step = seaWidth/wave_resolution;
    float depth_step = seaWidth/wave_resolution;


    // VERTICES
    for(int i=0; i<=wave_resolution; i++){
        for(int j=0; j<=wave_resolution; j++){
            float x = cameraCenter.x -halfWidth + i*width_step;
            float z = cameraCenter.z -halfDepth + j*depth_step;

            float r = ((double) rand() / (RAND_MAX)); //random number between 0 and 1
            r = r*2 -1; //random number between -1 and 1
            float y = -height + r*height_variance;
            if(i==1 && j==1){
                y = +height;
            }

            vertexRelativePositions.push_back(glm::vec3(x, y, z));
            vertexPositions.push_back(vertexRelativePositions[i]); //it will be update with the cosinus when the 
                                                            //update() function will be called
        }
    }


    //TRIANGLES
    for(int i=0; i<wave_resolution; i++){
        for(int j=0; j<wave_resolution; j++){
            int a, b, c, d; //indices of the corners of the square
            a = getVertexIndex(i, j);
            b = getVertexIndex(i+1, j);
            c = getVertexIndex(i+1, j+1);
            d = getVertexIndex(i, j+1);
            // a = i   +      j*(wave_resolution+1);
            // b = i+1 +      j*(wave_resolution+1);
            // c = i+1 +  (j+1)*(wave_resolution+1);
            // d = i   +  (j+1)*(wave_resolution+1);
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

void Scene::update(float dt, glm::vec3 cameraCenter){
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