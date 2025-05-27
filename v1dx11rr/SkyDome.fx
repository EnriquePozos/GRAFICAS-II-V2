// Skydome.fx - Shader para ciclo día-tarde-noche

// TRES TEXTURAS: Día, Tarde y Noche
Texture2D texturaDia : register(t0);
Texture2D texturaTarde : register(t1);
Texture2D texturaNoche : register(t2);

SamplerState colorSampler : register(s0);

cbuffer MatrixBuffer
{
    matrix worldMatrix;
    matrix viewMatrix;
    matrix projMatrix;
    float4 valores; // valores.x = tiempo del día [0.0 = Día, 0.5 = Tarde, 1.0 = Noche]
};

struct VS_Input
{
    float4 pos : POSITION;
    float2 tex0 : TEXCOORD0;
};

struct PS_Input
{
    float4 pos : SV_POSITION;
    float2 tex0 : TEXCOORD0;
    float timeOfDay : TEXCOORD1; // Pasar el tiempo al pixel shader
};

PS_Input VS_Main(VS_Input vertex)
{
    PS_Input vsOut = (PS_Input) 0;
    vsOut.pos = mul(vertex.pos, worldMatrix);
    vsOut.pos = mul(vsOut.pos, viewMatrix);
    vsOut.pos = mul(vsOut.pos, projMatrix);
    vsOut.tex0 = vertex.tex0;
    vsOut.timeOfDay = valores.x;
    return vsOut;
}

float4 PS_Main(PS_Input pix) : SV_TARGET
{
    float4 colorFinal;

    float4 colorDia = texturaDia.Sample(colorSampler, pix.tex0);
    float4 colorTarde = texturaTarde.Sample(colorSampler, pix.tex0);
    float4 colorNoche = texturaNoche.Sample(colorSampler, pix.tex0);

    // Interpolar según el tiempo del día
    if (pix.timeOfDay < 0.5f)
    {
        // Día a Tarde (0.0 a 0.5)
        float factor = pix.timeOfDay / 0.5f; // Normalizar a [0,1]
        colorFinal = lerp(colorDia, colorTarde, factor);
    }
    else
    {
        // Tarde a Noche (0.5 a 1.0)
        float factor = (pix.timeOfDay - 0.5f) / 0.5f; // Normalizar a [0,1]
        colorFinal = lerp(colorTarde, colorNoche, factor);
    }

    return colorFinal;
}