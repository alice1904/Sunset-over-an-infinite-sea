#version 430



// PARAMETERS

#define RECURSION_DEPTH 3
#define TEST_MODE



// INPUTS / OUTPUTS

in vec3 non_normalized_ray_direction;
out vec4 color;	  // Shader output: the color response attached to this fragment


struct ObjectProperties {
    //alignement set to 4 float for the vec3 in glsl
    vec3 color;
    int endIndex; //4 bytes like float

    float diffuseRatio;
    float specularRatio;
    float reflectionRatio;
    float refractionRatio;
};

layout(std430, binding = 0) buffer sceneVertexPositions {
    vec3[] vertexPositions;
};

layout(std430, binding = 1) buffer sceneObjectProperties {
    ObjectProperties[] objectProperties;
};

layout(std430, binding = 2) buffer scenetriangleIndices {
    uvec3[] triangleIndices;
};


uniform int n_triangles;
uniform int n_objects;
uniform vec3 lightDirection;
uniform vec3 camera_position;




// GENERAL USE FUNCTIONS AND VARIABLES

const float epsilon = 0.001f; 

bool float_is_null(float x){
	return x>-epsilon && x<epsilon;
}

bool ERROR_OCCURED = false;




// RAY TREE REPRESENTATION

struct rayInformation {
	bool shouldBeComputed; //true if the father ray intersected a triangle
	bool dataValid;	
	vec3 position;
	vec3 reflectedRay;
	vec3 refractedRay;
	vec3 triangleColor;   //color of the object intersected without recursion
	vec3 finalColor;      //final color with recursion
	int objectIndex;
};

const int rayTreeSize = 2<<RECURSION_DEPTH - 1; //2^(RECURSION_DEPTH+1) -1
rayInformation rayTree[rayTreeSize];

int getLeftSonIndex(int i){
	return 2*i+1;
}

int getRightSonIndex(int i){
	return 2*i+2;
}

int getFatherIndex(int i){
	if(i==0){
		return 0;
	}
	if(i%2==0){
		return (i-1)/2;
	}
	return i/2;
}




// BACKGROUND

vec3 getGradient(vec3 color0, vec3 color1, float x){
	//x must be between 0 and 1
	//return the interpolated color,
	//x between the "distance" to color0
	if(x<0.0 || x>1.0){
		ERROR_OCCURED=true;
	}
	return (1-x)*color0 + x*color1;
}

vec3 getBackgroundColor(vec3 rayDirection){
	//return the color of the sky in this direction
	float sun_closeness = dot(rayDirection, normalize(lightDirection));
	vec3 sunColor = vec3(1.0, 1.0, 0.5);
	vec3 orange =vec3(1.0, 0.63, 0.2); 
	vec3 pink = vec3(1.0, 0.5, 0.5);
	vec3 blue = vec3(0.0, 0.0, 0.26);
	float sunThreshold = 0.998;
	float haloThreshold = 0.99;
	float pinkSkyThreshold = 0.9;

	if(sun_closeness>sunThreshold){
		return sunColor;
	}
	else if(sun_closeness>haloThreshold){
		float x = (sun_closeness-haloThreshold)/(sunThreshold-haloThreshold);
		return getGradient(orange, sunColor, x);
	}
	else if(sun_closeness>pinkSkyThreshold){
		float x = (sun_closeness - pinkSkyThreshold)/(haloThreshold - pinkSkyThreshold);
		return getGradient(pink, orange, x);
	}
	else{
		float x = (sun_closeness - -1)/(pinkSkyThreshold - -1);
		return getGradient(blue, pink, x);
	}
}




// RAY TRACING

int findObjectIndex(int triangleIndex){
	int objectIndex = 0;
	while(objectIndex<n_objects){
		if(triangleIndex < objectProperties[objectIndex].endIndex){
			return objectIndex;
		}
		objectIndex++;
	}
	// we have an error because the triangle index is above the last object endIndex
	// let's return 0 and hope the user will look at this comment
	// when he will have the wrong color on its object.
	// (It's very frustrating not being able to raise exceptions
	// or send logs. I don't know whether it is possible or not in opengl.
	// I think it is possible in vulkan, but obviously I don't have the courage to 
	// face vulkan for this project)
	ERROR_OCCURED=true;
	return 0;
}

