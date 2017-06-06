// Sidescroller Video Game on Arduino with 2x16 LCD
// Written By Harrison Lambert
// Written For AERO 465 Final Project
// Future Updates: Should make it harder :/

#include <LiquidCrystal.h> // Include standard LCD library
LiquidCrystal lcd(12, 11, 5, 4, 3, 2); // Tell board what pins the LCD is on

const int button = 7; // Declare the button's pin
int buttonState = 0; // Whether or not the button is pressed; high or low
int height = 1; // Which row it is on, 0 for top 1 for bottom
int lives = 3; // How many lives the player has
int score = 0; // What score the player has
bool screen = false; // Is the user on the game screen? false for start screen
int rows [2][11] = {{0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0}}; // Array representation of enemy blocks, 1 means enemy in block 0 is no enemy
bool moving = true; // Switches between true and false to move man's legs
int timer = 0; // Timer for time logic, *doesn't work very well
int randNum; // Initialize random number for enemy logic
int bound = 50; // Bound for spawn logic
int spawnRow = 0; // Similar to height but for enemies
int altRow = 1; // Could probably get rid of this becuase it's just the opposite of spawnRow
bool newGameDelay = false; // For not restarting the game if the player is pressing the button when they die

void setup() {
  pinMode(button,INPUT); // Initialize button
  lcd.begin(16, 2);  // Start up the screen, beep beep boop
}

void loop() {
  buttonState = digitalRead(button); // Read the button
  height=!buttonState; // Height is the opposite of the button value; i.e. - High=NOT Low
  if(screen==true){ // If player is on the game screen
    moving=!moving; // Switch man moving back to not moving
    makeMan(height,moving); // Move man
    movement(); // Move enemies
    makeMan(height,moving);
    spawn(bound); // Spawn new enemies
    makeMan(height,moving);
    enemies(); // Make enemies
    hitbox(); // Check if player hit an enemy
    if(lives==0){ // If user has no more lives
      delay(2000); 
      lcd.clear(); // Clear screen
      newGameDelay=true;
      screen=false; // Tell game to switch to score screen
    }
    delay(100); // SPEED OF THE GAME
  }else{
    lcd.setCursor(0,0); // Move cursor to top left corner
    lcd.print("Your score: "+String(score)); // Write score screen stuff
    lcd.setCursor(0,1);
    lcd.print("Click 4 New Game");
    if(newGameDelay){
     delay(2000);
     newGameDelay=false; // Tell game to switch to game screen
    }
    if(height==0){ // If player clicks
      resetStats(); // Reset game
    }
  }
}

void makeMan(int loc, bool jog){ // Code to control man, input height and movement
    byte man[8] = { // LCD art for standing man
      B00100,
      B01010,
      B00100,
      B01110,
      B10101,
      B00100,
      B01010,
      B10001,
    };
    byte runMan[8] = { // LCD art for running man
      B00100,
      B01010,
      B00100,
      B01110,
      B01110,
      B00100,
      B01010,
      B00100,
    };
    lcd.createChar(1,man); // Turn byte art into special char
    lcd.createChar(2,runMan);
    lcd.setCursor(5,!loc); // Move cursor to man column (6)
    lcd.write(" ");
    lcd.setCursor(5,loc);
    if(jog){ // Choose which man to display
      lcd.write(byte(2));
    }else{
      lcd.write(byte(1));
    }
}

int makeHeart(int lives){ // LCD art for hearts; input # of lives
  byte heart[8] = { // Full life
    B00000,
    B00000,
    B01010,
    B11111,
    B01110,
    B00100,
    B00000,
    B00000,
  };
  byte emptyHeart[8] = { // Empty life
    B00000,
    B00000,
    B01010,
    B10101,
    B01010,
    B00100,
    B00000,
    B00000,
  };
  lcd.createChar(3,heart);
  lcd.createChar(4,emptyHeart);
  for(int i=0;i<3;i++){ // For each life show a heart
    lcd.setCursor(i,1);
    i<lives?lcd.write(byte(3)):lcd.write(byte(4));
  }
}

void spawn(int bound){
  randNum=random(0,100);
  if(randNum>=bound){
    if(randNum%2==0){
      spawnRow = 0;
      altRow = 1;
    }else{
      spawnRow = 1;
      altRow = 0;
    }
  }
  if((rows[altRow][10]==0) && (rows[altRow][9]==0) && (rows[spawnRow][10]==0)){
    rows[spawnRow][10]=1;
    rows[altRow][10]=0;
  }else if((rows[spawnRow][10]==0) && (rows[spawnRow][9]==0) && (rows[altRow][10]==0)){
    rows[altRow][10]=1;
    rows[spawnRow][10]=0;
  }else{
    rows[spawnRow][10]=0;
    rows[altRow][10]=0;
  }
};

void enemies(){
    byte ground[8] = { // LCD art for enemy on top row
      B01110,
      B10001,
      B10001,
      B11111,
      B01010,
      B10001,
      B01010,
      B10001,
    };
    byte sky[8] = { // LCD art for enemy on bottom row
      B00100,
      B01010,
      B10001,
      B11111,
      B01010,
      B10101,
      B10101,
      B10101,
    };
    lcd.createChar(5,ground);
    lcd.createChar(6,sky);
    for(int i = 0; i<2; i++){ // Iterate over 2x11 enemy space
      for(int j = 0; j<11; j++){
        lcd.setCursor(j+5,i);
        if(rows[i][j]){ // If the index in rows logic storage is 1, then write an enemy on the LCD
          i==0?lcd.write(byte(5)):lcd.write(byte(6));
        }else{
          if(j!=0){ // If index in rows logic is 0, then write empty block on LCD
            lcd.write(" ");
          }
        }
      }
    }
}

void movement(){ // Movement logic of enemies
    for(int i = 0; i<2; i++){
      for(int j = 0; j<10; j++){
        rows[i][j]=rows[i][j+1]; // Move enemies one block to the left
      }
    }
};

void scores(){ // Write score to top left corner of screen
  lcd.setCursor(0,0);
  lcd.print(score);
}

void hitbox(){ // Logic to see if player hit an enemy
   if(rows[0][0]==1 || rows[1][0]==1){
    if(rows[height][0]==0){
      score++; // If man and enemy are in different rows add 1 to score
    }else{
      lives=lives-1; // If man and enemy are in differenet rows add 
    }
    scores(); // Update score
    makeHeart(lives); // Update hearts
   }
}

void resetStats(){ // Reset everything for a new game
  lcd.clear();
  for(int i = 0; i<2; i++){
      for(int j = 0; j<11; j++){
        rows[i][j]=0; // Switch enemy index back to all 0's
      }
  }
  // Reset score and lives
  score=0;
  lives=3;
  makeHeart(lives);
  scores();
  screen=true; // Tell game to switch back to game screen
}

