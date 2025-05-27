#ifndef _dxrr
#define _dxrr
#define rotacionT 0.0016
#include <d3d11.h>
#include <d3dx11.h>
#include <d3dx10.h>
#include <d3dx10math.h>
#include "TerrenoRR.h"
#include "Camara.h"
#include "SkyDome.h"
#include "Billboard.h"
#include "ModeloRR.h"
#include "XACT3Util.h"
#include "Water.h"
#include "Enemigo.h"
#include "Vehiculo.h"

#include <sstream>
#include <windows.h>


class DXRR{	

private:
	int ancho;
	int alto;

	float gameTime = 0.0f;
	float cycleDuration = 2.0f * 60.0f; // duracion ciclo
public:	

	XACTINDEX cueIndex; // Ya la tienes, puedes renombrarla o usarla si es genérica
	XACTINDEX cuePlayerDamage; // Nueva variable para el sonido de daño
	CXACT3Util m_XACT3;


	HINSTANCE hInstance;
	HWND hWnd;
	HWND hCameraPosLabel;

	D3D_DRIVER_TYPE driverType;
	D3D_FEATURE_LEVEL featureLevel;

	ID3D11Device* d3dDevice;
	ID3D11DeviceContext* d3dContext;
	IDXGISwapChain* swapChain;
	ID3D11RenderTargetView* backBufferTarget;

	ID3D11Texture2D* depthTexture;
	ID3D11DepthStencilView* depthStencilView;

	ID3D11DepthStencilState* depthStencilState;
	ID3D11DepthStencilState* depthStencilDisabledState;

	ID3D11BlendState *alphaBlendState, *commonBlendState;

	int frameBillboard;

	TerrenoRR *terreno;
	SkyDome *skydome;
	WaterRR* water;
	//BillboardRR *billboard;
	BillboardRR* arbolito;
	BillboardRR* arbolito2;
	BillboardRR* grass;
	BillboardRR* grass2;
	BillboardRR* grass3;
	Camara *camara;
	ModeloRR* model;
	ModeloRR*      bodega;
	ModeloRR*  boteBasura;
	ModeloRR*  carroViejo;
	ModeloRR*	  cristal;
	ModeloRR* estacionGas;
	ModeloRR*	   moneda[4];
	ModeloRR*	   piedra;
	ModeloRR*  	 	 pozo;
	ModeloRR*   torreAgua;
	ModeloRR* garage;
	ModeloRR* molino;
	Vehiculo* moto;

	Enemigo* Golem;


	HitboxSystem* hitboxSystem;
	HitboxRenderer* hitboxRenderer;

	float movement;
	float izqder;
	float arriaba;
	float vel;
	bool breakpoint;
	vector2 uv1[32];
	vector2 uv2[32];
	vector2 uv3[32];
	vector2 uv4[32];
	
	vector2 uvArbol1[4];
	vector2 uvArbol2[4];
	vector2 uvArbol3[4];
	vector2 uvArbol4[4];



	XMFLOAT3 lightDirection;
	XMFLOAT3 lightColor;
	float Lightrota;

	//Booleans para jugabilidad
	bool moneda0Agarrada = false;
	bool moneda1Agarrada = false;
	bool moneda2Agarrada = false;
	bool moneda3Agarrada = false;
	bool cristalAgarrado = false;
	int score = 0;

	bool score0Aumentado = false;
	bool score1Aumentado = false;
	bool score2Aumentado = false;
	bool score3Aumentado = false;

	bool jugadorEnMoto = false;

	int vida = 3;

	bool colisionGolem1 = false;
	bool colisionGolem2 = false;
	bool colisionGolem3 = false;

	bool restarVida1 = false;
	bool restarVida2 = false;
	bool restarVida3 = false;

	bool jugadorInvulnerable;
	float tiempoFinInvulnerabilidad;
	float duracionInvulnerabilidad;

