#include <FrequencyTimer2.h>
#include <ArduinoQueue.h>

bool leds[8][8];
int mp[8][8] =  { \
    {0, 0, 0, 0, 0, 0, 0, 0 },\
    {0, 0, 0, 0, 0, 0, 0, 0 },\
    {0, 0, 0, 0, 0, 0, 0, 0 },\
    {0, 0, 0, 0, 0, 0, 0, 0 },\
    {0, 0, 0, 0, 0, 0, 0, 0 },\
    {0, 0, 0, 0, 0, 0, 0, 0 },\
    {0, 0, 0, 0, 0, 0, 0, 0 },\
    {0, 0, 0, 0, 0, 0, 0, 0 },\
};
int pins[16] = {7, 6, 5, 4, 3, 2, 1, 0, 13, 12, 11, 10, A0, A1, A2, A3};
bool playing = false;
int playerX = 4;
int playerY = 4;
int dir = 0;

struct snk {
  int x; 
  int y;
};

ArduinoQueue<snk> snake(100);

byte col = 0;

unsigned long pm = 0;

int cols[8] = {pins[12], pins[2], pins[3], pins[9], pins[5], pins[10], pins[14], pins[15]};
int rows[8] = {pins[8], pins[13], pins[7], pins[11], pins[0], pins[6], pins[1], pins[4]};

#define THREE { \
    {0, 0, 1, 1, 1, 1, 0, 0 },\
    {0, 0, 0, 0, 0, 0, 1, 0 },\
    {0, 0, 0, 0, 0, 0, 1, 0 },\
    {0, 0, 1, 1, 1, 1, 0, 0 },\
    {0, 0, 0, 0, 0, 0, 1, 0 },\
    {0, 0, 0, 0, 0, 0, 1, 0 },\
    {0, 0, 0, 0, 0, 0, 1, 0 },\
    {0, 0, 1, 1, 1, 1, 0, 0 },\ 
}

#define TWO { \
    {0, 0, 1, 1, 1, 1, 0, 0 },\
    {0, 0, 0, 0, 0, 0, 1, 0 },\
    {0, 0, 0, 0, 0, 0, 1, 0 },\
    {0, 0, 1, 1, 1, 1, 0, 0 },\
    {0, 1, 0, 0, 0, 0, 0, 0 },\
    {0, 1, 0, 0, 0, 0, 0, 0 },\
    {0, 1, 0, 0, 0, 0, 0, 0 },\
    {0, 0, 1, 1, 1, 1, 0, 0 },\ 
}

#define ONE { \
    {0, 0, 0, 0, 1, 0, 0, 0},\
    {0, 0, 0, 1, 1, 0, 0, 0},\
    {0, 0, 1, 0, 1, 0, 0, 0},\
    {0, 0, 0, 0, 1, 0, 0, 0},\
    {0, 0, 0, 0, 1, 0, 0, 0},\
    {0, 0, 0, 0, 1, 0, 0, 0},\
    {0, 0, 0, 0, 1, 0, 0, 0},\
    {0, 0, 1, 1, 1, 1, 1, 0},\
}

#define GAMEOVER { \
    {0, 0, 0, 0, 0, 0, 0, 0 },\
    {0, 0, 0, 0, 0, 0, 0, 0 },\
    {1, 0, 1, 0, 0, 1, 0, 1 },\
    {0, 1, 0, 0, 0, 0, 1, 0 },\
    {1, 0, 1, 0, 0, 1, 0, 1 },\
    {0, 0, 0, 0, 0, 0, 0, 0 },\
    {0, 1, 1, 1, 1, 1, 1, 0 },\
    {0, 0, 0, 0, 0, 0, 0, 0 },\
}

byte patterns[4][8][8] = { THREE, TWO, ONE, GAMEOVER };  

void setup()
{
    for (int i = 0; i < 16; i++)
    {
        pinMode(pins[i], OUTPUT);
    }
    FrequencyTimer2::setOnOverflow(display);
    setPattern(0);
    delay(1000);
    setPattern(1);
    delay(1000);
    setPattern(2);
    delay(1000);
    
    snk defaultPlayer = {playerX, playerY};
    snake.enqueue(defaultPlayer);
    mp[playerX][playerY] = 1;
    setNewItem();
    playing = true;
}

void loop()
{
  int x = analogRead(A4);
  int y = analogRead(A5);

  if (!playing)
    return;

  if (x == 0) {
    dir = 0;
  } else if (x == 1023) {
    dir = 1;
  } else if (y == 0) {
    dir = 2; //UP
  } else if (y == 1023) {
    dir = 3; //DOWN
  } 

  unsigned long mm = millis(); 
  if (mm - pm >= 500) {
    pm = mm;

    switch (dir) {
      case 0: //LEFT
        playerY--;
        if (playerY < 0 || mp[playerX][playerY] == 1) {
          gameOver();
          return;
        }
        break;
      case 1: //RIGHT
        playerY++;
        if (playerY > 7 || mp[playerX][playerY] == 1) {
          gameOver();
          return;
        }
        break;
      case 2: //UP
        playerX--;
        if (playerX < 0 || mp[playerX][playerY] == 1) {
          gameOver();
          return;
        }
        break;
      case 3: //DOWN
        playerX++;
        if (playerX > 7 || mp[playerX][playerY] == 1) {
          gameOver();
          return;
        }
        break;
    }
    if (mp[playerX][playerY] == 2) {
      setNewItem();
    } else {
      snk deleted = snake.dequeue();
      mp[deleted.x][deleted.y] = 0;
    }
    mp[playerX][playerY] = 1;
    snk newPlayer = {playerX, playerY};
    snake.enqueue(newPlayer);
  
    setMap();
  }
}

void setPattern(int pattern) { 
  for (int i = 0; i < 8; i++) 
    for (int j = 0; j < 8; j++) 
      leds[i][j] = patterns[pattern][i][j];   
}

void setNewItem() {
  int i = random(8), j = random(8);
  while (mp[i][j] != 0) {
    i = random(8);
    j = random(8);
  }
  mp[i][j] = 2;
}

void setMap() {
  for (int i = 0; i < 8; i++) 
    for (int j = 0; j < 8; j++) 
      leds[i][j] = mp[i][j];
}

void gameOver() {
  playing = false;
  setPattern(3);
}

void display() {
  digitalWrite(cols[col], HIGH); 
  col++;
  if (col == 8) {
    col = 0;
  }
  for (int row = 0; row <= 7; row++) {
    if (leds[col][7 - row] != 0) {  
      digitalWrite(rows[row], HIGH);
    }
    else {  
      digitalWrite(rows[row], LOW);
    }
  }
  digitalWrite(cols[col], LOW);  
}