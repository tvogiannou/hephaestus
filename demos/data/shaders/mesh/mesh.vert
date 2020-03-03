
#version 450

layout (location = 0) in vec3 inPos;
layout (location = 1) in vec3 inNormal;
layout (location = 2) in vec2 inUV;
layout (location = 3) in vec3 inColor;

layout (set = 0, binding = 0) uniform SceneUB
{
	mat4 projection;
	mat4 view;
	vec4 lightPos;
} sceneUB;

layout (set = 1, binding = 0) uniform MeshUB
{
	mat4 model;
} meshUB;

layout (location = 0) out vec3 outNormal;
layout (location = 1) out vec3 outColor;
layout (location = 2) out vec2 outUV;
layout (location = 3) out vec3 outViewVec;
layout (location = 4) out vec3 outLightVec;


out gl_PerVertex
{
	vec4 gl_Position;
};

void main() 
{
	outNormal = inNormal;
	outColor = inColor;
	outUV = inUV;
    
    // update camera position
    mat4 modelview = sceneUB.view * meshUB.model;
	gl_Position = sceneUB.projection * modelview * vec4(inPos.xyz, 1.0);
	
    // compute vectors for shading
	vec4 pos = modelview * vec4(inPos, 1.0);
	outNormal = mat3(modelview) * inNormal;
	vec3 lPos = mat3(modelview) * sceneUB.lightPos.xyz;
	outLightVec = lPos - pos.xyz;
	outViewVec = -pos.xyz;
}