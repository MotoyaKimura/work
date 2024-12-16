#include "MenuPeraHeader.hlsli"

float4 PS(Output input) : SV_TARGET
{
    float4 start = startTex.Sample(smp2, float2((input.uv.x) * 5, (input.uv.y) * 10));
    start.rgb *= start.a;
    float4 restart = restartTex.Sample(smp2, float2((input.uv.x - 0.4) * 5, (input.uv.y - 0.4) * 10));
    restart.rgb *= restart.a;
    float4 title = titleTex.Sample(smp2, float2((input.uv.x - 0.4) * 5, (input.uv.y - 0.6) * 10));
    title.rgb *= title.a;
    float4 menu = menuTex.Sample(smp2, float2((input.uv.x - 0.1) * 10, (input.uv.y) * 10));
    menu.rgb *= menu.a;
    float4 back = backTex.Sample(smp2, float2((input.uv.x) * 10 * screenWidth / screenHeight, (input.uv.y) * 10));
    back.rgb *= back.a;
    float4 backGround = backGroundTex.Sample(smp2, input.uv);

    if (restart.a > 0.1f)
    {
        return float4((backGround + restart * restartHoverCnt * fade).rgb, backGround.a);
    }
    if (title.a > 0.1f)
    {
        return float4((backGround + title * titleHoverCnt * fade).rgb, backGround.a);
    }
    if (menu.a > 0.1f)
    {
        return menu;
    }
    if (back.a > 0.1f)
    {
        return back;
    }

    return backGround;
   
}