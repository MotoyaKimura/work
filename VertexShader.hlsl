#include "ShaderHeader.hlsli"

matrix vertexSkinning(uint4 boneNo, float4 boneWeight, min16uint weightType)
{
    matrix ret = matrix(1, 0, 0, 0,
                        0, 1, 0, 0,
                        0, 0, 1, 0,
                        0, 0, 0, 1);
    
    switch (weightType)
    {
	    case 0:
        {
	    		matrix m0 = mul(bones[boneNo.x], invBones[boneNo.x]);
                ret = m0;
              
            break;
        }
        case 1:
        {
				matrix m0 = mul(bones[boneNo.x], invBones[boneNo.x]);
	    		matrix m1 = mul(bones[boneNo.y], invBones[boneNo.y]);
                
                ret = m0 * boneWeight.x + m1 * (1.0 - boneWeight.x);
               
            break;
        }
        case 2:
        {
				matrix m0 = mul(bones[boneNo.x], invBones[boneNo.x]);
                matrix m1 = mul(bones[boneNo.y], invBones[boneNo.y]);
                matrix m2 = mul(bones[boneNo.z], invBones[boneNo.z]);
                matrix m3 = mul(bones[boneNo.w], invBones[boneNo.w]);

                ret = m0 * boneWeight.x + m1 * boneWeight.y + m2 * boneWeight.z + m3 * boneWeight.w;
              
            break;
        }
        case 3:
            {
                break;
            }
        case 4:
            {
                break;
            }
    default:
        break;
    }
    return ret;
}

Output VS(
float4 pos : POSITION,
float4 normal : NORMAL,
float2 uv : TEXCOORD,
float3 morphPos : MORPHPOSITION,
float4 morphUV : MORPHUV,
uint4 boneNo : BONENO,
float4 boneWeight : WEIGHT,
min16uint weightType : WEIGHTTYPE,
uint instNo : SV_InstanceID
) 
{
	Output output;
    pos.xyz = pos.xyz + morphPos;
    matrix skinning = vertexSkinning(boneNo, boneWeight, weightType);
    output.pos = mul(skinning, pos);
	output.pos = mul(world, output.pos);
    output.pos = mul(shadowOffsetY, output.pos);
    if(instNo == 1)
    {
        output.pos = mul(shadow, output.pos);
    }
    output.pos = mul(invShadowOffsetY, output.pos);
    output.svpos = mul(mul(projection, view), output.pos);
    normal.w = 0;
    output.tpos = mul(lightCamera, output.pos);

    //skinning = matrix(skinning._11, skinning._12, skinning._13, 0,
    //                  skinning._21, skinning._22, skinning._23, 0,
    //                  skinning._31, skinning._32, skinning._33, 0,
    //                  0, 0, 0, 1);
    //output.normal = mul(skinning, normal);
    output.normal = mul(world, normal);
    output.vnormal = mul(view, output.normal);

    uv += morphUV.xy;
	output.uv = uv;
    output.ray = normalize(output.pos.xyz - mul(view, eye));
    output.instNo = instNo;
	return output;
}

Output rsmVS(
float4 pos : POSITION ,
float4 normal : NORMAL,
float2 uv : TEXCOORD,
float3 morphPos : MORPHPOSITION,
float4 morphUV : MORPHUV,
uint4 boneNo : BONENO,
float4 boneWeight : WEIGHT,
min16uint weightType : WEIGHTTYPE,
uint instNo : SV_InstanceID
) 
{
    Output output;
    matrix skinning = vertexSkinning(boneNo, boneWeight, weightType);
    pos.xyz = pos.xyz + morphPos;
    output.pos = mul(skinning, pos);
    output.pos = mul(world, output.pos);
    output.pos = mul(shadowOffsetY, output.pos);
    output.pos = mul(invShadowOffsetY, output.pos);
    output.svpos = mul(lightCamera, output.pos);
    normal.w = 0;
    output.tpos = mul(lightCamera, output.pos);
    //skinning = matrix(skinning._11, skinning._12, skinning._13, 0,
    //                  skinning._21, skinning._22, skinning._23, 0,
    //                  skinning._31, skinning._32, skinning._33, 0,
    //                  0, 0, 0, 1);
    //output.normal = mul(skinning, normal);
    output.normal = mul(world, normal);
    uv += morphUV.xy;
    output.uv = uv;
    output.instNo = instNo;
    return output;
}