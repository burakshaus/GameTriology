#include <stdio.h>  
#include <stdlib.h>  
#include <termios.h>  
#include <unistd.h>  
#include <fcntl.h>  
#include <time.h>  
// Function prototypes  
int kbhit(void);  
void moveCursorTo(int x, int y);  
void wait(unsigned int milliseconds);  
void displayInfo();  
void displayDino(int jumpType);  
void showObstacle();  
// Global variables  
int Height = 0;  
int Speed = 40;  
int main() // Main function  
{  
    // Set console mode for displaying the game  
    system("mode con: lines=35 cols=90");  
    char input;  
    // Display game information  
    displayInfo();  
    // Game loop  
    while (1)  
    {  
        // Continuous display of character and obstacle until a key is pressed  
        while (!kbhit())  
        {  
            displayDino(0);  
            showObstacle();  
        }  
        // Handle user input  
        input = getc;  
        // Perform actions based on user input  
        if (input == ' ')  
        {  
            // Jump animation when the space key is pressed  
            for (int x = 0; x <= 12; x++)  
            {  
                displayDino(1);  
                showObstacle();  
            }  
            for (int x = 0; x < 10; x++)  
            {  
                displayDino(2);  
                showObstacle();  
            }  
        }  
        else if (input == 'x' || input == 'X')  
        {  
            // Exit the game if the 'X' key is pressed  
            return 0;  
        }  
    }  
    return 0;  
}  
// Function to set the console cursor position  
void moveCursorTo(int x, int y)  
{  
    printf("\033[%d;%dH", y, x); // ANSI escape sequence for moving the cursor  
}  
// Function to pause execution for a given number of milliseconds  
void wait(unsigned int milliseconds)  
{  
    clock_t goal = milliseconds + clock();  
    while (goal > clock())  
        ;  
}  
// Function to display game information on the console  
void displayInfo()  
{  
    system("cls"); // Clear the console screen  
    moveCursorTo(6, 2);  
    printf("Press X to Exit, Press Space to Jump");  
    moveCursorTo(50, 3);  
    printf("SCORE IS: ");  
    moveCursorTo(1, 30);  
    for (int i = 0; i < 79; i++)  
        printf("-");  
}  
// Function to display the character on the console  
void displayDino(int jumpType)  
{  
    static int animeStat = 1;  
    // Update the jump height based on the jump type  
    if (jumpType == 0)  
        Height = 0;  
    else if (jumpType == 1)  
        Height--;  
    else  
        Height++;  
    // Display the character at the specified position  
    moveCursorTo(2, 11 - Height);  
    printf("               ");  
    moveCursorTo(2, 12 - Height);  
    printf("     ^^^^^     ");  
    moveCursorTo(2, 13 - Height);  
    printf("  ^^^^^^^^^    ");  
    moveCursorTo(2, 14 - Height);  
    printf(" ^^^^^^^^^^^   ");  
    moveCursorTo(2, 15 - Height);  
    printf(" ^^^^^^^^^^^^  ");  
    moveCursorTo(2, 16 - Height);  
    printf(" ^^^^^^^^^^^^^ ");  
    moveCursorTo(2, 17 - Height);  
    printf(" ^^^^^^^^^^^^  ");  
    moveCursorTo(2, 18 - Height);  
    printf("  ^^^^^^^^^^^  ");  
    moveCursorTo(2, 19 - Height);  
    if (jumpType == 1 || jumpType == 2)  
    {  
        printf("    ^^ ^^^^^   ");  
        moveCursorTo(2, 20 - Height);  
        printf("    ^^ ^^      ");  
    }  
    else if (animeStat == 1)  
    {  
        printf("   ^^^^ ^^^    ");  
        moveCursorTo(2, 20 - Height);  
        printf("       ^^      ");  
        animeStat = 2;  
    }  
    else if (animeStat == 2)  
    {  
        printf("   ^^^ ^^      ");  
        moveCursorTo(2, 20 - Height);  
        printf("        ^^     ");  
        animeStat = 1;  
    }  
    moveCursorTo(2, 21 - Height);  
    if (jumpType != 0)  
    {  
        printf("               ");  
    }  
    else  
    {  
        printf("^^^^^^^^^^^^^^^");  
    }  
    wait(Speed);  
}  
// Function to display the obstacle on the console  
void showObstacle()  
{  
    static int obstaclePos = 0;  
    static int score = 0;  
  
    // Check for collision with the obstacle  
    if (obstaclePos == 50 && Height < 5)  
    {  
        score = 0;  
        Speed = 50;  
        moveCursorTo(40, 9);  
        printf("Game Finish");  
        getchar(); // Read the extra newline character from the buffer  
        moveCursorTo(39, 8);  
        printf("              ");  
    }  
    // Display the obstacle at the specified position  
    moveCursorTo(74 - obstaclePos, 25);  
    printf("^ ^ ");  
    obstaclePos++;  
    // Update obstacle position and score  
    if (obstaclePos == 75)  
    {  
        obstaclePos = 0;  
        score++;  
        moveCursorTo(75, 2);  
        printf("  ");  
        moveCursorTo(75, 2);  
        printf("%d", score);  
        if (Speed > 25)  
            Speed--;  
    }  
}  
// Function to detect keyboard input  
int kbhit(void)  
{  
    struct termios oldt, newt;  
    int ch;  
    int oldf;  
    tcgetattr(STDIN_FILENO, &oldt);        // Get the current terminal I/O structure  
    newt = oldt;  
    newt.c_lflag &= ~(ICANON | ECHO);      // Disable canonical mode and echo mode  
    tcsetattr(STDIN_FILENO, TCSANOW, &newt); // Set the new terminal I/O structure  
    oldf = fcntl(STDIN_FILENO, F_GETFL, 0); // Get the file status flags  
    fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK); // Change the file status flags to non-blocking mode  
    ch = getchar(); // Get a character from the input stream  
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt); // Restore the old terminal I/O structure  
    fcntl(STDIN_FILENO, F_SETFL, oldf);      // Restore the old file status flags  
    if (ch != EOF)  
    {  
        ungetc(ch, stdin); // Push the character back onto the input stream  
        return 1;          // Return 1 if a character is available  
    }  
    return 0; // Return 0 if no character is available  
}  