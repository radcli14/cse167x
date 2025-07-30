#ifndef VARIABLES_H
#define VARIABLES_H

#include <vector>
#include <string>
#include "structures.h"

// Global variables for scene data
extern Camera camera;
extern std::vector<Light> lights;
extern std::vector<Sphere> spheres;
extern std::vector<Triangle> triangles;
extern std::vector<Vertex> vertices;
extern std::vector<Vertex> verticesWithNormals;
extern Material currentMaterial;
extern int maxDepth;
extern std::string outputFilename;
extern int imageWidth, imageHeight;

#endif 