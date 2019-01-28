#include <MsTimer2.h>
#include <avr/sleep.h>
#include "EEPROM.h"

//CONFIG
#define VOLTAGE_DIVIDER 27030 //(15kOHM 68Kohm 5v 10bit)

#define DESTINATION_VOLTAGE_1 198 //6s
#define DESTINATION_VOLTAGE_2 132 //4s
#define DESTINATION_VOLTAGE_3 99  //3s
#define DESTINATION_VOLTAGE_4 110 //Pb

#define DEFAULT_DESTINATION_VOLTAGE 3

//PIN ASSIGN
//aki d9 d12 d11
#define SEG_A_PIN 19 //A5
#define SEG_B_PIN 17 //A3
#define SEG_C_PIN  1
#define SEG_D_PIN  3
#define SEG_E_PIN  4
#define SEG_F_PIN 18 //A4
#define SEG_G_PIN  0
#define SEG_DP_PIN 2
 
#define SELECT_LED_1_PIN 6
#define SELECT_LED_2_PIN 15 //A1
#define SELECT_LED_3_PIN 16 //A2
 
#define DISCHARGE_GATE_PIN 10
#define NOT_USE_PIN_1 9
#define NOT_USE_PIN_2 11
#define NOT_USE_PIN_3 12

#define VOLTAGE_MONITOR_PIN 14 // A0
 
#define SELECT_SWITCH_PIN 8
#define ENTER_SWITCH_PIN 7

#define BUZZER_PIN 13
#define DISCHARGE_LED_PIN 5


uint16_t volt_div;
uint8_t destination_voltage_selector;

uint16_t select_destination_voltage[4] = {DESTINATION_VOLTAGE_1,
                                          DESTINATION_VOLTAGE_2,
                                          DESTINATION_VOLTAGE_3,
                                          DESTINATION_VOLTAGE_4};

volatile uint8_t led_select_pin_array[3] = {SELECT_LED_1_PIN,
                                            SELECT_LED_2_PIN,
                                            SELECT_LED_3_PIN};
 
volatile uint8_t led_digit_pin_array[8] = {SEG_A_PIN,
                                           SEG_B_PIN,
                                           SEG_C_PIN,
                                           SEG_D_PIN,
                                           SEG_E_PIN,
                                           SEG_F_PIN,
                                           SEG_G_PIN,
                                           SEG_DP_PIN};

uint16_t not_use_pin_array[3] = {NOT_USE_PIN_1,
                                 NOT_USE_PIN_2,
                                 NOT_USE_PIN_3};

volatile uint8_t led_digit_pattern[3] = {0,0,0};
 
volatile uint8_t led_flash_stage = 0;

volatile uint32_t beep_endtime = 0;

uint16_t destination_voltage;

void setup() {
  // put your setup code here, to run once:
  //pinMode(POWER_GATE_PIN,OUTPUT);
  //switch_on();

  pinMode(DISCHARGE_GATE_PIN,OUTPUT);
  pinMode(DISCHARGE_LED_PIN,OUTPUT);
  pinMode(BUZZER_PIN,OUTPUT);
  
  

  for (uint8_t i=0; i <= 7; i++)  pinMode(led_digit_pin_array[i],OUTPUT);
  for (uint8_t i=0; i <= 7; i++)  digitalWrite(led_digit_pin_array[i],HIGH);
  for (uint8_t i=0; i <= 2; i++)  pinMode(led_select_pin_array[i],OUTPUT);
  for (uint8_t i=0; i <= 2; i++)  digitalWrite(led_select_pin_array[i],LOW);
  for (uint8_t i=0; i <= 2; i++)  pinMode(not_use_pin_array[i],OUTPUT);
  for (uint8_t i=0; i <= 2; i++)  digitalWrite(not_use_pin_array[i],LOW);  
  
  
  pinMode(VOLTAGE_MONITOR_PIN,INPUT);
  analogReference(DEFAULT); //1023=5.0v
 
  pinMode(SELECT_SWITCH_PIN,INPUT_PULLUP);
  pinMode(ENTER_SWITCH_PIN,INPUT_PULLUP);
  
  if(readEeprom() == 0) {
                     volt_div = VOLTAGE_DIVIDER;
                     destination_voltage_selector = DEFAULT_DESTINATION_VOLTAGE;
                     writeEeprom();
                     beep_on(500);
                    }
  destination_voltage = select_destination_voltage[destination_voltage_selector];
  
  MsTimer2::set(2,ISR_led_flash);
  MsTimer2::start();
  
  if (selectButtonPushed()) voltage_calib();
  if (enterButtonPushed()) select_default_destination_voltage();

while(1) {startup();}
}
 
