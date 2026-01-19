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

layout(std430, binding = 0) buffer sceneVertexPositions {
    vec3[] vertexPositions;
};

layout(std430, binding = 1) buffer sceneVertexNormals{
	vec3[] vertexNormals;
};

layout(std430, binding = 2) buffer scenetriangleIndices {
    uvec3[] triangleIndices;
};



uniform int n_triangles;

//camera information
uniform vec3 camera_position;

const float epsilon = 0.01f;

bool float_is_null(float x){
	return x>-epsilon && x<epsilon;
}

bool floats_are_close(float x, float a, float threshold){
	float temp = x-a;
	return temp>-threshold && temp<threshold;
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

bool find_closest_intersected_triangle(vec3 ray_direction, out int triangleIndice, out float best_distance, out float best_u, out float best_v){
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

		if(ray_intersects_triangle(camera_position, ray_direction, v1, v2, v3, distance, u, v)){
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


void main() {
	//vec3 texColor = texture(material.albedoTex, fPosition.xy).rgb;
	//color = vec4(texColor, 1.);
	if(triangleIndices[1][0]==3){
		color = vec4(1.0, 1.0, 0.0, 1.0);
	}

	color = vec4(fColor, 1.0);

	vec3 ray_direction = normalize(non_normalized_ray_direction);
	color = vec4(ray_direction, 1.0);

	

	int triangleIndice;
	float distance;
	float u, v; //barycentrix coordinate
	if(find_closest_intersected_triangle(ray_direction, triangleIndice, distance, u, v)){
		vec3 fragmentColor;
		if(triangleIndice==1){
			fragmentColor = vec3(0.0, 1.0, 0.0);
		}
		else{
			fragmentColor = vec3(1.0, 0.0, 0.0);
		}

		vec3 normal = (1-u-v)*vertexNormals[triangleIndices[triangleIndice].x]
				+u*vertexNormals[triangleIndices[triangleIndice].y]
				+v*vertexNormals[triangleIndices[triangleIndice].z];
		normal = normalize(normal);
		vec3 position = camera_position + distance*ray_direction;

		//light
		vec3 v = -ray_direction;
		if(dot(v, normal)<0){
			normal = -normal; //we are looking at the other side of the triangle.
		}
		vec3 l = vec3(1.0, 1.0, 1.0);
		vec3 r = 2*(dot(l, normal))*normal - l;
		vec3 lightColor = normalize(vec3(1.0, 1.0, 1.0));
		float shininess = 2;

		float ambientRatio = 0.33;
		float diffuseRatio = 0.33;
		float specularRatio = 1 - ambientRatio - diffuseRatio;

		vec3 ambient  = ambientRatio  * fragmentColor;
		vec3 diffuse  = diffuseRatio  * fragmentColor * max(dot(normal, l), 0)*lightColor;
		vec3 specular = specularRatio * pow(max(dot(v, r), 0), shininess)*lightColor;

		color = vec4(ambient + diffuse + specular, 1.0);

	}
	else{
		//background
		color = vec4(0.0, 0.0, 1.0, 1.0);
		if(dot(ray_direction, normalize(vec3(1.0, 1.0, 1.0)))>0.99){
			color = vec4(1.0, 1.0, 0.5, 1.0);
		}
	}
}
