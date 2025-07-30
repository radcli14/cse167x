#include "raytrace.h"
#include <iostream>
#include <cmath>
#include <FreeImage.h>
#include <GLUT/glut.h>

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
    // Convert image data to FreeImage format
    FIBITMAP* bitmap = FreeImage_Allocate(width, height, 24);
    if (!bitmap) {
        std::cerr << "Error: Could not allocate FreeImage bitmap" << std::endl;
        return;
    }
    
    // Copy pixel data
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            const glm::vec3& color = pixels[i][j];
            RGBQUAD pixel;
            // FreeImage uses BGR order, so we need to swap R and B
            pixel.rgbBlue = static_cast<BYTE>(color.r * 255.0f);   // Red goes to Blue channel
            pixel.rgbGreen = static_cast<BYTE>(color.g * 255.0f);  // Green stays Green
            pixel.rgbRed = static_cast<BYTE>(color.b * 255.0f);    // Blue goes to Red channel
            pixel.rgbReserved = 0;
            
            FreeImage_SetPixelColor(bitmap, j, height - 1 - i, &pixel);
        }
    }
    
    // Save the image
    if (FreeImage_Save(FIF_PNG, bitmap, filename.c_str(), 0)) {
        std::cout << "Successfully saved image to: " << filename << std::endl;
    } else {
        std::cerr << "Error: Could not save image to: " << filename << std::endl;
    }
    
    FreeImage_Unload(bitmap);
}

// Ray tracing function implementations
Ray RayThruPixel(const Camera& cam, int i, int j, int width, int height) {
    // Ensure camera frame is up to date
    cam.updateFrame();

    // Image plane size
    float aspect = static_cast<float>(width) / static_cast<float>(height);
    float fovyRad = glm::radians(cam.fov);
    float fovxRad = aspect * fovyRad;

    // Horizontal alpha and vertical beta from canonical viewing slide, adding 0.5f to j and i to center the pixel
    float alpha = tan(0.5f * fovxRad) * (j + 0.5f - 0.5f * width) / (0.5f * width);
    float beta = tan(0.5f * fovyRad) * (0.5f * height - i - 0.5f) / (0.5f * height);

    // Ray direction in world space
    glm::vec3 dir = glm::normalize(alpha * cam.u + beta * cam.v - cam.w);
    return Ray(cam.lookfrom, dir);
}

Intersection Intersect(const Ray& ray, const Scene& scene) {
    Intersection hit;
    hit.ray = ray;  // Store the ray for access to camera/view direction
    float closest_t = INFINITY;
    float epsilon = 0.000001f; // Small value to avoid self-intersection

    // Loop over all spheres
    for (size_t i = 0; i < scene.spheres.size(); ++i) {
        const Sphere& sphere = scene.spheres[i];
        
        // Ray-sphere intersection using quadratic formula
        // Ray: P(t) = ray.origin + t * ray.direction
        // Sphere: |P - sphere.center|^2 = sphere.radius^2
        // Substitute ray equation into sphere equation and solve for t
        
        glm::vec3 oc = ray.origin - sphere.center;
        float a = glm::dot(ray.direction, ray.direction);
        float b = 2.0f * glm::dot(oc, ray.direction);
        float c = glm::dot(oc, oc) - sphere.radius * sphere.radius;
        
        float discriminant = b * b - 4 * a * c;
        
        if (discriminant > 0) {
            // Two intersection points
            float t1 = (-b - sqrt(discriminant)) / (2.0f * a);
            float t2 = (-b + sqrt(discriminant)) / (2.0f * a);
            
            // Use the closer intersection point (smaller t value)
            float t = (t1 > 0.0f) ? t1 : t2;
            
            if (t > epsilon && t < closest_t) {
                closest_t = t;
                hit.hit = true;
                hit.t = t;
                hit.point = ray.origin + t * ray.direction;
                hit.normal = glm::normalize(hit.point - sphere.center);
                hit.material = sphere.material;
            }
        }
    }
    
    // Loop over all triangles
    for (size_t i = 0; i < scene.triangles.size(); ++i) {
        const Triangle& tri = scene.triangles[i];
        // Get triangle vertices (assume global vertices vector)
        extern std::vector<Vertex> vertices;
        const glm::vec3& v0 = vertices[tri.v1].position;
        const glm::vec3& v1 = vertices[tri.v2].position;
        const glm::vec3& v2 = vertices[tri.v3].position;

        // Compute plane normal
        glm::vec3 n = glm::normalize(glm::cross(v1 - v0, v2 - v0));
        float denom = glm::dot(n, ray.direction);
        if (fabs(denom) < 1e-6f) continue; // Parallel, no intersection

        float t = glm::dot(n, v0 - ray.origin) / denom;
        if (t < epsilon) continue; // Intersection behind ray origin
        if (t > closest_t) continue; // Intersection is behind the closest intersection

        glm::vec3 p = ray.origin + t * ray.direction;

        // Barycentric coordinates
        glm::vec3 v0v1 = v1 - v0;
        glm::vec3 v0v2 = v2 - v0;
        glm::vec3 v0p = p - v0;
        float d00 = glm::dot(v0v1, v0v1);
        float d01 = glm::dot(v0v1, v0v2);
        float d11 = glm::dot(v0v2, v0v2);
        float d20 = glm::dot(v0p, v0v1);
        float d21 = glm::dot(v0p, v0v2);
        float denom_bary = d00 * d11 - d01 * d01;
        if (fabs(denom_bary) < 1e-8f) continue; // Degenerate triangle
        float v = (d11 * d20 - d01 * d21) / denom_bary;
        float w = (d00 * d21 - d01 * d20) / denom_bary;
        float u = 1.0f - v - w;
        if (u >= 0.0f && v >= 0.0f && w >= 0.0f && u <= 1.0f && v <= 1.0f && w <= 1.0f) {
            closest_t = t;
            hit.hit = true;
            hit.t = t;
            hit.point = p;
            hit.normal = n;
            hit.material = tri.material;
        }
    }
    return hit;
}

