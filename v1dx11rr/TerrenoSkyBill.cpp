#include <windows.h>
#include <windowsx.h>

#include "DXRR.h"
#include "GamePadRR.h"

#include <dinput.h>
#include <xinput.h>

#include <iostream> 

#pragma comment (lib, "d3d11.lib")
#pragma comment (lib, "d3dx11.lib")
#pragma comment (lib, "d3dx10.lib")

#define SCREEN_X 1920
#define SCREEN_Y 1080

LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

DXRR *dxrr;
GamePadRR *gamePad;
tagPOINT initialPoint;
tagPOINT actualPoint;
LPDIRECTINPUT8 m_pDirectInput = NULL;
LPDIRECTINPUTDEVICE8 m_pKeyboardDevice = NULL;
LPDIRECTINPUTDEVICE8 m_pMouseDevice = NULL;
HWND hCameraPosLabel;

D3DXVECTOR3 prevPosTest = D3DXVECTOR3(FLT_MAX, FLT_MAX, FLT_MAX);
bool lockPosTest = FALSE;

void createMouseDevice(HWND hWnd) {
    m_pDirectInput->CreateDevice(GUID_SysMouse, &m_pMouseDevice, 0);

    m_pMouseDevice->SetDataFormat(&c_dfDIMouse);

    m_pMouseDevice->SetCooperativeLevel(hWnd, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE);

    m_pMouseDevice->Acquire();

}

void createKeyboardDevice(HWND hWnd) {
    m_pDirectInput->CreateDevice(GUID_SysKeyboard, &m_pKeyboardDevice, 0);

    m_pKeyboardDevice->SetDataFormat(&c_dfDIKeyboard);

    m_pKeyboardDevice->SetCooperativeLevel(hWnd, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE);

    m_pKeyboardDevice->Acquire();

}


static void CopyToClipboard(const std::string& text) {
    if (!OpenClipboard(nullptr)) return;

    printf("%s\n", text);

    EmptyClipboard();

    // Allocate global memory for the text (plus null terminator)
    HGLOBAL hGlob = GlobalAlloc(GMEM_MOVEABLE, text.size() + 1);
    if (!hGlob) {
        CloseClipboard();
        return;
    }

    // Lock memory and copy the string
    void* pGlob = GlobalLock(hGlob);
    memcpy(pGlob, text.c_str(), text.size() + 1);
    GlobalUnlock(hGlob);

    // Set clipboard data as CF_TEXT (ANSI)
    SetClipboardData(CF_TEXT, hGlob);

    CloseClipboard();
}


void CopyHitboxData(const D3DXVECTOR3& prevPosTest, const D3DXVECTOR3& camPos) {
    char buffer[256];
    snprintf(buffer, sizeof(buffer),
        "hitboxSystem->RegisterHitbox(Box::FromCoords(%.2ff, %.2ff, %.2ff, %.2ff, %.2ff, %.2ff));",
        prevPosTest.x, prevPosTest.y, prevPosTest.z,
        camPos.x, camPos.y, camPos.z);
    
    CopyToClipboard(buffer);
}


