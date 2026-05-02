[[vk::binding(0, 0)]] Sampler2D inputTex;
[[vk::binding(1, 0)]] RWTexture2D<float4> outputTex;

float3 ACESFilm(float3 color, float exposure)
{
    float3 x = exp2(exposure) * color;
    float a = 2.51f, b = 0.03f, c = 2.43f, d = 0.59f, e = 0.14f;
    return saturate((x * (a * x + b)) / (x * (c * x + d) + e));
}

[shader("compute")]
[numthreads(16, 16, 1)]
void cs_main(uint3 threadId : SV_DispatchThreadID)
{
    uint2 dims;
    outputTex.GetDimensions(dims.x, dims.y);

    if (any(threadId.xy >= dims))
        return;

    float2 uv = (float2(threadId.xy) + 0.5f) / float2(dims);

    float3 hdr = inputTex.SampleLevel(uv, 0).rgb;
    float3 ldr = ACESFilm(hdr, 1.0f);

    outputTex[threadId.xy] = float4(ldr, 1.0f);
}