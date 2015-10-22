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

#if WIN32
#if _DLL
#define APIIMPORT __declspec(dllexport) 
#else
#define APIIMPORT __declspec(dllimport) 
#endif
#else
#define APIIMPORT 
#endif

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

#include <stdint.h>

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
#if !WIN32
#pragma GCC visibility push(default)
#endif

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
		/* TODO: figure out how to resolve this behavior... */
		/* APIIMPORT required when compiling the DLL and running in DLL example */
		/* APIIMPORT should be removed when running the lib example */
		APIIMPORT GlasslabSDK( const char* clientId, const char* deviceId, const char* dataPath = NULL, const char* uri = NULL );
        GlasslabSDK(nsGlasslabSDK::Core* core);
    
        // Some platforms require this to be called, usually very early in program execution.
        // For Mac and iOS, needs ot be called in the NSApplicationDelegate's
        // applicationWillFinishLaunching: method.
#if TARGET_OS_IPHONE || TARGET_IPHONE_SIMULATOR
        static void APIIMPORT initSDK(void *launchOptions);
#else
        static void APIIMPORT initSDK();
#endif
    
        // Message stack functions
        nsGlasslabSDK::Const::Status APIIMPORT getLastStatus();
        void APIIMPORT popMessageStack();
        nsGlasslabSDK::Const::Message APIIMPORT readTopMessageCode();
        const char APIIMPORT *readTopMessageString();
    
        // API functions
        void APIIMPORT connect( const char* gameId, const char* uri );
        void APIIMPORT deviceUpdate();
        void APIIMPORT authStatus();
        void APIIMPORT registerStudent( const char* username, const char* password, const char* firstName, const char* lastInitial );
        void APIIMPORT registerInstructor( const char* name, const char* email, const char* password, bool newsletter = true );
        void APIIMPORT getUserInfo();
        void APIIMPORT getPlayerInfo();
        void APIIMPORT login( const char* username, const char* password, const char* type = NULL );
        void APIIMPORT login( const char* username, const char* password );
        void APIIMPORT enroll( const char* courseCode );
        void APIIMPORT unenroll( const char* courseId );
        void APIIMPORT getCourses();
        void APIIMPORT logout();
        void APIIMPORT startSession();
        void APIIMPORT endSession();
        void APIIMPORT saveGame( const char* gameData );
        void APIIMPORT getSaveGame();
        void APIIMPORT deleteSaveGame();
        void APIIMPORT saveAchievement( const char* item, const char* group, const char* subGroup );
        void APIIMPORT createMatch( int opponentId );
        void APIIMPORT updateMatch( int matchId, const char* data, int nextPlayerTurn );
        void APIIMPORT sendTelemEvents();
        void APIIMPORT forceFlushTelemEvents();
        void APIIMPORT cancelRequest( const char* key );
    
        // Telemetry event values
        // One for every data type, need for extern "C" support
        void APIIMPORT addTelemEventValue( const char* key, const char* value );
        void APIIMPORT addTelemEventValue( const char* key, int8_t value );
        void APIIMPORT addTelemEventValue( const char* key, int16_t value );
        void APIIMPORT addTelemEventValue( const char* key, int32_t value );
        void APIIMPORT addTelemEventValue( const char* key, uint8_t value );
        void APIIMPORT addTelemEventValue( const char* key, uint16_t value );
        void APIIMPORT addTelemEventValue( const char* key, uint32_t value );
        void APIIMPORT addTelemEventValue( const char* key, float value );
        void APIIMPORT addTelemEventValue( const char* key, double value );
        void APIIMPORT addTelemEventValue( const char* key, bool value );

        // Telemetry event helpers
        void APIIMPORT clearTelemEventValues();
        void APIIMPORT saveTelemEvent( const char* name );

        // These functions allow for control over the user info data structure
        void APIIMPORT updatePlayerInfoKey( const char* key, const char* value );
        void APIIMPORT updatePlayerInfoKey( const char* key, int8_t value );
        void APIIMPORT updatePlayerInfoKey( const char* key, int16_t value );
        void APIIMPORT updatePlayerInfoKey( const char* key, int32_t value );
        void APIIMPORT updatePlayerInfoKey( const char* key, uint8_t value );
        void APIIMPORT updatePlayerInfoKey( const char* key, uint16_t value );
        void APIIMPORT updatePlayerInfoKey( const char* key, uint32_t value );
        void APIIMPORT updatePlayerInfoKey( const char* key, float value );
        void APIIMPORT updatePlayerInfoKey( const char* key, double value );
        void APIIMPORT updatePlayerInfoKey( const char* key, bool value );
        void APIIMPORT removePlayerInfoKey( const char* key );

        // Setters
        void APIIMPORT setGameSecret( const char* gameSecret );
        void APIIMPORT setName( const char* name );
        void APIIMPORT setVersion ( const char* version );
        void APIIMPORT setGameLevel( const char* gameLevel );
        void APIIMPORT setUserId( int userId );
        void APIIMPORT setConfig( nsGlasslabSDK::glConfig config );
        void APIIMPORT setTime( time_t time );
        void APIIMPORT setPlayerHandle( const char* handle );
        void APIIMPORT removePlayerHandle( const char* handle );
        void APIIMPORT setCookie( const char* cookie );
        void APIIMPORT setAutoSessionManagement( bool state );

        // Game timer functions
        void APIIMPORT startGameTimer();
        void APIIMPORT stopGameTimer();

        // Function to clear the database
        void APIIMPORT resetDatabase();

        // Getters
        const char APIIMPORT *getConnectUri();
        int APIIMPORT getUserId();
        const char APIIMPORT *getCookie();
    const char APIIMPORT *getMatchForId( int matchId );

        // Debug logging pop
        const char APIIMPORT *popLogQueue();
   
    private:
        // Core SDK
        nsGlasslabSDK::Core* m_core;
};
#if !WIN32
#pragma GCC visibility pop
#endif

/* ---------------------------- */
#endif
/* ---------------------------- */