// 入力データ形式
struct VS_IN
{
    float4 Pos : POSITION;
    float2 Texcoord : TEXCOORD0;
};

// 出力データ形式
struct VS_OUT
{
    float4 Pos : SV_POSITION;
    float2 Texcoord : TEXCOORD0;
};

// 定数バッファ
cbuffer cbMatrix : register(b0)
{
    matrix Screen;
    matrix Projection;
}

//--------------------------------------------------------------------------------------
// 頂点シェーダ
//--------------------------------------------------------------------------------------
VS_OUT vs_main(VS_IN input)
{
    VS_OUT output;
    
    output.Pos = mul(input.Pos, Screen);
    output.Pos = mul(output.Pos, Projection);
    output.Texcoord = input.Texcoord;
    
    return output;
}