#version 450

layout(location = 0) in vec3 inPos;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inUV;
layout(location = 3) in vec4 inTangent;

layout(push_constant) uniform PushConstants 
{
    mat4 model;
    mat4 view;
    mat4 proj;
} pc;

layout(location = 0) out vec3 fragNormal;
layout(location = 1) out vec2 fragUV;

void main()
{
    gl_Position = pc.proj * pc.view * pc.model * vec4(inPos, 1.0);
    fragNormal = mat3(pc.model) * inNormal;
    fragUV = inUV;
}
