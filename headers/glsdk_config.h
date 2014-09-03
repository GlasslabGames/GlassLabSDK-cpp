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