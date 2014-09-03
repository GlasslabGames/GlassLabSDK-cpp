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