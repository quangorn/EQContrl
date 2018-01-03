// EQContrl.cpp : Defines the exported functions for the DLL application.
//

#include <windows.h>
#include <fstream>
#include <time.h>
#include <string>
#include <sstream>
#include "hidapi.h"
#include "Utils.h"
#include "EQContrl.h"
#include "Protocol.h"
#include "RateCalculator.h"
#include "AngleCalculator.h"

using namespace std;
using namespace EQ;

static hid_device *pHandle = nullptr;
static uint8_t UsbBuf[USB_DATA_SIZE + 1];
static RateCalculator m_RateCalculator;
static AngleCalculator m_AngleCalculator;
static Config m_Config;

uint32_t WormMicrostepCount(int nMotorId, const Config& config) {
	return (uint32_t)config.m_AxisConfigs[nMotorId].m_nMicrostepCount *
		config.m_AxisConfigs[nMotorId].m_nStepsPerWormTurn /
		(config.m_AxisConfigs[nMotorId].m_nMicrostepsDivider / 2);
}

uint32_t WormMicrostepCount(int nMotorId) {
	return WormMicrostepCount(nMotorId, m_Config);
}

uint32_t TotalMicrostepCount(int nMotorId, const Config& config) {
	return WormMicrostepCount(nMotorId, config) * config.m_AxisConfigs[nMotorId].m_nWormGear;
}

uint32_t TotalMicrostepCount(int nMotorId) {
	return TotalMicrostepCount(nMotorId, m_Config);
}

DWORD Convert(En_Status nStatus) {
	switch (nStatus) {
	case STS_OK: return 0;
	case STS_INVALID_PARAMETERS: return 999;
	case STS_UNKNOWN_CMD: return 999;
	case STS_WRONG_CMD_SIZE: return 999;
	case STS_WRONG_RESP_SIZE: return 999;
	case STS_USB_INIT_FAILED: return 1;
	case STS_USB_DEVICE_NOT_FOUND: return 1;
	case STS_USB_COMMUNICATION_ERROR: return 1;
	case STS_USB_NOT_CONNECTED: return 10;
	case STS_RA_MOTOR_RUNNING: return 6;
	case STS_DEC_MOTOR_RUNNING: return 7;
	case STS_MOTOR_BUSY: return 4;
	case STS_MOTOR_NOT_INITIALIZED: return 11;
	case STS_MOTOR_LIMIT_REACHED: return 10;
	default: return 999;
	}
}

En_Status Connect() {
	LOG("Connect");
	if (pHandle != nullptr) {
		return STS_OK;
	}

	if (hid_init()) {
		LOG("hid_init failed");
		return STS_USB_INIT_FAILED;
	}

	pHandle = hid_open(USB_HID_VID, USB_HID_PID, NULL);
	if (pHandle == nullptr) {
		LOG("unable to open device");
		return STS_USB_DEVICE_NOT_FOUND;
	}

	return STS_OK;
}

En_Status Disconnect() {
	LOG("Disconnect");
	En_Status status = STS_OK;

	if (pHandle == nullptr) {
		return status;
	}
	
	if (pHandle != nullptr) {
		EqResp Resp;
		status = SendAndReadResp(EqDeInitMotorsReq(), Resp);

		hid_close(pHandle);
		pHandle = nullptr;

		/* Free static HIDAPI objects. */
		hid_exit();
	}

	return status;
}

En_Status ReadConfig(Config& config) {
	EqReadConfigResp Resp;
	En_Status status = SendAndReadResp(EqReq(CMD_READ_CONFIG), Resp);
	if (status == STS_OK) {
		config = Resp.m_Config;
	}
	LOG("Read config result: " << status);
	return status;
}

En_Status WriteConfig(const Config& config) {
	Config modConfig = config;
	for (int i = EQ::MI_RA; i <= EQ::MI_DEC; i++) {
		modConfig.m_AxisConfigs[i].m_nMicrostepsDivider = 2;
		while (TotalMicrostepCount(i, modConfig) >= 0x1000000) {
			modConfig.m_AxisConfigs[i].m_nMicrostepsDivider *= 2;
		}

		m_RateCalculator.CalculatePrescalersFromRate(
			TotalMicrostepCount(i, modConfig),
			m_RateCalculator.GetRate(TR_SIDEREAL),
			modConfig.m_AxisConfigs[i].m_nSiderealPeriod,
			modConfig.m_AxisConfigs[i].m_nSiderealPsc
		);
	}

	En_Status status = SendReq(EqWriteConfigReq(modConfig));
	if (status == STS_OK) {
		m_Config = modConfig;
	}
	LOG("Write config result: " << status);
	return status;
}

