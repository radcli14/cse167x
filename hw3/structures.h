#ifndef STRUCTURES_H
#define STRUCTURES_H

#include <glm/glm.hpp>

// Basic structures for scene objects
struct Vertex {
    glm::vec3 position;
    glm::vec3 normal;
    
    Vertex() : position(0.0f), normal(0.0f, 1.0f, 0.0f) {}
    Vertex(const glm::vec3& pos) : position(pos), normal(0.0f, 1.0f, 0.0f) {}
    Vertex(const glm::vec3& pos, const glm::vec3& norm) : position(pos), normal(norm) {}
};

struct Camera {
    glm::vec3 lookfrom;
    glm::vec3 lookat;
    glm::vec3 up;
    float fov;
    
    // Camera frame (computed as needed)
    mutable glm::vec3 u, v, w;
    mutable bool frameInitialized;

    Camera() : lookfrom(0,0,5), lookat(0,0,0), up(0,1,0), fov(30.0f),
        frameInitialized(false) {}

    void updateFrame() const {
        if (!frameInitialized) {
            w = glm::normalize(lookfrom - lookat); // camera forward (-z)
            u = glm::normalize(glm::cross(glm::normalize(up), w)); // right (x)
            v = glm::cross(w, u); // up (y)
            frameInitialized = true;
        }
    }
};

struct Light {
    glm::vec3 position;  // For point lights
    glm::vec3 direction; // For directional lights
    glm::vec3 color;
    bool isDirectional;
    glm::vec3 attenuation; // const, linear, quadratic
    
    Light() : position(0,0,0), direction(0,0,1), color(1,1,1), isDirectional(false), attenuation(1,0,0) {}
};

struct Material {
    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;
    glm::vec3 emission;
    float shininess;
    
    Material() : ambient(0.2f,0.2f,0.2f), diffuse(0.8f,0.8f,0.8f), specular(0,0,0), emission(0,0,0), shininess(0) {}
};

struct Sphere {
    glm::vec3 center;
    float radius;
    Material material;
    glm::mat4 transform;
    
    Sphere() : center(0,0,0), radius(1.0f), transform(1.0f) {}
};

struct Triangle {
    int v1, v2, v3;  // Vertex indices
    Material material;
    glm::mat4 transform;
    bool useSmoothShading;  // Flag to indicate smooth shading
    
    Triangle() : v1(0), v2(0), v3(0), transform(1.0f), useSmoothShading(false) {}
};

#endif 