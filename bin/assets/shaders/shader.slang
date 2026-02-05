[[vk::binding(0, 0)]]
cbuffer UniformBufferObject
{
    float4x4 view;
    float4x4 proj;
    float4x4 lightSpaceMatrix;
    float4 viewPos;
    
    struct LightData
    {
        float4 position;
        float4 color;
        float intensity;
        int enabled;
        int type;
        float lightRadius;
    };
    LightData lights[10];
    int numLights;
    uint frameCount;
};

[[vk::binding(2, 0)]]  
RaytracingAccelerationStructure tlas;

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

bool TraceShadowRay(float3 origin, float3 direction, float maxDistance)
{
    RayDesc ray;
    ray.Origin = origin;
    ray.Direction = direction;
    ray.TMin = 0.001;
    ray.TMax = maxDistance;
    
    RayQuery<RAY_FLAG_NONE> query;
    query.TraceRayInline(
        tlas,
        RAY_FLAG_ACCEPT_FIRST_HIT_AND_END_SEARCH,
        0xFF,
        ray
    );
    
    query.Proceed();
    
    return query.CommittedStatus() != COMMITTED_NOTHING;
}

[shader("fragment")]
float4 fragMain(VSOutput input) : SV_Target
{
    float3 baseColor = float3(0.5, 0.5, 0.5);
    
    if (numLights == 0 || lights[0].enabled == 0)
    {
        return float4(baseColor * 0.1, 1.0);
    }
    
    LightData light = lights[0];
    float3 N = normalize(input.normal);
    float3 L = normalize(light.position.xyz - input.worldPos);
    float NdotL = max(dot(N, L), 0.0);
    
    float distanceToLight = length(light.position.xyz - input.worldPos);
    float3 shadowRayOrigin = input.worldPos + N * 0.01; 
    
    bool inShadow = TraceShadowRay(shadowRayOrigin, L, distanceToLight - 0.01);
    float shadowFactor = inShadow ? 0.2 : 1.0; 

    float3 ambient = baseColor * 0.1;
    float3 diffuse = baseColor * light.color.xyz * light.intensity * NdotL * shadowFactor;
    
    return float4(ambient + diffuse, 1.0);
}