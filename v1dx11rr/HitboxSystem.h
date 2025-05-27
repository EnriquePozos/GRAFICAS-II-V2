#pragma once
#pragma comment(lib, "D3DCompiler.lib")
#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>


#include <vector>
#include <xnamath.h>

#include "Box.h"

class HitboxSystem
{
private:
  std::vector<Hitbox> hitboxes;
public:

  HitboxSystem()
  {
    
  }

  void RegisterHitbox(const Hitbox& hitbox)
  {
    hitboxes.push_back(hitbox);
  }

  void RegisterHitbox(const Box& box) {
    hitboxes.emplace_back(box);
  }

  bool CheckCollision(const D3DXVECTOR3& position) const {
    for (const auto& hitbox : hitboxes) {
      if (hitbox.active && hitbox.ContainsPoint(position)) {
        return true;
      }
    }
    return false;
  }

  std::vector<Hitbox> GetHitboxes() const
  {
    return hitboxes;
  }

  Hitbox* GetFirstCollidingHitbox(const D3DXVECTOR3& position) {
    for (auto& hitbox : hitboxes) {
      if (hitbox.active && hitbox.ContainsPoint(position)) {
        return &hitbox;
      }
    }
    return nullptr;
  }

  std::vector<Hitbox*> GetAllCollidingHitboxes(const D3DXVECTOR3& position) {
    std::vector<Hitbox*> colliding;
    for (auto& hitbox : hitboxes) {
      if (hitbox.active && hitbox.ContainsPoint(position)) {
        colliding.push_back(&hitbox);
      }
    }
    return colliding;
  }

  void Clear() {
    hitboxes.clear();
  }

  size_t GetHitboxCount() const {
    return hitboxes.size();
  }

  Hitbox& GetHitbox(size_t index) {
    return hitboxes[index];
  }
    
  const Hitbox& GetHitbox(size_t index) const {
    return hitboxes[index];
  }
  
};






class HitboxRenderer
{
private:
  struct Vertex {
    XMFLOAT3 position;
    XMFLOAT4 color;
  };

  struct ConstantBuffer {
    D3DXMATRIX worldViewProjection;
    D3DXVECTOR4 color;
  };

  ID3D11Device* device;
  ID3D11DeviceContext* context;
  ID3D11Buffer* vertexBuffer;
  ID3D11Buffer* indexBuffer;
  ID3D11Buffer* constantBuffer;
  ID3D11VertexShader* vertexShader;
  ID3D11PixelShader* pixelShader;
  ID3D11InputLayout* inputLayout;
  ID3D11RasterizerState* wireframeState;
  ID3D11BlendState* blendState;

  ID3D11Buffer* viewCB;
  ID3D11Buffer* projCB;
  ID3D11Buffer* worldCB;
  D3DXMATRIX viewMatrix;
  D3DXMATRIX projMatrix;

  // Shaders como strings
  const char* vertexShaderSource = R"(
        cbuffer ConstantBuffer : register(b0)
        {
            matrix WorldViewProjection;
            float4 Color;
        }

        struct VS_INPUT
        {
            float3 Position : POSITION;
            float4 Color : COLOR;
        };

        struct VS_OUTPUT
        {
            float4 Position : SV_POSITION;
            float4 Color : COLOR;
        };

        VS_OUTPUT main(VS_INPUT input)
        {
            VS_OUTPUT output;
            output.Position = mul(float4(input.Position, 1.0f), WorldViewProjection);
            output.Color = Color;
            return output;
        }
    )";

  const char* pixelShaderSource = R"(
        struct PS_INPUT
        {
            float4 Position : SV_POSITION;
            float4 Color : COLOR;
        };

        float4 main(PS_INPUT input) : SV_TARGET
        {
            return input.Color;
        }
    )";

