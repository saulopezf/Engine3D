#include "olcConsoleGameEngine.h"
#include <fstream> // leer archivos
#include <strstream> // uso de string
#include <algorithm> // para el sort

float escala = 0.5f;
float separacion = 1.0f;

// Vector de 3 dimensiones [ x, y, z ]
struct vec3d {
	float x, y, z;
};

// Structura de los triangulos, necesarios para dibujar los graficos
// p[3]: array formado por los 3 vectores que forman el triangulo
// sym y col: variables necesarias para colorear el triangulo
struct triangle {
	vec3d p[3];

	wchar_t sym;
	short col;
};

// Mesh compuesto de triangulos y asi formar los graficos
// tris: vector de triangulos
// loadObjectFromObjFile(): funcion para cargar objetos 3D desde archivos .obj
struct mesh {
	std::vector<triangle> tris;

	bool loadObjectFromObjFile(std::string filename) {
		std::ifstream objectFile(filename);
		if (!objectFile.is_open())
			return false;

		// Local cache of verts
		std::vector<vec3d> verts;

		while (!objectFile.eof()) {
			char line[128];
			objectFile.getline(line, 128);

			std::strstream s;
			s << line;

			char junk;

			if (line[0] == 'v') {
				vec3d v;
				s >> junk >> v.x >> v.y >> v.z;
				verts.push_back(v);
			}

			if (line[0] == 'f') {
				int f[3];
				s >> junk >> f[0] >> f[1] >> f[2];
				tris.push_back({verts[f[0] - 1], verts[f[1] - 1], verts[f[2] - 1] });
			}

		}
		return true;
	}
	
};

// matriz de 4x4
struct mat4x4 {
	float m[4][4] = { 0 }; // El "= { 0 }" inicializa toda la matriz de 4x4 a ceros
};

class Engine3D : public olcConsoleGameEngine {
public:
	Engine3D() {
		m_sAppName = L"3D Demo";
	}

private:
	mesh meshCube;
	mat4x4 matrizDeProyeccion;

	vec3d vCamara;

	float fTheta;

	void MultiplicarMatrizVector(vec3d &i, vec3d &o, mat4x4 &m) {
		o.x = i.x * m.m[0][0] + i.y * m.m[1][0] + i.z * m.m[2][0] + m.m[3][0];
		o.y = i.x * m.m[0][1] + i.y * m.m[1][1] + i.z * m.m[2][1] + m.m[3][1];
		o.z = i.x * m.m[0][2] + i.y * m.m[1][2] + i.z * m.m[2][2] + m.m[3][2];
		float w = i.x * m.m[0][3] + i.y * m.m[1][3] + i.z * m.m[2][3] + m.m[3][3];

		if (w != 0.0f) {
			o.x /= w; o.y /= w; o.z /= w;
		}
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

		// Cuadrado
		//meshCube.tris = {

		//	// SOUTH
		//	{ 0.0f, 0.0f, 0.0f,    0.0f, 1.0f, 0.0f,    1.0f, 1.0f, 0.0f },
		//	{ 0.0f, 0.0f, 0.0f,    1.0f, 1.0f, 0.0f,    1.0f, 0.0f, 0.0f },

		//	// EAST                                                      
		//	{ 1.0f, 0.0f, 0.0f,    1.0f, 1.0f, 0.0f,    1.0f, 1.0f, 1.0f },
		//	{ 1.0f, 0.0f, 0.0f,    1.0f, 1.0f, 1.0f,    1.0f, 0.0f, 1.0f },

		//	// NORTH                                                     
		//	{ 1.0f, 0.0f, 1.0f,    1.0f, 1.0f, 1.0f,    0.0f, 1.0f, 1.0f },
		//	{ 1.0f, 0.0f, 1.0f,    0.0f, 1.0f, 1.0f,    0.0f, 0.0f, 1.0f },

		//	// WEST                                                      
		//	{ 0.0f, 0.0f, 1.0f,    0.0f, 1.0f, 1.0f,    0.0f, 1.0f, 0.0f },
		//	{ 0.0f, 0.0f, 1.0f,    0.0f, 1.0f, 0.0f,    0.0f, 0.0f, 0.0f },

		//	// TOP                                                       
		//	{ 0.0f, 1.0f, 0.0f,    0.0f, 1.0f, 1.0f,    1.0f, 1.0f, 1.0f },
		//	{ 0.0f, 1.0f, 0.0f,    1.0f, 1.0f, 1.0f,    1.0f, 1.0f, 0.0f },

		//	// BOTTOM                                                    
		//	{ 1.0f, 0.0f, 1.0f,    0.0f, 0.0f, 1.0f,    0.0f, 0.0f, 0.0f },
		//	{ 1.0f, 0.0f, 1.0f,    0.0f, 0.0f, 0.0f,    1.0f, 0.0f, 0.0f },

		//};
		meshCube.loadObjectFromObjFile("cuadrado.obj");

		// Matriz de proyeccion
		float fNear = 0.1f; // Distancia entre eje x
		float fFar = 1000.0f; // Hasta cuanta distancia llega la vision
		float fFov = 90.f; // FOV 90º
		float fAspectRatio = (float)ScreenHeight() / (float)ScreenWidth();
		float fFovRad = 1.0f / tanf(fFov * 0.5f / 180.0f * 3.14159f);

		matrizDeProyeccion.m[0][0] = fAspectRatio * fFovRad;
		matrizDeProyeccion.m[1][1] = fFovRad;
		matrizDeProyeccion.m[2][2] = fFar / (fFar - fNear);
		matrizDeProyeccion.m[3][2] = (-fFar * fNear) / (fFar - fNear);
		matrizDeProyeccion.m[2][3] = 1.0f;
		matrizDeProyeccion.m[3][3] = 0.0f;

		return true;
	};

