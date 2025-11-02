#include <algorithm> // para el sort

#include "olcConsoleGameEngine.h"
#include "GeometricTypes.h"

float escala = 0.5f;
float separacion = 1.0f;

class Engine3D : public olcConsoleGameEngine {
public:
	Engine3D() {
		m_sAppName = L"3D Demo";
	}

private:
	mesh meshCube;
	mat4x4 matrizDeProyeccion;

	vec3d vCamara;
	vec3d lookDir;

	float camaraRot;

	float fTheta;

	vec3d matrizMultVec(mat4x4 &m, vec3d &i) {
		vec3d v;
		v.x = i.x * m.m[0][0] + i.y * m.m[1][0] + i.z * m.m[2][0] + i.w * m.m[3][0];
		v.y = i.x * m.m[0][1] + i.y * m.m[1][1] + i.z * m.m[2][1] + i.w * m.m[3][1];
		v.z = i.x * m.m[0][2] + i.y * m.m[1][2] + i.z * m.m[2][2] + i.w * m.m[3][2];
		v.w = i.x * m.m[0][3] + i.y * m.m[1][3] + i.z * m.m[2][3] + i.w * m.m[3][3];
		return v;
	}

	mat4x4 matrizMultMatriz(mat4x4& m1, mat4x4& m2) {
		mat4x4 matrix;
		for (int c = 0; c < 4; c++)
			for (int r = 0; r < 4; r++)
				matrix.m[r][c] = m1.m[r][0] * m2.m[0][c] + m1.m[r][1] * m2.m[1][c] + m1.m[r][2] * m2.m[2][c] + m1.m[r][3] * m2.m[3][c];
		return matrix;
	}

	mat4x4 matrizIdentidad() {
		mat4x4 matrix;
		matrix.m[0][0] = 1.0f;
		matrix.m[1][1] = 1.0f;
		matrix.m[2][2] = 1.0f;
		matrix.m[3][3] = 1.0f;
		return matrix;
	}

	mat4x4 matrizRotX(float fAngleRad) {
		mat4x4 matrix;
		matrix.m[0][0] = 1.0f;
		matrix.m[1][1] = cosf(fAngleRad);
		matrix.m[1][2] = sinf(fAngleRad);
		matrix.m[2][1] = -sinf(fAngleRad);
		matrix.m[2][2] = cosf(fAngleRad);
		matrix.m[3][3] = 1.0f;
		return matrix;
	}

	mat4x4 matrizRotY(float fAngleRad) {
		mat4x4 matrix;
		matrix.m[0][0] = cosf(fAngleRad);
		matrix.m[0][2] = sinf(fAngleRad);
		matrix.m[2][0] = -sinf(fAngleRad);
		matrix.m[1][1] = 1.0f;
		matrix.m[2][2] = cosf(fAngleRad);
		matrix.m[3][3] = 1.0f;
		return matrix;
	}

	mat4x4 matrizRotZ(float fAngleRad) {
		mat4x4 matrix;
		matrix.m[0][0] = cosf(fAngleRad);
		matrix.m[0][1] = sinf(fAngleRad);
		matrix.m[1][0] = -sinf(fAngleRad);
		matrix.m[1][1] = cosf(fAngleRad);
		matrix.m[2][2] = 1.0f;
		matrix.m[3][3] = 1.0f;
		return matrix;
	}

	mat4x4 matrizTraducir(float x, float y, float z) {
		mat4x4 matrix;
		matrix.m[0][0] = 1.0f;
		matrix.m[1][1] = 1.0f;
		matrix.m[2][2] = 1.0f;
		matrix.m[3][3] = 1.0f;
		matrix.m[3][0] = x;
		matrix.m[3][1] = y;
		matrix.m[3][2] = z;
		return matrix;
	}