public:
  HitboxRenderer(ID3D11Device* d3dDevice, ID3D11DeviceContext* d3dContext) 
      : device(d3dDevice), context(d3dContext), vertexBuffer(nullptr), 
        indexBuffer(nullptr), constantBuffer(nullptr), vertexShader(nullptr),
        pixelShader(nullptr), inputLayout(nullptr), wireframeState(nullptr),
        blendState(nullptr) {
    Initialize();
  }

  ~HitboxRenderer() {
    Cleanup();
  }

  void Initialize() {
    CreateShaders();
    CreateBuffers();
    CreateStates();
  }

  void CreateShaders() {
    // Compilar Vertex Shader
    ID3DBlob* vsBlob = nullptr;
    ID3DBlob* errorBlob = nullptr;
        
    HRESULT hr = D3DCompile(vertexShaderSource, strlen(vertexShaderSource), 
                           nullptr, nullptr, nullptr, "main", "vs_4_0", 
                           0, 0, &vsBlob, &errorBlob);
        
    if (SUCCEEDED(hr)) {
      device->CreateVertexShader(vsBlob->GetBufferPointer(), 
                               vsBlob->GetBufferSize(), 
                               nullptr, &vertexShader);

      // Input Layout
      D3D11_INPUT_ELEMENT_DESC layout[] = {
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0}
      };

      device->CreateInputLayout(layout, 2, vsBlob->GetBufferPointer(), 
                              vsBlob->GetBufferSize(), &inputLayout);
    }

    if (vsBlob) vsBlob->Release();
    if (errorBlob) errorBlob->Release();

    // Compilar Pixel Shader
    ID3DBlob* psBlob = nullptr;
    hr = D3DCompile(pixelShaderSource, strlen(pixelShaderSource), 
                   nullptr, nullptr, nullptr, "main", "ps_4_0", 
                   0, 0, &psBlob, &errorBlob);
        
    if (SUCCEEDED(hr)) {
      device->CreatePixelShader(psBlob->GetBufferPointer(), 
                              psBlob->GetBufferSize(), 
                              nullptr, &pixelShader);
    }

    if (psBlob) psBlob->Release();
    if (errorBlob) errorBlob->Release();
  }

  void CreateBuffers() {
    // Crear vertex buffer para un cubo (wireframe)
    Vertex cubeVertices[8] = {
      {XMFLOAT3(-0.5f, -0.5f, -0.5f), XMFLOAT4(1,1,1,1)},
      {XMFLOAT3( 0.5f, -0.5f, -0.5f), XMFLOAT4(1,1,1,1)},
      {XMFLOAT3( 0.5f,  0.5f, -0.5f), XMFLOAT4(1,1,1,1)},
      {XMFLOAT3(-0.5f,  0.5f, -0.5f), XMFLOAT4(1,1,1,1)},
      {XMFLOAT3(-0.5f, -0.5f,  0.5f), XMFLOAT4(1,1,1,1)},
      {XMFLOAT3( 0.5f, -0.5f,  0.5f), XMFLOAT4(1,1,1,1)},
      {XMFLOAT3( 0.5f,  0.5f,  0.5f), XMFLOAT4(1,1,1,1)},
      {XMFLOAT3(-0.5f,  0.5f,  0.5f), XMFLOAT4(1,1,1,1)}
    };

    D3D11_BUFFER_DESC vertexBufferDesc = {};
    vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
    vertexBufferDesc.ByteWidth = sizeof(cubeVertices);
    vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

    D3D11_SUBRESOURCE_DATA vertexData = {};
    vertexData.pSysMem = cubeVertices;

    device->CreateBuffer(&vertexBufferDesc, &vertexData, &vertexBuffer);

    // Índices para wireframe (líneas)
    WORD cubeIndices[24] = {
      // Bottom face
      0,1, 1,2, 2,3, 3,0,
      // Top face  
      4,5, 5,6, 6,7, 7,4,
      // Vertical edges
      0,4, 1,5, 2,6, 3,7
  };

    D3D11_BUFFER_DESC indexBufferDesc = {};
    indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
    indexBufferDesc.ByteWidth = sizeof(cubeIndices);
    indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;

    D3D11_SUBRESOURCE_DATA indexData = {};
    indexData.pSysMem = cubeIndices;

    device->CreateBuffer(&indexBufferDesc, &indexData, &indexBuffer);

    // Constant Buffer
    D3D11_BUFFER_DESC cbDesc = {};
    cbDesc.Usage = D3D11_USAGE_DEFAULT;
    cbDesc.ByteWidth = sizeof(ConstantBuffer);
    cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;

    device->CreateBuffer(&cbDesc, nullptr, &constantBuffer);
  }

  void CreateStates() {
    // Wireframe Rasterizer State
    D3D11_RASTERIZER_DESC wireframeDesc = {};
    wireframeDesc.FillMode = D3D11_FILL_WIREFRAME;
    wireframeDesc.CullMode = D3D11_CULL_NONE;
    wireframeDesc.FrontCounterClockwise = false;
    wireframeDesc.DepthBias = 0;
    wireframeDesc.DepthBiasClamp = 0.0f;
    wireframeDesc.SlopeScaledDepthBias = 0.0f;
    wireframeDesc.DepthClipEnable = true;
    wireframeDesc.ScissorEnable = false;
    wireframeDesc.MultisampleEnable = false;
    wireframeDesc.AntialiasedLineEnable = true;

    device->CreateRasterizerState(&wireframeDesc, &wireframeState);

    // Blend State para transparencia
    D3D11_BLEND_DESC blendDesc = {};
    blendDesc.RenderTarget[0].BlendEnable = true;
    blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
    blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
    blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
    blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
    blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
    blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
    blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

    device->CreateBlendState(&blendDesc, &blendState);
  }

  void RenderHitbox(const Hitbox& hitbox, const D3DXMATRIX& view, const D3DXMATRIX& projection,
                   const D3DXVECTOR4& color = D3DXVECTOR4(1.0f, 0.0f, 0.0f, 0.5f)) {
    if (!hitbox.active) return;

    D3DXVECTOR3 pos = hitbox.GetPosition();
    D3DXVECTOR3 size = hitbox.GetSize();

    // Matrices de transformación
    D3DXMATRIX scaleMat, translationMat, worldMat;

    D3DXMatrixScaling(&scaleMat, size.x, size.y, size.z);
    D3DXMatrixTranslation(&translationMat, pos.x, pos.y, pos.z);
    worldMat = scaleMat * translationMat;

    // Transponer matrices para el shader (si lo requiere)
    D3DXMatrixTranspose(&worldMat, &worldMat);
    D3DXMATRIX viewT = view;
    D3DXMATRIX projT = projection;
    D3DXMatrixTranspose(&viewT, &viewT);
    D3DXMatrixTranspose(&projT, &projT);

    // Actualizar constant buffers
    context->UpdateSubresource(worldCB, 0, nullptr, &worldMat, 0, 0);
    context->UpdateSubresource(viewCB, 0, nullptr, &viewT, 0, 0);
    context->UpdateSubresource(projCB, 0, nullptr, &projT, 0, 0);
    // context->UpdateSubresource(colorCB, 0, nullptr, &color, 0, 0);

    // Configurar pipeline
    UINT stride = sizeof(Vertex);
    UINT offset = 0;
    context->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);
    context->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R16_UINT, 0);
    context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
    context->IASetInputLayout(inputLayout);

    context->VSSetShader(vertexShader, nullptr, 0);
    context->PSSetShader(pixelShader, nullptr, 0);

    context->VSSetConstantBuffers(0, 1, &worldCB);
    context->VSSetConstantBuffers(1, 1, &viewCB);
    context->VSSetConstantBuffers(2, 1, &projCB);
    // context->PSSetConstantBuffers(0, 1, &colorCB);

    context->RSSetState(wireframeState);
    context->OMSetBlendState(blendState, nullptr, 0xffffffff);

    // Dibujar
    context->DrawIndexed(24, 0, 0);
  }


  void RenderBoxHitbox(const Box& box, ID3D11Device* device, ID3D11DeviceContext* context, const D3DXMATRIX& viewMatrix, const D3DXMATRIX& projMatrix) {
    std::vector<D3DXVECTOR3> vertices = box.GetVertices();

    std::vector<D3DXVECTOR3> lines = {
      vertices[0], vertices[1], vertices[1], vertices[2],
      vertices[2], vertices[3], vertices[3], vertices[0],
      vertices[4], vertices[5], vertices[5], vertices[6],
      vertices[6], vertices[7], vertices[7], vertices[4],
      vertices[0], vertices[4], vertices[1], vertices[5],
      vertices[2], vertices[6], vertices[3], vertices[7],
  };

    D3D11_BUFFER_DESC bufferDesc = {};
    bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    bufferDesc.ByteWidth = sizeof(D3DXVECTOR3) * static_cast<UINT>(lines.size());
    bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

    D3D11_SUBRESOURCE_DATA initData = {};
    initData.pSysMem = lines.data();

    ID3D11Buffer* vertexBuffer = nullptr;
    device->CreateBuffer(&bufferDesc, &initData, &vertexBuffer);

    UINT stride = sizeof(D3DXVECTOR3);
    UINT offset = 0;
    context->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);
    context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);


    context->Draw(static_cast<UINT>(lines.size()), 0);

    vertexBuffer->Release();
  }


  

  void Cleanup() {
    if (vertexBuffer) { vertexBuffer->Release(); vertexBuffer = nullptr; }
    if (indexBuffer) { indexBuffer->Release(); indexBuffer = nullptr; }
    if (constantBuffer) { constantBuffer->Release(); constantBuffer = nullptr; }
    if (vertexShader) { vertexShader->Release(); vertexShader = nullptr; }
    if (pixelShader) { pixelShader->Release(); pixelShader = nullptr; }
    if (inputLayout) { inputLayout->Release(); inputLayout = nullptr; }
    if (wireframeState) { wireframeState->Release(); wireframeState = nullptr; }
    if (blendState) { blendState->Release(); blendState = nullptr; }
  }

  void RenderAll(HitboxSystem& system, const D3DXMATRIX& view, const D3DXMATRIX& projection)
  {
    for (const Hitbox& hitbox : system.GetHitboxes())
    {
      RenderBoxHitbox(hitbox.box, device, context, view, projection);
    }
  }

};
