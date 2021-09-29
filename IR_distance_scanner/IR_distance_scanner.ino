/* Charlotte, Adi, Gabby */

/*
* Bug fix log, @adi 9.28.2021: 
*   - resolved issue with driving servo forwards and backwards with same code 
*   - resolved issue with mapping between 0 and target_pos, vs start_pos and target_pos
*   - implemented tilt_1_degree, but fixed ~ vs !
*   - fixed async servo functions completing early (wait for servo position & time to complete
*   - fixed async servo function starting new move to same position by checking for exit case before starting new case (within async move_pan and move_tilt fns)
*   - implemented checking for both exact & above max servo position, to send final packet in sweep or not
*   - setup STOP_DELAY to play with
*   
* TODO, @adi 9.28.2021: 
*   - determine & play with appropriate scanning params: 
*     - SERVO_SPEED
*     - STOP_DELAY
*     - TILT, PAN MAX, MIN RNG
*     - TILT, PAN INCR sizes
*     - required changes to filtering setup based on scanning params 
*   - build comms protocol to interface with .py including 
*     - clarify all possible commands 
*     - command completion byte
*     - command start byte / ACK byte 
*     - data format (4 byte buffer?) 
*     - setting settings like overall scan speed / precision 
*     - start scan() function params send back, to 'set expections on .py side', help progress tracking, etc
*   - fix FSM 
*     - draw out bubble diagram, fix and clarify transitions based on .py interface 
*/

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

#define PAN_INCR                  10
#define TILT_INCR                 10

#define PAN_MIN_RNG               40
#define PAN_MAX_RNG               140
#define TILT_MIN_RNG              40
#define TILT_MAX_RNG              140

#define STOP_DELAY                50 // delay in ms after stopping at every point for data recording 

enum FSM { // system state enum 
  WAITING, 
  HOMING, 
  SCANNING,
  FAULT,
};

FSM system_state = HOMING; 

volatile uint16_t sensor_value[ROLL_FILTER_SZ]  = { 0 }; 
volatile uint16_t filtered_sensor_value         = 0; 
volatile uint8_t  pointer                       = 0; 
volatile float    distance                      = 0.0; 

bool      tilt_climbing               = true; 

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

uint8_t   pan_set_point               = PAN_MIN_RNG + PAN_INCR; 
uint8_t   tilt_set_point              = TILT_MIN_RNG + TILT_INCR;

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

//    distance = (((float)filtered_sensor_value) - 644.0) / (-19.9);
  } else {
    return; 
  }
}


// handles async sweeping pan servo --> ideally only some variables from this should be exposed to scope of this file, also add ability to set speed from this function potentially 
bool move_pan(uint8_t target_pos){
  // function call exit case
  if (pan_pos == target_pos and ((millis() - start_time_pan) >= total_time_pan)){
    first_time_pan = true; 
    start_pos_pan = pan_pos; 
    return true; 
  }
  // function call first case
  if (first_time_pan){
    start_time_pan = millis(); 
    start_pos_pan  = pan_pos;
    total_time_pan = (abs(target_pos - start_pos_pan) / SERVO_SPEED); 
    first_time_pan = false; 
  }
  pan_pos = map(millis()-start_time_pan, 0, total_time_pan, start_pos_pan, target_pos);   
  pan_servo.write(pan_pos); 
  return false; 
  
}

// handles async sweeping tilt servo
bool move_tilt(uint8_t target_pos){
  // function call exit case
  if ((tilt_pos == target_pos) and ((millis() - start_time_tilt) >= total_time_tilt)){
    first_time_tilt = true; 
    start_pos_tilt = tilt_pos; 
    return true; 
  }
  
  // function call first case
  if (first_time_tilt){
    start_time_tilt = millis(); 
    start_pos_tilt  = tilt_pos;
    total_time_tilt = (abs(target_pos - start_pos_tilt) / SERVO_SPEED); 
    first_time_tilt = false;   
  }
   
  // map ellapsed time to position 
  tilt_pos = map(millis()-start_time_tilt, 0, total_time_tilt, start_pos_tilt, target_pos); 
  tilt_servo.write(tilt_pos); 
  return false; 
}