bool ray_intersects_triangle(vec3 ray_origin, vec3 ray_direction, vec3 v1, vec3 v2, vec3 v3, out float dist, out float u, out float v){
	vec3 edge1 = v2 - v1;
    vec3 edge2 = v3 - v1;
    vec3 ray_cross_edge2 = cross(ray_direction, edge2);
	float det = dot(ray_cross_edge2, edge1); 

	//initialize to avoid errors
	dist=0.0f;
	u=0.0f;
	v=0.0f;

	if(float_is_null(det)){
		//triangle parallel to the ray
		return false;
	}

	float inv_det = 1.0f/det;
	vec3 v1_to_origin = ray_origin - v1;

	u = inv_det * dot(v1_to_origin, ray_cross_edge2);
	if(u<0.0f || u>1.0f){
		//intersection point not in the triangle
		return false;
	}

	vec3 v1_to_origin_cross_edge1 = cross(v1_to_origin, edge1);
	v = inv_det * dot(ray_direction, v1_to_origin_cross_edge1);
	if(v<0.0f || u+v >1.0f){
		return false;
	}

	//the ray intersects the triangle, we compute the distace
	dist = inv_det * dot(edge2, v1_to_origin_cross_edge1);
	if(dist<epsilon){
		//behind the screen. 
		return false; 
	}

	return true;
}

bool find_closest_intersected_triangle(vec3 ray_origin, vec3 ray_direction, out int triangleIndice, out float best_distance, out float best_u, out float best_v){
	//return true if one triangle is intersected.
	//give the indices of the closest triangle.

	//initialize out variables
	best_distance = 0.0;
	triangleIndice = -1;
	best_u = 0.0;
	best_v = 0.0;

	bool triangle_found = false;
	
	vec3 v1, v2, v3;
	float dist;
	float u, v;
	for(int i=0; i<n_triangles; i++){
		
		v1 = vertexPositions[triangleIndices[i][0]];
		v2 = vertexPositions[triangleIndices[i][1]];
		v3 = vertexPositions[triangleIndices[i][2]];

		if(ray_intersects_triangle(ray_origin, ray_direction, v1, v2, v3, dist, u, v)){
			if(!triangle_found || dist < best_distance){
				triangle_found = true;
				best_distance = dist;
				best_u = u;
				best_v = v;
				triangleIndice = i;
			}
		}
	}
	return triangle_found;
}

bool getRayInformation(vec3 ray_origin, vec3 ray_direction, int outputNodeIndex){
	//return true if a triangle was intersected and false if it went to infinite.
	//Write the following information in the ray tree node:
	//		- position of the intersection point
	//		- reflected ray
	//		- refracted ray
	//		- triangleColor : the color of the triangle at the point, or color of the background
	//		- objectIndex : index of the object the triangle belongs to 


	rayTree[outputNodeIndex].dataValid = true;

	int triangleIndice;
	float dist;
	float u, v; //barycentrix coordinate
	if(find_closest_intersected_triangle(ray_origin, ray_direction, triangleIndice, dist, u, v)){

		//color
		int objectIndex = findObjectIndex(triangleIndice);
		rayTree[outputNodeIndex].objectIndex = objectIndex;
		vec3 fragmentColor = objectProperties[objectIndex].color;
		float diffuseRatio = objectProperties[objectIndex].diffuseRatio;
		float specularRatio = objectProperties[objectIndex].specularRatio;
		float ambientRatio = 1 - specularRatio - diffuseRatio;
		vec3 ambient  = ambientRatio  * fragmentColor;
		

		//position and normal
		vec3 position = ray_origin + dist*ray_direction;
		rayTree[outputNodeIndex].position = position;
		vec3 edge1 = vertexPositions[triangleIndices[triangleIndice].y] - vertexPositions[triangleIndices[triangleIndice].x];
		vec3 edge2 = vertexPositions[triangleIndices[triangleIndice].z] - vertexPositions[triangleIndices[triangleIndice].x];
		vec3 normal = cross(edge1, edge2);
		normal = normalize(normal);
		if(dot(ray_direction, normal)>0){
				normal = -normal; //we are looking at the other side of the triangle.
		}

			//triangleColor
		//light
		float shininess=2.0;
		vec3 lightColor = normalize(vec3(1.0, 1.0, 1.0));
		vec3 reflection = normalize(2*(dot(lightDirection, normal))*normal - lightDirection);
		vec3 vue = -ray_direction;
		vec3 diffuse  = diffuseRatio  * fragmentColor * max(dot(normal, lightDirection), 0)*lightColor;
		vec3 specular = specularRatio * pow(max(dot(vue, reflection), 0), shininess)*lightColor;
		rayTree[outputNodeIndex].triangleColor = ambient + diffuse + specular;
		//shadow
		int _triangleIndice;
		float _distance, _u, _v;
		if(dot(normal, lightDirection)<0 || find_closest_intersected_triangle(position, lightDirection, _triangleIndice, _distance, _u, _v)){
			rayTree[outputNodeIndex].triangleColor = rayTree[outputNodeIndex].triangleColor/2.0;
		}

			//rays out
		rayTree[outputNodeIndex].reflectedRay = normalize(2*(dot(-ray_direction, normal))*normal + ray_direction);
		rayTree[outputNodeIndex].refractedRay = ray_direction;

		return true;

	}
	else{
		//need to initialize out values to avoid
		//strange errors
		rayTree[outputNodeIndex].position = vec3(0.0, 0.0, 0.0);
		rayTree[outputNodeIndex].reflectedRay = vec3(0.0, 0.0, 0.0);
		rayTree[outputNodeIndex].refractedRay = vec3(0.0, 0.0, 0.0);
		rayTree[outputNodeIndex].objectIndex = -1;

		//background
		rayTree[outputNodeIndex].triangleColor = getBackgroundColor(ray_direction);
		return false;

	}
}




