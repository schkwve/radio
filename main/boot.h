#ifndef _BOOT_H
#define _BOOT_H_

// clang-format off
static byte chr[3][8][8] = {
	{
		{ 0x00, 0x00, 0x00, 0x00, 0x03, 0x07, 0x0E, 0x0E },
		{ 0x00, 0x00, 0x00, 0x00, 0x0F, 0x1F, 0x1F, 0x1F },
		{ 0x00, 0x00, 0x00, 0x03, 0x07, 0x1F, 0x1F, 0x1F },
		{ 0x00, 0x00, 0x05, 0x1F, 0x1D, 0x1F, 0x16, 0x06 },
		{ 0x0C, 0x18, 0x10, 0x00, 0x01, 0x01, 0x01, 0x00 },
		{ 0x1F, 0x1F, 0x1E, 0x17, 0x00, 0x00, 0x10, 0x00 },
		{ 0x1F, 0x1F, 0x03, 0x02, 0x14, 0x04, 0x02, 0x00 },
		{ 0x1C, 0x1C, 0x04, 0x04, 0x08, 0x00, 0x00, 0x00 }
	},
	{
		{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0x0F, 0x1E },
		{ 0x00, 0x00, 0x00, 0x00, 0x0E, 0x1F, 0x1F, 0x1F },
		{ 0x00, 0x00, 0x00, 0x01, 0x07, 0x1F, 0x1F, 0x1F },
		{ 0x00, 0x05, 0x1F, 0x1D, 0x1F, 0x1B, 0x13, 0x10 },
		{ 0x13, 0x03, 0x06, 0x0C, 0x10, 0x10, 0x00, 0x00 },
		{ 0x1F, 0x17, 0x06, 0x0C, 0x10, 0x10, 0x00, 0x00 },
		{ 0x1F, 0x1F, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00 },
		{ 0x10, 0x18, 0x1C, 0x08, 0x08, 0x00, 0x10, 0x00 }
	},
	{
		{ 0x00, 0x00, 0x00, 0x07, 0x0F, 0x0E, 0x1C, 0x18 },
		{ 0x00, 0x00, 0x00, 0x0F, 0x1F, 0x1F, 0x1F, 0x1F },
		{ 0x00, 0x00, 0x01, 0x03, 0x1F, 0x1F, 0x1F, 0x1F },
		{ 0x14, 0x1C, 0x1A, 0x1E, 0x1F, 0x13, 0x10, 0x10 },
		{ 0x13, 0x13, 0x02, 0x02, 0x04, 0x00, 0x00, 0x00 },
		{ 0x1F, 0x07, 0x0E, 0x06, 0x01, 0x00, 0x00, 0x00 },
		{ 0x0F, 0x03, 0x03, 0x01, 0x01, 0x00, 0x00, 0x00 },
		{ 0x10, 0x18, 0x0C, 0x02, 0x02, 0x11, 0x00, 0x00 }
	}
};
// clang-format on

extern LiquidCrystal_PCF8574 lcd;
static int x = 0;

static void show_bootanim()
{
    lcd.clear();
    for (int i = 0; i < 6; i++)
    {
        if (x > 15)
            x = -4;

        for (int f = 0; f < 3; f++)
        {
            lcd.clear();

            for (int i = 0; i < 8; i++)
            {
                lcd.createChar(i, chr[f][i]);
            }

            for (int c = 0; c < 4; c++)
            {
                int xc = x + c;

                if ((xc >= 0) && (xc < 16))
                {
                    lcd.setCursor(x + c, 0);
                    lcd.write(byte(c));
                    lcd.setCursor(x + c, 1);
                    lcd.write(byte(c + 4));
                }
            }

            x++;
            delay(300);
        }
    }

    lcd.clear();
    lcd.setCursor(1, 0);
    lcd.print("Super Radio");
    delay(500);
    lcd.setCursor(2, 1);
    lcd.print("Jozef Nagy");
    delay(2000);
}

#endif /* _BOOT_H */
