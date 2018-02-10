// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the EQCONTRL_EXPORTS
// symbol defined on the command line. This symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// EQCONTRL_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.
#ifdef EQCONTRL_EXPORTS
#define EQCONTRL_API __declspec(dllexport)
#else
#define EQCONTRL_API __declspec(dllimport)
#endif

#include "Protocol.h"

typedef unsigned long       DWORD;

DWORD Convert(EQ::En_Status nStatus);

EQCONTRL_API EQ::En_Status Connect();
EQCONTRL_API EQ::En_Status Disconnect();

EQCONTRL_API EQ::En_Status ReadConfig(EQ::Config& config);
EQCONTRL_API EQ::En_Status WriteConfig(const EQ::Config& config);

EQCONTRL_API EQ::En_Status GetEncoderValues(int& x, int& y);

EQCONTRL_API EQ::En_Status ClearEncoderCorrection();
EQCONTRL_API EQ::En_Status WriteEncoderCorrection(int16_t minX, int16_t maxX, int16_t minY, int16_t maxY, const int16_t(&data)[ENCODER_CORRECTION_DATA_SIZE]);
EQCONTRL_API EQ::En_Status ReadEncoderCorrection(int16_t& minX, int16_t& maxX, int16_t& minY, int16_t& maxY, int16_t(&data)[ENCODER_CORRECTION_DATA_SIZE]);

template <typename T>
EQ::En_Status SendReq(const T& Req);

template <typename T>
EQ::En_Status ReadResp(T& Resp);

template <typename T>
EQ::En_Status SendAndReadResp(const T& Req);

template <typename T, typename K>
EQ::En_Status SendAndReadResp(const T& Req, K& Resp);

// ************* Mount initialization ***************

// Intialize Mount / Driver
#pragma comment(linker, "/EXPORT:EQ_Init=?EQ_Init@@YGKPADKKK@Z")
EQCONTRL_API DWORD __stdcall EQ_Init(char *comportname, DWORD baud, DWORD timeout, DWORD retry);

// Initialize RA/DEC Motors
#pragma comment(linker, "/EXPORT:EQ_InitMotors=?EQ_InitMotors@@YGKKK@Z")
EQCONTRL_API DWORD __stdcall EQ_InitMotors(DWORD RA_val, DWORD DEC_val);

// Disconnect Driver
#pragma comment(linker, "/EXPORT:EQ_End=?EQ_End@@YGKXZ")
EQCONTRL_API DWORD __stdcall EQ_End();

// Motor Stop
#pragma comment(linker, "/EXPORT:EQ_MotorStop=?EQ_MotorStop@@YGKK@Z")
EQCONTRL_API DWORD __stdcall EQ_MotorStop(DWORD motor_id);

// Move motor based on microstep values
#pragma comment(linker, "/EXPORT:EQ_StartMoveMotor=?EQ_StartMoveMotor@@YGKKKKKK@Z")
EQCONTRL_API DWORD __stdcall EQ_StartMoveMotor(DWORD motor_id, DWORD hemisphere, DWORD direction, DWORD steps, DWORD stepslowdown);


// ************* Motor Parameters ******************

// Get Motor microstep position
#pragma comment(linker, "/EXPORT:EQ_GetMotorValues=?EQ_GetMotorValues@@YGKK@Z")
EQCONTRL_API DWORD __stdcall EQ_GetMotorValues(DWORD motor_id);

// Get Motor status
#pragma comment(linker, "/EXPORT:EQ_GetMotorStatus=?EQ_GetMotorStatus@@YGKK@Z")
EQCONTRL_API DWORD __stdcall EQ_GetMotorStatus(DWORD motor_id);

// Set Motor microstep position (counter value set only)
#pragma comment(linker, "/EXPORT:EQ_SetMotorValues=?EQ_SetMotorValues@@YGKKK@Z")
EQCONTRL_API DWORD __stdcall EQ_SetMotorValues(DWORD motor_id, DWORD motor_val);


// ************* Slewing and GOTO *****************

// Slew Motor based on specified slew speed 
#pragma comment(linker, "/EXPORT:EQ_Slew=?EQ_Slew@@YGKKKKK@Z")
EQCONTRL_API DWORD __stdcall EQ_Slew(DWORD motor_id, DWORD hemisphere, DWORD direction, DWORD rate);

