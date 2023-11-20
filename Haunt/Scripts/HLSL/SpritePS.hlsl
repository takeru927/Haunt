// 入力データ形式
struct PS_IN
{
    float4 Pos : SV_POSITION;
    float2 Texcoord : TEXCOORD0;
};

// 定数バッファ
cbuffer cbMaterial : register(b1)
{
    float4 Color;
    float  useTex;
    float3 dummy;
};

// テクスチャ
Texture2D Texture : register(t0);
SamplerState samplerLinear : register(s0);

//--------------------------------------------------------------------------------------
// ピクセルシェーダ
//--------------------------------------------------------------------------------------
float4 ps_main(PS_IN input) : SV_Target
{
    float4 output = Color;
    
    if(useTex)
        output *= Texture.Sample(samplerLinear, input.Texcoord);
    
    return output;
}