#include <vector>
#include "Quick3D.hpp"
#include <fstream>
#include <sstream>
#include <iostream>
#include <string>
#include "SFML/Graphics.hpp"

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <SFML/System/Vector3.hpp>
namespace q3d
{
    //from obj only
    void LoadSparseVertexField(SVF& svf, const char* filename)
    {
        std::ifstream file(filename);
        if (!file.is_open())
        {
            std::cerr << "Failed to open file: " << filename << std::endl;
            return;
        }

        std::string line;
        while (std::getline(file, line))
        {
            std::istringstream ss(line);
            std::string prefix;
            ss >> prefix;

            if (prefix == "v")
            {
                SparseVertex vertex;
                ss >> vertex.ox >> vertex.oy >> vertex.oz;
                vertex.x = vertex.ox;
                vertex.y = vertex.oy;
                vertex.z = vertex.oz;
                svf.vertices.push_back(vertex);
            }
        }

		//set the name of the SVF to the filename minus the path
		std::string name = filename;
		size_t lastSlash = name.find_last_of("/\\");
		if (lastSlash != std::string::npos)
		{
			name = name.substr(lastSlash + 1);
		}
		svf.name = name;
        
        

        file.close();
    }

sf::Color LerpColors(float w = 0, sf::Color a = sf::Color::Red, sf::Color b = sf::Color::Blue)
{
    return sf::Color(
        static_cast<sf::Uint8>(a.r + w * (b.r - a.r)),
        static_cast<sf::Uint8>(a.g + w * (b.g - a.g)),
        static_cast<sf::Uint8>(a.b + w * (b.b - a.b))
    );



}


sf::Color TriLerp(float w,
    sf::Color a = sf::Color(77, 77, 166),
    sf::Color b = sf::Color(33, 150, 243),
    sf::Color c  = sf::Color(200, 200, 200))
{
    if (w < 0.5f)
    {
        float localW = w * 2.0f; // scale w to [0, 1] for the first half
        return sf::Color(
            static_cast<sf::Uint8>(a.r + localW * (b.r - a.r)),
            static_cast<sf::Uint8>(a.g + localW * (b.g - a.g)),
            static_cast<sf::Uint8>(a.b + localW * (b.b - a.b))
        );
    }
    else
    {
        float localW = (w - 0.5f) * 2.0f; // scale w to [0, 1] for the second half
        return sf::Color(
            static_cast<sf::Uint8>(b.r + localW * (c.r - b.r)),
            static_cast<sf::Uint8>(b.g + localW * (c.g - b.g)),
            static_cast<sf::Uint8>(b.b + localW * (c.b - b.b))
        );
    }
}


void LoadTriangleMeshOBJ(TM& tm, const char* filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Failed to open file: " << filename << std::endl;
        return;
    }

    std::vector<sf::Vector3f> vertices;
    std::vector<sf::Vector3f> normals;
    std::string line;

    while (std::getline(file, line)) {
        std::istringstream ss(line);
        std::string prefix;
        ss >> prefix;

        if (prefix == "v") {
            float x, y, z;
            ss >> x >> y >> z;
            vertices.emplace_back(x, y, z);
        }
        else if (prefix == "vn") {
            float nx, ny, nz;
            ss >> nx >> ny >> nz;
            normals.emplace_back(nx, ny, nz);
        }
        else if (prefix == "f") {
            std::string v1_str, v2_str, v3_str;
            ss >> v1_str >> v2_str >> v3_str;

            auto parseVertexIndex = [](const std::string& str) -> std::pair<unsigned int, unsigned int> {
                size_t pos1 = str.find("//");
                if (pos1 != std::string::npos) {
                    unsigned int v_idx = std::stoi(str.substr(0, pos1));
                    unsigned int vn_idx = std::stoi(str.substr(pos1 + 2));
                    return { v_idx, vn_idx };
                }
                return { std::stoi(str), 0 }; // No normal index provided
                };

            auto [v1, vn1] = parseVertexIndex(v1_str);
            auto [v2, vn2] = parseVertexIndex(v2_str);
            auto [v3, vn3] = parseVertexIndex(v3_str);

            Triangle triangle;
            triangle.ox1 = vertices[v1 - 1].x;
            triangle.oy1 = vertices[v1 - 1].y;
            triangle.oz1 = vertices[v1 - 1].z;

            triangle.ox2 = vertices[v2 - 1].x;
            triangle.oy2 = vertices[v2 - 1].y;
            triangle.oz2 = vertices[v2 - 1].z;

            triangle.ox3 = vertices[v3 - 1].x;
            triangle.oy3 = vertices[v3 - 1].y;
            triangle.oz3 = vertices[v3 - 1].z;

            triangle.x1 = triangle.ox1;
            triangle.y1 = triangle.oy1;
            triangle.z1 = triangle.oz1;

            triangle.x2 = triangle.ox2;
            triangle.y2 = triangle.oy2;
            triangle.z2 = triangle.oz2;

            triangle.x3 = triangle.ox3;
            triangle.y3 = triangle.oy3;
            triangle.z3 = triangle.oz3;

            if (vn1 && vn2 && vn3) {
                triangle.nx = (normals[vn1 - 1].x + normals[vn2 - 1].x + normals[vn3 - 1].x) / 3;
                triangle.ny = (normals[vn1 - 1].y + normals[vn2 - 1].y + normals[vn3 - 1].y) / 3;
                triangle.nz = (normals[vn1 - 1].z + normals[vn2 - 1].z + normals[vn3 - 1].z) / 3;
            }
            else {
                triangle.nx = triangle.ny = triangle.nz = 0; // Default normal if not provided
            }

            tm.triangles.push_back(triangle);
        }
    }

