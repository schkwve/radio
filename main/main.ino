#include <Arduino.h>
#include <Wire.h>

#include <radio.h>
#include <RDA5807M.h>
#include <RDSParser.h>

#include <LiquidCrystal_PCF8574.h>

#include <OneButton.h>

#include "station.h"
#include "boot.h"

#define DEBUG

uint8_t usporny_rezim = 0;

// index prave naladene stanice
uint8_t indexStanice = 0;

// tlacitko MODE je pripojeno k A0
// tlacitko - je pripojeno k A1
// tlacitko + je pripojeno k A2
OneButton buttonMode(A0, true);
OneButton buttonMinus(A1, true);
OneButton buttonPlus(A2, true);

RDA5807M radio;
RDSParser rds;

LiquidCrystal_PCF8574 lcd(0x27);

enum repl_status { STATE_NONE = 0, STATE_PARSECOMMAND, STATE_PARSEINT };
repl_status status;

uint16_t indexZobrazeni = 0;

#include "funkce.h"

void setup()
{
	// nastav rychlost komunikace na 115200 baud
	Serial.begin(115200);

	// inicializuj lcd displej a zapni podsviceni
	lcd.begin(16, 2);
	lcd.setBacklight(1);
	delay(200);

	Serial.println("Inicializuji...");
	lcd.print("Inicializuji...");

	// inicializuj rds5807m:
	//   - inicializuj komunikaci
	//   - nastav frekvencni pasmo
	//   - nastav stereo a pocatecni hlasitost
#ifdef DEBUG
	radio.debugEnable(true);
	radio._wireDebug(true);
#else
	radio.debugEnable(false);
	radio._wireDebug(false);
#endif
	radio.initWire(Wire);
	radio.setBandFrequency(RADIO_BAND_FM, stanice[indexStanice].freq);
	radio.setMono(false);
	radio.setMute(false);
	radio.setVolume(10);

	// nastav retezec zpracovavani informaci RDS
	radio.attachReceiveRDS(processRDS);
	rds.attachServiceNameCallback(ukazNazevStanice);

	// nastav funkcionalitu ruznych funkci ovladacich tlacitek
	nastavCallback();
	buttonMode.attachClick(prepniRezimZobrazeni);
	buttonMode.attachDoubleClick(uspornyRezim);
	buttonMode.attachDuringLongPress(prepniMute);

	delay(800);
	show_bootanim();
	lcd.clear();

	status = STATE_PARSECOMMAND;
}

static unsigned long nextFreqTime = 0;
static unsigned long nextRadioInfoTime = 0;

static char command;
static int16_t value;
static RADIO_FREQ lastf = 0;
RADIO_FREQ f = 0;

void loop()
{
	char c;

	// zpracovani vstupu na seriovem kanalu
#ifdef DEBUG
	if (Serial.available() > 0) {
		c = Serial.peek();

		if ((status == STATE_PARSECOMMAND) && (c < 0x20)) {
			Serial.read();
		} else if (status == STATE_PARSECOMMAND) {
			command = Serial.read();
			status = STATE_PARSEINT;
		} else if (status == STATE_PARSEINT) {
			if ((c >= '0') && (c <= '9')) {
				c = Serial.read();
				value = (value * 10) + (c - '0');
			} else {
				runCommand(command, value);
				command = ' ';
				status = STATE_PARSECOMMAND;
				value = 0;
			}
		}
	}
#endif

	buttonMinus.tick();
	buttonPlus.tick();
	buttonMode.tick();
	radio.checkRDS();

	switch (indexZobrazeni) {
	case 0: // vychozi zobrazeni
		vychoziZobrazeni();
		break;
	case 1: // nastaveni zvuku
		nastaveniZvuku();
		break;
	case 2: // presne ladeni frekvence
		presneLadeni();
		break;
	default:
		vychoziZobrazeni();
		break;
	}

	delay(20);
}

