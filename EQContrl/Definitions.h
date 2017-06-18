#pragma once

#define MCU_TIMER_FREQ 36000000

#define USB_HID_VID 1155
#define USB_HID_PID 22355
#define USB_DATA_SIZE 32

#define EQMOD_TRACK_FACTOR 0x1000000

#define MICROSTEP_RA             32
#define MOTOR_STEPS_PER_TURN_RA  200
#define MOTOR_GEAR_RA            3
#define WORM_MICROSTEP_COUNT_RA  (uint32_t)(MICROSTEP_RA * MOTOR_STEPS_PER_TURN_RA * MOTOR_GEAR_RA)
#define WORM_GEAR_RA             180
#define TOTAL_MICROSTEP_COUNT_RA (WORM_MICROSTEP_COUNT_RA * WORM_GEAR_RA)

#define MICROSTEP_DEC             16
#define MOTOR_STEPS_PER_TURN_DEC  200
#define MOTOR_GEAR_DEC            2.5
#define WORM_MICROSTEP_COUNT_DEC  (uint32_t)(MICROSTEP_DEC * MOTOR_STEPS_PER_TURN_DEC * MOTOR_GEAR_DEC)
#define WORM_GEAR_DEC             980
#define TOTAL_MICROSTEP_COUNT_DEC (WORM_MICROSTEP_COUNT_DEC * WORM_GEAR_DEC)

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
