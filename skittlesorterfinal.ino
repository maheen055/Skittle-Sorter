/*
*Skittle Sorter
*This sketch uses an Arduino board and the circuit shown in the diagram above
*to sort skittles by color. The RGB sensor is used to detect the color of each skittle,
*and the servos are used to sort the skittles into designated test tubes.
*Author: Maheen Shoaib
*Date: January 25, 2023
*/

#include <Wire.h> // library for communication with I2C devices
#include "Adafruit_TCS34725.h" // library for the RGB sensor
#include<Servo.h> // library for controlling servo motors

/* Based on example code for the Adafruit TCS34725 breakout library */


//=======================Variables related to RGB sensor=========================


Adafruit_TCS34725 tcs = Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_24MS, TCS34725_GAIN_1X);  // setup RGB sensor

int colour = 0;//0-black, 1-yelow, 2-green, 3-orange, 4-red, 5-purple
int previous_colour = colour;
int black_count = 0;  // count of empty/unknown color

int start_measurment_angle = 88; //servo position to align skittle in front of sensor
int measurment_sweep = 2; //number of measure taken . used to average error
int measurment_angle_increment = 4; //angle increment between measurments

//limit values[] = {min_red_values, max_red_values, min_green_values, max_green_values, min_blue_values, max_blue_values}
uint16_t black_limit_values[] = {0, 200, 0, 200, 0, 200};// color limit values for blank (no skittle)
uint16_t yellow_limit_values[] = {850, 2200, 800, 2200, 400, 1100};// color limit values for yellow
uint16_t green_limit_values[] = {350, 800, 650, 1400, 250, 700};// color limit values for green
uint16_t orange_limit_values[] = {650, 1800, 300, 850, 210, 600};// color limit values for orange
uint16_t red_limit_values[] = {400, 950, 150, 450, 150, 400};// color limit values for red
uint16_t purple_limit_values[] = {150, 400, 150, 450, 150, 500};// color limit values for purple

//=======================Variables related to RGB led=========================

// pin for the red element of the RGB LED
byte R_pin = 5;
// pin for the green element of the RGB LED
byte G_pin = 6;
// pin for the blue element of the RGB LED
byte B_pin = 11;
// variable to store the current intensity of the red element of the RGB LED
int current_red_value = 255;
// variable to store the current intensity of the green element of the RGB LED
int current_green_value = 255;
// variable to store the current intensity of the blue element of the RGB LED
int current_blue_value = 255;

// array to store the intensity values for the orange color
int orange[] = {255, 30, 0};
// array to store the intensity values for the red color
int red[] = {255, 0, 0};
// array to store the intensity values for the green color
int green[] = {0, 204, 0};
// array to store the intensity values for the yellow color
int yellow[] = {255, 255, 0};
// array to store the intensity values for the purple color
int purple[] = {102, 0, 204};

//======================= Variables related to feeder servos =========================

// delay before releasing the skittle into the test tube
int release_delay = 220;
// angles of the test tubes
int colour_angles[] = {20, 59, 95, 135, 173};
// constant value used to adjust for backlash when moving the feeder servo in the backward direction
const int backward_anti_backlash = 2;
// constant value used to adjust for backlash when moving the feeder servo in the forward direction
const int forward_anti_backlash = 2;
Servo feeder_servo; //declare feeder servo
Servo holder_servo; //declare tubes holder servo

//=================================== SETUP =========================================

void setup(void) {
  
  Serial.begin(9600); // Start serial communication at 9600 baud rate

  pinMode(R_pin, OUTPUT);   // Set the pin mode for R_pin as OUTPUT 
  pinMode(G_pin, OUTPUT);   // Set the pin mode for G_pin as OUTPUT 
  pinMode(B_pin, OUTPUT);   // Set the pin mode for B_pin as OUTPUT 

  analogWrite(R_pin, 255);  // Turn off the red element of the RGB LED by setting its value to 255
  analogWrite(G_pin, 255);  // Turn off the green element of the RGB LED by setting its value to 255
  analogWrite(B_pin, 255);  // Turn off the blue element of the RGB LED by setting its value to 255

  feeder_servo.attach(3);  // Attach the feeder servo to pin 3
  holder_servo.attach(2);   // Attach the holder servo to pin 2
  feeder_servo.write(0);    // Initial position of the feeder servo is set to 0
  holder_servo.write(colour_angles[0]); // Initial position of the holder servo is set to the first angel in the colour_angles array

  int white[] = {0, 0, 0}; //local white values variable
  int black[] = {255, 255, 255}; //local black values variable
  set_led(1000, white);   // turn LED on (white)

  if (tcs.begin()) {    //check if sensor found
    Serial.println("Found sensor");
    starting_sequence();
  } else {
    Serial.println("No TCS34725 found ... check your connections");
    while (1)  //fast red blinking if sensor not found
    {
      set_led(300, red);
      set_led(300, black);
    }
  }
}

