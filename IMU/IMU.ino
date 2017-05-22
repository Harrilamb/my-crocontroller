// IMU 6DOF
// Written By: Harrison Lambert
// Calibrated for gravity, but still need to remove it from the accelerometer for correct position propagation
// Include Libraries
#include <Wire.h> // library for serial comms
#include <stdio.h> // standard library
// Map Accelerometer: datasheet http://www.nxp.com/assets/documents/data/en/application-notes/AN3397.pdf
#define ACC_ADD 0x53 // accelerometer address
#define ACCX0 0x32 // 0 for lower least significant bit; hex for port address
#define ACCX1 0x33 // 1 for higher most significant bit; hex for port address
#define ACCY0 0x34
#define ACCY1 0x35
#define ACCZ0 0x36
#define ACCZ1 0x37
// Map Gyroscope; datasheet https://www.sparkfun.com/datasheets/Sensors/Gyro/PS-ITG-3200-00-01.4.pdf
#define GYRO_ADD 0x68
#define SMPLRT_DIV 0x15 // Sample rate setting port
#define DLPF_FS 0x16 // Low pass filter setting port
#define GYROX0 0x1D
#define GYROX1 0x1E
#define GYROY0 0x1F
#define GYROY1 0x20
#define GYROZ0 0x21
#define GYROZ1 0x22
// Inidicator LED's
#define notready 2
#define good2go 3
// Power Settings for Accelerometer
#define POWER_CTL 0x2D
#define POWER_ON 0x08
#define STANDBY 0x04
// Sensor low pass settings for gyro in binary (bit shifted 1's)
char DLPF_CFG_0 = 1<<0;
char DLPF_CFG_1 = 1<<1;
char DLPF_CFG_2 = 1<<2;
char DLPF_FS_SEL_0 = 1<<3;
char DLPF_FS_SEL_1 = 1<<4;
// Declare class for 3D readouts
struct readout{
  float x;
  float y;
  float z;
};
typedef struct readout R;
// Instances of object for readout storage
R acc; R abias; R acorr; R pa; R scale; // objects for accelerometer storage; accelerometer vals, bias, corrected vals, past acceleration
R gyro; R pbias; R gcorr; R pg; // objects for gyroscope storage
R vel; R pv; R omg; R pw; // objects for velocity storage; velocity, past velocity, omega, past omega
R pos; R ang;// objects for position storage; position & angle
float gmag; // Not very accurate as each axis should be calibrated, but it'll do for this exercise
struct scale{ // Re-mapping of values needs to be done with var-type long
  long x;
  long y;
  long z;
};
void setup() {
// Initialize active pins
  pinMode(notready,OUTPUT);
  pinMode(good2go,OUTPUT);
// Start up the bus. vroom vroom
  Wire.begin();
  Serial.begin(9600);
// Initial LED states
  digitalWrite(notready,HIGH);
  digitalWrite(good2go,LOW);
// Turn on accelerometer
  writeRegister(ACC_ADD,POWER_CTL,POWER_ON);
// Configure gyroscope; set scale for the outputs to +/-2000 degrees per second and sample rate to 100Hz
  writeRegister(GYRO_ADD, DLPF_FS, (DLPF_FS_SEL_0|DLPF_FS_SEL_1|DLPF_CFG_0));
  writeRegister(GYRO_ADD, SMPLRT_DIV, 9);
}

// Initialize variables for sensing movement
int count=0; // initial calibration (1024 sum average) counter
int corr=0; // correction (64 sum average) counter
bool calibrated = false; // initial calibration
int lpf = 10; // Low pass filter's number of summations
int plot = 2; // Set 1-5 depending on what you want to graph; 1=velocity, 2= position, 3=omega, 4=angle, 5=textual

