#ifndef _VEHICULO_H // O _VEHICULO_ANCLADO_H
#define _VEHICULO_H

#include "ModeloRR.h"
#include "Camara.h"
#include "TerrenoRR.h" // Opcional, si quieres que la altura Y se ajuste al terreno
#include <d3dx10math.h>

class Vehiculo : public ModeloRR { // O VehiculoAnclado
private:
    D3DXVECTOR3 posicionActual; // Posici�n X, Y, Z donde se dibujar� el veh�culo
    float anguloOrientacionY;   // �ngulo de rotaci�n en Y del veh�culo

    Camara* camaraReferencia;     // Puntero a la c�mara del jugador
    TerrenoRR* terrenoReferencia; // Puntero al terreno (opcional)

    // Offset para posicionar el veh�culo relativo a la c�mara.
    // Por ejemplo, si la c�mara es la "cabeza" del conductor, el veh�culo podr�a estar un poco debajo y detr�s.
    // Este offset est� en el espacio LOCAL del veh�culo/c�mara.
    D3DXVECTOR3 offsetLocalDesdeCamara;

    // Si se usa el terreno, qu� tan alto sobre el terreno debe estar el pivote del veh�culo.
    float alturaSobreTerrenoSiAplica;

public:
    Vehiculo(
        ID3D11Device* D3DDevice, ID3D11DeviceContext* D3DContext,
        char* modelPath, WCHAR* colorTexturePath, WCHAR* specularTexturePath,
        Camara* cam, // C�mara a la que se adherir�
        D3DXVECTOR3 localOffset = D3DXVECTOR3(0.0f, -0.5f, 0.0f), // Offset por defecto: veh�culo 0.5 unidades debajo de la c�mara
        TerrenoRR* terr = nullptr, // Opcional: pasar el terreno
        float alturaEncimaTerreno = 0.2f // Si se usa terreno, altura de la base del veh�culo sobre �l
    ) : ModeloRR(D3DDevice, D3DContext, modelPath, colorTexturePath, specularTexturePath,
        cam ? cam->posCam.x : 0.0f, cam ? cam->posCam.z : 0.0f) { // Inicializa posX/Z de ModeloRR

        this->camaraReferencia = cam;
        this->terrenoReferencia = terr;
        this->offsetLocalDesdeCamara = localOffset;
        this->alturaSobreTerrenoSiAplica = alturaEncimaTerreno;

        this->anguloOrientacionY = 0.0f;
        this->posicionActual = D3DXVECTOR3(0.0f, 0.0f, 0.0f);

        // Actualizar inmediatamente la posici�n y orientaci�n basada en la c�mara
        if (this->camaraReferencia) {
            ActualizarTransformacionConCamara();
        }
    }

    // Actualiza la posici�n y orientaci�n del veh�culo para que coincida con la c�mara
    void ActualizarTransformacionConCamara() {
        if (!camaraReferencia) return;

        // 1. Orientaci�n del Veh�culo:
        // Tomar la orientaci�n Y de la c�mara (basada en su vector frontal)
        // camaraReferencia->refFront es el vector normalizado hacia donde mira la c�mara
        this->anguloOrientacionY = atan2f(camaraReferencia->refFront.x, camaraReferencia->refFront.z);

        // 2. Posici�n del Veh�culo:
        // Partimos de la posici�n de la c�mara
        D3DXVECTOR3 posicionBaseCamara = camaraReferencia->posCam;

        // Creamos una matriz de rotaci�n basada en la nueva orientaci�n del veh�culo (que es la de la c�mara)
        D3DXMATRIX rotacionVehiculoMat;
        D3DXMatrixRotationY(&rotacionVehiculoMat, this->anguloOrientacionY);

        // Transformamos el offset local por esta matriz de rotaci�n
        D3DXVECTOR4 offsetTransformado;
        D3DXVec3Transform(&offsetTransformado, &this->offsetLocalDesdeCamara, &rotacionVehiculoMat);
        D3DXVECTOR3 offsetGlobal(offsetTransformado.x, offsetTransformado.y, offsetTransformado.z);

        // La posici�n final del veh�culo es la de la c�mara m�s el offset rotado
        this->posicionActual = posicionBaseCamara + offsetGlobal;

        // 3. (Opcional) Ajustar la altura Y al terreno:
        // Si se proporciona un terreno, se ajusta la Y del veh�culo para que est� sobre �l,
        // ignorando la Y calculada a partir de la c�mara y el offset Y.
        if (this->terrenoReferencia) {
            this->posicionActual.y = this->terrenoReferencia->Superficie(this->posicionActual.x, this->posicionActual.z)
                + this->alturaSobreTerrenoSiAplica;
        }
        // Si no hay terrenoReferencia, this->posicionActual.y ya contiene la Y de la c�mara + la Y del offsetLocalDesdeCamara rotado.

        // 4. Actualizar los miembros posX y posZ de la clase base ModeloRR
        // (Aseg�rate que posX y posZ en ModeloRR sean 'protected' para poder hacer esto)
        this->posX = this->posicionActual.x;
        this->posZ = this->posicionActual.z;
    }

    // Dibuja el veh�culo usando el m�todo Draw de la clase base
    void Dibujar(D3DXMATRIX vista, D3DXMATRIX proyeccion) {
        if (!camaraReferencia) return; // No dibujar si no hay c�mara de referencia

        // El m�todo Draw de ModeloRR usa this->posX y this->posZ internamente.
        // ypos es la altura Y final del modelo en el mundo.
        // rot es el �ngulo de rotaci�n.
        // angle ('Y') especifica el eje de rotaci�n.
        ModeloRR::Draw(vista, proyeccion,
            this->posicionActual.y, // La Y calculada en ActualizarTransformacionConCamara
            this->camaraReferencia->posCam, // posCam para el shader (iluminaci�n, etc.)
            10.0f, // specForce (ejemplo)
            this->anguloOrientacionY, // El �ngulo calculado
            'Y',
            1.0f); // scale (ejemplo)
    }
};

#endif // _VEHICULO_H