//=================================== LOOP =========================================

void loop(void) {

  if (black_count < 10) // Check if the black_count is less than 10
  {
    feeder_servo.write(0); // Set the servo position to the top for skittle collection
    delay(450);  // Delay to let the feeder servo enough time to get in position
    previous_colour = colour;
    get_colour(); // Read skittle colour

    if (colour == 0) {
      black_count ++; //If no colour found, increment black count
      shake();// Gives a bit more shake to the reservoir even if no skittle is there
    }
    else if (colour == 6) {
      black_count ++; // If colour unknown, increment black count - no skittle release
    }
    else { // If colour found
      move_holder(); // Move tube holder
      release_skittle(); // Release skittle
      black_count = 0;
    }
  }

  else { // If black_count is greater than or equal to 10
    end_loop(1000); // End the loop and wait for 1 second
  }
}

//=================================== GET COLOUR =========================================

void get_colour() {

  uint16_t r, g, b, c;//, colorTemp, lux;
  uint16_t total_r = 0;
  uint16_t total_g = 0;
  uint16_t total_b = 0;

  feeder_servo.write(start_measurment_angle); //move to start measurement angle
  delay(200);

  Serial.println("----------------");
  for (int i = 0; i <= measurment_sweep; i++) //loop for each measure
  {
    tcs.getRawData(&r, &g, &b, &c);    //get color data

    feeder_servo.write(start_measurment_angle + i * measurment_angle_increment); //increment servo angle for next measure
    total_r += r; //add red value to total red value
    total_g += g; //add green value to total green value
    total_b += b; //add blue value to total blue value
    delay(15);
  }


  total_r /= measurment_sweep;  //average values across all measurements
  total_g /= measurment_sweep;
  total_b /= measurment_sweep;

  Serial.print(total_r); Serial.print(" ");
  Serial.print(total_g); Serial.print(" ");
  Serial.print(total_b); Serial.print(" ");
  Serial.println(" ");

  //compare values to determine the color .
  if ((total_r < black_limit_values[1]) &&    //check for black
      (total_g < black_limit_values[3]) &&
      (total_b < black_limit_values[5]))
  { Serial.println("black");
    colour = 0;
  }
  else if ((total_r >= yellow_limit_values[0]) &&   //check for yellow
           (total_r < yellow_limit_values[1]) &&
           (total_g >= yellow_limit_values[2]) &&
           (total_g < yellow_limit_values[3]) &&
           (total_b >= yellow_limit_values[4]) &&
           (total_b < yellow_limit_values[5]))
  { Serial.println("yellow");
    colour = 1;
  }
  else if ((total_r >= green_limit_values[0]) &&   //check for green
           (total_r < green_limit_values[1]) &&
           (total_g >= green_limit_values[2]) &&
           (total_g < green_limit_values[3]) &&
           (total_b >= green_limit_values[4]) &&
           (total_b < green_limit_values[5]))
  { Serial.println("green");
    colour = 2;
  }
  else if ((total_r >= orange_limit_values[0]) &&   //check for orange
           (total_r < orange_limit_values[1]) &&
           (total_g >= orange_limit_values[2]) &&
           (total_g < orange_limit_values[3]) &&
           (total_b >= orange_limit_values[4]) &&
           (total_b < orange_limit_values[5]))
  { Serial.println("orange");
    colour = 3;
  }
  else if ((total_r >= red_limit_values[0]) &&   //check for red
           (total_r < red_limit_values[1]) &&
           (total_g >= red_limit_values[2]) &&
           (total_g < red_limit_values[3]) &&
           (total_b >= red_limit_values[4]) &&
           (total_b < red_limit_values[5]))
  { Serial.println("red");
    colour = 4;
  }
  else if ((total_r >= purple_limit_values[0]) &&   //check for purple
           (total_r < purple_limit_values[1]) &&
           (total_g >= purple_limit_values[2]) &&
           (total_g < purple_limit_values[3]) &&
           (total_b >= purple_limit_values[4]) &&
           (total_b < purple_limit_values[5]))
  { Serial.println("purple");
    colour = 5;
  }
  else {
    Serial.println("unknown");    //if color not detected, set colour to 0 to force new check
    colour = 6;
  }

}

