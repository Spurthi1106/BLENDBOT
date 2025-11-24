#include <Wire.h>
#include <Adafruit_TCS34725.h>
#include <LiquidCrystal_I2C.h>


Adafruit_TCS34725 colorSensor = Adafruit_TCS34725(
    TCS34725_INTEGRATIONTIME_50MS,
    TCS34725_GAIN_4X
);


LiquidCrystal_I2C lcd(0x27, 16, 2);


enum SkinTone {
    VERY_FAIR = 0,
    FAIR = 1,
    MEDIUM = 2,
    OLIVE = 3,
    BROWN = 4,
    DARK_BROWN = 5,
    VERY_DARK = 6
};

struct SkinToneData {
    SkinTone tone;
    String name;
    uint8_t r, g, b;
    float brightness;
    float saturation;
    float hue;
};


void setup() {
    Serial.begin(115200);
    

    lcd.init();
    lcd.backlight();
    lcd.print("BLENDBOT v1.0");
    lcd.setCursor(0, 1);
    lcd.print("Color Detector");

    
if (!colorSensor.begin()) {
        Serial.println("ERROR: TCS34725 not found!");
        lcd.clear();
        lcd.print("SENSOR ERROR!");
        while (1);  
    }
    
    Serial.println("TCS34725 Color Sensor Ready");
    delay(2000);
    
    lcd.clear();
    lcd.print("Place on skin");
    lcd.setCursor(0, 1);
    lcd.print("Press button");
}


void loop() {
    detectSkinColor();
    delay(1000);
}


void detectSkinColor() {

    uint16_t r, g, b, c;
    colorSensor.getRawData(&r, &g, &b, &c);
    

    float norm_r = (float)r / c * 255;
    float norm_g = (float)g / c * 255;
    float norm_b = (float)b / c * 255;
    
    
    norm_r = constrain(norm_r, 0, 255);
    norm_g = constrain(norm_g, 0, 255);
    norm_b = constrain(norm_b, 0, 255);
    
    uint8_t final_r = (uint8_t)norm_r;
    uint8_t final_g = (uint8_t)norm_g;
    uint8_t final_b = (uint8_t)norm_b;
        SkinTone detected = classifySkinTone(final_r, final_g, final_b);
    
    
    SkinToneData toneData = getSkinToneData(detected, final_r, final_g, final_b);
    
   
    displayResults(toneData);
    
    
    printSerialData(toneData);
}


SkinTone classifySkinTone(uint8_t r, uint8_t g, uint8_t b) {
    

    float brightness = (r + g + b) / 3.0 / 255.0;
    

    uint8_t max_val = max(r, max(g, b));
    uint8_t min_val = min(r, min(g, b));
    float saturation = (max_val == 0) ? 0 : 
                       ((float)(max_val - min_val) / max_val);
    
  
    float redness = r / 255.0;
    float yellowness = (r + g) / 2.0 / 255.0;
    
    Serial.print("Brightness: "); Serial.print(brightness, 2);
    Serial.print(" | Saturation: "); Serial.print(saturation, 2);
    Serial.print(" | Red: "); Serial.print(redness, 2);
    Serial.println();
        if (brightness > 0.85) {
        return VERY_FAIR;
    } 
    else if (brightness > 0.70) {
        return FAIR;
    }
    else if (brightness > 0.55) {
        if (yellowness > 0.65) return OLIVE;
        return MEDIUM;
    }
    else if (brightness > 0.40) {
        if (redness > 0.60) return BROWN;
        return OLIVE;
    }
    else if (brightness > 0.25) {
        return DARK_BROWN;
    }
    else {
        return VERY_DARK;
    }
}

SkinToneData getSkinToneData(SkinTone tone, uint8_t r, uint8_t g, uint8_t b) {
    SkinToneData data;
    data.tone = tone;
    data.r = r;
    data.g = g;
    data.b = b;
    data.brightness = (r + g + b) / 3.0 / 255.0;
   
    float h, s, v;
    rgbToHsv(r, g, b, h, s, v);
    data.hue = h;
    data.saturation = s;
    
    switch (tone) {
        case VERY_FAIR:
            data.name = "Very Fair";
            break;
        case FAIR:
            data.name = "Fair";
            break;
        case MEDIUM:
            data.name = "Medium";
            break;
        case OLIVE:
            data.name = "Olive";
            break;
        case BROWN:
            data.name = "Brown";
            break;
        case DARK_BROWN:
            data.name = "Dark Brown";
            break;
        case VERY_DARK:
            data.name = "Very Dark";
            break;
    }
    
    return data;
}


void rgbToHsv(uint8_t r, uint8_t g, uint8_t b, 
              float &h, float &s, float &v) {
    
    float rf = r / 255.0;
    float gf = g / 255.0;
    float bf = b / 255.0;
    
    float max_val = max(rf, max(gf, bf));
    float min_val = min(rf, min(gf, bf));
    float delta = max_val - min_val;
    
    
    v = max_val;
    
    
    s = (max_val == 0) ? 0 : (delta / max_val);
    
  
    if (delta == 0) {
        h = 0;
    } 
    else if (max_val == rf) {
        h = fmod((gf - bf) / delta, 6.0) * 60.0;
    } 
    else if (max_val == gf) {
        h = ((bf - rf) / delta + 2.0) * 60.0;
    } 
    else {
        h = ((rf - gf) / delta + 4.0) * 60.0;
    }
    
    if (h < 0) h += 360;
}

void displayResults(SkinToneData data) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Skin: ");
    lcd.print(data.name);
    
    lcd.setCursor(0, 1);
    lcd.print("RGB:");
    lcd.print(data.r);
    lcd.print(",");
    lcd.print(data.g);
    lcd.print(",");
    lcd.print(data.b);
}

void printSerialData(SkinToneData data) {
    Serial.println("\n=== SKIN TONE DETECTION RESULTS ===");
    Serial.print("Detected Tone: ");
    Serial.println(data.name);
    
    Serial.print("RGB Values: (");
    Serial.print(data.r);
    Serial.print(", ");
    Serial.print(data.g);
    Serial.print(", ");
    Serial.print(data.b);
    Serial.println(")");
    
    Serial.print("Brightness: ");
    Serial.print(data.brightness, 3);
    Serial.println(" (0.0-1.0)");
    
    Serial.print("Saturation: ");
    Serial.print(data.saturation, 3);
    Serial.println(" (0.0-1.0)");
    
    Serial.print("Hue: ");
    Serial.print(data.hue, 1);
    Serial.println("° (0-360)");
    
    Serial.println("====================================\n");
}
