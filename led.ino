void ISR_led_flash() {
uint8_t flashChar = led_digit_pattern[led_flash_stage / 8];
uint8_t flashdigit = led_flash_stage % 8;
uint8_t lastdigit;
uint8_t ledoutput;
if (beep_endtime < millis() && beep_endtime != 0) {
                              digitalWrite(BUZZER_PIN,LOW);
                              beep_endtime = 0;
                             }
                             
if (flashdigit == 0) lastdigit = 6; else lastdigit = flashdigit - 2;

if (led_flash_stage == 0) {
                           digitalWrite(led_select_pin_array[2],LOW);
                           digitalWrite(led_select_pin_array[0],HIGH);
                          }
                          
if (led_flash_stage == 8) {
                           digitalWrite(led_select_pin_array[0],LOW);
                           digitalWrite(led_select_pin_array[1],HIGH);
                          }
                          
if (led_flash_stage == 16) {
                           digitalWrite(led_select_pin_array[1],LOW);
                           digitalWrite(led_select_pin_array[2],HIGH);
                          }
                          
digitalWrite(led_digit_pin_array[lastdigit],HIGH);
digitalWrite(led_digit_pin_array[lastdigit+1],HIGH);

flashChar >>= (7 - (flashdigit+1));

digitalWrite(led_digit_pin_array[flashdigit+1],(flashChar & 0x1)^0x1);
flashChar >>= 1;
digitalWrite(led_digit_pin_array[flashdigit],(flashChar & 0x1)^0x1);

led_flash_stage+= 2;
if (led_flash_stage == 24) led_flash_stage = 0;
}


void displayvoltage(uint16_t volt){
  char str[4];
  if (volt > 99){
  snprintf(str, 4, "%d", volt);
  led_digit_pattern[0]=set_led_digit(str[0]);
  led_digit_pattern[1]=set_led_digit(str[1]) + 1;
  led_digit_pattern[2]=set_led_digit(str[2]);
  }
  else if (volt > 9) {
  snprintf(str, 3, "%d", volt);
  led_digit_pattern[0]=set_led_digit(' ');
  led_digit_pattern[1]=set_led_digit(str[0]) + 1;
  led_digit_pattern[2]=set_led_digit(str[1]);
  }
}
void display_battery_type(uint8_t selector){
    switch (selector) {
    case 0:
          led_digit_pattern[0]=set_led_digit('6');
          led_digit_pattern[1]=set_led_digit('S');
          led_digit_pattern[2]=set_led_digit(' ');
          break;
    case 1:
          led_digit_pattern[0]=set_led_digit('4');
          led_digit_pattern[1]=set_led_digit('S');
          led_digit_pattern[2]=set_led_digit(' ');
          break;
    case 2:
          led_digit_pattern[0]=set_led_digit('3');
          led_digit_pattern[1]=set_led_digit('S');
          led_digit_pattern[2]=set_led_digit(' ');
          break;
    case 3:
          led_digit_pattern[0]=set_led_digit('P');
          led_digit_pattern[1]=set_led_digit('B') + 1;
          led_digit_pattern[2]=set_led_digit(' ');
          break;
    }
}
void displaystring(char *str){
  led_digit_pattern[0]=set_led_digit(str[0]);
  led_digit_pattern[1]=set_led_digit(str[1]);
  led_digit_pattern[2]=set_led_digit(str[2]) + 1;
}

uint8_t set_led_digit(uint8_t character)
{
  switch (character) {
    case '0':return(0xFC);
    case '1':return(0x60);
    case '2':return(0xDA);
    case '3':return(0xF2);
    case '4':return(0x66);
    case '5':return(0xB6);
    case '6':return(0xBE);
    case '7':return(0xE0);
    case '8':return(0xFE);
    case '9':return(0xF6);
    case 'A':return(0xEE);
    case 'B':return(0x3E);
    case 'C':return(0x9C);
    case 'D':return(0x7A);
    case 'E':return(0x9E);
    case 'F':return(0x8E);
    case 'G':return(0xBC);
    case 'H':return(0x2E);
    case 'I':return(0x20);
    case 'J':return(0x78);
    case 'K':return(0xAE);
    case 'L':return(0x1C);
    case 'M':return(0xEC);
    case 'N':return(0x2A);
    case 'O':return(0x3A);
    case 'P':return(0xCE);
    case 'Q':return(0xDE);
    case 'R':return(0x0A);
    case 'S':return(0x36);
    case 'T':return(0x1E);
    case 'U':return(0x38);
    case 'V':return(0x7C);
    case 'W':return(0x7E);
    case 'X':return(0x6E);
    case 'Y':return(0x76);
    case 'Z':return(0xD8);
    case '-':return(0x02);
    case '=':return(0x90);
    case '_':return(0x10);
    case ' ':return(0x00);
    default:return(0x00);
  }
}
