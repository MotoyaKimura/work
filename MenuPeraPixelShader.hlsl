#include "MenuPeraHeader.hlsli"

float4 PS(Output input) : SV_TARGET
{
    float4 start = startTex.Sample(smp, float2((input.uv.x) * 5, (input.uv.y) * 10));
    start.rgb *= start.a;
    float4 restart = restartTex.Sample(smp, float2((input.uv.x) * 5, (input.uv.y) * 10));
    restart.rgb *= restart.a;
    float4 title = titleTex.Sample(smp, float2((input.uv.x) * 5, (input.uv.y) * 10));
    title.rgb *= title.a;
    float4 backGround = backGroundTex.Sample(smp, input.uv);
    float4 menu = menuTex.Sample(smp, float2((input.uv.x) * 10, (input.uv.y) * 10));
    menu.rgb *= menu.a;

    if(input.uv.x > 0.1 && input.uv.x < 0.2 && input.uv.x > 0.0 && input.uv.y < 0.1)
    {
	    return float4((backGround + menu).rgb * fade, backGround.a);
    }
    if (input.uv.x > 0.4 && input.uv.x < 0.6 && input.uv.y > 0.4 && input.uv.y < 0.5)
    {
        return float4((backGround + restart).rgb * fade, backGround.a);
    }

    if (input.uv.x > 0.4 && input.uv.x < 0.6 && input.uv.y > 0.6 && input.uv.y < 0.7)
    {
        return float4((backGround + title).rgb * fade, backGround.a);
    }
    return float4((backGround * fade).rgb, backGround.a);
}