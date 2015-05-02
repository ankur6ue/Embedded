/**************************************************************************

Filename    :   Logger.cpp
Content     :
Created     :   Feb 2015
Authors     :   Ankur Mohan

Copyright   :   Copyright 2015 Ankur Mohan, All Rights reserved.

Use of this software is subject to the terms of the license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "Logger.h"
#include "SoftwareSerial.h"
#include "SerialDef.h"
#include "ErrorsDef.h"
#include "quadcopter.h"

extern int	MPUInterruptCounter;
extern ExceptionMgr	cExceptionMgr;

bool MyLog::AddtoLog(const char* str)
{
	if (bError)
		return false;
	if (!str)
		return false;

	int len = strlen(str);
	if (LogLength + len + 1 < LOGBUFSIZE)
	{
		sprintf(LogBuf + LogLength, "%s", str);
		LogLength += len;
		LogBuf[LogLength] = '\t';
		LogLength++;
		return true;
	} else
	{
		bError = true;
		// sprintf(LogBuf, "ERROR=%d", BUFFER_OVERFLOW);
		LogLength = strlen(LogBuf);
		return false;
	}
}

int MyLog::GetLogLength()
{
	return LogLength;
}

void MyLog::Reset()
{
	LogLength = 0;
	memset(LogBuf, 0, LOGBUFSIZE);
	bError = false;
}

bool MyLog::AddtoLog(const char* prefix, float numbers[], int numElem)
{
	if (bError)
		return false;

	if (AddtoLog(prefix))
	{
		int i = 0;
		char tempBuf[20];
		do
		{
			if (i >= numElem)
				break;
			dtostrf(numbers[i], 1, 2, tempBuf);
			i++;
		} while (AddtoLog(tempBuf));
	}
	return true;
}

bool MyLog::AddtoLog(const char* prefix, int numbers[], int numElem)
{
	if (bError)
		return false;

	if (AddtoLog(prefix))
	{
		int i = 0;
		char tempBuf[20];
		do
		{
			if (i >= numElem)
				break;
			itoa(numbers[i], tempBuf, 10);
			i++;
		} while (AddtoLog(tempBuf));
	}
	return true;
}

void PrintHelper::Serialize(const char* prefix, const char* name, float val)
{
	char tmp1[50];
	char tmp2[10];
	dtostrf(val, 3, 3, tmp2);
	sprintf(tmp1, "%s %s %s", prefix, name, tmp2);
	SERIAL.print(tmp1);
}

void PrintHelper::Serialize(const char* prefix, const char* name, int val)
{
	char tmp1[50];
	sprintf(tmp1, "%s %s %d", prefix, name, val);
	SERIAL.print(tmp1);
}

void PrintHelper::Print(char* str)
{
	SERIAL.print(str);
//	SERIAL.flush();
}

void PrintHelper::AttachSentinal()
{
	SERIAL.print("z");
}

unsigned long QuadStateLogger::Run()
{
	unsigned long before = millis();
	SendQuadState();
	unsigned long now = millis();
	return now - before;
}

void QuadStateLogger::SendQuadState()
{
	PrintHelper::Serialize("QS", "PKp", QuadState.PitchParams.Kp);
	PrintHelper::Serialize("QS", "PKi", QuadState.PitchParams.Ki);
	PrintHelper::Serialize("QS", "PKd", QuadState.PitchParams.Kd);

	PrintHelper::Serialize("QS", "Yaw_Kp", QuadState.YawParams.Kp);
	PrintHelper::Serialize("QS", "Yaw_Ki", QuadState.YawParams.Ki);
	PrintHelper::Serialize("QS", "Yaw_Kd", QuadState.YawParams.Kd);

	PrintHelper::Serialize("QS", "A2R_PKp", QuadState.A2R_PKp);
	PrintHelper::Serialize("QS", "A2R_RKp", QuadState.A2R_RKp);
	PrintHelper::Serialize("QS", "A2R_YKp", QuadState.A2R_YKp);

	PrintHelper::Serialize("QS", "MotorToggle", QuadState.bMotorToggle);
	PrintHelper::Serialize("QS", "Speed", QuadState.PID_Alt + QuadState.Speed);
	PrintHelper::Serialize("QS", "PIDType", QuadState.ePIDType);
	PrintHelper::AttachSentinal();
}

unsigned long PIDStateLogger::Run()
{
	unsigned long before = millis();
	Log.Reset();
	float angles[4] =
	{ (float)before, QuadState.Yaw, QuadState.Pitch, QuadState.Roll };
	Log.AddtoLog("ypr", angles, 4);
	SERIAL.print(Log.LogBuf);
	PrintHelper::AttachSentinal();
	delay(1);
	Log.Reset();
	float pidParams[4] =
	{ (float)before, QuadState.PID_Yaw, QuadState.PID_Pitch, QuadState.PID_Roll };
	Log.AddtoLog("PID", pidParams, 4);
	SERIAL.print(Log.LogBuf);
	// Avoid Flush. It appears to be quite expensive
//	SERIAL.flush();
	PrintHelper::AttachSentinal();
	delay(1);
	float motionParams[7] = {(float)before, /*QuadState.YawOmega, QuadState.PitchOmega,
	QuadState.RollOmega, */QuadState.CurrentAltitude, QuadState.PID_Alt, QuadState.Vz };
	Log.Reset();
	Log.AddtoLog("mpr", motionParams, 4);
	SERIAL.print(Log.LogBuf);
//	SERIAL.flush();
	PrintHelper::AttachSentinal();

	// SERIAL.print("NumInterrupts = "); SERIAL.println(MPUInterruptCounter);
	MPUInterruptCounter = 0;
	Log.Reset();
	unsigned long now = millis();
	return now - before;
}

unsigned long ExceptionLogger::Run()
{
	unsigned long before = millis();
	if (cExceptionMgr.IsException())
	{
		switch (cExceptionMgr.GetException())
		{
		case NO_BEACON_SIGNAL:
			SERIAL.println("Exception: NoBeaconReceived");
			PrintHelper::AttachSentinal();
			break;
		case EXCESSIVE_PID_OUTPUT:
			SERIAL.println("Exception: ExcessivePIDOutput");
			PrintHelper::AttachSentinal();
			break;
		case BAD_MPU_DATA:
			SERIAL.println("Exception: BadMPUData");
			PrintHelper::AttachSentinal();
			break;
		}
	}
	unsigned long now = millis();
	return now - before;
}

