/* 
Jerard Cabeguin 
CPE 301 Final Project
May 10, 2024
*/
#include <LiquidCrystal.h> //LCD 
#include <DHT11.h>
DHT11 dht11(26);
#include<Wire.h>
#include <RTClib.h> //Clock module library
RTC_DS1307 rtc;
#include <Stepper.h>

#define DHTpin 26 //Temperature and Humidity Pin
#define WPower 27 //Power Pin for Water Sensor
#define WSignal A5 //Signal Pin for Water Sensor
#define RDA 0x80
#define TBE 0x20
#define tempThr 20 //Temperature Threshold
#define LowWater 110 //Low Water Level Threshold

// Timer Pointers
volatile unsigned char *myTCCR1A  = (unsigned char *) 0x80;
volatile unsigned char *myTCCR1B  = (unsigned char *) 0x81;
volatile unsigned char *myTCCR1C  = (unsigned char *) 0x82;
volatile unsigned char *myTIMSK1  = (unsigned char *) 0x6F;
volatile unsigned char *myTIFR1   = (unsigned char *) 0x36;
volatile unsigned int  *myTCNT1   = (unsigned  int *) 0x84;

//Water Sensor
volatile unsigned char* my_ADMUX = (unsigned char*) 0x7C;
volatile unsigned char* my_ADCSRB = (unsigned char*) 0x7B;
volatile unsigned char* my_ADCSRA = (unsigned char*) 0x7A;
volatile unsigned int* my_ADC_DATA = (unsigned int*) 0x78;

//Water Sensor Power Pointers
volatile unsigned char *portA5 = (unsigned char*) 0x22;
volatile unsigned char *ddrA5  = (unsigned char*) 0x21;
volatile unsigned char *pinA5  = (unsigned char*) 0x20;

//ADC Pointers
volatile unsigned char *myUCSR0A = (unsigned char *) 0x00C0;
volatile unsigned char *myUCSR0B = (unsigned char *) 0x00C1;
volatile unsigned char *myUCSR0C = (unsigned char *) 0x00C2;
volatile unsigned int  *myUBRR0  = (unsigned int *)  0x00C4;
volatile unsigned char *myUDR0   = (unsigned char *) 0x00C6;

 //GPIO Interrupt
volatile unsigned char* portD0 = (unsigned char*) 0x2B;
volatile unsigned char* ddrD0  = (unsigned char*) 0x2A;
volatile unsigned char* pinD0  = (unsigned char*) 0x29;

//Button Pointers (Vent Control) 50(PB3) and 51 (PB2) 52(PB1)
volatile unsigned char *pinB3  = (unsigned char*) 0x23;
volatile unsigned char *ddrB3  = (unsigned char*) 0x24;
volatile unsigned char *portB3 = (unsigned char*) 0x25;

volatile unsigned char *pinB2  = (unsigned char*) 0x23;
volatile unsigned char *ddrB2  = (unsigned char*) 0x24;
volatile unsigned char *portB2 = (unsigned char*) 0x25;

volatile unsigned char *pinB1  = (unsigned char*) 0x23;
volatile unsigned char *ddrB1  = (unsigned char*) 0x24;
volatile unsigned char *portB1 = (unsigned char*) 0x25;

//DC Motor IC Pointers 
volatile unsigned char *portG1  = (unsigned char*) 0x34;
volatile unsigned char *ddrG1   = (unsigned char*) 0x33;
volatile unsigned char *pinG1   = (unsigned char*) 0x32;

volatile unsigned char *portG0 = (unsigned char*) 0x34;
volatile unsigned char *ddrG0  = (unsigned char*) 0x33;
volatile unsigned char *pinG0  = (unsigned char*) 0x32;

volatile unsigned char *portL7 = (unsigned char*) 0x10B;
volatile unsigned char *ddrL7  = (unsigned char*) 0x10A;
volatile unsigned char *pinL7  = (unsigned char*) 0x109;

//GPIO Yellow (22), Green (23), Red (24), Blue (25)
volatile unsigned char *pinA0  = (unsigned char*) 0x20;
volatile unsigned char *ddrA0  = (unsigned char*) 0x21;
volatile unsigned char *portA0 = (unsigned char*) 0x22;

