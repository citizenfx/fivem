GS_INPUT MAKE_FUNC(VS_, FUNC_NAME)(VS_INPUT _in) 
{
    GS_INPUT ret;
    ret.m_color = _in.m_color.abgr; // swizzle to correct endianness
    #if !defined(TRIANGLES)
        ret.m_color.a *= smoothstep(0.0, 1.0, _in.m_sizeUnused.x / kAntialiasing);
    #endif
    ret.m_size = max(_in.m_sizeUnused.x, kAntialiasing);
    ret.m_position = mul(uViewProjMatrix, float4(_in.m_position, 1.0));
    return ret;
}

#if defined(POINTS)
    // expand point -> triangle strip (quad)
    [maxvertexcount(4)]
    void MAKE_FUNC(GS_, FUNC_NAME)(point GS_INPUT _in[1], inout TriangleStream<VS_OUTPUT> out_)
    {
        VS_OUTPUT ret;
        
        float2 scale = 1.0 / uViewport * _in[0].m_size;
        ret.m_size  = _in[0].m_size;
        ret.m_color = _in[0].m_color;
        ret.m_edgeDistance = 0.0;
        
        ret.m_position = float4(_in[0].m_position.xy + float2(-1.0, -1.0) * scale * _in[0].m_position.w, _in[0].m_position.zw);
        ret.m_uv = float2(0.0, 0.0);
        out_.Append(ret);
        
        ret.m_position = float4(_in[0].m_position.xy + float2( 1.0, -1.0) * scale * _in[0].m_position.w, _in[0].m_position.zw);
        ret.m_uv = float2(1.0, 0.0);
        out_.Append(ret);
        
        ret.m_position = float4(_in[0].m_position.xy + float2(-1.0,  1.0) * scale * _in[0].m_position.w, _in[0].m_position.zw);
        ret.m_uv = float2(0.0, 1.0);
        out_.Append(ret);
        
        ret.m_position = float4(_in[0].m_position.xy + float2( 1.0,  1.0) * scale * _in[0].m_position.w, _in[0].m_position.zw);
        ret.m_uv = float2(1.0, 1.0);
        out_.Append(ret);
    }

#elif defined(LINES)
    // expand line -> triangle strip
    [maxvertexcount(4)]
    void MAKE_FUNC(GS_, FUNC_NAME)(line GS_INPUT _in[2], inout TriangleStream<VS_OUTPUT> out_)
    {
        float2 pos0 = _in[0].m_position.xy / _in[0].m_position.w;
        float2 pos1 = _in[1].m_position.xy / _in[1].m_position.w;
    
        float2 dir = pos0 - pos1;
        dir = normalize(float2(dir.x, dir.y * uViewport.y / uViewport.x)); // correct for aspect ratio
        float2 tng0 = float2(-dir.y, dir.x);
        float2 tng1 = tng0 * _in[1].m_size / uViewport;
        tng0 = tng0 * _in[0].m_size / uViewport;
    
        VS_OUTPUT ret;
        
        // line start
        ret.m_size = _in[0].m_size;
        ret.m_color = _in[0].m_color;
        ret.m_uv = float2(0.0, 0.0);
        ret.m_position = float4((pos0 - tng0) * _in[0].m_position.w, _in[0].m_position.zw); 
        ret.m_edgeDistance = -_in[0].m_size;
        out_.Append(ret);
        ret.m_position = float4((pos0 + tng0) * _in[0].m_position.w, _in[0].m_position.zw);
        ret.m_edgeDistance = _in[0].m_size;
        out_.Append(ret);
        
        // line end
        ret.m_size = _in[1].m_size;
        ret.m_color = _in[1].m_color;
        ret.m_uv = float2(1.0, 1.0);
        ret.m_position = float4((pos1 - tng1) * _in[1].m_position.w, _in[1].m_position.zw);
        ret.m_edgeDistance = -_in[1].m_size;
        out_.Append(ret);
        ret.m_position = float4((pos1 + tng1) * _in[1].m_position.w, _in[1].m_position.zw);
        ret.m_edgeDistance = _in[1].m_size;
        out_.Append(ret);
    }

#endif


float4 MAKE_FUNC(PS_, FUNC_NAME)(
#ifdef TRIANGLES
    GS_INPUT
#else
    VS_OUTPUT
#endif
    _in): SV_Target
{
    float4 ret = _in.m_color;
    
    #if   defined(LINES)
        float d = abs(_in.m_edgeDistance) / _in.m_size;
        d = smoothstep(1.0, 1.0 - (kAntialiasing / _in.m_size), d);
        ret.a *= d;
        
    #elif defined(POINTS)
        float d = length(_in.m_uv - float2(0.5, 0.5));
        d = smoothstep(0.5, 0.5 - (kAntialiasing / _in.m_size), d);
        ret.a *= d;
        
    #endif
    
    return ret;
}
