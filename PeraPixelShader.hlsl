#include "PeraShaderHeader.hlsli"

float4 PS(Output input) : SV_TARGET
{
 //   float lightDep =  lightDepthTex.Sample(smp, input.uv);
	////return float4(lightDep, lightDep, lightDep, 1);
	//float dep = pow(depthTex.Sample(smp, input.uv), 20);
	//return float4(dep, dep, dep, 1);
 //   return normalTex.Sample(smp, input.uv);
	//return tex.Sample(smp, input.uv);
	//return float4(dep, dep, dep, 1);

	if(input.uv.x < 0.2 && input.uv.y < 0.2)
    {
        float dep = pow(depthTex.Sample(smp, input.uv * 5), 20);
        return float4(dep, dep, dep, 1);
    }
    else if(input.uv.x < 0.2 && input.uv.y < 0.4)
    {
        float lightDep = lightDepthTex.Sample(smp, input.uv * 5);
        return float4(lightDep, lightDep, lightDep, 1);
    }
    else if(input.uv.x < 0.2 && input.uv.y < 0.6)
    {
        return normalTex.Sample(smp, input.uv * 5);
    }
    else
    {
        return tex.Sample(smp, input.uv);
    }
}