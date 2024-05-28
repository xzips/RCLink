#pragma once

#include <vector>
#include <string>
#include "SFML/Graphics.hpp"


namespace q3d
{
	struct SparseVertex
	{
		float ox, oy, oz; //original x, y, z from file
		float x, y, z; // transformed x, y, z for the partciular use case
		
	};
	
	//typedef std::vector<SparseVertex> SparseVertexField;
	

	typedef struct SparseVectorField
	{
		std::string name;
		std::vector<SparseVertex> vertices;

	} SVF;

	struct Triangle
	{
		float x1, y1, z1;
		float x2, y2, z2;
		float x3, y3, z3;

		float ox1, oy1, oz1;
		float ox2, oy2, oz2;
		float ox3, oy3, oz3;

		float nx, ny, nz;

		sf::Color color;
		
	};

	typedef struct TriangleMesh
	{
		std::string name;
		std::vector<Triangle> triangles;
		float scale = 1.0f;
		float x = 0.0f, y = 0.0f, z = 0.0f;
		float angleX = 0.0f, angleY = 0.0f, angleZ = 0.0f;
		
	} TM;


	//typedef SparseVertexField SVF;

	void LoadSparseVertexField(SVF& svf, const char* filename);

	void PositionSVF(SVF& svf, float x, float y, float z);

	void ScaleSVF(SVF& svf, float scale);
	
	
	sf::Vector2f ProjectVertex(const SparseVertex& vertex, float fov, float aspectRatio, float width, float height);

	void DrawSVF(sf::RenderWindow& window, SVF& svf);

	

	void LoadTriangleMeshOBJ(TM& tm, const char* filename);


	void DrawTM(sf::RenderWindow& window, TM& tm);

	void OffsetOriginalCoords(TM& tm, float x, float y, float z);
	
	void SetPositionTM(TM& tm, float x, float y, float z);
	void SetScaleTM(TM& tm, float scale);
	void SetRotationTM(TM& tm, float angleX, float angleY, float angleZ);
	void VertexTransformUpdateTM(TM& tm);



}