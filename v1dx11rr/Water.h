#ifndef _water
#define _water

//#define _XM_NO_INTRINSICS_

#include <d3d11.h>
#include <d3dx11.h>
#include <DxErr.h>
#include <D3Dcompiler.h>
#include <d3dx10math.h>
#include <xnamath.h>

class WaterRR {
private:
	struct VertexComponent
	{
		D3DXVECTOR3 pos;
		D3DXVECTOR2 UV;
		//D3DXVECTOR2 UV2;
		D3DXVECTOR3 normal;
		D3DXVECTOR3 tangente;
		D3DXVECTOR3 binormal;
	};

	struct VertexCollide
	{
		D3DXVECTOR3 pos;
	};

	ID3D11VertexShader* VertexShaderVS;
	ID3D11PixelShader* solidColorPS;

	ID3D11InputLayout* inputLayout;
	ID3D11Buffer* vertexBuffer;
	ID3D11Buffer* indexBuffer;

	ID3D11ShaderResourceView* colorMap;
	ID3D11ShaderResourceView* colorMapN;
	ID3D11SamplerState* colorMapSampler;

	ID3D11Buffer* viewCB;
	ID3D11Buffer* projCB;
	ID3D11Buffer* worldCB;
	ID3D11Buffer* lightDirectionCB;
	ID3D11Buffer* lightColorCB;
	ID3D11Buffer* texUvCB;
	D3DXMATRIX viewMatrix;
	D3DXMATRIX projMatrix;

	int ancho, alto;
	int anchoTexTerr, altoTexTerr;
	float anchof, altof;
	float deltax, deltay;
	float box[4];

	UINT* indices;
	VertexComponent* vertices;
	VertexCollide* vertcol;
	BYTE** alturaData;
	ID3D11Device* d3dDevice;
	ID3D11DeviceContext* d3dContext;

public:
	float* getBox() {
		box[0] = -ancho / 2 + 25;
		box[1] = -alto / 2 + 25;
		box[2] = ancho - 50;
		box[3] = alto - 50;

		return box;
	}

	WaterRR(int ancho, int alto, ID3D11Device* D3DDevice, ID3D11DeviceContext* D3DContext, WCHAR* textura, WCHAR* texturaN)
	{
		//copiamos el device y el device context a la clase terreno
		d3dContext = D3DContext;
		d3dDevice = D3DDevice;
		//este es el ancho y el alto del terreno en su escala
		this->ancho = ancho;
		this->alto = alto;
		//aqui cargamos las texturas de alturas y el cesped
		CargaParametros(textura, texturaN, 1.0f);
	}

	~WaterRR()
	{
		//libera recursos
		delete vertcol, vertices;
		UnloadContent();
	}

	bool CompileD3DShader(WCHAR* filePath, char* entry, char* shaderModel, ID3DBlob** buffer)
	{
		//forma de compilar el shader
		DWORD shaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;

		ID3DBlob* errorBuffer = 0;
		HRESULT result;

		result = D3DX11CompileFromFile(filePath, 0, 0, entry, shaderModel, shaderFlags,
			0, 0, buffer, &errorBuffer, 0);
		if (FAILED(result))
		{
			if (errorBuffer != 0)
			{
				OutputDebugStringA((char*)errorBuffer->GetBufferPointer());
				errorBuffer->Release();
			}
			return false;
		}

		if (errorBuffer != 0)
			errorBuffer->Release();

		return true;
	}

