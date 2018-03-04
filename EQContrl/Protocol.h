#pragma once

#include <stdint.h>
#include "Definitions.h"

#pragma pack (push, 1)

namespace EQ {
	enum En_Command {
		CMD_INIT_MOTORS,
		CMD_DEINIT_MOTORS,
		CMD_GET_MOTOR_STATUS,
		CMD_GET_MOTOR_VALUES,
		CMD_SET_MOTOR_VALUES,
		CMD_STOP_MOTOR,
		CMD_SLEW,
		CMD_GOTO,
		CMD_START_TRACK,
		CMD_READ_CONFIG,
		CMD_WRITE_CONFIG,
		CMD_READ_ENCODER_CORRECTION,
		CMD_WRITE_ENCODER_CORRECTION,
		CMD_CLEAR_ENCODER_CORRECTION,
	};

	enum En_Status {
		STS_OK,
		STS_INVALID_PARAMETERS,
		STS_UNKNOWN_CMD,
		STS_WRONG_CMD_SIZE,
		STS_WRONG_RESP_SIZE,
		STS_USB_INIT_FAILED,
		STS_USB_DEVICE_NOT_FOUND,
		STS_USB_COMMUNICATION_ERROR,
		STS_USB_NOT_CONNECTED,
		STS_RA_MOTOR_RUNNING,
		STS_DEC_MOTOR_RUNNING,
		STS_MOTOR_BUSY,
		STS_MOTOR_NOT_INITIALIZED,
		STS_MOTOR_LIMIT_REACHED,
		STS_FLASH_ERASE_ERROR,
		STS_FLASH_PROGRAM_ERROR,
	};

	struct AxisConfig {
		uint16_t m_nMotorMaxRate;			//Max speed (1 = sidereal)
		uint16_t m_nMicrostepCount;			//Microsteps per 1 step
		uint16_t m_nStepsPerWormTurn;		//Full motor steps per 1 worm turn
		uint16_t m_nWormGear;				//Worm gear turns per 360 turn
		uint8_t m_nMotorMaxAcceleration;	//Sidereal rates per 12.5 ms
		uint8_t m_lReverse;					//Reverse motor direction

		//Automatically calculated params
		uint16_t m_nMicrostepsDivider;		//Divider to microsteps count (needed to limit count to EQMOD max value)
		uint16_t m_nSiderealPeriod;			//First timer freq divider for sidereal rate
		uint16_t m_nSiderealPsc;			//Second timer freq divider for sidereal rate
	};

	struct Config {
		AxisConfig m_AxisConfigs[2];
		uint8_t m_lPeriodicErrorCorrectionEnabled;
	};

	struct EqReq {
		EqReq(En_Command nCmd) :
			m_nCmd(nCmd) {
		}

		uint8_t m_nCmd;
	};

	struct EqResp {
		EqResp() :
			m_nStatus(STS_OK)
		{}
		EqResp(En_Status nStatus) :
			m_nStatus(nStatus) {
		}

		uint8_t m_nStatus;
	};


	struct EqInitMotorsReq : public EqReq {
		EqInitMotorsReq(uint32_t nRaVal, uint32_t nDecVal) :
			EqReq(CMD_INIT_MOTORS),
			m_nRaVal(nRaVal),
			m_nDecVal(nDecVal) {
		}

		uint32_t m_nRaVal;
		uint32_t m_nDecVal;
	};

	struct EqGetMotorStatusReq : public EqReq {
		EqGetMotorStatusReq(En_MotorId nMotorId) :
			EqReq(CMD_GET_MOTOR_STATUS),
			m_nMotorId(nMotorId) {
		}

		uint8_t m_nMotorId;
	};

	struct EqGetMotorStatusResp : public EqResp {
		EqGetMotorStatusResp() :
			EqResp(STS_OK),
			m_lEnabled(false),
			m_lRunning(false),
			m_nDirection(DIR_FORWARD) {
		}
		EqGetMotorStatusResp(bool lEnabled, bool lRunning, En_Direction nDirection) :
			EqResp(STS_OK),
			m_lEnabled(lEnabled),
			m_lRunning(lRunning),
			m_nDirection(nDirection) {
		}

		uint8_t m_lEnabled;
		uint8_t m_lRunning;
		uint8_t m_nDirection;
	};

	struct EqGetMotorValuesReq : public EqReq {
		EqGetMotorValuesReq(En_MotorId nMotorId) :
			EqReq(CMD_GET_MOTOR_VALUES),
			m_nMotorId(nMotorId) {
		}

		uint8_t m_nMotorId;
	};

