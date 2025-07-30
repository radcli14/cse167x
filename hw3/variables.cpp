#include "variables.h"

// Define the global variables
Camera camera;
std::vector<Light> lights;
std::vector<Sphere> spheres;
std::vector<Triangle> triangles;
std::vector<Vertex> vertices;
std::vector<Vertex> verticesWithNormals;
Material currentMaterial;
int maxDepth = 5;
std::string outputFilename = "raytrace.png";
int imageWidth = 640, imageHeight = 480; 