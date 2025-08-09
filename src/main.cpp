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

// New includes for enemies/projectiles and RNG
#include <vector>
#include <random>
#include <chrono>
#include <cmath>   // acos, sqrt, etc

// Function Prototypes
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset); // Added for orbit camera
void processInput(GLFWwindow *window);
bool CheckCollision(const glm::vec3& sphereCenter, float sphereRadius, const glm::vec3& boxMin, const glm::vec3& boxMax);

// Window dimensions
const unsigned int SCR_WIDTH = 1280;
const unsigned int SCR_HEIGHT = 720;


// Plane state using Quaternions for orientation ---
glm::vec3 planePos( 0.0f, 550.0f,  50.0f ); // Starting position of the plane
float planeSpeed = 10.0f;
const float turnSpeed = 35.0f;
glm::quat planeOrientation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f); // Identity quaternion


float propellerAngle = 0.0f;
float rudderAngle = 0.0f;
float flapAngle = 0.0f;


// Initialize the camera to target the plane's starting position ---
Camera camera(planePos);
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// Timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

 // Tracking  Previous Plane Position to Calculate Direction
glm::vec3 lastPlanePos = planePos;


// ------------------ Enemy / Projectile system ------------------
struct Enemy {
    glm::vec3 pos;
    glm::vec3 target;   // where it's currently heading
    float speed;
    float yaw;          // orientation (radians)
};

struct Projectile {
    glm::vec3 pos;
    glm::vec3 vel;
    float life;         // seconds
};

std::vector<Enemy> enemies;       // active enemy planes
std::vector<Projectile> projectiles; // active bullets

float enemySpawnTimer = 0.0f;
float enemySpawnInterval = 6.0f;   // seconds between spawns (tune)
int   maxEnemies = 12;           // cap number of enemies

int lastMouseLeftState = GLFW_RELEASE; // to detect click -> on press

// control projectile visual size (tweak)
float bulletScale = 0.6f; // try 0.2 - 1.0 to adjust size