volatile unsigned char *pinA1  = (unsigned char*) 0x20;
volatile unsigned char *ddrA1  = (unsigned char*) 0x21;
volatile unsigned char *portA1 = (unsigned char*) 0x22;

volatile unsigned char *pinA2  = (unsigned char*) 0x20;
volatile unsigned char *ddrA2  = (unsigned char*) 0x21;
volatile unsigned char *portA2 = (unsigned char*) 0x22;

volatile unsigned char *pinA3  = (unsigned char*) 0x20;
volatile unsigned char *ddrA3  = (unsigned char*) 0x21;
volatile unsigned char *portA3 = (unsigned char*) 0x22;

//LCD Screen Initialization
const int RS = 11, EN = 12, D4 = 2, D5 = 3, D6 = 4, D7 = 5;
LiquidCrystal lcd(RS, EN, D4, D5, D6, D7); 

//Stepper Motor Steps Per Revolution and Initialization
const int steps = 2038;
Stepper myStepper(steps, 30, 31, 32, 33);

//One minute myDelay for the LCD
unsigned long previousMillis = 0;
const long interval = 60000;

//DHT11 Temperature and Humidity Value Initializaiton
int temp = 0;
int humi = 0;
int ht = 0;

int Wvalue = 0; //Variable to store the value of the water sensor
int mode = 0; //Variable to know which mode the program is at. 0 = DISABLED (YELLOW); 1 = IDLE (GREEN); 2 = ERROR (RED); 3 RUNNING (BLUE)
int ventButton = 0;
void setup(){
  U0init(9600); //Initialize the Serial port
  adc_init(); //Sets up the ADC

  myStepper.setSpeed(10); //Speed of how fast the motor takes its steps
  attachInterrupt(digitalPinToInterrupt(20), start, RISING); //sets up the attachInterrupt

  //Initialize the LCD 
  lcd.clear();
  lcd.begin(16, 2);// Sets up the number of columns and rows.
  lcd.setCursor(0,0);

  //Vent Buttons Setup
  *ddrB2 &= ~(0x01 << 2); //Pin 51 Button
  *portB2 |= (0x01 << 2);
  *ddrB3 &= ~(0x01 << 3); //Pin 50 Button
  *portB3 |= (0x01 << 3);
  *ddrB1 &= ~(0x01 << 1); //Pin 52 Button
  *portB1 |= (0x01<<1);

  //Water Sensor Setup
  *ddrA5 |= (0x01 << 5); //Configures the water sensor power pin as an output. 
  *portA5 &= ~(0x01 << 5); //Clears bit 4 in the register and sets to 0. Turns off the output signal.

  //LED Setup
  *ddrA0 |= (0x01 << 0); //Sets the Yellow LED as an output (Pin 22)
  *ddrA1 |= (0x01 << 1); //Sets the Green LED as an ouput (Pin 23)
  *ddrA2 |= (0x01 << 2); //Sets the Red LED as an output (Pin 24)
  *ddrA3 |= (0x01 << 3); //Sets the Blue LED as an output (Pin 25)

  //Motor Setup
  *ddrG1 |= (0x01 << 1); //Pin 40 (PG1) set as an output
  *ddrG0 |= (0x01 << 0); //Pin 41 (PG0) set as an output
  *ddrL7 |= (0x01 << 7); //Pin 42 (PL7) set as an output

  //Interrupt Setup
  *ddrD0 &= ~(0x01<<0);

  mode = 0; //Disabled State (Yellow LED)
}