	mat4x4 matrizProyeccion(float fFovDegrees, float fAspectRatio, float fNear, float fFar) {
		float fFovRad = 1.0f / tanf(fFovDegrees * 0.5f / 180.0f * 3.14159f);
		mat4x4 matrix;
		matrix.m[0][0] = fAspectRatio * fFovRad;
		matrix.m[1][1] = fFovRad;
		matrix.m[2][2] = fFar / (fFar - fNear);
		matrix.m[3][2] = (-fFar * fNear) / (fFar - fNear);
		matrix.m[2][3] = 1.0f;
		matrix.m[3][3] = 0.0f;
		return matrix;
	}

	mat4x4 matrixPointAt(vec3d& pos, vec3d& target, vec3d& up) {
		// Calcular frente
		vec3d forward = vecRestar(target, pos);
		forward = vecNormalizar(forward);

		// Calcular arriba
		vec3d a = vecMult(forward, vecDotProduct(up, forward));
		vec3d newUp = vecRestar(up, a);
		up = vecNormalizar(up);

		// Calcular derecha
		vec3d right = vecCrossProduct(up, forward);

		mat4x4 matrix;
		matrix.m[0][0] = right.x;	matrix.m[0][1] = right.y;	matrix.m[0][2] = right.z;	matrix.m[0][3] = 0.0f;
		matrix.m[1][0] = newUp.x;		matrix.m[1][1] = newUp.y;		matrix.m[1][2] = newUp.z;		matrix.m[1][3] = 0.0f;
		matrix.m[2][0] = forward.x;	matrix.m[2][1] = forward.y;	matrix.m[2][2] = forward.z;	matrix.m[2][3] = 0.0f;
		matrix.m[3][0] = pos.x;			matrix.m[3][1] = pos.y;			matrix.m[3][2] = pos.z;			matrix.m[3][3] = 1.0f;
		return matrix;
	}

	mat4x4 matrixInverse(mat4x4& m)
	{
		mat4x4 matrix;
		matrix.m[0][0] = m.m[0][0]; matrix.m[0][1] = m.m[1][0]; matrix.m[0][2] = m.m[2][0]; matrix.m[0][3] = 0.0f;
		matrix.m[1][0] = m.m[0][1]; matrix.m[1][1] = m.m[1][1]; matrix.m[1][2] = m.m[2][1]; matrix.m[1][3] = 0.0f;
		matrix.m[2][0] = m.m[0][2]; matrix.m[2][1] = m.m[1][2]; matrix.m[2][2] = m.m[2][2]; matrix.m[2][3] = 0.0f;
		matrix.m[3][0] = -(m.m[3][0] * matrix.m[0][0] + m.m[3][1] * matrix.m[1][0] + m.m[3][2] * matrix.m[2][0]);
		matrix.m[3][1] = -(m.m[3][0] * matrix.m[0][1] + m.m[3][1] * matrix.m[1][1] + m.m[3][2] * matrix.m[2][1]);
		matrix.m[3][2] = -(m.m[3][0] * matrix.m[0][2] + m.m[3][1] * matrix.m[1][2] + m.m[3][2] * matrix.m[2][2]);
		matrix.m[3][3] = 1.0f;
		return matrix;
	}

	vec3d vecSumar(vec3d &v1, vec3d &v2) {
		return { v1.x + v2.x, v1.y + v2.y, v1.z + v2.z };
	}

	vec3d vecRestar(vec3d &v1, vec3d& v2) {
		return { v1.x - v2.x, v1.y - v2.y, v1.z - v2.z };
	}

	vec3d vecMult(vec3d &v1, float k) {
		return { v1.x * k, v1.y * k, v1.z * k };
	}

	vec3d vecDiv(vec3d &v1, float k) {
		return { v1.x / k, v1.y / k, v1.z / k };
	}

	float vecDotProduct(vec3d &v1, vec3d &v2) {
		return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
	}

	float vecLength(vec3d &v) {
		return sqrtf(vecDotProduct(v, v));
	}

	vec3d vecNormalizar(vec3d &v) {
		float l = vecLength(v);
		return { v.x / l, v.y / l, v.z / l };
	}

