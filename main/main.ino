#include <Arduino.h>
#include <LiquidCrystal_PCF8574.h>
#include <OneButton.h>
#include <RDA5807M.h>
#include <RDSParser.h>
#include <Wire.h>
#include <radio.h>

#include "boot.h"
#include "station.h"

// Uncomment this to enable debug mode
// #define DEBUG

// Indicates if power saving mode is currently enabled
uint8_t power_saving = 0;

// index of currently selected predefined station
uint8_t cur_station = 0;

// Indicates which
uint8_t idx_menu = 0;

///////  CONNECTIONS ///////
// 'MODE' Button : A0     //
// '-' Button    : A1     //
// '+' Button    : A2     //
//                        //
// I2C LCD SDA   : SDA    //
// I2C LCD SCL   : SCL    //
//                        //
// RDA5807M SDA  : A4     //
// RDA5807M SCL  : A5     //
//// ////////////////// ////

#define BTN_MODE A0
#define BTN_MINUS A1
#define BTN_PLUS A2

OneButton buttonMode(BTN_MODE, true);
OneButton buttonMinus(BTN_MINUS, true);
OneButton buttonPlus(BTN_PLUS, true);

RDA5807M radio;
RDSParser rds;

// NOTE: If the LCD Display doesn't show anything, change the
//       I2C address.
LiquidCrystal_PCF8574 lcd(0x27);

#define ARRAY_SIZE(x) ((sizeof(x) / sizeof(x[0])))

void enable_power_saving();
void disable_power_saving();

void predefined_station_up();
void predefined_station_down();

void seek_up();
void seek_down();
void fine_seek_up();
void fine_seek_down();

void volume_up();
void volume_down();
void toggle_mute();

void toggle_stereo();
void toggle_bassboost();

void menu_set_callbacks();
void menu_cycle();

void setup()
{
#ifdef DEBUG
    // 115200 baud if debugging is enabled
    Serial.begin(115200);
#endif /* DEBUG */

    // LCD Initialization (16x2 chars, enable backlight)
    lcd.begin(16, 2);
    lcd.setBacklight(1);
    delay(200);

    Serial.println("Inicializuji...");
    lcd.print("Inicializuji...");

    // inicializuj rds5807m:
    //   - inicializuj komunikaci
    //   - nastav frekvencni pasmo
    //   - nastav stereo a pocatecni hlasitost
    // RDA5807M initialization
    ////
    // Default settings:
    // - First station the list of predefined stations
    // - Stereo
    // - 10/15 (~66%) volume
#ifdef DEBUG
    radio.debugEnable(true);
    radio._wireDebug(true);
#else
    radio.debugEnable(false);
    radio._wireDebug(false);
#endif
    radio.initWire(Wire);
    radio.setBandFrequency(RADIO_BAND_FM, predefined_stations[cur_station].freq);
    radio.setMono(false);
    radio.setMute(false);
    radio.setVolume(10);

    // Set up RDS processing
    radio.attachReceiveRDS(rds_process);
    rds.attachServiceNameCallback(rds_show_station_name);

    // set default callback functions for first (default) menu
    // and set (constant) MODE button callbacks
    menu_set_callbacks();
    buttonMode.attachClick(menu_cycle);
    buttonMode.attachDoubleClick(enable_power_saving);
    buttonMode.attachDuringLongPress(toggle_mute);

    // Initialization happens very fast, so let's
    // give the hardware some time to load up properly
    delay(800);

    // show some cute boot animation
    // if we're not in debug mode
#ifndef DEBUG
    show_bootanim();
#endif
    lcd.clear();
}

static unsigned long nextFreqTime = 0;
static unsigned long nextRadioInfoTime = 0;

static RADIO_FREQ lastf = 0;
RADIO_FREQ f = 0;

void loop()
{
    // process button presses
    buttonMinus.tick();
    buttonPlus.tick();
    buttonMode.tick();
    radio.checkRDS();

    switch (idx_menu)
    {
        case 0:  // main menu
            menu_main();
            break;
        case 1:  // sound settings
            menu_sound_settings();
            break;
        case 2:  // tuner
            menu_tuner();
            break;
        default:  // fallback to main menu
            menu_main();
            break;
    }

    // give the hardware some time to breathe
    delay(20);
}

