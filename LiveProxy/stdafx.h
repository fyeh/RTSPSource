//=============================================================================
// Copyright (c) 2013 Horth Systems. All rights reserved.
//
// This file contains trade secrets of Horth Systems.  No part may be reproduced or
// transmitted in any form by any means or for any purpose without the express
// written permission of Horth Systems.
//
// $File:$
// $Revision:$
// $Date:$
// $Author:$
//=============================================================================



#pragma once
#include <shlwapi.h>
#include <sstream>
#include <cctype>       // std::toupper
#include <atlbase.h>
#include <atlconv.h>
#include <fstream>
#include <stdarg.h>
#include <cstdlib>
#include <time.h>
#include <stdio.h>
#include <initguid.h>
#include <regex>
#include <string.h>



//Link in our dependent library
//live555 libraries
#pragma comment (lib,"liveMedia.lib")
#pragma comment (lib,"BasicUsageEnvironment.lib")
#pragma comment (lib,"groupsock.lib")
#pragma comment (lib,"UsageEnvironment.lib")

//ffmpeg libraries
#pragma comment (lib,"avcodec.lib")
#pragma comment (lib,"avutil.lib")
#pragma comment (lib,"swscale.lib")

//windows libs
#pragma comment (lib,"wsock32.lib")
//#pragma comment (lib,"webservices.lib")
#pragma comment (lib,"winmm.lib")
#pragma comment (lib,"Ws2_32.lib")