glm::vec3 FindColor(const Intersection& hit, const Scene& scene) {
    if (hit.hit) {
        float epsilon = 0.000001f; // Small value to avoid self-intersection
        
        // Start with ambient and emission
        glm::vec3 color = hit.material.ambient + hit.material.emission;
        
        // Loop through all lights
        for (size_t i = 0; i < scene.lights.size(); ++i) {
            const Light& light = scene.lights[i];
            
            // Calculate light direction vector and distance
            glm::vec3 lightDir;
            float attenuation = 1.0f;
            float distanceToLight;
            if (light.isDirectional) {
                // Directional light: direction is constant, no distance attenuation
                lightDir = glm::normalize(light.direction);
                distanceToLight = INFINITY; // No distance attenuation for directional lights
            } else {
                // Point light: direction from hit point to light position
                lightDir = glm::normalize(light.position - hit.point);
                distanceToLight = glm::length(light.position - hit.point);
                attenuation = 1.0f / (light.attenuation[0] + light.attenuation[1] * distanceToLight + light.attenuation[2] * (distanceToLight * distanceToLight));
            }
            
            // Check if light is blocked by an object, if it is, it is shadowed, so don't add that light
            // Offset ray origin slightly to avoid self-intersection
            glm::vec3 shadowRayOrigin = hit.point + epsilon * hit.normal;
            Ray lightRay = Ray(shadowRayOrigin, lightDir);
            Intersection lightHit = Intersect(lightRay, scene);
            
            // Check if shadow ray hits something closer than the light
            if (lightHit.hit && lightHit.t < distanceToLight) {
                continue; // Light is blocked by an object
            }
            
            // Calculate view direction using the ray direction from the intersection
            // The ray direction points from camera to hit point, so we negate it for view direction
            glm::vec3 viewDir = -hit.ray.direction; // View direction is opposite of ray direction
            
            // Calculate half vector for specular calculations
            // Half vector is halfway between view direction and light direction
            glm::vec3 halfVector = glm::normalize(viewDir + lightDir);
            
            // Calculate diffuse lighting
            float NdotL = glm::dot(hit.normal, lightDir);
            if (NdotL > 0.0f) {
                color += attenuation * light.color * hit.material.diffuse * NdotL;
            }
            
            // Calculate specular lighting
            float NdotH = glm::dot(hit.normal, halfVector);
            if (NdotH > 0.0f) {
                color += attenuation * light.color * hit.material.specular * pow(NdotH, hit.material.shininess);
            }

            // For now, just print the computed vectors for debugging
            std::cout << "Light " << i << ": dir=(" << lightDir.x << "," << lightDir.y << "," << lightDir.z 
                      << ") dist=" << distanceToLight 
                      << " view=(" << viewDir.x << "," << viewDir.y << "," << viewDir.z << ")"
                      << " half=(" << halfVector.x << "," << halfVector.y << "," << halfVector.z << ")" << std::endl;
        }
        
        // Return the computed color
        return color;
    } else {
        return glm::vec3(0.0f, 0.0f, 0.0f); // Black for misses
    }
}

