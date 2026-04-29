[[vk::binding(0, 0)]]
cbuffer UniformBufferObject
{
    float4x4 view;
    float4x4 proj;
    float4x4 lightSpaceMatrix;
    float4x4 viewProjInverse;
    float4x4 prevViewProj;
    float4x4 prevViewProjInverse;
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

[[vk::binding(1, 0)]] Sampler2D iblOutput;
[[vk::binding(2, 0)]] Sampler2D historyBuffer;
[[vk::binding(3, 0)]] Sampler2D velocityTexture;
[[vk::binding(4, 0)]] RWTexture2D<float4> outTaa;
[[vk::binding(5, 0)]] Sampler2D depthCurrent;
[[vk::binding(6, 0)]] Sampler2D depthHistory;

float3 ReconstructPosition(float2 uv, float depth, float4x4 vpInverse)
{
    float z = depth * 2.0f - 1.0f; 
    float4 clipPos  = float4(uv * 2.0f - 1.0f, z, 1.0f);
    float4 worldPos = mul(vpInverse, clipPos);
    return worldPos.xyz / worldPos.w;
}

float3 ReconstructNormal(float2 uv, float2 dims, float depth, float4x4 vpInverse, Sampler2D depthTex)
{
    float2 texelSize = 1.0f / dims;

    float depthR = depthTex.SampleLevel(uv + float2(texelSize.x, 0), 0).x;
    float depthU = depthTex.SampleLevel(uv + float2(0, texelSize.y), 0).x;

    float3 posC = ReconstructPosition(uv,                          depth,  vpInverse);
    float3 posR = ReconstructPosition(uv + float2(texelSize.x, 0), depthR, vpInverse);
    float3 posU = ReconstructPosition(uv + float2(0, texelSize.y), depthU, vpInverse);

    return normalize(cross(posR - posC, posU - posC));
}

[shader("compute")]
[numthreads(16, 16, 1)]
void cs_main(uint3 threadId: SV_DispatchThreadID)
{
    uint2 dims;
    outTaa.GetDimensions(dims.x, dims.y);

    if (any(threadId.xy >= dims))
        return;

    float2 uv = (float2(threadId.xy) + 0.5f) / float2(dims);

    float2 velocity = velocityTexture.SampleLevel(uv, 0).xy;
    float2 previousUV = uv - velocity;

    float3 currentColor = iblOutput.SampleLevel(uv, 0).xyz;

    float3 colorMin = float3( 1e9,  1e9,  1e9);
    float3 colorMax = float3(-1e9, -1e9, -1e9);

    for (int x = -1; x <= 1; x++)
    {
        for (int y = -1; y <= 1; y++)
        {
            float2 offset = float2(x, y) / float2(dims);
            float3 s = iblOutput.SampleLevel(uv + offset, 0).xyz;
            colorMin = min(colorMin, s);
            colorMax = max(colorMax, s);
        }
    }

    if (any(previousUV < 0.0f) || any(previousUV > 1.0f))
    {
        outTaa[threadId.xy] = float4(currentColor, 1.0f);
        return;
    }

    float depthCur  = depthCurrent.SampleLevel(uv,         0).x;
    float depthHist = depthHistory.SampleLevel(previousUV, 0).x;

    float3 worldCur  = ReconstructPosition(uv,         depthCur,  viewProjInverse);
    float3 worldPrev = ReconstructPosition(previousUV, depthHist, prevViewProjInverse);

    float linearCur = 1.0f / (depthCur + 0.0001f);
    float linearHist = 1.0f / (depthHist + 0.0001f);
    bool rejectDepth = abs(linearCur - linearHist) > 0.05f;

    float3 normalCur = ReconstructNormal(uv, float2(dims), depthCur, viewProjInverse, depthCurrent);
    float3 normalPrev = ReconstructNormal(previousUV, float2(dims), depthHist, prevViewProjInverse, depthHistory);

    bool rejectNormal = dot(normalCur, normalPrev) < 0.98f;

    if (rejectDepth || rejectNormal)
    {
        outTaa[threadId.xy] = float4(currentColor, 1.0f);
        return;
    }

    float3 historyColor = historyBuffer.SampleLevel(previousUV, 0).xyz;
    historyColor = clamp(historyColor, colorMin, colorMax);

    float speed = length(velocity);
    float motionBlend = lerp(0.75f, 0.5f, saturate(speed * 20.0f));

    float3 color = lerp(currentColor, historyColor, motionBlend);
    outTaa[threadId.xy] = float4(color, 1.0f);
}