void loop() {}

void startup(){
  uint16_t voltage;
  uint8_t selector = destination_voltage_selector;
  uint8_t switch_state = 0;
  while(1){
    voltage = get_voltage();
    if (voltage <= destination_voltage) switch_off();
    //displaystring('ACT');
    led_digit_pattern[0]=set_led_digit('A');
    led_digit_pattern[1]=set_led_digit('C');
    led_digit_pattern[2]=set_led_digit('T') + 1;

    switch_state = get_switch_state(1000);
    if (switch_state == 2) {
                            discharge();
                           }
    switch_state = 0;
    
    voltage = get_voltage();
    displayvoltage(voltage);

    switch_state = get_switch_state(1000);
    if (switch_state == 2) {
                            discharge();
                           }
    switch_state = 0;

    //displaystring('DST');
    display_battery_type(selector);

    switch_state = get_switch_state(1000);
    if (switch_state == 2) {
                            discharge();
                           }
    switch_state = 0;

    displayvoltage(destination_voltage);

    switch_state = get_switch_state(1000);
    if (switch_state == 2) {
                            discharge();
                           }
    switch_state = 0;
  }
}

void discharge(){
    uint16_t voltage;
    uint8_t switch_state = 0;
    beep_on(2000);
    digitalWrite(DISCHARGE_GATE_PIN,HIGH);
    digitalWrite(DISCHARGE_LED_PIN,HIGH);
    

    while(1){
      voltage = get_voltage();
      if (voltage <= destination_voltage) switch_off();
    
      //displaystring('ACT');
     led_digit_pattern[0]=set_led_digit('A');
     led_digit_pattern[1]=set_led_digit('C');
     led_digit_pattern[2]=set_led_digit('T') + 1;
     delay(1000);

     displayvoltage(voltage);
     switch_state = get_switch_state(1000);
     if (switch_state != 0) {
      digitalWrite(DISCHARGE_GATE_PIN,LOW);
      digitalWrite(DISCHARGE_LED_PIN,LOW);
      
      return;
     }
     
      //displaystring('DST');
     display_battery_type(destination_voltage_selector);
     switch_state = get_switch_state(1000);
     if (switch_state != 0) {
        digitalWrite(DISCHARGE_GATE_PIN,LOW);
        digitalWrite(DISCHARGE_LED_PIN,LOW);
        return;
     }
     
      displayvoltage(destination_voltage);
      switch_state = get_switch_state(1000);
      if (switch_state != 0) {
        digitalWrite(DISCHARGE_GATE_PIN,LOW);
        digitalWrite(DISCHARGE_LED_PIN,LOW);
        return;
      }
  }
}
 


uint16_t get_voltage(){
uint32_t volt;
volt = analogRead(VOLTAGE_MONITOR_PIN);
volt *= volt_div;
volt /= 100000;
return(volt);
}

uint8_t get_switch_state(uint16_t wait_time){
uint32_t wait_limit = millis() + wait_time;
while(wait_limit > millis()){
  if (selectButtonPushed()) return(1);
  if (enterButtonPushed()) return(2);
}
return(0);
}

uint8_t selectButtonPushed()
{
  if ( digitalRead(SELECT_SWITCH_PIN) == LOW )
  {
    delay(100);
    if (digitalRead(SELECT_SWITCH_PIN) == LOW) return(1);
  }
  return(0);
}

uint8_t enterButtonPushed()
{
  if ( digitalRead(ENTER_SWITCH_PIN) == LOW )
  {
    delay(100);
    if (digitalRead(ENTER_SWITCH_PIN) == LOW) return(1);
  }
  return(0);
}

void beep_on(uint32_t dulation)
{
beep_endtime = millis() + dulation;
digitalWrite(BUZZER_PIN,HIGH);
}

