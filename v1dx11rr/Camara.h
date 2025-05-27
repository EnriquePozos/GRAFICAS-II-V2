#ifndef _camara
#define _camara

//Clase camara hecha por Rafael Rosas para los UltraLMADs
//Videojuegos

#include <d3d11.h>
#include <D3DX11.h>
#include <D3DX10math.h>

#include "HitboxSystem.h"

const float MIN_X = -100.0f;
const float MAX_X = 100.0f;
const float MIN_Y = 0.0f;
const float MAX_Y = 50.0f;    
const float MIN_Z = -100.0f;
const float MAX_Z = 100.0f;

class Camara{
public:
	D3DXVECTOR3 posCam;
	D3DXVECTOR3 hdveo;
	D3DXVECTOR3 hdvoy;
	D3DXVECTOR3 refUp;
	D3DXVECTOR3 refRight;
	D3DXVECTOR3 refFront;
	D3DXMATRIX vista;
	D3DXMATRIX proyeccion;
	int ancho;
	int alto;
	float point[3];
	HitboxSystem* hitboxSystem;

	Camara(D3DXVECTOR3 eye, D3DXVECTOR3 target, D3DXVECTOR3 up, int Ancho, int Alto, HitboxSystem* system)
	{
		//posicion de la camara
		posCam = eye;
		//a donde ve
		hdveo = target;
		refUp = up;
		ancho = Ancho;
		alto = Alto;

		hitboxSystem = system;

		//crea la matriz de vista
		D3DXMatrixLookAtLH(&vista, &posCam, &hdveo, &refUp);
		//la de proyeccion
		D3DXMatrixPerspectiveFovLH( &proyeccion, D3DX_PI/2.0, ancho / alto, 0.01f, 1000.0f );
		//las transpone para acelerar la multiplicacion
		D3DXMatrixTranspose( &vista, &vista );
		D3DXMatrixTranspose( &proyeccion, &proyeccion );

		D3DXVec3Normalize(&refUp, &refUp);

		refFront = D3DXVECTOR3(target.x - eye.x, target.y - eye.y, target.z - eye.z);
		D3DXVec3Normalize(&refFront, &refFront);

		D3DXVec3Cross(&refRight, &refFront, &refUp);
		D3DXVec3Normalize(&refRight, &refRight);
		
		point[0] = posCam.x;
		point[1] = posCam.z;
		point[2] = posCam.y;
		
	}

	//funcion que devuelve el punto de la camara
	float* GetPoint()
	{
		return point;
	}

	D3DXMATRIX UpdateCam(float vel, float arriaba, float izqder, float movement)
	{
		D3DXVECTOR4 tempo;
		D3DXQUATERNION quatern; //quaternion temporal para la camara
		D3DXMATRIX giraUp, giraRight; //matrices temporales para los giros

		//creamos al quaternion segun el vector up
		D3DXQuaternionRotationAxis(&quatern, &refUp, izqder); 
		//lo normalizamos para que no acumule error
		D3DXQuaternionNormalize(&quatern, &quatern);
		//creamos la matriz de rotacion basados en el quaternion
		D3DXMatrixRotationQuaternion(&giraUp, &quatern);

		//transformamos a los vectores ded la camara
		D3DXVec3Transform(&tempo, &refFront, &giraUp);
		//como el resultado de la operacion anterior es Vec4 lo casteamos para hacerlo vec3
		refFront = (D3DXVECTOR3)tempo;
		//normalizamos para no acumular error
		D3DXVec3Normalize(&refFront, &refFront);
		//Con el vector de referencia y el nuevo front calculamos right de nuevo
		D3DXVec3Cross(&refRight, &refFront, &refUp);

		//una vez calculado right a partir de front y up ahora rotamos sobre right
		//repetimos el procedimiento anterior
		D3DXQuaternionRotationAxis(&quatern, &refRight, arriaba);
		D3DXQuaternionNormalize(&quatern, &quatern);
		D3DXMatrixRotationQuaternion(&giraRight, &quatern);

		D3DXVec3Transform(&tempo, &refFront, &giraRight);
		refFront = (D3DXVECTOR3)tempo;
		D3DXVec3Normalize(&refFront, &refFront);
		

		//ajustamos la matriz de vista con lo obtenido
		D3DXVECTOR3 prevPos = D3DXVECTOR3(posCam.x, posCam.y, posCam.z);
		prevPos += refFront * vel/10.0;
		prevPos += refRight * movement/10.0;

		if (!hitboxSystem->CheckCollision(prevPos))
		{
			prevPos.x = max(MIN_X, min(prevPos.x, MAX_X));
			prevPos.y = max(MIN_Y, min(prevPos.y, MAX_Y));
			prevPos.z = max(MIN_Z, min(prevPos.z, MAX_Z));

			posCam.x = prevPos.x;
			posCam.y = prevPos.y;
			posCam.z = prevPos.z;
		}
		
		hdveo = posCam + refFront;
		D3DXMatrixLookAtLH(&vista, &posCam, &hdveo, &refUp);
		D3DXMatrixTranspose( &vista, &vista );

		point[0] = posCam.x;
		point[1] = posCam.z;
		point[2] = posCam.y;

		return vista;
	}
	~Camara()
	{
	}
};
#endif