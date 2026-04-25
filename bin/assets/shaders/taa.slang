[[vk_binding(0, 0)]] Sampler2D iblOutput;
[[vk_binding(1, 0)]] RWTexture2D outTaa;

[shader("compute")]
[numthreads(16, 16, 1)]
void cs_main(uint32_t threadId : SV_DispatchThreadID)
{
}