En_Status GetEncoderValues(int& x, int& y) {
	EqGetMotorValuesResp Resp;
	En_Status nStatus = SendAndReadResp(EqGetMotorValuesReq(MI_RA), Resp);
	if (nStatus == STS_OK) {
		//m_AngleCalculator.CalculateAngle(Resp.m_nEncoderValueX, Resp.m_nEncoderValueY);
		x = Resp.m_nEncoderValueX;
		y = Resp.m_nEncoderValueY;
	}	
	return nStatus;
}

template <typename T>
En_Status SendReq(const T& Req) {
	if (pHandle == nullptr)
		return STS_USB_NOT_CONNECTED;

	uint8_t nSize = sizeof(Req);
	UsbBuf[0] = 0;
	UsbBuf[1] = nSize;
	memcpy(UsbBuf + 2, &Req, nSize);

	int res = hid_send_feature_report(pHandle, UsbBuf, sizeof(UsbBuf));
	if (res < 0) {
		LOG("Unable to write()");
		LOG("Error: " << hid_error(pHandle));
		return STS_USB_COMMUNICATION_ERROR;
	}
	return STS_OK;
}

template <typename T>
En_Status ReadResp(T& Resp) {
	if (pHandle == nullptr)
		return STS_USB_NOT_CONNECTED;

	int nReadSize = hid_read(pHandle, UsbBuf, sizeof(UsbBuf));
	//std::stringstream sstr;
	//for (int i = 0; i < nReadSize; i++)
	//	sstr << (int)UsbBuf[i] << " ";
	//LOG("Data read(" << nReadSize << "): " << sstr.str());

	uint8_t nRespSize = sizeof(Resp);
	uint8_t nStatusRespSize = sizeof(EqResp);
	if (nReadSize >= nRespSize + 1 && nRespSize == UsbBuf[0]) {
		memcpy(&Resp, UsbBuf + 1, nRespSize);
		if (auto *pEqResp = dynamic_cast<EqResp*>(&Resp)) {
			LOG("Resp status: " << (int)pEqResp->m_nStatus);
			return (En_Status)pEqResp->m_nStatus;
		}
		return STS_OK;
	} else if (nReadSize >= nStatusRespSize + 1 && nStatusRespSize == UsbBuf[0]) {
		auto *pEqResp = (EqResp*)(UsbBuf + 1);
		LOG("Resp status: " << (int)pEqResp->m_nStatus);
		return (En_Status)pEqResp->m_nStatus;
	}
	LOG("ReadResp failed. Required size: " << nRespSize <<
		"; Read size: " << nReadSize << "; Decoded size: " << UsbBuf[0]);
	return STS_WRONG_RESP_SIZE;
}

template <typename T, typename K>
En_Status SendAndReadResp(const T& Req, K& Resp) {
	En_Status nStatus = SendReq(Req);
	if (nStatus != STS_OK)
		return nStatus;

	return ReadResp(Resp);
}

//' Function name    : EQ_Init()
//' Description      : Connect to the EQ Controller via Serial and initialize the stepper board
//' Return type      : DOUBLE
//'                      000 - Success
//'                      001 - COM Port Not available
//'                      002 - COM Port already Open
//'                      003 - COM Timeout Error
//'                      005 - Mount Initialized on using non-standard parameters
//'                      010 - Cannot execute command at the current stepper controller state
//'                      999 - Invalid parameter
//' Argument         : STRING COMPORT Name
//' Argument         : DOUBLE baud - Baud Rate
//' Argument         : DOUBLE timeout - COMPORT Timeout (1 - 50000)
//' Argument         : DOUBLE retry - COMPORT Retry (0 - 100)
EQCONTRL_API DWORD __stdcall EQ_Init(char *comportname, DWORD baud, DWORD timeout, DWORD retry) {
	LOG("EQ_Init() comport:" << comportname << "; baud:" << baud << "; timeout:" << 
		timeout << "; retry:" << retry << ";");
	En_Status status = Connect();
	if (status == STS_OK) {
		status = ReadConfig(m_Config);
	}
	DWORD ret = Convert(status);
	LOG("EQ_Init() return:" << ret << ";");
	return ret;
}

