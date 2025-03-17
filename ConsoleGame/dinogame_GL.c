#define GL_SILENCE_DEPRECATION
#define GLFW_INCLUDE_GLCOREARB
#include <GLFW/glfw3.h>
#include <OpenGL/gl3.h>
#include <stdio.h>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include <stdbool.h>
#include <time.h>

#define WINDOW_WIDTH 600
#define WINDOW_HEIGHT 200

// Vertex Shader
const char* vertexShaderSource = "#version 330 core\n"
    "layout (location = 0) in vec2 aPos;\n"
    "layout (location = 1) in vec2 aTexCoord;\n"
    "out vec2 TexCoord;\n"
    "void main()\n"
    "{\n"
    "   gl_Position = vec4(aPos, 0.0, 1.0);\n"
    "   TexCoord = aTexCoord;\n"
    "}\0";

// Fragment Shader
const char* fragmentShaderSource = "#version 330 core\n"
    "in vec2 TexCoord;\n"
    "out vec4 FragColor;\n"
    "uniform sampler2D texture1;\n"
    "void main()\n"
    "{\n"
    "   FragColor = texture(texture1, TexCoord);\n"
    "}\0";

// Add these globals near the top of the file
float characterX = -1.0f;  // Starting X position
float characterY = -1.0f;  // Starting Y position
float velocityY = 0.0f;    // Vertical velocity
float gravity = -0.002f;   // Reduced gravity (was -0.02f)
float jumpForce = 0.075f;   // Reduced jump force (was 0.22f)
bool isJumping = false;    // Jump state

// Add near the top with other globals
GLuint texture1, texture2;          // Standing textures
GLuint crouchTexture1, crouchTexture2;  // Crouching textures
GLuint jumpTexture;
GLuint cactusTexture1, cactusTexture2;
GLuint cloudTexture, cloudTexture2, cloudTexture3;
GLuint replaybuttonTexture;
GLuint gameoverTexture;
bool isCrouching = false;
float normalHeight = 0.5f;     // Changed from 0.7f to 0.5f for 50px height
float crouchHeight = 0.25f;    // Half of normal height when crouching
int frameCount = 0;         // To track frames

// Add these globals near the top
GLuint cloudVAO, cloudVBO, cloudEBO;

// Add these globals near the top with other VAO/VBO declarations
GLuint cloudVAO2, cloudVBO2, cloudEBO2;
GLuint cloudVAO3, cloudVBO3, cloudEBO3;

// Add these globals near the top with other declarations
float cloud1X = 300.0f;  // Starting X positions in pixels
float cloud2X = 200.0f;
float cloud3X = 150.0f;
float cloud1Speed = 0.3f;  // Different speeds for each cloud (pixels per frame)
float cloud2Speed = 0.2f;
float cloud3Speed = 0.4f;
bool cloud1MovingLeft = true;  // Direction flags
bool cloud2MovingLeft = true;
bool cloud3MovingLeft = true;

// Add these structures and globals near the top
#define MAX_CACTI 10  // Maximum number of cacti on screen at once
#define MIN_DISTANCE 20  // Minimum distance between cactus groups

typedef struct {
    float x;
    float y;
    bool isSmall;  // true for small (30x30), false for big (40x40)
    bool isActive;
} Cactus;

typedef struct {
    Cactus cacti[3];  // Up to 3 cacti per group
    int count;        // Actual number of cacti in this group
    bool isActive;    // whether this group is currently in use
} CactusGroup;

CactusGroup cactusGroups[MAX_CACTI];
float lastGroupX = WINDOW_WIDTH;  // Start spawning from right edge

// Add near the top with other VAO/VBO declarations
GLuint cactusVAO, cactusVBO, cactusEBO;

// Add near the top with other globals
int targetActiveGroups;  // Number of desired active cactus groups

// Add these function declarations (prototypes) near the top of the file, after the structs
void initCactusGroups();
void generateNewCactusGroup();  // Add this prototype
void updateCactusGroups();
void renderCactusGroups();

// Add near the top with other structs
typedef struct {
    float x;
    float y;
    bool isActive;
} Bird;

