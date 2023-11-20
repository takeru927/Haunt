// 入力データ形式
struct PS_IN
{
    float4 Pos : SV_POSITION;
    float3 Texcoord : TEXCOORD0;
};

// テクスチャ
TextureCube Texture : register(t0);
SamplerState samplerLinear : register(s0)
{
    Filter = MIN_MAG_MIP_LINEAR;
    AddressU = WRAP;
    AddressV = WRAP;
};

//--------------------------------------------------------------------------------------
// ピクセルシェーダ
//--------------------------------------------------------------------------------------
float4 ps_main(PS_IN input) : SV_TARGET
{
    return Texture.Sample(samplerLinear, input.Texcoord);
}