void vychoziZobrazeni()
{
	unsigned long now = millis();
	// zobraz informace o aktualne naladene frekvenci
	if (now > nextFreqTime) {
		f = radio.getFrequency();
		ukazFrekvenci();
		lcd.setCursor(0, 1);
		lcd.print("        ");
		lastf = f;
		nextFreqTime = now + 400;
	}

	// zobraz aktualni silu signalu
	if (now > nextRadioInfoTime) {
		RADIO_INFO info;
		radio.getRadioInfo(&info);
		lcd.setCursor(14, 0);
		lcd.print(info.rssi);
		nextRadioInfoTime = now + 1000;
	}
}

void nastaveniZvuku()
{
	bool mono = radio.getMono();
	bool bb = radio.getBassBoost();

	lcd.setCursor(0, 0);
	lcd.print("+ ");
	lcd.setCursor(2, 0);
	lcd.print(mono ? "Mono  " : "Stereo");
	lcd.setCursor(0, 1);
	lcd.print("- Bass Boost:");
	lcd.setCursor(13, 1);
	lcd.print(bb ? "ANO" : " NE");
}

void presneLadeni()
{
	unsigned long now = millis();
	lcd.setCursor(11, 0);
	lcd.print("TUNER");

	// zobraz informace o aktualne naladene frekvenci
	if (now > nextFreqTime) {
		f = radio.getFrequency();
		ukazFrekvenci();
		lastf = f;
		nextFreqTime = now + 400;
	}
}

void runCommand(char cmd, int16_t value)
{
#ifdef DEBUG
	if (cmd == '?') {
		Serial.println();
		Serial.println("? pomoc");
		Serial.println("+ zvysit hlasitost");
		Serial.println("- snizit hlasitost");
		Serial.println("> dalsi stanice");
		Serial.println("< predchozi stanice");
		Serial.println(". zvysit frekvenci");
		Serial.println(", snizit frekvenci");
		Serial.println("f<XXXXX>: nalad na frekvenci <XXXXX>");
		Serial.println("i status stanice");
		Serial.println("s mono/stereo");
		Serial.println("b bass boost");
		Serial.println("m mute/unmute");
	} else if (cmd == '+') {
		zvysHlasitost();
	} else if (cmd == '-') {
		zmensiHlasitost();
	} else if (cmd == 'm') {
		radio.setMute(!radio.getMute());
	} else if (cmd == 's') {
		radio.setMono(!radio.getMono());
	} else if (cmd == 'b') {
		radio.setBassBoost(!radio.getBassBoost());
	} else if (cmd == '>') {
		naladDalsiStanici();
	} else if (cmd == '<') {
		naladPredchoziStanici();
	} else if (cmd == 'f') {
		radio.setFrequency(value);
		rds.init();
	} else if (cmd == '.') {
		naladDalsiFrekvenci(false);
	} else if (cmd == ':') {
		naladDalsiFrekvenci(true);
	} else if (cmd == ',') {
		naladPredchoziFrekvenci(false);
	} else if (cmd == ';') {
		naladPredchoziFrekvenci(true);
	} else if (cmd == '!') {
		if (value == 0)
			radio.term();
		if (value == 1)
			radio.init();
	} else if (cmd == 'i') {
		char s[12];
		radio.formatFrequency(s, sizeof(s));
		Serial.print("Stanice:");
		Serial.println(s);
		Serial.print("Radio:");
		radio.debugRadioInfo();
		Serial.print("Audio:");
		radio.debugAudioInfo();

		RADIO_INFO info;
		radio.getRadioInfo(&info);
		Serial.print("  RSSI: ");
		Serial.print(info.rssi);
		for (uint8_t i = 0; i < info.rssi - 15; i += 2) {
			Serial.write('*');
		}
		Serial.println();
		radio.checkRDS();
	} else if (cmd == 'x') {
		radio.debugStatus();
	}
#endif
}

void ukazFrekvenci()
{
	char s[12];
	radio.formatFrequency(s, sizeof(s));
	lcd.setCursor(0, 0);
	lcd.print(s);
}

void ukazNazevStanice(const char *name)
{
	lcd.setCursor(0, 1);
	lcd.print(name);
}

void processRDS(uint16_t block1, uint16_t block2, uint16_t block3,
				 uint16_t block4)
{
	rds.processData(block1, block2, block3, block4);
}
