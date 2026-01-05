#version 430
//#version 330 core	     // Minimal GL version support expected from the GPU


struct Material {

	sampler2D albedoTex;
};
uniform Material material;

in vec3 fPosition;
in vec3 fColor;

out vec4 color;	  // Shader output: the color response attached to this fragment


layout(std430, binding = 0) buffer sceneData {
    vec3[] data;
};


void main() {
	vec3 texColor = texture(material.albedoTex, fPosition.xy).rgb;
	color = vec4(texColor, 1.);
	color = vec4(data[0], 1.0);

}
