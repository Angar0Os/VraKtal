static const float PI = 3.14159265359f;

[[vk::binding(0, 0)]]
cbuffer UniformBufferObject
{
    float4x4 view;
    float4x4 proj;
    float4x4 viewProjInverse;
    float4x4 prevViewProj;
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
    int       numLights;
    uint      frameCount;
};

[[vk::binding(1, 0)]] Sampler2D albedoMetal;   
[[vk::binding(2, 0)]] Sampler2D normalRough;   
[[vk::binding(3, 0)]] Sampler2D depthTex;
[[vk::binding(4, 0)]] RaytracingAccelerationStructure tlas;
[[vk::binding(5, 0)]] Sampler2D envMapTex;       
[[vk::binding(6, 0)]] Sampler2D iblTex;          
                                                 


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
    o.position = float4(o.uv * 2.0f - 1.0f, 0.0f, 1.0f);
    return o;
}

float3 ReconstructPosition(float2 uv, float depth)
{
    float4 clipPos  = float4(uv * 2.0f - 1.0f, depth, 1.0f);
    float4 worldPos = mul(viewProjInverse, clipPos);
    return worldPos.xyz / worldPos.w;
}

bool TraceShadow(float3 origin, float3 direction, float maxDist)
{
    RayDesc ray;
    ray.Origin    = origin;
    ray.Direction = direction;
    ray.TMin      = 0.001f;
    ray.TMax      = maxDist - 0.001f;

    RayQuery<RAY_FLAG_ACCEPT_FIRST_HIT_AND_END_SEARCH> q;
    q.TraceRayInline(tlas, RAY_FLAG_ACCEPT_FIRST_HIT_AND_END_SEARCH, 0xFF, ray);
    q.Proceed();
    return q.CommittedStatus() != COMMITTED_NOTHING;
}

float D_GGX(float3 N, float3 H, float roughness)
{
    float a      = roughness * roughness;
    float a2     = a * a;
    float NdotH  = max(dot(N, H), 0.0f);
    float NdotH2 = NdotH * NdotH;
    float denom  = NdotH2 * (a2 - 1.0f) + 1.0f;
    return a2 / (PI * denom * denom);
}

float G_SchlickGGX(float NdotV, float roughness)
{
    float r = roughness + 1.0f;
    float k = (r * r) / 8.0f;
    return NdotV / (NdotV * (1.0f - k) + k);
}

float G_Smith(float3 N, float3 V, float3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0f);
    float NdotL = max(dot(N, L), 0.0f);
    return G_SchlickGGX(NdotV, roughness) * G_SchlickGGX(NdotL, roughness);
}

float3 F_Schlick(float cosTheta, float3 F0)
{
    return F0 + (1.0f - F0) * pow(saturate(1.0f - cosTheta), 5.0f);
}

float3 CookTorrance(float3 worldPos, float3 N, float3 albedo,
                    float metalness, float roughness, LightData light)
{
    float3 V    = normalize(viewPos.xyz - worldPos);
    float3 L    = normalize(light.position.xyz - worldPos);
    float3 H    = normalize(V + L);
    float  dist = length(light.position.xyz - worldPos);

    float  atten    = light.intensity / (dist * dist);
    float3 radiance = light.color.rgb * atten;

    float3 F0 = lerp(float3(0.04f, 0.04f, 0.04f), albedo, metalness);

    float  D        = D_GGX(N, H, roughness);
    float  G        = G_Smith(N, V, L, roughness);
    float3 F        = F_Schlick(max(dot(H, V), 0.0f), F0);

    float  NdotL    = max(dot(N, L), 0.0f);
    float  NdotV    = max(dot(N, V), 0.0f);
    float3 specular = (D * G * F) / max(4.0f * NdotV * NdotL, 0.0001f);

    float3 kS      = F;
    float3 kD      = (float3(1.0f, 1.0f, 1.0f) - kS) * (1.0f - metalness);
    float3 diffuse = kD * albedo / PI;

    bool   inShadow = TraceShadow(worldPos + N * 0.005f, L, dist);
    float  shadow   = inShadow ? 0.0f : 1.0f;

    return (diffuse + specular) * radiance * NdotL * shadow;
}

float3 ACESFilm(float3 color, float exposure)
{
    float3 x = exp2(exposure) * color;
    float a = 2.51f, b = 0.03f, c = 2.43f, d = 0.59f, e = 0.14f;
    return saturate((x * (a * x + b)) / (x * (c * x + d) + e));
}

float4 SampleEquirectangular(Sampler2D s, float3 dir)
{
    float theta = acos(clamp(dir.y, -1.0f, 1.0f));
    float phi   = atan2(dir.z, dir.x);
    float u     = (phi + PI) / (2.0f * PI);
    float v     = theta / PI;
    return s.Sample(float2(u, v));
}

[shader("fragment")]
float4 fragMain(VSOutput input) : SV_Target
{
    float2 uv    = input.uv;
    float  depth = depthTex.Sample(uv).r;

    if (depth >= 1.0f)
    {
        float3 worldPos = ReconstructPosition(uv, depth);
        float3 dir      = normalize(worldPos - viewPos.xyz);
        float3 sky      = SampleEquirectangular(envMapTex, dir).rgb;
        return float4(ACESFilm(sky, 1.0f), 1.0f);
    }

    float3 worldPos = ReconstructPosition(uv, depth);

    float4 albMet  = albedoMetal.Sample(uv);
    float4 norRgh  = normalRough.Sample(uv);

    float3 albedo    = albMet.rgb;
    float  metalness = albMet.a;
    float  roughness = norRgh.a;
    float3 N         = normalize(norRgh.xyz * 2.0f - 1.0f);

    float3 iblColor = iblTex.Sample(uv).rgb;

    float3 directColor = 0.0f.xxx;
    for (int i = 0; i < numLights; i++)
    {
        if (lights[i].enabled == 0) continue;
        directColor += CookTorrance(worldPos, N, albedo, metalness, roughness, lights[i]);
    }

    float3 color = iblColor + directColor;

    return float4(ACESFilm(color, 1.0f), 1.0f);
}
