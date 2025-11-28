[[vk::push_constant]]
cbuffer PushConstants
{
    float4x4 model;
};

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
        float _pad0;      
        float3 color;
        float intensity;
        int enabled;
        int type;
        float lightRadius;
        float _pad1;    
    };
    
    LightData lights[10];
    int numLights;
    
    float3 albedo;
    float metallic;
    float roughness;
    float ao;
    float3 emissive;
    
    int useAlbedoMap;
    int useNormalMap;
    int useMetallicMap;
    int useRoughnessMap;
    int useAOMap;
    int useEmissiveMap;
    
    uint frameCount;
};

[[vk::binding(1, 0)]]
Sampler2D albedoMap;

[[vk::binding(2, 0)]]
Sampler2D normalMap;

[[vk::binding(3, 0)]]
Sampler2D metallicMap;

[[vk::binding(4, 0)]]
Sampler2D roughnessMap;

[[vk::binding(5, 0)]]
Sampler2D aoMap;

[[vk::binding(6, 0)]]
Sampler2D emissiveMap;

[[vk::binding(8, 0)]]
RaytracingAccelerationStructure tlas;


bool TraceShadowRay(float3 origin, float3 direction, float maxDistance)
{
    RayDesc ray;
    ray.Origin = origin;
    ray.Direction = direction;
    ray.TMin = 0.001f;
    ray.TMax = maxDistance;
    
    RayQuery<RAY_FLAG_ACCEPT_FIRST_HIT_AND_END_SEARCH> query;
    
    query.TraceRayInline(tlas, RAY_FLAG_ACCEPT_FIRST_HIT_AND_END_SEARCH, 0xFF, ray);
    query.Proceed();
    return query.CommittedStatus() == COMMITTED_TRIANGLE_HIT;
}

struct VSOutput
{
    float4 position : SV_Position;
    float3 worldPos : POSITION0;
    float3 normal : NORMAL;
};

[shader("vertex")]
VSOutput vertMain(
    float3 position : POSITION,
    float3 normal : NORMAL)
{
    VSOutput output;
    
    float4 worldPos = mul(model, float4(position, 1.0));
    output.worldPos = worldPos.xyz;
    output.position = mul(mul(proj, view), worldPos);
    output.normal = normalize(mul((float3x3)model, normal));
    
    return output;
}

[shader("fragment")]
float4 fragMain(VSOutput input) : SV_Target
{
    float3 N = normalize(input.normal);
    
    float3 lightPos = float3(0.0, 5.0, 0.0); 
    
    for (int i = 0; i < numLights; i++)
    {
        if (lights[i].enabled != 0)
        {
            lightPos = lights[i].position;
            break;
        }
    }
    
    float3 lightDir = lightPos - input.worldPos;
    float distance = length(lightDir);
    lightDir = normalize(lightDir);
    
    float3 rayOrigin = input.worldPos + N * 0.01f;
    
    bool inShadow = TraceShadowRay(rayOrigin, lightDir, distance);
    
    float3 color = inShadow ? float3(0.0, 0.0, 0.0) : float3(1.0, 1.0, 1.0);
    
    return float4(color, 1.0);
}

struct ShadowVSOutput
{
    float4 position : SV_Position;
};

[shader("vertex")]
ShadowVSOutput shadowMain(float3 position : POSITION)
{
    ShadowVSOutput output;
    output.position = mul(lightSpaceMatrix, mul(model, float4(position, 1.0)));
    return output;
}