//' Function name    : EQ_InitMotors()
//' Description      : Initialize RA/DEC Motors and activate Motor Driver Coils
//' Return type      : DOUBLE
//'                     000 - Success
//'                     001 - COM PORT Not available
//'                     003 - COM Timeout Error
//'                     006 - RA Motor still running
//'                     007 - DEC Motor still running
//'                     008 - Error Initializing RA Motor
//'                     009 - Error Initilizing DEC Motor
//'                     010 - Cannot execute command at the current stepper controller state
//' Argument         : DOUBLE RA_val       Initial ra microstep counter value
//' Argument         : DOUBLE DEC_val     Initial dec microstep counter value
//'
EQCONTRL_API DWORD __stdcall EQ_InitMotors(DWORD RA_val, DWORD DEC_val) {
	LOG("EQ_InitMotors() RA:" << RA_val << "; DEC:" << DEC_val << ";");
	DWORD ret = 0;
	EqResp Resp;
	ret = Convert(SendAndReadResp(EqInitMotorsReq(RA_val, DEC_val), Resp));
	LOG("EQ_InitMotors() return:" << ret << ";");
	return ret;
}

//' Function name    : EQ_End()
//' Description      : Close the COM Port and end EQ Connection
//' Return type      : DOUBLE
//'          00 - Success
//'          01 - COM Port Not Openavailable
//'
EQCONTRL_API DWORD __stdcall EQ_End() {
	LOG("EQ_End()");
	DWORD ret = Convert(Disconnect());
	LOG("EQ_End() return:" << ret << ";");
	return ret;
}

//' Function name    : EQ_MotorStop()
//' Description      : Stop RA/DEC Motor
//' Return type      : DOUBLE
//'                     000 - Success
//'                     001 - Comport Not available
//'                     003 - COM Timeout Error
//'                     010 - Cannot execute command at the current stepper controller state
//'                     011 - Motor not initialized
//'                     999 - Invalid Parameter
//' Argument         : DOUBLE motor_id
//'                     00 - RA Motor
//'                     01 - DEC Motor
//'
EQCONTRL_API DWORD __stdcall EQ_MotorStop(DWORD motor_id) {
	LOG("EQ_MotorStop() motor_id:" << motor_id << ";");
	DWORD ret = 0;
	EqResp Resp;
	ret = Convert(SendAndReadResp(EqStopMotorReq((En_MotorId)motor_id), Resp));
	LOG("EQ_MotorStop() return:" << ret << ";");
	return ret;
}

//' Function name    : EQ_StartMoveMotor
//' Description      : Slew RA/DEC Motor based on provided microstep counts
//' Return type      : DOUBLE
//'                     000 - Success
//'                     001 - COM PORT Not available
//'                     003 - COM Timeout Error
//'                     004 - Motor still busy, aborted
//'                     010 - Cannot execute command at the current stepper controller state
//'                     011 - Motor not initialized
//'                     999 - Invalid Parameter
//' Argument         : DOUBLE motor_id
//'                     00 - RA Motor
//'                     01 - DEC Motor
//' Argument         : DOUBLE hemisphere
//'                     00 - North
//'                     01 - South
//' Argument         : DOUBLE direction
//'                     00 - Forward(+)
//'                     01 - Reverse(-)
//' Argument         : DOUBLE steps count
//' Argument         : DOUBLE motor de-acceleration  point (set between 50% t0 90% of total steps)
//'
EQCONTRL_API DWORD __stdcall EQ_StartMoveMotor(
			DWORD motor_id, DWORD hemisphere, DWORD direction, DWORD steps, DWORD stepslowdown) {
	LOG("EQ_StartMoveMotor() motor_id:" << motor_id << "; hemisphere:" << hemisphere << "; direction:" <<
		direction << "; steps:" << steps << "; stepslowdown:" << stepslowdown << ";");
	DWORD ret = 0;
	EqResp Resp;
	ret = Convert(SendAndReadResp(EqGoToReq(
		(En_MotorId)motor_id, (En_Hemisphere)hemisphere, (En_Direction)direction, steps), Resp));
	LOG("EQ_StartMoveMotor() return:" << ret << ";");
	return ret;
}

