#include <AiEsp32RotaryEncoder.h>
#include <AiEsp32RotaryEncoderNumberSelector.h>

#include <LiquidCrystal_I2C.h>

#include <Adafruit_NeoPixel.h>

#define LCD_SDA_PIN 15
#define LCD_SCL_PIN 16
#define ENC_CLK_PIN 46
#define ENC_DATA_PIN 3
#define ENC_SWITCH_PIN 8
#define DAC_OUT_PIN 17
#define ADC_IN_PIN 5

#define RGB_BUILTIN_PIN 18

const int baseVoltage = 500;   // millivolts
const int baseAdjust = 920;    // divide this by 100


LiquidCrystal_I2C lcd(0x27, 20, 4);

AiEsp32RotaryEncoder rotaryEncoder = AiEsp32RotaryEncoder(ENC_CLK_PIN, ENC_DATA_PIN, ENC_SWITCH_PIN, -1, 4);
Adafruit_NeoPixel pixel(1, RGB_BUILTIN_PIN, NEO_GRB+NEO_KHZ800);


long buttonPressBegin = 0;

void IRAM_ATTR readEncoderISR()
{
    rotaryEncoder.readEncoder_ISR();
}


void setup()
{
    Wire.begin(LCD_SDA_PIN, LCD_SCL_PIN);
    lcd.init();
    lcd.backlight();
	
    lcd.clear();
    lcd.setCursor(1, 0);
 
    rotaryEncoder.areEncoderPinsPulldownforEsp32=false;
    rotaryEncoder.begin();
    rotaryEncoder.setup(readEncoderISR);
    rotaryEncoder.setBoundaries(0, 255, false); //minValue, maxValue, circleValues true|false (when max go to min and vice versa)
    rotaryEncoder.setAcceleration(250);

    pixel.setBrightness(128);
    pixel.begin();
    pixel.setPixelColor(0,0);
    pixel.show();

    dacWrite(DAC_OUT_PIN, rotaryEncoder.readEncoder());

}

void loop()
{
    if (rotaryEncoder.encoderChanged())
    {
        long count = rotaryEncoder.readEncoder();
        lcd.setCursor(1,0);
        lcd.printf("Rotary value: %4d", count);
        lcd.setCursor(1,2);
        float calcVal = count*11.84512 + 120.5752;
        lcd.printf("Estimated: %4.0f", calcVal);
    }
    if (rotaryEncoder.isEncoderButtonDown())
    {
        lcd.setCursor(1,1);
        buttonPressBegin = millis();
        long encValue = rotaryEncoder.readEncoder();
        dacWrite(DAC_OUT_PIN, encValue);
        int inVolts = analogReadMilliVolts(ADC_IN_PIN);
        lcd.printf("Out: %4d ", encValue);
        lcd.setCursor(1,3);
        lcd.printf("Actual: %4d    ", inVolts);
        pixel.setPixelColor(0, pixel.Color(0,255,255) );
        pixel.show();
    }
    else if (millis() - buttonPressBegin > 500)   // 500 ms since button was last pressed
    {
        dacWrite(DAC_OUT_PIN, 255);   // set back to max - this is done 99% of the loop's
        pixel.setPixelColor(0,0);                // will it come back?
        pixel.show();
    }
    
}
