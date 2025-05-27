#ifndef _VEHICULO_H // O _VEHICULO_ANCLADO_H
#define _VEHICULO_H

#include "ModeloRR.h"
#include "Camara.h"
#include "TerrenoRR.h" // Opcional, si quieres que la altura Y se ajuste al terreno
#include <d3dx10math.h>

class Vehiculo : public ModeloRR { // O VehiculoAnclado
private:
    D3DXVECTOR3 posicionActual; // Posición X, Y, Z donde se dibujará el vehículo
    float anguloOrientacionY;   // Ángulo de rotación en Y del vehículo

    Camara* camaraReferencia;     // Puntero a la cámara del jugador
    TerrenoRR* terrenoReferencia; // Puntero al terreno (opcional)

    // Offset para posicionar el vehículo relativo a la cámara.
    // Por ejemplo, si la cámara es la "cabeza" del conductor, el vehículo podría estar un poco debajo y detrás.
    // Este offset está en el espacio LOCAL del vehículo/cámara.
    D3DXVECTOR3 offsetLocalDesdeCamara;

    // Si se usa el terreno, qué tan alto sobre el terreno debe estar el pivote del vehículo.
    float alturaSobreTerrenoSiAplica;

public:
    Vehiculo(
        ID3D11Device* D3DDevice, ID3D11DeviceContext* D3DContext,
        char* modelPath, WCHAR* colorTexturePath, WCHAR* specularTexturePath,
        Camara* cam, // Cámara a la que se adherirá
        D3DXVECTOR3 localOffset = D3DXVECTOR3(0.0f, -0.5f, 0.0f), // Offset por defecto: vehículo 0.5 unidades debajo de la cámara
        TerrenoRR* terr = nullptr, // Opcional: pasar el terreno
        float alturaEncimaTerreno = 0.2f // Si se usa terreno, altura de la base del vehículo sobre él
    ) : ModeloRR(D3DDevice, D3DContext, modelPath, colorTexturePath, specularTexturePath,
        cam ? cam->posCam.x : 0.0f, cam ? cam->posCam.z : 0.0f) { // Inicializa posX/Z de ModeloRR

        this->camaraReferencia = cam;
        this->terrenoReferencia = terr;
        this->offsetLocalDesdeCamara = localOffset;
        this->alturaSobreTerrenoSiAplica = alturaEncimaTerreno;

        this->anguloOrientacionY = 0.0f;
        this->posicionActual = D3DXVECTOR3(0.0f, 0.0f, 0.0f);

        // Actualizar inmediatamente la posición y orientación basada en la cámara
        if (this->camaraReferencia) {
            ActualizarTransformacionConCamara();
        }
    }

    // Actualiza la posición y orientación del vehículo para que coincida con la cámara
    void ActualizarTransformacionConCamara() {
        if (!camaraReferencia) return;

        // 1. Orientación del Vehículo:
        // Tomar la orientación Y de la cámara (basada en su vector frontal)
        // camaraReferencia->refFront es el vector normalizado hacia donde mira la cámara
        this->anguloOrientacionY = atan2f(camaraReferencia->refFront.x, camaraReferencia->refFront.z);

        // 2. Posición del Vehículo:
        // Partimos de la posición de la cámara
        D3DXVECTOR3 posicionBaseCamara = camaraReferencia->posCam;

        // Creamos una matriz de rotación basada en la nueva orientación del vehículo (que es la de la cámara)
        D3DXMATRIX rotacionVehiculoMat;
        D3DXMatrixRotationY(&rotacionVehiculoMat, this->anguloOrientacionY);

        // Transformamos el offset local por esta matriz de rotación
        D3DXVECTOR4 offsetTransformado;
        D3DXVec3Transform(&offsetTransformado, &this->offsetLocalDesdeCamara, &rotacionVehiculoMat);
        D3DXVECTOR3 offsetGlobal(offsetTransformado.x, offsetTransformado.y, offsetTransformado.z);

        // La posición final del vehículo es la de la cámara más el offset rotado
        this->posicionActual = posicionBaseCamara + offsetGlobal;

        // 3. (Opcional) Ajustar la altura Y al terreno:
        // Si se proporciona un terreno, se ajusta la Y del vehículo para que esté sobre él,
        // ignorando la Y calculada a partir de la cámara y el offset Y.
        if (this->terrenoReferencia) {
            this->posicionActual.y = this->terrenoReferencia->Superficie(this->posicionActual.x, this->posicionActual.z)
                + this->alturaSobreTerrenoSiAplica;
        }
        // Si no hay terrenoReferencia, this->posicionActual.y ya contiene la Y de la cámara + la Y del offsetLocalDesdeCamara rotado.

        // 4. Actualizar los miembros posX y posZ de la clase base ModeloRR
        // (Asegúrate que posX y posZ en ModeloRR sean 'protected' para poder hacer esto)
        this->posX = this->posicionActual.x;
        this->posZ = this->posicionActual.z;
    }

    // Dibuja el vehículo usando el método Draw de la clase base
    void Dibujar(D3DXMATRIX vista, D3DXMATRIX proyeccion) {
        if (!camaraReferencia) return; // No dibujar si no hay cámara de referencia

        // El método Draw de ModeloRR usa this->posX y this->posZ internamente.
        // ypos es la altura Y final del modelo en el mundo.
        // rot es el ángulo de rotación.
        // angle ('Y') especifica el eje de rotación.
        ModeloRR::Draw(vista, proyeccion,
            this->posicionActual.y, // La Y calculada en ActualizarTransformacionConCamara
            this->camaraReferencia->posCam, // posCam para el shader (iluminación, etc.)
            10.0f, // specForce (ejemplo)
            this->anguloOrientacionY, // El ángulo calculado
            'Y',
            1.0f); // scale (ejemplo)
    }
};

#endif // _VEHICULO_H