//' Function name    : EQ_GetMotorValues()
//' Description      : Get RA/DEC Motor microstep counts
//' Return type      : Double - Stepper Counter Values
//'                     0 - 16777215  Valid Count Values
//'                     0x1000000 - Mount Not available
//'                     0x1000005 - COM TIMEOUT
//'                     0x10000FF - Illegal Mount reply
//'                     0x3000000 - Invalid Parameter
//' Argument         : DOUBLE motor_id
//'                     00 - RA Motor
//'                     01 - DEC Motor
//'
EQCONTRL_API DWORD __stdcall EQ_GetMotorValues(DWORD motor_id) {
	LOG("EQ_GetMotorValues() motor_id:" << motor_id << ";");
	DWORD ret = 0;
	EqGetMotorValuesResp Resp;
	En_Status nStatus = SendAndReadResp(EqGetMotorValuesReq((En_MotorId)motor_id), Resp);
	if (nStatus == STS_USB_NOT_CONNECTED ||
		nStatus == STS_USB_COMMUNICATION_ERROR)
		ret = 0x1000000;
	else if (nStatus != STS_OK)
		ret = 0x10000FF;
	else {
		ret = Resp.m_nMicrostepCount;
		if (motor_id == MI_RA) {
			m_AngleCalculator.CalculateAngle(Resp.m_nEncoderValueX, Resp.m_nEncoderValueY);
		}
	}
	LOG("EQ_GetMotorValues() return:" << ret << ";");
	return ret;
}

//' Function name    : EQ_GetMotorStatus()
//' Description      : Get RA/DEC Stepper Motor Status
//' Return type      : DOUBLE
//'                     128 - Motor not rotating, Teeth at front contact
//'                     144 - Motor rotating, Teeth at front contact
//'                     160 - Motor not rotating, Teeth at rear contact
//'                     176 - Motor rotating, Teeth at rear contact
//'                     200 - Motor not initialized
//'                     001 - COM Port Not available
//'                     003 - COM Timeout Error
//'                     999 - Invalid Parameter
//' Argument         : DOUBLE motor_id
//'                     00 - RA Motor
//'                     01 - DEC Motor
//
EQCONTRL_API DWORD __stdcall EQ_GetMotorStatus(DWORD motor_id) {
	LOG("EQ_GetMotorStatus() motor_id:" << motor_id << ";");
	EqGetMotorStatusResp Resp;
	En_Status nStatus = SendAndReadResp(EqGetMotorStatusReq((En_MotorId)motor_id), Resp);
	DWORD ret;
	if (nStatus == STS_OK) {
		if (Resp.m_lEnabled) {
			if (Resp.m_nDirection == DIR_REVERSE) { //TODO: возможно надо поменять местами
				ret = Resp.m_lRunning ? 144 : 128;
			} else {
				ret = Resp.m_lRunning ? 176 : 160;
			}
		}
		else {
			ret = 200;
		}
	} else {
		ret = Convert(nStatus);
	}
	LOG("EQ_GetMotorStatus() return:" << ret << ";");
	return ret;
}

//' Function name    : EQ_SeTMotorValues()
//' Description      : Sets RA/DEC Motor microstep counters (pseudo encoder position)
//' Return type      : DOUBLE - Stepper Counter Values
//'                     000 - Success
//'                     001 - Comport Not available
//'                     003 - COM Timeout Error
//'                     010 - Cannot execute command at the current stepper controller state
//'                     011 - Motor not initialized
//'                     999 - Invalid Parameter
//' Argument         : DOUBLE motor_id
//'                     00 - RA Motor
//'                     01 - DEC Motor
//' Argument         : DOUBLE motor_val
//'                     0 - 16777215  Valid Count Values
//'
EQCONTRL_API DWORD __stdcall EQ_SetMotorValues(DWORD motor_id, DWORD motor_val) {
	LOG("EQ_SetMotorValues() motor_id:" << motor_id << "; motor_val:" << motor_val << ";");
	DWORD ret = 0;
	EqResp Resp;
	ret = Convert(SendAndReadResp(EqSetMotorValuesReq((En_MotorId)motor_id, motor_val), Resp));
	LOG("EQ_SetMotorValues() return:" << ret << ";");
	return ret;
}

