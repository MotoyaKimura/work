#include "HowToPlayPeraHeader.hlsli"

float4 PS(Output input) : SV_TARGET
{
    float4 howToPlay = howToPlayTex.Sample(smp, input.uv);
    howToPlay.rgb *= howToPlay.a;

    return float4((howToPlay * fade).rgb, howToPlay.a);
}