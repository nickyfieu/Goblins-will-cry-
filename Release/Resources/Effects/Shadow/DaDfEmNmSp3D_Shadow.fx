// This shader contains :
// Toggleable diffuse map
// Toggleable diffuse ambient map
// Toggleable specular map
// Toggleable normal map
// Toggleable "emmisive" map
//		note this is not real emmisiveness 
//		it is essentialy just a "emmisive" texture amplified and clamped 

//--------------------------------------------------------------------------------------
// Global Variables
//--------------------------------------------------------------------------------------

float4x4 gView : VIEW;
float4x4 gViewInverse : VIEWINVERSE;
float4x4 gWorld : WORLD;
float4x4 gWorldViewProj : WORLDVIEWPROJECTION; 

float3 gLightDirection = float3(-0.577f, -0.577f, 0.577f);

// shadow variables
float4x4 gWorldViewProj_Light;
float gShadowMapBias = 0.001f;
float gShadowColor = 0.55f;
Texture2D gShadowMap;

// diffuse ambient variables
bool gUseDiffuseAmbientMap = false;
float gAmbientIntensity = 0.25f;
Texture2D gDiffuseAmbientMap;

// diffuse variables
bool gUseDiffuseMap = false;
bool gUseHalfLambert = false;
Texture2D gDiffuseMap;

// emmisive variables
bool gUseEmmisiveMap = false;
float gEmmisiveStrength = 2.5f; 
Texture2D gEmmisiveMap;

// normal variables
bool gUseNormalMap = false;
bool gFlipGreenChannel = false;
Texture2D gNormalMap;

// specular variables
bool gUseSpecularMap = false;
bool gUseSpecularAsDiffuseTex = false;
int gShininess = 25;
float gSpecularModifier = 1.f;
Texture2D gSpecularMap;


//--------------------------------------------------------------------------------------
// Input & output struct
//--------------------------------------------------------------------------------------

struct VS_INPUT{
	float3 pos : POSITION;
	float3 normal : NORMAL;
	float3 tangent : TANGENT;
	float2 texCoord : TEXCOORD;
};
struct VS_OUTPUT{
	float4 pos : SV_POSITION;
	float4 worldPos : COLOR0;
	float3 normal : NORMAL;
	float3 tangent : TANGENT;
	float2 texCoord : TEXCOORD;
	float4 lPos : TEXCOORD1;
};

//--------------------------------------------------------------------------------------
// States
//--------------------------------------------------------------------------------------

SamplerState samLinear
{
	Filter = MIN_MAG_MIP_LINEAR;
	AddressU = WRAP;// or Mirror or Clamp or Border
	AddressV = WRAP;// or Mirror or Clamp or Border
	AddressW = WRAP;
};

SamplerComparisonState cmpSampler
{
	// sampler state
	Filter = COMPARISON_MIN_MAG_MIP_LINEAR;
	AddressU = MIRROR;
	AddressV = MIRROR;

	// sampler comparison state
	ComparisonFunc = LESS_EQUAL;
};

RasterizerState rsNoCulling
{
	CullMode = NONE;
};

DepthStencilState EnableDepth
{
	DepthEnable = TRUE;
	DepthWriteMask = ALL;
};

//--------------------------------------------------------------------------------------
// Functions
//--------------------------------------------------------------------------------------

// diffuse function
float4 CalculateDiffuse(float3 normal, float2 texCoord)
{
	float3 diffuseColor = (float3)0.f;
	if (gUseDiffuseMap == false)
		return float4(diffuseColor, 1.f);

	float4 diffuse = gDiffuseMap.Sample(samLinear, texCoord);
	diffuseColor = diffuse.rgb;

	float diffuseStrength = dot(normal, -gLightDirection);
	if (gUseHalfLambert)
	{
		diffuseStrength *= 0.5f;
		diffuseStrength += 0.5f;
	}
	diffuseStrength = saturate(diffuseStrength);

	return float4(diffuseColor * diffuseStrength, diffuse.a);
}

// diffuse ambient function
float3 CalculateDiffuseAmbient(float2 texCoord)
{
	float3 diffuseAmbientColor = (float3)0.f;

	if (gUseDiffuseAmbientMap == false)
		return diffuseAmbientColor;

	diffuseAmbientColor = saturate(gDiffuseAmbientMap.Sample(samLinear, texCoord).rgb) * gAmbientIntensity;
	return diffuseAmbientColor;
}

