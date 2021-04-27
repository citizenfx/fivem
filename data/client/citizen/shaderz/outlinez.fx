cbuffer outline_buffer {
    float2 ScreenSize <string UIName = "Screen texel size";>;
    float Intensity <string UIName = "Intensity";>;
    int Width <string UIName = "Width";>;
    float4 Color <string UIName = "Colour";>;
    float GaussSamples[32] <string UIName = "Set by code.";>;
}

SamplerState MainTexSampler;
Texture2D MainTex;

SamplerState MaskTexSampler;
Texture2D MaskTex;

void vs(in float4 pos : POSITION, in float2 uv : TEXCOORD0, out float4 pos_ : SV_Position, out float2 uv_ : TEXCOORD0)
{
    pos_ = pos;
    uv_ = uv;
}

float CalcIntensity(float2 uv, float2 offset)
{
    float intensity = 0;

    // Accumulates horizontal or vertical blur intensity for the specified texture position.
    // Set offset = (tx, 0) for horizontal sampling and offset = (0, ty) for vertical.
    for (int k = -Width; k <= Width; ++k)
    {
        intensity += MainTex.Sample(MainTexSampler, uv + k * offset).a * GaussSamples[abs(k)];
    }

    return intensity;
}

float CalcIntensityV(float2 uv, float2 offset)
{
    float intensity = 0;

    // Accumulates horizontal or vertical blur intensity for the specified texture position.
    // Set offset = (tx, 0) for horizontal sampling and offset = (0, ty) for vertical.
    for (int k = -Width; k <= Width; ++k)
    {
        intensity += MainTex.Sample(MainTexSampler, uv + k * offset).r * GaussSamples[abs(k)];
    }

    return intensity;
}

float4 ps_h(in float4 pos : SV_Position, in float2 uv : TEXCOORD0) : SV_Target
{
    float intensity = CalcIntensity(uv, float2(ScreenSize.x, 0));
    return float4(intensity, intensity, intensity, 1.0);
}

float4 ps_v(in float4 pos : SV_Position, in float2 uv : TEXCOORD0) : SV_Target
{
    if (MaskTex.Sample(MaskTexSampler, uv).a > 0)
    {
        // TODO: Avoid discard/clip to improve performance on mobiles.
        discard;
    }

    float intensity = CalcIntensityV(uv, float2(0, ScreenSize.y));
    intensity = Intensity > 99 ? step(0.01, intensity) : intensity * Intensity;
    return float4(Color.rgb, saturate(Color.a * intensity));
}

technique10 h {
    pass p {
        SetVertexShader(CompileShader(vs_4_0, vs()));
        SetPixelShader(CompileShader(ps_4_0, ps_h()));
    }
}

technique10 v {
    pass p {
        SetVertexShader(CompileShader(vs_4_0, vs()));
        SetPixelShader(CompileShader(ps_4_0, ps_v()));
    }
}