// Add with other globals
#define MAX_BIRDS 1  // Changed from 2 to 1
Bird birds[MAX_BIRDS];
GLuint birdTexture1, birdTexture2;  // Two bird textures for animation
int birdFrameCount = 0;  // To track animation frames
GLuint birdVAO, birdVBO, birdEBO;
float birdY = 30.0f;  // Fixed Y position for birds

// Add these function prototypes
void initBirds();
void updateBirds();
void renderBirds();
void generateNewBird();

// Add near the top with other globals
bool isGameOver = false;

// Add this function before main()
bool checkCollision() {
    // Get character hitbox (adjust these values based on visual testing)
    float charWidth = 32.0f;  // Character width in pixels but we want to check at perfectly matching between them
    float charHeight = isCrouching ? 25.0f : 50.0f;  // Height changes when crouching
    float charLeft = (characterX + 1.0f) * WINDOW_WIDTH / 2.0f + 20.0f;  // Changed from 50.0f to 20.0f
    float charRight = charLeft + charWidth;
    
    // Fix character Y position calculation
    float charBottom = WINDOW_HEIGHT - ((characterY + 1.0f) * WINDOW_HEIGHT / 2.0f);  // Convert from OpenGL to screen coordinates
    float charTop = charBottom - charHeight;  // Measure from bottom up

    printf("Character hitbox: L=%.1f, R=%.1f, T=%.1f, B=%.1f\n", 
           charLeft, charRight, charTop, charBottom);

    // Check collision with cacti
    for (int i = 0; i < MAX_CACTI; i++) {
        if (cactusGroups[i].isActive) {
            for (int j = 0; j < cactusGroups[i].count; j++) {
                Cactus* cactus = &cactusGroups[i].cacti[j];
                float cactusWidth = cactus->isSmall ? 30.0f : 40.0f;
                float cactusHeight = cactus->isSmall ? 30.0f : 40.0f;
                float cactusLeft = cactus->x;
                float cactusRight = cactusLeft + cactusWidth;
                float cactusTop = WINDOW_HEIGHT - cactusHeight;  // Measure from top of window
                float cactusBottom = WINDOW_HEIGHT;

                // Check for overlap
                if (charRight > cactusLeft && 
                    charLeft < cactusRight && 
                    charBottom > cactusTop && 
                    charTop < cactusBottom) {
                    printf("Collision with cactus at x=%f\n", cactus->x);
                    return true;
                }
            }
        }
    }

    // Check collision with bird
    if (birds[0].isActive) {
        float birdWidth = 40.0f;
        float birdHeight = 40.0f;
        float birdLeft = birds[0].x;
        float birdRight = birdLeft + birdWidth;
        float birdTop = birds[0].y;
        float birdBottom = birdTop + birdHeight;

        if (charRight > birdLeft && 
            charLeft < birdRight && 
            charBottom > birdTop && 
            charTop < birdBottom) {
            printf("Collision with bird at x=%f, y=%f\n", birds[0].x, birds[0].y);
            return true;
        }
    }

    return false;
}

// Add these function declarations (prototypes) near the top of the file, after the structs
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (isGameOver) return;  // Don't handle jumps if game is over
    
    if (key == GLFW_KEY_SPACE && action == GLFW_PRESS && !isJumping) {
        velocityY = jumpForce;
        isJumping = true;
    }
}

void initCactusGroups() {
    for (int i = 0; i < MAX_CACTI; i++) {
        cactusGroups[i].isActive = false;
        cactusGroups[i].count = 0;
    }
}