//' Function name    : EQ_Slew()
//' Description      : Slew RA/DEC Motor based on given rate
//' Return type      : DOUBLE
//'                     000 - Success
//'                     001 - Comport Not available
//'                     003 - COM Timeout Error
//'                     004 - Motor still busy
//'                     010 - Cannot execute command at the current stepper controller state
//'                     011 - Motor not initialized
//'                     999 - Invalid Parameter
//' Argument         : DOUBLE motor_id
//'                     00 - RA Motor
//'                     01 - DEC Motor
//' Argument         : INTEGER direction
//'                    00 - Forward(+)
//'                    01 - Reverse(-)
//' Argument         : INTEGER rate
//'                         1-800 of Sidreal Rate
//'
EQCONTRL_API DWORD __stdcall EQ_Slew(DWORD motor_id, DWORD hemisphere, DWORD direction, DWORD rate) {
	LOG("EQ_Slew() motor_id:" << motor_id << "; hemisphere:" << hemisphere <<
		"; direction:" << direction << "; rate:" << rate << ";");
	DWORD ret = 0;
	EqResp Resp;
	ret = Convert(SendAndReadResp(
		EqSlewReq((En_MotorId)motor_id, (En_Hemisphere)hemisphere, (En_Direction)direction, (uint16_t)rate), 
		Resp));
	LOG("EQ_Slew() return:" << ret << ";");
	return ret;
}

//' Function name    : EQ_StartRATrack()
//' Description      : Track or rotate RA/DEC Stepper Motors at the specified rate
//' Return type      : DOUBLE
//'                     000 - Success
//'                     001 - Comport Not available
//'                     003 - COM Timeout Error
//'                     010 - Cannot execute command at the current stepper controller state
//'                     011 - Motor not initialized
//'                     999 - Invalid Parameter
//' Argument         : DOUBLE trackrate
//'                     00 - Sidreal
//'                     01 - Lunar
//'                     02 - Solar
//' Argument         : DOUBLE hemisphere
//'                     00 - North
//'                     01 - South
//' Argument         : DOUBLE direction
//'                     00 - Forward(+)
//'                     01 - Reverse(-)
//'
EQCONTRL_API DWORD __stdcall EQ_StartRATrack(DWORD trackrate, DWORD hemisphere, DWORD direction) {
	LOG("EQ_StartRATrack() trackrate:" << trackrate << "; hemisphere:" << hemisphere <<
		"; direction:" << direction << ";");
	DWORD ret = 0;
	double fRate = m_RateCalculator.GetRate((En_TrackRate)trackrate);
	uint16_t nFirst, nSecond;
	if (m_RateCalculator.CalculatePrescalersFromRate(TotalMicrostepCount(EQ::MI_RA), fRate, nFirst, nSecond)) {
		LOG("Rate: " << fRate << "; " << nFirst * nSecond <<
			" = " << nFirst << " * " << nSecond);
		EqResp Resp;
		ret = Convert(SendAndReadResp(
			EqStartTrackReq(EQ::MI_RA, (En_Hemisphere)hemisphere, (En_Direction)direction, nFirst, nSecond),
			Resp));
	}
	else {
		ret = 999;
	}
	LOG("EQ_StartRATrack() return:" << ret << ";");
	return ret;
}

