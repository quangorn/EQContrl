#pragma once

#define MCU_TIMER_FREQ 36000000

#define USB_HID_VID 1155
#define USB_HID_PID 22355
#define USB_DATA_SIZE 64

#define EQMOD_TRACK_FACTOR 0x1000000

#define ENCODER_CORRECTION_PAGE_SIZE 60 //USB_DATA_SIZE - (size + cmd + page_num), and must be even (flash can be written only in 2 bytes)
#define ENCODER_CORRECTION_PAGES_COUNT 12
#define ENCODER_CORRECTION_DATA_SIZE ((ENCODER_CORRECTION_PAGE_SIZE * ENCODER_CORRECTION_PAGES_COUNT) / sizeof(int16_t) - 4 /* size of min-max values */)

namespace EQ {

	enum En_MotorId {
		MI_RA  = 0,
		MI_DEC = 1
	};

	enum En_Direction {
		DIR_NONE    = -1,
		DIR_FORWARD = 0, //microstep +
		DIR_REVERSE = 1  //microstep -
	};

	enum En_TrackRate {
		TR_SIDEREAL = 0,
		TR_LUNAR    = 1,
		TR_SOLAR    = 2
	};

	enum En_Hemisphere {
		HEM_NORTH = 0,
		HEM_SOUTH = 1,
	};

	enum En_ParameterType {
		PT_RA_MULTIPLIER        = 10003,
		PT_RA_TRACK_FACTOR      = 10004,
		PT_DEC_TRACK_FACTOR     = 10005,
		PT_WORM_MICROSTEP_COUNT = 10006,
	};
}
