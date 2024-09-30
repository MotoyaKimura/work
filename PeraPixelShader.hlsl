#include "PeraShaderHeader.hlsli"

float4 PS(Output input) : SV_TARGET
{

	if(input.uv.x < 0.2 && input.uv.y < 0.2)
    {
        float dep = pow(depthTex.Sample(smp, input.uv * 5), 30);
        dep = 1 - dep;
        return float4(dep, dep, dep, 1);
    }
    else if(input.uv.x < 0.2 && input.uv.y < 0.4)
    {
        float lightDep = lightDepthTex.Sample(smp, input.uv * 5);
        lightDep = 1 - lightDep;
        return float4(lightDep, lightDep, lightDep, 1);
    }
    else if(input.uv.x < 0.2 && input.uv.y < 0.6)
    {
        return normalTex.Sample(smp, input.uv * 5);
    }
    else if(input.uv.x < 0.2 && input.uv.y < 0.8)
    {
        float s = pow(ssaoTex.Sample(smp, (input.uv - float2(0, 0.6)) * 5), 10);
        return float4(s, s, s, 1);
    }
    else if(input.uv.x < 0.2 && input.uv.y < 1)
    {
        return worldTex.Sample(smp, input.uv * 5);
    }
    else if(input.uv.x < 0.4 && input.uv.y < 0.2)
    {
	    return lightNormalTex.Sample(smp, input.uv * 5);
    }
    else if(input.uv.x < 0.4 && input.uv.y < 0.4)
    {
	    return indirectLightTex.Sample(smp, input.uv * 5);
    }
    else
    {
        float dp = depthTex.Sample(smp, input.uv);
        float4 respos = mul(invprojection, float4(input.uv * float2(2.0f, -2.0f) + float2(-1.0f, 1.0f), dp, 1.0f));
        respos.xyz = respos.xyz / respos.w;
        float3 resp = normalize(respos.xyz);
        //return float4(resp, 1);
        float s = ssaoTex.Sample(smp, (input.uv));
        float4 texColor = tex.Sample(smp, input.uv);
       // return float4(s, s, s, 1);
        return float4(texColor.rgb * s, texColor.a);
    }
}