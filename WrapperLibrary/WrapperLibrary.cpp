// This is the main DLL file.

#include "stdafx.h"

#include "WrapperLibrary.h"

WrapperLibrary::Config::Config() {
	AxisConfigs = gcnew array<WrapperLibrary::AxisConfig^>(2);
	AxisConfigs[0] = gcnew AxisConfig();
	AxisConfigs[1] = gcnew AxisConfig();
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

void WrapperLibrary::Connector::ConvertFromEq(WrapperLibrary::Config^ config, const EQ::Config& conf) {
	for (int i = 0; i < config->AxisConfigs->Length; i++) {
		auto& src = conf.m_AxisConfigs[i];
		auto dst = config->AxisConfigs[i];
		dst->MotorMaxRate = src.m_nMotorMaxRate;
		dst->MotorMaxAcceleration = src.m_nMotorMaxAcceleration;
		dst->MicrostepCount = src.m_nMicrostepCount;
		dst->StepsPerWormTurn = src.m_nStepsPerWormTurn;
		dst->WormGear = src.m_nWormGear;
		dst->Reverse = (bool)src.m_lReverse;

		dst->MicrostepsDivider = src.m_nMicrostepsDivider;
		dst->SiderealPeriod = src.m_nSiderealPeriod;
		dst->SiderealPsc = src.m_nSiderealPsc;
	}
	config->LimitDetectorsReverse = (bool)conf.m_lLimitDetectorsReverse;
	config->EmergencyStopAccelerationMultiplier = conf.m_nEmergencyStopAccelerationMultiplier;
}

void WrapperLibrary::Connector::ConvertToEq(EQ::Config& conf, WrapperLibrary::Config^ config) {
	for (int i = 0; i < config->AxisConfigs->Length; i++) {
		auto& dst = conf.m_AxisConfigs[i];
		auto src = config->AxisConfigs[i];
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
