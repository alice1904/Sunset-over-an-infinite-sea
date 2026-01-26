#version 430
//#version 330 core	     // Minimal GL version support expected from the GPU


//struct Material {
//
//	sampler2D albedoTex;
//};
//uniform Material material;




in vec3 fPosition;
in vec3 fColor;

//ray information
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

layout(std430, binding = 1) buffer sceneVertexNormals{
	vec3[] vertexNormals;
};

layout(std430, binding = 2) buffer scenetriangleIndices {
    uvec3[] triangleIndices;
};

layout(std430, binding = 3) buffer sceneObjectProperties {
    ObjectProperties[] objectProperties;
};



uniform int n_triangles;
uniform int n_objects;
uniform vec3 lightDirection;

//camera information
uniform vec3 camera_position;

const float epsilon = 0.01f;

bool float_is_null(float x){
	return x>-epsilon && x<epsilon;
}


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
	return 0;
}


bool ray_intersects_triangle(vec3 ray_origin, vec3 ray_direction, vec3 v1, vec3 v2, vec3 v3, out float distance, out float u, out float v){
	vec3 edge1 = v2 - v1;
    vec3 edge2 = v3 - v1;
    vec3 ray_cross_edge2 = cross(ray_direction, edge2);
	float det = dot(ray_cross_edge2, edge1); 

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
	distance = inv_det * dot(edge2, v1_to_origin_cross_edge1);
	if(distance<epsilon){
		//behind the screen. 
		//TODO far and near with dot product with forward
		return false; 
	}

	return true;
}

bool find_closest_intersected_triangle(vec3 ray_origin, vec3 ray_direction, out int triangleIndice, out float best_distance, out float best_u, out float best_v){
	//return true if one triangle is intersected.
	//give the indices of the closest triangle.
	bool triangle_found = false;
	

	vec3 v1, v2, v3;
	float distance;
	float u, v;
	for(int i=0; i<n_triangles; i++){
		
		v1 = vertexPositions[triangleIndices[i][0]];
		v2 = vertexPositions[triangleIndices[i][1]];
		v3 = vertexPositions[triangleIndices[i][2]];

		if(ray_intersects_triangle(ray_origin, ray_direction, v1, v2, v3, distance, u, v)){
			if(!triangle_found || distance < best_distance){
				triangle_found = true;
				best_distance = distance;
				best_u = u;
				best_v = v;
				triangleIndice = i;
			}
		}
	}
	return triangle_found;
}

// vec3 getFragmentColor(int triangleIndice){
// 	if(triangleIndice==0){
// 		return vec3(0.0, 1.0, 0.0);
// 	}
// 	else{
// 		return vec3(1.0, 0.0, 0.0);
// 	}
// }

vec3 getRayColor(vec3 ray_origin, vec3 ray_direction){

	vec3 rayColor;

	int triangleIndice;
	float distance;
	float u, v; //barycentrix coordinate
	if(find_closest_intersected_triangle(ray_origin, ray_direction, triangleIndice, distance, u, v)){

		//color
		int objectIndex = findObjectIndex(triangleIndice);
		vec3 fragmentColor = objectProperties[objectIndex].color;
		float diffuseRatio = objectProperties[objectIndex].diffuseRatio;
		float specularRatio = objectProperties[objectIndex].specularRatio;
		float ambientRatio = 1 - specularRatio - diffuseRatio;
		vec3 ambient  = ambientRatio  * fragmentColor;
		

		//position and normal
		vec3 position = ray_origin + distance*ray_direction;
		// vec3 normal = (1-u-v)*vertexNormals[triangleIndices[triangleIndice].x]
		// 		+u*vertexNormals[triangleIndices[triangleIndice].y]
		// 		+v*vertexNormals[triangleIndices[triangleIndice].z];
		vec3 edge1 = vertexPositions[triangleIndices[triangleIndice].y] - vertexPositions[triangleIndices[triangleIndice].x];
		vec3 edge2 = vertexPositions[triangleIndices[triangleIndice].z] - vertexPositions[triangleIndices[triangleIndice].x];
		vec3 normal = cross(edge1, edge2);
		normal = normalize(normal);
		if(dot(ray_direction, normal)>0){
				normal = -normal; //we are looking at the other side of the triangle.
		}



		//shadow
		int _triangleIndice;
		float _distance;
		float _u;
		float _v;
		if(dot(normal, lightDirection)<0 || find_closest_intersected_triangle(position, lightDirection, _triangleIndice, _distance, _u, _v)){
			rayColor = ambient;
		}
		else{
			

			//light
			float shininess=2.0;
			vec3 lightColor = normalize(vec3(1.0, 1.0, 1.0));
			vec3 reflection = 2*(dot(lightDirection, normal))*normal - lightDirection;
			vec3 vue = -ray_direction;
			
			vec3 diffuse  = diffuseRatio  * fragmentColor * max(dot(normal, lightDirection), 0)*lightColor;
			vec3 specular = specularRatio * pow(max(dot(vue, reflection), 0), shininess)*lightColor;

			rayColor = ambient + diffuse + specular;
		}

		

		

	}
	else{
		//background
		float sun_closeness = dot(ray_direction, normalize(vec3(1.0, 1.0, 1.0)));
		if(sun_closeness>0.99){
			rayColor = vec3(1.0, 1.0, 0.5);
		}
		else{
			rayColor = vec3(0.0, 0.5, 1.0);
			// vec3 c = objectProperties[1].color;
			// color = vec4(c, 1.0);
		}

	}
	return rayColor;
}


void main() {
	vec3 ray_direction = normalize(non_normalized_ray_direction);

	color = vec4(getRayColor(camera_position, ray_direction), 1.0);

	

	
}
