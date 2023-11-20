// 入力データ形式
struct PS_IN
{
	float4 Pos		: SV_POSITION;
	float4 Color	: COLOR0;
    float2 Texcoord : TEXCOORD0;
};
// 定数バッファ
cbuffer cbMaterial : register(b1)
{
    float4 Ambient;
    float4 Diffuse;
    float  UseTexture;
    float3 dummy;
}

// テクスチャ
Texture2D Texture : register(t0);
SamplerState samplerLinear : register(s0)
{
    Filter = MIN_MAG_MIP_LINEAR;
    AddressU = WRAP;
    AddressV = WRAP;
};

//--------------------------------------------------------------------------------------
// ピクセルシェーダ
//--------------------------------------------------------------------------------------
float4 ps_main(PS_IN input) : SV_Target
{
    float4 outColor;
    
    if (UseTexture)
        outColor = input.Color * Texture.Sample(samplerLinear, input.Texcoord);
    
    else 
        outColor = input.Color;
        
	return outColor;
}