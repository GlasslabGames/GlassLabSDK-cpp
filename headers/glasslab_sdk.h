//
//  glasslab_sdk.h
//  Glasslab SDK
//
//  Authors:
//      Joseph Sutton
//      Ben Dapkiewicz
//
//  Copyright (c) 2014 Glasslab. All rights reserved.
//

/* ---------------------------- */
#ifndef GLASSLAB_SDK
#define GLASSLAB_SDK
/* ---------------------------- */

#if !WIN32
#include <unistd.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#if !WIN32
#include <pthread.h>
#endif
#include <string.h>
#include <string>
#include <sstream>
#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include <queue>
#include <cstdio>
#include <time.h>

#if WIN32
typedef __int8 int8_t;
typedef unsigned __int8 uint8_t;
typedef __int16 int16_t;
typedef unsigned __int16 uint16_t;
typedef __int32 int32_t;
typedef unsigned __int32 uint32_t;
typedef __int64 int64_t;
typedef unsigned __int64 uint64_t;
#else
#include <stdint.h>
#endif

#include <signal.h>
#include <evhttp.h>
#include <event2/event.h>
#include <event2/http.h>
#include <event2/buffer.h>

// https://jansson.readthedocs.org/en/2.5
#include <jansson.h>
// http://www.sqlite.org/
#include <sqlite3.h>
#include <CppSQLite3.h>

// The classes below are exported
#pragma GCC visibility push(default)

using namespace std;

class  GlasslabSDK;
class  GlasslabSDK_Core;

#include "glsdk_core.h"
#include "glsdk_const.h"
#include "glsdk_data_sync.h"


// used for client connection (get config), login, start/end session
//   - future feature: set/get client data (cloud saves)
// TODO: write simple c++ wrapper libevent
// TODO: is there a better way to handle the callbacks in from libevent?
//       so some client internal functions don't need to be exposed
//       for example setCookie, setSessionId
class GlasslabSDK {
    public:
        GlasslabSDK( const char* clientId, const char* deviceId, const char* dataPath = NULL, const char* uri = NULL );

        // Message stack functions
        nsGlasslabSDK::Const::Status getLastStatus();
        void popMessageStack();
        nsGlasslabSDK::Const::Message readTopMessageCode();
        const char * readTopMessageString();
    
        // API functions
        void connect( const char* gameId, const char* uri );
        void deviceUpdate();
        void authStatus();
        void registerStudent( const char* username, const char* password, const char* firstName, const char* lastInitial );
        void registerInstructor( const char* name, const char* email, const char* password, bool newsletter = true );
        void getUserInfo();
        void login( const char* username, const char* password, const char* type = NULL );
        void login( const char* username, const char* password );
        void enroll( const char* courseCode );
        void unenroll( const char* courseId );
        void getCourses();
        void logout();
        void startSession();
        void endSession();
        void saveGame( const char* gameData );
        void getSaveGame();
        void deleteSaveGame();
        void saveAchievement( const char* item, const char* group, const char* subGroup );
        void sendTelemEvents();
        void forceFlushTelemEvents();
        void cancelRequest( const char* key );
    
        // Telemetry event values
        // One for every data type, need for extern "C" support
        void addTelemEventValue( const char* key, const char* value );
        void addTelemEventValue( const char* key, int8_t value );
        void addTelemEventValue( const char* key, int16_t value );
        void addTelemEventValue( const char* key, int32_t value );
        void addTelemEventValue( const char* key, uint8_t value );
        void addTelemEventValue( const char* key, uint16_t value );
        void addTelemEventValue( const char* key, uint32_t value );
        void addTelemEventValue( const char* key, float value );
        void addTelemEventValue( const char* key, double value );
        void addTelemEventValue( const char* key, bool value );

        // Telemetry event helpers
        void clearTelemEventValues();
        void saveTelemEvent( const char* name );

        // These functions allow for control over the user info data structure
        void updatePlayerInfoKey( const char* key, const char* value );
        void updatePlayerInfoKey( const char* key, int8_t value );
        void updatePlayerInfoKey( const char* key, int16_t value );
        void updatePlayerInfoKey( const char* key, int32_t value );
        void updatePlayerInfoKey( const char* key, uint8_t value );
        void updatePlayerInfoKey( const char* key, uint16_t value );
        void updatePlayerInfoKey( const char* key, uint32_t value );
        void updatePlayerInfoKey( const char* key, float value );
        void updatePlayerInfoKey( const char* key, double value );
        void updatePlayerInfoKey( const char* key, bool value );
        void removePlayerInfoKey( const char* key );

        // Setters
        void setName( const char* name );
        void setVersion ( const char* version );
        void setGameLevel( const char* gameLevel );
        void setUserId( int userId );
        void setConfig( nsGlasslabSDK::glConfig config );
        void setTime( time_t time );
        void setPlayerHandle( const char* handle );
        void removePlayerHandle( const char* handle );
        void setCookie( const char* cookie );
        void setAutoSessionManagement( bool state );

        // Game timer functions
        void startGameTimer();
        void stopGameTimer();

        // Function to clear the database
        void resetDatabase();

        // Getters
        const char* getConnectUri();
        int getUserId();
        const char* getCookie();

        // Debug logging pop
        const char* popLogQueue();

    
    private:
        // Core SDK
        nsGlasslabSDK::Core* m_core;
};
#pragma GCC visibility pop

/* ---------------------------- */
#endif
/* ---------------------------- */