//============================== MOVE HOLDER =============================
// adjust delay depending how much the holder needs to move
// include an backlash compensation

void move_holder() {

 int new_holder_position = colour_angles[colour - 1]; //gets new position of tube
  int holder_delay = (abs(previous_colour - colour) * 70);// - release_delay; //leaves 70ms for the holder servo to turn 36 degrees

  if (holder_delay < 0) {  //keep delay superior to 0
    holder_delay = 0;
  }
  if (previous_colour > colour)
  {
    //forces the holder servo to go back further than normal position to compensation for gearing backlash
    int anti_backlash_angle = new_holder_position - backward_anti_backlash;
    holder_servo.write(anti_backlash_angle);
    //delay(holder_delay);
  }
  else
  {
    int anti_backlash_angle = new_holder_position + forward_anti_backlash;
    //holder_servo.write(anti_backlash_angle);
    holder_servo.write(new_holder_position); // move holder in position
    //delay(holder_delay);
  }

  if (colour == 1) {
    set_led(holder_delay , yellow);
  }
  else if (colour == 2) {
    set_led(holder_delay , green);
  }
  else if (colour == 3) {
    set_led(holder_delay , orange);
  }
  else if (colour == 4) {
    set_led(holder_delay , red);
  }
  else if (colour == 5) {
    set_led(holder_delay , purple);
  }
  else {}

}

//=============================  RELEASE SKITTLE  ===========================

void release_skittle() {

  feeder_servo.write(180); // set servo position to the bottom for skittle release
  delay(release_delay);

}

//=================================  SHAKE  =================================

void shake() {
  
  int shake_delay = 80;
  int shake_amount = 5;
  int shake_min_value = 90;
  int shake_max_value = 180;

  feeder_servo.write(180); // set servo position to the bottom for skittle release
  delay(release_delay);
  feeder_servo.write(120); // set servo position to the bottom for skittle release
  delay(80);
  for (int i = 0; i <= shake_amount; i++) //loop for each measure
  {
    feeder_servo.write(shake_min_value); // set servo position to the bottom for skittle release
    delay(shake_delay);
    feeder_servo.write(shake_max_value); // set servo position to the bottom for skittle release
    delay(shake_delay);
  }
}

//============================ STARTING SEQUENCE ============================

void starting_sequence() {

  colour = 1;
  move_holder();
  set_led(300, yellow);
  delay (400);

  colour = 2;
  move_holder();
  set_led(300, green);
  delay (400);

  colour = 3;
  move_holder();
  set_led(300, orange);
  delay (400);

  colour = 4;
  move_holder();
  set_led(300, red);
  delay (400);

  colour = 5;
  move_holder();
  set_led(300, purple);
  delay (400);

  previous_colour = colour;
  colour = 3;
  move_holder();

  end_loop(200);
}

//================================ END LOOP =================================

void end_loop(int duration) {

  set_led(duration , orange);
  set_led(duration , red);
  set_led(duration , green);
  set_led(duration , yellow);
  set_led(duration , purple);
}

//============================   SET RGB LED COLOUR   =======================
//   This act as a delay() but allows the LED to change color while waiting .

void set_led(int duration, int color[3]) {

  int start_time = millis();     // start time value
  int current_time = start_time; // current time value
  int current_duration = 0;      // total duration

  while (current_duration < duration)
  {
    analogWrite(R_pin, map(current_duration, 0, duration, current_red_value, color[0]));  //ramp up red value for each loop
    analogWrite(G_pin, map(current_duration, 0, duration, current_green_value, color[1])); //ramp up green value for each loop
    analogWrite(B_pin, map(current_duration, 0, duration, current_blue_value, color[2])); //ramp up blue value for each loop

    current_time = millis();   //update current time
    current_duration = current_time - start_time;  //calculate total duration
  }

  current_red_value = color[0];   // set new red current value
  current_green_value = color[1]; // set new green current value
  current_blue_value = color[2];  // set new blue current value

}
