/**************************************************************************

Filename    :   PlotIdDef.h
Content     :   
Created     :   Feb 2015
Authors     :   Ankur Mohan

Copyright   :   Copyright 2015 Ankur Mohan, All Rights reserved.

Use of this software is subject to the terms of the license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef PLOTIDDEF
#define PLOTIDDEF

#define NUM_PLOTS 3 // Yaw/Pitch/Roll
#define NUM_CURVES 3

enum PlotId
{
	yaw = 0,
	pitch,
	roll
};

enum CurveId
{
	MPU= 0,
	PID = 1,
	MPR = 2,
	SensorFusion = 3
};

#endif
