float4x4 gView : VIEW;
float4x4 gViewInverse : VIEWINVERSE;
float4x4 gWorld : WORLD;
float4x4 gWorldViewProj : WORLDVIEWPROJECTION;
float4x4 gLightViewProj;
float4x4 gBones[70];
 
DepthStencilState depthStencilState
{
	DepthEnable = TRUE;
	DepthWriteMask = ALL;
};

RasterizerState rasterizerState
{
	FillMode = SOLID;
	CullMode = NONE;
};

//--------------------------------------------------------------------------------------
// Vertex Shader [STATIC]
//--------------------------------------------------------------------------------------
float4 ShadowMapVS(float3 position:POSITION):SV_POSITION
{
	//TODO: return the position of the vertex in correct space (hint: seen from the view of the light)

	return mul(  float4(position,1.0f), mul( gWorld, gLightViewProj ) );
}

//--------------------------------------------------------------------------------------
// Vertex Shader [SKINNED]
//--------------------------------------------------------------------------------------
float4 ShadowMapVS_Skinned(float3 position:POSITION, float4 BoneIndices : BLENDINDICES, float4 BoneWeights : BLENDWEIGHTS) : SV_POSITION
{
	//TODO: return the position of the ANIMATED vertex in correct space (hint: seen from the view of the light)

	float4 originalPosition = float4(position, 1.0f);
	float4 transformedPosition = 0;

	// create a for loop that does 4 iterations
	for (int i = 0; i < 4; i++)
	{
		//Get the current  bone index.
		int currentBoneIndex = BoneIndices[i];
		//If the current bone index is bigger than -1 ( the vertex is attached to a bone )
		//	then update the transformedPosition and transformedNormal
		//( !NOTE! for each itteration you will add the boneweight times the original position transformed with the bonematrix to the position.
		//	For the normal the same but we only rotate the normal ( float3x3 )!
		if (currentBoneIndex > -1)
		{
			transformedPosition += mul(mul(originalPosition,gBones[currentBoneIndex]), BoneWeights[i]);
		}
		//Finaly set the w part of the transformed position to 1
		transformedPosition[3] = 1.f;
	}
	// calculate the output position
	return mul( transformedPosition, mul( gWorld, gLightViewProj ) );
}
 
//--------------------------------------------------------------------------------------
// Pixel Shaders
//--------------------------------------------------------------------------------------
void ShadowMapPS_VOID(float4 position:SV_POSITION){}

technique11 GenerateShadows
{
	pass P0
	{
		SetRasterizerState(rasterizerState);
	    SetDepthStencilState(depthStencilState, 0);
		SetVertexShader(CompileShader(vs_4_0, ShadowMapVS()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_4_0, ShadowMapPS_VOID()));
	}
}

technique11 GenerateShadows_Skinned
{
	pass P0
	{
		SetRasterizerState(rasterizerState);
		SetDepthStencilState(depthStencilState, 0);
		SetVertexShader(CompileShader(vs_4_0, ShadowMapVS_Skinned()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_4_0, ShadowMapPS_VOID()));
	}
}