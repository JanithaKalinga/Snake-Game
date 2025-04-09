#include "SPI.h"
#include "Adafruit_GFX.h"
#include "Adafruit_ILI9341.h"

// Define display pins
#define TFT_DC 9
#define TFT_CS 10
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC);

// Snake and apple positions
int position[100][2] = {{12, 16}, {12, 17}, {12, 18}};  // Start positions for the snake
int apple_position[2];  // Apple position
int score = 0;
int highscore = 0;
int speed = 300;
int element = 3;  // Initial snake length
int richtung = 1;  // 1=up, 2=right, 3=down, 4=left
int level = 1;
int barrier_position[2];
unsigned long food_start_time = 0;  // Track when the food appears
int food_display_duration = 9000;  // Time in milliseconds for food to stay on screen
bool food_visible = true;          // Food visibility flag
int red_food_position[2];   // Position of red food (increases points)
int blue_food_positions[10][2];  // Position of blue food (decreases points)
bool blue_food_active[10];       // Tracks if each blue food is active (true = present on the board)
int blue_food_count = 0;         // Current number of blue foods on the board
bool red_food_visible = false;  // Track visibility of red food
bool blue_food_visible = false;  // Track visibility of blue food



// Function prototypes (Declare all functions here)
void startscreen();
void game();
void draw_appleandsnake();
void draw_logo();
void draw_starttext();
void draw_design();
void draw_apple();
void spawn_apple();
void check_direction();
void step();
void draw_quadrat(int x_pos, int y_pos, int color);
void check_and_delay(int ms);
void check_apple();
void append_square();
void check_collision();
void draw_score();
void check_highscore();
void draw_level();
void place_barrier();
void erase_barrier();
void shift_snake_to_corner();
void gameover();
void check_food_timer();
void draw_countdown_timer();
void spawn_apple_upto_level2();
void spawn_apple_level3();
void spawn_apple_level4();
void reset_game();

void setup() {
  pinMode(2, INPUT_PULLUP);  // Joystick button input
  tft.begin();  // Initialize the display
}

void loop() {
  startscreen();  // Display start screen
  game();         // Start the game
}

// Start screen display
void startscreen() {
  tft.fillScreen(ILI9341_DARKGREEN);  // Green background
  draw_appleandsnake();  // Show the snake and apple
  draw_logo();           // Game logo
  draw_starttext();      // Instructions to start the game

  while (digitalRead(2) == HIGH) {}  // Wait for joystick press to start
}

// Main game function
void game() {
  draw_design();  // Draw game UI elements (score, border, etc.)
  if (level < 3) {
    spawn_apple_upto_level2();  // For Levels 1 and 2
  } 
  else if (level >= 3) {
    spawn_apple_level3();       // For Level 3
  } 
  else if(level >= 4){
    spawn_apple_level4();       // For Level 4 and beyond
  }

  // Draw initial snake
  for (int i = 0; i < element; i++) {
    draw_quadrat(position[i][0], position[i][1], ILI9341_BLACK);  // Draw snake elements
  }

  check_direction();  // Monitor direction and move snake
  

}

// Check current direction and move the snake accordingly
void check_direction() {
  while (true) {
    if (richtung == 1) {  // Move up
      step();
      position[0][1] -= 1;
    } else if (richtung == 2) {  // Move right
      step();
      position[0][0] += 1;
    } else if (richtung == 3) {  // Move down
      step();
      position[0][1] += 1;
    } else if (richtung == 4) {  // Move left
      step();
      position[0][0] -= 1;
    }

    // Screen wrapping logic
    if (position[0][0] > 22) position[0][0] = 1;  // Wrap right to left
    if (position[0][0] < 1) position[0][0] = 22;  // Wrap left to right
    if (position[0][1] > 30) position[0][1] = 3;  // Wrap down to top
    if (position[0][1] < 3) position[0][1] = 30;  // Wrap top to down

    draw_quadrat(position[0][0], position[0][1], ILI9341_BLACK);  // Draw head
    check_and_delay(speed);  // Delay and check for new joystick inputs
    if (level >= 3) {
      check_food_timer();      // Check if food should disappear
      draw_countdown_timer();  // Show countdown timer
    }
    
  }
}