Image Raytrace(const Camera& cam, const Scene& scene, int width, int height) {
    std::cout << "Starting ray tracing..." << std::endl;
    std::cout << "Image size: " << width << "x" << height << std::endl;
    std::cout << "Scene contains: " << scene.spheres.size() << " spheres, " 
              << scene.triangles.size() << " triangles, " << scene.lights.size() << " lights" << std::endl;
    
    Image image(width, height);
    int totalPixels = width * height;
    int processedPixels = 0;
    int pixelsWithHit = 0;
    
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            processedPixels++;
            
            // Show progress every 1000 pixels
            if (processedPixels % 10000 == 0) {
                std::cout << "Progress: " << processedPixels << "/" << totalPixels 
                          << " pixels (" << (processedPixels * 100 / totalPixels) << "%)" 
                          << " pixels with hit: " << pixelsWithHit << std::endl;
            }
            
            Ray ray = RayThruPixel(cam, i, j, width, height);
            Intersection hit = Intersect(ray, scene);
            if (hit.hit) {
                pixelsWithHit++;
            }
            image(i, j) = FindColor(hit, scene);
        }
    }
    
    std::cout << "Ray tracing completed! Pixels with hits: " << pixelsWithHit << std::endl;
    return image;
} 

// Global variables for GLUT display
static std::vector<unsigned char> g_displayBuffer;
static int g_displayWidth = 0, g_displayHeight = 0;

void display() {
    glClear(GL_COLOR_BUFFER_BIT);
    if (g_displayBuffer.size() > 0) {
        // Set the raster position to the bottom-left corner
        glRasterPos2i(0, 0);
        // Draw the pixels
        glDrawPixels(g_displayWidth, g_displayHeight, GL_RGB, GL_UNSIGNED_BYTE, g_displayBuffer.data());
    }
    glutSwapBuffers();
}

void keyboard(unsigned char key, int x, int y) {
    switch(key) {
        case 27:  // Escape to quit
            exit(0);
            break;
        case 's':  // Save screenshot
            // TODO: Implement screenshot saving
            std::cout << "Screenshot saved (not implemented yet)" << std::endl;
            break;
    }
}

void ShowRaytraceWindow(const Image& image, int width, int height) {
    g_displayWidth = width;
    g_displayHeight = height;
    
    // Convert image to display buffer
    g_displayBuffer.resize(width * height * 3);
    
    for (int i = 0; i < height; ++i) {
        for (int j = 0; j < width; ++j) {
            int idx = 3 * ((height - 1 - i) * width + j); // OpenGL expects bottom-left origin
            const glm::vec3& color = image(i, j);
            g_displayBuffer[idx + 0] = static_cast<unsigned char>(color.r * 255.0f); // R
            g_displayBuffer[idx + 1] = static_cast<unsigned char>(color.g * 255.0f); // G
            g_displayBuffer[idx + 2] = static_cast<unsigned char>(color.b * 255.0f); // B
        }
    }
    
    // Initialize GLUT
    int argc = 1;
    char* argv[1] = {(char*)"raycast"};
    glutInit(&argc, argv);
    
    // Use simpler display mode for better compatibility
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(width, height);
    glutCreateWindow("Raycast - Ray Tracing Result");
    
    // Set up OpenGL state
    glClearColor(0.0, 0.0, 0.0, 1.0);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0, width, 0, height);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    
    // Register callbacks
    glutDisplayFunc(display);
    glutKeyboardFunc(keyboard);
    
    std::cout << "Displaying ray tracing result. Press ESC to quit." << std::endl;
    glutMainLoop();
} 