/*

Copyright (c) 2014, GlassLab, Inc.
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
  list of conditions and the following disclaimer. 
2. Redistributions in binary form must reproduce the above copyright notice,
  this list of conditions and the following disclaimer in the documentation
  and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

The views and conclusions contained in the software and documentation are those
of the authors and should not be interpreted as representing official policies, 
either expressed or implied, of the FreeBSD Project.

*/


//
//  glsdk_config.h
//  GlasslabSDK
//
//  Authors:
//      Ben Dapkiewicz
//
//  Copyright (c) 2014 Glasslab. All rights reserved.
//


#define SDK_VERSION	"1.2"

#define DB_MESSAGE_CAP 32000

#define SESSION_TIMEOUT 60 * 10

#define THROTTLE_PRIORITY_DEFAULT 10
#define THROTTLE_INTERVAL_DEFAULT 30
#define THROTTLE_MIN_SIZE_DEFAULT 5
#define THROTTLE_MAX_SIZE_DEFAULT 50

#define API_CONNECT					"/sdk/connect"
#define API_GET_CONFIG        		"/api/v2/data/config/:gameId"
#define API_POST_REGISTER			"/api/v2/auth/user/register"
#define API_GET_USER_PROFILE		"/api/v2/auth/user/profile"
#define API_GET_AUTH_STATUS			"/api/v2/auth/login/status"
#define API_POST_LOGIN        		"/api/v2/auth/login/glasslab"
#define API_POST_LOGOUT        		"/api/v2/auth/logout"
#define API_POST_ENROLL       		"/api/v2/lms/course/enroll"
#define API_POST_UNENROLL     		"/api/v2/lms/course/unenroll"
#define API_GET_COURSES       		"/api/v2/lms/courses"
#define API_POST_SESSION_START 		"/api/v2/data/session/start"
#define API_POST_SESSION_END   		"/api/v2/data/session/end"
#define API_POST_DEVICE_UPDATE		"/api/v2/data/game/device"
#define API_POST_SAVEGAME      		"/api/v2/data/game/:gameId"
#define API_GET_SAVEGAME      		"/api/v2/data/game/:gameId"
#define API_DELETE_SAVEGAME      	"/api/v2/data/game/:gameId"
#define API_POST_PLAYERINFO			"/api/v2/data/user/pref/:gameId"
#define API_GET_PLAYERINFO			"/api/v2/data/game/:gameId/playInfo"
#define API_POST_TOTAL_TIME_PLAYED	"/api/v2/data/game/:gameId/totalTimePlayed"
#define API_POST_ACHIEVEMENT		"/api/v2/data/game/:gameId/achievement"
#define API_POST_EVENTS       		"/api/v2/data/events"