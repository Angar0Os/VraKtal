[[vk::binding(0, 0)]]
cbuffer UniformBufferObject
{
    float4x4 view;
    float4x4 proj;
    float4x4 lightSpaceMatrix;
    float4x4 viewProjInverse;
    float4   viewPos;

    struct LightData
    {
        float4 position;
        float4 color;
        float  intensity;
        int    enabled;
        int    type;
        float  lightRadius;
    };

    LightData lights[10];
    int      numLights;
    uint     frameCount;
};

[[vk::binding(1, 0)]] Sampler2D albedoMetal;
[[vk::binding(2, 0)]] Sampler2D normalRough;
[[vk::binding(3, 0)]] Sampler2D depthTex;
[[vk::binding(4, 0)]] RaytracingAccelerationStructure tlas;

struct VSOutput
{
    float4 position : SV_Position;
    float2 uv       : TEXCOORD0;
};

[shader("vertex")]
VSOutput vertMain(uint vertexID : SV_VertexID)
{
    VSOutput o;
    o.uv       = float2((vertexID << 1) & 2, vertexID & 2);
    o.position = float4(o.uv * 2.0 - 1.0, 0.0, 1.0);
    return o;
}

float3 ReconstructPosition(float2 uv, float depth)
{
    float4 clipPos  = float4(uv * 2.0 - 1.0, depth, 1.0);
    float4 worldPos = mul(viewProjInverse, clipPos);
    return worldPos.xyz / worldPos.w;
}

bool TraceShadow(float3 origin, float3 direction, float maxDist)
{
    RayDesc ray;
    ray.Origin    = origin;
    ray.Direction = direction;
    ray.TMin      = 0.001;
    ray.TMax      = maxDist - 0.001;

    RayQuery<RAY_FLAG_ACCEPT_FIRST_HIT_AND_END_SEARCH> query;
    query.TraceRayInline(tlas, RAY_FLAG_ACCEPT_FIRST_HIT_AND_END_SEARCH, 0xFF, ray);
    query.Proceed();

    return query.CommittedStatus() != COMMITTED_NOTHING;
}

float3 BlinnPhong(float3 worldPos, float3 N, float3 albedo, LightData light)
{
    float3 L          = normalize(light.position.xyz - worldPos);
    float3 V          = normalize(viewPos.xyz - worldPos);
    float3 H          = normalize(L + V);
    float  dist       = length(light.position.xyz - worldPos);
    float  atten      = light.intensity / (1.0 + dist * dist);

    float3 ambient    = albedo * 0.03;
    float3 diffuse    = albedo * light.color.rgb * max(dot(N, L), 0.0) * atten;
    float3 specular   = light.color.rgb * pow(max(dot(N, H), 0.0), 64.0) * atten * 0.5;

    bool   inShadow   = TraceShadow(worldPos + N * 0.005, L, dist);
    float  shadow     = inShadow ? 0.0 : 1.0;

    return ambient + (diffuse + specular) * shadow;
}

[shader("fragment")]
float4 fragMain(VSOutput input) : SV_Target
{
    float2 uv = input.uv;

    float  depth  = depthTex.Sample(uv).r;
    float4 albMet = albedoMetal.Sample(uv);
    float4 norRgh = normalRough.Sample(uv);

    float3 albedo   = albMet.rgb;
    float3 N        = normalize(norRgh.xyz * 2.0 - 1.0);
    float3 worldPos = ReconstructPosition(uv, depth);

    float3 color = float3(0.0, 0.0, 0.0);

    for (int i = 0; i < numLights; i++)
    {
        if (lights[i].enabled == 0) continue;
        color += BlinnPhong(worldPos, N, albedo, lights[i]);
    }

    if (numLights == 0)
        color = albedo * 0.1;

    return float4(color, 1.0);
}
