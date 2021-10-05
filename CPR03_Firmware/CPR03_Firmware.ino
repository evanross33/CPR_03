#include <Keypad.h>
#include <LiquidCrystal_I2C.h>
#include <LiquidCrystal.h>
#include <Wire.h>

//Function Delcarations
void Calculate();
void Parse();
void Error();
void Submit();
void Wipe_Vars();
void Clear();
void decimalCheck();


const int keyRows= 4; //Keypad Rows
const int keyCols= 4; //Keypad Columns

char keymap[keyRows][keyCols]=
{
{'1', '2', '3', '+'},
{'4', '5', '6', '-'},
{'7', '8', '9', '*'},
{'=', '0', '.', '/'}
};

byte rowPins[keyRows] = {10, 9 , 8, 7}; //Rows 0 to 3
byte colPins[keyCols]= {6, 5, 4, 3}; //Columns 0 to 3

Keypad myKeypad= Keypad(makeKeymap(keymap), rowPins, colPins, keyRows, keyCols);

//Variable Declarations
double num_1, num_2, ans;
String num_1_str ="", num_2_str="", input ="";
bool mulFlag = false, addFlag = false, subFlag = false, divFlag = false, clearFlag = false, prevCalc = false;
bool errorFlag = false, overflowFlag = false, manyOps = false, noNum = false;

LiquidCrystal_I2C lcd(0x27, 16, 2);

void setup(){
  //Assigning the pushbutton to a system interrupt to trigger a clear
  attachInterrupt(digitalPinToInterrupt(2), Clear_Int, RISING);
  //LCD Setup
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0,0);
  lcd.print("Hello (^w^)");
  
  delay(2000);
  lcd.setCursor(0,0);
  lcd.print("                ");

}

void loop(){
  lcd.setCursor(0,0);
  char keyEntry = myKeypad.getKey(); //Wait for Keypad Input...
  if (clearFlag) { //Check if SysInt has been triggered
    clearFlag = 0;
    Clear();
  }
  if (prevCalc) { //Check if this iteration is happening after a previous calculation (Clear old ans)
    prevCalc = false;
    Wipe_Vars();
    lcd.setCursor(0,0);
    lcd.print("                ");
  }
  if (keyEntry != NO_KEY) { //Check if Equals key has been submitted, if true call Submit()
    if (keyEntry == '=') {
      Submit();
    }
    else { //If key is not '=', attach char to the end of the input string
      lcd.setCursor(0,1);
      lcd.print("                   ");
      input = input + keyEntry;
      lcd.setCursor(0,0);
      lcd.print(input);
    }
  }
}

void Submit() {
    Parse(); //Call Parse() to parse out the input
    if (errorFlag) { //Check if Parse() threw an error
      Error();
    }
    else {
      Calculate(); //If Parse did not Error, Call Calculate()
      if (errorFlag) { //Check if Calculate Threw an Error
        Error();
      }
      else {
        char x[16]; //If Parse() and Calculate() did not error, display result
        dtostrf(ans, 8, 3, x);    
        String TempStr = String(x);

        if(TempStr.length() > 12) { //Output extends Precision of Arduino Double? if Yes display in Scientific Notation
           dtostre(ans,x,DTOSTR_ALWAYS_SIGN,DTOSTR_UPPERCASE);
           lcd.setCursor(0,1);
           lcd.print(x);
        }     
        else { //Else display normally
           lcd.setCursor(0,1); 
           lcd.print(x);
        }
      }
    }
  }

//Calculate is responsible for mathing num_1 and num_2 after Parse() has determined them
void Calculate() {
  if(input == "-" || input == "+" || input == "/" || input == "*" || num_2_str == ""){ //Last minute Error Checking for Bad Input
    noNum = true;
    errorFlag = true;
    return;
  }
  if (addFlag) { //The following will do the proper calcuation based on what op has been found by Parse()
    addFlag = false;
    prevCalc = true;
    ans = num_1 + num_2;
  }
  else if (subFlag) {
    subFlag = false;
    prevCalc = true;
    ans = num_1 - num_2;
  }
  else if(mulFlag) {
    mulFlag = false;
    prevCalc = true;
    ans = num_1 * num_2;
  }
  else if (divFlag) {
    divFlag = false;
    prevCalc = true;
    ans = num_1 / num_2;
  }
  else { //If no opFlag is set Error out (Should never get to Calculate() with no OpFlag set but just in case)
    errorFlag = true;
  }
}