void loop(){
  if(mode !=0){
    TH();
    wSensor();
    temp = tSensor();
    //Stepper Motor control (Works in every state except Error)
    if(mode != 3){
      if((*pinB3 & (0x01 << 3)) && ventButton == 0){
        myStepper.step(steps/4);
        ventButton = 1;
      }
      else if((*pinB3 & (0x01 << 3)) && ventButton == 1){
      myStepper.step(-steps/4);
      ventButton = 0;
      }
    }
  }
  if(mode == 0){ // DISABLED STATE (YELLOW LED ON)
   lcd.clear();
    //Turn yellow LED ON
    *portA0 |= (0x01<<0);
    
    //Turn other LEDs OFF
    *portA1 &= ~(0x01 << 1);
    *portA2 &= ~(0x01 << 2);
    *portA3 &= ~(0x01 << 3);
    //Turn Fan OFF
    *portG1 &= ~(0x01 << 1);
    *portG0 &= ~(0x01 << 0);
    *portL7 &= ~(0x01 << 7);

    if((*pinB3 & (0x01 << 3)) && ventButton == 0){
        myStepper.step(steps/4);
        ventButton = 1;
      }
      else if((*pinB3 & (0x01 << 3)) && ventButton == 1){
      myStepper.step(-steps/4);
      ventButton = 0;
      }
    
    //When start button is pressed, system starts and goes to idle. 
    if((*pinB2 & (0x01 <<2))){
      mode = 1;
      myDelay(1000);
    }
  }
  else if(mode == 1){ //Condition to check temperature
    if(temp < tempThr){ // System will be idle if the temperature is below the Temperature threshold
      mode = 2; 
    }
    else{ //If the system is higher than the temperature threshold, then the system will run. 
      mode = 4;
    }
  }
  else if(mode == 2){ //IDLE STATE (GREEN LED ON)
    //Turn green LED ON
    *portA1 |= (0x01 << 1);
    //Turn other LEDs OFF
    *portA0 &= ~(0x01 << 0);
    *portA2 &= ~(0x01 << 2);
    *portA3 &= ~(0x01 << 3);
    
    //Turn Fan OFF
    *portG1 &= ~(0x01 << 1);
    *portG0 &= ~(0x01 << 0);
    *portL7 &= ~(0x01 << 7);

    if(temp > tempThr){ //If the temperature is greater than the threshold, then the system will run. 
      mode = 4;
    }

    if((*pinB1 & (0x01 << 1))){ //If the stop button is pressed, then the system will be disabled. 
      mode = 0;
    }
  }
  else if(mode == 3){ //ERROR STATE (RED LED ON)
    //Turn RED LED ON
    *portA2 |= (0x01 << 2);
    //Turn other LEDs OFF
    *portA0 &= ~(0x01 << 0);
    *portA1 &= ~(0x01 << 1);
    *portA3 &= ~(0x01 << 3);

    error();
    
    //Turn Fan OFF
    *portG1 &= ~(0x01 << 1);
    *portG0 &= ~(0x01 << 0);
    *portL7 &= ~(0x01 << 7);

    if((*pinB2 & (0x01 <<2))){ //If the start/restart button is pressed, the button will go back into its idle state. 
      lcd.clear();
      mode = 2;
      myDelay(1000);
    }
    if((*pinB1 & (0x01 << 1))){ //If the stop button is pressed, then the system will be disabled. 
      mode = 0;
    }

  }
  else if(mode == 4){ //RUNNING STATE (BLUE LED ON)
    //Turn BLUE LED ON
    *portA3 |= (0x01<<3);
    //Turn other LEDs OFF
    *portA0 &= ~(0x01 << 0);
    *portA1 &= ~(0x01 << 1);
    *portA2 &= ~(0x01 << 2);

    //Turn Fan OFF
    *portG1 |= (0x01 << 1);
    *portG0 |= (0x01 << 0);
    *portL7 &= ~(0x01 << 7);
    if((*pinB1 & (0x01 << 1))){ //If the stop button is pressed, then the system will be in its disabled state. 
      mode = 0;
    }
    
  }
  myDelay(1000);
  U0putchar('\n');
}

void U0init(unsigned long U0baud){
 unsigned long FCPU = 16000000;
 unsigned int tbaud;
 tbaud = (FCPU / 16 / U0baud - 1);
 // Same as (FCPU / (16 * U0baud)) - 1;
 *myUCSR0A = 0x20;
 *myUCSR0B = 0x18;
 *myUCSR0C = 0x06;
 *myUBRR0  = tbaud;
}
unsigned char U0kbhit(){
  return *myUCSR0A & RDA;
}
unsigned char U0getchar(){
  return *myUDR0;
}
void U0putchar(unsigned char U0pdata){
  while((*myUCSR0A & TBE)==0);
  *myUDR0 = U0pdata;
}

