#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <pthread.h>
#include <termios.h>

// Constants
#define GROUND_LEVEL 5
#define JUMP_HEIGHT 3
#define SCREEN_WIDTH 50
#define OBSTACLE_SPEED 100000 // Initial speed of obstacles

// Global variables
static int score = 0;
int ESCPressed = 0; // For ESC key detection
int SPACEPressed = 0; // For Spacebar detection
pthread_t id_game, id_input; // Thread IDs

// Structs
typedef struct {
    int xPos;
    int yPos;
} dino;

typedef struct {
    int xPos;
    int yPos;
} obstacle;

// Function prototypes
void disableRAWMode();
void enableRAWMode();
void *inputThread();
void *gameThread();
void draw_dino(dino dino, obstacle obstacle);
void draw_game_over();
bool collision_detection(dino dino, obstacle obstacle);

// Terminal settings
struct termios original;

// Initialize Dino
dino init_dino() {
    dino dino = {5, GROUND_LEVEL}; // Start at x=5, y=GROUND_LEVEL
    return dino;
}

// Initialize Obstacle
obstacle init_obstacle() {
    obstacle obstacle = {SCREEN_WIDTH, GROUND_LEVEL}; // Start at the far right
    return obstacle;
}

// Print spaces
void give_space(int space_count) {
    for (int i = 0; i < space_count; i++) {
        printf(" ");
    }
}

// Enable raw mode for terminal input
void enableRAWMode() {
    struct termios raw;
    tcgetattr(STDIN_FILENO, &raw);
    tcgetattr(STDIN_FILENO, &original);
    atexit(&disableRAWMode);
    raw.c_lflag &= ~(ECHO | ICANON); // Disable echo and canonical mode
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

// Disable raw mode and restore original terminal settings
void disableRAWMode() {
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &original);
}

// Input thread to detect key presses
void *inputThread() {
    enableRAWMode();
    char ch;
    while (1) {
        ch = getchar();
        if (ch == 27) { // ESC key
            ESCPressed = 1;
            printf("\nESC Pressed! Exiting...\n");
            pthread_cancel(id_game);
            break;
        } else if (ch == ' ') { // Spacebar
            SPACEPressed = 1;
        }
    }
    pthread_exit(NULL);
}

// Draw Dino and Obstacle
void draw_dino(dino dino, obstacle obstacle) {
    system("clear"); // Clear the console

    // Draw ground
    for (int i = 0; i < SCREEN_WIDTH; i++) {
        printf("-");
    }
    printf("\n");

    // Draw Dino
    for (int i = 0; i < dino.yPos; i++) {
        printf("\n");
    }
    give_space(dino.xPos);
    printf("T-Rex\n");

    // Draw Obstacle
    for (int i = 0; i < obstacle.yPos; i++) {
        printf("\n");
    }
    give_space(obstacle.xPos);
    printf("|||\n"); // Simple cactus representation

    // Display score
    printf("\nScore: %d\n", score);
}

// Draw Game Over screen
void draw_game_over() {
    system("clear");
    printf("\n\n\n\n\n");
    printf("  GAME OVER!\n");
    printf("  Your Score: %d\n", score);
    printf("\n\nPress ESC to exit...\n");
}

// Collision detection
bool collision_detection(dino dino, obstacle obstacle) {
    if (dino.xPos + 5 >= obstacle.xPos && dino.yPos == obstacle.yPos) {
        return true; // Collision detected
    }
    return false;
}

// Main game loop
void *gameThread() {
    dino dino = init_dino();
    obstacle obstacle = init_obstacle();
    bool isJumping = false;
    int jumpCounter = 0;

    while (1) {
        if (ESCPressed) {
            break; // Exit if ESC is pressed
        }

        // Collision detection
        if (collision_detection(dino, obstacle)) {
            draw_game_over();
            break;
        }

        // Handle jumping
        if (SPACEPressed && !isJumping) {
            isJumping = true;
            jumpCounter = JUMP_HEIGHT;
            SPACEPressed = 0;
        }

        if (isJumping) {
            dino.yPos--;
            jumpCounter--;
            if (jumpCounter == 0) {
                isJumping = false;
            }
        } else if (dino.yPos < GROUND_LEVEL) {
            dino.yPos++; // Fall back to ground
        }

        // Move obstacle
        obstacle.xPos--;
        if (obstacle.xPos < 0) {
            obstacle = init_obstacle(); // Reset obstacle
            score++; // Increase score
        }

        // Draw the game
        draw_dino(dino, obstacle);

        // Adjust game speed
        usleep(OBSTACLE_SPEED - (score * 1000));
    }

    pthread_exit(NULL);
}

// Main function
int main() {
    enableRAWMode(); // Enable raw mode for terminal input

    // Create threads for game and input handling
    pthread_create(&id_game, NULL, gameThread, NULL);
    pthread_create(&id_input, NULL, inputThread, NULL);

    // Wait for game thread to finish
    pthread_join(id_game, NULL);

    return 0;
}