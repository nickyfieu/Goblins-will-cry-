//=============================================================================
//// Shader uses position and texture
//=============================================================================
SamplerState samPoint
{
    Filter = MIN_MAG_MIP_POINT;
    AddressU = Mirror;
    AddressV = Mirror;
};

Texture2D gTexture;

/// Create Depth Stencil State (ENABLE DEPTH WRITING)
DepthStencilState depthStencilState
{
	DepthEnable = TRUE;
	DepthWriteMask = ALL;
};

/// Create Rasterizer State (Backface culling) 
RasterizerState rasterizerState
{
	FillMode = SOLID;
	CullMode = BACK;
};


//IN/OUT STRUCTS
//--------------
struct VS_INPUT
{
    float3 Position : POSITION;
	float2 TexCoord : TEXCOORD0;

};

struct PS_INPUT
{
    float4 Position : SV_POSITION;
	float2 TexCoord : TEXCOORD1;
};


//VERTEX SHADER
//-------------
PS_INPUT VS(VS_INPUT input)
{
	PS_INPUT output = (PS_INPUT)0;
	// Set the Position
	output.Position = float4(input.Position,1.0f);
	// Set the TexCoord
	output.TexCoord = input.TexCoord;
	
	return output;
}


//PIXEL SHADER
//------------
float4 PS(PS_INPUT input): SV_Target
{
	uint width = 0;
	uint height = 0;
	// Step 1:	find the dimensions of the texture (the texture has a method for that)	
	gTexture.GetDimensions(width, height);
	// Step 2:	calculate dx and dy (UV space for 1 pixel)
	float dxp = 1.0f / (float)width;	// delta x pixel
	float dyp = 1.0f / (float)height;	// delta y pixel
	// Step 3:	Create a double for loop (5 iterations each)
	//			Inside the loop, calculate the offset in each direction. Make sure not to take every pixel but move by 2 pixels each time
	//			Do a texture lookup using your previously calculated uv coordinates + the offset, and add to the final color
	const int cItterations = 5;
	const int cPixOffset = 2;
	const int cItterBegin = cItterations / cPixOffset;

	float4 finalColor = float4(0.f,0.f,0.f,0.f);
	int offsetX = int(input.TexCoord.x / dxp);
	int offsetY = int(input.TexCoord.y / dyp);
	int itterations = 0;
	for (int i = -cItterBegin; i <= cItterBegin; i++)
	{
		bool isValidCol = ((offsetX + i * cPixOffset) >= 0) && ((offsetX + i * cPixOffset) < width);
		for (int j = -cItterBegin; j <= cItterBegin; j++)
		{
			bool isValidRow = ((offsetY + j * cPixOffset) >= 0) && ((offsetY + j * cPixOffset) < height);
			if (isValidCol && isValidRow)
			{
				finalColor += gTexture.Sample(samPoint, float2(input.TexCoord.x + i * cPixOffset * dxp, input.TexCoord.y + j * cPixOffset * dyp));
				itterations++;
			}
		}
	}

	// Step 4:	Divide the final color by the number of passes (in this case 5*5)
	if (itterations)
		finalColor /= float(itterations);
	// Step 5:	return the final color
	finalColor.a = 1.0f;
	return finalColor;
}


//TECHNIQUE
//---------
technique11 Blur
{
    pass P0
    {
		// Set states...
        SetVertexShader( CompileShader( vs_4_0, VS() ) );
        SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_4_0, PS() ) );
    }
}