	vec3d vecCrossProduct(vec3d &v1, vec3d &v2) {
		vec3d v;
		v.x = v1.y * v2.z - v1.z * v2.y;
		v.y = v1.z * v2.x - v1.x * v2.z;
		v.z = v1.x * v2.y - v1.y * v2.x;
		return v;
	}

	// Algoritmo para saber si una linea esta interceptando el plano 2d (la pantalla)
	vec3d vecIntersectPlane(vec3d& planePoint, vec3d& planeNormal, vec3d& lineStart, vec3d& lineEnd) {
		planeNormal = vecNormalizar(planeNormal);
		float plane_d = -vecDotProduct(planeNormal, planePoint);
		float ad = vecDotProduct(lineStart, planeNormal);
		float bd = vecDotProduct(lineEnd, planeNormal);
		float t = (-plane_d - ad) / (bd - ad);
		vec3d lineStartToEnd = vecRestar(lineEnd, lineStart);
		vec3d lineToIntersect = vecMult(lineStartToEnd, t);
		return vecSumar(lineStart, lineToIntersect);
	}

	vec3d Vector_IntersectPlane(vec3d& plane_p, vec3d& plane_n, vec3d& lineStart, vec3d& lineEnd) {

	}

	// Funcion para escala de grises en consola (copy pasted)
	CHAR_INFO GetColour(float lum) {
		short bg_col, fg_col;
		wchar_t sym;
		int pixel_bw = (int)(13.0f * lum);
		switch (pixel_bw)
		{
		case 0: bg_col = BG_BLACK; fg_col = FG_BLACK; sym = PIXEL_SOLID; break;

		case 1: bg_col = BG_BLACK; fg_col = FG_DARK_GREY; sym = PIXEL_QUARTER; break;
		case 2: bg_col = BG_BLACK; fg_col = FG_DARK_GREY; sym = PIXEL_HALF; break;
		case 3: bg_col = BG_BLACK; fg_col = FG_DARK_GREY; sym = PIXEL_THREEQUARTERS; break;
		case 4: bg_col = BG_BLACK; fg_col = FG_DARK_GREY; sym = PIXEL_SOLID; break;

		case 5: bg_col = BG_DARK_GREY; fg_col = FG_GREY; sym = PIXEL_QUARTER; break;
		case 6: bg_col = BG_DARK_GREY; fg_col = FG_GREY; sym = PIXEL_HALF; break;
		case 7: bg_col = BG_DARK_GREY; fg_col = FG_GREY; sym = PIXEL_THREEQUARTERS; break;
		case 8: bg_col = BG_DARK_GREY; fg_col = FG_GREY; sym = PIXEL_SOLID; break;

		case 9:  bg_col = BG_GREY; fg_col = FG_WHITE; sym = PIXEL_QUARTER; break;
		case 10: bg_col = BG_GREY; fg_col = FG_WHITE; sym = PIXEL_HALF; break;
		case 11: bg_col = BG_GREY; fg_col = FG_WHITE; sym = PIXEL_THREEQUARTERS; break;
		case 12: bg_col = BG_GREY; fg_col = FG_WHITE; sym = PIXEL_SOLID; break;
		default:
			bg_col = BG_BLACK; fg_col = FG_BLACK; sym = PIXEL_SOLID;
		}

		CHAR_INFO c;
		c.Attributes = bg_col | fg_col;
		c.Char.UnicodeChar = sym;
		return c;
	}

public:
	bool OnUserCreate() override {
		meshCube.loadObjectFromObjFile("teapot.obj");

		// Matriz de proyeccion
		float fNear = 0.1f; // Distancia entre eje x
		float fFar = 1000.0f; // Hasta cuanta distancia llega la vision
		float fFov = 90.f; // FOV 90º
		float fAspectRatio = (float)ScreenHeight() / (float)ScreenWidth();

		matrizDeProyeccion = matrizProyeccion(fFov, fAspectRatio, fNear, fFar);

		return true;
	};

