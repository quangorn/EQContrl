// WrapperLibrary.h

#pragma once
#include <EQContrl.h>

using namespace System;

namespace WrapperLibrary {

	public enum class Status {
		OK,
		INVALID_PARAMETERS,
		UNKNOWN_CMD,
		WRONG_CMD_SIZE,
		WRONG_RESP_SIZE,
		USB_INIT_FAILED,
		USB_DEVICE_NOT_FOUND,
		USB_COMMUNICATION_ERROR,
		USB_NOT_CONNECTED,
		RA_MOTOR_RUNNING,
		DEC_MOTOR_RUNNING,
		MOTOR_BUSY,
		MOTOR_NOT_INITIALIZED,
		MOTOR_LIMIT_REACHED,
		FLASH_ERASE_ERROR,
		FLASH_PROGRAM_ERROR,
	};

	public ref class AxisConfig {
	public:
		property int MotorMaxRate;
		property int MotorMaxAcceleration;
		property int MicrostepCount;
		property int StepsPerWormTurn;
		property int WormGear;
		property bool Reverse;

		//Automatically calculated params
		int MicrostepsDivider;
		int SiderealPeriod;
		int SiderealPsc;
	};

	public ref class Config {
	public:
		Config();

		property array<AxisConfig^>^ AxisConfigs;
		property bool PeriodicErrorCorrectionEnabled;
	};

	public ref class EncoderCorrection {
	public:
		EncoderCorrection();
		bool Equals(Object^ obj) override;

		property short MinX;
		property short MaxX;
		property short MinY;
		property short MaxY;
		property array<unsigned short>^ Data;
	};

	public ref class Connector {
	public:
		static Status Connect();
		static Status Disconnect();
		static Status ReadConfig(Config^ config);
		static Status WriteConfig(Config^ config);
		static Status StartRA_Motor(int speed);
		static Status StopRA_Motor();
		static Status GetEncoderValues(int% x, int% y, int% motorSteps, double% a);
		
		static int GetEncoderCorrectionDataSize();
		static Status WriteEncoderCorrection(EncoderCorrection^ correction);
		static Status ReadEncoderCorrection(EncoderCorrection^ correction);

	private:
		static void ConvertFromEq(Config^ config, const EQ::Config& conf);
		static void ConvertToEq(EQ::Config& conf, Config^ config);

		static bool m_lConnected = false;
	};
}
