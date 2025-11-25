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

static const float PI = 3.14159265359;

uint pcg_hash(uint seed)
{
    uint state = seed * 747796405u + 2891336453u;
    uint word = ((state >> ((state >> 28u) + 4u)) ^ state) * 277803737u;
    return (word >> 22u) ^ word;
}

float random(inout uint seed)
{
    seed = pcg_hash(seed);
    return float(seed) / 4294967296.0;
}

float2 randomDisk(inout uint seed)
{
    float angle = random(seed) * 2.0 * PI;
    float radius = sqrt(random(seed));
    return float2(cos(angle), sin(angle)) * radius;
}

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

void BuildOrthonormalBasis(float3 direction, out float3 tangent, out float3 bitangent)
{
    float3 up = abs(direction.y) < 0.999 ? float3(0.0, 1.0, 0.0) : float3(1.0, 0.0, 0.0);
    tangent = normalize(cross(up, direction));
    bitangent = cross(direction, tangent);
}

bool TraceShadowRay(float3 origin, float3 direction, float maxDistance)
{
    RayDesc ray;
    ray.Origin = origin;
    ray.Direction = direction;
    ray.TMin = 0.001;
    ray.TMax = maxDistance;
    
    RayQuery<RAY_FLAG_ACCEPT_FIRST_HIT_AND_END_SEARCH> query;
    
    query.TraceRayInline(
        tlas,
        RAY_FLAG_ACCEPT_FIRST_HIT_AND_END_SEARCH,
        0xFF, 
        ray
    );
    
    query.Proceed();
    
    return query.CommittedStatus() == COMMITTED_TRIANGLE_HIT;
}

float TraceSoftShadowMonteCarlo(float3 origin, float3 lightCenter, float lightRadius, float3 normal, float NdotL, inout uint rngSeed)
{
    if (NdotL <= 0.0)
        return 0.0;
    
    float3 toLight = lightCenter - origin;
    float distance = length(toLight);
    float3 lightDir = toLight / distance;
    
    if (lightRadius <= 0.001)
    {
        float3 rayOrigin = origin + normal * 0.01;
        bool inShadow = TraceShadowRay(rayOrigin, lightDir, distance);
        return inShadow ? 0.0 : 1.0;
    }
    
    float3 tangent, bitangent;
    BuildOrthonormalBasis(lightDir, tangent, bitangent);
    
    float2 diskOffset = randomDisk(rngSeed) * lightRadius;
    float3 sampleOffset = tangent * diskOffset.x + bitangent * diskOffset.y;
    float3 samplePoint = lightCenter + sampleOffset;
    
    float3 toSample = samplePoint - origin;
    float sampleDistance = length(toSample);
    float3 sampleDir = normalize(toSample);
    
    float3 rayOrigin = origin + normal * 0.01;
    bool occluded = TraceShadowRay(rayOrigin, sampleDir, sampleDistance);
    
    return occluded ? 0.0 : 1.0;
}

struct VSOutput
{
    float4 position : SV_Position;
    float3 worldPos : POSITION0;
    float3 normal : NORMAL;
    float2 texCoord : TEXCOORD0;
    float4 shadowCoord : TEXCOORD1;
};

[shader("vertex")]
VSOutput vertMain(
    float3 position : POSITION,
    float3 color : COLOR,
    float2 texCoord : TEXCOORD0,
    float3 normal : NORMAL)
{
    VSOutput output;
    
    float4 worldPos = mul(model, float4(position, 1.0));
    output.worldPos = worldPos.xyz;
    output.position = mul(mul(proj, view), worldPos);
    output.normal = mul((float3x3)model, normal);
    output.texCoord = texCoord;
    output.shadowCoord = mul(lightSpaceMatrix, worldPos);
    
    return output;
}

[shader("fragment")]
float4 fragMain(VSOutput input) : SV_Target
{
    uint rngSeed = uint(input.position.x) * 1973u + 
                   uint(input.position.y) * 9277u + 
                   frameCount * 26699u;
    
    float3 albedoValue = (useAlbedoMap != 0) ? albedoMap.Sample(input.texCoord).rgb : albedo;
    float metallicValue = (useMetallicMap != 0) ? metallicMap.Sample(input.texCoord).r : metallic;
    float roughnessValue = (useRoughnessMap != 0) ? roughnessMap.Sample(input.texCoord).r : roughness;
    float aoValue = (useAOMap != 0) ? aoMap.Sample(input.texCoord).r : ao;
    float3 emissiveValue = (useEmissiveMap != 0) ? emissiveMap.Sample(input.texCoord).rgb : emissive;
    
    float3 N = GetNormalFromMap(input.texCoord, input.worldPos, input.normal);
    float3 V = normalize(viewPos - input.worldPos);
    
    float3 F0 = float3(0.04, 0.04, 0.04);
    F0 = lerp(F0, albedoValue, metallicValue);
    
    float3 Lo = float3(0.0, 0.0, 0.0);
    
    int activeLights = 0;
    
    for (int i = 0; i < numLights; i++)
    {
        if (lights[i].enabled == 0)
            continue;
        
        activeLights++;
        
        float3 L = normalize(lights[i].position - input.worldPos);
        float distance = length(lights[i].position - input.worldPos);
        
        float NdotL = max(dot(N, L), 0.0);
        if (NdotL <= 0.001) 
            continue;
        
        float visibility = TraceSoftShadowMonteCarlo(
            input.worldPos,
            lights[i].position,
            lights[i].lightRadius,
            N,
            NdotL,
            rngSeed
        );
        
        if (visibility <= 0.001)
            continue;
        
        float3 H = normalize(V + L);
        
        float attenuation = 1.0 / (distance * distance);
        float3 radiance = lights[i].color * lights[i].intensity * attenuation * visibility;
        
        float NDF = DistributionGGX(N, H, roughnessValue);
        float G = GeometrySmith(N, V, L, roughnessValue);
        float3 F = FresnelSchlick(max(dot(H, V), 0.0), F0);
        
        float3 kS = F;
        float3 kD = float3(1.0, 1.0, 1.0) - kS;
        kD *= 1.0 - metallicValue;
        
        float3 numerator = NDF * G * F;
        float denominator = 4.0 * max(dot(N, V), 0.0) * NdotL + 0.0001;
        float3 specular = numerator / denominator;
        
        Lo += (kD * albedoValue / PI + specular) * radiance * NdotL;
    }
    
    float3 ambient = float3(0.01, 0.01, 0.01) * albedoValue * aoValue;
    
    if (activeLights == 0)
    {
        float3 defaultLightDir = normalize(float3(1.0, 1.0, 1.0));
        float NdotL = max(dot(N, defaultLightDir), 0.0);
        Lo += albedoValue * NdotL * 0.8;
    }
    
    float3 color = Lo + ambient + emissiveValue;

    float exposure = 1.5;
    color = float3(1.0, 1.0, 1.0) - exp(-color * exposure);
    
    color = pow(color, float3(1.0 / 2.2, 1.0 / 2.2, 1.0 / 2.2));
    
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