// MAIN COMPUTATIONS

void initRayTree(){
	//set valid flag of each node to 0
	for(int i=0; i<rayTreeSize; i++){
		rayTree[i].dataValid = false;
		rayTree[i].shouldBeComputed = false;
	}
}

void enableSonsComputation(int i){
	int refractedSon = getRightSonIndex(i);
	if(refractedSon<rayTreeSize){
		int reflectedSon = getLeftSonIndex(i);
		rayTree[reflectedSon].shouldBeComputed=true;
		rayTree[refractedSon].shouldBeComputed=true;
	}
}

void computeRayTree(vec3 camera_position, vec3 initial_ray_direction){
	//compute all the rays information
	//except final color
	//and write it in the rayTree
	initRayTree();
	if(getRayInformation(camera_position, initial_ray_direction, 0)){
		enableSonsComputation(0);
	}

	for(int i=1; i<rayTreeSize; i++){
		if(rayTree[i].shouldBeComputed){
			int fatherIndex = getFatherIndex(i);
			if(!rayTree[fatherIndex].dataValid){
				ERROR_OCCURED=true;
			}
		
			vec3 ray_direction;
			if(i%2==1){
				//we are a left son
				ray_direction = rayTree[fatherIndex].reflectedRay;
			}
			else{
				ray_direction = rayTree[fatherIndex].refractedRay;
			}
			if(getRayInformation(rayTree[fatherIndex].position, ray_direction, i)){
				//trangle intersected
				//enable sons computation
				enableSonsComputation(i);
			}
		}
	}
}

void computeRaysColor(){
	//compute the final color
	//of each ray of the rayTree

	for(int i = rayTreeSize-1; i>=0; i--){ 
		//the array implementation of the binary tree
		//ensures that the sons are placed after the father
		//so I can compute the color from leaves to root
		//this way
		if(rayTree[i].dataValid){
			int reflectedSon = getLeftSonIndex(i);
			int refractedSon = getRightSonIndex(i);
			if(refractedSon<rayTreeSize && rayTree[reflectedSon].dataValid && rayTree[refractedSon].dataValid){
				int objectIndex = rayTree[i].objectIndex;
				if(objectIndex<0){
					ERROR_OCCURED=true;
				}
				float reflectionRatio = objectProperties[objectIndex].reflectionRatio;
				float refractionRatio = objectProperties[objectIndex].refractionRatio;
				float triangleColorRatio = 1 - reflectionRatio - refractionRatio;

				vec3 reflectedColor = rayTree[reflectedSon].finalColor;
				vec3 refractedColor = rayTree[refractedSon].finalColor;
				vec3 triangleColor = rayTree[i].triangleColor;

				rayTree[i].finalColor = reflectionRatio*reflectedColor 
						+ refractionRatio*refractedColor 
						+ triangleColorRatio*triangleColor;
			}
			else{
				//we are in a leave of the tree either because of the 
				//limited number of recursion or because the ray
				//didn't intersect any triangle.
				//we put the final color to the triangleColor
				rayTree[i].finalColor = rayTree[i].triangleColor;
			}
		}

	}
}

void main() {
	vec3 ray_direction = normalize(non_normalized_ray_direction);
	computeRayTree(camera_position, ray_direction);
	computeRaysColor();
	color = vec4(rayTree[0].finalColor, 1.0);

	#ifdef TEST_MODE
	if(ERROR_OCCURED){
		//enable the programmer to detect errors
		color = vec4(1.0, 0.0, 0.0, 1.0);
	}
	#endif
}
