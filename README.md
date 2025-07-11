# cse167x
UC San DiegoX Computer Graphics

## Lecture 1 (Course Overview)

### Graphics Pipeline

- Pipeline Definition
  - Modeling (creating 3D geometry and textures)
  - Animation
  - Rendering (create realistic images considering geometry and animation)
 
- Rasterization (homework 2)
  - Goes through all geometric primitives, determine where they exist on the screen

- Raytracing (homework 3)
  - Goes through all pixels, and determines what geometry are in that pixel

### Historical Methods

- Diffuse lighting (Gourad), smooth shading, lessens facet edges
- Specular lighting (Phong), adds highlights, plastic appearance with body color component and specular highlight
- Curved surface and texture (Blinn)
- Z-Buffer (Catmull), hidden surface
- Global illumination (Whitted), recursive raytracing, bubble refraction image
- Radiosity (Goral, Torrance), interacting and reflecting light where one object illuminates another
- Rendering equation (Kajiya), unified many of the methods and visual phenomena

## Lecture 2 (Basic Math)

### Dot Products

- "Scalar" product
```math
\mathbf{a} \cdot \mathbf{b}= \mathbf{b} \cdot {a} = x_a x_b + y_a y_b = \| \mathbf{a} \|  \| \mathbf{b} \| \cos \phi
```
- Angle $$\phi$$ between two vectors
```math
\phi = \cos^{-1} \left( \frac{\mathbf{a} \cdot \mathbf{b}}{ \| \mathbf{a} \| \| \mathbf{b} \| }  \right)
```
- Used to project one vector onto another
```math
\| \mathbf{b} \rightarrow \mathbf{a} \| = \| \mathbf{b} \| \cos \phi = \frac{ \mathbf{a} \cdot \mathbf{b} } { \| a \| }
```
