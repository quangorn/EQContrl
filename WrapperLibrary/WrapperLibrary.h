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
		int MaxSpeed;
		int MaxFreq;
		int Microsteps;
	};

	public ref class Config {
	public:
		Config();
		array<AxisConfig^>^ AxisConfigs;
	};

	public ref class Connector {
	public:
		static Status Connect();
		static Status Disconnect();
		static Config^ GetConfig();
		static Status WriteConfig(Config^ config);

	private:
		static Config^ Convert(const EQ::Config& config);
		static void Convert(EQ::Config& conf, Config^ config);
	};
}