    DXRR(HWND hWnd, int Ancho, int Alto)
	{
		breakpoint = false;
		frameBillboard = 0;
		ancho = Ancho;
		alto = Alto;
		driverType = D3D_DRIVER_TYPE_NULL;
		featureLevel = D3D_FEATURE_LEVEL_11_0;
		d3dDevice = 0;
		d3dContext = 0;
		swapChain = 0;
		backBufferTarget = 0;
		IniciaD3D(hWnd);
   	movement = 0;
		izqder = 0;
		arriaba = 0;
		vel = 0.0f;
		Lightrota = 0.0f;
		lightDirection = XMFLOAT3(0.0f, -1.0f, 0.5f);  // Luz desde arriba y ligeramente diagonal
		lightColor = XMFLOAT3(1.0f, 1.0f, 0.9f);       // Luz blanca con toque cálido
		billCargaFuego();
		configurarUVArbolito();


		hitboxSystem = new HitboxSystem();
		hitboxRenderer = new HitboxRenderer(d3dDevice, d3dContext);    	

		camara = new Camara(D3DXVECTOR3(0,80,6), D3DXVECTOR3(0,80,0), D3DXVECTOR3(0,.5f,0), Ancho, Alto, hitboxSystem);
		terreno = new TerrenoRR(256, 256, d3dDevice, d3dContext);
		skydome = new SkyDome(32, 32, 100.0f, &d3dDevice, &d3dContext, L"cielo.jpg", L"cielo_atardecer.jpg", L"cielo_noche.jpg");


		// Inicializar XACT3 y cargar bancos
		if (!m_XACT3.Initialize()) { // CXACT3Util::Initialize() ya llama a XACT3CreateEngine y Initialize
			MessageBox(hWnd, L"Error al inicializar el motor XACT3.", L"Error de Audio", MB_OK | MB_ICONERROR);
			// Considera manejar este error (ej. deshabilitar sonidos)
		}

		// Reemplaza con los nombres y rutas correctas de tus archivos de bancos
		if (!m_XACT3.LoadWaveBank(L"Assets/Sonido/Win/GameSoundsWaveBank.xwb")) {
			MessageBox(hWnd, L"No se pudo cargar el Wave Bank.", L"Error de Audio", MB_OK | MB_ICONERROR);
		}
		if (!m_XACT3.LoadSoundBank(L"Assets/Sonido/Win/GameSoundEffectsSoundBank.xsb")) {
			MessageBox(hWnd, L"No se pudo cargar el Sound Bank.", L"Error de Audio", MB_OK | MB_ICONERROR);
		}

		// Obtener el índice del Cue para el sonido de daño
		if (m_XACT3.m_pSoundBank) { // m_pSoundBank es un miembro público en tu CXACT3Util
			cuePlayerDamage = m_XACT3.m_pSoundBank->GetCueIndex("roblox"); // Usa el nombre exacto del Cue que creaste en XACT3
			if (cuePlayerDamage == XACTINDEX_INVALID) {
				MessageBox(hWnd, L"No se encontró el Cue 'PlayerDamageCue'.", L"Error de Audio", MB_OK | MB_ICONWARNING);
			}
		}
		else {
			cuePlayerDamage = XACTINDEX_INVALID; // Marcar como inválido si no se cargó el sound bank
		}

    //COLLISIONS

    	//House
    	hitboxSystem->RegisterHitbox(Box::FromCoords(-80.12f, 3, -53.04f, -60.35f, 20, -86.69f));

    	// Garage
    	hitboxSystem->RegisterHitbox(Box::FromCoords(37.59f, 3, 6.88f, 54.62f, 15, 6.04f));
    	hitboxSystem->RegisterHitbox(Box::FromCoords(37.68f, 3, -5.03f, 38.25f, 15, -9.86f));
    	hitboxSystem->RegisterHitbox(Box::FromCoords(62.74f, 3, -21.80f, 61.49f, 15, 6.63f));
    	hitboxSystem->RegisterHitbox(Box::FromCoords(59.42f, 3, 6.76f, 61.46f, 15, 6.00f));
    	hitboxSystem->RegisterHitbox(Box::FromCoords(37.62f, 3, -20.66f, 62.33f, 15, -21.81f));


    	hitboxSystem->RegisterHitbox(Box::FromCoords(-63.98f, 3, 50.63f, -83.60f, 70, 70.01f));
    	hitboxSystem->RegisterHitbox(Box::FromCoords(-87.57f, 3, 63.51f, -83.61f, 70, 56.54f));
    	hitboxSystem->RegisterHitbox(Box::FromCoords(-77.35f, 3, 76.57f, -70.89f, 70, 73.51f));

    	hitboxSystem->RegisterHitbox(Box::FromCoords(60.22f, 3, 60.96f, 68.86f, 15, 70.86f));

    	hitboxSystem->RegisterHitbox(Box::FromCoords(-54.17f, 3, 90.61f, -45.31f, 12, 97.24f));

    	hitboxSystem->RegisterHitbox(Box::FromCoords(42.03f, 3, -66.84f, 34.30f, 30, -59.01f));


    	// CAR
    	hitboxSystem->RegisterHitbox(Box::FromCoords(-12.46f, 3, 42.05f, -5.70f, 11, 58.08f));

    	//Rock
    	hitboxSystem->RegisterHitbox(Box::FromCoords(32.53f, 3, 70.80f, 41.39f, 15, 63.39f));

    	//Gazoline
    	hitboxSystem->RegisterHitbox(Box::FromCoords(80.18f, 3, -61.64f, 73.29f, 15, -67.93f));

		


    	
		//billboard = new BillboardRR(L"Assets/Billboards/fuego-anim.png",L"Assets/Billboards/fuego-anim-normal.png", d3dDevice, d3dContext, 5);
		arbolito = new BillboardRR(L"Assets/Billboards/arbolito.png", L"Assets/Billboards/arbolito.png", d3dDevice, d3dContext, 5);
		arbolito2 = new BillboardRR(L"Assets/Billboards/arbolito2.png", L"Assets/Billboards/arbolito2.png", d3dDevice, d3dContext, 5);
		grass = new BillboardRR(L"Assets/Billboards/grass.png", L"Assets/Billboards/grass.png", d3dDevice, d3dContext, 5);
		grass2 = new BillboardRR(L"Assets/Billboards/grass2.png", L"Assets/Billboards/grass2.png", d3dDevice, d3dContext, 5);
		grass3 = new BillboardRR(L"Assets/Billboards/grass3.png", L"Assets/Billboards/grass3.png", d3dDevice, d3dContext, 5);
		//model = new ModeloRR(d3dDevice, d3dContext, "Assets/Cofre/Cofre.obj", L"Assets/Cofre/Cofre-color.png", L"Assets/Cofre/Cofre-spec.png", 0, 0);
    	
		bodega = new ModeloRR(d3dDevice, d3dContext, "Assets/Bodega/Bodega.obj", L"Assets/Bodega/Bodega.png", L"Assets/NoSpecMap.jpg", -70, -70);
		boteBasura = new ModeloRR(d3dDevice, d3dContext, "Assets/BoteBasura/BoteBasura.obj", L"Assets/BoteBasura/BoteBasura.png", L"Assets/NoSpecMap.jpg", -50, 76);
		carroViejo = new ModeloRR(d3dDevice, d3dContext, "Assets/CarroViejo/CarroViejo.obj", L"Assets/CarroViejo/CarroViejo.bmp", L"Assets/NoSpecMap.jpg", 6, 50);
		cristal = new ModeloRR(d3dDevice, d3dContext, "Assets/Cristal/Cristal.obj", L"Assets/Cristal/Cristal.png", L"Assets/NoSpecMap.jpg", 90, 70);
		estacionGas = new ModeloRR(d3dDevice, d3dContext, "Assets/EstacionGas/EstacionGas.obj", L"Assets/EstacionGas/EstacionGas.bmp", L"Assets/NoSpecMap.jpg", 77, -84);
		
		moneda[0] = new ModeloRR(d3dDevice, d3dContext, "Assets/Moneda/Moneda1.obj", L"Assets/Moneda/Moneda.png", L"Assets/NoSpecMap.jpg", 20, -98);
		moneda[1] = new ModeloRR(d3dDevice, d3dContext, "Assets/Moneda/Moneda1.obj", L"Assets/Moneda/Moneda.png", L"Assets/NoSpecMap.jpg", 56, 0);
		moneda[2] = new ModeloRR(d3dDevice, d3dContext, "Assets/Moneda/Moneda1.obj", L"Assets/Moneda/Moneda.png", L"Assets/NoSpecMap.jpg", -86, -58);
		moneda[3] = new ModeloRR(d3dDevice, d3dContext, "Assets/Moneda/Moneda1.obj", L"Assets/Moneda/Moneda.png", L"Assets/NoSpecMap.jpg", -15, 14);
		
		piedra = new ModeloRR(d3dDevice, d3dContext, "Assets/Piedra/Piedra.obj", L"Assets/Piedra/Piedra.bmp", L"Assets/NoSpecMap.jpg", 36,67);
		pozo = new ModeloRR(d3dDevice, d3dContext, "Assets/Pozo/Pozo.obj", L"Assets/Pozo/Pozo.jpg", L"Assets/NoSpecMap.jpg", 65, 66);
		torreAgua = new ModeloRR(d3dDevice, d3dContext, "Assets/TorreAgua/TorreAgua.obj", L"Assets/TorreAgua/TorreAgua.bmp", L"Assets/NoSpecMap.jpg", -75, 60);
		garage = new ModeloRR(d3dDevice, d3dContext, "Assets/garage/garage.obj", L"Assets/garage/garage.png", L"Assets/NoSpecMap.jpg", -30, -35);
		molino = new ModeloRR(d3dDevice, d3dContext, "Assets/molino/molino.obj", L"Assets/molino/molino.png", L"Assets/NoSpecMap.jpg", 36, -63);

		D3DXVECTOR3 offsetVehiculo(0.0f, -0.8f, 0.5f); // Ejemplo: un poco abajo y ligeramente delante de la cámara
		moto = new Vehiculo(d3dDevice, d3dContext,
			"Assets/moto/moto.obj", // Ruta a tu modelo de vehículo
			L"Assets/moto/moto.png", // Textura
			L"Assets/NoSpecMap.jpg", // Specular
			camara, // Pasa la cámara del jugador
			offsetVehiculo,
			terreno, // Pasa el terreno si quieres ajuste de altura
			0.3f // Altura de la base del vehículo sobre el terreno
		);
		
		Golem = new Enemigo(d3dDevice, d3dContext,
			"Assets/Golem/Golem.obj", L"Assets/Golem/GolemTextura.bmp", L"Assets/NoSpecMap.jpg",
			30.0f, 30.0f, // Posición inicial del enemigo
			camara, terreno,
			0.5f,  // Altura de la base de la moto sobre el terreno
			0.05f,  // Velocidad de movimiento
			D3DX_PI * 0.8f, // Velocidad de rotación
			40.0f  // Rango de persecución
		);

		water = new WaterRR(2000, 2000, d3dDevice, d3dContext, L"Assets/water.png", L"Assets/water.png");

		hCameraPosLabel = CreateWindowEx(0, L"STATIC", L"Posici�n de la c�mara: X=0, Y=0, Z=0",
			WS_CHILD | WS_VISIBLE | SS_LEFT,
			10, 10, 500, 20,
			hWnd, NULL, hInstance, NULL);


		vida = 3;
		colisionGolem1 = false; // Aunque estas podrían no ser necesarias con el nuevo enfoque
		colisionGolem2 = false;
		colisionGolem3 = false;
		restarVida1 = false;
		restarVida2 = false;
		restarVida3 = false;

		jugadorInvulnerable = false;
		tiempoFinInvulnerabilidad = 0.0f;
		duracionInvulnerabilidad = 1.5f;


		
	}

