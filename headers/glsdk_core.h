//
//  glsdk_core.h
//  Glasslab SDK
//
//  Authors:
//      Joseph Sutton
//      Ben Dapkiewicz
//
//  Copyright (c) 2014 Glasslab. All rights reserved.
//

/* ---------------------------- */
#ifndef GLASSLABSDK_CORE
#define GLASSLABSDK_CORE
/* ---------------------------- */


// The classes below are not exported
#pragma GCC visibility push(hidden)

using namespace std;

#include "glsdk_const.h"
#include "glsdk_data_sync.h"


namespace nsGlasslabSDK {

    class  Core;


    typedef struct _p_glSDKInfo p_glSDKInfo;

    typedef void(*CoreCallback_Func)(p_glSDKInfo);

    typedef struct _coreCallbackStructure {
        CoreCallback_Func coreCB;
        bool cancel;
    } coreCallbackStructure;

    typedef struct _p_glSDKInfo {
        GlasslabSDK*        sdk;
        Core*               core;
        string              data;
        void*               userData;
        bool                success;
    } p_glSDKInfo;

    typedef struct _glHttpRequest {
        GlasslabSDK*                sdk;
        Core*                       core;
        struct event_base*          base;
        struct evhttp_connection*   conn;
        struct evhttp_request*      req;
        string                      coreCBKey;
        int                         msgQRowId;
    } p_glHttpRequest;


    // used for client connection (get config), login, start/end session
    //   - future feature: set/get client data (cloud saves)
    // TODO: write simple c++ wrapper libevent
    // TODO: check for de-allocate pointers!!
    class Core {
        public:
            Core( GlasslabSDK* client, const char* gameId, const char* deviceId = NULL, const char* dataPath = NULL, const char* uri = NULL );
            ~Core();

            // Message stack functions
            void pushMessageStack( Const::Message msg, const char* data = "{}" );
            void popMessageStack();
            Const::Message readTopMessageCode();
            const char *readTopMessageString();
        
        
            // Primary GLGS API functions
            int connect( const char* gameId, const char* uri = NULL );
            void getConfig( const char* uri );
            int setUserInfo( const char* json );
            void deviceUpdate();
            void authStatus();
            void registerStudent( const char* username, const char* password, const char* firstName, const char* lastInitial );
            void registerInstructor( const char* name, const char* email, const char* password, bool newsletter = true);
            void getPlayerInfo();
            void getUserInfo();
            void login( const char* username, const char* password, const char* type = NULL );
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
            void savePlayerInfo();
            void sendTotalTimePlayed();
            void sendTelemEvents();
            void forceFlushTelemEvents();
            void attemptMessageDispatch();
            void mf_httpGetRequest( string path, string requestType, string coreCB, string postdata = "", const char* contentType = NULL, int rowId = -1 );

            // Allow the user to cancel a request from being sent to the server, or ignore the response
            void cancelRequest( const char* requestKey );

            // Callback map functions
            CoreCallback_Func getCoreCallback( string key );
            bool getCoreCallbackCancelState( string key );
            void setCoreCallbackCancelState( string key, bool state );

            // SQLite message queue functions
            void mf_addMessageToDataQueue( string path, string requestType, string coreCB, string postdata = "", const char* contentType = NULL );
            void mf_updateMessageStatusInDataQueue( int rowId, string status );
            // SQLite session table functions
            void mf_updateTotalTimePlayedInSessionTable( float totalTimePlayed );
        
            // Telemetry event values
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
            void setDefaultPlayerInfoKeys();
            void resetPlayerInfo();

            // Game timer functions
            void startGameTimer();
            void stopGameTimer();

            // Session timer functions
            void startSessionTimer();
            void stopSessionTimer();

            // Function to clear the database
            void resetDatabase();

            // Setters
            void setConnectUri( const char* uri );
            void setName( const char* name );
            void setVersion( const char* version );
            void setGameLevel( const char* gameLevel );
            void setUserId( int userId );
            void setConfig( nsGlasslabSDK::glConfig config );
            void setTime( time_t time );
            void setPlayerHandle( const char* handle );
            void removePlayerHandle( const char* handle );
            void setCookie( const char* cookie );
            void setSessionId( const char* sessionId );
            void setAutoSessionManagement( bool state );
        
            // Getters
            const char* getConnectUri();
            int getUserId();
            const char* getId();
            const char* getCookie();
            const char* getSessionId();
            Const::Status getLastStatus();
            float getTotalTimePlayed();

            // Functions for the connected state
            void setConnectedState( bool state );
            bool getConnectedState();

            // Config object
            glConfig config;

            // User info
            glUserInfo userInfo;

            // Helper function for displaying warnings and errors
            void displayWarning( string location, string warning );
            void displayError( string location, string error );
            void logMessage( const char* message, const char* data = NULL );
            bool mf_checkForJSONErrors( json_t* root );

            // Debug logging pop
            const char* popLogQueue();


        private:
            // SDK object
            GlasslabSDK* m_sdk;

            // SQLite Message Queue
            DataSync* m_dataSync;

            // State indicates if the user is connected
            bool m_connected;

            // State indicates if the SDK will automatically handle sessions
            bool m_autoSessionManagement;

            // General members
            string m_connectUri;
            string m_cookie;
            string m_gameId;
            string m_deviceId;
            string m_baseDeviceId;
            string m_sessionId;
            int    m_userId;
            string m_gameLevel;
            string m_playerHandle;
            string m_clientName;
            string m_clientVersion;
            time_t m_currentTime;
        
            // JSON members
            json_error_t m_jsonError;
            json_t* m_userInfo;
            json_t* m_telemEvents;
            json_t* m_telemEventValues;
            json_t* m_playerInfo;

            // Timer for delaying telemetry
            time_t m_telemetryLastTime;

            // Local variable for event order
            int m_gameSessionEventOrder;

            // Game timer variables used for total time played
            time_t m_gameTimerLast;
            bool m_gameTimerActive;

            // Session timer variables used for auto session management
            time_t m_sessionTimerLast;
            bool m_sessionTimerActive;
        
            // Status members
            Const::Status m_lastStatus;
            std::queue<Const::Response*> m_msgQueue;

            // Debug logging queue
            std::queue<std::string> m_logQueue;

            // Helper function for callback setup
            void mf_setupCallbacks();
            // Callback function maps
            map<string, coreCallbackStructure> m_coreCallbackMap;
    };
};
#pragma GCC visibility pop


/* ---------------------------- */
#endif /* defined(GLASSLABSDK_CORE) */
/* ---------------------------- */