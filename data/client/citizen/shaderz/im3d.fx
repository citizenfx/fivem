struct VS_INPUT
{
    float3 m_position     : POSITION;
    float3 m_sizeUnused   : NORMAL;
    float4 m_color        : COLOR;
};

struct GS_INPUT
{
	linear        float4 m_position     : SV_POSITION;
	linear        float4 m_color        : COLOR;
	noperspective float  m_size         : SIZE;
};

struct VS_OUTPUT
{
	linear        float4 m_position     : SV_POSITION;
	linear        float4 m_color        : COLOR;
	linear        float2 m_uv           : TEXCOORD;
	noperspective float  m_size         : SIZE;
	noperspective float  m_edgeDistance : EDGE_DISTANCE;
};

cbuffer cbContextData : register(b0)
{
    float4x4 uViewProjMatrix;
    float2   uViewport;
};

#define kAntialiasing 2.0

#define MAKE_FUNC(x, y) x##y

#undef FUNC_NAME
#define FUNC_NAME Triangles
#define TRIANGLES

#include "im3d.fxh"

#undef TRIANGLES

#undef FUNC_NAME
#define FUNC_NAME Points
#define POINTS

#include "im3d.fxh"

#undef POINTS

#undef FUNC_NAME
#define FUNC_NAME Lines
#define LINES

#include "im3d.fxh"

#undef LINES

technique10 lines {
    pass p0 {
        SetVertexShader(CompileShader(vs_4_0, VS_Lines()));
        SetGeometryShader(CompileShader(gs_4_0, GS_Lines()));
        SetPixelShader(CompileShader(ps_4_0, PS_Lines()));
    }
}

technique10 points {
    pass p0 {
        SetVertexShader(CompileShader(vs_4_0, VS_Points()));
        SetGeometryShader(CompileShader(gs_4_0, GS_Points()));
        SetPixelShader(CompileShader(ps_4_0, PS_Points()));
    }
}

technique10 triangles {
    pass p0 {
        SetVertexShader(CompileShader(vs_4_0, VS_Triangles()));
        SetPixelShader(CompileShader(ps_4_0, PS_Triangles()));
    }
}