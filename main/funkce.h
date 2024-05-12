#ifndef _POMOC_H
#define _POMOC_H

extern RDA5807M radio;
extern uint8_t indexStanice;
extern struct radio_station stanice[];
extern RDSParser rds;
extern uint8_t usporny_rezim;

#define ARRAY_SIZE(x) ((sizeof(x)/sizeof(x[0])))

void probud()
{
	lcd.display();
	lcd.setBacklight(1);
	usporny_rezim = 0;
}

void naladDalsiStanici()
{
	probud();
	if (indexStanice < ARRAY_SIZE(stanice) - 1) {
		indexStanice++;
		radio.setFrequency(stanice[indexStanice].freq);
		rds.init();
	}
}

void naladPredchoziStanici()
{
	probud();
	if (indexStanice > 0) {
		indexStanice--;
		radio.setFrequency(stanice[indexStanice].freq);
		rds.init();
	}
}

void naladDalsiFrekvenci(bool musiVysilat = true)
{
	probud();
	radio.seekUp(musiVysilat);
}

void naladPredchoziFrekvenci(bool musiVysilat = true)
{
	probud();
	radio.seekDown(musiVysilat);
}

void zvysHlasitost()
{
	probud();
	int v = radio.getVolume() + 1;
	v = constrain(v, 0, 15);
	radio.setVolume(v);
}

void zmensiHlasitost()
{
	probud();
	int v = radio.getVolume() - 1;
	v = constrain(v, 0, 15);
	radio.setVolume(v);
}

void prepniStereoMono()
{
	probud();
	radio.setMono(!radio.getMono());
}
void prepniBassBoost()
{
	probud();
	radio.setBassBoost(!radio.getBassBoost());
}

void jemneLadeniNahoru()
{
	probud();
	naladDalsiFrekvenci(false);
}

void jemneLadeniDolu()
{
	probud();
	naladPredchoziFrekvenci(false);
}

void nastavCallback()
{
	buttonPlus.reset();
	buttonMinus.reset();
	switch (indexZobrazeni) {
	case 0: {
		buttonPlus.attachClick(naladDalsiFrekvenci);
		buttonPlus.attachDoubleClick(naladDalsiStanici);
		buttonPlus.attachDuringLongPress(zvysHlasitost);
		//
		buttonMinus.attachClick(naladPredchoziFrekvenci);
		buttonMinus.attachDoubleClick(naladPredchoziStanici);
		buttonMinus.attachDuringLongPress(zmensiHlasitost);
		break;
	}
	case 1: {
		buttonPlus.attachClick(prepniStereoMono);
		buttonMinus.attachClick(prepniBassBoost);
		break;
	}
	case 2: {
		buttonPlus.attachClick(jemneLadeniNahoru);
		buttonMinus.attachClick(jemneLadeniDolu);
		break;
	}
	}
}

void prepniRezimZobrazeni()
{
	if (usporny_rezim == 1) {
		probud();
		indexZobrazeni = 0;
	} else {
		indexZobrazeni++;
	}

	lcd.clear();
	if (indexZobrazeni >= 3) {
		indexZobrazeni = 0;
	}

	nastavCallback();
}

void prepniMute()
{
	probud();
	radio.setMute(!radio.getMute());
}

void uspornyRezim()
{
	probud();
	lcd.noDisplay();
	lcd.setBacklight(0);
	delay(50);
	usporny_rezim = 1;
}

#endif /* _POMOC_H */