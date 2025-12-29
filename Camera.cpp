#include "Camera.h"
#include <cmath>

// ----------------------------
// Helper: cross product
Vec3 cross(const Vec3& a, const Vec3& b) {
    return {
        a.y * b.z - a.z * b.y,
        a.z * b.x - a.x * b.z,
        a.x * b.y - a.y * b.x
    };
}

// ----------------------------
Camera::Camera(Vec3 startPos, float startYaw, float startPitch, float camSpeed) {
    position = startPos;
    yaw = startYaw;
    pitch = startPitch;
    speed = camSpeed;
}

// ----------------------------
Vec3 Camera::getForwardVector() const {
    float radYaw = yaw * (3.14159265f / 180.0f);
    float radPitch = pitch * (3.14159265f / 180.0f);
    return {
        cos(radYaw) * cos(radPitch),
        sin(radPitch),
        sin(radYaw) * cos(radPitch)
    };
}

// ----------------------------
void Camera::processKeyboard(GLFWwindow* window, float deltaTime) {
    Vec3 forward = getForwardVector();
    Vec3 up = { 0,1,0 };
    Vec3 right = cross(forward, up);

    float velocity = speed * deltaTime;

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
        position.x += forward.x * velocity;
        position.y += forward.y * velocity;
        position.z += forward.z * velocity;
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        position.x -= forward.x * velocity;
        position.y -= forward.y * velocity;
        position.z -= forward.z * velocity;
    }
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
        position.x -= right.x * velocity;
        position.y -= right.y * velocity;
        position.z -= right.z * velocity;
    }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
        position.x += right.x * velocity;
        position.y += right.y * velocity;
        position.z += right.z * velocity;
    }
}

// ----------------------------
void Camera::getViewMatrix(float* view) const {
    Vec3 forward = getForwardVector();
    Vec3 center = { position.x + forward.x, position.y + forward.y, position.z + forward.z };
    Vec3 up = { 0,1,0 };

    Vec3 f = { forward.x, forward.y, forward.z };
    float lenF = sqrt(f.x * f.x + f.y * f.y + f.z * f.z);
    f.x /= lenF; f.y /= lenF; f.z /= lenF;

    Vec3 s = cross(f, up);
    float lenS = sqrt(s.x * s.x + s.y * s.y + s.z * s.z);
    s.x /= lenS; s.y /= lenS; s.z /= lenS;

    Vec3 u = cross(s, f);

    // column-major
    view[0] = s.x; view[4] = s.y; view[8] = s.z; view[12] = -(s.x * position.x + s.y * position.y + s.z * position.z);
    view[1] = u.x; view[5] = u.y; view[9] = u.z; view[13] = -(u.x * position.x + u.y * position.y + u.z * position.z);
    view[2] = -f.x; view[6] = -f.y; view[10] = -f.z; view[14] = f.x * position.x + f.y * position.y + f.z * position.z;
    view[3] = 0; view[7] = 0; view[11] = 0; view[15] = 1;
}
