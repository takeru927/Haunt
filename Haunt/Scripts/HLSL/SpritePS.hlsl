// ���̓f�[�^�`��
struct PS_IN
{
    float4 Pos : SV_POSITION;
    float2 Texcoord : TEXCOORD0;
};

// �萔�o�b�t�@
cbuffer cbMaterial : register(b1)
{
    float4 Color;
    float  useTex;
    float3 dummy;
};

// �e�N�X�`��
Texture2D Texture : register(t0);
SamplerState samplerLinear : register(s0);

//--------------------------------------------------------------------------------------
// �s�N�Z���V�F�[�_
//--------------------------------------------------------------------------------------
float4 ps_main(PS_IN input) : SV_Target
{
    float4 output = Color;
    
    if(useTex)
        output *= Texture.Sample(samplerLinear, input.Texcoord);
    
    return output;
}