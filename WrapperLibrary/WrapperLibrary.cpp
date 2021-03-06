// This is the main DLL file.

#include "stdafx.h"

#include "WrapperLibrary.h"

#define MOTOR_INIT_POS 0x800000

using System::Runtime::InteropServices::Marshal;

WrapperLibrary::Config::Config() {
	AxisConfigs = gcnew array<WrapperLibrary::AxisConfig^>(2);
	AxisConfigs[0] = gcnew AxisConfig();
	AxisConfigs[1] = gcnew AxisConfig();
}

WrapperLibrary::EncoderCorrection::EncoderCorrection() {
	MinX = 0;
	MaxX = 0;
	MinY = 0;
	MaxY = 0;
	Data = gcnew array<unsigned short>(ENCODER_CORRECTION_DATA_SIZE);
}

bool WrapperLibrary::EncoderCorrection::Equals(Object^ obj) {
	if (obj == nullptr || obj->GetType() != EncoderCorrection::typeid) {
		return false;
	}
	EncoderCorrection^ that = (EncoderCorrection^)obj;
	if (MinX != that->MinX ||
		MaxX != that->MaxX ||
		MinY != that->MinY ||
		MaxY != that->MaxY) {
		return false;
	}
	if (Data == nullptr && that->Data == nullptr) {
		return true;
	}
	if (Data == nullptr || that->Data == nullptr) {
		return false;
	}
	if (Data->Length != that->Data->Length) {
		return false;
	}
	for (int i = 0; i < Data->Length; i++) {
		if (Data[i] != that->Data[i]) {
			return false;
		}
	}
	return true;
}

WrapperLibrary::Status WrapperLibrary::Connector::Connect() {
	if (m_lConnected) {
		return Status::OK;
	}
	Status status = static_cast<Status>(::Connect());
	if (status == Status::OK) {
		m_lConnected = true;
	}
	return status;
}

WrapperLibrary::Status WrapperLibrary::Connector::Disconnect() {
	if (!m_lConnected) {
		return Status::OK;
	}
	m_lConnected = false;
	return static_cast<Status>(::Disconnect());
}

WrapperLibrary::Status WrapperLibrary::Connector::ReadConfig(Config^ config) {
	Status status = Connect();
	if (status != Status::OK) {
		return status;
	}
	EQ::Config conf;
	ConvertToEq(conf, config);
	status = static_cast<Status>(::ReadConfig(conf));
	ConvertFromEq(config, conf);
	return status;
}

WrapperLibrary::Status WrapperLibrary::Connector::WriteConfig(Config^ config) {
	Status status = Connect();
	if (status != Status::OK) {
		return status;
	}
	EQ::Config conf;
	ConvertToEq(conf, config);
	return static_cast<Status>(::WriteConfig(conf));
}

WrapperLibrary::Status WrapperLibrary::Connector::StartRA_Motor(int speed) {
	Status status = Connect();
	if (status != Status::OK) {
		return status;
	}
	status = static_cast<Status>(::EQ_Init("", 0, 0, 0));
	if (status != Status::OK) {
		return status;
	}
	status = static_cast<Status>(::EQ_InitMotors(MOTOR_INIT_POS, MOTOR_INIT_POS));
	if (status != Status::OK) {
		return status;
	}
	int rate = speed;
	int direction = EQ::DIR_FORWARD;
	if (speed < 0) {
		rate = EQMOD_TRACK_FACTOR / -rate + 30000;
		direction = EQ::DIR_REVERSE;
	} else {
		rate = EQMOD_TRACK_FACTOR / rate + 30000;
	}
	return static_cast<Status>(::EQ_SetCustomTrackRate(EQ::MI_RA, 1, rate, 0, 0, direction));
}

WrapperLibrary::Status WrapperLibrary::Connector::StopRA_Motor() {
	::EQ_MotorStop(EQ::MI_DEC);
	Status status = static_cast<Status>(::EQ_MotorStop(EQ::MI_RA));
	if (status != Status::OK) {
		Disconnect();
		return status;
	}
	int motorStatus = 0;
	do {
		motorStatus = ::EQ_GetMotorStatus(EQ::MI_RA);
	} while (motorStatus == EQ::MS_ROTATING_FRONT || motorStatus == EQ::MS_ROTATING_REAR);
	if (motorStatus != EQ::MS_NOT_ROTATING_FRONT && motorStatus != EQ::MS_NOT_ROTATING_REAR) {
		Disconnect();
		return Status::MOTOR_NOT_INITIALIZED;
	}
	int motorPosition = EQ_GetMotorValues(EQ::MI_RA);
	if (motorPosition >= 0x1000000) {
		Disconnect();
		return Status::USB_COMMUNICATION_ERROR;
	}
	int delta = motorPosition - MOTOR_INIT_POS;
	motorStatus = ::EQ_StartMoveMotor(EQ::MI_RA, 0, delta > 0 ? EQ::DIR_REVERSE : EQ::DIR_FORWARD,
		::abs(delta), 0);

	do {
		::Sleep(50);
		motorStatus = ::EQ_GetMotorStatus(EQ::MI_RA);		
	} while (motorStatus == EQ::MS_ROTATING_FRONT || motorStatus == EQ::MS_ROTATING_REAR);
	if (motorStatus != EQ::MS_NOT_ROTATING_FRONT && motorStatus != EQ::MS_NOT_ROTATING_REAR) {
		Disconnect();
		return Status::MOTOR_NOT_INITIALIZED;
	}
	motorPosition = EQ_GetMotorValues(EQ::MI_RA);
	Disconnect();
	return motorPosition == MOTOR_INIT_POS ? Status::OK : Status::MOTOR_BUSY;
}

