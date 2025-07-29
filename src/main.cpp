// This define must be at the top, before any includes
#define STB_IMAGE_IMPLEMENTATION

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/quaternion.hpp> // for rotation against plane's axis...

#include "Shader.h"
#include "Camera.h"
#include "Model.h" // Use the new Model header

#include <iostream>
#include <iomanip> // print speed on console


// Function Prototypes
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset); // Added for orbit camera
void processInput(GLFWwindow *window);

// Window dimensions
const unsigned int SCR_WIDTH = 1280;
const unsigned int SCR_HEIGHT = 720;


// --- UPDATED: Plane state using Quaternions for orientation ---
glm::vec3 planePos( 0.0f, 20.0f,  50.0f );
float planeSpeed = 10.0f;
const float turnSpeed = 80.0f;
glm::quat planeOrientation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f); // Identity quaternion

// --- UPDATED: Initialize the camera to target the plane's starting position ---
Camera camera(planePos);
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// Timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

 // Tracking  Previous Plane Position to Calculate Direction
glm::vec3 lastPlanePos = planePos;

// GLFW Error Callback
void glfw_error_callback(int error, const char* description) {
    std::cerr << "GLFW Error (" << error << "): " << description << std::endl;
}

int main() {
    // Set error callback and initialize GLFW
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }

    // Set window hints for OpenGL 3.3 Core Profile
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // Create the GLFW window
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "City Scene", NULL, NULL);
    if (window == NULL) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    // --- UPDATED: Register the scroll callback ---
    // Set callbacks
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    
    // Capture the mouse cursor
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // Initialize GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        return -1;
    }
    
    // Configure global OpenGL state
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);

    // Build and compile our shaders
    Shader ourShader("../src/shaders/vertex.glsl", "../src/shaders/fragment.glsl");

    // Load both models
    Model pierModel("../src/Models/casa_city_logo.glb");
    Model planeModel("../src/Models/plane/plane.glb");


    // Define a light source position in world space
    glm::vec3 lightPos(5.0f, 20.0f, 15.0f);

    // --- NEW: Apply initial correction rotations to the quaternion ---
    planeOrientation = glm::rotate(planeOrientation, glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    planeOrientation = glm::rotate(planeOrientation, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));


    // Main Render loop
    while (!glfwWindowShouldClose(window)) {
        // Per-frame time logic
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // Input
        processInput(window);

        // Render
        glClearColor(0.1f, 0.2f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Activate shader
        ourShader.use();

        // Set uniforms that are the same for all objects
        ourShader.setVec3("lightPos", lightPos);
        ourShader.setVec3("viewPos", camera.Position);
        ourShader.setVec3("lightColor", 1.0f, 1.0f, 1.0f);

        // --- UPDATED: Use a fixed FOV because the orbit camera has no "Zoom" member ---
        // Set view/projection matrices (same for all objects)
        glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 1000.0f);
        
        // --- UPDATED: The GetViewMatrix() now handles everything. No more manual camera positioning! ---
        glm::mat4 view = camera.GetViewMatrix();
        ourShader.setMat4("projection", projection);
        ourShader.setMat4("view", view);

        
        // --- Draw the Pier ---
        glm::mat4 modelMatrix = glm::mat4(1.0f);
        modelMatrix = glm::translate(modelMatrix, glm::vec3(0.0f, 0.0f, 0.0f));
        modelMatrix = glm::scale(modelMatrix, glm::vec3(2.0f, 2.0f, 2.0f));     
        ourShader.setMat4("model", modelMatrix);
        pierModel.Draw(ourShader);


        
        // --- FINALIZED PLANE LOGIC (with Quaternions) ---

        // 1. Control speed with keyboard
        if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) planeSpeed += 20.0f * deltaTime;
        if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS) planeSpeed -= 20.0f * deltaTime;
        if (planeSpeed < 0.0f) planeSpeed = 0.0f;

        // 2. Calculate rotation amounts for this frame
        float yawAmount = 0.0f;
        float pitchAmount = 0.0f;
        float rollAmount = 0.0f;
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) yawAmount = turnSpeed * deltaTime;
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) yawAmount = -turnSpeed * deltaTime;
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) pitchAmount = turnSpeed * deltaTime;
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) pitchAmount = -turnSpeed * deltaTime; 

        // 3. Create small rotation quaternions for this frame's input
        glm::quat pitchQuat = glm::angleAxis(glm::radians(pitchAmount), glm::vec3(1.0f, 0.0f, 0.0f));
        glm::quat yawQuat = glm::angleAxis(glm::radians(yawAmount), glm::vec3(0.0f, 1.0f, 0.0f));

        // 4. Update the main orientation by multiplying with the new rotations
        // This applies the rotation in the plane's local space
        planeOrientation = yawQuat * planeOrientation;
        planeOrientation = planeOrientation * pitchQuat;
        
        // 5. Derive the TRUE forward, up, and right vectors from the orientation
        // This model's "forward" is its local Y-axis due to initial rotations
        glm::vec3 planeForward = -(planeOrientation * glm::vec3(0.0f, 1.0f, 0.0f)); // so it moves forward in the direction it's facing
        glm::vec3 planeUp = planeOrientation * glm::vec3(0.0f, 0.0f, 1.0f); // Becomes the new "up"
        glm::vec3 planeRight = planeOrientation * glm::vec3(1.0f, 0.0f, 0.0f);


        // 6. Update the plane's position
        planePos += planeForward * planeSpeed * deltaTime;

        // 7. Update the camera's target
        camera.Target = planePos;

        // 8. Build the final model matrix for rendering
        modelMatrix = glm::mat4(1.0f);
        modelMatrix = glm::translate(modelMatrix, planePos);
        modelMatrix = modelMatrix * glm::mat4_cast(planeOrientation); // Convert quaternion to rotation matrix
        modelMatrix = glm::scale(modelMatrix, glm::vec3(0.05f));
        
        ourShader.setMat4("model", modelMatrix);
        planeModel.Draw(ourShader);





        // --- NEW: Print speed to console every half second ---
        std::cout << "Plane Speed: " << std::fixed << std::setprecision(1) << planeSpeed << " m/s\r";


        // Swap buffers and poll IO events
        glfwSwapBuffers(window);
        glfwPollEvents();
        
    }
    
    glfwTerminate();
    return 0;
}

// Handles keyboard input for camera movement
void processInput(GLFWwindow *window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

}

// Callback for when the window is resized
void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

// Callback for when the mouse is moved
void mouse_callback(GLFWwindow* window, double xposIn, double yposIn) {
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; 

    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement(xoffset, yoffset);
}

// --- UPDATED: Added the scroll callback function ---
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(static_cast<float>(yoffset));
}