// Looper is a great movie
void loop() {
// Get accelerometer
  acc.x = (readRegister(ACC_ADD,ACCX1) << 8 | readRegister(ACC_ADD,ACCX0)); // Bit shift the most significant bit to a high position, and least significant bit to a low position in the byte (16 bit)
  acc.y = (readRegister(ACC_ADD,ACCY1) << 8 | readRegister(ACC_ADD,ACCY0));
  acc.z = (readRegister(ACC_ADD,ACCZ1) << 8 | readRegister(ACC_ADD,ACCZ0));

// Get gyroscope
  gyro.x = (readRegister(GYRO_ADD,GYROX1) << 8 | readRegister(GYRO_ADD,GYROX0));
  gyro.y = (readRegister(GYRO_ADD,GYROY1) << 8 | readRegister(GYRO_ADD,GYROY0));
  gyro.z = (readRegister(GYRO_ADD,GYROZ1) << 8 | readRegister(GYRO_ADD,GYROZ0));
  
// Initial calibration
  if(count == 0x0400){ // once the counter is equal to 1024
    pbias.x = pbias.x / 1024, pbias.y = pbias.y / 1024, pbias.z = pbias.z / 1024; // divide sum to get average
    abias.x = abias.x / 1024, abias.y = abias.y / 1024, abias.z = abias.z / 1024;
    gmag = sqrt(sq(abias.x)+sq(abias.y)+sq(abias.z));
    //Serial.println(map(abias.x,-gmag,gmag,-1000,1000));
    calibrated = true; // board is now calibrated
    digitalWrite(notready,LOW); // led calibration indicators
    digitalWrite(good2go,HIGH);
    //Serial.println((String)"ACC BIAS: "+abias.x+" "+abias.y+" "+abias.z); // print out the calculated biases
    //Serial.println((String)"GYRO BIAS: "+pbias.x+" "+pbias.y+" "+pbias.z);
    count++; // up one more time to escape this conditional on next iteration
  }else if(count<0x0400){ // don't want to trigger if count>1024
    count++; // pump up the jam
  }

// No-Movement Filter
  if(calibrated){
    scale.x = map(acc.x,-gmag,gmag,-1000,1000);
    acc.x = scale.x/1000*9.8;
    //abs(acc.x-abias.x)<3?acc.x=0:acc.x=acc.x-abias.x; // if the accelerometer doesn't move more than 3 in an axis consider its value as 0
    //abs(acc.x/gmag)<1.1?acc.x=0:acc.x=acc.x/gmag;
    scale.y = map(acc.y,-gmag,gmag,-1000,1000);
    acc.y = scale.y/1000*9.8;
    //abs(acc.y-abias.y)<3?acc.y=0:acc.y=acc.y-abias.y; // 3 works pretty well for all axes on the accelerometer, y-axis is a little higher though
    //abs(acc.y/gmag)<1.1?acc.y=0:acc.y=acc.y/gmag;
    scale.z = map(acc.z,-gmag,gmag,-1000,1000);
    acc.z=scale.z/1000*9.8;
    //abs(acc.z-abias.z)<3?acc.z=0:acc.z=acc.z-abias.z; // Every threshold still has some random noise ~=4
    //abs(acc.z/gmag)<1.1?acc.z=0:acc.z=acc.z/gmag;
    abs(gyro.x-pbias.x)<700?gyro.x=0:gyro.x=(gyro.x-pbias.x)/1000; // if the gyro doesn't move more than 700 in an axis consider its value as 0
    abs(gyro.y-pbias.y)<700?gyro.y=0:gyro.y=(gyro.y-pbias.y)/1000; // 700 is the lowest value that gets a consistent 0 value at rest since the noise is super high
    abs(gyro.z-pbias.z)<700?gyro.z=0:gyro.z=(gyro.z-pbias.z)/1000;
  }else{ // if the board hasn't been calibrated yet then keep building up the bias value
    abias.x=abias.x+acc.x, abias.y=abias.y+acc.y, abias.z=abias.z+acc.z;
    pbias.x=pbias.x+gyro.x, pbias.y=pbias.y+gyro.y, pbias.z=pbias.z+gyro.z;
  }
  
// Low Pass Filter
  if(corr==lpf && calibrated){ // if corrector count is 64 and the board is already calibrated
    pa.x = acorr.x, pa.y = acorr.y, pa.z = acorr.z;
    pg.x = gcorr.x, pg.y = gcorr.y, pg.z = gcorr.z;
    acorr.x=acorr.x/lpf, acorr.y=acorr.y/lpf, acorr.z=acorr.z/lpf; // divide sum by 64 to get average
    gcorr.x=gcorr.x/lpf, gcorr.y=gcorr.y/lpf, gcorr.z=gcorr.z/lpf;
    pv.x = vel.x, pv.y = vel.y, pv.z = vel.z;
    pw.x = omg.x, pw.y = omg.y, pw.z = omg.z;
    // Propogate velocity and position
    vel.x = vel.x+pa.x+((acorr.x-pa.x)/2), vel.y = vel.y+pa.y+((acorr.y-pa.y)/2), vel.z = vel.z+pa.z+((acorr.z-pa.z)/2); // Linear velocity
    omg.x = omg.x+pg.x+((gcorr.x-pg.x)/2), omg.y = omg.y+pg.y+((gcorr.y-pg.y)/2), omg.z = omg.z+pg.z+((gcorr.z-pg.z)/2); // Angular velocity
    pos.x = pos.x+pv.x+((vel.x-pv.x)/2), pos.y = pos.y+pv.y+((vel.y-pv.y)/2), pos.z = pos.z+pv.z+((vel.z-pv.z)/2); // Linear position
    ang.x = ang.x+pw.x+((omg.x-pw.x)/2), ang.y = ang.y+pw.y+((omg.y-pw.y)/2), ang.z = ang.z+pw.z+((omg.z-pw.z)/2); // Angular position
    if(plot==1){ // Use Serial Plotter
      Serial.println((String)vel.x+" "+vel.y+" "+vel.z);
    }else if(plot==2){
      Serial.println((String)pos.x+" "+pos.y);
    }else if(plot==3){
      Serial.println((String)omg.x+" "+omg.y+" "+omg.z);
    }else if(plot==4){
      Serial.println((String)ang.x+" "+ang.y+" "+ang.z);
    }else if(plot==5){ // Use Serial Monitor
      Serial.println((String)"ACCEL: "+acorr.x+" "+acorr.y+" "+acorr.z); // print corrected acceleration
      Serial.println((String)"GYRO: "+gcorr.x+" "+gcorr.y+" "+gcorr.z); // print corrected gyroscope
    }
    acorr.x=0, acorr.y=0, acorr.z=0; // reset corrected acceleration
    gcorr.x=0, gcorr.y=0, gcorr.z=0; // reset corrected gyroscope
    corr=0; // reset correction counter
  }else if(corr!=lpf && calibrated){ // if correction counter is not 64 and board is already calibrated
    acorr.x=acorr.x+acc.x, acorr.y=acorr.y+acc.y, acorr.z=acorr.z+acc.z; // add to acceleration correction sum
    gcorr.x=gcorr.x+gyro.x, gcorr.y=gcorr.y+gyro.y, gcorr.z=gcorr.z+gyro.z; // add to gyroscope correction sum
    corr++; // pump up the volume
  }
  
}

// Function for I2C reading from port
byte readRegister(int deviceAddress, byte address){

  Wire.beginTransmission(deviceAddress);
  Wire.write(address); // register to read
  Wire.endTransmission();

  Wire.requestFrom(deviceAddress, 1); // read a byte
  
  byte v = Wire.read(); // Read value from register
  return v;
}

// Function for I2C writing to port
void writeRegister(byte add, byte reg, byte val){
  Wire.beginTransmission(add); // start transmission to device address
  Wire.write(reg);             // send register address
  Wire.write(val);             // send value to write
  Wire.endTransmission();      // end transmission
}
