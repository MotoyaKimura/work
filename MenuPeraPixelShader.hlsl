#include "MenuPeraHeader.hlsli"

float4 PS(Output input) : SV_TARGET
{
    return startTex.Sample(smp, input.uv) * fade;
}