/* Helper funciton for scan() to climb 1 degree in tilt */
void tilt_1_degree(bool climbing){
  if (tilt_pos < TILT_MAX_RNG){
    if (move_tilt(tilt_set_point)){
      tilt_climbing = !climbing; // switch into alternate conditional
      send_packet(); 
      tilt_set_point += TILT_INCR; 
      pan_set_point = (climbing) ? (PAN_MAX_RNG - PAN_INCR) : (PAN_MIN_RNG + PAN_INCR); 
      Serial.println("COMPLETED PAN SWEEP & TILT BUMP, SWITCHING DIRECTION");    
//            Serial.print("new pan_set_point: "); Serial.print(pan_set_point); 
      Serial.println(); 
      delay(STOP_DELAY); 
    }
  } else if (tilt_pos == TILT_MAX_RNG){ // scan complete 
    send_packet(); 
    exit_scan(); 
  } else {
    exit_scan(); 
  }
  
}

/* Coordinates dual servo motion to complete full range of motion scan without blocking */
void scan(){
  /* execute a simple up down sweep for both servos */
  
  // collect data over entire SCAN state at 5 Hz for rolling filter
  read_data_and_filter(); 

  if (tilt_climbing){
      if (pan_pos < PAN_MAX_RNG){
        // increment PAN
        if (move_pan(pan_set_point)){ // start_pos_pan + PAN_INCR
          // change state & send packet 
          send_packet(); 
          pan_set_point += PAN_INCR; 
          delay(STOP_DELAY); // for visbility, TODO test if needed for data quality 
        }
      } else if (pan_pos == PAN_MAX_RNG){
//        send packet and reverse direction 
          if (move_pan(PAN_MAX_RNG)){ // move needs to be reset
            send_packet(); 
            tilt_1_degree(tilt_climbing); 
          }
      } else {
            tilt_1_degree(tilt_climbing); 
      }
  } else { // pan falling 
    if (pan_pos > PAN_MIN_RNG){
      // decrement PAN
      if (move_pan(pan_set_point)){ 
        send_packet(); 
        pan_set_point -= PAN_INCR; 
        delay(STOP_DELAY); 
      }
    } else if (pan_pos == PAN_MIN_RNG) {
        send_packet(); // how to avoid repeated packets??
        tilt_1_degree(tilt_climbing); 
    } else {
      tilt_1_degree(tilt_climbing); 
    }
    } 
}


/* called on scan completion or exit command */
void exit_scan(){
//  Serial.write(0xFF); // Writes 255 over serial to indicate scan completion 
  Serial.println("SCANNING COMPLETE >> WAITING"); 
  system_state = WAITING; // HOMING? 
  return;   
}



/* sends packet of 3 bytes over serial: pan angle, tilt angle, distance measurement */
void send_packet(){  
  
  uint8_t data_arr[4] = {pan_pos, tilt_pos}; // stick filtered_sensor_val 
//  Serial.write(data_arr, 4); 


  Serial.print("PAN:"); Serial.print(pan_pos); Serial.print(", ");
  Serial.print("TILT:"); Serial.print(tilt_pos); Serial.print(", ");
  Serial.print("DATA:"); Serial.print(filtered_sensor_value); Serial.print(", ");
  Serial.println();
}


/* return servo to home state */
void return_to_home(){
  if (move_pan(PAN_MIN_RNG) and move_tilt(TILT_MIN_RNG)){ 
    system_state = SCANNING; // TODO switch to waiting down the line  
    Serial.println("HOMING COMPLETE>> STARTING SCAN."); 
    delay(500); 
  }
}

void process_serial_traffic(){
  return; // parses incoming serial data to dictate system state 
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
//  Serial.print("top:"); Serial.print(700); Serial.print(", ");
//  Serial.print("btm:"); Serial.print(50); Serial.print(", ");
//  Serial.print("data:"); Serial.print(filtered_sensor_value); Serial.print(", "); 
}
