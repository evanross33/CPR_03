#include <Keypad.h>
#include <LiquidCrystal_I2C.h>
#include <LiquidCrystal.h>
#include <Wire.h>

//Function Delcarations
void Calculate();
void Parse();
void Error();
void interupt();

const int keyRows= 4; //Keypad Rows
const int keyCols= 4; //Keypad Columns

char keymap[keyRows][keyCols]=
{
{'1', '2', '3', '+'},
{'4', '5', '6', '-'},
{'7', '8', '9', '*'},
{'!', '0', '.', '/'}
};

byte rowPins[keyRows] = {10, 9 , 8, 7}; //Rows 0 to 3
byte colPins[keyCols]= {6, 5, 4, 3}; //Columns 0 to 3

Keypad myKeypad= Keypad(makeKeymap(keymap), rowPins, colPins, keyRows, keyCols);

//Variable Declarations
float num_1, num_2, ans;
String num_1_str ="", num_2_str="", test, input ="";
bool mulFlag = false, addFlag = false, subFlag = false, divFlag = false;
bool errorFlag = false;
char x[7];

LiquidCrystal_I2C lcd(0x27, 16, 2);

void setup(){
  Serial.begin(9600);
  attachInterrupt(digitalPinToInterrupt(2), Clear, RISING);
  lcd.init();
  lcd.backlight();
  Serial.begin(9600);
  lcd.setCursor(0,0);
  lcd.print("Hello");
  delay(2000);
  lcd.setCursor(0,0);
  lcd.print("                ");

}

void loop(){
  lcd.setCursor(0,0);
  char keyEntry = myKeypad.getKey();
  if (keyEntry != NO_KEY) {
    if (keyEntry == '!') {
      interupt();
      //if(!input.empty()){
//        input="";
//        lcd.setCursor(0,0);
//        lcd.print("Cleared.");
//        delay(2000);
//        //Clearing LCD
//        lcd.setCursor(0,0);
//        lcd.print("                ");
//        lcd.setCursor(0,1);
//        lcd.print("                ");
      //}
    }
    else {
      lcd.setCursor(0,1);
      lcd.print("                   ");
      input = input + keyEntry;
      lcd.setCursor(0,0);
      lcd.print(input);
    }
  }
}

void interupt() {
  static unsigned long last_interrupt_time = 0;
  unsigned long interrupt_time = millis();
  if(interrupt_time - last_interrupt_time > 500){
    Parse();
    if (errorFlag) {
      Error();
    }
    else {
      Calculate();
      if (errorFlag) {
        Error();
      }
      else {
        dtostrf(ans, 7, 3, x);
        lcd.setCursor(0,1);
        lcd.print(x);
      }
    }
  }
    last_interrupt_time = interrupt_time;
}

void Calculate() {
  if (addFlag) {
    addFlag = false;
    ans = num_1 + num_2;
  }
  else if (subFlag) {
    subFlag = false;
    ans = num_1 - num_2;
  }
  else if(mulFlag) {
    mulFlag = false;
    ans = num_1 * num_2;
  }
  else if (divFlag) {
    divFlag = false;
    ans = num_1 / num_2;
  }
  else {
    errorFlag = true;
  }
}

