// This is the main DLL file.

#include "stdafx.h"

#include "WrapperLibrary.h"

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
	Data = gcnew array<short>(ENCODER_CORRECTION_DATA_SIZE);
}

WrapperLibrary::Status WrapperLibrary::Connector::Connect() {
	return static_cast<Status>(::Connect());
}

WrapperLibrary::Status WrapperLibrary::Connector::Disconnect() {
	return static_cast<Status>(::Disconnect());
}

WrapperLibrary::Status WrapperLibrary::Connector::ReadConfig(Config^ config) {
	EQ::Config conf;
	ConvertToEq(conf, config);
	Status status = static_cast<Status>(::ReadConfig(conf));
	ConvertFromEq(config, conf);
	return status;
}

WrapperLibrary::Status WrapperLibrary::Connector::WriteConfig(Config^ config) {
	EQ::Config conf;
	ConvertToEq(conf, config);
	return static_cast<Status>(::WriteConfig(conf));
}

WrapperLibrary::Status WrapperLibrary::Connector::StartRA_Motor(int speed) {
	Status status = static_cast<Status>(::EQ_Init("", 0, 0, 0));
	if (status != Status::OK) {
		return status;
	}
	status = static_cast<Status>(::EQ_InitMotors(0x10000, 0x10000));
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
	Status status = static_cast<Status>(::EQ_MotorStop(EQ::MI_RA));
	::EQ_MotorStop(EQ::MI_DEC);
	::EQ_End();
	return status;
}

WrapperLibrary::Status WrapperLibrary::Connector::GetEncoderValues(int% x, int% y) {
	int xVal, yVal;
	Status status = static_cast<Status>(::GetEncoderValues(xVal, yVal));
	if (status == Status::OK) {
		x = xVal;
		y = yVal;
	}
	return status;
}

int WrapperLibrary::Connector::GetEncoderCorrectionDataSize() {
	return ENCODER_CORRECTION_DATA_SIZE;
}

WrapperLibrary::Status WrapperLibrary::Connector::WriteEncoderCorrection(EncoderCorrection^ correction) {
	if (correction->Data->Length != ENCODER_CORRECTION_DATA_SIZE) {
		return Status::INVALID_PARAMETERS;
	}
	
	int16_t buf[ENCODER_CORRECTION_DATA_SIZE];
	Marshal::Copy(correction->Data, 0, IntPtr(buf), ENCODER_CORRECTION_DATA_SIZE);
	auto result = static_cast<Status>(::WriteEncoderCorrection(correction->MinX, correction->MaxX, correction->MinY, correction->MaxY, buf));
	if (result != Status::OK) {
		return result;
	}	
	return Status::OK;
}

WrapperLibrary::Status WrapperLibrary::Connector::ReadEncoderCorrection(EncoderCorrection^ correction) {
	if (correction->Data->Length != ENCODER_CORRECTION_DATA_SIZE) {
		return Status::INVALID_PARAMETERS;
	}

	int16_t buf[ENCODER_CORRECTION_DATA_SIZE];
	int16_t minX, maxX, minY, maxY;
	auto result = static_cast<Status>(::ReadEncoderCorrection(minX, maxX, minY, maxY, buf));	
	if (result != Status::OK) {
		return result;
	}
	correction->MinX = minX;
	correction->MaxX = maxX;
	correction->MinY = minY;
	correction->MaxY = maxY;
	Marshal::Copy(IntPtr(buf), correction->Data, 0, ENCODER_CORRECTION_DATA_SIZE);
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
	config->LimitDetectorsReverse = conf.m_lLimitDetectorsReverse != 0;
	config->EmergencyStopAccelerationMultiplier = conf.m_nEmergencyStopAccelerationMultiplier;
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
	conf.m_lLimitDetectorsReverse = config->LimitDetectorsReverse;
	conf.m_nEmergencyStopAccelerationMultiplier = config->EmergencyStopAccelerationMultiplier;
}
