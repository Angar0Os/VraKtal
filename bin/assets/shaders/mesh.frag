#version 450

layout(location = 0) in vec3 fragNormal;
layout(location = 1) in vec2 fragUV;

layout(location = 0) out vec4 outColor;

void main()
{
    vec3 N = normalize(fragNormal);
    float light = max(dot(N, normalize(vec3(0.5, 1.0, 0.3))), 0.0);
    outColor = vec4(light, light, light, 1.0);
}
