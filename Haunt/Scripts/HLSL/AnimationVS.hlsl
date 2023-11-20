// 入力データ形式
struct VS_IN
{
	float4 Pos       : POSITION;
    float4 Normal    : NORMAL;
    float2 Texcoord  : TEXCOORD0;
    float4 BoneIndex : BONEINDEX;
    float4 Weight    : WEIGHT;
};

// 出力データ形式
struct VS_OUT
{
	float4 Pos      : SV_POSITION;
	float4 Color    : COLOR0;
    float2 Texcoord : TEXCOORD0;
};

// 定数バッファ
cbuffer cbMatrix : register(b0)
{
	matrix World;
	matrix View;
	matrix Projection;
}

cbuffer cbMaterial : register(b1)
{
    float4 Ambient;
    float4 Diffuse;
    float  UseTexture;
    float3 dummy;
}

cbuffer cbLighting : register(b2)
{
    float4 LightDir;
    float4 LightAmbient;
    float4 LightDiffuse;
}

cbuffer cbBones : register(b3)
{
    matrix Bones[128];
}

//--------------------------------------------------------------------------------------
// 頂点シェーダ
//--------------------------------------------------------------------------------------
VS_OUT vs_main(VS_IN input)
{
	VS_OUT output;
    
    matrix anim = matrix(0.0f, 0.0f, 0.0f, 0.0f,
                         0.0f, 0.0f, 0.0f, 0.0f,
                         0.0f, 0.0f, 0.0f, 0.0f,
                         0.0f, 0.0f, 0.0f, 0.0f);
    
    anim += mul(Bones[input.BoneIndex.x], input.Weight.x);
    anim += mul(Bones[input.BoneIndex.y], input.Weight.y);
    anim += mul(Bones[input.BoneIndex.z], input.Weight.z);
    anim += mul(Bones[input.BoneIndex.w], input.Weight.w);
    
    output.Pos = mul(input.Pos, anim);
    output.Pos = mul(output.Pos, World);
    output.Pos = mul(output.Pos, View);
    output.Pos = mul(output.Pos, Projection);

	// 法線ベクトルの計算
    float3 nor = float3(0.0f, 0.0f, 0.0f);
    
    nor = mul(input.Normal.xyz, (float3x3) anim);
    nor = mul(nor, (float3x3) World);
    nor = normalize(nor);
    
    output.Color = saturate(dot(nor, (float3)LightDir));
    output.Color = Ambient * LightAmbient + Diffuse * output.Color * LightDiffuse;
    
    output.Texcoord = input.Texcoord;
	
	return output;
}