	~DXRR()
	{
		LiberaD3D();
		m_XACT3.Terminate();
	}

	
	bool IniciaD3D(HWND hWnd)
	{
		this->hInstance = hInstance;
		this->hWnd = hWnd;

		//obtiene el ancho y alto de la ventana donde se dibuja
		RECT dimensions;
		GetClientRect(hWnd, &dimensions);
		unsigned int width = dimensions.right - dimensions.left;
		unsigned int heigth = dimensions.bottom - dimensions.top;

		//Las formas en como la pc puede ejecutar el DX11, la mas rapida es D3D_DRIVER_TYPE_HARDWARE pero solo se puede usar cuando lo soporte el hardware
		//otra opcion es D3D_DRIVER_TYPE_WARP que emula el DX11 en los equipos que no lo soportan
		//la opcion menos recomendada es D3D_DRIVER_TYPE_SOFTWARE, es la mas lenta y solo es util cuando se libera una version de DX que no sea soportada por hardware
		D3D_DRIVER_TYPE driverTypes[]=
		{
			D3D_DRIVER_TYPE_HARDWARE, D3D_DRIVER_TYPE_WARP, D3D_DRIVER_TYPE_SOFTWARE
		};
		unsigned int totalDriverTypes = ARRAYSIZE(driverTypes);

		//La version de DX que utilizara, en este caso el DX11
		D3D_FEATURE_LEVEL featureLevels[]=
		{
			D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_1, D3D_FEATURE_LEVEL_10_0
		};
		unsigned int totalFeaturesLevels = ARRAYSIZE(featureLevels);

		DXGI_SWAP_CHAIN_DESC swapChainDesc;
		ZeroMemory(&swapChainDesc, sizeof(swapChainDesc));
		swapChainDesc.BufferCount = 1;
		swapChainDesc.BufferDesc.Width = width;
		swapChainDesc.BufferDesc.Height = heigth;
		swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		swapChainDesc.BufferDesc.RefreshRate.Numerator = 60;
		swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
		swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapChainDesc.OutputWindow = hWnd;
		swapChainDesc.Windowed = true;
		swapChainDesc.SampleDesc.Count = 1;
		swapChainDesc.SampleDesc.Quality = 0;

		HRESULT result;
		unsigned int driver = 0, creationFlags = 0;
		for(driver = 0; driver<totalDriverTypes; driver++)
		{
			result = D3D11CreateDeviceAndSwapChain(0, driverTypes[driver], 0,
				creationFlags, featureLevels, totalFeaturesLevels, 
				D3D11_SDK_VERSION, &swapChainDesc, &swapChain,
				&d3dDevice, &featureLevel, &d3dContext);

			if(SUCCEEDED(result))
			{
				driverType = driverTypes[driver];
				break;
			}
		}

		if(FAILED(result))
		{

			//Error al crear el Direct3D device
			return false;
		}
		
		ID3D11Texture2D* backBufferTexture;
		result = swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&backBufferTexture);
		if(FAILED(result))
		{
			//"Error al crear el swapChainBuffer
			return false;
		}