// RNG for spawning
std::mt19937 rng((unsigned)std::chrono::system_clock::now().time_since_epoch().count());
std::uniform_real_distribution<float> uniformAngle(0.0f, 2.0f * 3.14159265f);
std::uniform_real_distribution<float> uniformRadius(300.0f, 1200.0f); // spawn distance from center (tune)
std::uniform_real_distribution<float> uniformSpeed(2.0f, 8.0f); // enemy speed range
// ---------------------------------------------------------------

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
    Shader solidShader("../src/shaders/solid.vs", "../src/shaders/solid.fs"); //

    // Load models: city, player plane, sun (visual), and bullet (projectile)
    Model pierModel("../src/Models/casa_city_logo.glb");
    std::cout << "DEBUG:::" << " City model has " << pierModel.meshes.size() << " meshes." << std::endl;
    Model planeModel("../src/Models/plane/colombian_emb_314_tucano.glb");
    Model sunModel("../src/Models/sphere.obj");       // visual sphere used for sun / debug marker
    Model bulletModel("../src/Models/bullet.glb");     // projectile model (put bullet.glb here)

    // Define a light source position in world space
    glm::vec3 lightPos;

    // --- NEW: Apply initial correction rotations to the quaternion ---
    planeOrientation = glm::rotate(planeOrientation, glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f)); // without this, the plane faces the wrong way , don't know why , but it just works....
    // planeOrientation = glm::rotate(planeOrientation, glm::radians(-90.0f), glm::vec3(1.0f, .0f, 0.0f)); // sovled...... don't uncomment it back.


    // --- Shadow Mapping Setup ---
    const unsigned int SHADOW_WIDTH = 2048, SHADOW_HEIGHT = 2048;
    unsigned int depthMapFBO;
    glGenFramebuffers(1, &depthMapFBO);

    // Create depth texture
    unsigned int depthMap;
    glGenTextures(1, &depthMap);
    glBindTexture(GL_TEXTURE_2D, depthMap);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    float borderColor[] = { 1.0, 1.0, 1.0, 1.0 };
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

    // Attach depth texture as FBO's depth buffer
    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // Compile the new depth shader
    Shader depthShader("../src/shaders/shadow_depth.vs", "../src/shaders/shadow_depth.fs");

    // Set the texture units for the main shader (do this once)
    ourShader.use();
    ourShader.setInt("texture_diffuse1", 0);
    ourShader.setInt("shadowMap", 1);

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
        
        ourShader.setVec3("skyColor", 0.02f, 0.03f, 0.05f);   // A very dark, subtle sky blue
        ourShader.setVec3("groundColor", 0.03f, 0.03f, 0.03f); // A very dark, neutral gray for city reflections

        // Set view/projection matrices (same for all objects)
        glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 5000.0f);
        
        // Camera view matrix
        glm::mat4 view = camera.GetViewMatrix();
        ourShader.setMat4("projection", projection);
        ourShader.setMat4("view", view);

        // ======== 1. RENDER DEPTH MAP (Shadow Pass) ========
        glm::mat4 lightProjection, lightView;
        glm::mat4 lightSpaceMatrix;
        float near_plane = 1.0f, far_plane = 2000.0f; // Tweak these values based on your scene size
        // Use an orthographic projection for a directional light like the sun
        lightProjection = glm::ortho(-500.0f, 500.0f, -500.0f, 500.0f, near_plane, far_plane);
        lightView = glm::lookAt(lightPos, glm::vec3(0.0f), glm::vec3(0.0, 1.0, 0.0));
        lightSpaceMatrix = lightProjection * lightView;

        // Render scene from light's point of view
        depthShader.use();
        depthShader.setMat4("lightSpaceMatrix", lightSpaceMatrix);

        glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
        glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
        glClear(GL_DEPTH_BUFFER_BIT);
        // ONLY render objects that should CAST shadows.
        // For now, let's just make the plane cast a shadow.
        glm::mat4 planeModelMatrix = glm::translate(glm::mat4(1.0f), planePos) * glm::mat4_cast(planeOrientation);
        planeModelMatrix = glm::scale(planeModelMatrix, glm::vec3(0.05f));
        depthShader.setMat4("model", planeModelMatrix);
        planeModel.Draw(depthShader);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        // ======== 2. RENDER SCENE NORMALLY (Main Pass) ========
        // Reset viewport
        glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // Clear again for the main pass

        // --- Animate the Sun and Draw the Scene ---

        // 1. Animate the light source (the sun) to orbit the city
        glm::mat4 modelMatrix = glm::mat4(1.0f); 
        float orbitRadius = 400.0f;
        float orbitSpeed = 0.015f;
        lightPos.x = sin(glfwGetTime() * orbitSpeed) * orbitRadius;
        lightPos.y = 1600.0f; // Keep the sun at a constant height, HEIGHT OF THE SUN
        lightPos.z = cos(glfwGetTime() * orbitSpeed) * orbitRadius;

        // 2. Draw the Sun model (visual) using solid color shader (keeps it visible)
        solidShader.use();
        solidShader.setMat4("projection", projection);
        solidShader.setMat4("view", view);
        solidShader.setVec3("lightPos", lightPos); 
        solidShader.setVec3("viewPos", camera.Position);
        solidShader.setVec3("lightColor", 1.0f, 1.0f, 1.0f);
        solidShader.setVec3("objectColor", 1.0f, 1.0f, 0.0f); // Bright yellow for the sun

        modelMatrix = glm::mat4(1.0f);
        modelMatrix = glm::translate(modelMatrix, lightPos);
        modelMatrix = glm::scale(modelMatrix, glm::vec3(25.0f));
        solidShader.setMat4("model", modelMatrix);
        sunModel.Draw(solidShader); // draw visual sun

        // 3. Draw the City/Pier and Plane (using the main texture shader)
        ourShader.use();
        // Set the view and projection matrices for the camera
        ourShader.setMat4("projection", projection);
        ourShader.setMat4("view", view);

        // Pass the light space matrix to the main shader
        ourShader.setMat4("lightSpaceMatrix", lightSpaceMatrix);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, depthMap);

        // Draw the City/Pier model
        glActiveTexture(GL_TEXTURE0);
        modelMatrix = glm::mat4(1.0f);
        modelMatrix = glm::translate(modelMatrix, glm::vec3(0.0f, 0.0f, 0.0f));
        modelMatrix = glm::scale(modelMatrix, glm::vec3(2.0f, 2.0f, 2.0f));      
        ourShader.setMat4("model", modelMatrix);
        pierModel.Draw(ourShader);


        // ------------------ ENEMY SPAWN (timer) ------------------
        enemySpawnTimer += deltaTime;
        if (enemySpawnTimer >= enemySpawnInterval && (int)enemies.size() < maxEnemies) {
            enemySpawnTimer = 0.0f;
            float a = uniformAngle(rng);
            float r = uniformRadius(rng);
            Enemy e;
            e.pos = glm::vec3(sin(a) * r, 500.0f, cos(a) * r); // spawn high
            std::uniform_real_distribution<float> off(-150.0f, 150.0f);
            e.target = glm::vec3(off(rng), 0.0f, off(rng));
            e.speed = uniformSpeed(rng);
            e.yaw = 0.0f;
            enemies.push_back(e);
        }

        // ------------------ UPDATE & DRAW ENEMIES ------------------
        for (auto &e : enemies) {
            glm::vec3 toTarget = e.target - e.pos;
            float dist = glm::length(toTarget);
            glm::vec3 dir = (dist > 0.001f) ? glm::normalize(toTarget) : glm::vec3(0.0f);
            e.pos += dir * e.speed * deltaTime;
            if (dist < 20.0f) {
                std::uniform_real_distribution<float> off(-300.0f, 300.0f);
                e.target = glm::vec3(off(rng), 0.0f, off(rng));
            }
            if (glm::length(dir) > 0.001f) {
                float yaw = atan2(dir.x, dir.z);
                e.yaw = yaw;
            }

            // draw
            glm::mat4 enemyModel = glm::mat4(1.0f);
            enemyModel = glm::translate(enemyModel, e.pos);
            // rotate so model faces heading (adjust sign if needed)
            enemyModel = glm::rotate(enemyModel, -e.yaw, glm::vec3(0.0f, 1.0f, 0.0f));
            enemyModel = glm::scale(enemyModel, glm::vec3(0.05f));
            ourShader.setMat4("model", enemyModel);
            planeModel.Draw(ourShader);
        }

        // ------------------ SHOOTING (left mouse press) ------------------
        int curLeft = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT);
        if (curLeft == GLFW_PRESS && lastMouseLeftState == GLFW_RELEASE) {
            std::cout << "DEBUG: Fire! projectiles currently: " << projectiles.size() << std::endl;

            float bulletSpeed = 200.0f; // tune
            glm::vec3 localForward(0.0f, 0.0f, 1.0f);
            glm::vec3 localUp(0.0f, 1.0f, 0.0f);
            glm::vec3 localRight(1.0f, 0.0f, 0.0f);

            glm::vec3 planeForward = planeOrientation * localForward;
            glm::vec3 planeUp = planeOrientation * localUp;
            glm::vec3 planeRight = planeOrientation * localRight;

            // offsets in *model* space (tune these so bullets come from wings)
            float forwardOffset = 4.0f;   // forward from plane pivot
            float wingOffset = 8.0f;      // FIX 1: Increased value for wider bullet spread
            float verticalOffset = -0.5f; // small downward offset so not inside model

            // spawn two bullets: left (-1) and right (+1)
            for (int sign = -1; sign <= 1; sign += 2) {
                Projectile p;
                p.pos = planePos
                    + planeForward * forwardOffset
                    + planeRight * (sign * wingOffset)
                    + planeUp * verticalOffset;
                p.vel = planeForward * bulletSpeed;
                p.life = 6.0f;
                projectiles.push_back(p);
            }
        }
        lastMouseLeftState = curLeft;


        // ------------------ UPDATE & DRAW PROJECTILES ------------------
        for (int i = (int)projectiles.size() - 1; i >= 0; --i) {
            Projectile &p = projectiles[i];
            p.pos += p.vel * deltaTime;
            p.life -= deltaTime;
            bool removeProj = (p.life <= 0.0f);

            if (!removeProj) {
                // Draw projectile using the main textured shader so the GLB's original material shows.
                ourShader.use();
                ourShader.setMat4("projection", projection);
                ourShader.setMat4("view", view);
                ourShader.setInt("unlit", 1); // preserve the model's albedo / baked colours

                // build orientation so model forward aligns with velocity direction
                glm::vec3 dir = glm::normalize(p.vel);
                if (glm::length(dir) < 1e-6f) dir = glm::vec3(0.0f, 0.0f, 1.0f);

                // FIX 2: Replace manual quaternion math with a robust "lookAt" method
                // Create a rotation matrix that makes the bullet "look at" its direction of travel.
                // glm::lookAt creates a view matrix (which aligns -Z forward). We invert it to get a model matrix.
                glm::mat4 rot = glm::inverse(glm::lookAt(glm::vec3(0.0f), dir, glm::vec3(0.0f, 1.0f, 0.0f)));

                // The bullet model's nose is likely along its +Z axis, but lookAt aligns -Z.
                // We apply a 180-degree rotation around the Y axis to flip it around.
                rot = rot * glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));

                glm::mat4 projModel = glm::mat4(1.0f);
                projModel = glm::translate(projModel, p.pos);
                projModel = projModel * rot;                         // orient to flight direction
                projModel = glm::scale(projModel, glm::vec3(bulletScale)); // controlled size

                ourShader.setMat4("model", projModel);
                bulletModel.Draw(ourShader);

                ourShader.setInt("unlit", 0); // reset
            }

            // Check collision with enemies (simple distance test)
            if (!removeProj) {
                for (int j = (int)enemies.size() - 1; j >= 0; --j) {
                    float d = glm::length(projectiles[i].pos - enemies[j].pos);
                    const float hitThreshold = 8.0f; // tune
                    if (d < hitThreshold) {
                        enemies.erase(enemies.begin() + j);
                        removeProj = true;
                        break;
                    }
                }
            }

            if (removeProj) {
                projectiles.erase(projectiles.begin() + i);
            }
        }


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

        // --- NEW: Rudder Control Logic ---
        const float maxRudderAngle = 25.0f;   // The rudder's maximum turn in degrees.
        const float rudderTurnSpeed = 150.0f; // How fast the rudder reacts.

        if (yawAmount > 0)
        { // 'A' key is pressed
            rudderAngle += rudderTurnSpeed * deltaTime;
        }
        else if (yawAmount < 0)
        { // 'D' key is pressed
            rudderAngle -= rudderTurnSpeed * deltaTime;
        }
        else
        {
            // If no key is pressed, smoothly return the rudder to the center.
            if (rudderAngle > 0.1f)
            {
                rudderAngle -= rudderTurnSpeed * deltaTime;
            }
            else if (rudderAngle < -0.1f)
            {
                rudderAngle += rudderTurnSpeed * deltaTime;
            }
            else
            {
                rudderAngle = 0.0f;
            }
        }
        // Clamp the rudderAngle to its maximum limits.
        rudderAngle = glm::clamp(rudderAngle, -maxRudderAngle, maxRudderAngle);

        // FLAPSSSSSSSSS
        const float maxFlapAngle = 40.0f;   // Max deployment angle in degrees.
        const float minFlapAngle = -15.0f;  // Minimum retraction angle in degrees.
        const float flapDeploySpeed = 80.0f;  // How fast the flaps move.

        if (pitchAmount < 0) { // 'S' key is pressed, deploy flaps.
            flapAngle += flapDeploySpeed * deltaTime;
        } else if (pitchAmount > 0) { // 'W' key is pressed, retract flaps.
            flapAngle -= flapDeploySpeed * deltaTime;
        } else {
            // If NO pitch key is pressed, smoothly retract the flaps to their default position.
            if (flapAngle > 0.1f) {
                flapAngle -= flapDeploySpeed * deltaTime;
            } else if (flapAngle < -0.1f) {
                flapAngle += flapDeploySpeed * deltaTime;
            } else {
                flapAngle = 0.0f;
            }
        }
        // Clamp the flapAngle to its limits (0 = fully retracted, maxFlapAngle = fully deployed).
        flapAngle = glm::clamp(flapAngle, minFlapAngle, maxFlapAngle);

        // 3. Create small rotation quaternions for this frame's input
        glm::quat pitchQuat = glm::angleAxis(glm::radians(pitchAmount), glm::vec3(1.0f, 0.0f, 0.0f));
        glm::quat yawQuat = glm::angleAxis(glm::radians(yawAmount), glm::vec3(0.0f, 1.0f, 0.0f));

        // 4. Update the main orientation by multiplying with the new rotations
        // This applies the rotation in the plane's local space
        planeOrientation = yawQuat * planeOrientation;
        planeOrientation = planeOrientation * pitchQuat;
        
        // 5. Derive the TRUE forward, up, and right vectors from the orientation
        // For this new model, its nose points along its local Y-axis.
        glm::vec3 localForward(0.0f, 0.0f, 1.0f);
        // For this new model, its canopy points along its local Z-axis.
        glm::vec3 localUp(0.0f, 1.0f, 0.0f);

        // Transform these local axes into world-space vectors using the plane's current orientation
        glm::vec3 planeForward = planeOrientation * localForward;
        glm::vec3 planeUp = planeOrientation * localUp;
        glm::vec3 planeRight = glm::cross(planeForward, planeUp);


        // 6. Update the plane's position
        glm::vec3 nextPlanePos = planePos + planeForward * planeSpeed * deltaTime;

        // 7. Check for collisions
        bool collisionDetected = false;
        float planeRadius = 1.5f; // Bounding sphere radius for the plane, tweak as needed
        
        // Define the city's transformation matrix (position and scale)
        glm::mat4 cityModelMatrix = glm::mat4(1.0f);
        cityModelMatrix = glm::translate(cityModelMatrix, glm::vec3(0.0f, 0.0f, 0.0f));
        cityModelMatrix = glm::scale(cityModelMatrix, glm::vec3(2.0f, 2.0f, 2.0f));

        for (const auto& mesh : pierModel.meshes) {

            // --- NEW: Ignore any mesh that is unrealistically large ---
            if ((mesh.maxAABB.y - mesh.minAABB.y) > 100.0f) continue;

            // Transform the mesh's local AABB corners into world space
            glm::vec3 worldMin = cityModelMatrix * glm::vec4(mesh.minAABB, 1.0f);
            glm::vec3 worldMax = cityModelMatrix * glm::vec4(mesh.maxAABB, 1.0f);

            // IMPORTANT: Ensure min is actually min and max is max after transformation
            glm::vec3 realMin = glm::min(worldMin, worldMax);
            glm::vec3 realMax = glm::max(worldMin, worldMax);

            glm::vec3 boxSize = realMax - realMin;
            if (boxSize.y > 20.0f) { // Only print for objects taller than 20 units
                // std::cout << "DEBUG::: " << "Found a very large mesh! Size: Y = " << boxSize.y << std::endl;
            }

            if (CheckCollision(nextPlanePos, planeRadius, realMin, realMax)) {
                collisionDetected = true;
                break; // A collision was found, no need to check other meshes
            }

            
        }


        // 8. Build the final model matrix for rendering

        if (!collisionDetected) {
            planePos = nextPlanePos;
        }
        // camera.Target = planePos;


        // --- CAMERA TARGET FIX ---
        glm::vec3 modelCenterOffset(0.0f, 9.0f, 3.5f); 
        glm::vec3 visualCenter = planePos + (planeOrientation * modelCenterOffset);
        camera.Target = visualCenter;

        // Draw a small green marker at camera.Target using sunModel (keeps sunModel in the project)
        solidShader.use();
        solidShader.setMat4("projection", projection);
        solidShader.setMat4("view", view);
        solidShader.setVec3("objectColor", 0.0f, 1.0f, 0.0f); // Bright Green

        modelMatrix = glm::mat4(1.0f);
        modelMatrix = glm::translate(modelMatrix, camera.Target);
        modelMatrix = glm::scale(modelMatrix, glm::vec3(0.5f)); // Make it small
        solidShader.setMat4("model", modelMatrix);
        
        sunModel.Draw(solidShader); // Draw the green marker with the sphere model
        
        // Switch back to the main shader for the plane
        ourShader.use();
        // --------------------------------------------------------
        // 9. Update propeller angle for rotation
        const float idlePropellerSpeed = 60.0f;      // The propeller's minimum spin speed (degrees per second)
        const float propellerSpeedMultiplier = 9.0f; // How much faster the propeller spins per m/s of plane speed

        // Calculate the total rotation speed for this frame
        float currentPropellerSpeed = idlePropellerSpeed + (planeSpeed * propellerSpeedMultiplier);

        // Update the propeller's angle
        propellerAngle += currentPropellerSpeed * deltaTime;
        if (propellerAngle >= 360.0f) {
            propellerAngle -= 360.0f; // Keep the angle from growing infinitely large
        }

        // 10. Define the plane's base transformation for the current frame
        glm::mat4 planeBaseTransform = glm::translate(glm::mat4(1.0f), planePos) * glm::mat4_cast(planeOrientation);


        // // MESH NAMES OF THE PLANE MODEL
        // std::cout << "-------------------------------------\n";
        // std::cout << "\n--- Verifying Plane Mesh Names ---" << std::endl;
        // for (size_t i = 0; i < planeModel.meshes.size(); ++i) {
        //     std::cout << "Mesh at index " << i << " is named: '" << planeModel.meshes[i].name << "'" << std::endl;
        // }
        // std::cout << "-------------------------------------\n" << std::endl;


        // 11. Draw each part of the model with its correct transformation
        for (Mesh &mesh : planeModel.meshes)
        {
            glm::mat4 partTransform;

            // Check if the current mesh is the "Propeller"
            if (mesh.name == "Propeller_Paint_0")
            {
                // The offset to move the propeller to the nose of the plane
                glm::vec3 propellerOffset(0.0f, -0.1f, 1.75f);
                glm::mat4 propellerTranslate = glm::translate(glm::mat4(1.0f), propellerOffset);

                // --- The pivot correction you found in Part 1 ---
                glm::vec3 pivotCorrectionOffset(0.0f, 7.75f, 1.75f); // <-- Use your final tweaked values here!
                glm::mat4 translateToOrigin = glm::translate(glm::mat4(1.0f), -pivotCorrectionOffset);
                glm::mat4 propellerSpin = glm::rotate(glm::mat4(1.0f), glm::radians(propellerAngle), glm::vec3(0.0f, 0.0f, 1.0f));
                glm::mat4 translateBack = glm::translate(glm::mat4(1.0f), pivotCorrectionOffset);
                glm::mat4 correctedSpin = translateBack * propellerSpin * translateToOrigin;

                partTransform = planeBaseTransform * propellerTranslate * correctedSpin;
            }
            else if (mesh.name == "Rudder_Paint_0")
            {
                // 1. Define the rudder's pivot point (its hinge) in the plane's local space.
                glm::vec3 rudderPivot(0.0f, 0.85f, -23.0f); 

                // 2. Create matrices to perform a rotation around this specific pivot point.
                glm::mat4 translateToPivot = glm::translate(glm::mat4(1.0f), rudderPivot);
                glm::mat4 translateToModelOrigin = glm::translate(glm::mat4(1.0f), -rudderPivot);

                // 3. The rudder yaws around the plane's local UP axis (Y-axis).
                glm::mat4 rudderRotation = glm::rotate(glm::mat4(1.0f), glm::radians(rudderAngle), glm::vec3(0.0f, 1.0f, 0.0f));

                // 4. The final local transformation for the rudder.
                partTransform = planeBaseTransform * translateToPivot * rudderRotation * translateToModelOrigin;
            }
            // --- NEW: Check for the Flaps ---
            else if (mesh.name == "FlapR_Paint_0" || mesh.name == "FlapL_Paint_0")
            {
                // Define a separate pivot point (hinge) for each flap.
                // You will need to TUNE these values to position them correctly on the wings.
                glm::vec3 flapPivot; 
                if (mesh.name == "FlapR_Paint_0") {
                    flapPivot = glm::vec3(50.0f, 6.0f, 1.0f); // Right flap pivot (GUESS)
                } else { // FlapL_Paint_0
                    flapPivot = glm::vec3(-50.0f, 6.0f, 1.0f); // Left flap pivot (GUESS)
                }

                // Create matrices to rotate around the specific pivot point.
                glm::mat4 translateToPivot = glm::translate(glm::mat4(1.0f), flapPivot);
                glm::mat4 translateToModelOrigin = glm::translate(glm::mat4(1.0f), -flapPivot);

                // Flaps pitch around the plane's local RIGHT/LEFT axis (the X-axis).
                glm::mat4 flapRotation = glm::rotate(glm::mat4(1.0f), glm::radians(flapAngle), glm::vec3(1.0f, 0.0f, 0.0f));

                // Combine the matrices to create the final local transformation.
                partTransform = planeBaseTransform * translateToPivot * flapRotation * translateToModelOrigin;
            }
            else
            {
                // It's the plane body or another part, so just use the base transform.
                partTransform = planeBaseTransform;
            }

            // 12. Apply final scaling and draw the mesh.
            glm::mat4 finalModelMatrix = glm::scale(partTransform, glm::vec3(0.05f));
            ourShader.setMat4("model", finalModelMatrix);
            mesh.Draw(ourShader);
        }


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
void framebuffer_size_callback(GLFWwindow*window, int width, int height) {
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

bool CheckCollision(const glm::vec3& sphereCenter, float sphereRadius, const glm::vec3& boxMin, const glm::vec3& boxMax) {
    // Get the closest point on the AABB to the sphere's center
    float x = std::max(boxMin.x, std::min(sphereCenter.x, boxMax.x));
    float y = std::max(boxMin.y, std::min(sphereCenter.y, boxMax.y));
    float z = std::max(boxMin.z, std::min(sphereCenter.z, boxMax.z));

    // Calculate the distance between the closest point and the sphere's center
    float distance = std::sqrt(
        (x - sphereCenter.x) * (x - sphereCenter.x) +
        (y - sphereCenter.y) * (y - sphereCenter.y) +
        (z - sphereCenter.z) * (z - sphereCenter.z)
    );

    // If the distance is less than the sphere's radius, there is a collision
    return distance < sphereRadius;
}



void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(static_cast<float>(yoffset));
}