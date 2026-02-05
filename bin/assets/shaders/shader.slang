[[vk::binding(0, 0)]]
cbuffer UniformBufferObject
{
    float4x4 view;
    float4x4 proj;
    float4x4 lightSpaceMatrix;
    float3 viewPos;
    struct LightData
    {
        float3 position;
        float3 color;
        float intensity;
        int enabled;
        int type;
        float lightRadius;
    };
    LightData lights[10];
    int numLights;
    uint frameCount;
};

[[vk::binding(1, 0)]]
Sampler2D depthTexture;

[[vk::push_constant]]
cbuffer PushConstants
{
    float4x4 model;
};

struct VSOutput
{
    float4 position : SV_Position;
    float3 worldPos : TEXCOORD0;
    float3 normal   : TEXCOORD1;
    float2 uv       : TEXCOORD2;
};

[shader("vertex")]
VSOutput vertMain(
    float3 position : POSITION,
    float3 normal   : NORMAL,
    float2 uv       : TEXCOORD0)
{
    VSOutput o;
    float4 worldPos = mul(model, float4(position, 1.0));
    o.worldPos = worldPos.xyz;
    o.position = mul(proj, mul(view, worldPos));
    o.normal = normalize(mul((float3x3)model, normal));
    o.uv = uv;
    return o;
}

[shader("fragment")]
float4 fragMain(VSOutput input) : SV_Target
{
    float3 baseColor = float3(0.8, 0.8, 0.8);
    
    if (numLights == 0 || lights[0].enabled == 0)
    {
        return float4(baseColor, 1.0);
    }
    
    LightData light = lights[0];
    float3 N = normalize(input.normal);
    float3 L = normalize(light.position - input.worldPos);
    float NdotL = max(dot(N, L), 0.0);
    
    float3 diffuse = baseColor * light.color * light.intensity * NdotL;
    
    return float4(diffuse, 1.0);
}