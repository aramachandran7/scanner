
#define ARRAY_SIZE                3

uint16_t sensor_value[ARRAY_SIZE] = { 0 }; 
uint16_t filtered_sensor_value    = 0; 
uint8_t pointer                   = 0; 


void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200); 

}

void loop() {
  // read in data
  sensor_value[pointer] = analogRead(A0); 
  pointer = (pointer + 1) % ARRAY_SIZE; 
  

  // apply moving average filter 
  filtered_sensor_value = 0; 
  for (int i = 0; i < ARRAY_SIZE; i++){
    filtered_sensor_value += sensor_value[i]; 
  }
  filtered_sensor_value /= 3; 
  Serial.println(filtered_sensor_value); 
  
}
