// ���̓f�[�^�`��
struct VS_IN
{
    float4 Pos : POSITION;
    float2 Texcoord : TEXCOORD0;
};

// �o�̓f�[�^�`��
struct VS_OUT
{
    float4 Pos : SV_POSITION;
    float2 Texcoord : TEXCOORD0;
};

// �萔�o�b�t�@
cbuffer cbMatrix : register(b0)
{
    matrix Screen;
    matrix Projection;
}

//--------------------------------------------------------------------------------------
// ���_�V�F�[�_
//--------------------------------------------------------------------------------------
VS_OUT vs_main(VS_IN input)
{
    VS_OUT output;
    
    output.Pos = mul(input.Pos, Screen);
    output.Pos = mul(output.Pos, Projection);
    output.Texcoord = input.Texcoord;
    
    return output;
}