void Parse() {
    //parse start
    int size = input.length();
    int counter = 0;
    if (size > 16) {
      overflowFlag = true;
      errorFlag = true;
      return;
    }
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
                manyOps = true;
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
                manyOps = true;
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
                manyOps = true;
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
            if(counter == 0)
            {
              if(input[1] != '+' && input[1] != '-' && input[1] != '*' && input[1] != '/')
              {
                leftNegative = true;
                break; 
              }else{//if the char after a negative is anything other than . or num its an error
                manyOps = true;
                errorFlag = true;
                break;
              }
            }
            else{
              if(subFlag == false)
              {
                if(opIndex == 0)
                {
                  subFlag = true;
                  opIndex=counter;
                }else{
                  if(input[counter+1] = '-')
                  {
                    errorFlag = true; 
                    manyOps = true;
                  }
                }
                //if there is a '-' after the current '-'
                if(input[counter+1] == '-')
                {
                  if(input[counter + 2] != '+' && input[counter + 2] != '-' && input[counter + 2] != '*' && input[counter+2] != '/'){
                    if(rightNegative == true)
                    {
                      errorFlag = true;
                      manyOps = true;
                    }
                    rightNegative = true;
                    ++counter;
                  }else{
                    manyOps = true;
                    errorFlag = true;
                  }
                }
                if(input[counter+1]!= '.' && !(input[counter + 1] >= '0' && input[counter + 1] <= '9')) {errorFlag = true; manyOps = true;}
                break;
              }else{//subFlag is already turned on
                manyOps = true;
                errorFlag = true;
                break;
              }
            }
            
        //below is switchend brack
        }
        //while loop
        ++counter;
    }
    //ran through string
    int booltest = int(mulFlag) + int(divFlag) + int(addFlag) + int(subFlag);
    //if booltest is greater than 1 than that means more than one flag is set, which is an error
    if(booltest > 1)
    {
        manyOps = true;
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
            num_2 = 0.0;
            num_2_str = "0";
            //if the number is not negative
            if(leftNegative == false)
            {
              for(int i= 0; i < input.length(); ++i)
              {
                num_1_str = num_1_str + input[i];
              }
              num_1 = num_1_str.toDouble();
            }else{//if the number is negative
              for(int i= 1; i < input.length(); ++i)
              {
                num_1_str = num_1_str + input[i];
              }
              num_1 = num_1_str.toDouble();
              //set num to negative version
              num_1 = num_1 * (-1);
            }
            //everything needed is set, parse can be exited
        }
        //this just means if any of the flags are on, if more than one flag is on than
        //errorFlag would be on so we shouldnt get here inless something is very wrong

        else if(mulFlag || divFlag || addFlag || subFlag)
        {//if leftNegative
          if(leftNegative == false)
          {
            for(int i = 0; i < opIndex; ++i)
            {
              num_1_str = num_1_str + input[i];
            }
          }else
            {
              for(int i = 1; i < opIndex; ++i)
              {
                num_1_str = num_1_str + input[i];
              }
            }
          if(rightNegative == false)
          {
            for(int i = opIndex + 1; i < input.length(); ++i)
            {
              num_2_str = num_2_str + input[i];
            }
          }else{
            for(int i = opIndex + 2; i< input.length(); ++i)
            {
              num_2_str = num_2_str + input[i];
            }
          }
          num_1 = num_1_str.toDouble();
          //converts num 1 to negative if lefthand negative flag is set
          if(leftNegative == true)
          {
          num_1 = num_1 * (-1);
          }
          //creates num_2 and sets it negative if rightHand negative is true            
          num_2 = num_2_str.toDouble();
          if(rightNegative == true)
          {
            num_2 = num_2 * (-1);
          }
        }
        //decimal check is performed here because string to double does not crash the program even if there are multiple decimals in 
        //one string number, so decimalCheck is performed here in order to not liter the program with checks
        decimalCheck();
    }else{/*serial.print("There was an error, too many operators")*/}
}

void Error(){
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
  if (noNum) {
    manyOps = false;
    overflowFlag = false;
    noNum = false;
    lcd.print("Too Few Numbers");
    delay(2000);
    lcd.setCursor(0,0);
    lcd.print("                ");
    return;
  }
  else if (overflowFlag) {
    manyOps = false;
    overflowFlag = false;
    noNum = false;
    lcd.print("Overflow Error");
    delay(2000);
    lcd.setCursor(0,0);
    lcd.print("                ");
    return;
  }
  else if (manyOps) {
    manyOps = false;
    overflowFlag = false;
    noNum = false;
    lcd.print("Too Many Ops!");
    delay(2000);
    lcd.setCursor(0,0);
    lcd.print("                ");
    return;
  }
  else {
    lcd.print("Unknown Error");
    delay(2000);
    lcd.setCursor(0,0);
    lcd.print("                ");
    return;
  }

}

//Clear_Int() first debounces the push button and then proceeds to set the clearFlag 
void Clear_Int() {
  static unsigned long last_interrupt_time = 0;
  unsigned long interrupt_time = millis();
  if(interrupt_time - last_interrupt_time > 500){
  clearFlag = 1;
  }
  interrupt_time = last_interrupt_time;
}

//Clear() Resets all Variables and LCD to their inital states
void Clear() {
  lcd.setCursor(0,0);
  lcd.print("                ");
  lcd.setCursor(0,0);
  lcd.print("Cleared");
  lcd.setCursor(0,1);
  lcd.print("                ");
  Wipe_Vars();
  delay(2000);
  lcd.setCursor(0,0);
  lcd.print("                ");
}

//Wipe_Vars() is a helper function of Clear(), Wipe_Vars() resets only the Variables
void Wipe_Vars() {
  num_1 = 0;
  num_2 = 0;
  num_1_str = "";
  num_2_str = "";
  input = "";
}

//this function will check num1 string and num2 string in order to see if there is more than one decimal in the number, if there is errorflag will be set
void decimalCheck()
{
  int decimals = 0;
  for (int i = 0; i < num_1_str.length(); ++i)
  {
    if(num_1_str[i] == '.')
    ++decimals;
  }
  if (decimals > 1) {
  errorFlag = true;
  manyOps = true;
  }
  
  //check num2 string
  decimals = 0;

  for (int i = 0; i < num_2_str.length(); ++i)
  {
    if(num_2_str[i] == '.')
    ++decimals;
  }
  if (decimals > 1) {
  errorFlag = true;
  manyOps = true;
  }
}