WrapperLibrary::Status WrapperLibrary::Connector::GetEncoderValues(int% x, int% y, int% motorSteps, double% a) {
	int xVal, yVal, motorVal;
	double angle;
	Status status = static_cast<Status>(::GetEncoderValues(xVal, yVal, motorVal, angle));
	if (status == Status::OK) {
		x = xVal;
		y = yVal;
		a = angle;
		motorSteps = motorVal;
	}
	return status;
}

int WrapperLibrary::Connector::GetEncoderCorrectionDataSize() {
	return ENCODER_CORRECTION_DATA_SIZE;
}

WrapperLibrary::Status WrapperLibrary::Connector::WriteEncoderCorrection(EncoderCorrection^ correction) {
	Status status = Connect();
	if (status != Status::OK) {
		return status;
	}
	if (correction->Data->Length != ENCODER_CORRECTION_DATA_SIZE) {
		return Status::INVALID_PARAMETERS;
	}
	
	uint16_t buf[ENCODER_CORRECTION_DATA_SIZE];
	Marshal::Copy((array<short>^)correction->Data, 0, IntPtr(buf), ENCODER_CORRECTION_DATA_SIZE);
	return static_cast<Status>(::WriteEncoderCorrection(correction->MinX, correction->MaxX, correction->MinY, correction->MaxY, buf));
}

WrapperLibrary::Status WrapperLibrary::Connector::ReadEncoderCorrection(EncoderCorrection^ correction) {
	Status status = Connect();
	if (status != Status::OK) {
		return status;
	}
	if (correction->Data->Length != ENCODER_CORRECTION_DATA_SIZE) {
		return Status::INVALID_PARAMETERS;
	}

	uint16_t buf[ENCODER_CORRECTION_DATA_SIZE];
	int16_t minX, maxX, minY, maxY;
	status = static_cast<Status>(::ReadEncoderCorrection(minX, maxX, minY, maxY, buf));
	if (status != Status::OK) {
		return status;
	}
	correction->MinX = minX;
	correction->MaxX = maxX;
	correction->MinY = minY;
	correction->MaxY = maxY;
	Marshal::Copy(IntPtr(buf), (array<short>^)correction->Data, 0, ENCODER_CORRECTION_DATA_SIZE);
	return Status::OK;
}

void WrapperLibrary::Connector::ConvertFromEq(WrapperLibrary::Config^ config, const EQ::Config& conf) {
	for (int i = 0; i < config->AxisConfigs->Length; i++) {
		auto& src = conf.m_AxisConfigs[i];
		AxisConfig^ dst = (AxisConfig^)config->AxisConfigs[i];
		dst->MotorMaxRate = src.m_nMotorMaxRate;
		dst->MotorMaxAcceleration = src.m_nMotorMaxAcceleration;
		dst->MicrostepCount = src.m_nMicrostepCount;
		dst->StepsPerWormTurn = src.m_nStepsPerWormTurn;
		dst->WormGear = src.m_nWormGear;
		dst->Reverse = src.m_lReverse != 0;

		dst->MicrostepsDivider = src.m_nMicrostepsDivider;
		dst->SiderealPeriod = src.m_nSiderealPeriod;
		dst->SiderealPsc = src.m_nSiderealPsc;
	}
	config->PeriodicErrorCorrectionEnabled = conf.m_lPeriodicErrorCorrectionEnabled != 0;
}

void WrapperLibrary::Connector::ConvertToEq(EQ::Config& conf, WrapperLibrary::Config^ config) {
	for (int i = 0; i < config->AxisConfigs->Length; i++) {
		auto& dst = conf.m_AxisConfigs[i];
		AxisConfig^ src = (AxisConfig^)config->AxisConfigs[i];
		dst.m_nMotorMaxRate = src->MotorMaxRate;
		dst.m_nMotorMaxAcceleration = src->MotorMaxAcceleration;
		dst.m_nMicrostepCount = src->MicrostepCount;
		dst.m_nStepsPerWormTurn = src->StepsPerWormTurn;
		dst.m_nWormGear = src->WormGear;
		dst.m_lReverse = src->Reverse;
	}
	conf.m_lPeriodicErrorCorrectionEnabled = config->PeriodicErrorCorrectionEnabled;
}
