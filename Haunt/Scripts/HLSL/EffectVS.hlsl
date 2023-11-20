// ���̓f�[�^�`��
struct VS_IN
{
    float4 Pos      : POSITION;
    float4 Color    : COLOR;
    float2 Texcoord : TEXCOORD0;
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
};

//--------------------------------------------------------------------------------------
// ���_�V�F�[�_
//--------------------------------------------------------------------------------------
VS_OUT vs_main(VS_IN input)
{
    VS_OUT output;
    
    output.Pos = mul(input.Pos, World);
    output.Pos = mul(output.Pos, View);
    output.Pos = mul(output.Pos, Projection);
    output.Color = input.Color;
    output.Texcoord = input.Texcoord;
    
    return output;
}