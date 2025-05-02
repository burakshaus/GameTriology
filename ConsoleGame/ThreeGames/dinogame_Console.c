#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <curses.h>
#include <stdbool.h>
#include <time.h>
#include <pthread.h> // Multithreading
#include <termios.h>

// for windows enviroment #include <Windows.h>
// gcc -o example  dinogame_Console.c -pthread -lncurses


#define BELOWSPACE 5
#define SPACEMOVING 5
#define MAXSTARHEIGHT 3
#define JUMPSCALE 3
#define MAXSTARWIDTH 2
#define MAXLENGHTOFSCREEN 50
#define MAXSTARCOUNTCREATED 5

static int star_count = 0;  // Global static variable
static int sleepMilisecond = 50000;   // Global static sleep arg variable to change the speed of the game
static int score = 0;

struct termios original; // A struct to save the original state of terminal
int ESCPressed = 0;      // For thread communication
int SPACEPressed = 0;
int COLLISIONBit = 0;
pthread_t id_game, id_ESC, id_SPACE;  // Declare global thread IDs


void disableRAWMode();
void enableRAWMode();
void *asciRead();
void *print();


typedef struct
{
  int rArmXPos;
  int rArmYpos;
  int rLegXPos;
  int rLegYPos;
  
}human;

typedef struct  
{
  int layer;
  int xPos;
  int yPos;
  int generated_xPos;
}star;

star init_star(){
  system("clear");
  star star = {0};
  star_count += 1;
  star.layer = 1;
  star.generated_xPos = MAXLENGHTOFSCREEN+3;
  star.xPos = star.generated_xPos+2;
  star.yPos = star.layer;

  return star;
}

human init_human() {
    human testHuman = {2,2,2,2};  // Initialize all fields to 2
    return testHuman;
}