//' Function name    : EQ_SendCustomTrackRate()
//' Description      : Adjust the RA/DEC rotation trackrate based on a given speed adjustment offset
//' Return type      : DOUBLE
//'                     000 - Success
//'                     001 - Comport Not available
//'                     003 - COM Timeout Error
//'                     004 - Motor still busy
//'                     010 - Cannot Execute command at the current state
//'                     011 - Motor not initialized
//'                     999 - Invalid Parameter
//'
//' Argument         : DOUBLE motor_id
//'                     00 - RA Motor
//'                     01 - DEC Motor
//' Argument         : DOUBLE trackrate
//'                     00 - Sidreal
//'                     01 - Lunar
//'                     02 - Solar
//' Argument         : DOUBLE trackoffset
//'                     0 - 300
//' Argument         : DOUBLE trackdir
//'                     00 - Positive
//'                     01 - Negative
//' Argument         : DOUBLE hemisphere (used for DEC Motor)
//'                     00 - North
//'                     01 - South
//' Argument         : DOUBLE direction (used for DEC Motor)
//'                     00 - Forward(+)
//'                     01 - Reverse(-)
//'
EQCONTRL_API DWORD __stdcall EQ_SendCustomTrackRate(DWORD motor_id, DWORD trackrate, DWORD trackoffset, DWORD trackdir,
	DWORD hemisphere, DWORD direction) {
	LOG("EQ_SendCustomTrackRate() motor_id:" << motor_id << "; trackrate:" << trackrate << "; trackoffset:" << trackoffset <<
		"; trackdir:" << trackdir << "; hemisphere:" << hemisphere << "; direction:" << direction << ";");
	DWORD ret = 0;
	LOG("EQ_SendCustomTrackRate() return:" << ret << ";");
	return ret;
}

//' Function name    : EQ_SetCustomTrackRate()
//' Description      : Adjust the RA/DEC rotation trackrate based on a given speed adjustment offset
//' Return type      : DOUBLE
//'                     000 - Success
//'                     001 - Comport Not available
//'                     003 - COM Timeout Error
//'                     004 - Motor still busy
//'                     010 - Cannot Execute command at the current state
//'                     011 - Motor not initialized
//'                     999 - Invalid Parameter
//'
//' Argument         : DOUBLE motor_id
//'                     00 - RA Motor
//'                     01 - DEC Motor
//' Argument         : DOUBLE trackmode
//'                     01 - Initial
//'                     00 - Update
//' Argument         : DOUBLE trackoffset
//' Argument         : DOUBLE trackbase
//'                     00 - LowSpeed
//' Argument         : DOUBLE hemisphere
//'                     00 - North
//'                     01 - South
//' Argument         : DOUBLE direction
//'                     00 - Forward(+)
//'                     01 - Reverse(-)
//'
EQCONTRL_API DWORD __stdcall EQ_SetCustomTrackRate(DWORD motor_id, DWORD trackmode, DWORD trackoffset, DWORD trackbase,
	DWORD hemisphere, DWORD direction) {
	LOG("EQ_SetCustomTrackRate() motor_id:" << motor_id << "; trackmode:" << trackmode << "; trackoffset:" << trackoffset <<
		"; trackbase:" << trackbase << "; hemisphere:" << hemisphere << "; direction:" << direction << ";");
	DWORD ret = 0;
	double fRate = (m_RateCalculator.GetRate(TR_SIDEREAL) * EQMOD_TRACK_FACTOR) / (trackoffset - 30000);
	uint16_t nFirst, nSecond;
	En_MotorId nMotorId = (En_MotorId)motor_id;
	if (m_RateCalculator.CalculatePrescalersFromRate(TotalMicrostepCount(nMotorId), fRate, nFirst, nSecond)) {
		LOG("Rate: " << fRate << "; " << nFirst * nSecond <<
			" = " << nFirst << " * " << nSecond);
		EqResp Resp;
		ret = Convert(SendAndReadResp(
			EqStartTrackReq(nMotorId, (En_Hemisphere)hemisphere, (En_Direction)direction, nFirst, nSecond),
			Resp));
	}
	else {
		ret = 999;
	}
	LOG("EQ_SetCustomTrackRate() return:" << ret << ";");
	return ret;
}

