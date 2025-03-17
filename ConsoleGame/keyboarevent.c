#include <pthread.h> // Multithreading
#include <stdio.h>
#include <stdlib.h>  // for atexit()
#include <termios.h> // For changing terminal mode
#include <unistd.h>  // For changing terminal mode

struct termios original; // A struct to save the original state of terminal
int ESCPressed = 0;      // For thread communication

void disableRAWMode();
void enableRAWMode();
void *asciRead();
void *print();

int main() {
  // Start Multithreading
  pthread_t id_print, id_read;

  pthread_create(&id_print, NULL, print, NULL);
  pthread_create(&id_read, NULL, asciRead, NULL);

  pthread_join(id_print, NULL);
  pthread_join(id_read, NULL);

  return 0;
}

/// Reads keyboard input
void *asciRead() {
  enableRAWMode(); // local function: Enable Raw Mode
  char ch;
  while ((ch = getchar()) != 27)
    ; // ASCI code for ESC is 27
  ESCPressed = 1;
  printf("ESC Pressed!\n");
}

/// Doing Stuff while listening to keyboard
void *print() {
  while (!ESCPressed) { // When ESC is not pressed
    sleep(1);
    printf("I am Printing!\n");
  }
  printf("Printing Thread Finished!\n");
}

/// This function enables RAW mode for terminal.
void enableRAWMode() {
  struct termios raw;
  tcgetattr(STDIN_FILENO, &raw); // Save the state of the terminal to struct raw
                                 // STDIN_FILENO is from <stdlib.h>
                                 // tcgetattr() from <termios.h>
  tcgetattr(STDIN_FILENO, &original);
  atexit(&disableRAWMode); // Revert to canonical mode when exiting the program
                           // atext() from <stdlib.h>
  raw.c_lflag &=
      ~(ECHO | ICANON); // Turn off canonical mode
                        // Turn off ECHO mode so that keyboard is not
                        // printing to terminal
                        // ICANON and ECHO is bitflag. ~ is binary NOT operator

  tcsetattr(STDIN_FILENO, TCSAFLUSH,
            &raw); // Set the terminal to be in raw mode
                   // tcsetattr() from <termios.h>
}

void disableRAWMode() {
  tcsetattr(STDIN_FILENO, TCSAFLUSH,
            &original); // Set terminal to original state
}