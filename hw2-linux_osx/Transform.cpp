// Transform.cpp: implementation of the Transform class.

// Note: when you construct a matrix using mat4() or mat3(), it will be COLUMN-MAJOR
// Keep this in mind in readfile.cpp and display.cpp
// See FAQ for more details or if you're having problems.

#include "Transform.h"

mat3 identity3 = mat3(1.0f); // 3x3 identity matrix

// Helper rotation function.  Please implement this.  
mat3 Transform::rotate(const float degrees, const vec3& axis) 
{
  // Preload cosine and sine functions
	float c = cos(degrees * pi / 180.0f);
	float s = sin(degrees * pi / 180.0f);

	// Outer product of the axis with itself
	mat3 outer = mat3(
		axis.x * axis.x, axis.x * axis.y, axis.x * axis.z,
		axis.y * axis.x, axis.y * axis.y, axis.y * axis.z,
		axis.z * axis.x, axis.z * axis.y, axis.z * axis.z
	);

	// Cross product matrix
	mat3 aprime = transpose(mat3(
		0.0f, -axis.z, axis.y,
		axis.z, 0.0f, -axis.x,
		-axis.y, axis.x, 0.0f
	));

	// Rodrigues Rotation Formula
	return c * identity3 + (1.0f - c) * outer + s * aprime;
}

void Transform::left(float degrees, vec3& eye, vec3& up) 
{
  // Obtain the rotation matrix, then update the eye and up vectors 
	// according to a rotation of degrees aboout the up vector
	mat3 R = rotate(degrees, up);
	eye = R * eye;
	up = R * up;
}

void Transform::up(float degrees, vec3& eye, vec3& up) 
{
	// Obtain the axis pointing to the right of the eye using a cross product of eye and up
	vec3 right = normalize(cross(eye, up));

	// Obtain the rotation matrix, then update the eye and up vectors 
	// according to a rotation of degrees aboout the right vector
	mat3 R = rotate(degrees, right);
	eye = R * eye;
	up = R * up;
}

mat4 Transform::lookAt(const vec3 &eye, const vec3 &center, const vec3 &up) 
{
	// Compute the u, v, w vectors according to the equations provided in the 
	// orthornormal coordinate frames lecture, where a is eye, b is up
	vec3 u, v, w;
	w = normalize(eye);
	u = normalize(cross(up, w));
	v = cross(w, u);

	// Form the rotation matrix using the u, v, w vector, and assume e is the eye,
	// apply these to the gluLookAt final form equation
	return transpose(mat4(
		u.x, u.y, u.z, -(u.x * eye.x + u.y * eye.y + u.z * eye.z),
		v.x, v.y, v.z, -(v.x * eye.x + v.y * eye.y + v.z * eye.z),
		w.x, w.y, w.z, -(w.x * eye.x + w.y * eye.y + w.z * eye.z),
		0.0f, 0.0f, 0.0f, 1.0f
	));
}

mat4 Transform::perspective(float fovy, float aspect, float zNear, float zFar)
{
  mat4 ret;
  // Calculate the d term that is the cotangent of the half field-of-view
  float theta = fovy / 2.0f;
  float d = 1.0f / tan(theta);

  // Calculate the A and B values for the projection of the Z-component
  float A = -(zFar + zNear) / (zFar - zNear);
  float B = -2.0f * zFar * zNear / (zFar - zNear);

  // Build the matrix
  ret[0][0] = d / aspect;
  ret[1][1] = d;
  ret[2][2] = A;
  ret[2][3] = B;
  ret[3][2] = -1.0f;
  ret[3][3] = 0.0f;
  
  return ret;
}

mat4 Transform::scale(const float &sx, const float &sy, const float &sz) 
{
  // Initialize the returned matrix as an identity
  mat4 ret = mat4(1.0f);

  // Add the scale vector in the fourth row
  ret[3][0] = sx;
  ret[3][1] = sy;
  ret[3][2] = sz;

  return ret;
}

mat4 Transform::translate(const float &tx, const float &ty, const float &tz) 
{
  // Initialize the returned matrix as an identity
  mat4 ret = mat4(1.0f);

  // Add the translation vector in the fourth column
  ret[0][3] = tx;
  ret[1][3] = ty;
  ret[2][3] = tz;

  return ret;
}

// To normalize the up direction and construct a coordinate frame.  
// As discussed in the lecture.  May be relevant to create a properly 
// orthogonal and normalized up. 
// This function is provided as a helper, in case you want to use it. 
// Using this function (in readfile.cpp or display.cpp) is optional.  

vec3 Transform::upvector(const vec3 &up, const vec3 & zvec) 
{
  vec3 x = glm::cross(up,zvec); 
  vec3 y = glm::cross(zvec,x); 
  vec3 ret = glm::normalize(y); 
  return ret; 
}


Transform::Transform()
{

}

Transform::~Transform()
{

}