// ************* Tracking ************************

// Move motor at specified tracking speed
#pragma comment(linker, "/EXPORT:EQ_StartRATrack=?EQ_StartRATrack@@YGKKKK@Z")
EQCONTRL_API DWORD __stdcall EQ_StartRATrack(DWORD trackrate, DWORD hemisphere, DWORD direction);

// Customized speed (for orbital data)
#pragma comment(linker, "/EXPORT:EQ_SendCustomTrackRate=?EQ_SendCustomTrackRate@@YGKKKKKKK@Z")
EQCONTRL_API DWORD __stdcall EQ_SendCustomTrackRate(DWORD motor_id, DWORD trackrate, DWORD trackoffset, DWORD trackdir, 
	DWORD hemisphere, DWORD direction);

// Custom ALT/AZ Speed
#pragma comment(linker, "/EXPORT:EQ_SetCustomTrackRate=?EQ_SetCustomTrackRate@@YGKKKKKKK@Z")
EQCONTRL_API DWORD __stdcall EQ_SetCustomTrackRate(DWORD motor_id, DWORD trackmode, DWORD trackoffset, DWORD trackbase, 
	DWORD hemisphere, DWORD direction);


// ************* Guiding / PEC ******************

// Initiate speed change (guiding)
#pragma comment(linker, "/EXPORT:EQ_SendGuideRate=?EQ_SendGuideRate@@YGKKKKKKK@Z")
EQCONTRL_API DWORD __stdcall EQ_SendGuideRate(DWORD motor_id, DWORD trackrate, DWORD guiderate, DWORD guidedir, 
	DWORD hemisphere, DWORD direction);


// Autoguider port speed
#pragma comment(linker, "/EXPORT:EQ_SetAutoguiderPortRate=?EQ_SetAutoguiderPortRate@@YGKKK@Z")
EQCONTRL_API DWORD __stdcall EQ_SetAutoguiderPortRate(DWORD motor_id, DWORD portguiderate);


// ************* Mount and Driver Parameters *************

// Get mount stepping info
#pragma comment(linker, "/EXPORT:EQ_GetTotal360microstep=?EQ_GetTotal360microstep@@YGKK@Z")
EQCONTRL_API DWORD __stdcall EQ_GetTotal360microstep(DWORD motor_id);

// Mount version
#pragma comment(linker, "/EXPORT:EQ_GetMountVersion=?EQ_GetMountVersion@@YGKXZ")
EQCONTRL_API DWORD __stdcall EQ_GetMountVersion();

// Mount/Driver state
#pragma comment(linker, "/EXPORT:EQ_GetMountStatus=?EQ_GetMountStatus@@YGKXZ")
EQCONTRL_API DWORD __stdcall EQ_GetMountStatus();

// Driver Version
#pragma comment(linker, "/EXPORT:EQ_DriverVersion=?EQ_DriverVersion@@YGKXZ")
EQCONTRL_API DWORD __stdcall EQ_DriverVersion();

// Drift compenstation for standard mouns and for customised mounts apply tracking offsets
#pragma comment(linker, "/EXPORT:EQ_SetOffset=?EQ_SetOffset@@YGKKK@Z")
EQCONTRL_API DWORD __stdcall EQ_SetOffset(DWORD motor_id, DWORD doffset);

//' Function name    : EQ_GP()
//' Description      : Get Mount Parameters
//' Return type      : Double - parameter value
#pragma comment(linker, "/EXPORT:EQ_GP=?EQ_GP@@YGKKK@Z")
EQCONTRL_API DWORD __stdcall EQ_GP(DWORD motor_id, DWORD p_id);

//' Function name    : EQ_SP()
//' Description      : Set trackrate offset
//' Parameter        : value
//' Return type      : error code
#pragma comment(linker, "/EXPORT:EQ_SP=?EQ_SP@@YGKKKK@Z")
EQCONTRL_API DWORD __stdcall EQ_SP(DWORD motor_id, DWORD p_id, DWORD value);

//' Function name    : EQ_SetMountType
//' Description      : Sets Mount protocol tpye
//' Return type      : 0
#pragma comment(linker, "/EXPORT:EQ_SetMountType=?EQ_SetMountType@@YGKK@Z")
EQCONTRL_API DWORD __stdcall EQ_SetMountType(DWORD motor_type);