		result = d3dDevice->CreateRenderTargetView(backBufferTexture, 0, &backBufferTarget);
		if(backBufferTexture)
			backBufferTexture->Release();

		if(FAILED(result))
		{
			//Error al crear el renderTargetView
			return false;
		}


		D3D11_VIEWPORT viewport;
		viewport.Width = (FLOAT)width;
		viewport.Height = (FLOAT)heigth;
		viewport.MinDepth = 0.0f;
		viewport.MaxDepth = 1.0f;
		viewport.TopLeftX = 0.0f;
		viewport.TopLeftY = 0.0f;

		d3dContext->RSSetViewports(1, &viewport);

		D3D11_TEXTURE2D_DESC depthTexDesc;
		ZeroMemory(&depthTexDesc, sizeof(depthTexDesc));
		depthTexDesc.Width = width;
		depthTexDesc.Height = heigth;
		depthTexDesc.MipLevels = 1;
		depthTexDesc.ArraySize = 1;
		depthTexDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		depthTexDesc.SampleDesc.Count = 1;
		depthTexDesc.SampleDesc.Quality = 0;
		depthTexDesc.Usage = D3D11_USAGE_DEFAULT;
		depthTexDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
		depthTexDesc.CPUAccessFlags = 0;
		depthTexDesc.MiscFlags = 0;
		
		result = d3dDevice->CreateTexture2D(&depthTexDesc, NULL, &depthTexture);
		if(FAILED(result))
		{
			MessageBox(0, L"Error", L"Error al crear la DepthTexture", MB_OK);
			return false;
		}

		D3D11_DEPTH_STENCIL_VIEW_DESC descDSV;
		ZeroMemory(&descDSV, sizeof(descDSV));
		descDSV.Format = depthTexDesc.Format;
		descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
		descDSV.Texture2D.MipSlice = 0;

		result = d3dDevice->CreateDepthStencilView(depthTexture, &descDSV, &depthStencilView);
		if(FAILED(result))
		{
			MessageBox(0, L"Error", L"Error al crear el depth stencil target view", MB_OK);
			return false;
		}

		d3dContext->OMSetRenderTargets(1, &backBufferTarget, depthStencilView);

