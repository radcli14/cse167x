#include "raytrace.h"
#include <iostream>
#include <cmath>
#include <FreeImage.h>

// Image class implementation
Image::Image(int w, int h) : width(w), height(h) {
    pixels.resize(height);
    for (int i = 0; i < height; i++) {
        pixels[i].resize(width);
        for (int j = 0; j < width; j++) {
            pixels[i][j] = glm::vec3(0.0f, 0.0f, 0.0f); // Initialize to black
        }
    }
}

glm::vec3& Image::operator()(int i, int j) {
    return pixels[i][j];
}

const glm::vec3& Image::operator()(int i, int j) const {
    return pixels[i][j];
}

void Image::save(const std::string& filename) {
    // TODO: Implement image saving using FreeImage
    std::cout << "Saving image to: " << filename << std::endl;
}

// Ray tracing function implementations
Ray RayThruPixel(const Camera& cam, int i, int j, int width, int height) {
    // TODO: Implement ray generation through pixel (i,j)
    // This should create a ray from the camera through the specified pixel
    
    // Placeholder implementation
    glm::vec3 origin = cam.lookfrom;
    glm::vec3 direction = glm::normalize(cam.lookat - cam.lookfrom);
    
    return Ray(origin, direction);
}

Intersection Intersect(const Ray& ray, const Scene& scene) {
    // TODO: Implement ray-object intersection testing
    // This should test the ray against all spheres and triangles in the scene
    // and return the closest intersection
    
    Intersection hit;
    
    // Placeholder: just return no hit for now
    return hit;
}

glm::vec3 FindColor(const Intersection& hit, const Scene& scene) {
    // TODO: Implement lighting calculation
    // This should compute the color at the intersection point using
    // the material properties and scene lighting
    
    if (!hit.hit) {
        return glm::vec3(0.0f, 0.0f, 0.0f); // Background color (black)
    }
    
    // Placeholder: return material diffuse color
    return hit.material.diffuse;
}

Image Raytrace(const Camera& cam, const Scene& scene, int width, int height) {
    std::cout << "Starting ray tracing..." << std::endl;
    std::cout << "Image size: " << width << "x" << height << std::endl;
    std::cout << "Scene contains: " << scene.spheres.size() << " spheres, " 
              << scene.triangles.size() << " triangles, " << scene.lights.size() << " lights" << std::endl;
    
    Image image(width, height);
    int totalPixels = width * height;
    int processedPixels = 0;
    
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            processedPixels++;
            
            // Show progress every 1000 pixels
            if (processedPixels % 1000 == 0) {
                std::cout << "Progress: " << processedPixels << "/" << totalPixels 
                          << " pixels (" << (processedPixels * 100 / totalPixels) << "%)" << std::endl;
            }
            
            Ray ray = RayThruPixel(cam, i, j, width, height);
            Intersection hit = Intersect(ray, scene);
            image(i, j) = FindColor(hit, scene);
        }
    }
    
    std::cout << "Ray tracing completed!" << std::endl;
    return image;
} 