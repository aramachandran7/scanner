/* Charlotte, Adi, Gabby */
#include <Servo.h>

#define ARRAY_SIZE                20
#define TIME_TO_MAX_POS           10000

/* */
volatile uint16_t sensor_value[ARRAY_SIZE] = { 0 }; 
volatile uint16_t filtered_sensor_value    = 0; 
volatile uint8_t pointer                   = 0; 

const int ir_scanner_pin              = A0; 
const int servo_pan_pin               = 9; // TODO fix arbitrary 
const int servo_tilt_pin              = 10; // TODO fix arbitrary 

uint8_t   pan_pos               = 0; 
uint8_t   tilt_pos              = 0; 

uint32_t  start_time_pan        = 0; 
bool first_time_pan             = true; 

uint32_t  start_time_tilt        = 0; 
bool first_time_tilt             = true; 

Servo pan_servo; 
Servo tilt_servo; 


/* Call once on every superloop */
void read_data_and_filter(){
  // read in data
  sensor_value[pointer] = analogRead(ir_scanner_pin); 
  pointer = (pointer + 1) % ARRAY_SIZE; 
  // apply moving average filter 
  filtered_sensor_value = 0; 
  
  for (int i = 0; i < ARRAY_SIZE; i++){
    filtered_sensor_value += sensor_value[i]; 
  }
  
  filtered_sensor_value /= ARRAY_SIZE; 
}

// handles async sweeping pan servo 
void move_pan(uint8_t max_pos){
  if (pan_pos == max_pos){
    return; 
  }
  if (first_time_pan){
    start_time_pan = millis(); 
    first_time_pan = false; 
  }
  // map ellapsed time to position 

  pan_pos = map(millis()-start_time_pan, 0, TIME_TO_MAX_POS, 0, max_pos); 
  pan_servo.write(pan_pos); 
  
}

// handles async sweeping tilt servo // TODO 
void move_tilt(){
  return; 
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200); 
  pan_servo.attach(servo_pan_pin); 
  tilt_servo.attach(servo_tilt_pin); 

}

void loop() {
  read_data_and_filter(); 
  delay(5); 
//  move_pan(180); 
//  move_tilt(); 


  
//  Serial.println(filtered_sensor_value); 

  Serial.print("top:"); Serial.print(700); Serial.print(", ");
  Serial.print("btm:"); Serial.print(50); Serial.print(", ");
  Serial.print("data:"); Serial.print(filtered_sensor_value); Serial.print(", ");
  Serial.println();
  
}
