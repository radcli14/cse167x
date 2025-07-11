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

### Cross Products

- "Vector" product, orthogonal to the two initial vectors
```math
\mathbf{a} \times \mathbf{b} = -\mathbf{b} \times \mathbf{a}
```

- Used to calculate sine of the angle between vectors
```math
\| \mathbf{a} \times \mathbf{b} \| = \| \mathbf{a} \| \| \mathbf{b} \| \sin \phi
```

- Represented using the determinant $$| \cdot |$$
```math
\mathbf{a} \times \mathbf{b} =
\begin{vmatrix}
\mathbf{x} & \mathbf{y} & \mathbf{z} \\
x_a & y_a & z_a \\
x_b & y_b & z_b 
\end{vmatrix} =
\begin{pmatrix}
y_a z_b - y_b z_a \\
z_a x_b - z_b x_a \\
x_a y_b - x_b y_a
\end{pmatrix}
```

- Can be calculated using the skew-symmetric "dual" matrix of $$\mathbf{a}$$, referred as $$\mathbf{A}^*$$
```math
\mathbf{a} \times \mathbf{b} = \mathbf{A}^* \mathbf{b} =
\begin{bmatrix}
0 & -z_a & y_a \\
z_a & 0 & -x_a \\
-y_a & x_a & 0 
\end{bmatrix}
\begin{pmatrix}
x_b \\ y_b \\ z_b
\end{pmatrix}
```

### Orthonormal Coordinate Frames
- Global, local, world, model, parts of model
- Constructing a coordinate frame (u,v,w)
```math
\mathbf{w} = \frac{ \mathbf{a} }{ \| \mathbf{a} \| }
```
```math
\mathbf{u} = \frac{ \mathbf{b} \times \mathbf{w} }{ \| \mathbf{b} \times \mathbf{w}  \| }
```
```math
\mathbf{v} = \mathbf{w} \times \mathbf{u}
```
- This is often used where we are associated $$\mathbf{a}$$ with a viewing direction, and we are using a second vector $$\mathbf{b}$$ which is an up vector

### Matrices
- In matrix multiplication, element $$(i, j)$$ is the dot product of the vector defined by row $$i$$ of the first matrix with column $$j$$ of the second matrix
- Not commutative, $$\mathbf{AB} \neq \mathbf{BA}$$ in general, can be dimensionally incompatible
