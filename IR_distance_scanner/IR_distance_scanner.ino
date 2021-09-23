/* Charlotte, Adi, Gabby */
#include <Servo.h>

#define ROLL_FILTER_SZ            20    // size of rolling / moving avg filter for scan data
#define SCANNER_POLL_RATE         20    // in Hz

#define SLOW 
//#define FAST

#ifdef  SLOW
#define SERVO_SPEED               .018  // in deg/ms
#else
#define SERVO_SPEED               .024  // in deg/ms
#endif

#define PAN_INCR                  2
#define TILT_INCR                 2

#define PAN_MIN_RNG               20
#define PAN_MAX_RNG               160
#define TILT_MIN_RNG              20
#define TILT_MAX_RNG              160

enum FSM {
  WAITING, 
  HOMING, 
  SCANNING,
  FAULT,
};

FSM system_state = HOME; 

volatile uint16_t sensor_value[ROLL_FILTER_SZ]  = { 0 }; 
volatile uint16_t filtered_sensor_value         = 0; 
volatile uint8_t  pointer                       = 0; 
volatile float    distance                      = 0.0; 


const int ir_scanner_pin              = A0; 

const int servo_pan_pin               = 9;  
const int servo_tilt_pin              = 10; 

uint8_t   pan_pos                     = 0; 
uint8_t   tilt_pos                    = 0; 

uint8_t   total_time_pan              = 0; 
uint8_t   total_time_tilt             = 0; 

uint8_t   start_pos_pan               = 0; 
uint8_t   start_pos_tilt              = 0; 

uint32_t  start_time_pan              = 0; 
bool      first_time_pan              = true; 


uint32_t  start_time_tilt             = 0; 
bool      first_time_tilt             = true; 

uint32_t  start_time_data             = 0; 
bool      first_time_data             = true; 

Servo pan_servo; 
Servo tilt_servo; 


/* Handles async ADC polling & FP distance calculations */
void read_data_and_filter(){
  uint32_t current_time = millis(); 
  if (first_time_data){
    start_time_data = current_time; 
    first_time_data = false; 
  } 

//  only read in data at 5ms intervals
  if (current_time-start_time_data - (1000/SCANNER_POLL_RATE) >= 0){
    start_time_data = current_time; 
    
    // read in data
    sensor_value[pointer] = analogRead(ir_scanner_pin); 
    pointer = (pointer + 1) % ROLL_FILTER_SZ; 
    
    // apply moving average filter 
    filtered_sensor_value = 0; 
    
    for (int i = 0; i < ROLL_FILTER_SZ; i++){
      filtered_sensor_value += sensor_value[i]; 
    }
    
    filtered_sensor_value /= ROLL_FILTER_SZ; 

    distance = (((float)filtered_sensor_value) - 644.0) / (-19.9);
  } else {
    return; 
  }
}


// handles async sweeping pan servo 
bool move_pan(uint8_t target_pos){
  if ((millis()-start_time_pan > total_time_pan) and pan_pos == target_pos){
    first_time_pan = true; 
    start_pos_pan = pan_pos; 
    return true; 
  }
  if (first_time_pan){
    start_time_pan = millis(); 
    first_time_pan = false; 
    total_time_pan = (abs(target_pos - pan_pos) / SERVO_SPEED); 
    start_pos_pan  = pan_pos; 
  }
  // map ellapsed time to position 

  pan_pos = map(millis()-start_time_pan, 0, total_time_pan, 0, target_pos); 
  pan_servo.write(pan_pos); 
  return false; 
  
}

// handles async sweeping tilt servo
bool move_tilt(uint8_t target_pos){
  if ((millis()-start_time_tilt > total_time_tilt) and (tilt_pos == target_pos)){
    first_time_tilt = true; 
    start_pos_tilt = tilt_pos; 
    return true; 
  }
  if (first_time_tilt){
    start_time_tilt = millis(); 
    first_time_tilt = false; 
    total_time_tilt = (abs(target_pos - tilt_pos) / SERVO_SPEED); 
    start_pos_tilt  = tilt_pos; 
  }
  // map ellapsed time to position 

  tilt_pos = map(millis()-start_time_tilt, 0, total_time_tilt, 0, target_pos); 
  tilt_servo.write(tilt_pos); 
  return false; 
}

/* sends packet of 3 bytes over serial: pan angle, tilt angle, distance measurement */
void send_packet(){  
  Serial.write({pan_pos, tilt_pos, distance}, 3); 
}

/* Coordinates dual servo motion to complete full range of motion scan without blocking */
void scan(){
  /* execute a simple up down sweep for both servos */
  
  // collect data over entire SCAN state at 5 Hz for rolling filter
  read_data_and_filter(); 

  // increment PAN within tilt, PAN essentially monitors progress 
  if (tilt_climbing == true){
    // tilt climbing 
    if (tilt_pos < TILT_MAX_RNG){
      if (move_tilt(start_pos_tilt + TILT_INCR)){
        // move has completed, send data
        send_packet(); 
      }
    } else {
        if (pan_pos < PAN_MAX_RNG){
          // increment PAN
          if move_pan(start_pos_pan + PAN_INCR){
            // change state & send packet 
            send_packet(); 
            tilt_climbing = true; 
          }
        } else {
          // COMPLETE
          send_packet(); 
          exit_scan(); 
        }
    }
    
  } else {
    // tilt_falling
    if (tilt_pos> MIN_TILT_RNG){
      if (move_tilt(start_pos_tilt - TIL_INCR)){
        // move has completed, send data
        send_packet(); 
      }
    } else {
      if (pan_pos < PAN_MAX_RNG){
        // increment PAN
        if move_pan(start_pos_pan + PAN_INCR){
          // change state & send packet 
          send_packet(); 
          tilt_climbing = true; 
        }
      } else {
        // COMPLETE 
        send_packet(); 
        exit_scan(); 
    } 
  }
  }
  return;
}


/* called on scan completion or exit command */
void exit_scan(){
  return;   
}

/* return servo to home state */
void return_to_home(){
  if (move_pan(PAN_MIN_RANGE) and move_tilt(TILT_MIN_RANGE)){ // TODO does this allow async operation 
    system_state = WAITING; 
  }
}


void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200); 
  pan_servo.attach(servo_pan_pin); 
  tilt_servo.attach(servo_tilt_pin); 

}

void loop() {

  process_serial_traffic(); 

  /* FSM */
  switch(system_state){
    case WAITING: 
      // do nothing. 
      break; 
    case HOMING: 
      return_to_home(); 
      break;
      
    case SCANNING:
      scan(); 
      break;
      
    default: 
      break; 
  } 
  move_pan(180); 
  move_tilt(180); 
   

//  Serial.print("top:"); Serial.print(700); Serial.print(", ");
//  Serial.print("btm:"); Serial.print(50); Serial.print(", ");
//  Serial.print("data:"); Serial.print(filtered_sensor_value); Serial.print(", ");
  Serial.print("top:"); Serial.print(24); Serial.print(", ");
  Serial.print("btm:"); Serial.print(8); Serial.print(", ");
  Serial.print("data:"); Serial.print(distance); Serial.print(", ");
  Serial.println();
  
}