	struct EqGetMotorValuesResp : public EqResp {
		EqGetMotorValuesResp() :
			EqResp(STS_OK),
			m_nMicrostepCount(0),
			m_nEncoderValueX(0),
			m_nEncoderValueY(0) {
		}
		EqGetMotorValuesResp(
				uint32_t nMicrostepCount,
				uint16_t nEncoderValueX = 0,
				uint16_t nEncoderValueY = 0) :
			EqResp(STS_OK),
			m_nMicrostepCount(nMicrostepCount),
			m_nEncoderValueX(nEncoderValueX),
			m_nEncoderValueY(nEncoderValueY) {
		}

		uint32_t m_nMicrostepCount;
		int16_t m_nEncoderValueX;
		int16_t m_nEncoderValueY;
	};

	struct EqSetMotorValuesReq : public EqReq {
		EqSetMotorValuesReq(En_MotorId nMotorId, uint32_t nMotorVal) :
			EqReq(CMD_SET_MOTOR_VALUES),
			m_nMotorId(nMotorId),
			m_nMotorVal(nMotorVal) {
		}

		uint8_t m_nMotorId;
		uint32_t m_nMotorVal;
	};

	struct EqStopMotorReq : public EqReq {
		EqStopMotorReq(En_MotorId nMotorId) :
			EqReq(CMD_STOP_MOTOR),
			m_nMotorId(nMotorId) {
		}

		uint8_t m_nMotorId;
	};

	struct EqSlewReq : public EqReq {
		EqSlewReq(En_MotorId nMotorId, En_Hemisphere nHemisphere, En_Direction nDirection, uint16_t nRate) :
			EqReq(CMD_SLEW),
			m_nMotorId(nMotorId),
			m_nHemisphere(nHemisphere),
			m_nDirection(nDirection),
			m_nRate(nRate) {
		}

		uint8_t m_nMotorId;
		uint8_t m_nHemisphere;
		uint8_t m_nDirection;
		uint16_t m_nRate;
	};

	struct EqGoToReq : public EqReq {
		EqGoToReq(En_MotorId nMotorId, En_Hemisphere nHemisphere, En_Direction nDirection, uint32_t nStepCount) :
			EqReq(CMD_GOTO),
			m_nMotorId(nMotorId),
			m_nHemisphere(nHemisphere),
			m_nDirection(nDirection),
			m_nStepCount(nStepCount) {
		}

		uint8_t m_nMotorId;
		uint8_t m_nHemisphere;
		uint8_t m_nDirection;
		uint32_t m_nStepCount;
	};

	struct EqStartTrackReq : public EqReq {
		EqStartTrackReq(uint8_t nMotorId, En_Hemisphere nHemisphere, En_Direction nDirection, 
				uint16_t nFirstPrescaler, uint16_t nSecondPrescaler) :
			EqReq(CMD_START_TRACK),
			m_nMotorId(nMotorId),
			m_nHemisphere(nHemisphere),
			m_nDirection(nDirection),
			m_nFirstPrescaler(nFirstPrescaler),
			m_nSecondPrescaler(nSecondPrescaler) {
		}

		uint8_t m_nMotorId;
		uint8_t m_nHemisphere;
		uint8_t m_nDirection;
		uint16_t m_nFirstPrescaler;
		uint16_t m_nSecondPrescaler;
	};

	struct EqReadConfigResp : public EqResp {
		EqReadConfigResp() :
			EqResp(STS_OK) {
		}

		EqReadConfigResp(const Config& Config) :
			EqResp(STS_OK),
			m_Config(Config) {
		}

		Config m_Config;
	};

	struct EqWriteConfigReq : public EqReq {
		EqWriteConfigReq(const Config& Config) :
			EqReq(CMD_WRITE_CONFIG),
			m_Config(Config) {
		}

		Config m_Config;
	};

	struct EqReadEncoderCorrectionReq : public EqReq {
		EqReadEncoderCorrectionReq(uint8_t nPageNumber) :
			EqReq(CMD_READ_ENCODER_CORRECTION),
			m_nPageNumber(nPageNumber)
		{}

		uint8_t m_nPageNumber;
	};

	struct EqReadEncoderCorrectionResp : public EqResp {
		EqReadEncoderCorrectionResp() :
			EqResp(STS_OK) {
		}

		uint8_t m_Data[ENCODER_CORRECTION_PAGE_SIZE];
	};

	struct EqWriteEncoderCorrectionReq : public EqReq {
		EqWriteEncoderCorrectionReq(uint8_t nPageNumber) :
			EqReq(CMD_WRITE_ENCODER_CORRECTION),
			m_nPageNumber(nPageNumber)
		{}

		uint8_t m_nPageNumber;
		uint8_t m_Data[ENCODER_CORRECTION_PAGE_SIZE];
	};
}

#pragma pack (pop)