//' Function name    : EQ_SendGuideRate()
//' Description      : Adjust the RA/DEC rotation trackrate based on a given speed adjustment rate
//' Return type      : int
//'                     000 - Success
//'                     001 - Comport Not available
//'                     003 - COM Timeout Error
//'                     004 - Motor still busy
//'                     010 - Cannot execute command at the current stepper controller state
//'                     011 - Motor not initialized
//'                     999 - Invalid Parameter
//'
//' Argument         : DOUBLE motor_id
//'                     00 - RA Motor
//'                     01 - DEC Motor
//' Argument         : DOUBLE trackrate
//'                     00 - Sidreal
//'                     01 - Lunar
//'                     02 - Solar
//' Argument         : DOUBLE guiderate
//'                     00 - No Change
//'                     01 - 10%
//'                     02 - 20%
//'                     03 - 30%
//'                     04 - 40%
//'                     05 - 50%
//'                     06 - 60%
//'                     07 - 70%
//'                     08 - 80%
//'                     09 - 90%
//' Argument         : DOUBLE guidedir
//'                     00 - Positive
//'                     01 - Negative
//' Argument         : DOUBLE hemisphere (used for DEC Motor control)
//'                     00 - North
//'                     01 - South
//' Argument         : DOUBLE direction (used for DEC Motor control)
//'                     00 - Forward(+)
//'                     01 - Reverse(-)
//'
EQCONTRL_API DWORD __stdcall EQ_SendGuideRate(DWORD motor_id, DWORD trackrate, DWORD guiderate, DWORD guidedir,
	DWORD hemisphere, DWORD direction) {
	LOG("EQ_SendGuideRate() motor_id:" << motor_id << "; trackrate:" << trackrate << "; guiderate:" << guiderate <<
		"; guidedir:" << guidedir << "; hemisphere:" << hemisphere << "; direction:" << direction);
	DWORD ret = 0;
	En_MotorId nMotorId = (En_MotorId)motor_id;
	//Для DEC направление на самом деле передаётся в guidedir, а direction всегда = 0
	En_Direction nDirection = DIR_FORWARD;
	double fRate = m_RateCalculator.GetRate((En_TrackRate)trackrate);
	if (nMotorId == MI_RA) {
		if (guidedir) {
			fRate -= fRate * guiderate / 10;
		}
		else {
			fRate += fRate * guiderate / 10;
		}
	}
	else {
		fRate *= (double)guiderate / 10;
		nDirection = guidedir ? DIR_FORWARD : DIR_REVERSE; //Странно что не наоборот, возможно перепутали в EQMOD
	}
	uint16_t nFirst, nSecond;
	if (m_RateCalculator.CalculatePrescalersFromRate(TotalMicrostepCount(nMotorId), fRate, nFirst, nSecond)) {
		LOG("Rate: " << fRate << "; " << nFirst * nSecond <<
			" = " << nFirst << " * " << nSecond);
		EqResp Resp;
		ret = Convert(SendAndReadResp(
			EqStartTrackReq(nMotorId, (En_Hemisphere)hemisphere, nDirection, nFirst, nSecond),
			Resp));
	}
	else {
		ret = 999;
	}
	LOG("EQ_SendGuideRate() return:" << ret << ";");
	return ret;
}


//' Function name    : EQ_SetAutoguiderPortRate()
//' Description      : Sets RA/DEC Autoguideport rate
//' Return type      : DOUBLE - Stepper Counter Values
//'                     000 - Success
//'                     001 - Comport Not available
//'                     003 - COM Timeout Error
//'                     999 - Invalid Parameter
//' Argument         : motor_id
//'                       00 - RA Motor
//'                       01 - DEC Motor
//' Argument         : DOUBLE guideportrate
//'                       00 - 0.25x
//'                       01 - 0.50x
//'                       02 - 0.75x
//'                       03 - 1.00x
//'
EQCONTRL_API DWORD __stdcall EQ_SetAutoguiderPortRate(DWORD motor_id, DWORD portguiderate) {
	LOG("EQ_SetAutoguiderPortRate() motor_id:" << motor_id << "; portguiderate:" << portguiderate << ";");
	DWORD ret = 0;
	LOG("EQ_SetAutoguiderPortRate() return:" << ret << ";");
	return ret;
}

//' Function name    : EQ_GetTotal360microstep()
//' Description      : Get RA/DEC Motor Total 360 degree microstep counts
//' Return type      : Double - Stepper Counter Values
//'                     0 - 16777215  Valid Count Values
//'                     0x1000000 - Mount Not available
//'                     0x3000000 - Invalid Parameter
//' Argument         : DOUBLE motor_id
//'                     00 - RA Motor
//'                     01 - DEC Motor
//'
EQCONTRL_API DWORD __stdcall EQ_GetTotal360microstep(DWORD motor_id) {
	LOG("EQ_GetTotal360microstep() motor_id:" << motor_id << ";");
	DWORD ret = TotalMicrostepCount(motor_id);
	LOG("EQ_GetTotal360microstep() return:" << ret << ";");
	return ret;
}

