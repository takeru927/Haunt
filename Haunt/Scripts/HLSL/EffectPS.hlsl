// 入力データ形式
struct PS_IN
{
    float4 Pos      : SV_POSITION;
    float4 Color    : COLOR0;
    float2 Texcoord : TEXCOORD0;
};

// テクスチャ
Texture2D Texture : register(t0);
SamplerState samplerLinear : register(s0)
{
    Filter = MIN_MAG_MIP_LINEAR;
    AddressU = CLAMP;
    AddressV = CLAMP;
};

//--------------------------------------------------------------------------------------
// ピクセルシェーダ
//--------------------------------------------------------------------------------------
float4 ps_main(PS_IN input) : SV_Target
{
    float4 color = Texture.Sample(samplerLinear, input.Texcoord);
    
    input.Color.a = 1.0f;
    
    return input.Color * color;
}