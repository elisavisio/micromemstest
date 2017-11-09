#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Max72xxPanel.h>
#include <MD_MAX72xx.h>
#include <ESP8266WiFi.h>

int count;
String ChipIdString;
unsigned int maxPeakToPeak = 0;   // peak-to-peak level
unsigned long previousMillis = 0;  
int8_t graph[32];
bool scroll = false;
uint8_t i, col, pos = 0;

// replace with your WiFi connection info
#define PIN_ANALOG_IN A0

/////////////////////////////////////////////////////////////////////////// Display config
#define  MAX_DEVICES 4

#define  CLK_PIN   14
#define DATA_PIN  13
#define CS_PIN    0

MD_MAX72XX mx = MD_MAX72XX(CS_PIN, MAX_DEVICES);                      // SPI hardware interface

#define spacer  1
unsigned int width =5 + spacer; // Espaces entre caracteres
#define pinCS  0 // Attach CS to this pin, DIN to MOSI and CLK to SCK 
#define numberOfHorizontalDisplays   4 //Nombre de matrice de Led (Horizontal)
#define numberOfVerticalDisplays   1 //Nombre de matrice de Led (Vertical)

Max72xxPanel matrix = Max72xxPanel(pinCS, numberOfHorizontalDisplays, numberOfVerticalDisplays);

void setup() 
{
  Serial.begin(115200);

  matrix.setIntensity(4); // Use a value between 0 and 15 for brightness
  matrix.setPosition(0, 3, 0); // The first display is at <0, 7>
  matrix.setPosition(1, 2, 0); // The second display is at <1, 0>
  matrix.setPosition(2, 1, 0); // The third display is at <2, 0>
  matrix.setPosition(3, 0, 0); // And the last display is at <3, 0>
  pinMode(A0, INPUT);
  count = 0;
  ChipIdString = String(ESP.getChipId());
  Serial.println(ChipIdString);
}
//////////////////////////////////////////////////////////////////////////////////////////
//                                   print text                           //
//  Description: Affiche une ligne de texte                  //
//////////////////////////////////////////////////////////////////////////////////////////
  void PrintLineNoScroll(String msg)
  {

    int msgLength=msg.length()+1;
    char msgArray[msgLength];
    msg.toCharArray( msgArray, msgLength);
    
    int maxPixelLine = width * msgLength + matrix.width() - 0 - spacer;
    

      matrix.fillScreen(LOW);
      //int letter = 0 / width;
      int letter = 5;
      int x = matrix.width()-2;
      int y = (matrix.height() - 8) / 2; // center the text vertically
    
      while ( x + width - spacer >= 0 && letter >= 0 ) {
        if ( letter < sizeof(msgArray) )
        {
          matrix.drawChar(x, y,  msgArray[letter], HIGH, LOW, 1);    
        }
        letter--;
        x -= width;
      }
      //(rotation des afficheurs)
      matrix.setRotation(0, 3);
      matrix.setRotation(1, 3);
      matrix.setRotation(2, 3);
      matrix.setRotation(3, 3);  
      matrix.write(); // Send bitmap to display

    
  }

  //////////////////////////////////////////////////////////////////////////////////////////
//                                    get Noise                                     //
//  Description: Lecture du bruit (microphone) et renvoie la valeurs max obtenue    //
//////////////////////////////////////////////////////////////////////////////////////////
unsigned int getNoise()
{
  const int sampleWindow = 500; // Sample window width in mS (50 mS = 20Hz)
  unsigned int sample;
  unsigned long startMillis = millis(); // Start of sample window
  int signalMax = 0;
  int signalMin = 1023;
  unsigned int peakToPeak=0;

  //Annalyse du gain
  while (millis() - startMillis < sampleWindow)
  {
    sample = analogRead(A0);
    
//    server.handleClient();                        // checks for incoming messages
//    ArduinoOTA.handle();  

    // Serial.println(sample);
    if (sample < 1023 )  // toss out spurious readings
    {
      if (sample > signalMax)
      {
        signalMax = sample;  // save just the max levels
      }
      else if (sample < signalMin)
      {
        signalMin = sample;  // save just the min levels
      }
    }
  }
  peakToPeak = signalMax - signalMin;  // max - min = peak-peak amplitude
//peakToPeak = signalMax - 800;  // max - min = peak-peak amplitude
if (maxPeakToPeak < peakToPeak)
{
  maxPeakToPeak = peakToPeak;
}
maxPeakToPeak -=3;
 if (maxPeakToPeak >=7)
  {
    maxPeakToPeak = 7;
  }

 // Serial.println(String(maxPeakToPeak));
  return  maxPeakToPeak;
}
unsigned int getSimpleNoise()
{
  const int sampleWindow = 1000; // Sample window width in mS (50 mS = 20Hz)
  unsigned int sample;
  unsigned long startMillis = millis(); // Start of sample window
  int signalMax = 0;
  int signalAvg = 0;
  unsigned long count = 0;
  unsigned int peakToPeak=0;


  //Annalyse du gain
  while ((millis() - startMillis < sampleWindow))
  {
    sample = analogRead(PIN_ANALOG_IN);
 //Serial.println(String(sample));
     if (sample > signalMax)
      {
        signalMax = sample;  // save just the max levels
      }
     delayMicroseconds(125);
     count++;
  }
 
  //Serial.println(String(count));
  return  signalMax;
}
void loop() 
{
  int value, i;
  float decibelsValueQuiet = 49.5;
  float decibelsValueModerate = 65;
  float decibelsValueLoud = 70;
  float valueDb;
  
  // Check the envelope input
  
  value = analogRead(PIN_ANALOG_IN);
  //value =getNoise();  
 
// graph[pos] = (log10((float)maxPeakToPeak/20.0)+1)*7;
 graph[pos] = maxPeakToPeak;
  
 /** 
 Serial.println(((float)maxPeakToPeak/100.0));  
  Serial.println(log10((float)maxPeakToPeak/100.0));
     Serial.print("maxPeakToPeak. Value: ");
    Serial.print( graph[pos]);
    Serial.print("  ");
     Serial.print("pos. Value: ");
    Serial.print(pos);
    Serial.print("  ");
     Serial.println("");
    */
 maxPeakToPeak = 0;
/** print vumeter
 matrix.fillScreen(LOW);


  // Draw the graph left to right until 84 columns visible
  // After that shuffle the graph to the left and update the right most column
  if (!scroll) {
    for (i = 0; i <= pos; i++) {
      
      matrix.drawLine(i, 0, i , graph[i], HIGH); //X1,Y1,X2,Y2
    }
  }
  else {
    for (i = 0; i < 32; i++) {
      col = (i + pos + 1) % 32;
 
      matrix.drawLine(i, 0, i , graph[col], HIGH); //X1,Y1,X2,Y2
    }
  }
        //(rotation des afficheurs)
      matrix.setRotation(0, 3);
      matrix.setRotation(1, 3);
      matrix.setRotation(2, 3);
      matrix.setRotation(3, 3);  
     matrix.write();
  */
  //PrintLineNoScroll(ChipIdString);
  PrintLineNoScroll((String)value);
  

 /*   for ( int x = 0; x < matrix.width() - 1; x++ ) {
    matrix.fillScreen(LOW);
    matrix.drawLine(31, 0, 31 , 3, HIGH); //X1,Y1,X2,Y2
    matrix.write(); // Send bitmap to display
    delay(50);
  }*/
  //PrintLineNoScroll("67.12");
    // After the first pass, scroll the graph to the left
  if (pos == 32) {
    pos = 0;
    scroll = true;
  }
  else {
    pos++;
  }
  delay(1000);
}