// Move the entire snake
void step() {
  int last = element - 1;
  draw_quadrat(position[last][0], position[last][1], ILI9341_DARKGREEN);  // Erase the tail

  // Move body elements
  for (int i = last; i > 0; i--) {
    position[i][0] = position[i - 1][0];
    position[i][1] = position[i - 1][1];
  }

  check_apple();     // Check if snake eats an apple
  check_collision();  // Check for self-collision
}

// Check if snake eats the apple
void check_apple() {

  bool respawnNeeded = false;
  // Check if the snake's head is on the apple's position
  if (position[0][0] == apple_position[0] && position[0][1] == apple_position[1]) {
    score += 1;  // Increase the score
    draw_score();  // Update the score on the display
    append_square();  // Add a new square to the snake
    red_food_visible = false;  // Mark red food as not visible
    respawnNeeded = true; // Set flag to respawn food

    // Reset food timer since red food was eaten
    food_start_time = millis();

    // Level progression: Increase level every 2 points
    if (score % 2 == 0) {
      level++;  // Increase level after every 2 points
      draw_level();  // Update the display for the new level
    }

    if (level >= 2) {
      place_barrier();  // Place the barrier when level 2 is reached
      shift_snake_to_corner();  // Shift snake to a corner if it's near the center
    }

  }

  for (int i = 0; i < blue_food_count; i++) {
        if (blue_food_active[i] && position[0][0] == blue_food_positions[i][0] && position[0][1] == blue_food_positions[i][1]) {
            score -= 1;  // Decrease score for blue food
            draw_score();  // Update the score on the display
            blue_food_active[i] = false;  // Disable this blue food after being eaten
        }
    }
  if (respawnNeeded) {
    if (level < 3) {
      spawn_apple_upto_level2();  // Use normal apple spawn for Levels 1 and 2
    } else if (level == 3) {
      spawn_apple_level3();  // Use timed apple spawn for Level 3
    } else if (level >= 4) {
      spawn_apple_level4();  // Respawn both red and blue foods for Level 4 and above
    }
  }   
}


// Check for self-collision
void check_collision() {

  if (position[0][0] == barrier_position[0] && position[0][1] == barrier_position[1]) {
    gameover();
  }

  for (int i = 2; i < element; i++) {
    if (position[0][0] == position[i][0] && position[0][1] == position[i][1]) {
      gameover();
    }
  }
}

// Add a new square to the snake
void append_square() {
  position[element][0] = position[element - 1][0];
  position[element][1] = position[element - 1][1];
  element++;
}

// Delay and check for joystick input
void check_and_delay(int ms) { //checkt Joystickeingabe
  for (int i = 0; i < ms; i++) {
    delay(1);
    if (analogRead(A0) == 1023 and richtung != 3) {
      richtung = 1; //oben
    }
    if (analogRead(A1) == 0    and richtung != 4) {
      richtung = 2; //rechts
    }
    if (analogRead(A0) == 0    and richtung != 1) {
      richtung = 3; //unten
    }
    if (analogRead(A1) == 1023 and richtung != 2) {
      richtung = 4; //links
    }
  }
}

// Other functions remain unchanged: draw_quadrat, spawn_apple, draw_design, draw_score, check_highscore, etc.
void draw_quadrat(int x_pos, int y_pos, int color) {
  for (int i = 0; i < 9; i++) {
    for (int j = 0; j < 9; j++) {
      tft.drawPixel(x_pos * 10 + i, y_pos * 10 + j, color);
    }
  }
}

void spawn_apple() {
  bool validPosition = false;
  while (!validPosition) {
    int random_x = random(1, 23);   // Random x-coordinate
    int random_y = random(3, 31);   // Random y-coordinate
    validPosition = true;

    // Check if apple spawns on the snake or too close to the barrier
    for (int i = 0; i < element; i++) {
      if (position[i][0] == random_x && position[i][1] == random_y) {
        validPosition = false;  // Repeat if apple spawns on the snake
        break;
      }
    }
    if (validPosition) {
      apple_position[0] = random_x;
      apple_position[1] = random_y;
      draw_quadrat(random_x, random_y, ILI9341_RED);  // Draw apple

      food_start_time = millis();  // Reset the food timer
      food_visible = true;
    }
  }
}



// Define other functions like draw_appleandsnake(), draw_logo(), gameover(), reset_game(), etc.

