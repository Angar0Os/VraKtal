module frameUniforms;

public struct UniformBufferObject
{
    public float4x4 view;
    public float4x4 proj;
    public float4x4 lightSpaceMatrix;
    public float3 viewPos;

    public struct LightData
    {
        public float3 position;
        public float3 color;
        public float intensity;
        public int enabled;
        public int type;
        public float lightRadius;
    };

    public LightData lights[10];
    public int numLights;

    public float3 albedo;
    public float metallic;
    public float roughness;
    public float ao;
    public float3 emissive;

    public int useAlbedoMap;
    public int useNormalMap;
    public int useMetallicMap;
    public int useRoughnessMap;
    public int useAOMap;
    public int useEmissiveMap;

    public uint frameCount