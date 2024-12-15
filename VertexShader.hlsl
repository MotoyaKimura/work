#include "ShaderHeader.hlsli"

//クォータニオンの線形補間
float4 QuaternionSlerp(float4 q0, float4 q1, float t)
{
    float4 q = float4(0, 0, 0, 1);

    float d = dot(q0, q1);
    if (d < 0)
    {
        q1 = -q1;
        d = -d;
    }
    if (d > 0.9995)
    {
        q = lerp(q0, q1, t);
        return q;
    }
    float theta = acos(d);
    float theta1 = theta * t;

    float4 q1a = q1 - q0 * d;
    q1a = normalize(q1a);
    return  q0 * cos(theta1) + q1a * sin(theta1);
}

//クォータニオンから行列に変換
matrix QuaternionToMatrix(float4 quat)
{
	float x = quat.x;
    float y = quat.y;
    float z = quat.z;
    float w = quat.w;

    float xx = x * x;
    float yy = y * y;
    float zz = z * z;
    float xy = x * y;
    float xz = x * z;
    float yz = y * z;
    float wx = w * x;
    float wy = w * y;
    float wz = w * z;

    matrix rotationMatrix = matrix(1 - 2 * (yy + zz), 2 * (xy - wz), 2 * (xz + wy), 0,
                                    2 * (xy + wz), 1 - 2 * (xx + zz), 2 * (yz - wx), 0,
                                    2 * (xz - wy), 2 * (yz + wx), 1 - 2 * (xx + yy), 0,
                                    0, 0, 0, 1);
    return rotationMatrix;
}

//頂点スキニング
float4 vertexSkinning(float4 pos, uint4 boneNo, float4 boneWeight, min16uint weightType, float3 sdefC, float3 sdefR0, float3 sdefR1, float4 q0, float4 q1)
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
	    		return mul(ret, pos);
            break;
        }
        case 1:
        {
				matrix m0 = mul(bones[boneNo.x], invBones[boneNo.x]);
	    		matrix m1 = mul(bones[boneNo.y], invBones[boneNo.y]);
                
                ret = m0 * boneWeight.x + m1 * (1.0 - boneWeight.x);
                return mul(ret, pos);
            break;
        }
        case 2:
        {
				matrix m0 = mul(bones[boneNo.x], invBones[boneNo.x]);
                matrix m1 = mul(bones[boneNo.y], invBones[boneNo.y]);
                matrix m2 = mul(bones[boneNo.z], invBones[boneNo.z]);
                matrix m3 = mul(bones[boneNo.w], invBones[boneNo.w]);

                ret = m0 * boneWeight.x + m1 * boneWeight.y + m2 * boneWeight.z + m3 * boneWeight.w;
                return mul(ret, pos);
            break;
        }
        case 3:
            {
                matrix m0 = mul(bones[boneNo.x], invBones[boneNo.x]);
                ret = m0;
                return mul(ret, pos);
                break;
            }
        case 4:
            {
				float w0 = boneWeight.x;
                float w1 = 1.0f - w0;

				float3 rw = sdefR0 * w0 + sdefR1 * w1;
                float3 r0 = sdefC + sdefR0 - rw;
                float3 r1 = sdefC + sdefR1 - rw;

                float3 cr0 = (sdefC + r0) * 0.5;
                float3 cr1 = (sdefC + r1) * 0.5;

                matrix m0 = mul(bones[boneNo.x], invBones[boneNo.x]);
                matrix m1 = mul(bones[boneNo.y], invBones[boneNo.y]);

	    		matrix rotation = QuaternionToMatrix(QuaternionSlerp(q0, q1, w1));

                return mul(rotation, pos - sdefC) + mul(m0, cr0) * w0 + mul(m1, cr1) * w1;
                break;
            }
    default:
        break;
    }
    return mul(ret, pos);

}

Output VS(
float4 pos : POSITION,
float4 normal : NORMAL,
float2 uv : TEXCOORD,
float3 morphPos : MORPHPOSITION,
float4 morphUV : MORPHUV,
float3 sdefC : SDEFC,
float3 sdefR0 : SDEFRZERO,
float3 sdefR1 : SDEFRONE,
float4 q0 : QZERO,
float4 q1 : QONE,
uint4 boneNo : BONENO,
float4 boneWeight : WEIGHT,
min16uint weightType : WEIGHTTYPE,
uint instNo : SV_InstanceID
) 
{
	Output output;
    pos.xyz = pos.xyz + morphPos;
    output.pos = vertexSkinning(pos, boneNo, boneWeight, weightType, sdefC, sdefR0, sdefR1, q0, q1);
	output.pos = mul(world, output.pos);
    output.svpos = mul(mul(projection, view), output.pos);
    normal.w = 0;
    output.tpos = mul(lightCamera, output.pos);
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
float3 sdefC : SDEFC,
float3 sdefR0 : SDEFRZERO,
float3 sdefR1 : SDEFRONE,
float4 q0 : QZERO,
float4 q1 : QONE,
uint4 boneNo : BONENO,
float4 boneWeight : WEIGHT,
min16uint weightType : WEIGHTTYPE,
uint instNo : SV_InstanceID
) 
{
    Output output;
    
    pos.xyz = pos.xyz + morphPos;
    output.pos = vertexSkinning(pos, boneNo, boneWeight, weightType, sdefC, sdefR0, sdefR1, q0, q1);
    output.pos = mul(world, output.pos);
    output.svpos = mul(lightCamera, output.pos);
    normal.w = 0;
    output.tpos = mul(lightCamera, output.pos);
    output.normal = mul(world, normal);
    uv += morphUV.xy;
    output.uv = uv;
    output.instNo = instNo;
    return output;
}