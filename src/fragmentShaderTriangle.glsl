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

layout(std430, binding = 1) buffer scenetriangleIndices {
    uvec3[] triangleIndices;
};

layout(std430, binding = 2) buffer sceneData {
    vec3[] data;
};

//camera information
uniform vec3 camera_position;


void main() {
	//vec3 texColor = texture(material.albedoTex, fPosition.xy).rgb;
	//color = vec4(texColor, 1.);
	if(triangleIndices[1][0]==3){
		color = vec4(1.0, 1.0, 0.0, 1.0);
	}

	color = vec4(fColor, 1.0);

	vec3 ray_direction = normalize(non_normalized_ray_direction);
	color = vec4(ray_direction, 1.0);

}
