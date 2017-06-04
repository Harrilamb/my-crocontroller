// Written By Harrison Lambert
// 1 June 2017
// Parallax TSL230R: http://www.mouser.com/ds/2/321/27924-TSL230R-v1.0-21734.pdf
// Most of this is not my code; the CMOS chip reads differently
// then how explained in its data sheet, so the credit for figuring
// out how to get a frequency from it goes to http://bit.ly/2qMLfTg
// Future Updates: Not sure why code works since the interrupt is attached to the wrong pin

//Map light sensor
#define TSL_S0 5 // Sensitivity ports
#define TSL_S1 6
#define TSL_S2 7 // Scaling ports
#define TSL_S3 8
#define TSL_FREQ_PIN 2 // Frequency Output
#define READ_TM 1000 // How many 
// LED's to test specific frequencies
#define RED 9
#define GREEN 10
#define BLUE 11
#define NIR 12 // Infrared
#define BUTTON 13 // Changes LED's, detects start of push

unsigned long cur_tm = millis(); // current time; initialize as now
unsigned long pre_tm = cur_tm; // previous time; initialize as equal to current time

unsigned int tm_diff = 0; // difference in time

unsigned long pulse_cnt = 0; // running sum of pulses

int calc_sensitivity = 10; // sensitivity setting, declared as int but is a binary value

int lastButt=1; // button is 1 when it isn't clicked
int ledCount=0; // number to indicate current led; 0-3
bool change=false; // whether or not to change the LED

void setup() {
  attachInterrupt(0, add_pulse, RISING); // add a count every time the pin goes from low to high
  // Make the sensitivity ports writable
  pinMode(TSL_S0, OUTPUT);
  pinMode(TSL_S1, OUTPUT);
  pinMode(TSL_S2, OUTPUT);
  pinMode(TSL_S3, OUTPUT);
  // Make LED's writable
  pinMode(RED, OUTPUT);
  pinMode(BLUE, OUTPUT);
  pinMode(GREEN, OUTPUT);
  pinMode(NIR, OUTPUT);
  // Make frequency port readable
  pinMode(TSL_FREQ_PIN, INPUT);
  // Make button readable
  pinMode(BUTTON,INPUT);
  Serial.begin(9600); // set data rate
  // Set sensitivity according to datasheet
  analogWrite(TSL_S0, LOW);
  analogWrite(TSL_S1, HIGH);
  analogWrite(TSL_S2, LOW);
  analogWrite(TSL_S3, LOW);
}

void loop() {
  pre_tm = cur_tm; // Set previous time
  cur_tm = millis(); // set current time

  if (cur_tm > pre_tm) { // Increment time difference
    tm_diff += cur_tm - pre_tm;
  } else if (cur_tm < pre_tm) { // Handle overflow situation
    tm_diff += (cur_tm + (34359737 - pre_tm));
  }

  // When time difference is greater than your summation period perform measurements
  if (tm_diff >= READ_TM) {
    tm_diff = 0; // reset time difference
    unsigned long frequency = get_tsl_freq(); // get frequency
    Serial.println(frequency);
    float uwatt = calc_uwatt_cm2(frequency); // get power per area
    //Serial.println(uwatt); // Uncomment to see power per area
  }

  // If the button is clicked change the LED
  if(digitalRead(BUTTON)==0 && lastButt==1){
    ledCount==3?ledCount=0:ledCount++; // After 4th LED reset to 1st
    lastButt=digitalRead(BUTTON); // Update last button value
    change = true; // Trigger change to LED
  }else{
    lastButt=digitalRead(BUTTON); // Update last button value
  }

  // If LED change was triggered
  if(change){
    turnOff();
    switch(ledCount){ // Turn on the LED based on the number
      case 0:
          analogWrite(RED,HIGH);
        break;
      case 1:
          analogWrite(GREEN,HIGH);
        break;
      case 2:
          analogWrite(BLUE,HIGH);
        break;
      case 3:
          analogWrite(NIR,HIGH);
        break;
    }
    change=false; // Reset change trigger to false
  }
  
}

// Get frequency function
unsigned long get_tsl_freq() {
  unsigned long freq = pulse_cnt * 100; // Scale frequency
  pulse_cnt = 0; // Reset frequency
  return (freq);
}

// Get power per area function
float calc_uwatt_cm2(unsigned long freq) {
  float uw_cm2 = (float) freq / (float) calc_sensitivity; // Calculate power per area from frequency
  uw_cm2 *= ( (float) 1 / (float) 0.0136); // Scale power
  return (uw_cm2);
}

// Iterate frequency counter
void add_pulse() {
  pulse_cnt++;
  return;
}

// Not used but included if sensitivity wants to be modified dynamically in the future
// Specific settings in datasheet
void sensitivity( bool dir ) {
  int pin_0;
  int pin_1;
  if( dir==true){
    if( calc_sensitivity == 100) {
      pin_0 = true;
      pin_1 = true;
    }else{
      pin_0 = false;
      pin_1 = true;
    }

    calc_sensitivity *= 10;
  }else{
    if( calc_sensitivity == 10){
      return;
    }
    if(calc_sensitivity == 100){
      pin_0 = true;
      pin_1 = true;
    }else{
      pin_0 = false;
      pin_1 = true;
    }

    calc_sensitivity = calc_sensitivity / 10;
  }

  digitalWrite(TSL_S0,pin_0);
  digitalWrite(TSL_S1,pin_1);
  return;
}

// Turn off all LED's function; used before turning on an LED
void turnOff(){
  analogWrite(RED,LOW);
   analogWrite(GREEN,LOW);
    analogWrite(BLUE,LOW);
     analogWrite(NIR,LOW);
     return;
}