	bool CargaParametros(WCHAR* diffuseTex, WCHAR* diffuseTexN, float tile)
	{
		HRESULT d3dResult;
		//carga el mapa de alturas

		ID3DBlob* vsBuffer = 0;

		//cargamos el shaders de vertices que esta contenido en el Shader.fx, note
		//que VS_Main es el nombre del vertex shader en el shader, vsBuffer contendra
		//al puntero del mismo
		bool compileResult = CompileD3DShader(L"Water.fx", "VS_Main", "vs_4_0", &vsBuffer);
		//en caso de no poder cargarse ahi muere la cosa
		if (compileResult == false)
		{
			return false;
		}

		//aloja al shader en la memeoria del gpu o el cpu
		d3dResult = d3dDevice->CreateVertexShader(vsBuffer->GetBufferPointer(),
			vsBuffer->GetBufferSize(), 0, &VertexShaderVS);
		//en caso de falla sale
		if (FAILED(d3dResult))
		{

			if (vsBuffer)
				vsBuffer->Release();

			return false;
		}

		//lo nuevo, antes creabamos una estructura con la definicion del vertice, ahora creamos un mapa o layout
		//de como queremos construir al vertice, esto es la definicion
		D3D11_INPUT_ELEMENT_DESC solidColorLayout[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 1, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{ "NORMAL", 1, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{ "NORMAL", 2, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0}
		};

		unsigned int totalLayoutElements = ARRAYSIZE(solidColorLayout);

		d3dResult = d3dDevice->CreateInputLayout(solidColorLayout, totalLayoutElements,
			vsBuffer->GetBufferPointer(), vsBuffer->GetBufferSize(), &inputLayout);

		vsBuffer->Release();

		if (FAILED(d3dResult))
		{
			return false;
		}

		ID3DBlob* psBuffer = 0;
		// de los vertices pasamos al pixel shader, note que el nombre del shader es el mismo
		//ahora buscamos al pixel shader llamado PS_Main
		compileResult = CompileD3DShader(L"Water.fx", "PS_Main", "ps_4_0", &psBuffer);

		if (compileResult == false)
		{
			return false;
		}
		//ya compilado y sin error lo ponemos en la memoria
		d3dResult = d3dDevice->CreatePixelShader(psBuffer->GetBufferPointer(),
			psBuffer->GetBufferSize(), 0, &solidColorPS);

		psBuffer->Release();

		if (FAILED(d3dResult))
		{
			return false;
		}

		//lo de siempre, para centrarlo 
		anchof = (float)(ancho / 2.0f);
		altof = (float)(alto / 2.0f);
		//el tile o mosaiqueo
		anchoTexTerr = 2;
		altoTexTerr = 2;
		float du = tile / (float)(anchoTexTerr);
		float dv = tile / (float)(altoTexTerr);

		//cantidad de vertices
		int cuenta = anchoTexTerr * altoTexTerr;
		vertices = new VertexComponent[cuenta];
		vertcol = new VertexCollide[cuenta];

		// Se obtiene el espaciado entre cada vertice
		deltay = (float)alto / (float)altoTexTerr;
		deltax = (float)ancho / (float)anchoTexTerr;

		for (int x = 0; x < altoTexTerr; x++)
		{
			for (int y = 0; y < anchoTexTerr; y++)
			{
				int indiceArreglo = x * anchoTexTerr + y;

				// Se calculan los vertices 'x' y 'z'. 'Y' se saca del mapa de normales
				vertcol[indiceArreglo].pos.x = vertices[indiceArreglo].pos.x = deltax * y - anchof;
				vertcol[indiceArreglo].pos.y = vertices[indiceArreglo].pos.y = 0;
				vertcol[indiceArreglo].pos.z = vertices[indiceArreglo].pos.z = deltay * x - altof;
				vertices[indiceArreglo].UV.x = y * du;
				vertices[indiceArreglo].UV.y = x * dv;
				//note las operaciones matematicas empiezan con XM, son herencia del XNA
				vertices[indiceArreglo].normal = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
				vertices[indiceArreglo].tangente = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
				vertices[indiceArreglo].binormal = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
			}
		}

		//una vez creados los vertices generamos las normales
		generaNormales(vertices);

		//proceso de guardar el buffer de vertices para su uso en el render
		D3D11_BUFFER_DESC vertexDesc;
		ZeroMemory(&vertexDesc, sizeof(vertexDesc));
		vertexDesc.Usage = D3D11_USAGE_DEFAULT;
		vertexDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		vertexDesc.ByteWidth = sizeof(VertexComponent) * cuenta;

		D3D11_SUBRESOURCE_DATA resourceData;
		ZeroMemory(&resourceData, sizeof(resourceData));
		resourceData.pSysMem = vertices;

		d3dResult = d3dDevice->CreateBuffer(&vertexDesc, &resourceData, &vertexBuffer);

		if (FAILED(d3dResult))
		{
			MessageBox(0, L"Error", L"Error al crear vertex buffer", MB_OK);
			return false;
		}
		//ya una vez pasados los vertices al buffer borramos el arreglo donde los habiamos creado inicialmente
		//delete vertices;

		//creamos los indices para hacer el terreno
		estableceIndices();
		//crea los accesos de las texturas para los shaders 
		d3dResult = D3DX11CreateShaderResourceViewFromFile(d3dDevice, diffuseTex, 0, 0, &colorMap, 0);
		d3dResult = D3DX11CreateShaderResourceViewFromFile(d3dDevice, diffuseTexN, 0, 0, &colorMapN, 0);

		if (FAILED(d3dResult))
		{
			return false;
		}
		//aqui creamos el sampler
		D3D11_SAMPLER_DESC colorMapDesc;
		ZeroMemory(&colorMapDesc, sizeof(colorMapDesc));
		colorMapDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
		colorMapDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
		colorMapDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
		colorMapDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
		colorMapDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		colorMapDesc.MaxLOD = D3D11_FLOAT32_MAX;
		//con la estructura de descripion creamos el sampler
		d3dResult = d3dDevice->CreateSamplerState(&colorMapDesc, &colorMapSampler);

		if (FAILED(d3dResult))
		{
			return false;
		}

		//creamos los buffers para el shader para poder pasarle las matrices
		D3D11_BUFFER_DESC constDesc;
		ZeroMemory(&constDesc, sizeof(constDesc));
		constDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		constDesc.ByteWidth = sizeof(D3DXMATRIX);
		constDesc.Usage = D3D11_USAGE_DEFAULT;
		//de vista
		d3dResult = d3dDevice->CreateBuffer(&constDesc, 0, &viewCB);

		if (FAILED(d3dResult))
		{
			return false;
		}
		//de proyeccion
		d3dResult = d3dDevice->CreateBuffer(&constDesc, 0, &projCB);

		if (FAILED(d3dResult))
		{
			return false;
		}
		//de mundo
		d3dResult = d3dDevice->CreateBuffer(&constDesc, 0, &worldCB);

		if (FAILED(d3dResult))
		{
			return false;
		}

		d3dResult = d3dDevice->CreateBuffer(&constDesc, 0, &lightDirectionCB);

		if (FAILED(d3dResult))
		{
			return false;
		}

		d3dResult = d3dDevice->CreateBuffer(&constDesc, 0, &lightColorCB);

		if (FAILED(d3dResult))
		{
			return false;
		}

		d3dResult = d3dDevice->CreateBuffer(&constDesc, 0, &texUvCB);

		if (FAILED(d3dResult))
		{
			return false;
		}

		//posicion de la camara
		D3DXVECTOR3 eye = D3DXVECTOR3(0.0f, 100.0f, 200.0f);
		//a donde ve
		D3DXVECTOR3 target = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
		D3DXVECTOR3 up = D3DXVECTOR3(0.0f, 1.0f, 0.0f);

		//crea la matriz de vista
		D3DXMatrixLookAtLH(&viewMatrix, &eye, &target, &up);
		//la de proyeccion
		D3DXMatrixPerspectiveFovLH(&projMatrix, D3DX_PI / 4.0, ancho / alto, 0.01f, 1000.0f);
		//las transpone para acelerar la multiplicacion
		D3DXMatrixTranspose(&viewMatrix, &viewMatrix);
		D3DXMatrixTranspose(&projMatrix, &projMatrix);

		return true;
	}

	bool UnloadContent()
	{
		if (colorMapSampler)
			colorMapSampler->Release();
		if (colorMap)
			colorMap->Release();
		if (colorMapN)
			colorMapN->Release();
		if (VertexShaderVS)
			VertexShaderVS->Release();
		if (solidColorPS)
			solidColorPS->Release();
		if (inputLayout)
			inputLayout->Release();
		if (vertexBuffer)
			vertexBuffer->Release();
		if (viewCB)
			viewCB->Release();
		if (projCB)
			projCB->Release();
		if (worldCB)
			worldCB->Release();
		if (lightDirectionCB)
			lightDirectionCB->Release();
		if (lightColorCB)
			lightColorCB->Release();
		if (texUvCB)
			texUvCB->Release();
		if (alturaData)
		{
			for (int i = 0; i < altoTexTerr; i++)
			{
				delete alturaData[i];
			}
			delete alturaData;
		}
		alturaData = 0;

		colorMapSampler = 0;
		colorMap = 0;
		colorMapN = 0;
		VertexShaderVS = 0;
		solidColorPS = 0;
		inputLayout = 0;
		vertexBuffer = 0;
		indexBuffer = 0;
		viewCB = 0;
		projCB = 0;
		worldCB = 0;
		lightDirectionCB = 0;
		lightColorCB = 0;
		texUvCB = 0;
	}

	void Update(float dt)
	{
	}

	void Draw(D3DXMATRIX vista, D3DXMATRIX proyeccion, float xpos, float ypos, float zpos, float velocidad, XMFLOAT3 LightDirection, XMFLOAT3 LightColor)
	{
		if (vertexBuffer)
			vertexBuffer->Release();
		vertexBuffer = NULL;

		static float rotation = 0.0f;
		rotation += 0.01;

		static float water_factor_min = 20.0f;
		static float water_factor_max = 30.0f;
		water_factor_min += velocidad;
		water_factor_max += velocidad;
		if (water_factor_min > 40)
		{
			water_factor_min = 20.0f;
			water_factor_max = 30.0f;
		}
		XMFLOAT2 texUV = XMFLOAT2(water_factor_min, water_factor_max);


		vertices[0].UV.x = water_factor_max;
		vertices[0].UV.y = water_factor_max;

		vertices[1].UV.x = water_factor_max;
		vertices[1].UV.y = water_factor_min;

		vertices[2].UV.x = water_factor_min;
		vertices[2].UV.y = water_factor_max;

		vertices[3].UV.x = water_factor_min;
		vertices[3].UV.y = water_factor_min;


		//proceso de guardar el buffer de vertices para su uso en el render
		D3D11_BUFFER_DESC vertexDesc;
		ZeroMemory(&vertexDesc, sizeof(vertexDesc));
		vertexDesc.Usage = D3D11_USAGE_DEFAULT;
		vertexDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		vertexDesc.ByteWidth = sizeof(VertexComponent) * 4;

		D3D11_SUBRESOURCE_DATA resourceData;
		ZeroMemory(&resourceData, sizeof(resourceData));
		resourceData.pSysMem = vertices;

		d3dDevice->CreateBuffer(&vertexDesc, &resourceData, &vertexBuffer);


		//paso de datos, es decir cuanto es el ancho de la estructura
		unsigned int stride = sizeof(VertexComponent);
		unsigned int offset = 0;

		//define la estructura del vertice a traves de layout
		d3dContext->IASetInputLayout(inputLayout);

		//define con que buffer trabajara
		d3dContext->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);
		//define con buffer de indices trabajara
		d3dContext->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R32_UINT, 0);
		//define la forma de conexion de los vertices
		d3dContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		//Establece el vertex y pixel shader que utilizara
		d3dContext->VSSetShader(VertexShaderVS, 0, 0);
		d3dContext->PSSetShader(solidColorPS, 0, 0);
		//pasa lo sbuffers al shader
		d3dContext->PSSetShaderResources(0, 1, &colorMap);
		d3dContext->PSSetShaderResources(1, 1, &colorMapN);
		d3dContext->PSSetSamplers(0, 1, &colorMapSampler);

