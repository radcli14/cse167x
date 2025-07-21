// Transform.cpp: implementation of the Transform class.


#include "Transform.h"

//Please implement the following functions:
mat3 identity3 = mat3(1.0f); // 3x3 identity matrix

// Helper rotation function.  
mat3 Transform::rotate(const float degrees, const vec3& axis) {
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

// Transforms the camera left around the "crystal ball" interface
void Transform::left(float degrees, vec3& eye, vec3& up) {
	// Obtain the rotation matrix, then update the eye and up vectors 
	// according to a rotation of -degrees aboout the up vector
	mat3 R = rotate(-degrees, up);
	eye = R * eye;
	up = R * up;
}

// Transforms the camera up around the "crystal ball" interface
void Transform::up(float degrees, vec3& eye, vec3& up) {
	// Obtain the axis pointing to the right of the eye using a cross product of eye and up
	vec3 right = normalize(cross(eye, up));

	// Obtain the rotation matrix, then update the eye and up vectors 
	// according to a rotation of degrees aboout the right vector
	mat3 R = rotate(degrees, right);
	eye = R * eye;
	up = R * up;
}

// Your implementation of the glm::lookAt matrix
mat4 Transform::lookAt(vec3 eye, vec3 up) {
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

Transform::Transform()
{

}

Transform::~Transform()
{

}
