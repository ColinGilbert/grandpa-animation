///////////////////////////////////////////////////////////////////////////////
//全局变量
///////////////////////////////////////////////////////////////////////////////
float4x4 g_mWorldViewProjection;
float4x4 g_mWorld;

float	g_fAmbient;
float	g_fDiffuse;

float3	g_vLightDir;
float3	g_vCameraPos;

texture g_texDiffuse;
texture	g_texNormal;
texture	g_texGloss;

bool	g_bDiffuse = true;
bool	g_bGloss = true;

static const int MAX_MATRICES = 48;
float4x4 g_boneMatrices[MAX_MATRICES] : WORLDMATRIXARRAY;

///////////////////////////////////////////////////////////////////////////////
//纹理采样
///////////////////////////////////////////////////////////////////////////////
sampler DiffuseSampler = 
sampler_state
{
	Texture		= <g_texDiffuse>;
	MipFilter	= LINEAR;
	MinFilter	= LINEAR;
	MagFilter	= LINEAR;
	AddressU	= WRAP;
	AddressV	= WRAP;
};

sampler NormalSampler = 
sampler_state
{
	Texture		= <g_texNormal>;
	MipFilter	= LINEAR;
	MinFilter	= LINEAR;
	MagFilter	= LINEAR;
	AddressU	= WRAP;
	AddressV	= WRAP;
};

sampler GlossSampler = 
sampler_state
{
	Texture		= <g_texGloss>;
	MipFilter	= LINEAR;
	MinFilter	= LINEAR;
	MagFilter	= LINEAR;
	AddressU	= WRAP;
	AddressV	= WRAP;
};

///////////////////////////////////////////////////////////////////////////////
//顶点声明
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
struct VS_IN_NORMALMAP
{
	float4	vPos		: POSITION;
	float3	vNormal		: NORMAL;
	float3	vTangent	: TANGENT;
	float3	vBinormal	: BINORMAL;
	float2	vTexCoord	: TEXCOORD0;
};

struct VS_IN_NORMALMAP_GPU_SKINNING
{
	float4	vPos			: POSITION;
	float3	vNormal			: NORMAL;
	float3	vTangent		: TANGENT;
	float3	vBinormal		: BINORMAL;
	float2	vTexCoord		: TEXCOORD0;
	float4  BlendIndices	: BLENDINDICES;
	float4  BlendWeights	: BLENDWEIGHT;
};

struct VS_OUT_NORMALMAP
{
	float4	vPosProj	: POSITION;
	float3	vLightDir	: TEXCOORD0;
	float3	vViewDir	: TEXCOORD1;
	float2	vTexCoord	: TEXCOORD2;
};

///////////////////////////////////////////////////////////////////////////////
struct VS_IN_VERTEXLIGHT
{
	float4	vPos		: POSITION;
	float3	vNormal		: NORMAL;
	float2	vTexCoord	: TEXCOORD0;
};

struct VS_IN_VERTEXLIGHT_GPU_SKINNING
{
	float4	vPos			: POSITION;
	float3	vNormal			: NORMAL;
	float2	vTexCoord		: TEXCOORD0;
	float4  BlendIndices	: BLENDINDICES;
	float4  BlendWeights	: BLENDWEIGHT;
};

struct VS_OUT_VERTEXLIGHT
{
	float4	vPosProj	: POSITION;
	float3	vDiffuse	: TEXCOORD0;
	float2	vTexCoord	: TEXCOORD1;
};

///////////////////////////////////////////////////////////////////////////////
struct VS_IN_NOLIGHT
{
	float4	vPos		: POSITION;
	float2	vTexCoord	: TEXCOORD0;
};

struct VS_OUT_NOLIGHT
{
	float4	vPosProj	: POSITION;
	float3	vColor		: TEXCOORD0;
	float2	vTexCoord	: TEXCOORD1;
};

///////////////////////////////////////////////////////////////////////////////
struct VS_IN_SIMPLE
{
	float4	vPos		: POSITION;
};

struct VS_OUT_SIMPLE
{
	float4	vPosProj	: POSITION;
};

///////////////////////////////////////////////////////////////////////////////
struct PS_OUT 
{
	float4	vColor		: COLOR0;
};

