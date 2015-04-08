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
//  glsdk_data_sync.h
//  GlasslabSDK
//
//  Authors:
//      Joseph Sutton
//      Ben Dapkiewicz
//
//  Copyright (c) 2014 Glasslab. All rights reserved.
//

/* ---------------------------- */
#ifndef GLASSLABSDK_DATASYNC
#define GLASSLABSDK_DATASYNC
/* ---------------------------- */

#define CONFIG_TABLE_NAME "CONFIG"
#define MSG_QUEUE_TABLE_NAME "MSG_QUEUE"
#define SESSION_TABLE_NAME "SESSION"

namespace nsGlasslabSDK {

    class Core;
    
    class DataSync {
    public:
        DataSync( Core* core, const char* dbPath = NULL );
        ~DataSync();
        
        // Message Queue (MSG_QUEUE) table operations
        void addToMsgQ( string deviceId, string path, string requestType, string coreCB, string postdata, const char* contentType );
        void removeFromMsgQ( int rowId );
        void updateMessageStatus( int rowId, string status );
        int getMessageTableSize();

        // Session (SESSION) table operations
        void updateSessionTableWithCookie( string deviceId, string cookie );
        void updateSessionTableWithGameSessionId( string deviceId, string gameSessionId );
        void updateSessionTableWithPlayerHandle( string deviceIdWithHandle );
        void removeSessionWithDeviceId( string deviceId );
        const char* getCookieFromDeviceId( string deviceId );
        void updatePlayerInfoFromDeviceId( string deviceId, float totalTimePlayed, int gameSessionEventOrder );
        float getTotalTimePlayedFromDeviceId( string deviceId );
        void updateGameSessionEventOrderWithDeviceId( string deviceId, int gameSessionEventOrder );
        int getGameSessionEventOrderFromDeviceId( string deviceId );

        // Function flushes MSG_QUEUE, converting all stored API events into HTTP requests on Core
        void doFlushMsgQ();
#ifdef MULTITHREADED
        bool queueFlushRequested = false;
#endif
        void flushMsgQ();

        // Function forces a database reset
        void resetDatabase();

        
    private:
        // Initialization and validation
        void initDB();
        void validateSDKVersion();

        // Table create/drop/migrate functions
        void createTables();
        void dropTables();
        void migrateTables();
        void migrateTable( string table, string newSchema );
        // Debug display
        void displayTable( string table );

        // Helper function for creating a new SESSION entry
        string createNewSessionEntry( string deviceId, string cookie, string gameSessionId );

        // The SQLite database to store all API events and session information
        CppSQLite3DB m_db;

        // Core reference
        Core* m_core;
        
        // Names
        string m_dbName;
        string m_configTableName;
        string m_sessionTableName;
        string m_hmqTableName;

        // Event counts
        int m_messageTableSize;
    };
};

/* ---------------------------- */
#endif /* defined(GLASSLABSDK_DATASYNC) */
/* ---------------------------- */