void updateCactusGroups() {
    if (isGameOver) return;  // Don't update if game is over
    
    float scrollSpeed = 3.0f;
    int activeCount = 0;
    float rightmostX = 0;
    
    // Update positions and count active groups
    for (int i = 0; i < MAX_CACTI; i++) {
        if (cactusGroups[i].isActive) {
            activeCount++;
            for (int j = 0; j < cactusGroups[i].count; j++) {
                Cactus* cactus = &cactusGroups[i].cacti[j];
                cactus->x -= scrollSpeed;
                
                float cactusRight = cactus->x + (cactus->isSmall ? 30.0f : 40.0f);
                if (cactusRight > rightmostX) {
                    rightmostX = cactusRight;
                }
            }
            
            // Deactivate if rightmost cactus in group is off screen
            if (cactusGroups[i].cacti[cactusGroups[i].count - 1].x + 40.0f < 0) {
                cactusGroups[i].isActive = false;
                activeCount--;
                targetActiveGroups = (rand() % 3) + 1;  // Random number between 1 and 3
                printf("Group %d deactivated. New target: %d groups\n", i, targetActiveGroups);
            }
        }
    }
    
    // Generate new group if needed and enough space
    if (activeCount < targetActiveGroups && (rightmostX < WINDOW_WIDTH - 200 || rightmostX == 0)) {
        printf("Generating new group. Active: %d, Target: %d\n", activeCount, targetActiveGroups);
        generateNewCactusGroup();
    }
}

void generateNewCactusGroup() {
    for (int i = 0; i < MAX_CACTI; i++) {
        if (!cactusGroups[i].isActive) {
            cactusGroups[i].isActive = true;
            cactusGroups[i].count = (rand() % 2) + 1;  // 1 or 2 cacti per group
            
            float startX = WINDOW_WIDTH;  // Start at right edge of screen
            
            for (int j = 0; j < cactusGroups[i].count; j++) {
                cactusGroups[i].cacti[j].x = startX + (j * 25);  // Changed from 50 to 25 pixels spacing
                cactusGroups[i].cacti[j].y = -1.0f;  // Ground level
                cactusGroups[i].cacti[j].isSmall = (rand() % 2) == 0;  // Randomly choose small or large
            }
            break;
        }
    }
}

void renderCactusGroups() {
    glBindVertexArray(cactusVAO);
    
    for (int i = 0; i < MAX_CACTI; i++) {
        if (cactusGroups[i].isActive) {
            for (int j = 0; j < cactusGroups[i].count; j++) {
                Cactus* cactus = &cactusGroups[i].cacti[j];
                
                // Adjust width and height based on cactus size
                float cactusWidth = cactus->isSmall ? 0.075f : 0.1f;    // 30px vs 40px
                float cactusHeight = cactus->isSmall ? 0.375f : 0.5f;   // 30px vs 40px
                float cactusX = (cactus->x / WINDOW_WIDTH) * 2.0f - 1.0f;
                float cactusY = -1.0f;  // Ground level for all cacti
                
                float vertices[] = {
                    cactusX,           cactusY + cactusHeight, 0.0f, 0.0f,  // top left
                    cactusX,           cactusY,                0.0f, 1.0f,  // bottom left
                    cactusX + cactusWidth, cactusY,           1.0f, 1.0f,  // bottom right
                    cactusX + cactusWidth, cactusY + cactusHeight, 1.0f, 0.0f   // top right
                };
                
                glBindBuffer(GL_ARRAY_BUFFER, cactusVBO);
                glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);
                
                glBindTexture(GL_TEXTURE_2D, cactusTexture1);
                
                glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
            }
        }
    }
}

void initBirds() {
    for (int i = 0; i < MAX_BIRDS; i++) {
        birds[i].isActive = false;
    }
}

void generateNewBird() {
    // Only generate if there's no active bird
    if (!birds[0].isActive) {
        // Check if there's enough space between cactus groups
        bool hasSpace = true;
        for (int i = 0; i < MAX_CACTI; i++) {
            if (cactusGroups[i].isActive) {
                for (int j = 0; j < cactusGroups[i].count; j++) {
                    float cactusX = cactusGroups[i].cacti[j].x;
                    // Check if any cactus is too close to where we want to spawn the bird
                    if (cactusX > WINDOW_WIDTH - 100 && cactusX < WINDOW_WIDTH + 50) {
                        hasSpace = false;
                        break;
                    }
                }
            }
        }
        
        if (hasSpace) {
            birds[0].isActive = true;
            birds[0].x = WINDOW_WIDTH;  // Start at right edge (x = 600)
            birds[0].y = birdY;
            printf("Generated bird at x=%f\n", birds[0].x);
        }
    }
}

