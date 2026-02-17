#include "scene.h"



// GENERAL USE FUNCTIONS AND VARIABLES

void print_vec3(glm::vec3 v){
    std::cout<<v.x<<" "<<v.y<<" "<<v.z<<std::endl;
}

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

int getRoundInt(float x){
    if(x>=0){
        int n = (int) x;
        if (x-n>0.5){
            return n+1;
        }
        return n;
    }
    int n = (int)x;
    if(n-x>0.5){
        return n-1;
    }
    return n;
}



// GEOMETRY

int Scene::getVertexIndex(int i ,int j){
    //return the index of the vertex in vertexPosition and vertexRelativePOsitions
    //note : i and j can be <0 or >SIZE
    //in this case, we compute the result of==for i%SIZE and j%SIZE
    i = getModulo(i, wave_resolution+1);
    j = getModulo(j, wave_resolution+1);
    return j + i*(wave_resolution+1);;
}

void Scene::updateTriangles(){
    int vertexIndex = 0;
    for(int i=0; i<=wave_resolution; i++){
        for(int j=0; j<=wave_resolution; j++){
            //vertexIndex = j + i*(wave_resolution+1)
            if(squareInfos[vertexIndex].hasChanged){
                squareInfos[vertexIndex].hasChanged = false;
                if(squareInfos[vertexIndex].shouldBeDisplayed){
                    int a, b, c, d; //indices of the corners of the square
                    a = getVertexIndex(i, j);
                    b = getVertexIndex(i+1, j);
                    c = getVertexIndex(i+1, j+1);
                    d = getVertexIndex(i, j+1);
                    triangleIndices[vertexIndex*2]   =  glm::uvec3(a, b, c);
                    triangleIndices[vertexIndex*2+1] = glm::uvec3(a, c, d);
                }
                else{
                    //this triangle is a point and will not be visible
                    triangleIndices[vertexIndex*2] = glm::uvec3(0, 0, 0);
                    triangleIndices[vertexIndex*2+1] = glm::uvec3(0, 0, 0);
                }
                vertexIndex+=1;
            }
        }
    }
}



// INIT

void Scene::init(glm::vec3 cameraCenter){
    //the sea is set around the point the camera is looking at
    testGetModulo();
    printf("init scene\n");
    
    float halfWidth = seaWidth/2;
    float height = 0.0f;
    float halfDepth = halfWidth; //the sea is a grid of size wave_resolution x wave_resolution
    float depth_step = width_step;


    // VERTICES
    for(int i=0; i<=wave_resolution; i++){
        for(int j=0; j<=wave_resolution; j++){
            float x = cameraCenter.x -halfWidth + i*width_step;
            float z = cameraCenter.z -halfDepth + j*depth_step;
            float r = ((double) rand() / (RAND_MAX)); //random number between 0 and 1
            r = r*2 -1; //random number between -1 and 1
            float y = -height + r*height_variance;

            vertexRelativePositions.push_back(glm::vec3(x, y, z));
            vertexPositions.push_back(glm::vec3(x, y, z)); //it will be update with the cosinus when the 
                                                            //update() function will be called
            
            if(i<wave_resolution && j<wave_resolution){
                squareInfos.push_back({true, true});
            }
            else{
                squareInfos.push_back({false, true});
            }
        }
    }

    //TRIANGLES
    triangleIndices.resize(2*vertexPositions.size());
    updateTriangles();


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



// UPDATE

void Scene::getGridPositionRelativeToCamera(glm::vec3 pos, glm::vec3 cameraCenter, int* i, int*j){
    float halfWIdth = seaWidth/2;
    *i = getRoundInt((pos.x - cameraCenter.x + halfWIdth)/width_step);
    *j = getRoundInt((pos.z - cameraCenter.z + halfWIdth)/width_step);
}

float Scene::waveFunction(float x, float y, float t){
    float phase = waveOmega*t - glm::dot(waveVector, glm::vec2(x, y));
    return waveAmplitude*(cos(phase));
}

void Scene::update(float dt, glm::vec3 cameraCenter){
    currentTime+=dt; //time since simulation started
    for(int vertexIndex=0; vertexIndex<vertexPositions.size(); vertexIndex++){

        //infinite sea : moving vertices
        int i,j;
        getGridPositionRelativeToCamera(vertexRelativePositions[vertexIndex], cameraCenter, &i, &j);
        int new_i, new_j;
        new_i = getModulo(i, wave_resolution+1);
        new_j = getModulo(j, wave_resolution+1);
        if(i!=new_i || j!=new_j){
            //should move vertex
            if(i!=new_i){
                vertexRelativePositions[vertexIndex].x += ((new_i - i)/(wave_resolution+1))*(seaWidth+width_step);
            }
            if(j!=new_j){
                vertexRelativePositions[vertexIndex].z += ((new_j - j)/(wave_resolution+1))*(seaWidth+width_step);
            }
        }

        //infinite sea : updating triangles
        bool shouldBeDisplayed = true;
        if(new_i==wave_resolution || new_j==wave_resolution){
            shouldBeDisplayed = false;
        }
        if(squareInfos[vertexIndex].shouldBeDisplayed!=shouldBeDisplayed){
            squareInfos[vertexIndex].shouldBeDisplayed = shouldBeDisplayed;
            squareInfos[vertexIndex].hasChanged = true;
        }
        squareInfos[vertexIndex].shouldBeDisplayed = shouldBeDisplayed;
        squareInfos[vertexIndex].hasChanged = true;

        //spring force
        glm::vec3 direction = glm::vec3(0.0, vertexRelativePositions[vertexIndex].y, 0.0);
        float length = glm::length(direction);
        if(length!=0.0){
            direction = glm::normalize(direction);
        }
        float k = springConstants[vertexIndex];
        glm::vec3 acc = -k*(length)*direction; //we ignore the mass which is included in k
        vertexVelocities[vertexIndex] += dt*acc;
        vertexRelativePositions[vertexIndex] += dt*vertexVelocities[vertexIndex];

        //general wave offset
        float x = vertexRelativePositions[vertexIndex].x;
        float z = vertexRelativePositions[vertexIndex].z;
        float offset = waveFunction(x, z, currentTime);

        vertexPositions[vertexIndex] = glm::vec3(0, offset, 0) + vertexRelativePositions[vertexIndex];
        vertexPositions[vertexIndex] = glm::vec3(0, offset, 0) + vertexRelativePositions[vertexIndex];
    }

    updateTriangles();
}