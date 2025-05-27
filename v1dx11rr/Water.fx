// Water.fx - Shader para efecto de agua
// Estructuras de entrada y salida

struct VS_INPUT
{
    float4 position : POSITION;
    float2 uv : TEXCOORD0;
    float2 uv2 : TEXCOORD1;
    float3 normal : NORMAL0;
    float3 tangent : NORMAL1;
    float3 binormal : NORMAL2;
};

struct PS_INPUT
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD0;
    float3 normal : TEXCOORD1;
    float3 worldPos : TEXCOORD2;
    float3 tangent : TEXCOORD3;
    float3 binormal : TEXCOORD4;
};

// Constant Buffers (deben coincidir con el orden en C++)
cbuffer WorldMatrix : register(b0)
{
    matrix worldMatrix;
}

cbuffer ViewMatrix : register(b1)
{
    matrix viewMatrix;
}

cbuffer ProjectionMatrix : register(b2)
{
    matrix projectionMatrix;
}

cbuffer TexUVBuffer : register(b3)
{
    float2 texUV;
    float2 padding;
}

cbuffer LightDirection : register(b0) // Para Pixel Shader
{
    float3 lightDir;
    float lightPadding1;
}

cbuffer LightColor : register(b1) // Para Pixel Shader
{
    float3 lightColor;
    float lightPadding2;
}

// Texturas y Samplers
Texture2D diffuseTexture : register(t0);
Texture2D normalTexture : register(t1);
SamplerState textureSampler : register(s0);

// VERTEX SHADER
PS_INPUT VS_Main(VS_INPUT input)
{
    PS_INPUT output;
    
    // Transformar posición del vértice
    output.position = mul(input.position, worldMatrix);
    output.worldPos = output.position.xyz;
    output.position = mul(output.position, viewMatrix);
    output.position = mul(output.position, projectionMatrix);
    
    // Animar las coordenadas UV para crear el efecto de agua en movimiento
    output.uv = input.uv + texUV * 0.01f;
    
    // Transformar normales al espacio mundial
    output.normal = normalize(mul(input.normal, (float3x3) worldMatrix));
    output.tangent = normalize(mul(input.tangent, (float3x3) worldMatrix));
    output.binormal = normalize(mul(input.binormal, (float3x3) worldMatrix));
    
    return output;
}

// PIXEL SHADER
float4 PS_Main(PS_INPUT input) : SV_Target
{
    // Normalizar las normales interpoladas
    float3 normal = normalize(input.normal);
    
    // Muestrear la textura de agua
    float4 diffuseColor = diffuseTexture.Sample(textureSampler, input.uv);
    
    // Muestrear el normal map y convertir de [0,1] a [-1,1]
    float3 normalMap = normalTexture.Sample(textureSampler, input.uv * 2.0f).rgb;
    normalMap = normalize(normalMap * 2.0f - 1.0f);
    
    // Crear matriz TBN (Tangent, Binormal, Normal)
    float3 tangent = normalize(input.tangent);
    float3 binormal = normalize(input.binormal);
    float3x3 TBN = float3x3(tangent, binormal, normal);
    
    // Transformar la normal del mapa de normales al espacio mundial
    float3 bumpNormal = normalize(mul(normalMap, TBN));
    
    // Calcular iluminación difusa
    float3 lightDirection = normalize(-lightDir);
    float diffuseFactor = max(dot(bumpNormal, lightDirection), 0.1f);
    
    // Aplicar color de luz
    float3 finalColor = diffuseColor.rgb * lightColor * diffuseFactor;
    
    // Agregar un poco de transparencia y reflexión para el efecto de agua
    float alpha = diffuseColor.a * 0.8f;
    
    // Agregar un tinte azulado para agua
    finalColor = lerp(finalColor, finalColor * float3(0.7f, 0.9f, 1.2f), 0.3f);
    
    return float4(finalColor, alpha);
}