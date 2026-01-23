Texture2D sceneTex : register(t0);
SamplerState samp : register(s0);

cbuffer BloomParam : register(b0)
{
    float threshold;
    float intensity;
    float2 padding;
};

struct PSInput
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD0;
};

float4 main(PSInput input) : SV_TARGET
{
    // SceneRT から色を取得
    float3 color = sceneTex.Sample(samp, input.uv).rgb;

    // 輝度を計算（人間の目に近い）
    float luminance = dot(color, float3(0.2126, 0.7152, 0.0722));

    // threshold 未満なら黒
    if (luminance < threshold)
    {
        return float4(0, 0, 0, 1);
    }

    // 明るい部分だけ抽出
    return float4(color * intensity, 1);
}