void draw_design() { //design vom Spielbildschirm
  tft.fillScreen(ILI9341_DARKGREEN);      //grüner Hintergrund
  tft.drawRect(9, 29, 222, 282, ILI9341_BLACK); //Rahmen

  tft.setCursor(50, 5);
  tft.setTextColor(ILI9341_BLACK);
  tft.setTextSize(3);
  tft.println("SCORE: ");
  tft.setCursor(175, 5);
  tft.println(score);
  draw_apple();
}

void draw_score () { //resetet & zeichnet Score
  tft.setCursor(175, 5);
  tft.setTextSize(3);
  tft.setTextColor(ILI9341_DARKGREEN);
  tft.println(score - 1);

  tft.setCursor(175, 5);
  tft.setTextSize(3);
  tft.setTextColor(ILI9341_BLACK);
  tft.println(score);
}

void draw_appleandsnake() { //print Apfel/Schlange auf Titelscreen

  tft.fillRect(80, 30, 130, 10, ILI9341_ORANGE);
  tft.fillRect(200, 30, 10, 70, ILI9341_ORANGE);
  tft.fillRect(30, 90, 170, 10, ILI9341_ORANGE);
  tft.fillRect(30, 90, 10, 80, ILI9341_ORANGE);
  tft.fillRect(30, 170, 80, 10, ILI9341_ORANGE);

  tft.fillRect(104, 176, 2, 2, ILI9341_BLACK);
  tft.fillRect(104, 172, 2, 2, ILI9341_BLACK);
  tft.fillRect(110, 174, 4, 2, ILI9341_RED);

  tft.fillCircle(140, 174, 5, ILI9341_RED);
  tft.drawCircle(140, 174, 6, ILI9341_BLACK);
  tft.fillCircle(143, 165, 1, ILI9341_GREEN);
  tft.fillCircle(144, 165, 1, ILI9341_GREEN);
  tft.fillCircle(144, 166, 1, ILI9341_GREEN);
  tft.drawLine(140, 170, 143, 163, ILI9341_MAROON);
}

void draw_logo (){  //print Logo
  tft.setCursor(50, 52);
  tft.setTextColor(ILI9341_WHITE);
  tft.setTextSize(5);
  tft.println("SNAKE");
  tft.setCursor(65, 105);
  tft.println("GAME");
}

void draw_starttext(){  //print Text auf Startbildschirm 
  //print play
  tft.setCursor(20, 240);
  tft.setTextColor(ILI9341_LIGHTGREY);
  tft.setTextSize(2);
  tft.println("> PRESS TO PLAY <");

  //print credits
  tft.setCursor(40, 300);
  tft.setTextColor(ILI9341_WHITE);
  tft.setTextSize(1);
  tft.println("- created by Philipp & Max -");
}

void draw_apple() { //print Apfel
  tft.fillCircle(220, 17, 8, ILI9341_RED);
  tft.drawCircle(220, 17, 9, ILI9341_BLACK);
  tft.fillCircle(223, 8, 2, ILI9341_GREEN);
  tft.fillCircle(224, 8, 2, ILI9341_GREEN);
  tft.fillCircle(224, 9, 2, ILI9341_GREEN);
  tft.drawLine(220, 13, 223, 3, ILI9341_MAROON);
  tft.drawLine(219, 13, 222, 3, ILI9341_MAROON);
}

void check_highscore() { //Highscore aktuallisieren & printen
  if (score > highscore) {    //wenn neuer Score größer als Highscore -> neuer Highscore
    highscore = score;

    tft.setCursor(40, 230);           //print NEW HIGHSCORE
    tft.setTextColor(ILI9341_YELLOW);
    tft.setTextSize(2);
    tft.println("NEW HIGHSCORE!");
  }

  tft.setCursor(40, 5);           //print Highscore
  tft.setTextColor(ILI9341_YELLOW);
  tft.setTextSize(2);
  tft.println("HIGHSCORE:");
  tft.setCursor(180, 5);
  tft.println(highscore);
}