void adc_init(){
  // setup the A register
  *my_ADCSRA |= 0b10000000; 
  *my_ADCSRA &= 0b11011111; 
  *my_ADCSRA &= 0b11110111; 
  *my_ADCSRA &= 0b11111000; 
  // setup the B register
  *my_ADCSRB &= 0b11110111; 
  *my_ADCSRB &= 0b11111000; 
  // setup the MUX Register
  *my_ADMUX  &= 0b01111111; 
  *my_ADMUX  |= 0b01000000; 
  *my_ADMUX  &= 0b11011111; 
  *my_ADMUX  &= 0b11100000; 
}
unsigned int adc_read(unsigned char adc_channel_num){
  // clear the channel selection bits (MUX 4:0)
  *my_ADMUX  &= 0b11100000;
  // clear the channel selection bits (MUX 5)
  *my_ADCSRB &= 0b11110111;
  // set the channel number
  if(adc_channel_num > 7){
    // set the channel selection bits, but remove the most significant bit (bit 3)
    adc_channel_num -= 8;
    // set MUX bit 5
    *my_ADCSRB |= 0b00001000;
  }
  // set the channel selection bits
  *my_ADMUX  += adc_channel_num;
  // set bit 6 of ADCSRA to 1 to start a conversion
  *my_ADCSRA |= 0x40;
  // wait for the conversion to complete
  while((*my_ADCSRA & 0x40) != 0);
  // return the result in the ADC data register
  return *my_ADC_DATA;
}


void myDelay(unsigned int freq){
  //calc period
  double period = 1.0/double(freq);
  // 50% duty cycle
  double half_period = period/2.0f;
  // clock period def
  double clk_period = 0.0000000625;
  //calc tick
  unsigned int ticks = half_period/clk_period;
  // stop the timer
  *myTCCR1B &= 0xF8;
  // set the counts
  *myTCNT1 = (unsigned int)(65536 - ticks);
  // start the timer
  *myTCCR1A = 0x0;
  *myTCCR1B |= 0b00000001;
  //wait for overflow
  while((*myTIFR1 & 0x01)==0); // 0b 0000 0000
  //stop the timer
  *myTCCR1B &= 0xF8;
  //rest TOV
  *myTIFR1 |= 0x01;
}
void TH(){ //Reads the temperature and humidity and displays the values onto the LCD screen. 
  unsigned long currentMillis = millis();

  if(currentMillis - previousMillis >=interval){
    previousMillis = currentMillis;

  if(mode != 0){
    ht = dht11.readTemperatureHumidity(temp, humi);
    lcd.setCursor(0,0);
    lcd.print("Temp: ");
    lcd.print(temp);
    lcd.print((char)223);
    lcd.print(("C"));
    lcd.setCursor(0,1);
    lcd.print("Humidity: ");
    lcd.print(humi);
    lcd.print("%");
    }
  }
}

void wSensor(){// Sensor for the water level and determines whether the system should go into the error state. 
  *portA5 |= (0x01 << 5);
  unsigned int wLevel = adc_read(5);
  Serial.print("Water Level: ");
  Serial.print(wLevel);
  if(wLevel < 150){
    mode = 3;
  }
  return mode;
}

unsigned int tSensor(){ //Reads the temperature from the DHT11
  unsigned int temperature = dht11.readTemperature();
  return temperature;
}

void dis(){
  mode = 0;
  lcd.clear();
}
void start(){
  if(mode ==0){
    mode = 1;
  }
}
void reset(){
  if(!(*pinB2 & (0x01 << 2))){
    setup();
  }
}
void error(){
  *portB2 |= (0x01 << 2);
  if(mode == 3){
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Water Level is");
    lcd.setCursor(3, 2);
    lcd.print("too low");
  }
}