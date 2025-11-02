#pragma once
#include <vector>
#include <strstream> // uso de string
#include <fstream> // leer archivos

// Vector de 3 dimensiones [ x, y, z ]
struct vec3d {
	float x = 0;
	float y = 0;
	float z = 0;
	float w = 1;
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
				tris.push_back({ verts[f[0] - 1], verts[f[1] - 1], verts[f[2] - 1] });
			}

		}
		return true;
	}

};

// matriz de 4x4
struct mat4x4 {
	float m[4][4] = { 0 }; // El "= { 0 }" inicializa toda la matriz de 4x4 a ceros
};