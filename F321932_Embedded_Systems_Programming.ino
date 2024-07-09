//Source Code of Library that measures FREERAM
extern unsigned int __heap_start;
extern void *__brkval;

int freeMemory() {
  int free_memory;
  if ((int)__brkval == 0) {
    free_memory = ((int)&free_memory) - ((int)&__heap_start);
  } else {
    free_memory = ((int)&free_memory) - ((int)__brkval);
  }
  return free_memory;
}

//Libraries
#include <Wire.h>
#include <Adafruit_RGBLCDShield.h>
#include <utility/Adafruit_MCP23017.h>
Adafruit_RGBLCDShield lcd = Adafruit_RGBLCDShield();

//Initial times as 0
unsigned long Entry = 0;
unsigned long Exit = 0;

//Defining the Cases
#define A 0
#define S 1
#define T 2
#define L 3
#define R 4

//Up Arrow Design
byte Up_Arrow[] = {
  B00100,
  B01110,
  B11111,
  B00100,
  B00100,
  B00100,
  B00100,
  B00100
};

//Down Arrow Design
byte Down_Arrow[] = {
  B00100,
  B00100,
  B00100,
  B00100,
  B00100,
  B11111,
  B01110,
  B00100
};

//Vehicle Struct
struct Vehicle {
  String Reg_Number;
  char Vehicle_Type;
  String Parking_Location;
  String Payment_Status;
  int Backlight;
  unsigned long Entry_Minutes;
  unsigned long Entry_Hours;
  unsigned long Exit_Minutes;
  unsigned long Exit_Hours;
};


//Key information for Vehicles
const int Max_Vehicles = 10;
Vehicle vehicles[Max_Vehicles];
int Vehicle_Num = 0;

//Function for Case A
void add_Vehicle(String Reg_Number, char Vehicle_Type, String Parking_Location, String Payment_Status, int Backlight, unsigned long Entry_Hours, unsigned long Entry_Minutes, unsigned long Exit_Hours, unsigned long Exit_Minutes) {
  if (Vehicle_Num <= Max_Vehicles) {
    vehicles[Vehicle_Num].Reg_Number = Reg_Number;
    vehicles[Vehicle_Num].Vehicle_Type = Vehicle_Type;
    vehicles[Vehicle_Num].Parking_Location = Parking_Location;
    vehicles[Vehicle_Num].Payment_Status = Payment_Status;
    vehicles[Vehicle_Num].Backlight = Backlight;
    vehicles[Vehicle_Num].Entry_Hours = Entry_Hours;
    vehicles[Vehicle_Num].Entry_Minutes = Entry_Minutes;
    vehicles[Vehicle_Num].Exit_Hours = Exit_Hours;
    vehicles[Vehicle_Num].Exit_Minutes = Exit_Minutes;
    //Number of vehicles increases by 1
    Vehicle_Num = Vehicle_Num + 1;
  }
}

//Function for Case S
void Change_Status(String Reg_Number, String Payment_Status, int Backlight, unsigned long Exit_Hours, unsigned long Exit_Minutes, unsigned long Entry_Minutes, unsigned long Entry_Hours) {
  for (int i = 0; i < Vehicle_Num; i++) {
    if (vehicles[i].Reg_Number == Reg_Number) {
      vehicles[i].Payment_Status = Payment_Status;
      //Colour corresponding to payment status
      vehicles[i].Backlight = Backlight;

      if (Payment_Status == "PD") {
        //New exit times
        vehicles[i].Exit_Hours = Exit_Hours;
        vehicles[i].Exit_Minutes = Exit_Minutes;
      }

      if (Payment_Status == "NPD") {
        //New entry times
        vehicles[i].Entry_Hours = Entry_Hours;
        vehicles[i].Entry_Minutes = Entry_Minutes;
      }
      break;
    }
  }
}

//Function for Case T
void Change_Type(String Reg_Number, char Vehicle_Type) {
  for (int i = 0; i < Vehicle_Num; i++) {
    if (vehicles[i].Reg_Number == Reg_Number) {
      vehicles[i].Vehicle_Type = Vehicle_Type;
    }
  }
}

//Function for Case L
void Change_Location(String Reg_Number, String Parking_Location) {
  for (int i = 0; i < Vehicle_Num; i++) {
    if (vehicles[i].Reg_Number == Reg_Number) {
      vehicles[i].Parking_Location = Parking_Location;
    }
  }
}

//Function for Case R
void Remove_Vehicle(String Reg_Number) {
  int index = -1;
  for (int i = 0; i < Max_Vehicles; i++) {
    if (Reg_Number == vehicles[i].Reg_Number) {
      //Checks where the vehicle is in the array
      index = i;
      break;
    }
  }
  if (index != -1) {
    //The vehicle will be shifted to the right until it reaches the second last element where it is then replaced with the final element
    for (int j = index; j < Vehicle_Num - 1; j++) {
      vehicles[j] = vehicles[j + 1];
    }
    //Refers to the final element becoming empty as it is the same as the second last element we do -1 and it staarts from 0
    vehicles[Vehicle_Num - 1] = Vehicle();

    //Number of vehicles decreases
    Vehicle_Num--;
  }
}