		//mueve la camara
		D3DXMATRIX rotationMat;
		D3DXMatrixRotationYawPitchRoll(&rotationMat, 0.0f, 0.0f, 0.0f);
		D3DXMATRIX translationMat;
		D3DXMatrixTranslation(&translationMat, xpos, ypos, zpos);
		D3DXMATRIX ry;
		D3DXMatrixRotationY(&ry, 0.01);
		viewMatrix *= ry;

		D3DXMATRIX worldMat = rotationMat * translationMat;
		D3DXMatrixTranspose(&worldMat, &worldMat);
		//actualiza los buffers del shader
		d3dContext->UpdateSubresource(worldCB, 0, 0, &worldMat, 0, 0);
		d3dContext->UpdateSubresource(viewCB, 0, 0, &vista, 0, 0);
		d3dContext->UpdateSubresource(projCB, 0, 0, &proyeccion, 0, 0);
		d3dContext->UpdateSubresource(lightDirectionCB, 0, 0, &LightDirection, 0, 0);
		d3dContext->UpdateSubresource(lightColorCB, 0, 0, &LightColor, 0, 0);
		d3dContext->UpdateSubresource(texUvCB, 0, 0, &texUV, 0, 0);
		//le pasa al shader los buffers
		d3dContext->VSSetConstantBuffers(0, 1, &worldCB);
		d3dContext->VSSetConstantBuffers(1, 1, &viewCB);
		d3dContext->VSSetConstantBuffers(2, 1, &projCB);
		d3dContext->PSSetConstantBuffers(0, 1, &lightDirectionCB);
		d3dContext->PSSetConstantBuffers(1, 1, &lightColorCB);
		d3dContext->VSSetConstantBuffers(3, 1, &texUvCB);
		//cantidad de trabajos
		int cuenta = (anchoTexTerr - 1) * (altoTexTerr - 1) * 6;
		d3dContext->DrawIndexed(cuenta, 0, 0);
	}