void menu_main()
{
    unsigned long now = millis();

    // show frequency
    if (now > nextFreqTime)
    {
        f = radio.getFrequency();
        display_freq();
        lcd.setCursor(0, 1);
        lcd.print("        ");
        lastf = f;
        nextFreqTime = now + 400;
    }

    // show current received signal strength indication
    if (now > nextRadioInfoTime)
    {
        RADIO_INFO info;
        radio.getRadioInfo(&info);
        lcd.setCursor(14, 0);
        lcd.print(info.rssi);
        nextRadioInfoTime = now + 1000;
    }
}

void menu_sound_settings()
{
    lcd.setCursor(0, 0);
    lcd.print("+ ");
    lcd.setCursor(2, 0);
    lcd.print(radio.getMono() ? "Mono  " : "Stereo");
    lcd.setCursor(0, 1);
    lcd.print("- Bass Boost:");
    lcd.setCursor(13, 1);
    lcd.print(radio.getBassBoost() ? "ANO" : " NE");
}

void menu_tuner()
{
    unsigned long now = millis();
    lcd.setCursor(11, 0);
    lcd.print("TUNER");

    // frequency should change by 0.1 MHz
    if (now > nextFreqTime)
    {
        f = radio.getFrequency();
        display_freq();
        lastf = f;
        nextFreqTime = now + 400;
    }
}

void display_freq()
{
    char s[12];
    radio.formatFrequency(s, sizeof(s));
    lcd.setCursor(0, 0);
    lcd.print(s);
}

void rds_show_station_name(const char *name)
{
    lcd.setCursor(0, 1);
    lcd.print(name);
}

void rds_process(uint16_t block1, uint16_t block2, uint16_t block3, uint16_t block4)
{
    rds.processData(block1, block2, block3, block4);
}

void enable_power_saving()
{
    disable_power_saving();
    lcd.noDisplay();
    lcd.setBacklight(0);
    delay(50);
    power_saving = 1;
}

void disable_power_saving()
{
    lcd.display();
    lcd.setBacklight(1);
    power_saving = 0;
}

void predefined_station_up()
{
    disable_power_saving();
    if (cur_station < ARRAY_SIZE(predefined_stations) - 1)
    {
        cur_station++;
        radio.setFrequency(predefined_stations[cur_station].freq);
        rds.init();
    }
}

void predefined_station_down()
{
    disable_power_saving();
    if (cur_station > 0)
    {
        cur_station--;
        radio.setFrequency(predefined_stations[cur_station].freq);
        rds.init();
    }
}

void seek_up(void)
{
    disable_power_saving();
    radio.seekUp(true);
}

void seek_down(void)
{
    disable_power_saving();
    radio.seekDown(true);
}

void fine_seek_up()
{
    disable_power_saving();
    radio.seekUp(false);
}

void fine_seek_down()
{
    disable_power_saving();
    radio.seekDown(false);
}

void volume_up()
{
    disable_power_saving();
    uint8_t v = radio.getVolume() + 1;
    v = constrain(v, 0, 15);
    radio.setVolume(v);
}

void volume_down()
{
    disable_power_saving();
    uint8_t v = radio.getVolume() - 1;
    v = constrain(v, 0, 15);
    radio.setVolume(v);
}

void toggle_mute()
{
    disable_power_saving();
    radio.setMute(!radio.getMute());
}

void toggle_stereo()
{
    disable_power_saving();
    radio.setMono(!radio.getMono());
}
void toggle_bassboost()
{
    disable_power_saving();
    radio.setBassBoost(!radio.getBassBoost());
}

void menu_set_callbacks()
{
    buttonPlus.reset();
    buttonMinus.reset();
    switch (idx_menu)
    {
        case 0:
        {
            buttonPlus.attachClick(seek_up);
            buttonPlus.attachDoubleClick(predefined_station_up);
            buttonPlus.attachDuringLongPress(volume_up);
            //
            buttonMinus.attachClick(seek_down);
            buttonMinus.attachDoubleClick(predefined_station_down);
            buttonMinus.attachDuringLongPress(volume_down);
            break;
        }
        case 1:
        {
            buttonPlus.attachClick(toggle_stereo);
            buttonMinus.attachClick(toggle_bassboost);
            break;
        }
        case 2:
        {
            buttonPlus.attachClick(fine_seek_up);
            buttonMinus.attachClick(fine_seek_down);
            break;
        }
    }
}

void menu_cycle()
{
    // if we were in power saving mode, just wake up.
    if (power_saving == 1)
    {
        disable_power_saving();
        return;
    }
    else
    {
        idx_menu++;
    }

    lcd.clear();
    if (idx_menu >= 3)
    {
        idx_menu = 0;
    }

    menu_set_callbacks();
}
