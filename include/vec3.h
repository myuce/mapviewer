#include <cmath>
#include <cstdio>

class vec3 {
    public:
    float x, y, z;

    vec3(float a, float b, float c) {
        x = a;
        y = b;
        z = c;
    }

    vec3() {
        x = 0;
        y = 0;
        z = 0;
    }

    vec3 operator+(vec3 rhs) {
        return vec3(x + rhs.x, y + rhs.y, z + rhs.z);
    }

    vec3 operator+(float rhs) {
        return vec3(x + rhs, y + rhs, z + rhs);
    }

    vec3 operator-(vec3 rhs) {
        return vec3(x - rhs.x, y - rhs.y, z - rhs.z);
    }

    vec3 operator-(float rhs) {
        return vec3(x - rhs, y - rhs, z - rhs);
    }

    vec3 operator*(vec3 rhs) {
        return vec3(x * rhs.x, y * rhs.y, z * rhs.z);
    }

    vec3 operator*(float rhs) {
        return vec3(x * rhs, y * rhs, z * rhs);
    }

    vec3 operator/(vec3 rhs) {
        return vec3(x / rhs.x, y / rhs.y, z / rhs.z);
    }

    vec3 operator/(float rhs) {
        return vec3(x / rhs, y / rhs, z / rhs);
    }

    bool operator==(vec3 rhs) {
        return (vec3(x, y , z) - rhs).len() <= 0.01;
    }

    vec3 abs() {
        return vec3(fabs(x), fabs(y), fabs(z));
    }

    float dot(vec3 rhs) {
        return (x * rhs.x) + (y * rhs.y) + (z * rhs.z);
    }

    float sqrLen() {
        return dot(vec3(x, y, z));
    }

    float len() {
        return sqrt(sqrLen());
    }

    vec3 normalize() {
        return vec3(x, y, z) / len();
    }

    vec3 cross(vec3 rhs) {
        return vec3(
            y * rhs.z - z * rhs.y,
            z * rhs.x - x * rhs.z,
            x * rhs.y - y * rhs.x
        );
    }

    float distance(vec3 rhs) {
        return sqrtf(powf(x - rhs.x, 2) + powf(y - rhs.y, 2) + powf(z - rhs.z, 2));
    }

    vec3 lerp(vec3 rhs, int alpha) {
        return vec3(
            x + ((rhs.x - x) * alpha),
            y + ((rhs.y - y) * alpha),
            z + ((rhs.z - z) * alpha)
        );
    }
};