void updateBirds() {
    if (isGameOver) return;  // Don't update if game is over
    
    float scrollSpeed = 3.0f;
    
    for (int i = 0; i < MAX_BIRDS; i++) {
        if (birds[i].isActive) {
            birds[i].x -= scrollSpeed;
            
            // Deactivate if off screen
            if (birds[i].x + 40.0f < 0) {  // Assuming 40px width
                birds[i].isActive = false;
                printf("Bird %d deactivated\n", i);
            }
        }
    }
    
    // Try to generate new bird
    if (rand() % 100 < 2) {  // 2% chance each frame
        generateNewBird();
    }
}

void renderBirds() {
    glBindVertexArray(birdVAO);
    
    for (int i = 0; i < MAX_BIRDS; i++) {
        if (birds[i].isActive) {
            // Fixed size for birds (40x40 pixels)
            float birdWidth = 0.133f;   // 40px / 300px window width
            float birdHeight = 0.4f;    // 40px / 100px window height
            float birdX = (birds[i].x / WINDOW_WIDTH) * 2.0f - 1.0f;
            // Convert from pixel coordinates to OpenGL coordinates
            float birdY = ((WINDOW_HEIGHT - birds[i].y) / WINDOW_HEIGHT) * 2.0f - 1.0f - birdHeight;
            
            float vertices[] = {
                birdX,           birdY + birdHeight,  0.0f, 0.0f,  // top left
                birdX,           birdY,               0.0f, 1.0f,  // bottom left
                birdX + birdWidth, birdY,            1.0f, 1.0f,  // bottom right
                birdX + birdWidth, birdY + birdHeight, 1.0f, 0.0f   // top right
            };
            
            glBindBuffer(GL_ARRAY_BUFFER, birdVBO);
            glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);
            
            // Always use first bird texture when game is over
            if (isGameOver) {
                glBindTexture(GL_TEXTURE_2D, birdTexture1);
            } else {
                glBindTexture(GL_TEXTURE_2D, (birdFrameCount / 20) % 2 == 0 ? birdTexture1 : birdTexture2);
            }
            
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        }
    }
}

