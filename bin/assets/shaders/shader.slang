import "frameUniforms";



[[vk::push_constant]]
cbuffer PushConstants
{
    float4x4 model;
};

struct Set0
{
    UniformBufferObject ubo;
    Sampler2D albedoMap;
};

[[vk::binding(0, 0)]]
ParameterBlock<Set0> set0;

[[vk::binding(8, 0)]]
RaytracingAccelerationStructure tlas;

static const float PI = 3.14159265359;

// PDF of a light = 1 
public struct Reservoir
{
    uint  candidate = uint(-1);
    float Wsum      = 0.0f;
    float W         = 0.0f;
    float M         = 0.0f;
}

bool TraceShadowRay(float3 origin, float3 direction, float maxDistance)
{
    RayDesc ray;
    ray.Origin = origin;
    ray.Direction = direction;
    ray.TMin = 0.001f;
    ray.TMax = maxDistance;

    RayQuery<RAY_FLAG_ACCEPT_FIRST_HIT_AND_END_SEARCH> q;
    q.TraceRayInline(tlas, RAY_FLAG_ACCEPT_FIRST_HIT_AND_END_SEARCH, 0xFF, ray);
    q.Proceed();

    return q.CommittedStatus() == COMMITTED_TRIANGLE_HIT;
}

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
    o.position = mul(set0.ubo.proj, mul(set0.ubo.view, worldPos));
    o.normal   = normalize(mul((float3x3)model, normal));
    o.uv       = uv;

    return o;
}

[shader("fragment")]
float4 fragMain(VSOutput input) : SV_Target
{
    float3 N = normalize(input.normal);
    float3 V = normalize(set0.ubo.viewPos - input.worldPos);

    float3 baseColor = set0.ubo.albedo;
    if (set0.ubo.useAlbedoMap != 0)
    {
        baseColor = set0.albedoMap.Sample(input.uv).rgb;
    }

    if (set0.ubo.numLights == 0 || set0.ubo.lights[0].enabled == 0)
    {
        return float4(baseColor * 0.5, 1.0);
    }

    set0.ubo.LightData light = set0.ubo.lights[0];

    float3 Lvec = light.position - input.worldPos;
    float dist  = length(Lvec);
    float3 L    = normalize(Lvec);

    float NdotL = max(dot(N, L), 0.0);

    if (NdotL <= 0.0)
        return float4(baseColor * 0.1, 1.0);

    float3 rayOrigin = input.worldPos + N * 0.01;
    bool inShadow = TraceShadowRay(rayOrigin, L, dist);

    float shadow = inShadow ? 0.0 : 1.0;

    float attenuation = 1.0 / (dist * dist);
    float3 radiance = light.color * light.intensity * attenuation;

    float3 color = baseColor * radiance * NdotL * shadow;

    color += baseColor * 0.05;

    return float4(color, 1.0);
}