void gameover() { //Gameover Bildschirm
  tft.fillScreen(ILI9341_BLACK);    //print GAME OVER
  tft.setCursor(15, 140);
  tft.setTextColor(ILI9341_RED);
  tft.setTextSize(4);
  tft.println("GAME OVER");

  tft.setCursor(70, 200);           //print Score
  tft.setTextColor(ILI9341_WHITE);
  tft.setTextSize(2);
  tft.println("Score:");
  tft.setCursor(155, 200);
  tft.println(score);

  check_highscore();

  tft.setCursor(7, 300);           //print continue
  tft.setTextColor(ILI9341_DARKGREY);
  tft.setTextSize(2);
  tft.println("<press to continue>");

  while (digitalRead(2) == HIGH) {};
  reset_game();
}

void reset_game() { //setzt aktuelles Game zurück
  //Variablen reset
  richtung = 1;
  score = 0;
  speed = 300;
  element = 3;

  //Schlange reset
  for (int i = 0; i < 3; i++) {
    position[i][0] = 12;
    position[i][1] = 16 + i;
  }

  game(); //Neustart game
}

void place_barrier() {
  bool validPosition = false;
  
  // Erase the previous barrier if it exists
  erase_barrier();

  // Find a new valid random position for the barrier
  while (!validPosition) {
    barrier_position[0] = random(1, 23);
    barrier_position[1] = random(3, 31);
    validPosition = true;

    // Check if the barrier spawns on the snake
    for (int i = 0; i < element; i++) {
      if (position[i][0] == barrier_position[0] && position[i][1] == barrier_position[1]) {
        validPosition = false;
        break;
      }
    }

    // Check if the barrier is too close to the apple
    if (abs(barrier_position[0] - apple_position[0]) < 3 && abs(barrier_position[1] - apple_position[1]) < 3) {
      validPosition = false;
    }
  }

  // Draw the new barrier
  tft.setCursor(barrier_position[0] * 10, barrier_position[1] * 10);
  tft.setTextColor(ILI9341_RED);
  tft.setTextSize(3);
  tft.print("6");  // Display the barrier as the digit "9"
}


void draw_level() {
  tft.setCursor(5, 20);  // Position for level display
  tft.setTextColor(ILI9341_WHITE);
  tft.setTextSize(2);
  tft.print("Level: ");
  tft.print(level);
}

void shift_snake_to_corner() {
  // Define the center and corner coordinates
  int center_x = 16;
  int center_y = 20;
  int corner_x = 1;
  int corner_y = 3;

  if (position[0][0] > center_x - 5 && position[0][0] < center_x + 5 &&
      position[0][1] > center_y - 5 && position[0][1] < center_y + 5) {
    // Shift snake to a corner
    position[0][0] = corner_x;
    position[0][1] = corner_y;
    for (int i = 1; i < element; i++) {
      position[i][0] = corner_x;
      position[i][1] = corner_y + i;
    }
    draw_apple();  // Draw apple to ensure it does not overlap with the barrier
  }
}



void spawn_apple_upto_level2() {
  bool validPosition = false;
  
  while (!validPosition) {
    int random_x = random(1, 23);   // Random x-coordinate
    int random_y = random(3, 31);   // Random y-coordinate
    validPosition = true;

    // Check if apple spawns on the snake
    for (int i = 0; i < element; i++) {
      if (position[i][0] == random_x && position[i][1] == random_y) {
        validPosition = false;  // If the apple spawns on the snake, repeat
        break;
      }
    }

    // Check if the apple spawns too close to the barrier (in Level 2)
    if (level == 2 && abs(random_x - barrier_position[0]) < 3 && abs(random_y - barrier_position[1]) < 3) {
      validPosition = false;  // Ensure a margin around the barrier in Level 2
    }

    if (validPosition) {
      apple_position[0] = random_x;
      apple_position[1] = random_y;
      draw_quadrat(random_x, random_y, ILI9341_RED);  // Draw apple as red square
    }
  }
}

void spawn_apple_level3() {
  bool validPosition = false;

  while (!validPosition) {
    int random_x = random(1, 23);   // Random x-coordinate
    int random_y = random(3, 31);   // Random y-coordinate
    validPosition = true;

    // Check if apple spawns on the snake
    for (int i = 0; i < element; i++) {
      if (position[i][0] == random_x && position[i][1] == random_y) {
        validPosition = false;  // If the apple spawns on the snake, repeat
        break;
      }
    }

    // Check if apple is too close to the barrier
    if (abs(random_x - barrier_position[0]) < 3 && abs(random_y - barrier_position[1]) < 3) {
      validPosition = false;  // Ensure a margin around the barrier
    }

    if (validPosition) {
      apple_position[0] = random_x;
      apple_position[1] = random_y;
      draw_quadrat(random_x, random_y, ILI9341_RED);  // Draw apple as red square

      // Start the timer for apple visibility in Level 3
      food_start_time = millis();  // Record the current time
      food_visible = true;         // Set apple to visible
    }
  }
}