	bool OnUserUpdate(float fElapsedTime) override {

		// Clear la pantalla
		Fill(0, 0, ScreenWidth(), ScreenHeight(), PIXEL_SOLID, FG_BLACK);

		// Set up rotation matrices
		mat4x4 matRotZ, matRotX;
		fTheta += 1.0f * fElapsedTime;

		// Rotation Z
		matRotZ.m[0][0] = cosf(fTheta);
		matRotZ.m[0][1] = sinf(fTheta);
		matRotZ.m[1][0] = -sinf(fTheta);
		matRotZ.m[1][1] = cosf(fTheta);
		matRotZ.m[2][2] = 1;
		matRotZ.m[3][3] = 1;

		// Rotation X
		matRotX.m[0][0] = 1;
		matRotX.m[1][1] = cosf(fTheta * 0.5f);
		matRotX.m[1][2] = sinf(fTheta * 0.5f);
		matRotX.m[2][1] = -sinf(fTheta * 0.5f);
		matRotX.m[2][2] = cosf(fTheta * 0.5f);
		matRotX.m[3][3] = 1;

		std::vector<triangle> vectorTriangulosOrdenados;

		// Dibujar todos los triangulos
		for (auto& tri : meshCube.tris) {

			// trianguloProyectado será el triangulo que proyectemos en la pantalla
			// trianguloTraducido será el triangulo que nos viene (tri) añadiendole valor a la z para que entre en nuestro campo de vision
			// trianguloRotadoZ triangulo ya transformado habiendo movido las z
			// trianguloRotadoZX triangulo ya transformado habiendo movido la z y x
			triangle trianguloProyectado, trianguloTraducido, trianguloRotadoZ, trianguloRotadoZX;

			// Rotate in Z-Axis
			MultiplicarMatrizVector(tri.p[0], trianguloRotadoZ.p[0], matRotZ);
			MultiplicarMatrizVector(tri.p[1], trianguloRotadoZ.p[1], matRotZ);
			MultiplicarMatrizVector(tri.p[2], trianguloRotadoZ.p[2], matRotZ);

			// Rotate in X-Axis
			MultiplicarMatrizVector(trianguloRotadoZ.p[0], trianguloRotadoZX.p[0], matRotX);
			MultiplicarMatrizVector(trianguloRotadoZ.p[1], trianguloRotadoZX.p[1], matRotX);
			MultiplicarMatrizVector(trianguloRotadoZ.p[2], trianguloRotadoZX.p[2], matRotX);

			// Traduciendo el triangulo añadiendole z
			trianguloTraducido = trianguloRotadoZX;
			trianguloTraducido.p[0].z = trianguloRotadoZX.p[0].z + 8.0f;
			trianguloTraducido.p[1].z = trianguloRotadoZX.p[1].z + 8.0f;
			trianguloTraducido.p[2].z = trianguloRotadoZX.p[2].z + 8.0f;

			// Calcular la normal de cada triangulo
			vec3d normal, linea1, linea2;
			linea1.x = trianguloTraducido.p[1].x - trianguloTraducido.p[0].x;
			linea1.y = trianguloTraducido.p[1].y - trianguloTraducido.p[0].y;
			linea1.z = trianguloTraducido.p[1].z - trianguloTraducido.p[0].z;

			linea2.x = trianguloTraducido.p[2].x - trianguloTraducido.p[0].x;
			linea2.y = trianguloTraducido.p[2].y - trianguloTraducido.p[0].y;
			linea2.z = trianguloTraducido.p[2].z - trianguloTraducido.p[0].z;

			normal.x = linea1.y * linea2.z - linea1.z * linea2.y;
			normal.y = linea1.z * linea2.x - linea1.x * linea2.z;
			normal.z = linea1.x * linea2.y - linea1.y * linea2.x;

			float l = sqrtf(normal.x * normal.x + normal.y * normal.y + normal.z * normal.z);
			normal.x /= l; 
			normal.y /= l;
			normal.z /= l;

			// Si el vector maximo del campo de vision hace 90º o mas respecto a la normal de cualquier traingulo
			// significa que no podemos ver ese triangulo.
			// Por lo que aplicamos esta condicion
			if(normal.x * (trianguloTraducido.p[0].x - vCamara.x) +
				normal.y * (trianguloTraducido.p[0].y - vCamara.y) +
				normal.z * (trianguloTraducido.p[0].z - vCamara.z) < 0.0f) {

				//Iluminacion
				vec3d direccion_luz = { 0.0f, 0.0f, -1.0f }; // 0 0 -1 la luz viene hacia la camara
				float l = sqrtf(direccion_luz.x* direccion_luz.x + direccion_luz.y * direccion_luz.y + direccion_luz.z * direccion_luz.z);
				direccion_luz.x /= l; direccion_luz.y /= l; direccion_luz.z /= l;

				float producto_escalar = normal.x * direccion_luz.x + normal.y * direccion_luz.y + normal.z * direccion_luz.z;

				CHAR_INFO c = GetColour(producto_escalar);
				trianguloTraducido.col = c.Attributes;
				trianguloTraducido.sym = c.Char.UnicodeChar;

				// Saca la proyeccion de los graficos 3D -> 2D
				MultiplicarMatrizVector(trianguloTraducido.p[0], trianguloProyectado.p[0], matrizDeProyeccion);
				MultiplicarMatrizVector(trianguloTraducido.p[1], trianguloProyectado.p[1], matrizDeProyeccion);
				MultiplicarMatrizVector(trianguloTraducido.p[2], trianguloProyectado.p[2], matrizDeProyeccion);
				trianguloProyectado.col = c.Attributes;
				trianguloProyectado.sym = c.Char.UnicodeChar;

				// Escalar los graficos
				trianguloProyectado.p[0].x += separacion;
				trianguloProyectado.p[0].y += separacion;
				trianguloProyectado.p[1].x += separacion;
				trianguloProyectado.p[1].y += separacion;
				trianguloProyectado.p[2].x += separacion;
				trianguloProyectado.p[2].y += separacion;
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

			// Dibuja el triangulo unitario
			DrawTriangle(trianguloProyectado.p[0].x, trianguloProyectado.p[0].y,
				trianguloProyectado.p[1].x, trianguloProyectado.p[1].y,
				trianguloProyectado.p[2].x, trianguloProyectado.p[2].y,
				PIXEL_SOLID, FG_BLACK);
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