// normal function
float3 CalculateNormal(float3 normal, float3 tangent, float2 texCoord)
{
	if (gUseNormalMap == false)
		return normal;

	float3 binormal = cross(normal, tangent);

	if (gFlipGreenChannel)
		binormal.g *= -1.f;

	float3x3 tangentSpaceAxis = float3x3(tangent, binormal, normal);

	float3 calcNormal = gNormalMap.Sample(samLinear, texCoord);
	calcNormal = calcNormal * 2.f - 1.f;
	calcNormal = mul(calcNormal, tangentSpaceAxis);
	return calcNormal;
}

// "emmisive" function
float3 CalculateEmmisiveColor(float2 texCoord)
{
	float3 EmmisiveColor = (float3)0.f;
	if (gUseEmmisiveMap == false)
	{
		return EmmisiveColor;
	}

	EmmisiveColor = gEmmisiveMap.Sample(samLinear, texCoord).rgb * gEmmisiveStrength;
	return EmmisiveColor;
}

// specular function

float3 CalculateSpecular(float3 viewDirection, float3 normal, float2 texCoord)
{
	float3 specularColor = (float3)0.f;
	if (gUseSpecularMap == false)
		return specularColor;

	specularColor = gSpecularMap.Sample(samLinear, texCoord).rgb;
	
	if (gUseSpecularAsDiffuseTex)
	{
		return specularColor * gSpecularModifier;
	}
	else
	{

		float3 lightReflect = reflect(gLightDirection, normal);
		float specularFactor = pow(saturate(dot(viewDirection, lightReflect)), gShininess);

		return specularColor * specularFactor * gSpecularModifier;
	}
}

//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
VS_OUTPUT VS(VS_INPUT input){

	VS_OUTPUT output;
	output.pos = mul ( float4(input.pos,1.0f), gWorldViewProj );
	output.worldPos = mul(float4(input.pos, 1.0f), gWorld);
	// normalizing in the vertex shader why?
	// well there will be less verticies for each model than there will be  pixels on the screen
	// 1280 x 720 pixels = 921600 times the pixel shader gets called;
	// none of the models used in this project have anything close to 921600 verticies
	float3x3 worldRot = (float3x3)gWorld;
	output.normal = normalize(mul(input.normal, worldRot));
	output.tangent = normalize(mul(input.tangent, worldRot));

	output.texCoord = input.texCoord;
	output.lPos = mul(float4(input.pos, 1.0f), gWorldViewProj_Light);
	return output;
}

// --------------------------------------------------------------------------------------
// Shadow map functions
// --------------------------------------------------------------------------------------

float2 texOffset(int u, int v)
{
	//TODO: return offseted value (our shadow map has the following dimensions: 1280 * 720)
	return float2(u / 1280.0f, v / 720.0f);
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
	lpos.x = lpos.x / 2.0f + 0.5f;
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
	if (lpos.z > total)	return gShadowColor;
	//else return not in shadow
	return 1.f;
}

//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 PS(VS_OUTPUT input) : SV_TARGET
{
	float2 texCoordinate = input.texCoord;
	float3 viewDirection = normalize(input.worldPos.xyz - gViewInverse[3].xyz);

	// normal
	float3 calculatedNormal = CalculateNormal(input.normal, input.tangent, texCoordinate);

	// diffuse
	float4 diffuseColor = CalculateDiffuse(calculatedNormal, texCoordinate);
	
	// diffuse ambient
	float3 diffuseAmbient = CalculateDiffuseAmbient(texCoordinate);

	// "emmisive"
	float3 emmisiveColor = CalculateEmmisiveColor(texCoordinate);

	// specular
	float3 specularColor = CalculateSpecular(viewDirection, calculatedNormal, texCoordinate);
	
	// shadow
	float shadowValue = EvaluateShadowMap(input.lPos);

	// final color
	float3 finalColor = ((diffuseColor.rgb + diffuseAmbient + specularColor) * shadowValue)  + emmisiveColor;

	return float4(saturate(finalColor), diffuseColor.w);
}

//--------------------------------------------------------------------------------------
// Technique
//--------------------------------------------------------------------------------------
technique11 Default
{
    pass P0
    {
		SetRasterizerState(rsNoCulling);
		SetDepthStencilState(EnableDepth, 0);

		SetVertexShader( CompileShader( vs_4_0, VS() ) );
		SetGeometryShader( NULL );
		SetPixelShader( CompileShader( ps_4_0, PS() ) );
    }
}