uint8_t readEeprom()
{
  uint8_t temp[3];
  uint8_t checksum = 0;
      temp[0] = EEPROM.read(0);
      temp[1] = EEPROM.read(1);
      temp[2] = EEPROM.read(2);
      checksum = temp[0] + temp[1] + temp[2];
      if( EEPROM.read(3) == checksum) {
                                       volt_div = (temp[0] << 8)+ temp[1] ;
                                       destination_voltage_selector = temp[2];
                                       return(1);
                                      }
      else return(0);
}

void writeEeprom()
{ 
  uint8_t temp[3];
  uint8_t checksum;
  temp[0] = highByte(volt_div);
  temp[1] = lowByte(volt_div);
  temp[2] = destination_voltage_selector;
  checksum = temp[0] + temp[1] + temp[2];
  EEPROM.write(0,temp[0]);
  EEPROM.write(1,temp[1]);
  EEPROM.write(2,temp[2]);
  EEPROM.write(3,checksum);
}

void voltage_calib()
{
  uint16_t voltage;
  beep_on(500);
  delay(500);
  beep_on(500);
  while(1)
  {
    voltage = get_voltage();
    displayvoltage(voltage);
    if(selectButtonPushed()){
                             volt_div += 10;
                             if (volt_div > 28030) volt_div = 26030;
                            }
    if(enterButtonPushed()){
                            writeEeprom();
                            led_digit_pattern[0]=set_led_digit('S');
                            led_digit_pattern[1]=set_led_digit('A');
                            led_digit_pattern[2]=set_led_digit('V') + 1;
                            beep_on(1000);
                            delay(3000);
                            switch_off();
                           }
  }
}

void select_default_destination_voltage()
{
  uint16_t voltage;
  beep_on(500);
  delay(500);
  beep_on(500);
  delay(500);
  beep_on(500);
  destination_voltage = select_destination_voltage[destination_voltage_selector];
  while(1)
  {
    voltage = get_voltage();
    if(selectButtonPushed()){
                            destination_voltage_selector++;
                            if (destination_voltage_selector == 4) destination_voltage_selector = 0;
                            destination_voltage = select_destination_voltage[destination_voltage_selector];
                           }
    displayvoltage(destination_voltage);
    
    if(enterButtonPushed()){
                            writeEeprom();
                            led_digit_pattern[0]=set_led_digit('S');
                            led_digit_pattern[1]=set_led_digit('A');
                            led_digit_pattern[2]=set_led_digit('V') + 1;
                            beep_on(1000);
                            delay(3000);
                            switch_off();
                           }
  }
}

void switch_off(){
  digitalWrite(DISCHARGE_GATE_PIN,LOW);
  digitalWrite(DISCHARGE_LED_PIN,LOW);
  led_digit_pattern[0]=set_led_digit('O');
  led_digit_pattern[1]=set_led_digit('F');
  led_digit_pattern[2]=set_led_digit('F');
  digitalWrite(BUZZER_PIN,HIGH);
  delay(3000);
  digitalWrite(BUZZER_PIN,LOW);
  MsTimer2::stop();
  for (uint8_t i=0; i <= 7; i++)  digitalWrite(led_digit_pin_array[i],LOW);
  for (uint8_t i=0; i <= 2; i++)  digitalWrite(led_select_pin_array[i],LOW);
  for (uint8_t i=0; i <= 2; i++)  digitalWrite(not_use_pin_array[i],LOW);
  pinMode(SELECT_SWITCH_PIN,INPUT);
  pinMode(ENTER_SWITCH_PIN,INPUT);
  
  //パワーダウンモードの選択とスリープ機能の許可(データシート14.12.1SMCR参照)
  SMCR |= (1 << SM1);// パワーダウンモードに設定
  SMCR |= (1 << SE);// スリープ機能の許可

  //A/Dコンバータの停止(ADC:Analog-to-Digital Converter)の停止(データシート28.9.2. ADCSRA参照)
  //ADCSRA |= (1 << ADEN);  // ON
  ADCSRA &= ~(1 << ADEN); // OFF


  //低電圧検出器(BOD:Brown-Out Detecter)の停止(データシート14.12.2MCUCR参照)
  MCUCR |= (1 << BODSE)|(1 << BODS); // BOD設定変更のため，BODSとBODSEに同時に1を出力
  MCUCR = (MCUCR & ~(1 << BODSE))|(1 << BODS);   // その後4CLK内にBODSに1，BODSEに0を出力してBODをOFF

  asm("sleep"); // その後3クロック周期内にスリープ状態へ移行
}
