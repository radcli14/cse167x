#include "raytrace.h"
#include "variables.h"
#include <iostream>
#include <cmath>
#include <FreeImage.h>
#include <GLUT/glut.h>

// Constants
const float EPSILON = 0.0001f; // Small value to avoid self-intersection

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

    // Horizontal alpha and vertical beta from canonical viewing slide, adding 0.5f to j and i to center the pixel
    float alpha = aspect * tan(0.5f * fovyRad) * (j + 0.5f - 0.5f * width) / (0.5f * width);
    float beta = tan(0.5f * fovyRad) * (0.5f * height - i - 0.5f) / (0.5f * height);

    // Ray direction in world space
    glm::vec3 dir = glm::normalize(alpha * cam.u + beta * cam.v - cam.w);
    return Ray(cam.lookfrom, dir);
}

Intersection Intersect(const Ray& ray, const Scene& scene) {
    Intersection hit;
    hit.ray = ray;  // Store the ray for access to camera/view direction
    float closest_t = INFINITY;

    // Loop over all spheres
    for (size_t i = 0; i < scene.spheres.size(); ++i) {
        const Sphere& sphere = scene.spheres[i];
        
        // Transform ray to sphere's local coordinate system
        glm::mat4 invTransform = glm::inverse(sphere.transform);
        glm::vec3 localOrigin = glm::vec3(invTransform * glm::vec4(ray.origin, 1.0f));
        glm::vec3 localDirection = glm::vec3(invTransform * glm::vec4(ray.direction, 0.0f));
        
        // Ray-sphere intersection using quadratic formula in local coordinates
        // Ray: P(t) = localOrigin + t * localDirection
        // Sphere: |P - sphere.center|^2 = sphere.radius^2
        // Substitute ray equation into sphere equation and solve for t
        
        glm::vec3 oc = localOrigin - sphere.center;
        float a = glm::dot(localDirection, localDirection);
        float b = 2.0f * glm::dot(oc, localDirection);
        float c = glm::dot(oc, oc) - sphere.radius * sphere.radius;
        
        float discriminant = b * b - 4 * a * c;
        
        if (discriminant > 0) {
            // Two intersection points
            float t1 = (-b - sqrt(discriminant)) / (2.0f * a);
            float t2 = (-b + sqrt(discriminant)) / (2.0f * a);
            
            // Use the closer intersection point (smaller t value)
            float t = (t1 > 0.0f) ? t1 : t2;
            
            if (t > EPSILON && t < closest_t) {
                closest_t = t;
                hit.hit = true;
                hit.t = t;
                
                // Transform intersection point back to world coordinates
                glm::vec3 localPoint = localOrigin + t * localDirection;
                hit.point = glm::vec3(sphere.transform * glm::vec4(localPoint, 1.0f));
                
                // Transform normal back to world coordinates (transpose of inverse)
                glm::vec3 localNormal = glm::normalize(localPoint - sphere.center);
                glm::mat3 normalMatrix = glm::transpose(glm::inverse(glm::mat3(sphere.transform)));
                hit.normal = glm::normalize(normalMatrix * localNormal);
                
                hit.material = sphere.material;
            }
        }
    }
    
    // Loop over all triangles
    for (size_t i = 0; i < scene.triangles.size(); ++i) {
        const Triangle& tri = scene.triangles[i];
        
        // Transform ray to triangle's local coordinate system
        glm::mat4 invTransform = glm::inverse(tri.transform);
        glm::vec3 localOrigin = glm::vec3(invTransform * glm::vec4(ray.origin, 1.0f));
        glm::vec3 localDirection = glm::vec3(invTransform * glm::vec4(ray.direction, 0.0f));
        
        // Get triangle vertices in local coordinates
        extern std::vector<Vertex> vertices;
        const glm::vec3& v0 = vertices[tri.v1].position;
        const glm::vec3& v1 = vertices[tri.v2].position;
        const glm::vec3& v2 = vertices[tri.v3].position;

        // Compute plane normal in local coordinates
        glm::vec3 n = glm::normalize(glm::cross(v1 - v0, v2 - v0));
        float denom = glm::dot(n, localDirection);
        if (fabs(denom) < 1e-6f) continue; // Parallel, no intersection

        float t = glm::dot(n, v0 - localOrigin) / denom;
        if (t < EPSILON) continue; // Intersection behind ray origin
        if (t > closest_t) continue; // Intersection is behind the closest intersection

        glm::vec3 localPoint = localOrigin + t * localDirection;

        // Barycentric coordinates
        glm::vec3 v0v1 = v1 - v0;
        glm::vec3 v0v2 = v2 - v0;
        glm::vec3 v0p = localPoint - v0;
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
            
            // Transform intersection point back to world coordinates
            hit.point = glm::vec3(tri.transform * glm::vec4(localPoint, 1.0f));
            
            // Transform normal back to world coordinates (transpose of inverse)
            glm::mat3 normalMatrix = glm::transpose(glm::inverse(glm::mat3(tri.transform)));
            hit.normal = glm::normalize(normalMatrix * n);
            
            hit.material = tri.material;
        }
    }
    return hit;
}

