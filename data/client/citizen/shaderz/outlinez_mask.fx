cbuffer outline_buffer {
    float2 ScreenSize <string UIName = "Screen texel size";>;
    float4 Color <string UIName = "Colour";>;
}

SamplerState MaskTexSampler;
Texture2D MaskTex;

void vs(in float4 pos : POSITION, in float2 uv : TEXCOORD0, out float4 pos_ : SV_Position, out float2 uv_ : TEXCOORD0)
{
    pos_ = pos;
    uv_ = uv;
}

float4 ps(in float4 pos : SV_Position, in float2 uv : TEXCOORD0) : SV_Target
{
    if (MaskTex.Sample(MaskTexSampler, uv).a > 0)
    {
        return float4(.0f, .0f, .0f, .0f);
    }

    return Color;
}

technique10 v {
    pass p {
        SetVertexShader(CompileShader(vs_4_0, vs()));
        SetPixelShader(CompileShader(ps_4_0, ps()));
    }
}