///////////////////////////////////////////////////////////////////////////////
//shader
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
VS_OUT_NORMALMAP VS_Normalmap( VS_IN_NORMALMAP vsIn )
{
	VS_OUT_NORMALMAP vsOut;
	vsOut.vPosProj = mul( vsIn.vPos, g_mWorldViewProjection );
	vsOut.vTexCoord = vsIn.vTexCoord;
	
	float3x3 mWorldRotation = g_mWorld;
	float3 tangent = mul( vsIn.vTangent, mWorldRotation );
	float3 binormal = mul( vsIn.vBinormal, mWorldRotation );
	float3 normal = mul( vsIn.vNormal, mWorldRotation );
	normal = normalize( normal );
	tangent = normalize( tangent );
	binormal = normalize( binormal );
	float3x3 mTangentToWorld = float3x3( tangent, binormal, normal );
	vsOut.vLightDir = mul( mTangentToWorld, g_vLightDir );
	
	float3 worldPos = mul( vsIn.vPos, g_mWorld );
	float3 vViewDir = worldPos - g_vCameraPos;
	vsOut.vViewDir = mul( mTangentToWorld, vViewDir );
	
	//temp
	//vsOut.vLightDir = vsIn.vBinormal;
	
	return vsOut;
}

///////////////////////////////////////////////////////////////////////////////
VS_OUT_NORMALMAP VS_Normalmap_Gpu_Skinning( VS_IN_NORMALMAP_GPU_SKINNING vsIn )
{
	VS_OUT_NORMALMAP vsOut;

	float weights[4] = (float[4])vsIn.BlendWeights;
	int indices[4] = (int[4])vsIn.BlendIndices;
	float3 pos = 0.0f;
	float3 normal = 0.0f;
	float3 tangent = 0.0f;
	float3 binormal = 0.0f;
	for ( int i = 0; i < 4; i++ )
	{
		pos += mul( vsIn.vPos, g_boneMatrices[indices[i]] ) * weights[i];
		normal += mul( vsIn.vNormal, g_boneMatrices[indices[i]] ) * weights[i];
		tangent += mul( vsIn.vTangent, g_boneMatrices[indices[i]] ) * weights[i];
		binormal += mul( vsIn.vBinormal, g_boneMatrices[indices[i]] ) * weights[i];
	}

	float3x3 mWorldRotation = g_mWorld;	
	tangent = mul( tangent, mWorldRotation );
	binormal = mul( binormal, mWorldRotation );
	normal = mul( normal, mWorldRotation );
	normal = normalize(normal);
	tangent = normalize(tangent);
	binormal = normalize(binormal);
    
	float3x3 mTangentToWorld = float3x3( tangent, binormal, normal );
	vsOut.vLightDir = mul( mTangentToWorld, g_vLightDir );
	
	vsOut.vPosProj = mul( float4(pos.xyz, 1.0f), g_mWorldViewProjection );
	vsOut.vTexCoord = vsIn.vTexCoord;
	
	float3 worldPos = mul( pos, g_mWorld );
	float3 vViewDir = worldPos - g_vCameraPos;
	vsOut.vViewDir = mul( mTangentToWorld, vViewDir );
	
	return vsOut;
}

PS_OUT PS_Normalmap( VS_OUT_NORMALMAP psIn )
{
	PS_OUT psOut;
	float3 vNormal = normalize( tex2D( NormalSampler, psIn.vTexCoord ).xyz - 0.5 );
	
	float3 vLightDir = normalize( psIn.vLightDir );
	
	float4 vColor;
	vColor.xyz = g_fAmbient + g_fDiffuse * saturate( dot( vNormal, vLightDir ) );
	vColor.a = 1;
	if ( g_bDiffuse )
	{
		vColor *= tex2D( DiffuseSampler, psIn.vTexCoord );
	}
	psOut.vColor = vColor;
	
	if ( g_bGloss )
	{
		float3 vViewDir = normalize( psIn.vViewDir );
		float3 vReflect = reflect( vLightDir, vNormal );
		float3 vGloss = g_fDiffuse * pow( saturate( dot( vReflect, vViewDir ) ), 32 );
		vGloss *= tex2D( GlossSampler, psIn.vTexCoord ).xyz;
		psOut.vColor.xyz += vGloss;
	}
	//temp
	//psOut.vColor.xyz = psIn.vLightDir;
	//psOut.vColor.xyz = tex2D( NormalSampler, psIn.vTexCoord );
	
	return psOut;
}

///////////////////////////////////////////////////////////////////////////////
VS_OUT_VERTEXLIGHT VS_VertexLight( VS_IN_VERTEXLIGHT vsIn )
{
	VS_OUT_VERTEXLIGHT vsOut;
	vsOut.vPosProj = mul( vsIn.vPos, g_mWorldViewProjection );
	vsOut.vTexCoord = vsIn.vTexCoord;
	float3x3 mWorldRotation = g_mWorld;
	float3 normal = mul( vsIn.vNormal, mWorldRotation );
	normal = normalize(normal);
	vsOut.vDiffuse = g_fAmbient + g_fDiffuse * saturate( dot( normal, g_vLightDir ) );
	return vsOut;
}