void *inputThread() {
    enableRAWMode();  // Enable raw mode for key detection
    char ch;
    while (1) {
        ch = getchar();
        if (ch == 27) {  // ESC key
            ESCPressed = 1;
            printf("\nESC Pressed! Exiting...\n");
            pthread_cancel(id_game);
            break;
        } else if (ch == ' ') {  // Space key
            SPACEPressed = 1;
        }
    }

    pthread_exit(NULL);
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

void below_space(int Space){
  for (int i = 0; i < Space; i++)
  {
    printf("\n");
  }
  
}

void draw_star(){
  printf("*\n");
}

void draw_floor(){
  for (int i = 0; i < MAXLENGHTOFSCREEN+6; i++)
  {
    printf("^");
  }
  printf("\n");
}

void draw_first_line(){
  printf(" O\n");
}

void draw_second_line(){
  printf("/|\\\n");
}

void give_space(int space_count){
  for (int i = 0; i < space_count; i++)
  {
    printf(" ");
  }
  
}

void give_space_at_1(int pos){
  switch (pos)
  {
  case 1:
    printf("  *\n");
    break;
  case 0:
    printf(" *\n");
    break;
  case -1:
    printf("*\n");
  default:
    break;
  }
  

}

void give_space_floor(int space_count){  
  for (int i = 0; i < space_count; i++)
  {
    printf("^");
  }
  for (int i = 0; i < MAXLENGHTOFSCREEN - space_count; i++)
  {
    printf("^");
  }
  
}

void draw_human(star star){
  below_space(BELOWSPACE);
  draw_first_line();
  switch (star.layer)
  {
  case  3:
    printf("/|\\ ");
    give_space(star.xPos - 2);
    draw_star();
    printf(" |  ");
    give_space(star.xPos - 2);
    draw_star();
    printf("/ \\ ");
    give_space(star.xPos - 2);
    draw_star();
    draw_floor();
    break;
  case 2:
    printf("/|\\ \n");
    printf(" | ");
    give_space(star.xPos - 2);
    draw_star();
    printf("/ \\");
    give_space(star.xPos - 2);
    draw_star();
    draw_floor();
    break;
  case 1:
    printf("/|\\ \n");
    printf(" | \n");
    printf("/ \\ ");
    give_space(star.xPos-2);
    draw_star();
    draw_floor();
    break;
  default:
    break;
  }
  printf("\n");
}

void draw_human_collision(star star){
  below_space(BELOWSPACE);
  draw_first_line();
  switch (star.layer)
  {
  case  3:
    printf("/|\\*\n");
    printf(" | *\n");
    printf("/ \\*\n");
    draw_floor();
    break;
  case 2:
    printf("/|\\ \n");
    printf(" | *\n");
    printf("/ \\*\n");
    draw_floor();
    break;
  case 1:
    printf("/|\\ \n");
    printf(" | \n");
    printf("/ \\*\n");
    // give_space(star.xPos-2);
    // draw_star();
    draw_floor();
    break;
  default:
    break;
  }
  printf("\n");
}

bool collision_detection(star star, human human){
  switch (star.layer)
  {
  case 1:
    if (human.rLegXPos == star.xPos)
  {
    return true;
  }
    break;
  
  default:
    break;
  }
  return false;
  
}

void draw_jumping(star star, human human){
  for (int i = 0; i <= 2; i++)
  {
    switch (i)
    {
    case 0:
    if (star.xPos<4)
    {
      system("clear");
      printf("human leg x pos %d and stars x pos %d\n",human.rLegXPos, star.xPos);    
      below_space(4);
      draw_first_line();
      switch (star.layer)
      {
      case 1:
        printf("/|\\ \n");
        printf(" | \n");
        printf("/ \\\n");
        give_space_at_1(1);
        draw_star();
        draw_floor();
        human.rArmYpos+=1;
        human.rLegYPos+=1;
        star.xPos-=1;
        collision_detection(star,human);
        usleep(sleepMilisecond - (100 * score));
        break;
      case 2:
        break;
      case 3: 
        break;
      default:
        break;
      }
    }
    else
    {
      system("clear");
      printf("human leg x pos %d and stars x pos %d\n",human.rLegXPos, star.xPos);    
      below_space(4);
      draw_first_line();
      switch (star.layer)
      {
      case 1:
        printf("/|\\ \n");
        printf(" | \n");
        printf("/ \\\n");
        give_space(star.xPos+2);
        draw_star();
        draw_floor();
        human.rArmYpos+=1;
        human.rLegYPos+=1;
        star.xPos-=1;
        collision_detection(star,human);
        usleep(sleepMilisecond - (100 * score));
        break;
      case 2:
        break;
      case 3: 
        break;
      default:
        break;
      }
    }
      break;
    case 1:
        if (star.xPos<4)
    {
      system("clear");
      printf("human leg x pos %d and stars x pos %d\n",human.rLegXPos, star.xPos);    
      below_space(4);
      draw_first_line();
      switch (star.layer)
      {
      case 1:
        printf("/|\\ \n");
        printf(" | \n");
        printf("/ \\\n");
        printf("\n");
        give_space_at_1(0);
        draw_star();
        draw_floor();
        human.rArmYpos+=1;
        human.rLegYPos+=1;
        star.xPos-=1;
        collision_detection(star,human);
        usleep(sleepMilisecond - (100 * score));
        break;
      case 2:
        break;
      case 3: 
        break;
      default:
        break;
      }
    }
    else
    {
            system("clear");
      printf("human leg x pos %d and stars x pos %d\n",human.rLegXPos, star.xPos);    
      below_space(3);
      draw_first_line();
      switch (star.layer)
      {
      case 1:
        printf("/|\\ \n");
        printf(" | \n");
        printf("/ \\\n");
        printf("\n");
        give_space(star.xPos+2);
        draw_star();
        draw_floor();
        human.rArmYpos+=1;
        human.rLegYPos+=1;
        star.xPos-=1;
        collision_detection(star,human);
        usleep(sleepMilisecond - (100 * score));
        break;
      default:
        break;
      }
    }
    
      break;
    case 2:
    if (star.xPos<4)
    {
      system("clear");
      printf("human leg x pos %d and stars x pos %d\n",human.rLegXPos, star.xPos);    
      below_space(4);
      draw_first_line();
      switch (star.layer)
      {
      case 1:
        printf("/|\\ \n");
        printf(" | \n");
        printf("/ \\\n");
        printf("\n\n");
        give_space_at_1(-1);
        draw_star();
        draw_floor();
        human.rArmYpos+=1;
        human.rLegYPos+=1;
        star.xPos-=1;
        collision_detection(star,human);
        usleep(sleepMilisecond - (100 * score));
        break;
      case 2:
        break;
      case 3: 
        break;
      default:
        break;
      }
    }
    else
    {
       system("clear");
      printf("human leg x pos %d and stars x pos %d\n",human.rLegXPos, star.xPos);    
      // usleep(sleepMilisecond - (100 * score));
      below_space(2);
      draw_first_line();
      switch (star.layer)
      {
      case 1:
        printf("/|\\ \n");
        printf(" | \n");
        printf("/ \\\n");
        printf("\n\n");
        give_space(star.xPos+2);
        draw_star();
        draw_floor();
        human.rArmYpos+=1;
        human.rLegYPos+=1;
        star.xPos-=1;
        collision_detection(star,human);
        SPACEPressed = 0;
        //usleep(sleepMilisecond - (100 * score));
        break;
      default:
        break;
      }
    }
      break;
    default:
      break;
    }
  }
  printf("\n");
  
}

void draw_falling(star star, human human){
  for (int i = 0; i < 2; i++)
  {
    switch (i)
    {
    case 0:
      if (star.xPos<4)
      {
        switch (star.xPos)
        {
        case 3:
            system("clear");
            printf("human leg x pos %d and stars x pos %d\n",human.rLegXPos, star.xPos);    
            // usleep(sleepMilisecond - (100 * score));
            below_space(2);
            draw_first_line();
            switch (star.layer)
            {
            case 1:
              printf("/|\\ \n");
              printf(" | \n");
              printf("/ \\\n");
              printf("\n");
              give_space(star.xPos+2);
              draw_star();
              draw_floor();
              human.rArmYpos-=1;
              human.rLegYPos-=1;
              star.xPos-=1;
              collision_detection(star,human);
              usleep(sleepMilisecond - (100 * score));  
              break;
            case 2:
            break;
            case 3:
            break;
            default:
              break;
            }
          break;
        case 2:
          break;
        case 1:
          break;
        case 0:
          break;
        case -1:
          break;
        default:
          break;
        }
      }
      else {
      system("clear");
      printf("human leg x pos %d and stars x pos %d\n",human.rLegXPos, star.xPos);    
      // usleep(sleepMilisecond - (100 * score));
      below_space(3);
      draw_first_line();
      switch (star.layer)
      {
      case 1:
        printf("/|\\ \n");
        printf(" | \n");
        printf("/ \\\n");
        printf("\n");
        give_space(star.xPos+2);
        draw_star();
        draw_floor();
        human.rArmYpos+=1;
        human.rLegYPos+=1;
        star.xPos-=1;
        collision_detection(star,human);
        SPACEPressed = 0;
        //usleep(sleepMilisecond - (100 * score));
        break;
      default:
        break;
      }
      }
      
      break;
    case 1:
      if (star.layer<=4)
      {
        /* code */
      }
      else {
      system("clear");
      printf("human leg x pos %d and stars x pos %d\n",human.rLegXPos, star.xPos);    
      // usleep(sleepMilisecond - (100 * score));
      below_space(4);
      draw_first_line();
      switch (star.layer)
      {
      case 1:
        printf("/|\\ \n");
        printf(" | \n");
        printf("/ \\\n");
        give_space(star.xPos+2);
        draw_star();
        draw_floor();
        human.rArmYpos+=1;
        human.rLegYPos+=1;
        star.xPos-=1;
        collision_detection(star,human);
        SPACEPressed = 0;
        //usleep(sleepMilisecond - (100 * score));
        break;
      case 2:
        
        break;
      default:
        break;
      }
      }
      
      break;
    
      break;
    default:
      break;
    }
  }
  
}

void clear_console(){
  system("clear");
}

void print_star_information(star star){
  printf("star's count : %d\n",star.layer);
  printf("star's xpos : %d\n",star.xPos);
  printf("star's ypos : %d\n", star.yPos);
}

void *init_game2() {
  system("clear");
  bool running2 = true;
  star testStar = init_star();
  human human = init_human();
  bool collision = false;

  while (running2) {
    //printf("human leg x pos %d and stars x pos %d\n",human.rLegXPos, testStar.xPos);    
    if (collision_detection(testStar,human))
    {
      draw_human_collision(testStar);
      running2 = false;
      collision = true; 
      break;
    }
    else {

    }
    if (SPACEPressed==1) {
      draw_jumping(testStar,human);
      usleep(sleepMilisecond - (100 * score));
      testStar.xPos-=3;
      draw_falling(testStar, human);
      usleep(sleepMilisecond - (100 * score));
      testStar.xPos -= 2; 
    }
    else {
      human = init_human();
      draw_human(testStar);
      usleep(sleepMilisecond - (100 * score));
      testStar.xPos -= 1;
    }

    if (testStar.xPos <= -2) {
      testStar = init_star();
      score += 1;
    }

    system("clear");
  }
  
  if (collision)
  {
    printf("Game Over! %d\n", score);
    pthread_exit(NULL);
  }

  printf("BRAVO! Your score: %d\n", score);
  pthread_exit(NULL);
}

int main() {
    enableRAWMode(); // Enable raw mode once

    pthread_create(&id_game, NULL, init_game2, NULL);
    pthread_create(&id_ESC, NULL, inputThread, NULL);  // Reuse id_ESC for input handling

    pthread_join(id_game, NULL); // Only wait for game thread

    return 0;
}