		return true;			
		
	}

	void LiberaD3D(void)
	{
		if(depthTexture)
			depthTexture->Release();
		if(depthStencilView)
			depthStencilView->Release();
		if(backBufferTarget)
			backBufferTarget->Release();
		if(swapChain)
			swapChain->Release();
		if(d3dContext)
			d3dContext->Release();
		if(d3dDevice)
			d3dDevice->Release();

		depthTexture = 0;
		depthStencilView = 0;
		d3dDevice = 0;
		d3dContext = 0;
		swapChain = 0;
		backBufferTarget = 0;
	}
	
	void Render(void)
	{

		if (Lightrota > 2 * 3.141592) {
			Lightrota = 0;
		}
		else {
			Lightrota += rotacionT;
		}

		if (Lightrota <= 3.341592 / 2 && Lightrota > 0.0f) {
			if (lightColor.x > 120.0f / 255.0f) {
				lightColor.x -= rotacionT / 2;
			}
			if (lightColor.y > 148.0f / 255.0f) {
				lightColor.y -= rotacionT / 2;
			}
			if (lightColor.z > 149.0f / 255.0f) {
				lightColor.z -= rotacionT / 2;
			}
		}
		if (Lightrota <= 4 && Lightrota > 3.341592 / 2) {
			if (lightColor.x > 2.0f / 255.0f) {
				lightColor.x -= rotacionT / 2;
			}
			if (lightColor.y > 8.0f / 255.0f) {
				lightColor.y -= rotacionT / 2;
			}
			if (lightColor.z > 6.0f / 255.0f) {
				lightColor.z -= rotacionT / 2;
			}
		}

		if (Lightrota <= 3 * 3.341592 / 2 && Lightrota > 4) {
			if (lightColor.x < 113.0f / 255.0f) {
				lightColor.x += rotacionT / 2;
			}
			if (lightColor.y < 94.0f / 255.0f) {
				lightColor.y += rotacionT / 2;
			}
			if (lightColor.z < 82.0f / 255.0f) {
				lightColor.z += rotacionT / 2;
			}
		}

		if (Lightrota <= 3.141592 * 2 && Lightrota > 3 * 3.341592 / 2) {
			if (lightColor.x < 253.0f / 255.0f) {
				lightColor.x += rotacionT / 2;
			}
			if (lightColor.y < 248.0f / 255.0f) {
				lightColor.y += rotacionT / 2;
			}
			if (lightColor.z < 223.0f / 255.0f) {
				lightColor.z += rotacionT / 2;
			}
		}


		if (Lightrota < 3.141592) {
			lightDirection.y -= (rotacionT / 3.141592) * 2;
		}
		else {
			lightDirection.y += (rotacionT / 3.141592) * 2;
		}
		lightDirection.z = sin(Lightrota);
		lightDirection.x = cos(Lightrota);



		float sphere[3] = { 0,0,0 };
		float prevPos[3] = { camara->posCam.x, camara->posCam.z, camara->posCam.z };
		static float angle = 0.0f;
		angle += 0.005;
		if (angle >= 360) angle = 0.0f;
		bool collide = false;
		if( d3dContext == 0 )
			return;

		// Actualizar el tiempo del juego (más rápido para ver cambios)
		gameTime += 0.05f;

		// Implementación del ciclo de ida y vuelta para transición suave
		// Calcular el tiempo normalizado (0.0 a 1.0 para un ciclo completo)
		float normalizedTime = fmod(gameTime, cycleDuration) / cycleDuration;
		float timeOfDay;

		// Si estamos en la primera mitad del ciclo, avanzamos de 0.0 a 1.0 (día a tarde)
		if (normalizedTime < 0.5f) {
			timeOfDay = normalizedTime * 2.0f;
		}
		// Si estamos en la segunda mitad, retrocedemos de 1.0 a 0.0 (tarde a día)
		else {
			timeOfDay = 2.0f - (normalizedTime * 2.0f);
		}

		float clearColor[4] = { 0, 0, 0, 1.0f };
		d3dContext->ClearRenderTargetView( backBufferTarget, clearColor );
		d3dContext->ClearDepthStencilView( depthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0 );
		camara->posCam.y = terreno->Superficie(camara->posCam.x, camara->posCam.z) + 5 ;
		camara->UpdateCam(vel, arriaba, izqder, movement);
		skydome->Update(camara->vista, camara->proyeccion);

		skydome->Update(camara->vista, camara->proyeccion, timeOfDay);

		float camPosXZ[2] = { camara->posCam.x, camara->posCam.z };

    	
		TurnOffDepth();
		skydome->Render(camara->posCam);
		TurnOnDepth();
		terreno->Draw(camara->vista, camara->proyeccion);
		TurnOnAlphaBlending();
		water->Draw(camara->vista, camara->proyeccion, 90.0f, terreno->Superficie(25, -5) + 1, 10, 0.0005f, lightDirection, lightColor);
		TurnOffAlphaBlending();

		m_XACT3.DoWork();

		//TurnOnAlphaBlending();
		//billboard->Draw(camara->vista, camara->proyeccion, camara->posCam,
			//-51, -78, 4, 5, uv1, uv2, uv3, uv4, frameBillboard);

    	// DEBUG HITBOX RENDERER
		//hitboxRenderer->RenderAll(*hitboxSystem, camara->vista, camara->proyeccion);   	

		arbolito->Draw(camara->vista, camara->proyeccion, camara->posCam,
			-30, -50, 3, 5, uvArbol1, uvArbol2, uvArbol3, uvArbol4, 0);
		arbolito2->Draw(camara->vista, camara->proyeccion, camara->posCam,
			-55, 70, 5, 5, uvArbol1, uvArbol2, uvArbol3, uvArbol4, 0);
		grass->Draw(camara->vista, camara->proyeccion, camara->posCam,
			65, 47, 2, 5, uvArbol1, uvArbol2, uvArbol3, uvArbol4, 0);
		grass2->Draw(camara->vista, camara->proyeccion, camara->posCam,
			46, -55, 4, 5, uvArbol1, uvArbol2, uvArbol3, uvArbol4, 0);
		grass3->Draw(camara->vista, camara->proyeccion, camara->posCam,
			-20, 65, 4, 5, uvArbol1, uvArbol2, uvArbol3, uvArbol4, 0);

		//TurnOffAlphaBlending();
		//model->Draw(camara->vista, camara->proyeccion, terreno->Superficie(100, 20), camara->posCam, 10.0f, 0, 'A', 1);


		//Conducción
		if (moto) { // Primero, asegurarse de que el objeto moto existe
			if (jugadorEnMoto) {
				// Si el jugador está en la moto, la moto se adhiere a la cámara.
				moto->ActualizarTransformacionConCamara();
			}
			else {

			}
			moto->Dibujar(camara->vista, camara->proyeccion);
		}

		//Enemigo persigue al jugador
		if (Golem) {
			Golem->ActualizarIA(1); // deltaTime es el tiempo transcurrido del frame
		}
    	
		if (Golem) {
			Golem->Dibujar(camara->vista, camara->proyeccion, camara->posCam);
		}
		
		
		bodega->Draw(camara->vista, camara->proyeccion, terreno->Superficie(-65, -80), camara->posCam, 10.0f, 0, 'A', 1);
		boteBasura->Draw(camara->vista, camara->proyeccion, terreno->Superficie(-50, 76), camara->posCam, 10.0f, 0, 'A', 1);
		carroViejo->Draw(camara->vista, camara->proyeccion, terreno->Superficie(6, 50), camara->posCam, 10.0f, 0, 'A', 1);
		cristal->Draw(camara->vista, camara->proyeccion, terreno->Superficie(90, 70), camara->posCam, 10.0f, 0, 'A', 1);
		estacionGas->Draw(camara->vista, camara->proyeccion, terreno->Superficie(77, -84), camara->posCam, 10.0f, 0, 'A', 1);
		
		piedra->Draw(camara->vista, camara->proyeccion, terreno->Superficie(36, 67), camara->posCam, 10.0f, 0, 'A', 1);
		pozo->Draw(camara->vista, camara->proyeccion, terreno->Superficie(65, 66), camara->posCam, 10.0f, 0, 'A', 1);
		torreAgua->Draw(camara->vista, camara->proyeccion, terreno->Superficie(-54, -18), camara->posCam, 10.0f, 0, 'A', 1);
		garage->Draw(camara->vista, camara->proyeccion, terreno->Superficie(-30, -35), camara->posCam, 10.0f, 0, 'A', 1);
		molino->Draw(camara->vista, camara->proyeccion, terreno->Superficie(36, -63), camara->posCam, 10.0f, 0, 'A', 1);
		//moto->Draw(camara->vista, camara->proyeccion, terreno->Superficie(-30, -43), camara->posCam, 10.0f, 0, 'A', 1);
		
		

    	//JUGABILIDAD
		
		//Moneda 0
		if (!isPointInsideSphere(camara->GetPoint(), moneda[0]->getSphere(1)) && moneda0Agarrada == false) {
			moneda[0]->Draw(camara->vista, camara->proyeccion, terreno->Superficie(20, -98), camara->posCam, 10.0f, 0, 'A', 1);
			//camara->posCam = prevPos; 
		}
		else {
			moneda0Agarrada = true;
			if (!score0Aumentado) {
				score0Aumentado = true;
				score += 10;
			}
		}
		//Moneda 1
		if (!isPointInsideSphere(camara->GetPoint(), moneda[1]->getSphere(1)) && moneda1Agarrada == false) {
			moneda[1]->Draw(camara->vista, camara->proyeccion, terreno->Superficie(56, 0), camara->posCam, 10.0f, 0, 'A', 1);
			//camara->posCam = prevPos; 
		}
		else {
			moneda1Agarrada = true;
			if (!score1Aumentado) {
				score1Aumentado = true;
				score += 10;
			}
		}
		//Moneda 2
		if (!isPointInsideSphere(camara->GetPoint(), moneda[2]->getSphere(1)) && moneda2Agarrada == false) {
			moneda[2]->Draw(camara->vista, camara->proyeccion, terreno->Superficie(-86, -58), camara->posCam, 10.0f, 0, 'A', 1);
			//camara->posCam = prevPos; 
		}
		else {
			moneda2Agarrada = true;
			if (!score2Aumentado) {
				score2Aumentado = true;
				score += 10;
			}
		}
		//Moneda 3
		if (!isPointInsideSphere(camara->GetPoint(), moneda[3]->getSphere(1)) && moneda3Agarrada == false) {
			moneda[3]->Draw(camara->vista, camara->proyeccion, terreno->Superficie(-15, 14), camara->posCam, 10.0f, 0, 'A', 1);
			//camara->posCam = prevPos; 
		}
		else {
			moneda3Agarrada = true;
			if (!score3Aumentado) {
				score3Aumentado = true;
				score += 10;
			}
		}

		if (vida > 0) { // Solo procesar si el jugador tiene vida
			// Asumimos que tienes un puntero a tu enemigo/golem, ej. 'enemigoGolemMIA'
			// y que tiene un método getSphere(radio). Ajusta el radio según sea necesario.
			bool hayColisionConGolem = false;
			if (Golem) { // Verifica que el puntero al golem es válido
				// El radio de la esfera del golem (ej. 2.0f) debe ajustarse al tamaño de tu modelo
				hayColisionConGolem = isPointInsideSphere(camara->GetPoint(), Golem->getSphere(2.0f));
			}


			// Manejar fin de invulnerabilidad
			if (jugadorInvulnerable && gameTime >= tiempoFinInvulnerabilidad) {
				jugadorInvulnerable = false;
				OutputDebugString(L"Jugador ya no es invulnerable.\n");
			}

			// Si hay colisión Y el jugador NO es invulnerable
			if (hayColisionConGolem && !jugadorInvulnerable) {
				bool vidaPerdidaEsteFrame = false; // Para asegurar que el sonido solo suene una vez por "toque"
				if (!restarVida1) {
					vida--;
					restarVida1 = true;
					vidaPerdidaEsteFrame = true;
					// ... (tu OutputDebugString) ...
				}
				else if (!restarVida2) {
					vida--;
					restarVida2 = true;
					vidaPerdidaEsteFrame = true;
					// ... (tu OutputDebugString) ...
				}
				else if (!restarVida3) {
					vida--;
					restarVida3 = true;
					vidaPerdidaEsteFrame = true;
					// ... (tu OutputDebugString) ...
				}

				if (vidaPerdidaEsteFrame) {
					jugadorInvulnerable = true;
					tiempoFinInvulnerabilidad = gameTime + duracionInvulnerabilidad;

					// Reproducir el sonido de daño
					if (m_XACT3.m_pSoundBank && cuePlayerDamage != XACTINDEX_INVALID) {
						m_XACT3.m_pSoundBank->Play(cuePlayerDamage, 0, 0, NULL);
					}
				}
			}
		} // Fin de if (vida > 0)

		// Comprobar Game Over
		if (vida <= 0) {
			// Evitar que se ejecute la lógica de Game Over múltiples veces
			static bool gameOverProcesado = false;
			if (!gameOverProcesado) {
				OutputDebugString(L"GAME OVER\n");
				MessageBox(hWnd, L"Has sido derrotado por el Golem.", L"Game Over", MB_OK | MB_ICONINFORMATION);
				// Aquí podrías:
				// - Mostrar un menú de Game Over.
				// - Reiniciar el juego/nivel (restableciendo 'vida', 'restarVidaX', posición del jugador/golem, etc.).
				// - Salir de la aplicación:
				PostQuitMessage(0);
				gameOverProcesado = true; // Marcar como procesado
			}
		}
		
		
		
		
		if (score == 40) {
			MessageBox(hWnd, L"Felicidades, has recolectado todas las monedas!", L"Juego Terminado", MB_OK);
			PostQuitMessage(0);
		}

    	
		std::wstringstream ss;
		ss << L"Posici�n de la c�mara: X=" << camara->posCam.x << L", Y=" << camara->posCam.y << L", Z=" << camara->posCam.z << " Vida = " << vida;
		SetWindowText(hCameraPosLabel, ss.str().c_str());

		swapChain->Present( 1, 0 );
	}

	bool isPointInsideSphere(float* point, float* sphere) {
		bool collition = false;

		float distance = sqrt((point[0] - sphere[0]) * (point[0] - sphere[0]) +
			(point[1] - sphere[1]) * (point[1] - sphere[1]));

		if (distance < sphere[2])
			collition = true;
		return collition;
	}

	//Activa el alpha blend para dibujar con transparencias
	void TurnOnAlphaBlending()
	{
		float blendFactor[4];
		blendFactor[0] = 0.0f;
		blendFactor[1] = 0.0f;
		blendFactor[2] = 0.0f;
		blendFactor[3] = 0.0f;
		HRESULT result;

		D3D11_BLEND_DESC descABSD;
		ZeroMemory(&descABSD, sizeof(D3D11_BLEND_DESC));
		descABSD.RenderTarget[0].BlendEnable = TRUE;
		descABSD.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
		descABSD.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
		descABSD.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
		descABSD.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
		descABSD.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
		descABSD.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
		descABSD.RenderTarget[0].RenderTargetWriteMask = 0x0f;

		result = d3dDevice->CreateBlendState(&descABSD, &alphaBlendState);
		if(FAILED(result))
		{
			MessageBox(0, L"Error", L"Error al crear el blend state", MB_OK);
			return;
		}

		d3dContext->OMSetBlendState(alphaBlendState, blendFactor, 0xffffffff);
	}

	//Regresa al blend normal(solido)
	void TurnOffAlphaBlending()
	{
		D3D11_BLEND_DESC descCBSD;
		ZeroMemory(&descCBSD, sizeof(D3D11_BLEND_DESC));
		descCBSD.RenderTarget[0].BlendEnable = FALSE;
		descCBSD.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
		descCBSD.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
		descCBSD.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
		descCBSD.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
		descCBSD.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
		descCBSD.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
		descCBSD.RenderTarget[0].RenderTargetWriteMask = 0x0f;
		HRESULT result;

		result = d3dDevice->CreateBlendState(&descCBSD, &commonBlendState);
		if(FAILED(result))
		{
			MessageBox(0, L"Error", L"Error al crear el blend state", MB_OK);
			return;
		}

		d3dContext->OMSetBlendState(commonBlendState, NULL, 0xffffffff);
	}

	void TurnOnDepth()
	{
		D3D11_DEPTH_STENCIL_DESC descDSD;
		ZeroMemory(&descDSD, sizeof(descDSD));
		descDSD.DepthEnable = true;
		descDSD.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
		descDSD.DepthFunc = D3D11_COMPARISON_LESS;
		descDSD.StencilEnable=true;
		descDSD.StencilReadMask = 0xFF;
		descDSD.StencilWriteMask = 0xFF;
		descDSD.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
		descDSD.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
		descDSD.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
		descDSD.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
		descDSD.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
		descDSD.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
		descDSD.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
		descDSD.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

		d3dDevice->CreateDepthStencilState(&descDSD, &depthStencilState);
		
		d3dContext->OMSetDepthStencilState(depthStencilState, 1);
	}

	void TurnOffDepth()
	{
		D3D11_DEPTH_STENCIL_DESC descDDSD;
		ZeroMemory(&descDDSD, sizeof(descDDSD));
		descDDSD.DepthEnable = false;
		descDDSD.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
		descDDSD.DepthFunc = D3D11_COMPARISON_LESS;
		descDDSD.StencilEnable=true;
		descDDSD.StencilReadMask = 0xFF;
		descDDSD.StencilWriteMask = 0xFF;
		descDDSD.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
		descDDSD.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
		descDDSD.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
		descDDSD.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
		descDDSD.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
		descDDSD.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
		descDDSD.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
		descDDSD.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

		d3dDevice->CreateDepthStencilState(&descDDSD, &depthStencilDisabledState);
		d3dContext->OMSetDepthStencilState(depthStencilDisabledState, 1);
	}

	void billCargaFuego()
	{
		uv1[0].u = .125;
		uv2[0].u = .125;
		uv3[0].u = 0;
		uv4[0].u = 0;

		uv1[0].v = .25;
		uv2[0].v = 0;
		uv3[0].v = 0;
		uv4[0].v = .25;


		for (int j = 0; j < 8; j++) {
			uv1[j].u = uv1[0].u + (j * .125);
			uv2[j].u = uv2[0].u + (j * .125);
			uv3[j].u = uv3[0].u + (j * .125);
			uv4[j].u = uv4[0].u + (j * .125);

			uv1[j].v = .25;
			uv2[j].v = 0;
			uv3[j].v = 0;
			uv4[j].v = .25;
		}
		for (int j = 0; j < 8; j++) {
			uv1[j + 8].u = uv1[0].u + (j * .125);
			uv2[j + 8].u = uv2[0].u + (j * .125);
			uv3[j + 8].u = uv3[0].u + (j * .125);
			uv4[j + 8].u = uv4[0].u + (j * .125);

			uv1[j + 8].v = .5;
			uv2[j + 8].v = .25;
			uv3[j + 8].v = .25;
			uv4[j + 8].v = .5;
		}

		for (int j = 0; j < 8; j++) {
			uv1[j + 16].u = uv1[0].u + (j * .125);
			uv2[j + 16].u = uv2[0].u + (j * .125);
			uv3[j + 16].u = uv3[0].u + (j * .125);
			uv4[j + 16].u = uv4[0].u + (j * .125);

			uv1[j + 16].v = .75;
			uv2[j + 16].v = .5;
			uv3[j + 16].v = .5;
			uv4[j + 16].v = .75;
		}

		for (int j = 0; j < 8; j++) {
			uv1[j + 24].u = uv1[0].u + (j * .125);
			uv2[j + 24].u = uv2[0].u + (j * .125);
			uv3[j + 24].u = uv3[0].u + (j * .125);
			uv4[j + 24].u = uv4[0].u + (j * .125);

			uv1[j + 24].v = 1;
			uv2[j + 24].v = .75;
			uv3[j + 24].v = .75;
			uv4[j + 24].v = 1;
		}
	}

	// Configurar UV para arbolito2 (imagen completa, sin animaci�n)
	void configurarUVArbolito() {
		uvArbol1[0].u = 0.0f; uvArbol1[0].v = 1.0f;
		uvArbol2[0].u = 0.0f; uvArbol2[0].v = 0.0f;
		uvArbol3[0].u = 1.0f; uvArbol3[0].v = 0.0f;
		uvArbol4[0].u = 1.0f; uvArbol4[0].v = 1.0f;
	}

	void HandleInput(WPARAM wParam)
	{
		std::wstringstream ss;
		switch (wParam)
		{
		case VK_F1:
			// Mostrar la posici�n de la c�mara  
			ss << L"Posicion de la camara: X=" << camara->posCam.x << L", Y=" << camara->posCam.y << L", Z=" << camara->posCam.z;
			MessageBox(hWnd, ss.str().c_str(), L"Posici�n de la c�mara", MB_OK | MB_ICONINFORMATION);
			break;
		default:
			break;
		}
	}

   
};
#endif

