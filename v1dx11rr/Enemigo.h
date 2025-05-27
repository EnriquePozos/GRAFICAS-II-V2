#ifndef _ENEMIGO_H
#define _ENEMIGO_H

#include "ModeloRR.h" //
#include "Camara.h"   //
#include "TerrenoRR.h" //
#include <d3dx10math.h> // Para D3DXVECTOR3, D3DX_PI, etc.

class Enemigo : public ModeloRR {
private:
    // Estado interno del enemigo
    D3DXVECTOR3 posicionActual;   // Posición X, Y, Z precisa del enemigo
    float       anguloActualY;    // Orientación actual en el eje Y (radianes)

    // Propiedades de comportamiento
    float velocidadMovimiento;
    float velocidadRotacion;
    float rangoDePersecucion;
    float alturaModeloSobreTerreno; // Para ajustar la base del modelo al terreno

    // Referencias a objetos necesarios para la IA
    Camara* objetivoJugador;
    TerrenoRR* terrenoReferencia;

public:
    // Constructor
    Enemigo(
        ID3D11Device* D3DDevice, ID3D11DeviceContext* D3DContext,
        char* modelPath, WCHAR* colorTexturePath, WCHAR* specularTexturePath,
        float startX, float startZ, // Posición inicial del enemigo
        Camara* camaraJugador, TerrenoRR* terreno, // Objetos del entorno
        float alturaDelModelo = 1.0f, // Ajuste para que la base del modelo toque el terreno
        float velocidad = 3.0f,       // Unidades por segundo
        float velRot = D3DX_PI,     // Radianes por segundo (180 grados/seg)
        float rango = 30.0f         // Unidades de distancia para empezar a perseguir
    ) : ModeloRR(D3DDevice, D3DContext, modelPath, colorTexturePath, specularTexturePath, startX, startZ) { //

        this->objetivoJugador = camaraJugador;
        this->terrenoReferencia = terreno;

        this->velocidadMovimiento = velocidad;
        this->velocidadRotacion = velRot;
        this->rangoDePersecucion = rango;
        this->alturaModeloSobreTerreno = alturaDelModelo;

        this->posicionActual.x = startX;
        this->posicionActual.z = startZ;
        if (this->terrenoReferencia) {
            this->posicionActual.y = this->terrenoReferencia->Superficie(startX, startZ) + this->alturaModeloSobreTerreno; //
        }
        else {
            this->posicionActual.y = this->alturaModeloSobreTerreno; // Altura por defecto si no hay terreno
        }
        this->anguloActualY = 0.0f; // Orientación inicial

        // Los miembros posX y posZ de ModeloRR son inicializados por el constructor base.
        // Los mantenemos sincronizados si es necesario, o nos aseguramos que Draw use los de Enemigo.
    }

    // Método para actualizar la lógica de IA del enemigo
    void ActualizarIA(float deltaTime) {
        if (!objetivoJugador || !terrenoReferencia) {
            return; // No se puede actualizar sin objetivo o terreno
        }

        D3DXVECTOR3 posJugador = objetivoJugador->posCam; //
        D3DXVECTOR3 vectorHaciaJugador = posJugador - this->posicionActual;
        float distanciaAlJugador = D3DXVec3Length(&vectorHaciaJugador);

        // Solo perseguir si está dentro del rango y no demasiado cerca (para evitar que se "suba" al jugador)
        if (distanciaAlJugador < this->rangoDePersecucion && distanciaAlJugador > 1.5f) { // Umbral mínimo de 1.5 unidades
            D3DXVec3Normalize(&vectorHaciaJugador, &vectorHaciaJugador);

            // --- Movimiento ---
            this->posicionActual += vectorHaciaJugador * this->velocidadMovimiento * deltaTime;

            // Ajustar altura al terreno
            this->posicionActual.y = this->terrenoReferencia->Superficie(this->posicionActual.x, this->posicionActual.z) + this->alturaModeloSobreTerreno; //

            // Actualizar los miembros posX y posZ de la clase base ModeloRR
            // para que ModeloRR::Draw los utilice correctamente para la traslación.
            this->posX = this->posicionActual.x;
            this->posZ = this->posicionActual.z;

            // --- Rotación ---
            float anguloDeseadoY = atan2(vectorHaciaJugador.x, vectorHaciaJugador.z); // Ángulo para encarar al jugador
            float diferenciaAngulo = anguloDeseadoY - this->anguloActualY;

            // Normalizar la diferencia de ángulo para tomar el camino más corto
            while (diferenciaAngulo > D3DX_PI) diferenciaAngulo -= 2.0f * D3DX_PI;
            while (diferenciaAngulo < -D3DX_PI) diferenciaAngulo += 2.0f * D3DX_PI;

            float rotacionEsteFrame = this->velocidadRotacion * deltaTime;

            if (abs(diferenciaAngulo) < rotacionEsteFrame) {
                this->anguloActualY = anguloDeseadoY; // Ajuste directo si la diferencia es pequeña
            }
            else {
                this->anguloActualY += (diferenciaAngulo > 0 ? 1 : -1) * rotacionEsteFrame; // Giro gradual
            }

            // Mantener el ángulo normalizado entre -PI y PI (opcional, pero bueno para consistencia)
            while (this->anguloActualY > D3DX_PI) this->anguloActualY -= 2.0f * D3DX_PI;
            while (this->anguloActualY < -D3DX_PI) this->anguloActualY += 2.0f * D3DX_PI;
        }
        // else {
        // Aquí podrías añadir lógica para cuando el jugador está fuera de rango:
        // - Patrullar entre puntos.
        // - Quedarse quieto.
        // - Volver a una posición inicial.
        // }
    }

    // Método para dibujar el enemigo (utiliza el Draw de la clase base)
    void Dibujar(D3DXMATRIX vista, D3DXMATRIX proyeccion, D3DXVECTOR3 posCamara) {
        // ModeloRR::Draw(D3DXMATRIX vista, D3DXMATRIX proyeccion, float ypos, D3DXVECTOR3 posCam, float specForce, float rot, char angle, float scale)
        // this->posX y this->posZ ya están actualizados en ActualizarIA.
        // ypos es la altura final del modelo en el mundo.
        // rot es el ángulo de rotación.
        // angle especifica el eje de rotación.
        ModeloRR::Draw(vista, proyeccion, this->posicionActual.y, posCamara, 10.0f, this->anguloActualY, 'Y', 1.0f); //
    }

    // Getters por si necesitas acceder a la posición o ángulo desde DXRR
    D3DXVECTOR3 GetPosicionActual() const { return posicionActual; }
    float GetAnguloActualY() const { return anguloActualY; }
};

#endif // _ENEMIGO_H