//' Function name    : EQ_GetMountVersion()
//' Description      : Get Mount's Firmware version
//' Return type      : Double - Mount's Firmware Version
//'
//'                     0x1000000 - Mount Not available
//'
EQCONTRL_API DWORD __stdcall EQ_GetMountVersion() {
	LOG("EQ_GetMountVersion()");
	DWORD ret = 1;
	LOG("EQ_GetMountVersion() return:" << ret << ";");
	return ret;
}

//' Function name    : EQ_GetMountStatus()
//' Description      : Get Mount's Status
//' Return type      : Double - Mount Status
//'
//'                     000 - Not Connected
//'                     001 - Connected
//'
EQCONTRL_API DWORD __stdcall EQ_GetMountStatus() {
	LOG("EQ_GetMountStatus()");
	DWORD ret = pHandle != nullptr;
	LOG("EQ_GetMountStatus() return:" << ret << ";");
	return ret;
}

//' Function name    : EQ_DriverVersion()
//' Description      : Get Driver Version
//' Return type      : Double - Driver Version
//'
EQCONTRL_API DWORD __stdcall EQ_DriverVersion() {
	LOG("EQ_DriverVersion()");
	DWORD ret = 1;
	LOG("EQ_DriverVersion() return:" << ret << ";");
	return ret;
}

// Drift compenstation for standard mounts and for customised mounts apply tracking offsets
EQCONTRL_API DWORD __stdcall EQ_SetOffset(DWORD motor_id, DWORD doffset) {
	LOG("EQ_SetOffset() motor_id:" << motor_id << "; offset:" << doffset << ";");
	DWORD ret = 0;
	LOG("EQ_SetOffset() return:" << ret << ";");
	return ret;
}

//' Function name    : EQ_GP()
//' Description      : Get Mount Parameters
//' Return type      : Double - parameter value
EQCONTRL_API DWORD __stdcall EQ_GP(DWORD motor_id, DWORD p_id) {
	LOG("EQ_GP() motor_id:" << motor_id << "; p_id:" << p_id << ";");
	DWORD ret = 0;
	switch (p_id) {
	//case 10000: ret = 1281; break;
	//case 10001: ret = 9024000; break;
	//case 10002: ret = 64935; break;
	//case 10003: ret = 16; break;
	//case 10004: ret = motor_id == EQ::MI_DEC ? 9920 : 620; break;
	//case 10005: ret = motor_id == EQ::MI_DEC ? 9920 : 620; break;
	//case 10006: ret = 50133; break;
	//case 10007: ret = 0; break;
	//default: ret = 32; break;
	case PT_RA_MULTIPLIER: ret = 256; break;
	case PT_RA_TRACK_FACTOR: ret = EQMOD_TRACK_FACTOR; break;
	case PT_DEC_TRACK_FACTOR: ret = EQMOD_TRACK_FACTOR; break;
	case PT_WORM_MICROSTEP_COUNT: 
		ret = (uint32_t)m_Config.m_AxisConfigs[motor_id].m_nMicrostepCount *
			m_Config.m_AxisConfigs[motor_id].m_nStepsPerWormTurn /
			(m_Config.m_AxisConfigs[motor_id].m_nMicrostepsDivider / 2); 
		break;
	}
	LOG("EQ_GP() return:" << ret << ";");
	return ret;
}

//' Function name    : EQ_SP()
//' Description      : Set trackrate offset
//' Parameter        : value
//' Return type      : error code
EQCONTRL_API DWORD __stdcall EQ_SP(DWORD motor_id, DWORD p_id, DWORD value) {
	LOG("EQ_SP() motor_id:" << motor_id << "; p_id:" << p_id << "; value:" << value << ";");
	DWORD ret = 0;
	LOG("EQ_SP() return:" << ret << ";");
	return ret;
}

//' Function name    : EQ_SetMountType
//' Description      : Sets Mount protocol tpye
//' Return type      : 0
EQCONTRL_API DWORD __stdcall EQ_SetMountType(DWORD motor_type) {
	LOG("EQ_SetMountType() motor_type:" << motor_type << ";");
	DWORD ret = 0;
	LOG("EQ_SetMountType() motor_type:" << ret << ";");
	return ret;
}
