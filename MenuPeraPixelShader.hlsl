#include "MenuPeraHeader.hlsli"

float4 PS(Output input) : SV_TARGET
{
    float4 start = startTex.Sample(smp, input.uv);
    start.rgb *= start.a;
    return float4((start * fade).rgb, 1.0f);
}