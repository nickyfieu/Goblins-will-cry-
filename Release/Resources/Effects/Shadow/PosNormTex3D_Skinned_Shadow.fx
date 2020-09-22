float4x4 gWorld : WORLD;
float4x4 gWorldViewProj : WORLDVIEWPROJECTION;
float4x4 gView : VIEW;
float4x4 gViewInverse : VIEWINVERSE;
float4x4 gWorldViewProj_Light;
float3 gLightDirection = float3(-0.577f, -0.577f, 0.577f);
float gShadowMapBias = 0.01f;
float gShadowColor = 0.55f;
float4x4 gBones[70];

Texture2D gDiffuseMap;
Texture2D gShadowMap;

SamplerComparisonState cmpSampler
{
	// sampler state
	Filter = COMPARISON_MIN_MAG_MIP_LINEAR;
	AddressU = MIRROR;
	AddressV = MIRROR;

	// sampler comparison state
	ComparisonFunc = LESS_EQUAL;
};

SamplerState samLinear
{
    Filter = MIN_MAG_MIP_LINEAR;
    AddressU = Wrap;// or Mirror or Clamp or Border
    AddressV = Wrap;// or Mirror or Clamp or Border
};

struct VS_INPUT
{
	float3 pos : POSITION;
	float3 normal : NORMAL;
	float2 texCoord : TEXCOORD;
	float4 BoneIndices : BLENDINDICES;
	float4 BoneWeights : BLENDWEIGHTS;
};

struct VS_OUTPUT
{
	float4 pos : SV_POSITION;
	float3 normal : NORMAL;
	float2 texCoord : TEXCOORD;
	float4 lPos : TEXCOORD1;
};

DepthStencilState EnableDepth
{
	DepthEnable = TRUE;
	DepthWriteMask = ALL;
};

RasterizerState NoCulling
{
	CullMode = NONE;
};

//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
VS_OUTPUT VS(VS_INPUT input)
{
	VS_OUTPUT output = (VS_OUTPUT)0;

	float4 originalPosition = float4(input.pos, 1);
	float4 transformedPosition = 0;
	float3 transformedNormal = 0;

	// create a for loop that does 4 iterations
	for (int i = 0; i < 4; i++)
	{
		// get the current  bone index.
		int currentBoneIndex = input.BoneIndices[i];
		// if the current bone index is bigger than -1 ( the vertex is attached to a bone )
		//		then update the transformedPosition and transformedNormal
		//	( !NOTE! for each itteration you will add the boneweight times the original position transformed with the bonematrix to the position.
		//		For the normal the same but we only rotate the normal ( float3x3 )!
		if (currentBoneIndex > -1)
		{
			transformedPosition += mul(mul(originalPosition,gBones[currentBoneIndex]), input.BoneWeights[i]);
			transformedNormal += mul(normalize(mul(input.normal, (float3x3)gBones[currentBoneIndex])), input.BoneWeights[i]);
		}
		// finaly set the w part of the transformed position to 1
		transformedPosition[3] = 1.f;
	}
	output.pos = mul ( transformedPosition, gWorldViewProj ); //Non skinned position
	output.normal = normalize(mul(transformedNormal, (float3x3)gWorld)); //Non skinned normal
	//Projecting position into light clip space and storing it inside of lPos
    output.lPos = mul(float4(input.pos,1.0f), gWorldViewProj_Light );

	output.texCoord = input.texCoord;
	return output;
}

float2 texOffset(int u, int v)
{
	//TODO: return offseted value (our shadow map has the following dimensions: 1280 * 720)
	return float2(u / 1280.0f,v / 720.0f);
}

float EvaluateShadowMap(float4 lpos)
{	
	//re-homogenize position after interpolation
    lpos.xyz /= lpos.w;
	
	if (lpos.x < -1.0f || lpos.x > 1.0f ||
	lpos.y < -1.0f || lpos.y > 1.0f ||
	lpos.z < 0.0f || lpos.x > 1.0f)
	{
		return 1.0f;
	}
	
	//transform clip space coords to texture space coords (-1:1 to 0:1)
    lpos.x = lpos.x /  2.0f + 0.5f;
    lpos.y = lpos.y / -2.0f + 0.5f;
	
	//apply shadow map bias
    lpos.z -= gShadowMapBias;
	
	//sample shadow map
    float total = 0;

	for (float x = -3.5f; x < 3.5f; x++)
	{
		for (float y = -3.5f; y < 3.5f; y++)
		{
			total += gShadowMap.SampleCmpLevelZero(cmpSampler, lpos.xy + texOffset(x, y), lpos.z).r;
		}
	}

	total /= 64;
	
    //if clip space z is greater than shadowMapDepth its in shadow
    if ( lpos.z > total )	return gShadowColor;
	//else return not in shadow
	return 1.f;
}

//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 PS(VS_OUTPUT input) : SV_TARGET
{
	float shadowValue = EvaluateShadowMap(input.lPos);

	float4 diffuseColor = gDiffuseMap.Sample( samLinear,input.texCoord );
	float3 color_rgb= diffuseColor.rgb;
	float color_a = diffuseColor.a;
	
	//HalfLambert Diffuse :)
	float diffuseStrength = dot(input.normal, -gLightDirection);
	diffuseStrength = diffuseStrength * 0.5 + 0.5;
	diffuseStrength = saturate(diffuseStrength);
	color_rgb = color_rgb * diffuseStrength;

	return float4( color_rgb * shadowValue, color_a );
}

//--------------------------------------------------------------------------------------
// Technique
//--------------------------------------------------------------------------------------
technique11 Default
{
    pass P0
    {
		SetRasterizerState(NoCulling);
		SetDepthStencilState(EnableDepth, 0);

		SetVertexShader( CompileShader( vs_4_0, VS() ) );
		SetGeometryShader( NULL );
		SetPixelShader( CompileShader( ps_4_0, PS() ) );
    }
}

