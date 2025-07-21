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

### Homework 0
- First homework is almost entirely to set up the system to compile a demo C++ app that calls on dependencies that will be used in subsequent homework projects
- The FAQ in the course indicates it is *required* to run this demo using Rosetta on Apple Silicon; this is *not* true
- Review of the discussion board, particularly [this post](https://discussions.edx.org/course-v1:UCSanDiegoX+CSE167x+2T2018/posts/61ddb33b6100fc048488dc3f) provides explanation of how to configure your system on an M1 or more recent chip
- Brew should be installed, though the default, ARM-native version will work, counter to what the FAQ says
- Install the dependencies using:
```console
brew install glew freeglut mesa-glu freeimage  # brew installed in /opt/homebrew
```
- [This commit](https://github.com/radcli14/cse167x/commit/25a193ef776406de68e326c7dc18a79142daa292) shows changes to the Makefile which are required to make sure the dependencies get linked properly, particularly the change of the source paths in the `INCFLAGS` and `LDFLAGS`
- The same commit shows the change in the C++ file to change the highlight color, resulting renders look as follows:

| Screenshot 1                                   | Screenshot 2                                   |
|------------------------------------------------|------------------------------------------------|
| ![Screenshot 1](hw0-linux_osx/screenshot1.png) | ![Screenshot 2](hw0-linux_osx/screenshot2.png) |

## Lecture 3 (Transforms 1)

### Basic 2D Transforms
- Scaling transformation
```math
\begin{bmatrix}
s_x && 0 \\ 0 && s_y 
\end{bmatrix}
\begin{bmatrix}
x \\ y
\end{bmatrix} =
\begin{bmatrix} s_x x \\ s_y y \end{bmatrix}
```
- Shear transform, turns a rectangular into a parallelogram
```math
\begin{bmatrix}
1 && a \\ 0 && 1
\end{bmatrix}
\begin{bmatrix}
x \\ y
\end{bmatrix} =
\begin{bmatrix}
x + a y \\ y
\end{bmatrix}
```

### Composing Transforms
- Not commutative, order matters when combining rotations and scales
- Rotation prior to a scale has similar effect to a scale
- Last transformation applied needs to be the first one undone in an inverse operation

### 3D Rotations
- 2D case of rotation
```math
\begin{bmatrix} x^\prime \\ y^\prime \end{bmatrix} =
\begin{bmatrix}
\cos \theta && -\sin \theta \\
\sin \theta && \cos \theta
\end{bmatrix}
\begin{bmatrix} x \\ y \end{bmatrix} 
```
- 3D case, rows of matrix are unit vectors in a new coordinate frame, form a projection of a point into new coordinate frame
- Inverse or transpose of the rotation matrix transpose project `uvw` vector back into the original `xyz` frame
- Not commutative, a rotation by $$x$$ before $$y$$ is not the same as a rotation by $$y$$ before $$x$$
- Angle axis rotations, rotate vector $$b$$ by an angle $$\theta$$ about vector $$a$$
  - Vector $$\mathbf{b}$$ may be considered to have a component $$\mathbf{b}_\parallel$$ parallel to $$a$$ and a component orthogonal  
  - $$\mathbf{b}_\parallel = (\mathbf{a} \cdot \mathbf{b}) \mathbf{a}$$
  - $$\mathbf{b}_\perp = \mathbf{b} - \mathbf{b}_\parallel$$
  - Vector $$\mathbf{c}$$ is parallel to both $$\mathbf{a}$$ and $$\mathbf{b}$$
  - <img width="612" height="516" alt="image" src="https://github.com/user-attachments/assets/2e8429d6-bc97-4e92-92b8-ee2d0149df8a" />
  - "Rodrigues Rotation Formula"
  - <img width="608" height="521" alt="image" src="https://github.com/user-attachments/assets/b97f6c0a-0e78-4468-b471-570eaf0e43c1" />

## Lecture 4 (Transforms 2)

### Homogeneous Coordinates
- Add a fourth coordinate to the xyz vector, call it "w", can use "w=1"
- Translation in matrix format, example, translate by 5 in x
```math
\begin{bmatrix} x+5 \\ y \\ z \\ 1 \end{bmatrix} =
\begin{bmatrix}
1 & 0 & 0 & 5 \\
0 & 1 & 0 & 0 \\
0 & 0 & 1 & 0 \\
0 & 0 & 0 & 1
\end{bmatrix}
\begin{bmatrix} x \\ y \\ z \\ 1 \end{bmatrix} =
\begin{bmatrix} x \\ y \\ z \\ w \end{bmatrix}
```
- The "unhomogenous" coordinates are the case where $$w \neq 1$$, get "homogeneous" by dividing by w
- Modifying w can provide additional operations
- Homogeneous provide a unified framework of a transform matrix for translation, viewing, rotation, and can concatenate homogenous coordinates
- General translation matrix
```math
\mathbf{T} =
\begin{bmatrix}
1 & 0 & 0 & T_x \\
0 & 1 & 0 & T_y \\
0 & 0 & 1 & T_z \\
0 & 0 & 0 & 1
\end{bmatrix} =
\begin{bmatrix}
\mathbf{I}_3 & \boldsymbol{T} \\
0 & 1
\end{bmatrix}
```
- In the above, the translation is in the reference coordinates, applied first, then the rotation

### Finding Normal Transformations
- Important in lighting, intensity on a surface depends on angle between the light and the surface, highlights depend on the angle between the viewer, surface normal, and light source
- Normal transforms do not follow point transforms (tangents do follow point transforms, as they are points)
- Normals are orthogonal to the tangents, so dot product with tangents equals zero
- Normal transform matrix is inverse transpose of the tangent transform matrix, only applies to the upper 3x3, not to the homogeneous coordinate

### Rotations and Coordinate Frames
- Defines both an origin position and an orientation
- Rows of a rotation matrix are three unit vectors that make up the frame

### Derivation of gluLookAt
- Derives a 4x4 matrix to view objects and position a camera in the world
- `gluLookAt(eyex, eyey, eyez, centerx, centery, centerz, upx, upy, upz`
- Camera is eye, looking at center, with up vector
- Creates a coordinate frame, defines a rotation, and applies a transformation
- Coordinate frame calculated according to "orthornormal coordinate frame" section earlier, $$a$$ vector is eye-center, $$b$$ vector is up
- In OpenGL convention, camera always looks in the -Z direction
- Rows of rotation matrix are the the new unit vectors constructed above
- Translation is applied first to bring the camera to its new origin before rotating it
- <img width="611" height="351" alt="image" src="https://github.com/user-attachments/assets/318bca5c-1bd4-4469-bd3e-63091ce00220" />




