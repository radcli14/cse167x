# version 330 core
// Do not use any version older than 330!

/* This is the fragment shader for reading in a scene description, including 
   lighting.  Uniform lights are specified from the main program, and used in 
   the shader.  As well as the material parameters of the object.  */

// Inputs to the fragment shader are the outputs of the same name of the vertex shader.
// Note that the default output, gl_Position, is inaccessible!
in vec3 mynormal; 
in vec4 myvertex; 

// You will certainly need this matrix for your lighting calculations
uniform mat4 modelview;

// This first defined output of type vec4 will be the fragment color
out vec4 fragColor;

uniform vec3 color;

const int numLights = 10; 
uniform bool enablelighting; // are we lighting at all (global).
uniform vec4 lightposn[numLights]; // positions of lights 
uniform vec4 lightcolor[numLights]; // colors of lights
uniform int numused;               // number of lights used

// Now, set the material parameters.
// I use ambient, diffuse, specular, shininess. 
// But, the ambient is just additive and doesn't multiply the lights.  

uniform vec4 ambient; 
uniform vec4 diffuse; 
uniform vec4 specular; 
uniform vec4 emission; 
uniform float shininess; 

vec4 ComputeLight (vec3 direction, vec4 lightcolor, vec3 normal, vec3 halfvec, vec4 mydiffuse, vec4 myspecular, float myshininess) {

    float nDotL = dot(normal, direction)  ;         
    vec4 lambert = mydiffuse * lightcolor * max (nDotL, 0.0) ;  

    float nDotH = dot(normal, halfvec) ; 
    vec4 phong = myspecular * lightcolor * pow (max(nDotH, 0.0), myshininess) ; 

    vec4 retval = lambert + phong ; 
    return retval ;
}       

void main (void) 
{       
    if (enablelighting) {       
        vec4 finalcolor; 

        // Get direction vector from the eye to the current vertex position, in modelview coordinate
        const vec3 eyepos = vec3(0, 0, 0);
        vec4 _vertexpos = modelview * myvertex;
        vec3 vertexpos = _vertexpos.xyz / _vertexpos.w; // Dehomogenize
        vec3 eyedirection = normalize(eyepos - vertexpos);

        // Get normal direction, in modelview coordinate
        mat4 inverse_matrix = inverse(modelview);
        mat4 inverse_transpose_matrix = transpose(inverse_matrix);
        vec4 _normal = inverse_transpose_matrix * vec4(mynormal, 0);
        vec3 normal = normalize(_normal.xyz);

        finalcolor = ambient + emission;

        // Add lights
        for (int n=0; n<numused; n++) { 
            // Get direction vector from the light to the vertex
            vec3 direction;
            if (lightposn[n].w == 0) {
                // Directional light, direction only dependent on light position, independent of vertex
                direction = normalize(lightposn[n].xyz);
            } else {
                // Point light, direction dependent on vector from light to vertex
                vec3 lightpos = lightposn[n].xyz / lightposn[n].w;
                direction = normalize(lightpos - vertexpos);
            }

            // Get halfvec
            vec3 halfvec = normalize(direction + eyedirection);

            // Use ComputeLight function to get the lighting contribution
            finalcolor += ComputeLight(direction, lightcolor[n], normal, halfvec, diffuse, specular, shininess);
        }

        fragColor = finalcolor; 
    } else {
        fragColor = vec4(color, 1.0f); 
    }
}
