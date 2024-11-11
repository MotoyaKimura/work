#include "SSAOShaderHeader.hlsli"
float random(float2 uv)
{
    return frac(sin(dot(uv, float2(12.9898, 78.233))) * 43758.5453);
}

float ssaoPS(Output input) : SV_TARGET
{
   
    float dp = depthTex.Sample(smp, input.uv);
    float w, h, miplevels;
    depthTex.GetDimensions(0, w, h, miplevels);
    float dx = 1.0f / w;
    float dy = 1.0f / h;

    //å≥ÇÃç¿ïWÇïúå≥Ç∑ÇÈ
    float4 respos = mul(invprojection, float4(input.uv * float2(2.0f, -2.0f) + float2(-1.0f, 1.0f), dp, 1.0f));

    respos.xyz = respos.xyz / respos.w;
  //return respos.z;
    float div = 0.0f;
    float ao = 0.0f;
    float3 norm = normalize((normalTex.Sample(smp, input.uv).xyz * 2) - 1);
    const int trycnt = 256;
    const float radius = 0.5f;

    if (dp < 1.0f)
    {
        for (int i = 0; i < trycnt; ++i)
        {
            float rnd1 = random(float2(i * dx, i * dy)) * 2.0f - 1.0f;
            float rnd2 = random(float2(rnd1, i * dy)) * 2.0f - 1.0f;
            float rnd3 = random(float2(rnd2, rnd1)) * 2.0f - 1.0f;
            float3 omega = normalize(float3(rnd1, rnd2, rnd3));
            
            omega = normalize(omega);

            //óêêîÇÃåãâ ñ@ê¸ÇÃîΩëŒë§Ç…å¸Ç¢ÇƒÇ¢ÇΩÇÁîΩì]Ç∑ÇÈ
            float dt = dot(norm, omega);
            float sgn = sign(dt);
            omega *= sign(dt);
            
            //åãâ ÇÃç¿ïWÇçƒÇ—éÀâeïœä∑Ç∑ÇÈ
            float4 rpos = mul(projection, mul(view, float4(respos.xyz + omega * radius, 1.0f)));
            rpos.xyz = rpos.xyz / rpos.w;
            dt *= sgn;
            div += dt;
            
            //åvéZåãâ Ç™åªç›ÇÃèÍèäÇÃê[ìxÇÊÇËâúÇ…ì¸Ç¡ÇƒÇ¢ÇÈÇ»ÇÁ
            //é’ï¡Ç≥ÇÍÇƒÇ¢ÇÈÇÃÇ≈â¡éZ
            ao += step(depthTex.Sample(smp, (rpos.xy + float2(1.0f, -1.0f)) * float2(0.5f, -0.5f)), rpos.z) * dt;
        }
        ao /= div;

    }
    return 1.0f - ao;
}
