// ���̓f�[�^�`��
struct PS_IN
{
    float4 Pos : SV_POSITION;
    float2 Texcoord : TEXCOORD0;
};

// �e�N�X�`��
Texture2D Texture : register(t0);
SamplerState samplerLinear : register(s0)
{
    Filter = MIN_MAG_MIP_LINEAR;
    AddressU = CLAMP;
    AddressV = CLAMP;
};

//--------------------------------------------------------------------------------------
// �s�N�Z���V�F�[�_
//--------------------------------------------------------------------------------------
float4 ps_main(PS_IN input) : SV_Target
{
    return Texture.Sample(samplerLinear, input.Texcoord);
}