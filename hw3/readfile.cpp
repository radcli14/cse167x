/*****************************************************************************/
/* This is the program skeleton for homework 3 in CSE167x by Ravi Ramamoorthi */
/* Extends HW 2 to deal with ray casting and rendering                        */
/*****************************************************************************/

// Basic includes to get this file to work.  
#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <deque>
#include <stack>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "structures.h"

using namespace std;

// Global variables for scene data
extern Camera camera;
extern vector<Light> lights;
extern vector<Sphere> spheres;
extern vector<Triangle> triangles;
extern vector<Vertex> vertices;
extern vector<Vertex> verticesWithNormals;
extern Material currentMaterial;
extern int maxDepth;
extern string outputFilename;
extern int imageWidth, imageHeight;

// Function to read the input data values
bool readvals(stringstream &s, const int numvals, float* values) 
{
  for (int i = 0; i < numvals; i++) {
    s >> values[i]; 
    if (s.fail()) {
      cout << "Failed reading value " << i << " will skip\n"; 
      return false;
    }
  }
  return true; 
}

void readfile(const char* filename) 
{
  string str, cmd; 
  ifstream in;
  in.open(filename); 
  if (in.is_open()) {

    // Matrix stack to store transforms
    stack <glm::mat4> transfstack; 
    transfstack.push(glm::mat4(1.0));  // identity

    // Initialize defaults
    maxDepth = 5;
    outputFilename = "raytrace.png";
    imageWidth = 640;
    imageHeight = 480;

    getline (in, str); 
    while (in) {
      if ((str.find_first_not_of(" \t\r\n") != string::npos) && (str[0] != '#')) {
        // Ruled out comment and blank lines 

        stringstream s(str);
        s >> cmd; 
        int i; 
        float values[10]; // Up to 10 params for various commands
        bool validinput; // Validity of input 

        // General commands
        if (cmd == "size") {
          validinput = readvals(s, 2, values); 
          if (validinput) { 
            imageWidth = (int) values[0]; 
            imageHeight = (int) values[1];
            cout << "Parsed SIZE: " << imageWidth << " x " << imageHeight << endl;
          } 
        } else if (cmd == "maxdepth") {
          validinput = readvals(s, 1, values); 
          if (validinput) { 
            maxDepth = (int) values[0];
            cout << "Parsed MAXDEPTH: " << maxDepth << endl;
          } 
        } else if (cmd == "output") {
          s >> outputFilename;
          cout << "Parsed OUTPUT: " << outputFilename << endl;
        }

        // Camera command
        else if (cmd == "camera") {
          validinput = readvals(s, 10, values); // lookfrom, lookat, up, fov
          if (validinput) {
            camera.lookfrom = glm::vec3(values[0], values[1], values[2]);
            camera.lookat = glm::vec3(values[3], values[4], values[5]);
            camera.up = glm::normalize(glm::vec3(values[6], values[7], values[8]));
            camera.fov = values[9];
            cout << "Parsed CAMERA: lookfrom(" << values[0] << "," << values[1] << "," << values[2] 
                 << ") lookat(" << values[3] << "," << values[4] << "," << values[5] 
                 << ") up(" << values[6] << "," << values[7] << "," << values[8] 
                 << ") fov(" << values[9] << ")" << endl;
          }
        }

        // Geometry commands
        else if (cmd == "sphere") {
          validinput = readvals(s, 4, values); // x, y, z, radius
          if (validinput) {
            Sphere sphere;
            sphere.center = glm::vec3(values[0], values[1], values[2]);
            sphere.radius = values[3];
            sphere.transform = transfstack.top();
            sphere.material = currentMaterial;
            spheres.push_back(sphere);
            cout << "Parsed SPHERE: pos(" << values[0] << "," << values[1] << "," << values[2] 
                 << ") radius(" << values[3] << ")" << endl;
          }
        } else if (cmd == "maxverts") {
          validinput = readvals(s, 1, values); 
          if (validinput) { 
            int maxverts = (int) values[0];
            vertices.reserve(maxverts);
            cout << "Parsed MAXVERTS: " << maxverts << endl;
          } 
        } else if (cmd == "maxvertnorms") {
          validinput = readvals(s, 1, values); 
          if (validinput) { 
            int maxvertnorms = (int) values[0];
            verticesWithNormals.reserve(maxvertnorms);
            cout << "Parsed MAXVERTNORMS: " << maxvertnorms << endl;
          } 
        } else if (cmd == "vertex") {
          validinput = readvals(s, 3, values); // x, y, z
          if (validinput) {
            Vertex vertex(glm::vec3(values[0], values[1], values[2]));
            vertices.push_back(vertex);
            cout << "Parsed VERTEX: (" << values[0] << "," << values[1] << "," << values[2] << ")" << endl;
          }
        } else if (cmd == "vertexnormal") {
          validinput = readvals(s, 6, values); // x, y, z, nx, ny, nz
          if (validinput) {
            Vertex vertex(glm::vec3(values[0], values[1], values[2]), 
                         glm::vec3(values[3], values[4], values[5]));
            verticesWithNormals.push_back(vertex);
            cout << "Parsed VERTEXNORMAL: pos(" << values[0] << "," << values[1] << "," << values[2] 
                 << ") normal(" << values[3] << "," << values[4] << "," << values[5] << ")" << endl;
          }
        } else if (cmd == "tri") {
          validinput = readvals(s, 3, values); // v1, v2, v3 indices
          if (validinput) {
            Triangle triangle;
            triangle.v1 = (int)values[0];
            triangle.v2 = (int)values[1];
            triangle.v3 = (int)values[2];
            triangle.transform = transfstack.top();
            triangle.material = currentMaterial;
            triangles.push_back(triangle);
            cout << "Parsed TRI: vertices(" << (int)values[0] << "," << (int)values[1] << "," << (int)values[2] << ")" << endl;
          }
        } else if (cmd == "trinormal") {
          validinput = readvals(s, 3, values); // v1, v2, v3 indices
          if (validinput) {
            Triangle triangle;
            triangle.v1 = (int)values[0];
            triangle.v2 = (int)values[1];
            triangle.v3 = (int)values[2];
            triangle.transform = transfstack.top();
            triangle.material = currentMaterial;
            triangles.push_back(triangle);
            cout << "Parsed TRINORMAL: vertices(" << (int)values[0] << "," << (int)values[1] << "," << (int)values[2] << ")" << endl;
          }
        }

        // Transform commands
        else if (cmd == "translate") {
          validinput = readvals(s, 3, values); 
          if (validinput) {
            glm::mat4 translate = glm::translate(glm::mat4(1.0f), glm::vec3(values[0], values[1], values[2]));
            transfstack.top() = transfstack.top() * translate;
            cout << "Parsed TRANSLATE: (" << values[0] << "," << values[1] << "," << values[2] << ")" << endl;
          }
        } else if (cmd == "rotate") {
          validinput = readvals(s, 4, values); // x, y, z axis, angle
          if (validinput) {
            glm::vec3 axis = glm::normalize(glm::vec3(values[0], values[1], values[2]));
            glm::mat4 rotate = glm::rotate(glm::mat4(1.0f), glm::radians(values[3]), axis);
            transfstack.top() = transfstack.top() * rotate;
            cout << "Parsed ROTATE: axis(" << values[0] << "," << values[1] << "," << values[2] 
                 << ") angle(" << values[3] << ")" << endl;
          }
        } else if (cmd == "scale") {
          validinput = readvals(s, 3, values); 
          if (validinput) {
            glm::mat4 scale = glm::scale(glm::mat4(1.0f), glm::vec3(values[0], values[1], values[2]));
            transfstack.top() = transfstack.top() * scale;
            cout << "Parsed SCALE: (" << values[0] << "," << values[1] << "," << values[2] << ")" << endl;
          }
        } else if (cmd == "pushTransform") {
          transfstack.push(transfstack.top()); 
          cout << "Parsed PUSHTRANSFORM" << endl;
        } else if (cmd == "popTransform") {
          if (transfstack.size() <= 1) {
            cerr << "Stack has no elements. Cannot Pop\n"; 
          } else {
            transfstack.pop(); 
            cout << "Parsed POPTRANSFORM" << endl;
          }
        }

        // Light commands
        else if (cmd == "directional") {
          validinput = readvals(s, 6, values); // x, y, z direction, r, g, b color
          if (validinput) {
            Light light;
            light.direction = glm::normalize(glm::vec3(values[0], values[1], values[2]));
            light.color = glm::vec3(values[3], values[4], values[5]);
            light.isDirectional = true;
            lights.push_back(light);
            cout << "Parsed DIRECTIONAL: direction(" << values[0] << "," << values[1] << "," << values[2] 
                 << ") color(" << values[3] << "," << values[4] << "," << values[5] << ")" << endl;
          }
        } else if (cmd == "point") {
          validinput = readvals(s, 6, values); // x, y, z position, r, g, b color
          if (validinput) {
            Light light;
            light.position = glm::vec3(values[0], values[1], values[2]);
            light.color = glm::vec3(values[3], values[4], values[5]);
            light.isDirectional = false;
            lights.push_back(light);
            cout << "Parsed POINT: position(" << values[0] << "," << values[1] << "," << values[2] 
                 << ") color(" << values[3] << "," << values[4] << "," << values[5] << ")" << endl;
          }
        } else if (cmd == "attenuation") {
          validinput = readvals(s, 3, values); // const, linear, quadratic
          if (validinput) {
            // Set attenuation for the last added light
            if (!lights.empty()) {
              lights.back().attenuation = glm::vec3(values[0], values[1], values[2]);
            }
            cout << "Parsed ATTENUATION: const(" << values[0] << ") linear(" << values[1] << ") quadratic(" << values[2] << ")" << endl;
          }
        } else if (cmd == "ambient") {
          validinput = readvals(s, 3, values); // r, g, b (no alpha)
          if (validinput) {
            currentMaterial.ambient = glm::vec3(values[0], values[1], values[2]);
            cout << "Parsed AMBIENT: (" << values[0] << "," << values[1] << "," << values[2] << ")" << endl;
          }
        }

        // Material commands
        else if (cmd == "diffuse") {
          validinput = readvals(s, 3, values); // r, g, b (no alpha)
          if (validinput) {
            currentMaterial.diffuse = glm::vec3(values[0], values[1], values[2]);
            cout << "Parsed DIFFUSE: (" << values[0] << "," << values[1] << "," << values[2] << ")" << endl;
          }
        } else if (cmd == "specular") {
          validinput = readvals(s, 3, values); // r, g, b (no alpha)
          if (validinput) {
            currentMaterial.specular = glm::vec3(values[0], values[1], values[2]);
            cout << "Parsed SPECULAR: (" << values[0] << "," << values[1] << "," << values[2] << ")" << endl;
          }
        } else if (cmd == "shininess") {
          validinput = readvals(s, 1, values); 
          if (validinput) {
            currentMaterial.shininess = values[0];
            cout << "Parsed SHININESS: " << values[0] << endl;
          }
        } else if (cmd == "emission") {
          validinput = readvals(s, 3, values); // r, g, b (no alpha)
          if (validinput) {
            currentMaterial.emission = glm::vec3(values[0], values[1], values[2]);
            cout << "Parsed EMISSION: (" << values[0] << "," << values[1] << "," << values[2] << ")" << endl;
          }
        }

        else {
          cerr << "Unknown Command: " << cmd << " Skipping \n"; 
        }
      }
      getline (in, str); 
    }


    
    cout << "Finished parsing scene file: " << filename << endl;
    cout << "Image size: " << imageWidth << "x" << imageHeight << endl;
    cout << "Max depth: " << maxDepth << endl;
    cout << "Output file: " << outputFilename << endl;
    cout << "Number of spheres: " << spheres.size() << endl;
    cout << "Number of triangles: " << triangles.size() << endl;
    cout << "Number of lights: " << lights.size() << endl;
    cout << "Number of vertices: " << vertices.size() << endl;

  } else {
    cerr << "Unable to Open Input Data File " << filename << "\n"; 
    throw 2; 
  }
} 