int main() {
    if (!glfwInit()) {
        fprintf(stderr, "Failed to initialize GLFW\n");
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    GLFWwindow* window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Dino Game", NULL, NULL);
    if (!window) {
        fprintf(stderr, "Failed to create GLFW window\n");
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    // Add after window creation
    glfwSetKeyCallback(window, key_callback);

    // Initialize random seed
    srand(time(NULL));

    // Initialize cactus system
    initCactusGroups();

    // Set initial target for active groups (1-3)
    int targetActiveGroups = (rand() % 3) + 1;
    printf("Initial target active groups: %d\n", targetActiveGroups);

    // Initialize cactus VAO/VBO/EBO
    glGenVertexArrays(1, &cactusVAO);
    glGenBuffers(1, &cactusVBO);
    glGenBuffers(1, &cactusEBO);

    glBindVertexArray(cactusVAO);
    glBindBuffer(GL_ARRAY_BUFFER, cactusVBO);

    // Set up indices
    unsigned int indices[] = {
        0, 1, 2,   // First triangle
        2, 3, 0    // Second triangle
    };
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cactusEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // Set up vertex attributes for cactus
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);

    printf("Cactus VAO/VBO/EBO initialized\n");

    // Generate initial cactus group
    generateNewCactusGroup();

    // Quad vertices (X, Y, Texture X, Texture Y)
    float vertices[] = {
        // Positions   // Texture Coords
        -1.0f,         -0.5f,         0.0f, 0.0f,  // Top-left
        -1.0f,         -1.0f,         0.0f, 1.0f,  // Bottom-left
        -1.0f + 0.167f, -1.0f,        1.0f, 1.0f,  // Bottom-right
        -1.0f + 0.167f, -0.5f,        1.0f, 0.0f   // Top-right
    };

    GLuint VAO, VBO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // Compile shaders
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);

    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);

    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    // Load first texture
    int width, height, nrChannels;
    unsigned char* data = stbi_load("assets/main-character1.png", &width, &height, &nrChannels, 0);
    if (!data) {
        fprintf(stderr, "Failed to load texture 1\n");
        return -1;
    }

    glGenTextures(1, &texture1);
    glBindTexture(GL_TEXTURE_2D, texture1);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);
    stbi_image_free(data);

    // Load second texture
    data = stbi_load("assets/main-character2.png", &width, &height, &nrChannels, 0);
    if (!data) {
        fprintf(stderr, "Failed to load texture 2\n");
        return -1;
    }

    glGenTextures(1, &texture2);
    glBindTexture(GL_TEXTURE_2D, texture2);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);
    stbi_image_free(data);

    // Load crouching textures
    data = stbi_load("assets/main-character5.png", &width, &height, &nrChannels, 0);
    if (!data) {
        fprintf(stderr, "Failed to load crouch texture 1\n");
        return -1;
    }

    glGenTextures(1, &crouchTexture1);
    glBindTexture(GL_TEXTURE_2D, crouchTexture1);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);
    stbi_image_free(data);

    data = stbi_load("assets/main-character6.png", &width, &height, &nrChannels, 0);
    if (!data) {
        fprintf(stderr, "Failed to load crouch texture 2\n");
        return -1;
    }

    glGenTextures(1, &crouchTexture2);
    glBindTexture(GL_TEXTURE_2D, crouchTexture2);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);
    stbi_image_free(data);

    data = stbi_load("assets/main-character3.png", &width, &height, &nrChannels, 0);
    if (!data)
    {
        fprintf(stderr, "Failed to load jump texture 3\n");
        return -1;
    }

    glGenTextures(1, &jumpTexture);
    glBindTexture(GL_TEXTURE_2D, jumpTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);
    stbi_image_free(data);

    data = stbi_load("assets/cactus1.png", &width, &height, &nrChannels, 0);
    if (data) {
        glGenTextures(1, &cactusTexture1);
        glBindTexture(GL_TEXTURE_2D, cactusTexture1);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        printf("Cactus texture loaded successfully: %dx%d\n", width, height);
        stbi_image_free(data);
    } else {
        printf("Failed to load cactus texture\n");
    }

    data = stbi_load("assets/cactus2.png", &width, &height, &nrChannels, 0);
    if(!data){
        fprintf(stderr, "Failed to load cactus texture 2\n");
        return -1;
    }

    glGenTextures(1, &cactusTexture2);
    glBindTexture(GL_TEXTURE_2D, cactusTexture2);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);
    stbi_image_free(data);

    data = stbi_load("assets/cloud.png", &width, &height, &nrChannels, 0);  
    if(!data){
        fprintf(stderr, "Failed to load cloud texture\n");
        return -1;
    }

    glGenTextures(1, &cloudTexture);
    glBindTexture(GL_TEXTURE_2D, cloudTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);
    stbi_image_free(data);

    data = stbi_load("assets/replay_button.png", &width, &height, &nrChannels, 0);
    if(!data){
        fprintf(stderr, "Failed to load replay button texture\n");
        return -1;
    }

    glGenTextures(1, &replaybuttonTexture);
    glBindTexture(GL_TEXTURE_2D, replaybuttonTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);
    stbi_image_free(data);

    data = stbi_load("assets/gameover_text.png", &width, &height, &nrChannels, 0);
    if(!data){
        fprintf(stderr, "Failed to load game over texture\n");
        return -1;
    }   

    glGenTextures(1, &gameoverTexture);
    glBindTexture(GL_TEXTURE_2D, gameoverTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);
    stbi_image_free(data);
    
    // After other texture loading
    data = stbi_load("assets/bird1.png", &width, &height, &nrChannels, 0);
    if (data) {
        glGenTextures(1, &birdTexture1);
        glBindTexture(GL_TEXTURE_2D, birdTexture1);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
        printf("Bird texture loaded successfully: %dx%d\n", width, height);
        stbi_image_free(data);
    } else {
        printf("Failed to load bird texture\n");
    }

    // Load second bird texture
    data = stbi_load("assets/bird2.png", &width, &height, &nrChannels, 0);
    if (data) {
        glGenTextures(1, &birdTexture2);
        glBindTexture(GL_TEXTURE_2D, birdTexture2);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
        printf("Bird texture 2 loaded successfully: %dx%d\n", width, height);
        stbi_image_free(data);
    } else {
        printf("Failed to load bird texture 2\n");
    }

    // Initialize bird VAO/VBO/EBO
    glGenVertexArrays(1, &birdVAO);
    glGenBuffers(1, &birdVBO);
    glGenBuffers(1, &birdEBO);

    glBindVertexArray(birdVAO);
    glBindBuffer(GL_ARRAY_BUFFER, birdVBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);  // Reuse the same EBO as other objects

    // Set up vertex attributes for bird
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // Initialize bird system
    initBirds();

    // Convert pixel coordinates (300, 10) to OpenGL coordinates
    float cloudX = (300.0f / WINDOW_WIDTH) * 2.0f - 1.0f;  // Convert 300 to OpenGL coords
    float cloudY = 1.0f - (10.0f / WINDOW_HEIGHT) * 2.0f;  // Convert 10 to OpenGL coords
    float cloudWidth = (50.0f / WINDOW_WIDTH) * 2.0f;   // Convert 50px to OpenGL coords
    float cloudHeight = (50.0f / WINDOW_HEIGHT) * 2.0f;  // Convert 50px to OpenGL coords

    float cloudVertices[] = {
        // Positions                      // Texture Coords
        cloudX,           cloudY,           0.0f, 0.0f,  // Top-left
        cloudX,           cloudY - cloudHeight, 0.0f, 1.0f,  // Bottom-left
        cloudX + cloudWidth, cloudY - cloudHeight, 1.0f, 1.0f,  // Bottom-right
        cloudX + cloudWidth, cloudY,           1.0f, 0.0f   // Top-right
    };

    unsigned int cloudIndices[] = {
        0, 1, 2,   // First triangle
        2, 3, 0    // Second triangle
    };

    glGenVertexArrays(1, &cloudVAO);
    glGenBuffers(1, &cloudVBO);
    glGenBuffers(1, &cloudEBO);

    glBindVertexArray(cloudVAO);

    glBindBuffer(GL_ARRAY_BUFFER, cloudVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cloudVertices), cloudVertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cloudEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(cloudIndices), cloudIndices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // After setting up the first cloud's buffers, add these two new cloud setups:
    // Cloud 2 at (200, 50)
    float cloud2X = (200.0f / WINDOW_WIDTH) * 2.0f - 1.0f;
    float cloud2Y = 1.0f - (50.0f / WINDOW_HEIGHT) * 2.0f;

    float cloud2Vertices[] = {
        cloud2X,           cloud2Y,           0.0f, 0.0f,
        cloud2X,           cloud2Y - cloudHeight, 0.0f, 1.0f,
        cloud2X + cloudWidth, cloud2Y - cloudHeight, 1.0f, 1.0f,
        cloud2X + cloudWidth, cloud2Y,           1.0f, 0.0f
    };

    glGenVertexArrays(1, &cloudVAO2);
    glGenBuffers(1, &cloudVBO2);
    glGenBuffers(1, &cloudEBO2);

    glBindVertexArray(cloudVAO2);
    glBindBuffer(GL_ARRAY_BUFFER, cloudVBO2);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cloud2Vertices), cloud2Vertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cloudEBO2);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(cloudIndices), cloudIndices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // Cloud 3 at (150, 20)
    float cloud3X = (150.0f / WINDOW_WIDTH) * 2.0f - 1.0f;
    float cloud3Y = 1.0f - (20.0f / WINDOW_HEIGHT) * 2.0f;

    float cloud3Vertices[] = {
        cloud3X,           cloud3Y,           0.0f, 0.0f,
        cloud3X,           cloud3Y - cloudHeight, 0.0f, 1.0f,
        cloud3X + cloudWidth, cloud3Y - cloudHeight, 1.0f, 1.0f,
        cloud3X + cloudWidth, cloud3Y,           1.0f, 0.0f
    };

    glGenVertexArrays(1, &cloudVAO3);
    glGenBuffers(1, &cloudVBO3);
    glGenBuffers(1, &cloudEBO3);

    glBindVertexArray(cloudVAO3);
    glBindBuffer(GL_ARRAY_BUFFER, cloudVBO3);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cloud3Vertices), cloud3Vertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cloudEBO3);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(cloudIndices), cloudIndices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // Main loop
    while (!glfwWindowShouldClose(window)) {
        glUseProgram(shaderProgram);  // Move shader binding here
        // Check for crouch input
        isCrouching = (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS || 
                      glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS);

        // Update physics
        if (!isGameOver) {
            if (isJumping && isCrouching) {
                velocityY = -0.2f;  // Fast fall when crouching mid-jump
            } else {
                velocityY += gravity;
            }
            characterY += velocityY;

            // Ground collision
            if (characterY <= -1.0f) {
                characterY = -1.0f;
                velocityY = 0;
                isJumping = false;
            }

            // Check for collision
            if (checkCollision()) {
                isGameOver = true;
                printf("Game Over!\n");
            }
        }

        // Update vertex positions with dynamic height
        float currentHeight = isCrouching ? crouchHeight : normalHeight;
        float vertices[] = {
            // Positions                           // Texture Coords
            characterX,         characterY + currentHeight, 0.0f, 0.0f,  // Top-left
            characterX,         characterY,                 0.0f, 1.0f,  // Bottom-left
            characterX + 0.167f, characterY,                1.0f, 1.0f,  // Bottom-right
            characterX + 0.167f, characterY + currentHeight,1.0f, 0.0f   // Top-right
        };

        // Update VBO with new positions
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

        // Update cloud positions with direction changes
        if (!isGameOver) {  // Only update cloud positions if game is not over
            if (cloud1MovingLeft) {
                cloud1X -= cloud1Speed;
                if (cloud1X <= 100.0f) cloud1MovingLeft = false;
            } else {
                cloud1X += cloud1Speed;
                if (cloud1X >= 500.0f) cloud1MovingLeft = true;
            }

            if (cloud2MovingLeft) {
                cloud2X -= cloud2Speed;
                if (cloud2X <= 100.0f) cloud2MovingLeft = false;
            } else {
                cloud2X += cloud2Speed;
                if (cloud2X >= 500.0f) cloud2MovingLeft = true;
            }

            if (cloud3MovingLeft) {
                cloud3X -= cloud3Speed;
                if (cloud3X <= 100.0f) cloud3MovingLeft = false;
            } else {
                cloud3X += cloud3Speed;
                if (cloud3X >= 500.0f) cloud3MovingLeft = true;
            }
        }

        // Convert pixel coordinates to OpenGL coordinates for each cloud
        cloudWidth = (50.0f / WINDOW_WIDTH) * 2.0f;   // Changed from 30.0f to 50.0f
        cloudHeight = (50.0f / WINDOW_HEIGHT) * 2.0f;  // Changed from 30.0f to 50.0f

        // Update cloud 1 vertices
        cloudX = (cloud1X / WINDOW_WIDTH) * 2.0f - 1.0f;
        cloudY = 1.0f - (10.0f / WINDOW_HEIGHT) * 2.0f;
        float cloud1Vertices[] = {
            cloudX,           cloudY,           0.0f, 0.0f,
            cloudX,           cloudY - cloudHeight, 0.0f, 1.0f,
            cloudX + cloudWidth, cloudY - cloudHeight, 1.0f, 1.0f,
            cloudX + cloudWidth, cloudY,           1.0f, 0.0f
        };
        glBindBuffer(GL_ARRAY_BUFFER, cloudVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(cloud1Vertices), cloud1Vertices, GL_STATIC_DRAW);

        // Update cloud 2 vertices
        cloudX = (cloud2X / WINDOW_WIDTH) * 2.0f - 1.0f;
        cloudY = 1.0f - (50.0f / WINDOW_HEIGHT) * 2.0f;
        float cloud2Vertices[] = {
            cloudX,           cloudY,           0.0f, 0.0f,
            cloudX,           cloudY - cloudHeight, 0.0f, 1.0f,
            cloudX + cloudWidth, cloudY - cloudHeight, 1.0f, 1.0f,
            cloudX + cloudWidth, cloudY,           1.0f, 0.0f
        };
        glBindBuffer(GL_ARRAY_BUFFER, cloudVBO2);
        glBufferData(GL_ARRAY_BUFFER, sizeof(cloud2Vertices), cloud2Vertices, GL_STATIC_DRAW);

        // Update cloud 3 vertices
        cloudX = (cloud3X / WINDOW_WIDTH) * 2.0f - 1.0f;
        cloudY = 1.0f - (20.0f / WINDOW_HEIGHT) * 2.0f;
        float cloud3Vertices[] = {
            cloudX,           cloudY,           0.0f, 0.0f,
            cloudX,           cloudY - cloudHeight, 0.0f, 1.0f,
            cloudX + cloudWidth, cloudY - cloudHeight, 1.0f, 1.0f,
            cloudX + cloudWidth, cloudY,           1.0f, 0.0f
        };
        glBindBuffer(GL_ARRAY_BUFFER, cloudVBO3);
        glBufferData(GL_ARRAY_BUFFER, sizeof(cloud3Vertices), cloud3Vertices, GL_STATIC_DRAW);

        // Update cactus groups
        updateCactusGroups();

        // Update birds
        updateBirds();

        // Render
        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);  // White background
        glClear(GL_COLOR_BUFFER_BIT);

        // Draw all clouds first
        glBindTexture(GL_TEXTURE_2D, cloudTexture);
        
        glBindVertexArray(cloudVAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        
        glBindVertexArray(cloudVAO2);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        
        glBindVertexArray(cloudVAO3);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        // Then draw character with appropriate texture
        glBindVertexArray(VAO);
        if (isGameOver) {
            // Use first frame of animation when game is over
            if (isJumping) {
                glBindTexture(GL_TEXTURE_2D, jumpTexture);
            } else if (isCrouching) {
                glBindTexture(GL_TEXTURE_2D, crouchTexture1);
            } else {
                glBindTexture(GL_TEXTURE_2D, texture1);
            }
        } else {
            if (isJumping) {
                glBindTexture(GL_TEXTURE_2D, jumpTexture);
            } else if (isCrouching) {
                glBindTexture(GL_TEXTURE_2D, (frameCount/20) % 2 == 0 ? crouchTexture1 : crouchTexture2);
            } else {
                glBindTexture(GL_TEXTURE_2D, (frameCount/20) % 2 == 0 ? texture1 : texture2);
            }
        }
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        // Render cactus groups
        renderCactusGroups();

        // Render birds
        renderBirds();

        if (!isGameOver) {
            frameCount++;  // Only increment frame counter if game is not over
            birdFrameCount++;  // Only increment bird frame counter if game is not over
        }
        
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    glDeleteProgram(shaderProgram);
    glDeleteTextures(1, &texture1);
    glDeleteTextures(1, &texture2);
    glDeleteTextures(1, &crouchTexture1);
    glDeleteTextures(1, &crouchTexture2);
    glDeleteTextures(1, &jumpTexture);
    glDeleteVertexArrays(1, &cloudVAO);
    glDeleteBuffers(1, &cloudVBO);
    glDeleteBuffers(1, &cloudEBO);
    glDeleteVertexArrays(1, &cloudVAO2);
    glDeleteBuffers(1, &cloudVBO2);
    glDeleteBuffers(1, &cloudEBO2);
    glDeleteVertexArrays(1, &cloudVAO3);
    glDeleteBuffers(1, &cloudVBO3);
    glDeleteBuffers(1, &cloudEBO3);
    glDeleteVertexArrays(1, &cactusVAO);
    glDeleteBuffers(1, &cactusVBO);
    glDeleteBuffers(1, &cactusEBO);
    glDeleteVertexArrays(1, &birdVAO);
    glDeleteBuffers(1, &birdVBO);
    glDeleteBuffers(1, &birdEBO);
    glDeleteTextures(1, &birdTexture1);
    glDeleteTextures(1, &birdTexture2);
    glfwTerminate();

    return 0;
}
