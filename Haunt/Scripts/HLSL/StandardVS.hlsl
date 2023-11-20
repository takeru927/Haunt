// ���̓f�[�^�`��
struct VS_IN
{
	float4 Pos       : POSITION;
    float4 Normal    : NORMAL;
    float2 Texcoord  : TEXCOORD0;
};

// �o�̓f�[�^�`��
struct VS_OUT
{
	float4 Pos      : SV_POSITION;
	float4 Color    : COLOR0;
    float2 Texcoord : TEXCOORD0;
};

// �萔�o�b�t�@
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

//--------------------------------------------------------------------------------------
// ���_�V�F�[�_
//--------------------------------------------------------------------------------------
VS_OUT vs_main(VS_IN input)
{
	VS_OUT output;
    
    output.Pos = mul(input.Pos, World);
    output.Pos = mul(output.Pos, View);
    output.Pos = mul(output.Pos, Projection);

	// �@���x�N�g���̌v�Z
    float3 nor = float3(0.0f, 0.0f, 0.0f);
    nor = mul(input.Normal.xyz, (float3x3) World);
    nor = normalize(nor);
    
    output.Color = saturate(dot(nor, (float3)LightDir));
    output.Color = Ambient * LightAmbient + Diffuse * output.Color * LightDiffuse;
    
    output.Texcoord = input.Texcoord;
	
	return output;
}