	bool OnUserUpdate(float fElapsedTime) override {

		if (GetKey(VK_UP).bHeld) vCamara.y -= 8.0f * fElapsedTime;
		if (GetKey(VK_DOWN).bHeld) vCamara.y += 8.0f * fElapsedTime;
		if (GetKey(VK_LEFT).bHeld) camaraRot += 2.0f * fElapsedTime;
		if (GetKey(VK_RIGHT).bHeld) camaraRot -= 2.0f * fElapsedTime;

		vec3d forward = vecMult(lookDir, 8.0f * fElapsedTime);

		if (GetKey(L'W').bHeld) vCamara = vecSumar(vCamara, forward);
		if (GetKey(L'S').bHeld) vCamara = vecRestar(vCamara, forward);
		if (GetKey(L'D').bHeld) vCamara.x += 8.0f * fElapsedTime;
		if (GetKey(L'A').bHeld) vCamara.x -= 8.0f * fElapsedTime;

		// Clear la pantalla
		Fill(0, 0, ScreenWidth(), ScreenHeight(), PIXEL_SOLID, FG_BLACK);

		// Set up rotation matrices
		//fTheta += 1.0f * fElapsedTime;

		mat4x4 matRotZ = matrizRotZ(fTheta * 0.5f);
		mat4x4 matRotX = matrizRotX(fTheta);

		mat4x4 matTrans = matrizTraducir(0.0f, 0.0f, 8.0f);

		mat4x4 matWorld = matrizIdentidad();
		matWorld = matrizMultMatriz(matRotZ, matRotX);
		matWorld = matrizMultMatriz(matWorld, matTrans);

		lookDir = { 0, 0, 1 }; // Mirando al frente (eje z)
		vec3d target = { 0, 0, 1 }; // vector eje z
		vec3d up = { 0, 1, 0 }; // vector mirando al eje y

		// Rotamos sobre el eje Y lo que nos diga la variable camaraRot
		mat4x4 cameraRot = matrizRotY(camaraRot);

		// Con esta matriz rotada sacamos la nueva direccion a la que estamos mirando
		lookDir = matrizMultVec(cameraRot, target);

		// Cambiamos el target
		target = vecSumar(vCamara, lookDir);

		mat4x4 camera = matrixPointAt(vCamara, target, up);

		// Nuestra vision es la matriz inversa a la matriz de la camara
		mat4x4 view = matrixInverse(camera);

		std::vector<triangle> vectorTriangulosOrdenados;

		// Dibujar todos los triangulos
		for (auto& tri : meshCube.tris) {

			// trianguloProyectado será el triangulo que proyectemos en la pantalla
			// trianguloTraducido será el triangulo que nos viene (tri) añadiendole valor a la z para que entre en nuestro campo de vision
			// trianguloRotadoZ triangulo ya transformado habiendo movido las z
			// trianguloRotadoZX triangulo ya transformado habiendo movido la z y x
			triangle trianguloProyectado, triTransformed, triViewed;

			triTransformed.p[0] = matrizMultVec(matWorld, tri.p[0]);
			triTransformed.p[1] = matrizMultVec(matWorld, tri.p[1]);
			triTransformed.p[2] = matrizMultVec(matWorld, tri.p[2]);

			// Calcular la normal de cada triangulo
			vec3d normal, linea1, linea2;
			
			linea1 = vecRestar(triTransformed.p[1], triTransformed.p[0]);
			linea2 = vecRestar(triTransformed.p[2], triTransformed.p[0]);

			normal = vecCrossProduct(linea1, linea2);

			normal = vecNormalizar(normal);

			// Obtener el rayo del triangulo hacia la camara
			vec3d rayoCamara = vecRestar(triTransformed.p[0], vCamara);

			// Si el vector maximo del campo de vision hace 90º o mas respecto a la normal de cualquier traingulo
			// significa que no podemos ver ese triangulo.
			// Por lo que aplicamos esta condicion
			if(vecDotProduct(normal, rayoCamara) < 0.0f) {

				//Iluminacion
				vec3d direccion_luz = { 0.0f, 0.0f, -1.0f }; // 0 0 -1 la luz viene hacia la camara
				direccion_luz = vecNormalizar(direccion_luz);

				// Cuanto de alineado esta la direccion de la luz con la normal del triangulo
				float dp = (((0.1f) > (vecDotProduct(direccion_luz, normal))) ? (0.1f) : (vecDotProduct(direccion_luz, normal)));

				CHAR_INFO c = GetColour(dp);
				triTransformed.col = c.Attributes;
				triTransformed.sym = c.Char.UnicodeChar;

				// Mundo -> vista
				triViewed.p[0] = matrizMultVec(view, triTransformed.p[0]);
				triViewed.p[1] = matrizMultVec(view, triTransformed.p[1]);
				triViewed.p[2] = matrizMultVec(view, triTransformed.p[2]);

				// Saca la proyeccion de los graficos 3D -> 2D
				trianguloProyectado.p[0] = matrizMultVec(matrizDeProyeccion, triViewed.p[0]);
				trianguloProyectado.p[1] = matrizMultVec(matrizDeProyeccion, triViewed.p[1]);
				trianguloProyectado.p[2] = matrizMultVec(matrizDeProyeccion, triViewed.p[2]);
				trianguloProyectado.col = triTransformed.col;
				trianguloProyectado.sym = triTransformed.sym;

				trianguloProyectado.p[0] = vecDiv(trianguloProyectado.p[0], trianguloProyectado.p[0].w);
				trianguloProyectado.p[1] = vecDiv(trianguloProyectado.p[1], trianguloProyectado.p[1].w);
				trianguloProyectado.p[2] = vecDiv(trianguloProyectado.p[2], trianguloProyectado.p[2].w);

				// Offset verts into visible normalised space
				vec3d vOffsetView = { 1, 1, 0 };
				trianguloProyectado.p[0] = vecSumar(trianguloProyectado.p[0], vOffsetView);
				trianguloProyectado.p[1] = vecSumar(trianguloProyectado.p[1], vOffsetView);
				trianguloProyectado.p[2] = vecSumar(trianguloProyectado.p[2], vOffsetView);
				trianguloProyectado.p[0].x *= escala * (float)ScreenWidth();
				trianguloProyectado.p[0].y *= escala * (float)ScreenHeight();
				trianguloProyectado.p[1].x *= escala * (float)ScreenWidth();
				trianguloProyectado.p[1].y *= escala * (float)ScreenHeight();
				trianguloProyectado.p[2].x *= escala * (float)ScreenWidth();
				trianguloProyectado.p[2].y *= escala * (float)ScreenHeight();

				// Los añadimos al vector para ordenarlos luego
				vectorTriangulosOrdenados.push_back(trianguloProyectado);
			}
		}

		// Ordenar triangulos de atras hacia delante (z)
		std::sort(vectorTriangulosOrdenados.begin(), vectorTriangulosOrdenados.end(), [](triangle& t1, triangle& t2) {
			float z1 = (t1.p[0].z + t1.p[1].z + t1.p[2].z / 3.0f);
			float z2 = (t2.p[0].z + t2.p[1].z + t2.p[2].z / 3.0f);
			return z1 > z2;
		});

		for (auto &trianguloProyectado : vectorTriangulosOrdenados) {
			// Dibuja el triangulo unitario
			FillTriangle(trianguloProyectado.p[0].x, trianguloProyectado.p[0].y,
				trianguloProyectado.p[1].x, trianguloProyectado.p[1].y,
				trianguloProyectado.p[2].x, trianguloProyectado.p[2].y,
				trianguloProyectado.sym, trianguloProyectado.col);

			// Dibuja la hitbox del triangulo
			//DrawTriangle(trianguloProyectado.p[0].x, trianguloProyectado.p[0].y,
			//	trianguloProyectado.p[1].x, trianguloProyectado.p[1].y,
			//	trianguloProyectado.p[2].x, trianguloProyectado.p[2].y,
			//	PIXEL_SOLID, FG_BLACK);
		}

		return true;
	};
};


int main() {
	Engine3D demo;
	if (demo.ConstructConsole(256, 240, 4, 4))
		demo.Start();
	return 0;
}