    std::string name = filename;
    size_t lastSlash = name.find_last_of("/\\");
    if (lastSlash != std::string::npos) {
        name = name.substr(lastSlash + 1);
    }
    tm.name = name;

    file.close();

    //finally we want to center model around 0, 0, 0, so comptte the average of all vertices and subtract that from all vertices
    float avgX = 0;
    float avgY = 0;
    float avgZ = 0;

    for (const Triangle& triangle : tm.triangles)
    {
        avgX += (triangle.ox1 + triangle.ox2 + triangle.ox3) / 3;
        avgY += (triangle.oy1 + triangle.oy2 + triangle.oy3) / 3;
        avgZ += (triangle.oz1 + triangle.oz2 + triangle.oz3) / 3;
    }

    avgX /= tm.triangles.size();
    avgY /= tm.triangles.size();
    avgZ /= tm.triangles.size();

    for (Triangle& triangle : tm.triangles)
    {
        triangle.ox1 = triangle.ox1 - avgX;
        triangle.oy1 = triangle.oy1 - avgY;
        triangle.oz1 = triangle.oz1 - avgZ;

        triangle.ox2 = triangle.ox2 - avgX;
        triangle.oy2 = triangle.oy2 - avgY;
        triangle.oz2 = triangle.oz2 - avgZ;

        triangle.ox3 = triangle.ox3 - avgX;
        triangle.oy3 = triangle.oy3 - avgY;
        triangle.oz3 = triangle.oz3 - avgZ;
    }

    float maxdepth = -std::numeric_limits<float>::max();

    for (const auto& triangle : tm.triangles)
    {
        maxdepth = std::max(maxdepth, std::max(triangle.z1, std::max(triangle.z2, triangle.z3)));
    }


    float mindepth = std::numeric_limits<float>::max();

    for (const auto& triangle : tm.triangles)

    {
        mindepth = std::min(mindepth, std::min(triangle.z1, std::min(triangle.z2, triangle.z3)));

    }

    int len = tm.triangles.size();