void Parse() {
    //parse start
    int size = input.length();
    int counter = 0;
    //operator index
    int opIndex= 0;
    bool leftNegative = false;
    bool rightNegative = false;
    while (counter < size && errorFlag == false)
    {
        char element = input[counter];
        switch (element)
        {
            case '*':
            //flag has already been set which means more than one operator
            if(mulFlag == true)
            {
                errorFlag = true;
                break;
            }
            else
            {
                mulFlag = true;
                opIndex = counter;
                break;
            }

            //divsion
            case '/':
            //flag has already been set which means more than one operator
            if(divFlag == true)
            {
                errorFlag = true;
                break;
            }
            else
            {
                divFlag = true;
                opIndex = counter;
                break;
            }

            //addition
            case '+':
            //flag has already been set which means more than one operator
            if(addFlag == true)
            {
                errorFlag = true;
                break;
            }
            else
            {
                addFlag = true;
                opIndex = counter;
                break;
            }

            //subtraction
            case '-':
            //flag has already been set which means more than one operator
            if(subFlag == true)
            {
              errorFlag = true;
              break;
            }
            else
            {
              if(counter != 0 && counter != opIndex + 1)
              {
                subFlag = true;
                opIndex = counter;
                break;
              }else{
                //this will see if - is a minus or a leftNegative
                // a '-' at the beginning signifies the left hand is negative 
                if(counter == 0)
                {
                  leftNegative = true;
                }
                //a '-' after the operator signifies the right hand side is negative
                if(counter == opIndex + 1)
                {
                  rightNegative = true;
                }
              }
            }

        }
        //while loop
        ++counter;
    }
    //ran through string
    int booltest = int(mulFlag) + int(divFlag) + int(addFlag) + int(subFlag);
    //if booltest is greater than 1 than that means more than one flag is set, which is an error
    if(booltest > 1)
    {
        errorFlag = true;
    }
    //if errorflag is not on then everything should go according to plan
    if(errorFlag != true){
        //if none found return original number, this is done by setting num_1_str to the input, then converting it to an int
        //the add flag will be set and num_2 will be set to 0 so calculator function will perform input + 0
        if((mulFlag == false) && (divFlag == false) && (addFlag == false) && (subFlag == false))
        {
            //this loop will set num_1_str to
            addFlag = true;
            num_2 = 0;
            //if the number is not negative
            if(leftNegative == false)
            {
              for(int i= 0; i < input.length(); ++i)
              {
                num_1_str = num_1_str + input[i];
              }
              num_1 = num_1_str.toFloat();
            }else{//if the number is negative
              for(int i= 0; i < input.length(); ++i)
              {
                num_1_str = num_1_str + input[i];
              }
              num_1 = num_1_str.toFloat();
              //set num to negative version
              num_1 = num_1 * (-1);
            }
            //everything needed is set, parse can be exited
        }
        //this just means if any of the flags are on, if more than one flag is on than
        //errorFlag would be on so we shouldnt get here inless something is very wrong

        if(mulFlag || divFlag || addFlag || subFlag)
        {//if no numbers are negative
          if(rightNegative == false && leftNegative == false)
          {
            for(int i = 0; i < opIndex; ++i)
            {
              num_1_str = num_1_str + input[i];
            }
            for(int i = opIndex + 1; i < input.length(); ++i)
            {
              num_2_str = num_2_str + input[i];
            }
            num_1 = num_1_str.toFloat();
            num_2 = num_2_str.toFloat();
          }
          if(rightNegative == true && leftNegative == false)
          {
            for(int i = 0; i < opIndex; ++i)
            {
              num_1_str = num_1_str + input[i];
            }
            for(int i = opIndex + 1; i < input.length(); ++i)
            {
              num_2_str = num_2_str + input[i];
            }
            num_1 = num_1_str.toFloat();
            num_1 = num_1 *(-1);
            num_2 = num_2_str.toFloat();
          }
          if(rightNegative == false && leftNegative == true)
          {
            for(int i = 0; i < opIndex; ++i)
            {
              num_1_str = num_1_str + input[i];
            }
            for(int i = opIndex + 1; i < input.length(); ++i)
            {
              num_2_str = num_2_str + input[i];
            }
            num_1 = num_1_str.toFloat();
            num_2 = num_2_str.toFloat();
            num_2 = num_2 * (-1);
          }
          if(rightNegative == true && leftNegative == true)
          {
            for(int i = 0; i < opIndex; ++i)
            {
              num_1_str = num_1_str + input[i];
            }
            for(int i = opIndex + 1; i < input.length(); ++i)
            {
              num_2_str = num_2_str + input[i];
            }
            num_1 = num_1_str.toFloat();
            num_1 = num_1 * (-1);
            num_2 = num_2_str.toFloat();
            num_2 = num_2 *(-1);
          }
        }
    }else{/*serial.print("There was an error, too many operators")*/}
}

void Error(){
  Serial.println("start error");
  errorFlag = false;
  divFlag = false;
  mulFlag = false;
  addFlag = false;
  subFlag = false;
  input = "";
  num_1_str = "";
  num_2_str = "";
  num_1 = 0;
  num_2 = 0;
  lcd.setCursor(0,0);
  lcd.print("                ");
  lcd.setCursor(0,1);
  lcd.print("                ");
  lcd.setCursor(0,0);
  lcd.print("Error");
  delay(2000);
  lcd.print(input);

}

void Clear() {
  lcd.setCursor(0,0);
  lcd.print("                ");
  lcd.setCursor(1,0);
  lcd.print("                ");
  input = "";
  num_1_str = "";
  num_2_str = "";
  num_1 = 0;
  num_2 = 0;
}
