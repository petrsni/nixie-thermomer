#include <OneWire.h>
#include <DallasTemperature.h>

#define THERMOSENSOR 11 //DS18B20 data

// 74141 BCDA outputs
#define BCDA 7 
#define BCDB 6
#define BCDC 5
#define BCDD 4

// nixie tube anodes
#define NIXIE1 8
#define NIXIE2 9
#define NIXIE3 10


int updateInterval = 10000; //ms
OneWire oneWire(THERMOSENSOR);
DallasTemperature sensors(&oneWire);

/*
1) read temperature => float
2) convert it to int (21,3700005 => 214)
3) split it to byte[]: 2,1,4
4) convert to bcd 2 => 0010
5) show
*/

void setup() {
    Serial.println("start");
    sensors.begin();
    Serial.begin(9600);
    pinMode(BCDA, OUTPUT);
    pinMode(BCDB, OUTPUT);
    pinMode(BCDC, OUTPUT);
    pinMode(BCDD, OUTPUT);
    pinMode(NIXIE1, OUTPUT);
    pinMode(NIXIE2, OUTPUT);
    pinMode(NIXIE2, OUTPUT);

}

void toggleNixies(bool n1, bool n2, bool n3) {
    toggleSingleNixie(NIXIE1, n1);
    toggleSingleNixie(NIXIE2, n2);
    toggleSingleNixie(NIXIE3, n3);
}

// turn specified tube on/off
void toggleSingleNixie( int nid,  bool on) {
    digitalWrite(nid, uint8_t(on));
}

// convert float temperature to int  - 21.580009 => 216 to show on 3 nixies
int floatToDisplayValue(float f) {
    if(f>99.9){ 
        return 999; //can't display larger value
    }
    f=f*10;
    return round(f);
}

float requestTemperature() {
    sensors.setWaitForConversion(false);  // makes it async
    sensors.requestTemperatures();
    sensors.setWaitForConversion(true);
}


// convert a single byte to array of 4 bits  1 => byte[3]{1,2,3}
void intToBcdArray(byte val, int* bcdArray, int size) {
  byte bcd = ((val / 10) << 4) | (val % 10);
  for (int i = 0; i < size; i++) {
    bcdArray[i] = (bcd >> i) & 1;
  }
}

void displayNum(int* bcd) {
    digitalWrite(BCDA, bcd[0]);
    digitalWrite(BCDB, bcd[1]);
    digitalWrite(BCDC, bcd[2]);
    digitalWrite(BCDD, bcd[3]);  
}

//display number on 1st  (most significant) tube
void display1(byte dig) {
    // if dig == 0, do not show
    if(dig == 0) {return;}
    toggleNixies(true, false, false);
    int bcdArray[4];
    intToBcdArray(dig, bcdArray, 4);
    displayNum(bcdArray);
}

//display number on 2nd tube
void display2(byte dig) {
    // if dig == 0, do not show
    toggleNixies(false, true, false);
    int bcdArray[4];
    intToBcdArray(dig, bcdArray, 4);
    displayNum(bcdArray);
}

// display number on 3rd tube - decimal
void display3(byte dig) {
    // if dig == 0, do not show
    toggleNixies(false, false, true);
    int bcdArray[4];
    intToBcdArray(dig, bcdArray, 4);
    displayNum(bcdArray);
}

void display(int num) {
        byte digits[3];

        numToDecs(num, digits, 3);
        display1(digits[0]);
        delay(5);

        display2(digits[1]);
        delay(5);

        display3(digits[2]);
        delay(5);
}


// convert number to array of byte decimals int 123 => byte[1,2,3]
void numToDecs(int num, byte* decs, int len) {
  int i = len;
  int div = 0;
    while(i>0) {
      div = pow(10, i-1)+0.5; // pow works with doubles, casting float to int only removes the decimals (10**2 => 99,9 => (int)99 ), thus we add 0.5 to make sure this is correct
      decs[len-i]=num/div;
      num = num % div;
      i--;
    }
}

int dv = 666; // initial value
float temp=0;
void loop() {
    //request temp
    requestTemperature();

    byte digits[3];
    for(uint32_t start = millis(); (millis()-start <updateInterval);) {
       if(millis() - start >750 && millis() - start <770 ){
          temp=sensors.getTempCByIndex(0);
          dv = floatToDisplayValue(temp);
       }
       display(dv);
    }
}

