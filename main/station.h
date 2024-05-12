#ifndef _STATION_H
#define _STATION_H

#include <RDA5807M.h>

struct radio_station {
	const char *name;
	RADIO_FREQ freq;
};

// predem nadefinovane stanice
// frekvence se pise bez desetinnych carek:
//   - 89.8 FM -> 8980
//   - 103.7 FM -> 10370

// clang-format off
static struct radio_station stanice[] = {
	{
		"Radio CAS Rock",
		8950 // 89.8 FM
	},
	{
		"Radio KISS",
		8980 // 89.8 FM
	},
	{
		"Frekvence 1",
		9110 // 91.1 FM
	},
	{
		"CRo Radiozurnal",
		9950 // 99.5 FM
	},
	{
		"CRo Dvojka",
		9680 // 96.8 FM
	},
	{
		"Frekvence 1",
		10420 // 104.2 FM
	}
};
// clang-format on

#endif /* _STATION_H */