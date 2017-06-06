// Sidescroller Video Game on Arduino with 2x16 LCD
// Written By Harrison Lambert
// Written For AERO 465 Final Project
// Future Updates: Need to comment what I did, but it's late, and I'm going to bed

#include <LiquidCrystal.h>

LiquidCrystal lcd(12, 11, 5, 4, 3, 2);
const int button = 7;
int buttonState = 0;
int height = 1;
int lives = 3;
int score = 0;
bool screen = false;
//int topRow = [0,0,0,0,0,0,0,0,0,0,0];
//int botRow = [0,0,0,0,0,0,0,0,0,0,0];
int rows [2][11] = {{0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0}};

void setup() {
  pinMode(button,INPUT);
  lcd.begin(16, 2);  
}

bool moving = true;
int timer = 0;
int randNum;
int bound = 50;
int spawnRow = 0;
int altRow = 1;
bool newGameDelay = false;

void loop() {
  buttonState = digitalRead(button);
  height=!buttonState;
  if(screen==true){
    moving=!moving;
    makeMan(height,moving);
    movement();
    makeMan(height,moving);
    spawn(bound);
    makeMan(height,moving);
    enemies();
    hitbox();
    if(lives==0){
      delay(2000);
      lcd.clear();
      newGameDelay=true;
      screen=false;
    }
    delay(100);
  }else{
    lcd.setCursor(0,0);
    lcd.print("Your score: "+String(score));
    lcd.setCursor(0,1);
    lcd.print("Click 4 New Game");
    if(newGameDelay){
     delay(2000);
     newGameDelay=false;
    }
    if(height==0){
      resetStats();
    }
  }
}

void makeMan(int loc, bool jog){
    byte man[8] = {
      B00100,
      B01010,
      B00100,
      B01110,
      B10101,
      B00100,
      B01010,
      B10001,
    };
    byte runMan[8] = {
      B00100,
      B01010,
      B00100,
      B01110,
      B01110,
      B00100,
      B01010,
      B00100,
    };
    lcd.createChar(1,man);
    lcd.createChar(2,runMan);
    lcd.setCursor(5,!loc);
    lcd.write(" ");
    lcd.setCursor(5,loc);
    if(jog){
      lcd.write(byte(2));
    }else{
      lcd.write(byte(1));
    }
}

int makeHeart(int lives){
  byte heart[8] = {
    B00000,
    B00000,
    B01010,
    B11111,
    B01110,
    B00100,
    B00000,
    B00000,
  };
  byte emptyHeart[8] = {
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
  for(int i=0;i<3;i++){
    lcd.setCursor(i,1);
    i<lives?lcd.write(byte(3)):lcd.write(byte(4));
  }
}

void enemies(){
    byte ground[8] = {
      B01110,
      B10001,
      B10001,
      B11111,
      B01010,
      B10001,
      B01010,
      B10001,
    };
    byte sky[8] = {
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
    for(int i = 0; i<2; i++){
      for(int j = 0; j<11; j++){
        lcd.setCursor(j+5,i);
        if(rows[i][j]){
          i==0?lcd.write(byte(5)):lcd.write(byte(6));
        }else{
          if(j!=0){
            lcd.write(" ");
          }
        }
      }
    }
}

void switchScreen(bool start){
 
}

void movement(){
    for(int i = 0; i<2; i++){
      for(int j = 0; j<10; j++){
        rows[i][j]=rows[i][j+1];
      }
    }
};

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

void scores(){
  lcd.setCursor(0,0);
  lcd.print(score);
}

void hitbox(){
   if(rows[0][0]==1 || rows[1][0]==1){
    if(rows[height][0]==0){
      score++;
    }else{
      lives=lives-1;
    }
    scores();
    makeHeart(lives);
   }
}

void resetStats(){
  lcd.clear();
  for(int i = 0; i<2; i++){
      for(int j = 0; j<11; j++){
        rows[i][j]=0;   
      }
  }
  score=0;
  lives=3;
  makeHeart(lives);
  lcd.setCursor(0,0);
  lcd.print(score);
  screen=true;
}

