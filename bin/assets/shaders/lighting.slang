static const float PI = 3.14159f;

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
[[vk::binding(5, 0)]] Sampler2D envMapTex;

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

//GGX
//PBR

float D_GGX(float3 N, float3 H, float roughness)
{
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;
    float denom = NdotH2 * (a2 - 1.0) + 1.0;
    return a2 / (PI * denom * denom);
}

float G_SchlickGGX(float NdotV, float roughness)
{
    float r = roughness + 1.0;
    float k = (r * r) / 8.0;
    return NdotV / (NdotV * (1.0 - k) + k);
}

float G_Smith(float3 N, float3 V, float3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    return G_SchlickGGX(NdotV, roughness) * G_SchlickGGX(NdotL, roughness);
}

float3 F_Schlick(float cosTheta, float3 F0)
{
    return F0 + (1.0 - F0) * pow(saturate(1.0 - cosTheta), 5.0);
}

float3 CookTorrance(float3 worldPos, float3 N, float3 albedo,
                    float metalness, float roughness, LightData light)
{
    float3 V = normalize(viewPos.xyz - worldPos);
    float3 L = normalize(light.position.xyz - worldPos);
    float3 H = normalize(V + L);
    float dist = length(light.position.xyz - worldPos);

    float atten = light.intensity / (dist * dist);
    float3 radiance = light.color.rgb * atten;

    float3 F0 = lerp(float3(0.04, 0.04, 0.04), albedo, metalness);

    float D = D_GGX(N, H, roughness);
    float G = G_Smith(N, V, L, roughness);
    float3 F = F_Schlick(max(dot(H, V), 0.0), F0);

    float NdotL = max(dot(N, L), 0.0);
    float NdotV = max(dot(N, V), 0.0);
    float3 specular = (D * G * F) / max(4.0 * NdotV * NdotL, 0.0001);

    float3 kS = F;
    float3 kD = (float3(1.0, 1.0, 1.0) - kS) * (1.0 - metalness);
    float3 diffuse = kD * albedo / PI;

    bool inShadow = TraceShadow(worldPos + N * 0.005, L, dist);
    float shadow = inShadow ? 0.0 : 1.0;

    return (diffuse + specular) * radiance * NdotL * shadow;
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

float3 ACESFilm(float3 color, float exposure)
{
    float3 x = exp2(exposure) * color;

    float a = 2.51f;
    float b = 0.03f;
    float c = 2.43f;
    float d = 0.59f;
    float e = 0.14f;
    return saturate((x * (a * x + b)) / (x * (c * x + d) + e));
}

[shader("fragment")]
float4 fragMain(VSOutput input) : SV_Target
{
    float2 uv = input.uv;

    float3 color    = 0.0f.xxx;
    float  depth    = depthTex.Sample(uv).r;
    float3 worldPos = ReconstructPosition(uv, depth);

    if (depth < 1.0f)
    {
        float4 albMet = albedoMetal.Sample(uv);
        float4 norRgh = normalRough.Sample(uv);

        float3 albedo   = albMet.rgb;
        float3 N        = normalize(norRgh.xyz * 2.0 - 1.0);

        color += 0.065f * albedo;

        for (int i = 0; i < numLights; i++)
        {
            if (lights[i].enabled == 0) continue;
            color += CookTorrance(worldPos, N, albedo, albMet.w, norRgh.w, lights[i]);
        }

        if (numLights == 0)
            color = albedo * 0.1;
    }
    else
    {
        float3 dir = normalize(worldPos - viewPos.xyz);

        float theta = acos(dir.y);
        float phi = atan2(dir.z, dir.x);

        float V = theta / PI;
        float U = (phi + PI) / (2 * PI);

        color = envMapTex.Sample(float2(U, V)).xyz;
    }

    float3 tonemapped = ACESFilm(color, 1.);
    return float4(tonemapped, 1.0);
}
