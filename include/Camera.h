#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>

// Defines several possible options for camera movement.
enum Camera_Movement {
    ORBIT_LEFT,
    ORBIT_RIGHT,
    ORBIT_UP,
    ORBIT_DOWN,
    ZOOM_IN,
    ZOOM_OUT
};

// Default camera values
const float YAW         = 90.0f;
const float PITCH       =  20.0f;
const float SPEED       =  2.5f;
const float SENSITIVITY =  0.005f;
const float MIN_ZOOM    =  5.0f;
const float MAX_ZOOM    =  50.0f;


class Camera
{
public:
    // --- New Orbit Camera Members ---
    glm::vec3 Target;      // The point the camera is looking at
    float Distance;        // The distance from the target

    // camera Attributes
    glm::vec3 Position;
    glm::vec3 Front;
    glm::vec3 Up;
    glm::vec3 Right;
    glm::vec3 WorldUp;
    // euler Angles
    float Yaw;
    float Pitch;
    // camera options
    float MovementSpeed;
    float MouseSensitivity;

    // constructor with vectors
    Camera(glm::vec3 target = glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f), float yaw = YAW, float pitch = PITCH) : Front(glm::vec3(0.0f, 0.0f, -1.0f)), MovementSpeed(SPEED), MouseSensitivity(SENSITIVITY)
    {
        Target = target;
        Distance = 15.0f; // Start 15 units away from the target
        WorldUp = up;
        Yaw = yaw;
        Pitch = pitch;
        updateCameraVectors();
    }

    // returns the view matrix calculated using Euler Angles and the LookAt Matrix
    glm::mat4 GetViewMatrix()
    {
        // --- UPDATED: Recalculate position every frame ---
        updateCameraVectors();
        return glm::lookAt(Position, Target, Up);
    }

    // Processes input received from the mouse.
    void ProcessMouseMovement(float xoffset, float yoffset)
    {
        xoffset *= MouseSensitivity;
        yoffset *= MouseSensitivity;

        Yaw   += xoffset;
        Pitch -= yoffset; // Invert y-axis for natural orbit

        // Make sure that when pitch is out of bounds, screen doesn't get flipped
        if (Pitch > 89.0f)
            Pitch = 89.0f;
        if (Pitch < -89.0f)
            Pitch = -89.0f;

        // update Front, Right and Up Vectors using the updated Euler angles
        updateCameraVectors();
    }

    // Processes input received from a mouse scroll-wheel event.
    void ProcessMouseScroll(float yoffset)
    {
        Distance -= yoffset;
        if (Distance < MIN_ZOOM)
            Distance = MIN_ZOOM;
        if (Distance > MAX_ZOOM)
            Distance = MAX_ZOOM;
        
        updateCameraVectors();
    }


private:
    // Calculates the front vector from the Camera's (updated) Euler Angles
    void updateCameraVectors()
    {
        // Calculate the new camera position using spherical coordinates
        float x = Target.x + Distance * cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
        float y = Target.y + Distance * sin(glm::radians(Pitch));
        float z = Target.z + Distance * sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
        Position = glm::vec3(x, y, z);

        // Recalculate the direction vectors
        Front = glm::normalize(Target - Position);
        Right = glm::normalize(glm::cross(Front, WorldUp));
        Up    = glm::normalize(glm::cross(Right, Front));
    }
};