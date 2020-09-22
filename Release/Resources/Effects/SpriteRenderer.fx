float4x4 gView : VIEW;
float4x4 gViewInverse : VIEWINVERSE;
float4x4 m_MatrixWorldViewProj : WORLDVIEWPROJECTION;
float4x4 m_MatrixWorld : WORLD;

float4x4 gTransform : WorldViewProjection;
Texture2D gSpriteTexture;
float2 gTextureSize;

SamplerState samPoint
{
    Filter = MIN_MAG_MIP_POINT;
    AddressU = WRAP;
    AddressV = WRAP;
};

BlendState EnableBlending 
{     
	BlendEnable[0] = TRUE;
	SrcBlend = SRC_ALPHA;
    DestBlend = INV_SRC_ALPHA;
};

DepthStencilState NoDepth
{
	DepthEnable = FALSE;
	DepthWriteMask = ZERO;
};

RasterizerState BackCulling 
{ 
	CullMode = BACK; 
};

//SHADER STRUCTS
//**************
struct VS_DATA
{
	uint TextureId: TEXCOORD0;
	float4 TransformData : POSITION; //PosX, PosY, Depth (PosZ), Rotation
	float4 TransformData2 : POSITION1; //PivotX, PivotY, ScaleX, ScaleY
	float4 Color: COLOR;	
};

struct GS_DATA
{
	float4 Position : SV_POSITION;
	float4 Color: COLOR;
	float2 TexCoord: TEXCOORD0;
};

//VERTEX SHADER
//*************
VS_DATA MainVS(VS_DATA input)
{
	return input;
}

//GEOMETRY SHADER
//***************
void CreateVertex(inout TriangleStream<GS_DATA> triStream, float3 pos, float4 col, float2 texCoord, float rotation, float2 rotCosSin, float2 offset, float2 pivotOffset)
{
	if (rotation != 0)
	{
		//Step 3.
		//Do rotation calculations
		//Transform to origin
		pos -= float3(offset - pivotOffset,0);
		//Rotate
		pos.xy = float2((pos.x * rotCosSin.x) - (pos.y * rotCosSin.y),(pos.y * rotCosSin.x) + (pos.x * rotCosSin.y));
		//Retransform to initial position
		pos += float3(offset,0);
	}
	else
	{
		//Step 2.
		//No rotation calculations (no need to do the rotation calculations if there is no rotation applied > redundant operations)
		//Just apply the pivot offset
		pos += float3(pivotOffset,0);
	}

	//Geometry Vertex Output
	GS_DATA geomData = (GS_DATA) 0;
	geomData.Position = mul(float4(pos, 1.0f), gTransform);
	geomData.Color = col;
	geomData.TexCoord = texCoord;
	triStream.Append(geomData);
}

[maxvertexcount(4)]
void MainGS(point VS_DATA vertex[1], inout TriangleStream<GS_DATA> triStream)
{
	//Given Data (Vertex Data)
	float3	position 	= float3(vertex[0].TransformData.xyz); //Extract the position data from the VS_DATA vertex struct
	float	rotation 	= vertex[0].TransformData.w; //Extract the rotation data from the VS_DATA vertex struct
	float2	pivot 		= float2(vertex[0].TransformData2.xy); //Extract the pivot data from the VS_DATA vertex struct
	float2	scale 		= float2(gTextureSize * vertex[0].TransformData2.zw); //Extract the scale data from the VS_DATA vertex struct
	float2	offset 		= vertex[0].TransformData.xy; //Extract the offset data from the VS_DATA vertex struct (initial X and Y position)
	float2	texCoord 	= float2(0,0); //Initial Texture Coordinate
	float2	rotCosSin	= float2(cos(rotation),sin(rotation)); // calculate rotCosSin ( + 3.14 due to it beeing rotated by 180 degrees )
	float4 	color 		= vertex[0].Color;
	//...

	// LT----------RT //TringleStrip (LT > RT > LB, LB > RB > RT)
	// |          / |
	// |       /    |
	// |    /       |
	// | /          |
	// LB----------RB

	//VERTEX 1 [LT]
	CreateVertex(triStream, position, color, texCoord, rotation, rotCosSin, offset, (float2(0,0) - pivot) * scale); //Change the color data too!
	
	texCoord 	= float2(1,0);
	
	//VERTEX 2 [RT]
	CreateVertex(triStream, position, color, texCoord, rotation, rotCosSin, offset, (float2(1,0) - pivot) * scale); //Change the color data too!

	texCoord 	= float2(0,1);

	//VERTEX 3 [LB]
	CreateVertex(triStream, position, color, texCoord, rotation, rotCosSin, offset, (float2(0,1) - pivot) * scale); //Change the color data too!

	texCoord 	= float2(1,1);
	
	//VERTEX 4 [RB]
	CreateVertex(triStream, position, color, texCoord, rotation, rotCosSin, offset, (float2(1,1) - pivot) * scale); //Change the color data too!
}

//PIXEL SHADER
//************
float4 MainPS(GS_DATA input) : SV_TARGET
{
	// otherwise ui and text did weird going trough textures behind it 
	float4 Color = gSpriteTexture.Sample(samPoint, input.TexCoord) * input.Color;
	clip(Color.a < 0.1f ? -1 : 1);
	return Color;
}

// Default Technique
technique11 Default {

	pass p0 {
		SetRasterizerState(BackCulling);
		SetBlendState(EnableBlending,float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);
		//SetDepthStencilState(NoDepth,0);
		SetVertexShader(CompileShader(vs_4_0, MainVS()));
		SetGeometryShader(CompileShader(gs_4_0, MainGS()));
		SetPixelShader(CompileShader(ps_4_0, MainPS()));
	}
}
