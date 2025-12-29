#pragma once
#include <GLFW/glfw3.h>

struct Vec3 { float x, y, z; };

class Camera {
public:
    Vec3 position;
    float yaw;
    float pitch;
    float speed;

    Camera(Vec3 startPos = { 0, 0, 3 }, float startYaw = -90.0f, float startPitch = 0.0f, float camSpeed = 2.5f);

    void processKeyboard(GLFWwindow* window, float deltaTime);
    Vec3 getForwardVector() const;
    void getViewMatrix(float* view) const; // fill 4x4 column-major matrix
};