void check_food_timer() {
  unsigned long current_time = millis();

  if (food_visible && (current_time - food_start_time >= food_display_duration)) {
    // Apple has been on screen for 5 seconds, make it disappear
    draw_quadrat(apple_position[0], apple_position[1], ILI9341_DARKGREEN);  // Erase apple
    red_food_visible = false;  // Mark red food as not visible
    food_visible = false;  // Set apple to invisible

    // Respawn a new apple after a short delay
    delay(100);  // Optional delay between disappearing and respawning new apple
    if (level >= 4) {
      spawn_apple_level4();  // Spawn new red and blue food for Level 4
    } 
    else if (level == 3) {
      spawn_apple_level3();  // Spawn new apple for Level 3
    }
  }
}

void draw_countdown_timer() {
  if (food_visible) {  // Only show countdown when apple is visible
    int time_left = (food_display_duration - (millis() - food_start_time)) / 1000;  // Calculate seconds remaining
    tft.setCursor(160, 5);  // Adjust coordinates as necessary
    tft.setTextSize(2);
    tft.setTextColor(ILI9341_BLACK, ILI9341_DARKGREEN);  // Background green to overwrite old time
    tft.println(time_left);
  }
}

void erase_barrier() {
  // Erase the barrier by drawing over it with the background color
  tft.setCursor(barrier_position[0] * 10, barrier_position[1] * 10);
  tft.setTextColor(ILI9341_DARKGREEN);  // Use the background color
  tft.setTextSize(3);
  tft.print("6");  // Erase the barrier
}

void spawn_apple_level4() {
    bool redFoodValid = false;
    bool blueFoodValid = false;

    // Spawn red food (increases points) only if it's not already visible
    if (!red_food_visible || red_food_position[0] == -1 || red_food_position[1] == -1) {
        while (!redFoodValid) {
            int random_x = random(1, 23);
            int random_y = random(3, 31);
            redFoodValid = true;

            // Check if red food spawns on the snake or too close to the barrier
            for (int i = 0; i < element; i++) {
                if (position[i][0] == random_x && position[i][1] == random_y) {
                    redFoodValid = false;
                    break;
                }
            }
            if (abs(random_x - barrier_position[0]) < 3 && abs(random_y - barrier_position[1]) < 3) {
                redFoodValid = false;
            }

            if (redFoodValid) {
                red_food_position[0] = random_x;
                red_food_position[1] = random_y;
                draw_quadrat(random_x, random_y, ILI9341_RED);  // Draw red food
                red_food_visible = true;  // Mark red food as visible
            }
        }
    }

    // Spawn new blue food (decreases points)
    if (blue_food_count < 10) {  // Limit the number of blue foods to 10
        while (!blueFoodValid) {
            int random_x = random(1, 23);
            int random_y = random(3, 31);
            blueFoodValid = true;

            // Check if blue food spawns on the snake, red food, or too close to the barrier
            for (int i = 0; i < element; i++) {
                if (position[i][0] == random_x && position[i][1] == random_y) {
                    blueFoodValid = false;
                    break;
                }
            }
            if ((random_x == red_food_position[0] && random_y == red_food_position[1]) ||
                (abs(random_x - barrier_position[0]) < 3 && abs(random_y - barrier_position[1]) < 3)) {
                blueFoodValid = false;
            }

            if (blueFoodValid) {
                blue_food_positions[blue_food_count][0] = random_x;
                blue_food_positions[blue_food_count][1] = random_y;
                blue_food_active[blue_food_count] = true;  // Mark this blue food as active
                draw_quadrat(random_x, random_y, ILI9341_BLUE);  // Draw blue food
                blue_food_count++;  // Increase the count of blue foods
            }
        }
    }

    food_start_time = millis();  // Reset the food timer for red and blue foods
}
