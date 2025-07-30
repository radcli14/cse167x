#ifndef RAYTRACE_H
#define RAYTRACE_H

#include <vector>
#include <glm/glm.hpp>
#include "structures.h"

// Forward declarations
struct Ray;
struct Intersection;
struct Scene;
class Image;

// Ray structure
struct Ray {
    glm::vec3 origin;
    glm::vec3 direction;
    
    Ray() : origin(0.0f), direction(0.0f, 0.0f, -1.0f) {}
    Ray(const glm::vec3& o, const glm::vec3& d) : origin(o), direction(glm::normalize(d)) {}
};

// Intersection structure
struct Intersection {
    bool hit;
    float t;
    glm::vec3 point;
    glm::vec3 normal;
    Material material;
    
    Intersection() : hit(false), t(INFINITY), point(0.0f), normal(0.0f, 1.0f, 0.0f) {}
};

// Scene structure to hold all objects
struct Scene {
    std::vector<Sphere> spheres;
    std::vector<Triangle> triangles;
    std::vector<Light> lights;
    Material globalAmbient;
    
    Scene() {
        globalAmbient.ambient = glm::vec3(0.2f, 0.2f, 0.2f);
    }
};

// Image class
class Image {
private:
    int width, height;
    std::vector<std::vector<glm::vec3>> pixels;

public:
    Image(int w, int h);
    glm::vec3& operator()(int i, int j);
    const glm::vec3& operator()(int i, int j) const;
    void save(const std::string& filename);
    int getWidth() const { return width; }
    int getHeight() const { return height; }
};

// Ray tracing functions
Ray RayThruPixel(const Camera& cam, int i, int j, int width, int height);
Intersection Intersect(const Ray& ray, const Scene& scene);
glm::vec3 FindColor(const Intersection& hit, const Scene& scene);
Image Raytrace(const Camera& cam, const Scene& scene, int width, int height);
void ShowRaytraceWindow(const Image& image, int width, int height);

#endif 