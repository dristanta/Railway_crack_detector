#include <Wire.h>
#include<SoftwareSerial.h>
SoftwareSerial Serial1 (2,3); 
SoftwareSerial gps(10,11); //RX,TX
#define DEVICE (0x53)      
#define TO_READ (6)       
byte buff[TO_READ] ;        
char str[512];             
int regAddress = 0x32;      
int x, y, z;                   
#define samples 10
int i=0,k=0,n=8;
int  gps_status=0;
float latitude=0; 
float logitude=0;                       
String Speed="";
String gpsString="";
char *test="$GPRMC";

//intializing input pin and for reading data
int buttonPin = 9;
int reading;

void initModule(String cmd, char *res, int t)
{
  while(1)
  {
    Serial.println(cmd);
    Serial1.println(cmd);
    delay(100);
    while(Serial1.available()>0)
    {
       if(Serial1.find(res))
       {
        Serial.println(res);
        delay(t);
        return;
       }
       else
       {
        Serial.println("Error");
       }
    }
    delay(t);
  }
}



void setup() {
  Wire.begin();         // join i2c bus (address optional for master)
  Serial.begin(9600);  // start serial for output
  Serial1.begin(9600);

  pinMode(buttonPin, INPUT);

  Serial.println("Initializing....");
  initModule("AT","OK",1000);
  initModule("ATE1","OK",1000);
  initModule("AT+CPIN?","READY",1000);  
  initModule("AT+CMGF=1","OK",1000);     
  initModule("AT+CNMI=2,2,0,0,0","OK",1000);  
  Serial.println("Initialized Successfully");
  gps.begin(9600);
  get_gps();
  show_coordinate();
  Serial.println("System Ready..");

  //Turning on the ADXL345
  writeTo(DEVICE, 0x2D, 0);      
  writeTo(DEVICE, 0x2D, 16);
  writeTo(DEVICE, 0x2D, 8);
}

void loop() {

  readFrom(DEVICE, regAddress, TO_READ, buff); 
  Serial.print(str);
  Serial.write(10);
  Serial.println("");
  
  //reading the input from input pin
  reading = digitalRead(buttonPin);
  
  //It appears that delay is needed in order not to clog the port
  delay(500);
      if(reading == LOW)
        {
          get_gps();
          show_coordinate();
          delay(1000);

          if(reading == LOW)
          {
            Serial.println("Sending SMS");
            Send();
            Serial.println("SMS Sent");
            exit(0);
          }
        
        }
      }

void writeTo(int device, byte address, byte val) {
  Wire.beginTransmission(device); //start transmission to device 
  Wire.write(address);        // send register address
  Wire.write(val);        // send value to write
  Wire.endTransmission(); //end transmission
}
//reads num bytes starting from address register on device in to buff array
void readFrom(int device, byte address, int num, byte buff[]) {
  Wire.beginTransmission(device); //start transmission to device 
  Wire.write(address);        //sends address to read from
  Wire.endTransmission(); //end transmission
  Wire.beginTransmission(device); //start transmission to device
  Wire.requestFrom(device, num);    // request 6 bytes from device
  int i = 0;
  while(Wire.available())    //device may send less than requested (abnormal)
  { 
    buff[i] = Wire.read(); // receive a byte
    i++;
  }
  Wire.endTransmission(); //end transmission
}
void gpsEvent()
{
  gpsString="";
  while(1)
  {
   while (gps.available()>0)            //Serial incoming data from GPS
   {
    char inChar = (char)gps.read();
     gpsString+= inChar;                    //store incoming data from GPS to temparary string str[]
     i++;
    // Serial.print(inChar);
     if (i < 7)                      
     {
      if(gpsString[i-1] != test[i-1])         //check for right string
      {
        i=0;
        gpsString="";
      }
     }
    if(inChar=='\r')
    {
     if(i>60)
     {
       gps_status=1;
       break;
     }
     else
     {
       i=0;
     }
    }
  }
   if(gps_status)
    break;
  }
}
void get_gps()
{
  
   gps_status=0;
   int x=0;
   while(gps_status==0)
   {
    gpsEvent();
    int str_lenth=i;
    coordinate2dec();
    i=0;x=0;
    str_lenth=0;
   }
}
void show_coordinate()
{

    Serial.print("Latitude:");
    Serial.println(26.47);
    Serial.print("Longitude:");
    Serial.println(logitude);
    Serial.print("Speed(in knots)=");
    Serial.println(Speed);
    delay(2000);
    
}
void coordinate2dec()
{
  String lat_degree="";
    for(i=20;i<=21;i++)         
      lat_degree+=gpsString[i];
      
  String lat_minut="";
     for(i=22;i<=28;i++)         
      lat_minut+=gpsString[i];
  String log_degree="";
    for(i=32;i<=34;i++)
      log_degree+=gpsString[i];
  String log_minut="";
    for(i=35;i<=41;i++)
      log_minut+=gpsString[i];
    
    Speed="";
    for(i=45;i<48;i++)          //extract longitude from string
      Speed+=gpsString[i];
      
     float minut= lat_minut.toFloat();
     minut=minut/60;
     float degree=lat_degree.toFloat();
     latitude=degree+minut;
     
     minut= log_minut.toFloat();
     minut=minut/60;
     degree=log_degree.toFloat();
     logitude=degree+minut;
}
void Send()
{ 
   Serial1.println("AT");
   delay(500);
   serialPrint();
   Serial1.println("AT+CMGF=1");
   delay(500);
   serialPrint();
   Serial1.print("AT+CMGS=");
   Serial1.print('"');
   Serial1.print("+917002379534");    //mobile no. for SMS alert
   Serial1.println('"');
   delay(500);
   serialPrint();
   Serial1.print("Hey priyam we got an Crack at "); 
   Serial1.print("Latitude:");
   Serial1.println(26.47);
   delay(500);
   serialPrint();
   Serial1.print(" longitude:");
   Serial1.println(logitude);
   delay(500);
   serialPrint();
   Serial1.print(" Speed:");
   Serial1.print(Speed);
   Serial1.println("Knots");
   delay(500);
   serialPrint();
   Serial1.print("http://maps.google.com/maps?&z=15&mrt=yp&t=k&q=");
   Serial1.print(26.47,6);
   Serial1.print("+");              
   Serial1.print(logitude,6);
   Serial1.write(26);
   delay(2000);
   serialPrint();
}
void serialPrint()
{
  while(Serial1.available()>0)
  {
    Serial.print(Serial1.read());
  }
}
