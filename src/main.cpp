// This define must be at the top, before any includes
#define STB_IMAGE_IMPLEMENTATION

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Shader.h"
#include "Camera.h"
#include "Model.h" // Use the new Model header

#include <iostream>

// Function Prototypes
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void processInput(GLFWwindow *window);

// Window dimensions
const unsigned int SCR_WIDTH = 1280;
const unsigned int SCR_HEIGHT = 720;

// Camera - Adjusted for a better view of the scene
Camera camera(glm::vec3(0.0f, 4.0f, 18.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// Timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

// ——————————————————————————————————————————
// Plane state (replaces the sine‑wave animation)
glm::vec3 planePos( 0.0f, 2.0f,  0.0f );
const float  planeSpeed =  5.0f;  // units per second
// ——————————————————————————————————————————

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

    // Set callbacks
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);

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
    Model pierModel("../src/Models/pier.obj");
    Model planeModel("../src/Models/plane/plane.glb");
    
    // Define a light source position in world space
    glm::vec3 lightPos(5.0f, 20.0f, 15.0f);

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

        // Set view/projection matrices (same for all objects)
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        
        // offset the camera 10 units back and 3 up relative to the plane
        camera.Position = planePos + glm::vec3(0.0f, 3.0f, 10.0f);
        camera.Front    = glm::normalize(planePos - camera.Position);

        glm::mat4 view = camera.GetViewMatrix();
        ourShader.setMat4("projection", projection);
        ourShader.setMat4("view", view);

        
        // --- Draw the Pier ---
        glm::mat4 modelMatrix = glm::mat4(1.0f);
        modelMatrix = glm::translate(modelMatrix, glm::vec3(0.0f, -2.0f, 0.0f)); 
        modelMatrix = glm::scale(modelMatrix, glm::vec3(2.0f, 2.0f, 2.0f));     
        ourShader.setMat4("model", modelMatrix);
        pierModel.Draw(ourShader);

        // ——— 1) drive planePos in world axes ———
        float moveSpeed = planeSpeed * deltaTime;
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) planePos += glm::vec3( 0, 0, -1) * moveSpeed;
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) planePos += glm::vec3( 0, 0,  1) * moveSpeed;
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) planePos += glm::vec3(-1, 0,  0) * moveSpeed;
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) planePos += glm::vec3( 1, 0,  0) * moveSpeed;

        // ——— 2) chase‑cam behind plane ———
        camera.Position = planePos + glm::vec3(0.0f, 3.0f, 10.0f);
        camera.Front    = glm::normalize(planePos - camera.Position);
        view = camera.GetViewMatrix();
        ourShader.setMat4("view", view);

        // ——— 3) draw plane at planePos, no yaw ———
        modelMatrix = glm::mat4(1.0f);
        modelMatrix = glm::translate(modelMatrix, planePos);
        // flip X↔Y if your model lies flat, otherwise remove:
        modelMatrix = glm::rotate(modelMatrix, glm::radians(-90.0f), glm::vec3(1,0,0));
        modelMatrix = glm::scale(modelMatrix, glm::vec3(0.01f));
        ourShader.setMat4("model", modelMatrix);
        planeModel.Draw(ourShader);

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