private:
	void estableceIndices()
	{
		HRESULT d3dResult;
		int cuenta = (anchoTexTerr - 1) * (altoTexTerr - 1) * 6;
		indices = new UINT[cuenta];

		int counter = 0;
		for (int y = 0; y < altoTexTerr - 1; y++)
		{
			for (int x = 0; x < anchoTexTerr - 1; x++)
			{
				int lowerLeft = y * anchoTexTerr + x;
				int lowerRight = y * anchoTexTerr + (x + 1);
				int topLeft = (y + 1) * anchoTexTerr + x;
				int topRight = (y + 1) * anchoTexTerr + (x + 1);

				indices[counter++] = lowerLeft;
				indices[counter++] = topLeft;
				indices[counter++] = lowerRight;

				indices[counter++] = lowerRight;
				indices[counter++] = topLeft;
				indices[counter++] = topRight;
			}
		}

		D3D11_BUFFER_DESC indexDesc;
		ZeroMemory(&indexDesc, sizeof(indexDesc));
		indexDesc.Usage = D3D11_USAGE_DEFAULT;
		indexDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
		indexDesc.ByteWidth = sizeof(INT) * cuenta;
		indexDesc.CPUAccessFlags = 0;

		D3D11_SUBRESOURCE_DATA resourceData;
		ZeroMemory(&resourceData, sizeof(resourceData));
		resourceData.pSysMem = indices;

		d3dResult = d3dDevice->CreateBuffer(&indexDesc, &resourceData, &indexBuffer);

		if (FAILED(d3dResult))
		{
			return;
		}
		delete indices;
	}

	void generaNormales(VertexComponent* vertices)
	{
		for (int i = 0; i < altoTexTerr - 1; i++)
		{
			for (int j = 0; j < anchoTexTerr - 1; j++)
			{
				int bottomleft = i * anchoTexTerr + j;
				int topright = (i + 1) * anchoTexTerr + (j + 1);
				int topleft = (i + 1) * anchoTexTerr + j;
				int bottomright = i * anchoTexTerr + (j + 1);

				// Se obtiene la normal del primer triangulo
				D3DXVECTOR3 v1 = vertices[topleft].pos - vertices[bottomleft].pos;
				D3DXVECTOR3 v2 = vertices[bottomright].pos - vertices[bottomleft].pos;
				D3DXVECTOR3 T1;
				D3DXVec3Cross(&T1, &v1, &v2);
				D3DXVECTOR3 sumNor;
				sumNor = T1;
				vertices[bottomleft].normal = D3DXVECTOR3(vertices[bottomleft].normal.x + sumNor.x,
					vertices[bottomleft].normal.y + sumNor.y,
					vertices[bottomleft].normal.z + sumNor.z);
				vertices[topleft].normal = D3DXVECTOR3(vertices[topleft].normal.x + sumNor.x,
					vertices[topleft].normal.y + sumNor.y,
					vertices[topleft].normal.z + sumNor.z);
				vertices[bottomright].normal = D3DXVECTOR3(vertices[bottomright].normal.x + sumNor.x,
					vertices[bottomright].normal.y + sumNor.y,
					vertices[bottomright].normal.z + sumNor.z);

				D3DXVECTOR3 tang;
				tang = vertices[bottomright].pos - vertices[bottomleft].pos;
				vertices[bottomleft].tangente += tang;
				vertices[topleft].tangente += tang;
				vertices[bottomright].tangente += tang;

				v1 = vertices[topleft].pos - vertices[bottomright].pos;
				v2 = vertices[topright].pos - vertices[bottomright].pos;
				D3DXVec3Cross(&T1, &v1, &v2);
				sumNor = T1;

				vertices[topright].normal = D3DXVECTOR3(vertices[topright].normal.x + sumNor.x,
					vertices[topright].normal.y + sumNor.y,
					vertices[topright].normal.z + sumNor.z);
				vertices[topleft].normal = D3DXVECTOR3(vertices[topleft].normal.x + sumNor.x,
					vertices[topleft].normal.y + sumNor.y,
					vertices[topleft].normal.z + sumNor.z);
				vertices[bottomright].normal = D3DXVECTOR3(vertices[bottomright].normal.x + sumNor.x,
					vertices[bottomright].normal.y + sumNor.y,
					vertices[bottomright].normal.z + sumNor.z);

				tang = vertices[topright].pos - vertices[topleft].pos;
				vertices[topright].tangente += tang;
				vertices[topleft].tangente += tang;
				vertices[bottomright].tangente += tang;
			}
		}

		for (int i = 0; i < anchoTexTerr * altoTexTerr; i++)
		{
			D3DXVECTOR3 normal = vertices[i].normal;
			D3DXVec3Normalize(&normal, &normal);
			vertices[i].normal = normal;

			D3DXVECTOR3 tangent = vertices[i].tangente;
			D3DXVec3Normalize(&tangent, &tangent);
			vertices[i].tangente = tangent;
		}

		for (int i = 0; i < anchoTexTerr * altoTexTerr; i++)
		{
			D3DXVECTOR3 binorm;
			D3DXVec3Cross(&binorm, &vertices[i].tangente, &vertices[i].normal);
			D3DXVec3Normalize(&binorm, &binorm);
			vertices[i].binormal = binorm;
		}
	}

};
#endif