    //set color property of each triangle
    for (int i = 0; i < len; i++)
    {
        //tm.triangles[i].color = LerpColors((tm.triangles[i].z1 - mindepth) / (maxdepth - mindepth), sf::Color::Red, sf::Color::Green);
        tm.triangles[i].color = TriLerp((tm.triangles[i].z1 - mindepth) / (maxdepth - mindepth));
    }

}


    





    
    void PositionSVF(SVF& svf, float x, float y, float z)
    {
		for (SparseVertex& vertex : svf.vertices)
		{
			vertex.x = vertex.ox + x;
			vertex.y = vertex.oy + y;
			vertex.z = vertex.oz + z;

		}
    }

    sf::Vector2f ProjectVertex(const SparseVertex& vertex, float fov, float aspectRatio, float width, float height)
    {
        // Perspective projection parameters
        float fovRad = 1.0f / tan(fov * 0.5f / 180.0f * 3.14159f);

        // Perspective transformation
        float x = vertex.x * fovRad * aspectRatio;
        float y = vertex.y * fovRad;
        float z = vertex.z;

        // Normalize to screen coordinates
        float screenX = (x / z) * (width / 2.0f) + (width / 2.0f);
        float screenY = (y / z) * (height / 2.0f) + (height / 2.0f);

        return sf::Vector2f(screenX, screenY);
    }

    void DrawSVF(sf::RenderWindow& window, SVF& svf)
    {

        //print length of sv verts for debugging
		//std::cout << "Drawing SVF: " << svf.name << " with " << svf.vertices.size() << " vertices" << std::endl;

        // Define the perspective projection parameters
        float fov = 90.0f;
        float aspectRatio = window.getSize().x / static_cast<float>(window.getSize().y);
        float width = static_cast<float>(window.getSize().x);
        float height = static_cast<float>(window.getSize().y);

        sf::CircleShape point(2);
        point.setFillColor(sf::Color::White);


        // Render vertices
        for (const auto& vertex : svf.vertices)
        {
            sf::Vector2f screenPos = ProjectVertex(vertex, fov, aspectRatio, width, height);
           
            point.setPosition(screenPos);
            
            window.draw(point);
        }
    }

    void ScaleSVF(SVF& svf, float scale)
    {
		for (SparseVertex& vertex : svf.vertices)
		{
			float original_offset_x = vertex.x - vertex.ox;
			float original_offset_y = vertex.y - vertex.oy;
			float original_offset_z = vertex.z - vertex.oz;
            


			vertex.x = vertex.ox * scale;
			vertex.y = vertex.oy * scale;
			vertex.z = vertex.oz * scale;

			vertex.x += original_offset_x;
			vertex.y += original_offset_y;
			vertex.z += original_offset_z;
            

            
		}
    }


    sf::Vector2f ProjectVertex(const sf::Vector3f& vertex, float fov, float aspectRatio, float width, float height) {
        // Perspective projection parameters
        float fovRad = 1.0f / tan(fov * 0.5f * 3.14159f / 180.0f);

        // Perspective transformation
        float x = vertex.x * fovRad * aspectRatio;
        float y = vertex.y * fovRad;
        float z = vertex.z;

        // Prevent division by zero (or very close to zero values)
        if (std::abs(z) < 1e-6) {
            z = 1e-6;
        }

        // Normalize to screen coordinates
        float screenX = (x / z) * (width / 2.0f) + (width / 2.0f);
        float screenY = (y / z) * (height / 2.0f) + (height / 2.0f);

        return sf::Vector2f(screenX, screenY);
    }

    


    
    void DrawTMSLOW(sf::RenderWindow& window, TM& tm)
    {
        // Define the perspective projection parameters
        float fov = 90.0f;
        float aspectRatio = window.getSize().x / static_cast<float>(window.getSize().y);
        float width = static_cast<float>(window.getSize().x);
        float height = static_cast<float>(window.getSize().y);


        sf::Vector2f p1;
        sf::Vector2f p2;
        sf::Vector2f p3;

        


        // Render triangles
        for (const auto& triangle : tm.triangles)
        {

            sf::Color col = triangle.color;

            // Project each vertex of the triangle
            p1 = ProjectVertex(sf::Vector3f(triangle.x1, triangle.y1, triangle.z1), fov, aspectRatio, width, height);
            p2 = ProjectVertex(sf::Vector3f(triangle.x2, triangle.y2, triangle.z2), fov, aspectRatio, width, height);
            p3 = ProjectVertex(sf::Vector3f(triangle.x3, triangle.y3, triangle.z3), fov, aspectRatio, width, height);

            // Draw lines between the vertices of the triangle
            sf::Vertex line1[] = { sf::Vertex(p1, col), sf::Vertex(p2, col) };
            sf::Vertex line2[] = { sf::Vertex(p2, col), sf::Vertex(p3, col) };
            sf::Vertex line3[] = { sf::Vertex(p3, col), sf::Vertex(p1, col) };

            window.draw(line1, 2, sf::Lines);
            window.draw(line2, 2, sf::Lines);
            window.draw(line3, 2, sf::Lines);
            
        }
    }

    void DrawTM(sf::RenderWindow& window, TM& tm)
    {
        float fov = 90.0f;
        float aspectRatio = window.getSize().x / static_cast<float>(window.getSize().y);
        float width = static_cast<float>(window.getSize().x);
        float height = static_cast<float>(window.getSize().y);

        // Pre-calculate the number of vertices needed
        size_t totalVertices = tm.triangles.size() * 6; // 3 lines per triangle, 2 vertices per line
        sf::VertexArray lines(sf::Lines, totalVertices);

        size_t currentIndex = 0;
        for (const auto& triangle : tm.triangles)
        {
            sf::Color col = triangle.color;
            sf::Vector2f p1 = ProjectVertex(sf::Vector3f(triangle.x1, triangle.y1, triangle.z1), fov, aspectRatio, width, height);
            sf::Vector2f p2 = ProjectVertex(sf::Vector3f(triangle.x2, triangle.y2, triangle.z2), fov, aspectRatio, width, height);
            sf::Vector2f p3 = ProjectVertex(sf::Vector3f(triangle.x3, triangle.y3, triangle.z3), fov, aspectRatio, width, height);

            lines[currentIndex++] = sf::Vertex(p1, col);
            lines[currentIndex++] = sf::Vertex(p2, col);
            lines[currentIndex++] = sf::Vertex(p2, col);
            lines[currentIndex++] = sf::Vertex(p3, col);
            lines[currentIndex++] = sf::Vertex(p3, col);
            lines[currentIndex++] = sf::Vertex(p1, col);
        }

        window.draw(lines);
    }




    // Helper functions for rotations
    void RotateX(float& y, float& z, float angle) {
        float cosAngle = cos(angle);
        float sinAngle = sin(angle);

        float newY = y * cosAngle - z * sinAngle;
        float newZ = y * sinAngle + z * cosAngle;

        y = newY;
        z = newZ;
    }

    void RotateY(float& x, float& z, float angle) {
        float cosAngle = cos(angle);
        float sinAngle = sin(angle);

        float newX = x * cosAngle + z * sinAngle;
        float newZ = -x * sinAngle + z * cosAngle;

        x = newX;
        z = newZ;
    }

    void RotateZ(float& x, float& y, float angle) {
        float cosAngle = cos(angle);
        float sinAngle = sin(angle);

        float newX = x * cosAngle - y * sinAngle;
        float newY = x * sinAngle + y * cosAngle;

        x = newX;
        y = newY;
    }


    // Set the position of the TM
    void SetPositionTM(TM& tm, float x, float y, float z) {
        tm.x = x;
        tm.y = y;
        tm.z = z;
    }

    // Set the scale of the TM
    void SetScaleTM(TM& tm, float scale) {
        tm.scale = scale;
    }

    // Set the rotation of the TM
    void SetRotationTM(TM& tm, float angleX, float angleY, float angleZ) {
        tm.angleX = angleX;
        tm.angleY = angleY;
        tm.angleZ = angleZ;
    }

    // Update the vertex transformation
    void VertexTransformUpdateTM(TM& tm) {
        for (Triangle& triangle : tm.triangles) {
            // Set all x1, y1, z1, ... to ox1, oy1, oz1, ...
            triangle.x1 = triangle.ox1;
            triangle.y1 = triangle.oy1;
            triangle.z1 = triangle.oz1;

            triangle.x2 = triangle.ox2;
            triangle.y2 = triangle.oy2;
            triangle.z2 = triangle.oz2;

            triangle.x3 = triangle.ox3;
            triangle.y3 = triangle.oy3;
            triangle.z3 = triangle.oz3;

            // Perform rotation
            // Perform rotation around the center (0,0,0)
            RotateX(triangle.y1, triangle.z1, tm.angleX);
            RotateY(triangle.x1, triangle.z1, tm.angleY);
            RotateZ(triangle.x1, triangle.y1, tm.angleZ);

            RotateX(triangle.y2, triangle.z2, tm.angleX);
            RotateY(triangle.x2, triangle.z2, tm.angleY);
            RotateZ(triangle.x2, triangle.y2, tm.angleZ);

            RotateX(triangle.y3, triangle.z3, tm.angleX);
            RotateY(triangle.x3, triangle.z3, tm.angleY);
            RotateZ(triangle.x3, triangle.y3, tm.angleZ);

            // Perform scaling
            triangle.x1 *= tm.scale;
            triangle.y1 *= tm.scale;
            triangle.z1 *= tm.scale;

            triangle.x2 *= tm.scale;
            triangle.y2 *= tm.scale;
            triangle.z2 *= tm.scale;

            triangle.x3 *= tm.scale;
            triangle.y3 *= tm.scale;
            triangle.z3 *= tm.scale;

            // Perform translation
            triangle.x1 += tm.x;
            triangle.y1 += tm.y;
            triangle.z1 += tm.z;

            triangle.x2 += tm.x;
            triangle.y2 += tm.y;
            triangle.z2 += tm.z;

            triangle.x3 += tm.x;
            triangle.y3 += tm.y;
            triangle.z3 += tm.z;
        }


    }



    void OffsetOriginalCoords(TM& tm, float x, float y, float z)
    {
        for (auto& t : tm.triangles)
        {
			t.ox1 += x;
			t.oy1 += y;
			t.oz1 += z;

			t.ox2 += x;
			t.oy2 += y;
			t.oz2 += z;

			t.ox3 += x;
			t.oy3 += y;
			t.oz3 += z;

        }
        

    }


}