// This is the main DLL file.

#include "stdafx.h"

#include "WrapperLibrary.h"



WrapperLibrary::Status WrapperLibrary::Connector::Connect() {
	return static_cast<Status>(::Connect());
}

WrapperLibrary::Status WrapperLibrary::Connector::Disconnect() {
	return static_cast<Status>(::Disconnect());
}

WrapperLibrary::Config^ WrapperLibrary::Connector::GetConfig() {
	return Convert(::GetConfig());
}

WrapperLibrary::Config^ WrapperLibrary::Connector::Convert(const EQ::Config& config) {
	auto conf = gcnew WrapperLibrary::Config();
	conf->AxisConfigs = gcnew array<WrapperLibrary::AxisConfig^>(sizeof(config.m_AxisConfigs) / sizeof(config.m_AxisConfigs[0]));
	for (int i = 0; i < conf->AxisConfigs->Length; i++) {
		conf->AxisConfigs[i] = gcnew AxisConfig();
		auto& src = config.m_AxisConfigs[i];
		auto dst = conf->AxisConfigs[i];
		dst->MaxSpeed = src.m_nMaxSpeed;
		dst->MaxFreq = src.m_nMaxFreq;
		dst->Microsteps = src.m_nMicrosteps;
	}
	return conf;
}
