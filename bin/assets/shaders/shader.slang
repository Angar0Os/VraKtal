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
        float3 color;
        float intensity;
        int enabled;
        int type;
        float lightRadius;
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

static const float PI = 3.14159265359;

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
    float2 uv : TEXCOORD0;  
};

float DistributionGGX(float3 N, float3 H, float roughness)
{
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;
    
    float num = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;
    
    return num / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0;
    
    float num = NdotV;
    float denom = NdotV * (1.0 - k) + k;
    
    return num / denom;
}

float GeometrySmith(float3 N, float3 V, float3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);
    
    return ggx1 * ggx2;
}

float3 FresnelSchlick(float cosTheta, float3 F0)
{
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

float3 GetNormalFromMap(float2 texCoords, float3 worldPos, float3 normal)
{
    if (useNormalMap == 0)
        return normalize(normal);
    
    float3 tangentNormal = normalMap.Sample(texCoords).xyz * 2.0 - 1.0;
    
    float3 Q1 = ddx(worldPos);
    float3 Q2 = ddy(worldPos);
    float2 st1 = ddx(texCoords);
    float2 st2 = ddy(texCoords);
    
    float3 N = normalize(normal);
    float3 T = normalize(Q1 * st2.y - Q2 * st1.y);
    float3 B = -normalize(cross(N, T));
    float3x3 TBN = float3x3(T, B, N);
    
    return normalize(mul(tangentNormal, TBN));
}

[shader("vertex")]
VSOutput vertMain(
    float3 position : POSITION,
    float3 normal : NORMAL,
    float2 uv : TEXCOORD0) 
{
    VSOutput output;
    
    float4 worldPos = mul(model, float4(position, 1.0));
    output.worldPos = worldPos.xyz;
    
    output.position = mul(proj, mul(view, worldPos));
    
    output.normal = normalize(mul((float3x3)model, normal));
    
    output.uv = uv;
    
    return output;
}

public uint srandf(uint val0, uint val1, uint backoff = 16)
{
    uint v0 = val0, v1 = val1, s0 = 0;

    [unroll]
    for (uint n = 0; n < backoff; n++)
    {
        s0 += 0x9e3779b9;
        v0 += ((v1 << 4) + 0xa341316c) ^ (v1 + s0) ^ ((v1 >> 5) + 0xc8013ea4);
        v1 += ((v0 << 4) + 0xad90777d) ^ (v0 + s0) ^ ((v0 >> 5) + 0x7e95761e);
    }
    return v0;
}

public uint urand(inout uint s)
{
    s = (1664525u * s + 1013904223u);
    return s;
}

public float frand01(inout uint s)
{
    s = (1664525u * s + 1013904223u);
    return float(s & 0x00FFFFFF) / float(0x01000000);
}

[shader("fragment")]
float4 fragMain(VSOutput input) : SV_Target
{
    float3 N = normalize(input.normal);
    float3 V = normalize(viewPos - input.worldPos);
    
    uint seed = srandf(frameCount + asuint(input.position.x), frameCount + asuint(input.position.y) + 19937);

    float3 lightPos = float3(0.0, 5.0, 0.0); 
    float3 lightColor = float3(1.0, 1.0, 1.0);
    float lightIntensity = 1.0;
    
    for (int i = 0; i < numLights; i++)
    {
        if (lights[i].enabled != 0)
        {
            lightPos = lights[i].position + lights[i].lightRadius * (float3(
                frand01(seed), frand01(seed), frand01(seed)
            ) * 2.0f - 1.0f.xxx);
            lightColor = lights[i].color;
            lightIntensity = lights[i].intensity;

            break;
        }
    }
    
    float3 lightDir = lightPos - input.worldPos;
    float distance = length(lightDir);
    lightDir = normalize(lightDir);

    float3 rayOrigin = input.worldPos + N * 0.01f;
    
    bool inShadow = TraceShadowRay(rayOrigin, lightDir, distance);
    
    float NdotL = max(dot(N, lightDir), 0.0);
    
    float attenuation = 1.0 / (1.0 + 0.09 * distance + 0.032 * distance * distance);
    
    float3 baseColor = albedo;
    if (useAlbedoMap != 0)
    {
        baseColor = albedoMap.Sample(input.uv).rgb;
    }
    
    float metallicValue = metallic;
    if (useMetallicMap != 0)
    {
        metallicValue = metallicMap.Sample(input.uv).r;
    }
    
    float roughnessValue = roughness;
    if (useRoughnessMap != 0)
    {
        roughnessValue = roughnessMap.Sample(input.uv).r;
    }
    
    float aoValue = ao;
    if (useAOMap != 0)
    {
        aoValue = aoMap.Sample(input.uv).r;
    }
    
    float3 ambient = float3(0.03, 0.03, 0.03) * baseColor * aoValue;
    
    float3 diffuse = baseColor * lightColor * lightIntensity * NdotL * attenuation;
    
    float shadowFactor = inShadow ? 0.0 : 1.0;
    
    float3 color = ambient + diffuse * shadowFactor;
    
    if (useEmissiveMap != 0)
    {
        color += emissiveMap.Sample(input.uv).rgb;
    }
    else
    {
        color += emissive;
    }

    float3 emissiveValue = (useEmissiveMap != 0) ? emissiveMap.Sample(input.uv).rgb : emissive;

    float3 F0 = 0.04f.xxx;
    F0 = lerp(F0, baseColor, metallicValue);

    for (int i = 0; i < numLights; i++)
    {
        if (lights[i].enabled != 0)
        {
            float3 lp = lights[i].position - input.worldPos;
            float3 L = normalize(lp);
            float distance = length(lp);

            float NdotL = max(dot(N, L), 0.0);
            if (NdotL <= 0.001)
                continue;

            float3 H = normalize(V + L);
            float3 radiance = lights[i].color * lights[i].intensity;

            float NDF = DistributionGGX(N, H, roughnessValue);
            float G = GeometrySmith(N, V, L, roughnessValue);
            float3 F = FresnelSchlick(max(dot(H, V), 0.0), F0);

            float3 kS = F;
            float3 kD = float3(1.0, 1.0, 1.0) - saturate(kS);
            kD *= 1.0f - metallicValue;

            float3 numerator = NDF * G * F;
            float denominator = 4.0 * max(dot(N, V), 0.0) * NdotL + 0.0001;
            float3 specular = numerator / denominator;

            color += (kD * baseColor / PI + specular) * radiance * NdotL * attenuation * shadowFactor;
        }
    }

    if (numLights == 0)
    {
        float3 defaultLightDir = normalize(float3(1.0, 1.0, 1.0));
        float NdotL = max(dot(N, defaultLightDir), 0.0);
        color += baseColor * NdotL * 0.8;
    }

    float3 outputColor = color + ambient + emissiveValue;

    float exposure = 1.5;
    color = float3(1.0, 1.0, 1.0) - exp(-color * exposure);

    color = color / (color + 1.0f.xxx);
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