VS_OUT_VERTEXLIGHT VS_VertexLight_Gpu_Skinning( VS_IN_VERTEXLIGHT_GPU_SKINNING vsIn )
{
	VS_OUT_VERTEXLIGHT vsOut;
	
	float weights[4] = (float[4])vsIn.BlendWeights;
	int indices[4] = (int[4])vsIn.BlendIndices;

	float3 pos = 0.0f;
	float3 normal = 0.0f;
	for ( int i = 0; i < 4; i++ )
	{
		pos += mul( vsIn.vPos, g_boneMatrices[indices[i]] ) * weights[i];
		normal += mul( vsIn.vNormal, g_boneMatrices[indices[i]] ) * weights[i];
	}

	vsOut.vPosProj = mul( float4(pos.xyz,1.0f), g_mWorldViewProjection );
	vsOut.vTexCoord = vsIn.vTexCoord;
	float3x3 mWorldRotation = g_mWorld;
	normal = mul( normal, mWorldRotation );
	normal = normalize(normal);
	vsOut.vDiffuse = g_fAmbient + g_fDiffuse * saturate( dot( normal, g_vLightDir ) );
	return vsOut;
}

PS_OUT PS_VertexLight( VS_OUT_VERTEXLIGHT psIn )
{
	PS_OUT psOut;
		
	float4 vColor;
	vColor.xyz = psIn.vDiffuse;
	vColor.a = 1;
	if ( g_bDiffuse )
	{
		vColor *= tex2D( DiffuseSampler, psIn.vTexCoord );
	}
	psOut.vColor = vColor;
	return psOut;
}

///////////////////////////////////////////////////////////////////////////////
VS_OUT_NOLIGHT VS_NoLight( VS_IN_NOLIGHT vsIn )
{
	VS_OUT_NOLIGHT vsOut;
	vsOut.vPosProj = mul( vsIn.vPos, g_mWorldViewProjection );
	vsOut.vTexCoord = vsIn.vTexCoord;
	
	float3 vView = normalize( g_vCameraPos - vsIn.vPos );
	float fTotalLight = g_fAmbient + g_fDiffuse * ( dot( vView, g_vLightDir ) + 1.0 ) / 3.5;
	vsOut.vColor.xyz = fTotalLight;
	
	return vsOut;
}

PS_OUT PS_NoLight( VS_OUT_NOLIGHT psIn )
{
	PS_OUT psOut;
	
	float3 vColor = psIn.vColor;
	if ( g_bDiffuse )
	{
		vColor *= tex2D( DiffuseSampler, psIn.vTexCoord );
	}
	psOut.vColor.xyz = vColor;
	psOut.vColor.a = 1;
	
	return psOut;
}

///////////////////////////////////////////////////////////////////////////////
VS_OUT_SIMPLE VS_Simple( VS_IN_SIMPLE vsIn )
{
	VS_OUT_SIMPLE vsOut;
	vsOut.vPosProj = mul( vsIn.vPos, g_mWorldViewProjection );
	return vsOut;
}

PS_OUT PS_Simple( VS_OUT_SIMPLE psIn )
{
	PS_OUT psOut;
	psOut.vColor = 1;
	return psOut;
}

///////////////////////////////////////////////////////////////////////////////
//technique
///////////////////////////////////////////////////////////////////////////////
technique Normalmap 
{
	pass P0 
	{
		VertexShader = compile vs_2_0 VS_Normalmap();
		PixelShader = compile ps_2_0 PS_Normalmap();
	}
}

technique Normalmap_Gpu_Skinning
{
	pass P0 
	{
		VertexShader = compile vs_2_0 VS_Normalmap_Gpu_Skinning();
		PixelShader = compile ps_2_0 PS_Normalmap();
	}
}

technique VertexLight
{
	pass P0
	{
		VertexShader = compile vs_2_0 VS_VertexLight();
		PixelShader = compile ps_2_0 PS_VertexLight();
	}
}

technique VertexLight_Gpu_Skinning
{
	pass P0
	{
		VertexShader = compile vs_2_0 VS_VertexLight_Gpu_Skinning();
		PixelShader = compile ps_2_0 PS_VertexLight();
	}
}

technique NoLight
{
	pass P0
	{
		VertexShader = compile vs_2_0 VS_NoLight();
		PixelShader = compile ps_2_0 PS_NoLight();
	}
}

technique Simple
{
	pass P0
	{
		VertexShader = compile vs_2_0 VS_Simple();
		PixelShader = compile ps_2_0 PS_Simple();
	}
}
