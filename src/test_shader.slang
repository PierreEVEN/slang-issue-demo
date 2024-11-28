
extern static const bool PARAM_NORMAL_TEXTURE = false;
extern static const bool PARAM_METAL_ROUGHNESS_TEXTURE = false;

SamplerState sSampler;
Texture2D mr_map;
Texture2D normal_map;

struct PushConsts
{
    float3 base;
};
[[vk::push_constant]] PushConsts pc;

struct FsOutput
{
    float3 albedo;
};

[shader("fragment")]
FsOutput fragment_main(float2 Uvs)
{
    FsOutput output;

    output.albedo = pc.base;

    if (PARAM_NORMAL_TEXTURE)
        output.albedo += (normal_map.Sample(sSampler, Uvs, 1).rgb - 0.5) * 2;
    if (PARAM_METAL_ROUGHNESS_TEXTURE)
        output.albedo = mr_map.Sample(sSampler, Uvs, 1).rgb;

    return output;
}