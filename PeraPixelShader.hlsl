#include "PeraShaderHeader.hlsli"

float4 PS(Output input) : SV_TARGET
{
    //float s = ssaoTex.Sample(smp, (input.uv));
    //float4 texColor = tex.Sample(smp, input.uv);
    //return float4(texColor.rgb * s, texColor.a);

	if(input.uv.x < 0.2 && input.uv.y < 0.4 && input.uv.y > 0.2)
    {
        float dep = pow(depthTex.Sample(smp, input.uv * 5), 50);
        dep = 1 - dep;
        return float4(dep, dep, dep, 1);
    }
    else if (input.uv.x < 0.2 && input.uv.y < 0.6 && input.uv.y > 0.2)
    {
        return normalTex.Sample(smp, input.uv * 5);
    }
    else if (input.uv.x < 0.2 && input.uv.y < 0.8 && input.uv.y > 0.2)
    {
        float s = pow(ssaoTex.Sample(smp, (input.uv - float2(0, 0.6)) * 5), 10);
        return float4(s, s, s, 1);
    }
    else if (input.uv.x > 0.2 && input.uv.x < 0.4 && input.uv.y < 0.2)
    {
        float lightDep = lightDepthTex.Sample(smp, input.uv * 5);
        lightDep = 1 - lightDep;
        return float4(lightDep, lightDep, lightDep, 1);
    }
    else if (input.uv.x > 0.2 && input.uv.x < 0.6 && input.uv.y < 0.2)
    {
        return worldTex.Sample(smp, input.uv * 5);
    }
    else if (input.uv.x > 0.2 && input.uv.x < 0.8 && input.uv.y < 0.2)
    {
        return lightNormalTex.Sample(smp, input.uv * 5);
    }
    else if (input.uv.x > 0.2 && input.uv.x < 1.0 && input.uv.y < 0.2)
    {
        return indirectLightTex.Sample(smp, input.uv * 5);
    }
    else
    {
        float s = ssaoTex.Sample(smp, (input.uv));
        float4 texColor = tex.Sample(smp, input.uv);
    	return float4(texColor.rgb * s, texColor.a);
    }
}