[[vk::binding(0, 0)]]
cbuffer UniformBufferObject
{
    float4x4 view;
    float4x4 proj;
    float4x4 lightSpaceMatrix;
    float4x4 viewProjInverse;
    float4x4 prevViewProj;
    float4x4 prevModel;
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

[[vk::binding(1, 0)]] Sampler2D currentColor;
[[vk::binding(2, 0)]] Sampler2D historyColor;
[[vk::binding(3, 0)]] Sampler2D velocityTex;

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

float Luminance(float3 c)
{
    return dot(c, float3(0.2126, 0.7152, 0.0722));
}

float3 RGBToYCoCg(float3 c)
{
    return float3(
         0.25 * c.r + 0.5 * c.g + 0.25 * c.b,
        -0.25 * c.r + 0.5 * c.g - 0.25 * c.b,  
         0.5  * c.r              - 0.5  * c.b
    );
}

float3 YCoCgToRGB(float3 c)
{
    float Y  = c.x;
    float Co = c.z; 
    float Cg = c.y;
    return float3(
        Y - Cg + Co,
        Y + Cg,
        Y - Cg - Co
    );
}

float3 ClipHistory(float2 uv, float3 history)
{
    float3 colorMin = float3( 1e9,  1e9,  1e9);
    float3 colorMax = float3(-1e9, -1e9, -1e9);

    float2 texelSize;
    currentColor.GetDimensions(texelSize.x, texelSize.y);
    texelSize = 1.0 / texelSize;

    [unroll]
    for (int y = -1; y <= 1; ++y)
    [unroll]
    for (int x = -1; x <= 1; ++x)
    {
        float3 s = RGBToYCoCg(currentColor.Sample(uv + float2(x, y) * texelSize).rgb);
        colorMin = min(colorMin, s);
        colorMax = max(colorMax, s);
    }

    float3 histYCoCg = RGBToYCoCg(history);
    float3 clamped   = clamp(histYCoCg, colorMin, colorMax);
    return YCoCgToRGB(clamped);
}

[shader("fragment")]
float4 fragMain(VSOutput input) : SV_Target0
{
    float2 uv = input.uv;

    float2 velocity   = velocityTex.Sample(uv).rg;
    float2 historyUV  = uv - velocity;

    float4 current = currentColor.Sample(uv);

    bool outOfScreen = any(historyUV < 0.0) || any(historyUV > 1.0);

    float4 history;
    if (outOfScreen)
    {
        history = current;
        history.a = 1.0;
    }
    else
    {
        history.rgb = ClipHistory(uv, historyColor.Sample(historyUV).rgb);
        history.a   += 1.0;
    }

    float  speed       = length(velocity);
    float  blendFactor = lerp(0.1, 0.5, saturate(speed * 20.0));

    float3 result = lerp(history.rgb, current.rgb, blendFactor);

    return float4(result, 1.0);
}