glm::vec3 ComputeLight(glm::vec3 direction, glm::vec3 lightcolor, glm::vec3 normal, glm::vec3 halfvec, glm::vec3 mydiffuse, glm::vec3 myspecular, float myshininess) {

    float nDotL = glm::dot(normal, direction)  ;         
    glm::vec3 lambert = mydiffuse * lightcolor * glm::max (nDotL, 0.0f) ;  

    float nDotH = glm::dot(normal, halfvec) ; 
    glm::vec3 phong = myspecular * lightcolor * pow (glm::max(nDotH, 0.0f), myshininess) ; 

    glm::vec3 retval = lambert + phong ; 
    return retval ;
}     

glm::vec3 FindColor(const Intersection& hit, const Scene& scene, int depth) {
    if (hit.hit) {
        
        // Start with ambient and emission
        glm::vec3 ambient = hit.material.ambient;
        glm::vec3 emissive = hit.material.emission;
        glm::vec3 direct;
        glm::vec3 reflection;

        // Calculate view direction using the ray direction from the intersection
        // The ray direction points from camera to hit point, so we negate it for view direction
        glm::vec3 viewDir = -hit.ray.direction; // View direction is opposite of ray direction
            
        // Ensure normal is normalized
        glm::vec3 normal = glm::normalize(hit.normal);
        float NdotV = glm::dot(normal, viewDir);

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
            glm::vec3 shadowRayOrigin = hit.point + EPSILON * normal;
            Ray lightRay = Ray(shadowRayOrigin, lightDir);
            Intersection lightHit = Intersect(lightRay, scene);
            
            // Check if shadow ray hits something closer than the light
            if (lightHit.hit && lightHit.t < distanceToLight) {
                continue; // Light is blocked by an object
            }
            
            // Calculate half vector for specular calculations
            // Half vector is halfway between view direction and light direction
            glm::vec3 halfVector = glm::normalize(viewDir + lightDir);
            
            // Calculate diffuse and specular lighting from the direct source
            glm::vec3 lightContribution = ComputeLight(lightDir, light.color, hit.normal, halfVector, hit.material.diffuse, hit.material.specular, hit.material.shininess);
            direct += attenuation * lightContribution;
        }
        
        // Recursive reflection ray tracing
        if (depth < maxDepth) {
            // Only reflect if we're not grazing the surface
            if (NdotV > 0.0f) {
                glm::vec3 reflectionDir = -(viewDir - 2.0f * NdotV * normal);
                reflectionDir = glm::normalize(reflectionDir);
                
                // Create reflection ray with offset to avoid self-intersection
                glm::vec3 reflectionRayOrigin = hit.point + EPSILON * normal;
                Ray reflectionRay = Ray(reflectionRayOrigin, reflectionDir);
                
                // Trace the reflection ray recursively
                Intersection reflectionHit = Intersect(reflectionRay, scene);
                
                // Add reflection if it hit something
                if (reflectionHit.hit && reflectionHit.t > EPSILON) {
                    glm::vec3 reflectionColor = FindColor(reflectionHit, scene, depth + 1);
                    
                    // Calculate half vector for the reflection (between view direction and reflection direction)
                    glm::vec3 reflectionHalfVector = glm::normalize(viewDir + reflectionDir);
                    
                    // Treat the reflection as a light source using ComputeLight
                    // reflectionDir is the "light direction", reflectionColor is the "light color"
                    // For reflections, we typically only want the specular component, so set the diffuse to 0.
                    reflection = ComputeLight(reflectionDir, reflectionColor, normal, reflectionHalfVector, 
                                            glm::vec3(0.0f, 0.0f, 0.0f), hit.material.specular, hit.material.shininess);
                }
            }
        }
        
        // Return the computed color
        glm::vec3 color = ambient + emissive + direct + reflection;
        
        // Clamp color values to [0, 1] range to prevent overflow
        color.r = glm::clamp(color.r, 0.0f, 1.0f);
        color.g = glm::clamp(color.g, 0.0f, 1.0f);
        color.b = glm::clamp(color.b, 0.0f, 1.0f);
        
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
            image(i, j) = FindColor(hit, scene, 0); // Start recursion with depth 0
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