void setup() {
  //Start Timing for Entry and Exit
  Entry = millis();
  Exit = millis();

  //Initialising LCD
  Serial.begin(9600);
  lcd.begin(16, 2);
  lcd.setCursor(0, 0);

  //Creating Up and Down Arrow characters
  lcd.createChar(0, Up_Arrow);
  lcd.createChar(1, Down_Arrow);

  //Initial as 0 for time duration
  unsigned long initial = 0;

  uint8_t buttons = lcd.readButtons();


  //Synchronisation Phase:
  while (true) {
    lcd.setBacklight(5);
    unsigned long current = millis();
    if (current - initial >= 1000) {
      Serial.print("Q");
      initial = current;
    }
    if (Serial.available() > 0) {
      char x = Serial.read();
      if (x == 'X') {
        break;
      }
    }
  }

  //Post Synchronisation:
  Serial.print("UDCHARS,FREERAM,SCROLL\n");
  lcd.setBacklight(7);
}

//Index for first Vehicle and Pressed for SELECT Button timing
int index = 0;
unsigned long pressed = 0;
bool selected = false;

void loop() {

  uint8_t buttons = lcd.readButtons();

  //When SELECT is held then Student ID and FREERAM is outputted
  if (buttons & BUTTON_SELECT) {
    if (selected == false) {
      pressed = millis();
      selected = true;
    } else {
      if (millis() - pressed >= 1000) {
        lcd.clear();
        lcd.setBacklight(5);
        lcd.setCursor(5, 0);
        lcd.print("F321932");
        lcd.setCursor(0, 1);
        lcd.print("FREERAM: ");
        lcd.print(freeMemory());
        selected = false;
      }
    }
  } else {
    if (selected == true) {
      lcd.clear();
      lcd.setBacklight(7);
    }
    selected = false;
  }

  unsigned long Entry = millis();
  unsigned long Exit = millis();

  //Can only scroll through vehicles when there are vehicles present
  if (Vehicle_Num > 0) {

    if (buttons & BUTTON_DOWN) {

      //Ensures index is not negative when scrolling through number of vehicles
      index = (index + 1) % Vehicle_Num;

      //Situation when only 1 vehicle is present so no arrows are displayed
      if (index == 0 && index == Vehicle_Num - 1) {
        lcd.clear();
        lcd.setCursor(1, 1);
        lcd.print(vehicles[index].Vehicle_Type);
        lcd.print(" ");
        lcd.print(vehicles[index].Payment_Status);

        lcd.setCursor(7, 1);
        if (vehicles[index].Entry_Hours < 10) {
          lcd.print("0");
          lcd.print(vehicles[index].Entry_Hours);
          if (vehicles[index].Entry_Minutes < 10) {
            lcd.print("0");
            lcd.print(vehicles[index].Entry_Minutes);
          }
          if (vehicles[index].Entry_Minutes > 10) {
            lcd.print(vehicles[index].Entry_Minutes);
          }
        } else {
          if (vehicles[index].Entry_Hours > 10) {
            lcd.print("0");
            lcd.print(vehicles[index].Entry_Hours);
            if (vehicles[index].Entry_Minutes < 10) {
              lcd.print("0");
              lcd.print(vehicles[index].Entry_Minutes);
            }
            lcd.print("0");
            if (vehicles[index].Entry_Minutes > 10) {
              lcd.print(vehicles[index].Entry_Minutes);
            }
          }
        }
        lcd.setCursor(12, 1);
        if (vehicles[index].Payment_Status == "PD") {
          if (vehicles[index].Exit_Hours < 10) {
            lcd.print("0");
            lcd.print(vehicles[index].Exit_Hours);
            if (vehicles[index].Exit_Minutes < 10) {
              lcd.print("0");
              lcd.print(vehicles[index].Exit_Minutes);
            }
            if (vehicles[index].Exit_Minutes > 10) {
              lcd.print(vehicles[index].Exit_Minutes);
            }
          }
        } else {
          if (vehicles[index].Exit_Hours > 10) {
            lcd.print(vehicles[index].Exit_Hours);
            if (vehicles[index].Exit_Minutes < 10) {
              lcd.print("0");
              lcd.print(vehicles[index].Exit_Minutes);
            }
            if (vehicles[index].Exit_Minutes > 10) {
              lcd.print(vehicles[index].Exit_Minutes);
            }
          }
        }

        lcd.setBacklight(vehicles[index].Backlight);
        lcd.setCursor(1, 0);
        lcd.print(vehicles[index].Reg_Number);
        lcd.print(" ");
        lcd.print(vehicles[index].Parking_Location);
        if (vehicles[index].Parking_Location.length() > 7) {
          for (int i = 0; i <= vehicles[index].Parking_Location.length() - 7; i++) {
            lcd.setCursor(9, 0);
            lcd.print(vehicles[index].Parking_Location.substring(i, vehicles[index].Parking_Location.length()));
            delay(500);
          }
        }
        lcd.setCursor(9, 0);
        lcd.print(vehicles[index].Parking_Location);

        //Situation when the first vehicle is displayed so only the down arrow is displayed
      } else if (index == 0) {
        lcd.clear();
        lcd.setCursor(0, 1);
        lcd.write((uint8_t)1);
        lcd.print(vehicles[index].Vehicle_Type);
        lcd.print(" ");
        lcd.print(vehicles[index].Payment_Status);

        lcd.setCursor(7, 1);
        if (vehicles[index].Entry_Hours < 10) {
          lcd.print("0");
          lcd.print(vehicles[index].Entry_Hours);
          if (vehicles[index].Entry_Minutes < 10) {
            lcd.print("0");
            lcd.print(vehicles[index].Entry_Minutes);
          }
          if (vehicles[index].Entry_Minutes > 10) {
            lcd.print(vehicles[index].Entry_Minutes);
          }
        } else {
          if (vehicles[index].Entry_Hours > 10) {
            lcd.print("0");
            lcd.print(vehicles[index].Entry_Hours);
            if (vehicles[index].Entry_Minutes < 10) {
              lcd.print("0");
              lcd.print(vehicles[index].Entry_Minutes);
            }
            lcd.print("0");
            if (vehicles[index].Entry_Minutes > 10) {
              lcd.print(vehicles[index].Entry_Minutes);
            }
          }
        }
        lcd.setCursor(12, 1);
        if (vehicles[index].Payment_Status == "PD") {
          if (vehicles[index].Exit_Hours < 10) {
            lcd.print("0");
            lcd.print(vehicles[index].Exit_Hours);
            if (vehicles[index].Exit_Minutes < 10) {
              lcd.print("0");
              lcd.print(vehicles[index].Exit_Minutes);
            }
            if (vehicles[index].Exit_Minutes > 10) {
              lcd.print(vehicles[index].Exit_Minutes);
            }
          }
        } else {
          if (vehicles[index].Exit_Hours > 10) {
            lcd.print(vehicles[index].Exit_Hours);
            if (vehicles[index].Exit_Minutes < 10) {
              lcd.print("0");
              lcd.print(vehicles[index].Exit_Minutes);
            }
            if (vehicles[index].Exit_Minutes > 10) {
              lcd.print(vehicles[index].Exit_Minutes);
            }
          }
        }

        lcd.setBacklight(vehicles[index].Backlight);
        lcd.setCursor(1, 0);
        lcd.print(vehicles[index].Reg_Number);
        lcd.print(" ");
        lcd.print(vehicles[index].Parking_Location);
        if (vehicles[index].Parking_Location.length() > 7) {
          for (int i = 0; i <= vehicles[index].Parking_Location.length() - 7; i++) {
            lcd.setCursor(9, 0);
            lcd.print(vehicles[index].Parking_Location.substring(i, vehicles[index].Parking_Location.length()));
            delay(500);
          }
        }
        lcd.setCursor(9, 0);
        lcd.print(vehicles[index].Parking_Location);

        //Situation when the final vehicle is displayed so only up arrow is displayed
      } else if (index == Vehicle_Num - 1) {
        lcd.clear();
        lcd.setCursor(1, 1);
        lcd.print(vehicles[index].Vehicle_Type);
        lcd.print(" ");
        lcd.print(vehicles[index].Payment_Status);

        lcd.setCursor(7, 1);
        if (vehicles[index].Entry_Hours < 10) {
          lcd.print("0");
          lcd.print(vehicles[index].Entry_Hours);
          if (vehicles[index].Entry_Minutes < 10) {
            lcd.print("0");
            lcd.print(vehicles[index].Entry_Minutes);
          }
          if (vehicles[index].Entry_Minutes > 10) {
            lcd.print(vehicles[index].Entry_Minutes);
          }
        } else {
          if (vehicles[index].Entry_Hours > 10) {
            lcd.print("0");
            lcd.print(vehicles[index].Entry_Hours);
            if (vehicles[index].Entry_Minutes < 10) {
              lcd.print("0");
              lcd.print(vehicles[index].Entry_Minutes);
            }
            lcd.print("0");
            if (vehicles[index].Entry_Minutes > 10) {
              lcd.print(vehicles[index].Entry_Minutes);
            }
          }
        }
        lcd.setCursor(12, 1);
        if (vehicles[index].Payment_Status == "PD") {
          if (vehicles[index].Exit_Hours < 10) {
            lcd.print("0");
            lcd.print(vehicles[index].Exit_Hours);
            if (vehicles[index].Exit_Minutes < 10) {
              lcd.print("0");
              lcd.print(vehicles[index].Exit_Minutes);
            }
            if (vehicles[index].Exit_Minutes > 10) {
              lcd.print(vehicles[index].Exit_Minutes);
            }
          }
        } else {
          if (vehicles[index].Exit_Hours > 10) {
            lcd.print(vehicles[index].Exit_Hours);
            if (vehicles[index].Exit_Minutes < 10) {
              lcd.print("0");
              lcd.print(vehicles[index].Exit_Minutes);
            }
            if (vehicles[index].Exit_Minutes > 10) {
              lcd.print(vehicles[index].Exit_Minutes);
            }
          }
        }

        lcd.setBacklight(vehicles[index].Backlight);
        lcd.setCursor(0, 0);
        lcd.write((uint8_t)0);
        lcd.print(vehicles[index].Reg_Number);
        lcd.print(" ");
        lcd.print(vehicles[index].Parking_Location);
        if (vehicles[index].Parking_Location.length() > 7) {
          for (int i = 0; i <= vehicles[index].Parking_Location.length() - 7; i++) {
            lcd.setCursor(9, 0);
            lcd.print(vehicles[index].Parking_Location.substring(i, vehicles[index].Parking_Location.length()));
            delay(500);
          }
        }
        lcd.setCursor(9, 0);
        lcd.print(vehicles[index].Parking_Location);
      } else {

        lcd.clear();
        lcd.setCursor(0, 1);
        lcd.write((uint8_t)1);
        lcd.print(vehicles[index].Vehicle_Type);
        lcd.print(" ");
        lcd.print(vehicles[index].Payment_Status);

        lcd.setCursor(7, 1);
        if (vehicles[index].Entry_Hours < 10) {
          lcd.print("0");
          lcd.print(vehicles[index].Entry_Hours);
          if (vehicles[index].Entry_Minutes < 10) {
            lcd.print("0");
            lcd.print(vehicles[index].Entry_Minutes);
          }
          if (vehicles[index].Entry_Minutes > 10) {
            lcd.print(vehicles[index].Entry_Minutes);
          }
        } else {
          if (vehicles[index].Entry_Hours > 10) {
            lcd.print("0");
            lcd.print(vehicles[index].Entry_Hours);
            if (vehicles[index].Entry_Minutes < 10) {
              lcd.print("0");
              lcd.print(vehicles[index].Entry_Minutes);
            }
            lcd.print("0");
            if (vehicles[index].Entry_Minutes > 10) {
              lcd.print(vehicles[index].Entry_Minutes);
            }
          }
        }
        lcd.setCursor(12, 1);
        if (vehicles[index].Payment_Status == "PD") {
          if (vehicles[index].Exit_Hours < 10) {
            lcd.print("0");
            lcd.print(vehicles[index].Exit_Hours);
            if (vehicles[index].Exit_Minutes < 10) {
              lcd.print("0");
              lcd.print(vehicles[index].Exit_Minutes);
            }
            if (vehicles[index].Exit_Minutes > 10) {
              lcd.print(vehicles[index].Exit_Minutes);
            }
          }
        } else {
          if (vehicles[index].Exit_Hours > 10) {
            lcd.print(vehicles[index].Exit_Hours);
            if (vehicles[index].Exit_Minutes < 10) {
              lcd.print("0");
              lcd.print(vehicles[index].Exit_Minutes);
            }
            if (vehicles[index].Exit_Minutes > 10) {
              lcd.print(vehicles[index].Exit_Minutes);
            }
          }
        }

        lcd.setBacklight(vehicles[index].Backlight);
        lcd.setCursor(0, 0);
        lcd.write((uint8_t)0);
        lcd.print(vehicles[index].Reg_Number);
        lcd.print(" ");
        lcd.print(vehicles[index].Parking_Location);
        if (vehicles[index].Parking_Location.length() > 7) {
          for (int i = 0; i <= vehicles[index].Parking_Location.length() - 7; i++) {
            lcd.setCursor(9, 0);
            lcd.print(vehicles[index].Parking_Location.substring(i, vehicles[index].Parking_Location.length()));
            delay(500);
          }
        }
        lcd.setCursor(9, 0);
        lcd.print(vehicles[index].Parking_Location);
      }
      //Delay between pressing the button
      delay(250);
    }


    if (buttons & BUTTON_UP) {

      index = (index - 1 + Vehicle_Num) % Vehicle_Num;

      //Same formatting as BUTTON_DOWN scenario
      if (index == 0 && index == Vehicle_Num - 1) {
        lcd.clear();
        lcd.setCursor(1, 1);
        lcd.print(vehicles[index].Vehicle_Type);
        lcd.print(" ");
        lcd.print(vehicles[index].Payment_Status);

        lcd.setCursor(7, 1);
        if (vehicles[index].Entry_Hours < 10) {
          lcd.print("0");
          lcd.print(vehicles[index].Entry_Hours);
          if (vehicles[index].Entry_Minutes < 10) {
            lcd.print("0");
            lcd.print(vehicles[index].Entry_Minutes);
          }
          if (vehicles[index].Entry_Minutes > 10) {
            lcd.print(vehicles[index].Entry_Minutes);
          }
        } else {
          if (vehicles[index].Entry_Hours > 10) {
            lcd.print("0");
            lcd.print(vehicles[index].Entry_Hours);
            if (vehicles[index].Entry_Minutes < 10) {
              lcd.print("0");
              lcd.print(vehicles[index].Entry_Minutes);
            }
            lcd.print("0");
            if (vehicles[index].Entry_Minutes > 10) {
              lcd.print(vehicles[index].Entry_Minutes);
            }
          }
        }
        lcd.setCursor(12, 1);
        if (vehicles[index].Payment_Status == "PD") {
          if (vehicles[index].Exit_Hours < 10) {
            lcd.print("0");
            lcd.print(vehicles[index].Exit_Hours);
            if (vehicles[index].Exit_Minutes < 10) {
              lcd.print("0");
              lcd.print(vehicles[index].Exit_Minutes);
            }
            if (vehicles[index].Exit_Minutes > 10) {
              lcd.print(vehicles[index].Exit_Minutes);
            }
          }
        } else {
          if (vehicles[index].Exit_Hours > 10) {
            lcd.print(vehicles[index].Exit_Hours);
            if (vehicles[index].Exit_Minutes < 10) {
              lcd.print("0");
              lcd.print(vehicles[index].Exit_Minutes);
            }
            if (vehicles[index].Exit_Minutes > 10) {
              lcd.print(vehicles[index].Exit_Minutes);
            }
          }
        }

        lcd.setBacklight(vehicles[index].Backlight);
        lcd.setCursor(1, 0);
        lcd.print(vehicles[index].Reg_Number);
        lcd.print(" ");
        lcd.print(vehicles[index].Parking_Location);
        if (vehicles[index].Parking_Location.length() > 7) {
          for (int i = 0; i <= vehicles[index].Parking_Location.length() - 7; i++) {
            lcd.setCursor(9, 0);
            lcd.print(vehicles[index].Parking_Location.substring(i, vehicles[index].Parking_Location.length()));
            delay(500);
          }
        }
        lcd.setCursor(9, 0);
        lcd.print(vehicles[index].Parking_Location);
      } else if (index == 0) {
        lcd.clear();
        lcd.setCursor(0, 1);
        lcd.write((uint8_t)1);
        lcd.print(vehicles[index].Vehicle_Type);
        lcd.print(" ");
        lcd.print(vehicles[index].Payment_Status);

        lcd.setCursor(7, 1);
        if (vehicles[index].Entry_Hours < 10) {
          lcd.print("0");
          lcd.print(vehicles[index].Entry_Hours);
          if (vehicles[index].Entry_Minutes < 10) {
            lcd.print("0");
            lcd.print(vehicles[index].Entry_Minutes);
          }
          if (vehicles[index].Entry_Minutes > 10) {
            lcd.print(vehicles[index].Entry_Minutes);
          }
        } else {
          if (vehicles[index].Entry_Hours > 10) {
            lcd.print("0");
            lcd.print(vehicles[index].Entry_Hours);
            if (vehicles[index].Entry_Minutes < 10) {
              lcd.print("0");
              lcd.print(vehicles[index].Entry_Minutes);
            }
            lcd.print("0");
            if (vehicles[index].Entry_Minutes > 10) {
              lcd.print(vehicles[index].Entry_Minutes);
            }
          }
        }
        lcd.setCursor(12, 1);
        if (vehicles[index].Payment_Status == "PD") {
          if (vehicles[index].Exit_Hours < 10) {
            lcd.print("0");
            lcd.print(vehicles[index].Exit_Hours);
            if (vehicles[index].Exit_Minutes < 10) {
              lcd.print("0");
              lcd.print(vehicles[index].Exit_Minutes);
            }
            if (vehicles[index].Exit_Minutes > 10) {
              lcd.print(vehicles[index].Exit_Minutes);
            }
          }
        } else {
          if (vehicles[index].Exit_Hours > 10) {
            lcd.print(vehicles[index].Exit_Hours);
            if (vehicles[index].Exit_Minutes < 10) {
              lcd.print("0");
              lcd.print(vehicles[index].Exit_Minutes);
            }
            if (vehicles[index].Exit_Minutes > 10) {
              lcd.print(vehicles[index].Exit_Minutes);
            }
          }
        }

        lcd.setBacklight(vehicles[index].Backlight);
        lcd.setCursor(1, 0);
        lcd.print(vehicles[index].Reg_Number);
        lcd.print(" ");
        lcd.print(vehicles[index].Parking_Location);
        if (vehicles[index].Parking_Location.length() > 7) {
          for (int i = 0; i <= vehicles[index].Parking_Location.length() - 7; i++) {
            lcd.setCursor(9, 0);
            lcd.print(vehicles[index].Parking_Location.substring(i, vehicles[index].Parking_Location.length()));
            delay(500);
          }
        }
        lcd.setCursor(9, 0);
        lcd.print(vehicles[index].Parking_Location);

      } else if (index == Vehicle_Num - 1) {
        lcd.clear();
        lcd.setCursor(1, 1);
        lcd.print(vehicles[index].Vehicle_Type);
        lcd.print(" ");
        lcd.print(vehicles[index].Payment_Status);

        lcd.setCursor(7, 1);
        if (vehicles[index].Entry_Hours < 10) {
          lcd.print("0");
          lcd.print(vehicles[index].Entry_Hours);
          if (vehicles[index].Entry_Minutes < 10) {
            lcd.print("0");
            lcd.print(vehicles[index].Entry_Minutes);
          }
          if (vehicles[index].Entry_Minutes > 10) {
            lcd.print(vehicles[index].Entry_Minutes);
          }
        } else {
          if (vehicles[index].Entry_Hours > 10) {
            lcd.print("0");
            lcd.print(vehicles[index].Entry_Hours);
            if (vehicles[index].Entry_Minutes < 10) {
              lcd.print("0");
              lcd.print(vehicles[index].Entry_Minutes);
            }
            lcd.print("0");
            if (vehicles[index].Entry_Minutes > 10) {
              lcd.print(vehicles[index].Entry_Minutes);
            }
          }
        }
        lcd.setCursor(12, 1);
        if (vehicles[index].Payment_Status == "PD") {
          if (vehicles[index].Exit_Hours < 10) {
            lcd.print("0");
            lcd.print(vehicles[index].Exit_Hours);
            if (vehicles[index].Exit_Minutes < 10) {
              lcd.print("0");
              lcd.print(vehicles[index].Exit_Minutes);
            }
            if (vehicles[index].Exit_Minutes > 10) {
              lcd.print(vehicles[index].Exit_Minutes);
            }
          }
        } else {
          if (vehicles[index].Exit_Hours > 10) {
            lcd.print(vehicles[index].Exit_Hours);
            if (vehicles[index].Exit_Minutes < 10) {
              lcd.print("0");
              lcd.print(vehicles[index].Exit_Minutes);
            }
            if (vehicles[index].Exit_Minutes > 10) {
              lcd.print(vehicles[index].Exit_Minutes);
            }
          }
        }

        lcd.setBacklight(vehicles[index].Backlight);
        lcd.setCursor(0, 0);
        lcd.write((uint8_t)0);
        lcd.print(vehicles[index].Reg_Number);
        lcd.print(" ");
        lcd.print(vehicles[index].Parking_Location);
        if (vehicles[index].Parking_Location.length() > 7) {
          for (int i = 0; i <= vehicles[index].Parking_Location.length() - 7; i++) {
            lcd.setCursor(9, 0);
            lcd.print(vehicles[index].Parking_Location.substring(i, vehicles[index].Parking_Location.length()));
            delay(500);
          }
        }
        lcd.setCursor(9, 0);
        lcd.print(vehicles[index].Parking_Location);
      } else {

        lcd.clear();
        lcd.setCursor(0, 1);
        lcd.write((uint8_t)1);
        lcd.print(vehicles[index].Vehicle_Type);
        lcd.print(" ");
        lcd.print(vehicles[index].Payment_Status);

        lcd.setCursor(7, 1);
        if (vehicles[index].Entry_Hours < 10) {
          lcd.print("0");
          lcd.print(vehicles[index].Entry_Hours);
          if (vehicles[index].Entry_Minutes < 10) {
            lcd.print("0");
            lcd.print(vehicles[index].Entry_Minutes);
          }
          if (vehicles[index].Entry_Minutes > 10) {
            lcd.print(vehicles[index].Entry_Minutes);
          }
        } else {
          if (vehicles[index].Entry_Hours > 10) {
            lcd.print("0");
            lcd.print(vehicles[index].Entry_Hours);
            if (vehicles[index].Entry_Minutes < 10) {
              lcd.print("0");
              lcd.print(vehicles[index].Entry_Minutes);
            }
            lcd.print("0");
            if (vehicles[index].Entry_Minutes > 10) {
              lcd.print(vehicles[index].Entry_Minutes);
            }
          }
        }
        lcd.setCursor(12, 1);
        if (vehicles[index].Payment_Status == "PD") {
          if (vehicles[index].Exit_Hours < 10) {
            lcd.print("0");
            lcd.print(vehicles[index].Exit_Hours);
            if (vehicles[index].Exit_Minutes < 10) {
              lcd.print("0");
              lcd.print(vehicles[index].Exit_Minutes);
            }
            if (vehicles[index].Exit_Minutes > 10) {
              lcd.print(vehicles[index].Exit_Minutes);
            }
          }
        } else {
          if (vehicles[index].Exit_Hours > 10) {
            lcd.print(vehicles[index].Exit_Hours);
            if (vehicles[index].Exit_Minutes < 10) {
              lcd.print("0");
              lcd.print(vehicles[index].Exit_Minutes);
            }
            if (vehicles[index].Exit_Minutes > 10) {
              lcd.print(vehicles[index].Exit_Minutes);
            }
          }
        }

        lcd.setBacklight(vehicles[index].Backlight);
        lcd.setCursor(0, 0);
        lcd.write((uint8_t)0);
        lcd.print(vehicles[index].Reg_Number);
        lcd.print(" ");
        lcd.print(vehicles[index].Parking_Location);
        if (vehicles[index].Parking_Location.length() > 7) {
          for (int i = 0; i <= vehicles[index].Parking_Location.length() - 7; i++) {
            lcd.setCursor(9, 0);
            lcd.print(vehicles[index].Parking_Location.substring(i, vehicles[index].Parking_Location.length()));
            delay(500);
          }
        }
        lcd.setCursor(9, 0);
        lcd.print(vehicles[index].Parking_Location);
      }
      delay(250);
    }
  }

  static int state = -1;

  //Defining choice made
  String choice;

  //Array defining vehicle types
  char Type[] = { 'C', 'M', 'V', 'L', 'B' };

  //Reading serial input stage
  if (Serial.available() > 0) {
    choice = Serial.readString();
    choice.trim();

    //Condition to make sure it is either A, S, T, L, R
    if ((choice == "") || ((choice[1] == '-') && (choice[0] == 'A' || choice[0] == 'S' || choice[0] == 'T' || choice[0] == 'L' || choice[0] == 'R'))) {
    } else {
      Serial.print("\nERROR: Invalid Input Format\n");
    }

    switch (choice[0]) {
      case 'A':
        {
          bool exit = true;
          unsigned long Entry_Hours = Entry / 3600000;
          unsigned long Entry_Minutes = Entry / 60000;
          unsigned long Exit_Hours = 0;
          unsigned long Exit_Minutes = 0;

          String Reg_Number = choice.substring(2, 9);

          for (int i = 0; i <= 1; i++) {
            if (!(Reg_Number[i] >= 'A' && Reg_Number[i] <= 'Z')) {
              exit = false;
              break;
            }
          }
          for (int i = 2; i <= 3; i++) {
            if (!(Reg_Number[i] >= '0' && Reg_Number[i] <= '9')) {
              exit = false;
              break;
            }
          }
          for (int i = 4; i <= 6; i++) {
            if (!(Reg_Number[i] >= 'A' && Reg_Number[i] <= 'Z')) {
              exit = false;
              break;
            }
          }

          if (exit == false) {
            Serial.print("\nERROR: Invalid Register Number Format\n");
            break;
          }

          char Vehicle_Type = choice.charAt(10);

          for (int i = 0; i < 5; i++) {
            if (Vehicle_Type == Type[i] && choice[9] == '-' && choice[11] == '-') {
              exit = true;
              break;
            } else {
              exit = false;
            }
          }
          if (exit == false) {
            Serial.print("\nERROR: Invalid Vehicle Type\n");
            break;
          }

          String Parking_Location = choice.substring(12, 45);

          for (int i = 0; i < Max_Vehicles; i++) {
            if (Reg_Number == vehicles[i].Reg_Number && Vehicle_Type == vehicles[i].Vehicle_Type && Parking_Location == vehicles[i].Parking_Location) {
              exit = false;
              break;
            }
          }

          if (exit == false) {
            Serial.print("\nERROR: There is already a vehicle with this information\n");
            break;
          }

          for (int i = 0; i < Max_Vehicles; i++) {
            if (Reg_Number == vehicles[i].Reg_Number && vehicles[i].Payment_Status == "NPD") {
              exit = false;
              break;
            } else {
              exit = true;
            }
          }
          if (exit == false) {
            Serial.print("\nERROR: Non Payment Status\n");
            break;
          }
          for (int i = 0; i < Max_Vehicles; i++) {
            if (Reg_Number == vehicles[i].Reg_Number && vehicles[i].Payment_Status == "NPD") {
              exit = false;
              break;
            } else {
              exit = true;
            }
          }
          if (exit == false) {
            Serial.print("\nERROR: Non Payment Status\n");
            break;
          }

          if (Parking_Location.length() < 1 || Parking_Location.length() > 11) {
            Serial.print("\nERROR: Invalid Parking Location\n");
            break;
          }

          String Payment_Status = "NPD";
          int Backlight = 3;

          exit = true;
          for (int i = 0; i < Max_Vehicles; i++) {
            if (Reg_Number == vehicles[i].Reg_Number && vehicles[i].Payment_Status == "PD") {
              vehicles[i].Vehicle_Type = Vehicle_Type;
              vehicles[i].Parking_Location = Parking_Location;
              vehicles[i].Backlight = 2;
              vehicles[i].Payment_Status = "PD";
              exit = false;
              break;
            }
          }
          for (int i = 0; i < Max_Vehicles; i++) {
            if (Reg_Number == vehicles[i].Reg_Number && vehicles[i].Payment_Status != "PD") {
              Serial.print("\nERROR: Payment Status is NPD\n");
              exit = false;
              break;
            }
          }

          if (exit == true) {
            add_Vehicle(Reg_Number, Vehicle_Type, Parking_Location, Payment_Status, Backlight, Entry_Hours, Entry_Minutes, Exit_Minutes, Exit_Hours);
          }
        }
        break;

      case 'S':
        {
          bool exit = true;
          String Reg_Number = choice.substring(2, 9);
          for (int i = 0; i < Max_Vehicles; i++) {
            if (Reg_Number == vehicles[i].Reg_Number) {
              exit = true;
              break;
            } else {
              exit = false;
            }
          }
          if (exit == false) {
            Serial.print("\nERROR: Register Number does not exist\n");
          }

          String Payment_Status = choice.substring(10, 13);
          unsigned long Entry_Hours = Entry / 3600000;
          unsigned long Entry_Minutes = Entry / 60000;
          for (int i = 0; i < Max_Vehicles; i++) {
            if (Reg_Number == vehicles[i].Reg_Number && Payment_Status == vehicles[i].Payment_Status) {
              Serial.print("\nERROR: Payment Status has not changed\n");
              break;
            }
          }
          if (Payment_Status == "PD") {
            unsigned long Exit_Hours = Exit / 3600000;
            unsigned long Exit_Minutes = Exit / 60000;
            for (int i = 0; i < Max_Vehicles; i++) {
              if (Reg_Number == vehicles[i].Reg_Number) {
                unsigned long Entry_Hours = vehicles[i].Exit_Hours;
                unsigned long Entry_Minutes = vehicles[i].Entry_Minutes;
                break;
              }
            }
            int Backlight = 2;
            Change_Status(Reg_Number, Payment_Status, Backlight, Exit_Hours, Exit_Minutes, Entry_Minutes, Entry_Hours);
          } else if (Payment_Status == "NPD") {
            unsigned long Exit_Hours = 0;
            unsigned long Exit_Minutes = 0;
            unsigned long Entry_Hours = Entry / 3600000;
            unsigned long Entry_Minutes = Entry / 60000;
            int Backlight = 3;
            Change_Status(Reg_Number, Payment_Status, Backlight, Exit_Hours, Exit_Minutes, Entry_Minutes, Entry_Hours);
          } else {
            Serial.print("\nERROR: Not a Valid Payment Status\n");
          }
        }
        break;

      case 'T':
        {
          bool exit = true;
          String Reg_Number = choice.substring(2, 9);
          for (int i = 0; i < Max_Vehicles; i++) {
            if (Reg_Number == vehicles[i].Reg_Number) {
              exit = true;
              break;
            } else {
              exit = false;
            }
          }
          if (exit == false) {
            Serial.print("\nERROR: Register Number does not exist\n");
            break;
          }

          char Vehicle_Type = choice.charAt(10);
          for (int i = 0; i < Max_Vehicles; i++) {
            if (Reg_Number == vehicles[i].Reg_Number && vehicles[i].Payment_Status == "NPD") {
              exit = false;
              break;
            } else {
              exit = true;
            }
          }
          if (exit == false) {
            Serial.print("\nERROR: Non Payment Status\n");
            break;
          }

          for (int i = 0; i < Max_Vehicles; i++) {
            if (Reg_Number == vehicles[i].Reg_Number && Vehicle_Type == vehicles[i].Vehicle_Type) {
              exit = false;
              break;
            } else {
              exit = true;
            }
          }
          if (exit == false) {
            Serial.print("\nERROR: Vehicle Type is the same\n");
            break;
          }

          for (int i = 0; i < 5; i++) {
            if (Vehicle_Type == Type[i]) {
              Change_Type(Reg_Number, Vehicle_Type);
              exit = true;
              break;
            } else {
              exit = false;
            }
          }
          if (exit == false) {
            Serial.print("\nERROR: Invalid Vehicle Type\n");
            break;
          }
        }
        break;

      case 'L':
        {
          bool exit = true;
          String Reg_Number = choice.substring(2, 9);
          for (int i = 0; i < Max_Vehicles; i++) {
            if (Reg_Number == vehicles[i].Reg_Number) {
              exit = true;
              break;
            } else {
              exit = false;
            }
          }
          if (exit == false) {
            Serial.print("\nERROR: Register Number does not exist\n");
          }

          String Parking_Location = choice.substring(10, 23);
          for (int i = 0; i < Max_Vehicles; i++) {
            if (Reg_Number == vehicles[i].Reg_Number && vehicles[i].Payment_Status == "NPD") {
              exit = false;
              break;
            } else {
              exit = true;
            }
          }
          if (exit == false) {
            Serial.print("\nERROR: Non Payment Status\n");
            break;
          }

          for (int i = 0; i < Max_Vehicles; i++) {
            if (Reg_Number == vehicles[i].Reg_Number && Parking_Location == vehicles[i].Parking_Location) {
              exit = false;
              break;
            } else {
              exit = true;
            }
          }
          if (exit == false) {
            Serial.print("\nERROR: Location is already the same\n");
            break;
          }

          for (int i = 0; i < Max_Vehicles; i++) {
            if (Reg_Number == vehicles[i].Reg_Number && vehicles[i].Payment_Status == "NPD") {
              exit = false;
              break;
            } else {
              exit = true;
            }
          }
          if (exit == false) {
            Serial.print("\nERROR: Non Payment Status\n");
            break;
          }

          for (int i = 0; i < 5; i++) {
            if (!(Parking_Location.length() < 1 || Parking_Location.length() > 11)) {
              Change_Location(Reg_Number, Parking_Location);
              exit = true;
            } else {
              exit = false;
            }
          }
          if (exit == false) {
            Serial.print("\nERROR: Invalid Parking Location\n");
            break;
          }
        }
        break;

      case 'R':
        {
          bool exit = true;
          String Reg_Number = choice.substring(2, 9);
          for (int i = 0; i < Max_Vehicles; i++) {
            if (Reg_Number == vehicles[i].Reg_Number) {
              exit = true;
              break;
            } else {
              exit = false;
            }
          }
          if (exit == false) {
            Serial.print("\nERROR: Register Number does not exist\n");
          }

          for (int i = 0; i < Max_Vehicles; i++) {
            if (Reg_Number == vehicles[i].Reg_Number && vehicles[i].Payment_Status == "NPD") {
              exit = false;
              break;
            } else if (Reg_Number == vehicles[i].Reg_Number && vehicles[i].Payment_Status == "PD") {
              lcd.setBacklight(7);
              lcd.clear();
              Remove_Vehicle(Reg_Number);
              exit = true;
              break;
            }
          }
          if (exit == false) {
            Serial.print("\nERROR: Non Payment Status\n");
            break;
          } else {
          }
        }
        break;
    }
  }
}