int WINAPI WinMain(HINSTANCE hInstance,
                   HINSTANCE hPrevInstance,
                   LPSTR lpCmdLine,
                   int nCmdShow)
{
    HWND hWnd;
    WNDCLASSEX wc;
    DEVMODE dmScreenSettings;
    int posX, posY;
    ZeroMemory(&wc, sizeof(WNDCLASSEX));

    wc.cbSize = sizeof(WNDCLASSEX);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.lpszClassName = L"DXRR_E1";
    wc.cbSize = sizeof(WNDCLASSEX);


    RegisterClassEx(&wc);

    // If full screen set the screen to maximum size of the users desktop and 32bit.
    memset(&dmScreenSettings, 0, sizeof(dmScreenSettings));
    dmScreenSettings.dmSize = sizeof(dmScreenSettings);
    dmScreenSettings.dmPelsWidth = (unsigned long)SCREEN_X;
    dmScreenSettings.dmPelsHeight = (unsigned long)SCREEN_Y;
    dmScreenSettings.dmBitsPerPel = 32;
    dmScreenSettings.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;

    // Change the display settings to full screen.
    // Si tienes problemas con correr el codigo apaga lo sig:
    //ChangeDisplaySettings(&dmScreenSettings, CDS_FULLSCREEN);

    // Set the position of the window to the top left corner.
    posX = posY = 0;

    //RECT wr = {0, 0, SCREEN_X, SCREEN_Y};
    //AdjustWindowRect(&wr, WS_OVERLAPPEDWINDOW, FALSE);
    hWnd = CreateWindowEx(WS_EX_APPWINDOW,
                          L"DXRR_E1",
                          L"PLANTILLA PROYECTO",
                          WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_POPUP,
        posX,
        posY,
        SCREEN_X,
        SCREEN_Y,
                          NULL,
                          NULL,
                          hInstance,
                          NULL);

    ShowWindow(hWnd, nCmdShow);
	dxrr = new DXRR(hWnd, 800, 600);
	dxrr->vel=0;
    gamePad = new GamePadRR(1);

    ClientToScreen(hWnd, &initialPoint);
    actualPoint.x = initialPoint.x + SCREEN_X / 2;
    actualPoint.y = initialPoint.y + SCREEN_Y / 2;

	SetTimer(hWnd, 100, 33, NULL);
    MSG msg;
    ::DirectInput8Create(
        hInstance, DIRECTINPUT_VERSION,
        IID_IDirectInput8, (void**)&m_pDirectInput, 0);
    createMouseDevice(hWnd);
    createKeyboardDevice(hWnd);
    while(TRUE)
    {
        if(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);

            if(msg.message == WM_QUIT)
                break;
        }

        dxrr->Render();
    }

    if (m_pMouseDevice) {
        m_pMouseDevice->Unacquire();
        m_pMouseDevice->Release();
    }
    m_pMouseDevice = NULL;

    if (m_pKeyboardDevice) {
        m_pKeyboardDevice->Unacquire();
        m_pKeyboardDevice->Release();
    }
    m_pKeyboardDevice = NULL;

    if (m_pDirectInput)
        m_pDirectInput->Release();
    m_pDirectInput = NULL;

    return msg.wParam;
}


LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    float xPos = 0;
    float yPos = 0;

    switch(message)
    {
        case WM_DESTROY:
            {
				KillTimer(hWnd, 100);
                PostQuitMessage(0);
                return 0;
            } break;

		case WM_TIMER:
			{

			} break;

        case WM_KEYDOWN:
        {
            dxrr->HandleInput(wParam);
        } break;

        
        case WM_MOUSEMOVE: {
            SetCursorPos(actualPoint.x, actualPoint.y);
            ShowCursor(FALSE);
            dxrr->frameBillboard++;
            if (dxrr->frameBillboard == 32)
                dxrr->frameBillboard = 0;

            dxrr->movement = 0;
            dxrr->izqder = 0;
            dxrr->arriaba = 0;
            dxrr->vel = 0;
                
            char keyboardData[256];
            m_pKeyboardDevice->GetDeviceState(sizeof(keyboardData), (void*)&keyboardData);

            float runSpeed = keyboardData[DIK_LCONTROL] & 0x80 ? 2 : keyboardData[DIK_LSHIFT] & 0x80 ? 0.5 : 1;
                
            if (keyboardData[DIK_S] & 0x80) {
                dxrr->vel = -1.f * runSpeed;
            }

                if (keyboardData[DIK_C] & 0x80 && !lockPosTest) {
                    lockPosTest = true;
                    if (prevPosTest.x == FLT_MAX && prevPosTest.y == FLT_MAX && prevPosTest.z == FLT_MAX) {
                        prevPosTest = dxrr->camara->posCam;
                    }
                    else
                    {
                        CopyHitboxData(prevPosTest, dxrr->camara->posCam);
                        prevPosTest = D3DXVECTOR3(FLT_MAX, FLT_MAX, FLT_MAX);
                    }
                }

            if (keyboardData[DIK_R] & 0x80)
            {
                lockPosTest = false;
            }


            if (keyboardData[DIK_W] & 0x80) {
                dxrr->vel = 1.f * runSpeed;
            }
            if (keyboardData[DIK_D] & 0x80) {
                dxrr->movement = -1.f * runSpeed;
            }
                
            if (keyboardData[DIK_A] & 0x80) {
                dxrr->movement = 1.f * runSpeed;
            }   

            if (keyboardData[DIK_B] & 0x80) {
                dxrr->breakpoint = true;
            }

            if (keyboardData[DIK_ESCAPE] & 0x80) {
                KillTimer(hWnd, 100);
                PostQuitMessage(0);
                return 0;
            }

            if (keyboardData[DIK_E] & 0x80) { // Si se presiona 'E'
                if (!dxrr->jugadorEnMoto && dxrr->moto) { // Si el jugador NO está en la moto y la moto existe
                    // Comprobar si la cámara está cerca de la moto
                    if (dxrr->isPointInsideSphere(dxrr->camara->GetPoint(), dxrr->moto->getSphere(3.0f))) { // Radio de interacción de 3.0f (ajusta según necesites)
                        dxrr->jugadorEnMoto = true;
                        // Opcional: podrías querer "teletransportar" ligeramente la cámara o la moto
                        // para un mejor alineamiento visual inicial, o ajustar un offset en la clase Vehiculo.
                        // Por ahora, simplemente activamos el estado.
                    }
                }
            }

            // Lógica para BAJAR de la moto con la tecla 'Q'
            if (keyboardData[DIK_Q] & 0x80) { // Si se presiona 'Q'
                if (dxrr->jugadorEnMoto && dxrr->moto) { // Si el jugador SÍ está en la moto
                    dxrr->jugadorEnMoto = false;
                    // Opcional: Al bajarse, podrías querer reposicionar la cámara ligeramente
                    // al lado de la moto para que no aparezca dentro de ella.
                    // Por ejemplo:
                    // D3DXVECTOR3 posMoto = dxrr->moto->GetPosicionActual(); // Asumiendo que tu Vehiculo tiene GetPosicionActual()
                    // D3DXVECTOR3 dirFrontalMoto = dxrr->moto->GetDireccionFrontal(); // Asumiendo GetDireccionFrontal()
                    // dxrr->camara->posCam = posMoto - (dirFrontalMoto * 1.0f) + D3DXVECTOR3(-1.5f, dxrr->camara->posCam.y - posMoto.y + 0.5f, 0.0f); // Posicionar a un lado
                    // dxrr->camara->posCam.y = dxrr->terreno->Superficie(dxrr->camara->posCam.x, dxrr->camara->posCam.z) + altura_camara_normal;
                }
            }




            DIMOUSESTATE mouseData;
            m_pMouseDevice->GetDeviceState(sizeof(mouseData), (void*)&mouseData);

            // Mouse move
            dxrr->izqder = (mouseData.lX / 1000.0f);
            dxrr->arriaba = -(mouseData.lY / 1000.0f);

            if (gamePad->IsConnected())
            {


                float grados = (float)gamePad->GetState().Gamepad.sThumbRX / 32767.0;

                if (grados > 0.19 || grados < -0.19) dxrr->izqder = grados / 15;

                grados = (float)gamePad->GetState().Gamepad.sThumbRY / 32767.0;

                if (grados > 0.19 || grados < -0.19)dxrr->arriaba = grados / 15;


                float velocidad = (float)gamePad->GetState().Gamepad.sThumbLY / 32767.0;
                if (velocidad > 0.19 || velocidad < -0.19) {
                    if (gamePad->GetState().Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER)
                        velocidad *= 14.5;
                    else if (gamePad->GetState().Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER) velocidad /= 3;
                    else velocidad *= 2.5;
                    if (velocidad > 0.19) dxrr->vel = velocidad;
                    else if (velocidad < -0.19) dxrr->vel = velocidad;
                }

            }

        }break;

    }

    return DefWindowProc (hWnd, message, wParam, lParam);
}




