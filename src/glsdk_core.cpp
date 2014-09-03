//
//  glsdk_core.cpp
//  Glasslab SDK
//
//  Authors:
//      Joseph Sutton
//      Ben Dapkiewicz
//
//  Copyright (c) 2014 Glasslab. All rights reserved.
//

#include "glasslab_sdk.h"
#include "glsdk_config.h"


namespace nsGlasslabSDK {

    /**
     * Core constructor to setup the SDK and perform an initial connection to the server.
     */
    Core::Core( GlasslabSDK* sdk, const char* gameId, const char* deviceId, const char* dataPath, const char* uri ) {
        logMessage( "Initializing the SDK" );

        // set device ID only if not null and contains a string of length 0
        if( ( deviceId != NULL ) && strlen( deviceId ) > 0 ) {
           m_deviceId = deviceId;
           m_baseDeviceId = deviceId;
        }
        else {
            displayError( "Core::Core()", "The deviceId was not valid. Core object will not be created." );
            delete this;
            return;
        }

        // Set the SDK
        m_sdk = sdk;

        // Set the default information
        m_connectUri    = "http://127.0.0.1:8000";
        m_clientName    = "";
        m_clientVersion = "";
        m_gameId        = "";
        m_sessionId     = "";
        m_gameLevel     = "";
        m_userId        = 0;
        m_lastStatus    = Const::Status_Ok;
        m_userInfo      = NULL;
        m_playerInfo    = json_object();
        m_autoSessionManagement = false;
        
        // Set JSON telemetry objects
        m_telemEvents       = json_array();
        m_telemEventValues  = json_object();
        // Clear telemetry
        clearTelemEventValues();

        // Store core function callbacks in the hash
        mf_setupCallbacks();
        if(dataPath) {
            // Create the SQLite data sync object
            //printf( "Data path set: %s\n", dataPath );
            logMessage( "Data path set: ", dataPath );
        }
        m_dataSync = new DataSync( this, dataPath );

        // Reset the player info
        resetPlayerInfo();

        // Set default throttle variables
        config.eventsDetailLevel = THROTTLE_PRIORITY_DEFAULT;
        config.eventsPeriodSecs = THROTTLE_INTERVAL_DEFAULT;
        config.eventsMinSize = THROTTLE_MIN_SIZE_DEFAULT;
        config.eventsMaxSize = THROTTLE_MAX_SIZE_DEFAULT;

        // Set default user info variables
        userInfo.username = "";
        userInfo.firstName = "";
        userInfo.lastInitial = "";
        userInfo.email = "";

        // Set the last time since telemetry was fired
        m_telemetryLastTime = time( NULL );
        // Get the game session event order to update
        m_gameSessionEventOrder = m_dataSync->getGameSessionEventOrderFromDeviceId( m_deviceId );

        // Stop the timers, initially
        stopGameTimer();
        stopSessionTimer();


        // Set the initial player handle and cookie
        setPlayerHandle( "" );
        setCookie( "" );


        // Attempt a connection now that the SDK is created
        m_connected = false;
        connect( gameId, uri );
    }

    /**
     * Core deconstructor.
     */
    Core::~Core() {
    }


    //--------------------------------------
    //--------------------------------------
    //--------------------------------------
    /**
     * Push to the Message Stack.
     */
    void Core::pushMessageStack( Const::Message msg, const char* data ) {
        // Only allow valid messages to be added to the queue
        if( msg != Const::Message_None ) {
            Const::Response* response = new Const::Response();
            response->m_message = msg;
            response->m_data    = data;
            m_msgQueue.push( response );
        }
    }

    /**
     * Pop from the Message Stack.
     */
    void Core::popMessageStack() {
        Const::Response* t = NULL;
        if(!m_msgQueue.empty()){
            t = m_msgQueue.front();
        }
        
        if(t != NULL) {
            m_msgQueue.pop();
            
            // free top item from Q
            delete t;
        }
    }

    Const::Message Core::readTopMessageCode() {
        Const::Response* t = NULL;
        if(!m_msgQueue.empty()){
            t = m_msgQueue.front();
        }
        
        if(t == NULL) {
            return Const::Message_None;
        } else {
           return t->m_message;
        }
    }
    
    const char * Core::readTopMessageString() {
        Const::Response* t = NULL;
        if(!m_msgQueue.empty()){
            t = m_msgQueue.front();
        }
        
        if(t != NULL) {
            //printf( "readTopMessageString: %s\n", t->m_data.c_str() );
            return t->m_data.c_str();
        } else {
            return NULL;
        }
    }


    //--------------------------------------
    //--------------------------------------
    //--------------------------------------
    /**
     * Callback function occurs when API_CONNECT is successful.
     *
     * Sets the server we should be pointing at.
     */
    void getConnect_Done( p_glSDKInfo sdkInfo ) {
        const char* uri = sdkInfo.data.c_str();
        sdkInfo.core->logMessage( "---------------------------" );
        sdkInfo.core->logMessage( "getConnect_Done", uri );
        sdkInfo.core->logMessage( "---------------------------" );

        // Call getConfig next
        if( sdkInfo.success ) {
            sdkInfo.core->getConfig( uri );
        }
        else {
            sdkInfo.core->getConfig( "" );
        }
    }

    /**
     * Function is triggered when the user first connects to the server. Called
     * automatically when the SDK object is created. Continues only if the gameId
     * and URI are valid strings.
     */
    int Core::connect( const char* gameId, const char* uri ) {
        // If the URI was set properly, record it
        if( ( uri != NULL ) && strcmp( uri, "" ) != 0 ) {
            m_connectUri = uri;
            logMessage( "connectUri set:", m_connectUri.c_str() );
        }
        // URI was not set properly
        else {
            displayError( "Core::connect()", "Valid URI was not specified when trying to connect." );
            return 1;
        }
        
        // If the gameId was set properly, record it
        if( ( gameId != NULL ) && strcmp( gameId, "" ) != 0 ) {
            m_gameId = gameId;
            logMessage( "gameId set:", m_gameId.c_str() );
        }
        // gameId was not set properly
        else {
            displayError( "Core::connect()", "The gameId was not set or is invalid." );
            return 1;
        }
        
        // Make the request
        mf_httpGetRequest( API_CONNECT, "GET", "getConnect_Done", "", "text/plain; charset=utf-8" );
        
        // Success
        return 0;
    }


    //--------------------------------------
    //--------------------------------------
    //--------------------------------------
    /**
     * Callback function occurs when API_GET_CONFIG is successful.
     *
     * Sets telemetry settings, including batch sizes and priority
     * levels.
     */
    void getConfig_Done( p_glSDKInfo sdkInfo ) {
        const char* json = sdkInfo.data.c_str();
        sdkInfo.core->logMessage( "---------------------------" );
        sdkInfo.core->logMessage( "getConfig_Done", json );
        sdkInfo.core->logMessage( "---------------------------" );
        //printf( "\n---------------------------\n" );
        //printf( "getConfig_Done: \n%s", json );
        //printf( "\n---------------------------\n" );
        
        json_t* root;
        json_error_t error;

        // Set the return message
        Const::Message returnMessage = Const::Message_Connect;
        
        // parse JSON data from session
        root = json_loads( json, 0, &error );
        if( root && json_is_object( root ) ) {
            // First, check for errors
            if( sdkInfo.core->mf_checkForJSONErrors( root ) ) {
                //returnMessage = Const::Message_Error;
            }
            else {
                // Set the connected state
                sdkInfo.core->setConnectedState( true );

                json_t* eventsDetailLevel = json_object_get( root, "eventsDetailLevel" );
                if( eventsDetailLevel && json_is_integer( eventsDetailLevel ) ) {
                    sdkInfo.core->config.eventsDetailLevel = (int)json_integer_value( eventsDetailLevel );
                }
                
                json_t* eventsPeriodSecs = json_object_get( root, "eventsPeriodSecs" );
                if( eventsPeriodSecs && json_is_integer( eventsPeriodSecs ) ) {
                    sdkInfo.core->config.eventsPeriodSecs = (int)json_integer_value( eventsPeriodSecs );
                }
                
                json_t* eventsMinSize = json_object_get( root, "eventsMinSize" );
                if( eventsMinSize && json_is_integer( eventsMinSize ) ) {
                    sdkInfo.core->config.eventsMinSize = (int)json_integer_value( eventsMinSize );
                }
                
                json_t* eventsMaxSize = json_object_get( root, "eventsMaxSize" );
                if( eventsMaxSize && json_is_integer( eventsMaxSize ) ) {
                    sdkInfo.core->config.eventsMaxSize = (int)json_integer_value( eventsMaxSize );
                }
            }
        }
        json_decref( root );
        
        sdkInfo.core->logMessage( "getConfig_Done: done reading json data");
        
        // Push Connect message
        sdkInfo.core->pushMessageStack( returnMessage, json );
    }

    /**
     * Function is triggered whenever we want to get config information from the server.
     */
    void Core::getConfig( const char* uri ) {
        // If the URI was set properly, record it
        if( ( uri != NULL ) && strcmp( uri, "" ) != 0 ) {
            m_connectUri = uri;
            logMessage( "connectUri set from CONFIG:", m_connectUri.c_str() );
        }
        // URI was not set properly
        else {
            displayError( "getConnect_Done", "Valid URI was not specified when trying to retrieve config." );
        }

        // Reset the connected state
        setConnectedState( false );
        
        // Make the request
        mf_httpGetRequest( API_GET_CONFIG, "GET", "getConfig_Done" );
    }


    //--------------------------------------
    //--------------------------------------
    //--------------------------------------
    /**
     * Callback function occurs when API_POST_DEVICE_UPDATE is successful.
     */
    void deviceUpdate_Done( p_glSDKInfo sdkInfo ) {
        const char* json = sdkInfo.data.c_str();
        sdkInfo.core->logMessage( "---------------------------" );
        sdkInfo.core->logMessage( "deviceUpdate_Done", json );
        sdkInfo.core->logMessage( "---------------------------" );
        //printf( "\n---------------------------\n" );
        //printf( "deviceUpdate_Done: \n%s", json );
        //printf( "\n---------------------------\n" );

        json_t* root;
        json_error_t error;

        // Set the return message
        Const::Message returnMessage = Const::Message_DeviceUpdate;
        
        // Parse the JSON data from the response
        root = json_loads( json, 0, &error );
        if( root && json_is_object( root ) ) {
            // First, check for errors
            if( sdkInfo.core->mf_checkForJSONErrors( root ) ) {
                returnMessage = Const::Message_Error;
            }
        }
        json_decref( root );
        
        // Push Login message
        sdkInfo.core->pushMessageStack( returnMessage );
    }

    /**
     * Function makes a POST request to the server with the device Id to update the record
     * server-side. This particular call will also be used to get the wasession to validate
     * against the local record. If there is a mismatch, the user needs to logout.
     */
    void Core::deviceUpdate() {
        // Setup the data
        string data = "deviceId=";
        data += m_deviceId;
        data += "&gameId=";
        data += m_gameId;

        // Make the request
        mf_httpGetRequest( API_POST_DEVICE_UPDATE, "POST", "deviceUpdate_Done", data );
    }

    //--------------------------------------
    //--------------------------------------
    //--------------------------------------
    /**
     * Callback function occurs when API_GET_AUTH_STATUS is successful.
     */
    void authStatus_Done( p_glSDKInfo sdkInfo ) {
        const char* json = sdkInfo.data.c_str();
        sdkInfo.core->logMessage( "---------------------------" );
        sdkInfo.core->logMessage( "authStatus_Done", json );
        sdkInfo.core->logMessage( "---------------------------" );
        //printf( "\n---------------------------\n" );
        //printf( "authStatus_Done: \n%s", json );
        //printf( "\n---------------------------\n" );

        json_t* root;
        json_error_t error;

        // Set the return message
        Const::Message returnMessage = Const::Message_AuthStatus;
        
        // Parse the JSON data from the response
        root = json_loads( json, 0, &error );
        if( root && json_is_object( root ) ) {
            // First, check for errors
            if( sdkInfo.core->mf_checkForJSONErrors( root ) ) {
                //returnMessage = Const::Message_Error;
            }
        }
        json_decref( root );
        
        // Push Login message
        sdkInfo.core->pushMessageStack( returnMessage, json );

        // Get the player info
        sdkInfo.core->getPlayerInfo();
    }

    /**
     * Function makes a POST request to the server to get the authentication
     * status of a user using the cookie.
     */
    void Core::authStatus() {
        // Make the request
        mf_httpGetRequest( API_GET_AUTH_STATUS, "GET", "authStatus_Done", "" );
    }


    //--------------------------------------
    //--------------------------------------
    //--------------------------------------
    /**
     * Callback function occurs when API_POST_REGISTER is successful.
     *
     * Parses user data returned from the server.
     */
    void register_Done( p_glSDKInfo sdkInfo ) {
        const char* json = sdkInfo.data.c_str();
        sdkInfo.core->logMessage( "---------------------------" );
        sdkInfo.core->logMessage( "register_Done", json );
        sdkInfo.core->logMessage( "---------------------------" );
        //printf( "\n---------------------------\n" );
        //printf( "register_Done: \n%s", json );
        //printf( "\n---------------------------\n" );

        json_t* root;
        json_error_t error;

        // Set the return message
        Const::Message returnMessage = Const::Message_Register;
        
        // Parse the JSON data from the response
        root = json_loads( json, 0, &error );
        if( root && json_is_object( root ) ) {
            // First, check for errors
            if( sdkInfo.core->mf_checkForJSONErrors( root ) ) {
                returnMessage = Const::Message_Error;
            }
        }
        json_decref( root );
        
        // Parse the returned JSON data, which should indicated user information
        if( sdkInfo.core->setUserInfo( json ) ) {
            // TODO: error handling
            sdkInfo.core->displayWarning( "register_Done", "Object userInfo is undefined. User and course Ids will not be included in the start session." );
        }
        
        // Push Login message
        sdkInfo.core->pushMessageStack( returnMessage );

        // Call the update device Id API
        sdkInfo.core->deviceUpdate();
    }

    /**
     * Register student function passes registration information to the server for validation.
     */
    void Core::registerStudent( const char* username, const char* password, const char* firstName, const char* lastInitial ) {
        // Setup the data
        string data = "systemRole=student";
        data += "&username=";
        data += username;
        data += "&firstName=";
        data += firstName;
        data += "&lastName=";
        data += lastInitial;
        data += "&password=";
        data += password;
        
        // Make the request
        mf_httpGetRequest( API_POST_REGISTER, "POST", "register_Done", data );
    }

    /**
     * Register teacher function passes registration information to the server for validation.
     */
    void Core::registerInstructor( const char* name, const char* email, const char* password, bool newsletter ) {
        // Parse the name
        string fullName = name;
        string firstName = fullName.substr( 0, strchr( name, ' ' ) - name );
        string lastName = fullName.substr( strchr( name, ' ' ) - name + 1 );

        // Setup the data
        string data = "systemRole=instructor";
        data += "&email=";
        data += email;
        data += "&firstName=";
        data += firstName.c_str();
        data += "&lastName=";
        data += lastName.c_str();
        data += "&password=";
        data += password;
        data += "&newsletter=";
        data += newsletter;
        
        // Make the request
        mf_httpGetRequest( API_POST_REGISTER, "POST", "register_Done", data );
    }


    //--------------------------------------
    //--------------------------------------
    //--------------------------------------
    /**
     * Callback function occurs when API_GET_PLAYERINFO is successful.
     *
     * Parses player info returned from the server.
     */
    void getPlayerInfo_Done( p_glSDKInfo sdkInfo ) {
        const char* json = sdkInfo.data.c_str();
        sdkInfo.core->logMessage( "---------------------------" );
        sdkInfo.core->logMessage( "getPlayerInfo_Done", json );
        sdkInfo.core->logMessage( "---------------------------" );
        //printf( "\n---------------------------\n" );
        //printf( "getPlayerInfo_Done: %s", json );
        //printf( "\n---------------------------\n" );
        
        json_t* root;
        json_error_t error;
        
        // Set the return message
        Const::Message returnMessage = Const::Message_GetPlayerInfo;
        
        // Parse the JSON data from the response
        root = json_loads( json, 0, &error );
        if( root && json_is_object( root ) ) {
            // First, check for errors
            if( sdkInfo.core->mf_checkForJSONErrors( root ) ) {
                returnMessage = Const::Message_Error;
            }

            // Parse JSON data if there was no error
            if( returnMessage != Const::Message_Error ) {
                // Get the client totalTimePlayed value to do a comparison
                float clientTotalTimePlayed = sdkInfo.core->getTotalTimePlayed();
                
                // Parse the totalTimePlayed received from the server
                float serverTotalTimePlayed = 0.0;
                json_t* totalTimePlayed = json_object_get( root, "totalTimePlayed" );
                if( totalTimePlayed && json_is_integer( totalTimePlayed ) ) {
                    serverTotalTimePlayed = (float)json_integer_value( totalTimePlayed );
                }

                //printf( "client time: %f, server time: %f\n", clientTotalTimePlayed, serverTotalTimePlayed );
                
                // If the client is greater than the server, the info is already stored in the player
                // info and database, no need to do anything. Otherwise, set the local values.
                if( clientTotalTimePlayed < serverTotalTimePlayed ) {
                    sdkInfo.core->updatePlayerInfoKey( "$totalTimePlayed$", serverTotalTimePlayed );
                    sdkInfo.core->mf_updateTotalTimePlayedInSessionTable( serverTotalTimePlayed );
                    //printf( "using server time: %f\n", serverTotalTimePlayed );
                }
                else {
                    //printf( "using client time: %f\n", clientTotalTimePlayed );
                }
            }
        }
        json_decref( root );
        
        // Push SavePlayerInfo message
        sdkInfo.core->pushMessageStack( returnMessage );
    }

    /**
     * GetPlayerInfo function will grab player info as JSON from server. This info
     * contains total time played and achievement infofmration.
     */
    void Core::getPlayerInfo() {
        // Make the request
        mf_httpGetRequest( API_GET_PLAYERINFO, "GET", "getPlayerInfo_Done" );
    }


    //--------------------------------------
    //--------------------------------------
    //--------------------------------------
    /**
     * Callback function occurs when API_GET_USER_PROFILE is successful.
     *
     * Parses user info returned from the server.
     */
    void getUserInfo_Done( p_glSDKInfo sdkInfo ) {
        const char* json = sdkInfo.data.c_str();
        sdkInfo.core->logMessage( "---------------------------" );
        sdkInfo.core->logMessage( "getUserInfo_Done", json );
        sdkInfo.core->logMessage( "---------------------------" );
        //printf( "\n---------------------------\n" );
        //printf( "getUserInfo_Done: %s", json );
        //printf( "\n---------------------------\n" );
        
        json_t* root;
        json_error_t error;
        
        // Set the return message
        Const::Message returnMessage = Const::Message_GetUserInfo;

        // Parse the JSON data from the response
        root = json_loads( json, 0, &error );
        if( root && json_is_object( root ) ) {
            // First, check for errors
            if( sdkInfo.core->mf_checkForJSONErrors( root ) ) {
                //returnMessage = Const::Message_Error;
            }
            
            // Parse JSON data if there was no error
            if( returnMessage != Const::Message_Error ) {

                // The username must be valid
                json_t* username = json_object_get( root, "username" );
                if( username && json_is_string( username ) ) {
                    sdkInfo.core->userInfo.username = json_string_value( username );
                }

                // The first name must be valid
                json_t* firstName = json_object_get( root, "firstName" );
                if( firstName && json_is_string( firstName ) ) {
                    sdkInfo.core->userInfo.firstName = json_string_value( firstName );
                }

                // The last initial must be valid
                json_t* lastName = json_object_get( root, "lastName" );
                if( lastName && json_is_string( lastName ) ) {
                    sdkInfo.core->userInfo.lastInitial = json_string_value( lastName );
                }

                // The email must be valid
                json_t* email = json_object_get( root, "email" );
                if( email && json_is_string( email ) ) {
                    sdkInfo.core->userInfo.email = json_string_value( email );
                }
            }
        }
        json_decref( root );
        
        // Push SavePlayerInfo message
        sdkInfo.core->pushMessageStack( returnMessage, json );
    }

    /**
     * GetUserInfo function will grab user info as JSON from server. This info
     * contains username, first name, last initial, etc.
     */
    void Core::getUserInfo() {
        // Make the request
        mf_httpGetRequest( API_GET_USER_PROFILE, "GET", "getUserInfo_Done" );
    }

    /**
     * Extract userInfo from login_Done callback.
     * Grabs userId information.
     */
    int Core::setUserInfo( const char* json ) {
        json_error_t error;

        if( !m_userInfo ) {
            json_decref( m_userInfo );
        }
        
        // parse JSON data from session
        m_userInfo = json_loads( json, 0, &error );
        
        // If the userInfo does not exist in any form, we have an error
        if( !m_userInfo || !json_is_object( m_userInfo ) ) {
            // Error handled in function calling this
            m_userInfo = NULL;
            return 1;
        }
        
        // Parse the userId
        json_t *userId = json_object_get( m_userInfo, "id" );
        if( userId && json_is_integer( userId ) ) {
            m_userId = (int)json_integer_value( userId );
        }
        // UserId was invalid
        else {
            return 1;
        }
        
        // Success
        return 0;
    }

    /**
     * Callback function occurs when API_POST_LOGIN is successful.
     *
     * Parses user data returned from the server.
     */
    void login_Done( p_glSDKInfo sdkInfo ) {
        const char* json = sdkInfo.data.c_str();
        sdkInfo.core->logMessage( "---------------------------" );
        sdkInfo.core->logMessage( "login_Done", json );
        sdkInfo.core->logMessage( "---------------------------" );
        //printf( "\n---------------------------\n" );
        //printf( "login_Done: \n%s", json );
        //printf( "\n---------------------------\n" );

        json_t* root;
        json_error_t error;

        // Set the return message
        Const::Message returnMessage = Const::Message_Login;
        
        // Parse the JSON data from the response
        root = json_loads( json, 0, &error );
        if( root && json_is_object( root ) ) {
            // First, check for errors
            if( sdkInfo.core->mf_checkForJSONErrors( root ) ) {
                //returnMessage = Const::Message_Error;
            }
        }
        json_decref( root );
        
        // Parse the returned JSON data, which should indicated user information
        if( sdkInfo.core->setUserInfo( json ) ) {
            // TODO: error handling
            sdkInfo.core->displayWarning( "login_Done", "Object userInfo is undefined. User and course Ids will not be included in the start session." );
    	}
        
        // Push Login message
        sdkInfo.core->pushMessageStack( returnMessage, json );

        // Call the update device Id API
        sdkInfo.core->deviceUpdate();

        // Get the player info
        sdkInfo.core->getPlayerInfo();
    }

    /**
     * Login function passes username and password strings to the server for validation.
     */
    void Core::login( const char* username, const char* password, const char* type ) {
        // Allow for null types and "glasslab"
        if( type == NULL || strncmp( type, "glasslab", 8 ) ) {
            // Set the username and password in the postdata
            string data = "username=";
            data += username;
            data += "&password=";
            data += password;
            
            // Make the request
            mf_httpGetRequest( API_POST_LOGIN, "POST", "login_Done", data );
        }
        // Type is unrecognized
        else {
            displayWarning( "Core::login()", "Type must either be NULL or \"glasslab\". Login will not be called." );
        }
    }
    
    
    //--------------------------------------
    //--------------------------------------
    //--------------------------------------
    /**
     * Callback function occurs when API_POST_ENROLL is successful.
     *
     * Indicates successfull enrollment.
     */
    void enroll_Done( p_glSDKInfo sdkInfo ) {
        const char* json = sdkInfo.data.c_str();
        sdkInfo.core->logMessage( "---------------------------" );
        sdkInfo.core->logMessage( "enroll_Done", json );
        sdkInfo.core->logMessage( "---------------------------" );
        //printf( "\n---------------------------\n" );
        //printf( "enroll_Done: \n%s", json );
        //printf( "\n---------------------------\n" );

        json_t* root;
        json_error_t error;

        // Set the return message
        Const::Message returnMessage = Const::Message_Enroll;
        
        // Parse the JSON data from the response
        root = json_loads( json, 0, &error );
        if( root && json_is_object( root ) ) {
            // First, check for errors
            if( sdkInfo.core->mf_checkForJSONErrors( root ) ) {
                //returnMessage = Const::Message_Error;
            }
        }
        json_decref( root );
        
        // Push Enroll message
        sdkInfo.core->pushMessageStack( returnMessage, json );
    }
    
    /**
     * Enroll function passes the courseCode to the server to attempt enrollment into a class.
     */
    void Core::enroll( const char* courseCode ) {
        // Setup the data
        string data = "courseCode=";
        data += courseCode;
        
        // Make the request
        mf_httpGetRequest( API_POST_ENROLL, "POST", "enroll_Done", data );
    }
    
    /**
     * Callback function occurs when API_POST_UNENROLL is successful.
     *
     * Indicates successful unenrollment
     */
    void unenroll_Done( p_glSDKInfo sdkInfo ) {
        const char* json = sdkInfo.data.c_str();
        sdkInfo.core->logMessage( "---------------------------" );
        sdkInfo.core->logMessage( "unenroll_Done", json );
        sdkInfo.core->logMessage( "---------------------------" );
        //printf( "\n---------------------------\n" );
        //printf( "unenroll_Done: \n%s", json );
        //printf( "\n---------------------------\n" );

        json_t* root;
        json_error_t error;

        // Set the return message
        Const::Message returnMessage = Const::Message_UnEnroll;
        
        // Parse the JSON data from the response
        root = json_loads( json, 0, &error );
        if( root && json_is_object( root ) ) {
            // First, check for errors
            if( sdkInfo.core->mf_checkForJSONErrors( root ) ) {
                returnMessage = Const::Message_Error;
            }
        }
        json_decref( root );
        
        // Push UnEnroll message
        sdkInfo.core->pushMessageStack( returnMessage );
    }
    
    /**
     * Unenroll function passes the courseId to the server to attempt unenrollment from a class.
     */
    void Core::unenroll( const char* courseId ) {
        // Setup the data
        string data = "courseId=";
        data += courseId;

        // Make the request
        mf_httpGetRequest( API_POST_UNENROLL, "POST", "unenroll_Done", data );
    }


    //--------------------------------------
    //--------------------------------------
    //--------------------------------------
    /**
     * Callback function occurs when API_GET_COURSES is successful.
     *
     * Indicates successfull course retrieval.
     */
    void getCourses_Done( p_glSDKInfo sdkInfo ) {
        const char* json = sdkInfo.data.c_str();
        sdkInfo.core->logMessage( "---------------------------" );
        sdkInfo.core->logMessage( "getCourses_Done", json );
        sdkInfo.core->logMessage( "---------------------------" );
        //printf( "\n---------------------------\n" );
        //printf( "getCourses_Done: \n%s", json );
        //printf( "\n---------------------------\n" );

        json_t* root;
        json_error_t error;

        // Set the return message
        Const::Message returnMessage = Const::Message_GetCourses;
        
        // Parse the JSON data from the response
        root = json_loads( json, 0, &error );
        if( root && json_is_object( root ) ) {
            // First, check for errors
            if( sdkInfo.core->mf_checkForJSONErrors( root ) ) {
                returnMessage = Const::Message_Error;
            }
        }
        json_decref( root );
        
        // Push GetCourses message
        sdkInfo.core->pushMessageStack( returnMessage, json );
    }
    
    /**
     * GetCourses function communicates with the server to get course information.
     */
    void Core::getCourses() {
        // Make the request
        mf_httpGetRequest( API_GET_COURSES, "GET", "getCourses_Done" );
    }
    

    //--------------------------------------
    //--------------------------------------
    //--------------------------------------
    /**
     * Callback function occurs when API_POST_LOGOUT is successful.
     *
     * Indicates successfull user logout.
     */
    void logout_Done( p_glSDKInfo sdkInfo ) {
        const char* json = sdkInfo.data.c_str();
        sdkInfo.core->logMessage( "---------------------------" );
        sdkInfo.core->logMessage( "logout_Done", json );
        sdkInfo.core->logMessage( "---------------------------" );
        //printf( "\n---------------------------\n" );
        //printf( "logout_Done: \n%s", json );
        //printf( "\n---------------------------\n" );

        json_t* root;
        json_error_t error;

        // Set the return message
        Const::Message returnMessage = Const::Message_Logout;
        
        // Parse the JSON data from the response
        root = json_loads( json, 0, &error );
        if( root && json_is_object( root ) ) {
            // First, check for errors
            if( sdkInfo.core->mf_checkForJSONErrors( root ) ) {
                returnMessage = Const::Message_Error;
            }
        }
        json_decref( root );
        
        // Push Logout message
        sdkInfo.core->pushMessageStack( returnMessage );
    }

    /**
     * Logout function communicates with the server to log the current user out.
     */
    void Core::logout() {
        // Setup the data
        string data = " ";

        // Make the request
        mf_httpGetRequest( API_POST_LOGOUT, "POST", "logout_Done", data );
    }


    //--------------------------------------
    //--------------------------------------
    //--------------------------------------
    /**
     * Callback function occurs when API_POST_SESSION_START is successful.
     *
     * Extracts the gameSessionId from the server response to be used later for
     * sending telemetry and further session management.
     */
    void startSession_Done( p_glSDKInfo sdkInfo ) {
        const char* json = sdkInfo.data.c_str();
        sdkInfo.core->logMessage( "---------------------------" );
        sdkInfo.core->logMessage( "startSession_Done", json );
        sdkInfo.core->logMessage( "---------------------------" );
        //printf( "\n---------------------------\n" );
        //printf( "startSession_Done: \n%s", json );
        //printf( "\n---------------------------\n" );
        
        json_t* root;
        json_error_t error;

        // Set the return message
        Const::Message returnMessage = Const::Message_StartSession;
        
        // Parse the JSON data from the response
        root = json_loads( json, 0, &error );
        if( root && json_is_object( root ) ) {
            // First, check for errors
            if( sdkInfo.core->mf_checkForJSONErrors( root ) ) {
                returnMessage = Const::Message_Error;
            }
            else {
                // We receive back a gameSessionId, which is important for sending telemetry and closing sessions
                json_t* sessionId = json_object_get( root, "gameSessionId" );

                // The gameSessionId must be valid
                if( sessionId && json_is_string( sessionId ) ) {
                    sdkInfo.core->setSessionId( json_string_value( sessionId ) );
                    //printf( "sessionId: %s\n", sdkInfo.core->getSessionId() );
                    
                    // Decrease the reference count, this way Jansson can release "sessionId" resources
#if !WIN32
                    json_decref( root );
#endif
                }
                // Invalid or non-existent gameSessionId
                else {
                    sdkInfo.core->displayError( "startSession_Done()", "The gameSessionId is missing from the startSession callback response!" );
                    returnMessage = Const::Message_Error;
                }
            }
        }
        // There is no data in the response
        else {
            sdkInfo.core->displayError( "startSession_Done()", "The startSession callback response is empty!" );
            returnMessage = Const::Message_Error;
        }
        
        // Decrease the reference count, this way Jansson can release "root" resources
        json_decref( root );
        
        // Push the StartSession message
        sdkInfo.core->pushMessageStack( returnMessage );
    }

    /**
     * StartSession function communicates with the server to open a new session.
     * Server requires deviceId in order to proceed. CourseId and gameLevel are
     * also sent along but optional.
     */
    void Core::startSession() {
        // Start the session timer
        startSessionTimer();

        // Set initial parameters to set in the API call
        string courseOut = "";
        string dataOut = "";
        
        // Check for the deviceId and append it to the postdata
        if( m_deviceId.length() > 0 ) {
            dataOut += "deviceId=";
            dataOut += m_deviceId;
        }
        // The deviceId is invalid
        else {
            // TODO: error
            displayError( "Core::startSession()", "The deviceId was missing in the startSession API call!" );
            return;
        }

        // Reset the gameSessionEventOrder in the SQLite database
        m_gameSessionEventOrder = 1;
        m_dataSync->updateGameSessionEventOrderWithDeviceId( m_deviceId, m_gameSessionEventOrder );

        // Append gameLevel info to the postdata if it exists
        if(m_gameLevel.length() > 0){
            dataOut += "&gameLevel=";
            dataOut += m_gameLevel;
        }

        // Append the gameId
        dataOut += "&gameId=";
        dataOut += m_gameId;

        // Append timestamp info to the postdata
        char t[21];
        sprintf(t, "%d", (int)time(NULL));
        dataOut += "&timestamp=";
        dataOut += t;

        // Add this message to the message queue
        mf_addMessageToDataQueue( API_POST_SESSION_START, "POST", "startSession_Done", dataOut, "application/x-www-form-urlencoded" );

        // Record an "start session" telemetry event
        saveTelemEvent( "Game_start_session" );
    }


    //--------------------------------------
    //--------------------------------------
    //--------------------------------------
    /**
     * Callback function occurs when API_POST_SESSION_END is successful.
     *
     * Unset the current gameSessionId as final cleanup.
     */
    void endSession_Done( p_glSDKInfo sdkInfo ) {
        const char* json = sdkInfo.data.c_str();
        sdkInfo.core->logMessage( "---------------------------" );
        sdkInfo.core->logMessage( "endSession_Done", json );
        sdkInfo.core->logMessage( "---------------------------" );
        //printf( "\n---------------------------\n" );
        //printf( "endSession_Done: \n%s", json );
        //printf( "\n---------------------------\n" );

        json_t* root;
        json_error_t error;

        // Set the return message
        Const::Message returnMessage = Const::Message_EndSession;
        
        // Parse the JSON data from the response
        root = json_loads( json, 0, &error );
        if( root && json_is_object( root ) ) {
            // First, check for errors
            if( sdkInfo.core->mf_checkForJSONErrors( root ) ) {
                returnMessage = Const::Message_Error;
            }
        }
        json_decref( root );

        // Clear the session Id if the callback was valid
        if( returnMessage == Const::Message_EndSession ) {
            sdkInfo.core->setSessionId( "" );
        }

        // Push EndSession message
        sdkInfo.core->pushMessageStack( returnMessage );
    }

    /**
     * EndSession function communicates with the server to end an existing session. The
     * gameSessionId is passed along in the postdata so the server knows which session to
     * end. Since the API call is stored in SQLite prior to upload, the gameSessionId is
     * stored as "$gameSessionId$", as it will be replaced later with the correct value
     * during the message queue flushing.
     */
    void Core::endSession() {
        // Stop the session timer
        stopSessionTimer();

        // Record an "end session" telemetry event
        saveTelemEvent( "Game_end_session" );

        // Send all events before end session
        sendTelemEvents();

        // Append the gameSessionId to the postdata
        string dataOut = "";
        dataOut += "gameSessionId=$gameSessionId$";
        // Append the timestamp to the postdata
        char t[21];
        sprintf(t, "%d", (int)time(NULL));
        dataOut += "&timestamp=";
        dataOut += t;

        // Add this message to the message queue
        mf_addMessageToDataQueue( API_POST_SESSION_END, "POST", "endSession_Done", dataOut, "application/x-www-form-urlencoded" );
    }
    
    
    //--------------------------------------
    //--------------------------------------
    //--------------------------------------
    /**
     * Callback function occurs when API_POST_GAMEDATA is successful.
     */
    void saveGame_Done( p_glSDKInfo sdkInfo ) {
        const char* json = sdkInfo.data.c_str();
        //sdkInfo.core->logMessage( "---------------------------" );
        //sdkInfo.core->logMessage( "saveGame_Done", json );
        //sdkInfo.core->logMessage( "---------------------------" );
        //printf( "\n---------------------------\n" );
        //printf( "saveGame_Done: %s", json );
        //printf( "\n---------------------------\n" );
        
        json_t* root;
        json_error_t error;
        
        // Set the return message
        Const::Message returnMessage = Const::Message_GameSave;
        
        // Parse the JSON data from the response
        root = json_loads( json, 0, &error );
        if( root && json_is_object( root ) ) {
            // First, check for errors
            if( sdkInfo.core->mf_checkForJSONErrors( root ) ) {
                //returnMessage = Const::Message_Error;
            }
        }
        json_decref( root );
        
        // Push GameSave message
        sdkInfo.core->pushMessageStack( returnMessage, json );
    }
    
    /**
     * SaveGame function communicates with the server to save the game data.
     */
    void Core::saveGame( const char* gameData ) {
        // Add this message to the message queue
        //mf_addMessageToDataQueue( url, "saveGame_Done", cb, gameData, "application/json" );
        mf_httpGetRequest( API_POST_SAVEGAME, "POST", "saveGame_Done", gameData, "application/json" );
    }

    /**
     * Callback function occurs when API_GET_GAMEDATA is successful.
     */
    void getSaveGame_Done( p_glSDKInfo sdkInfo ) {
        const char* json = sdkInfo.data.c_str();
        //sdkInfo.core->logMessage( "---------------------------" );
        //sdkInfo.core->logMessage( "getSaveGame_Done", json );
        //sdkInfo.core->logMessage( "---------------------------" );
        //printf( "\n---------------------------\n" );
        //printf( "getSaveGame_Done: %s", json );
        //printf( "\n---------------------------\n" );
        
        json_t* root;
        json_error_t error;
        
        // Set the return message
        Const::Message returnMessage = Const::Message_GetGameSave;
        
        // Parse the JSON data from the response
        root = json_loads( json, 0, &error );
        if( root && json_is_object( root ) ) {
            // First, check for errors
            if( sdkInfo.core->mf_checkForJSONErrors( root ) ) {
                //returnMessage = Const::Message_Error;
            }
        }
        json_decref( root );
        
        // Push GameSave message
        sdkInfo.core->pushMessageStack( returnMessage, json );
    }

    /**
     * GetSaveGame function communicates with the server to sget the save game data for the user.
     */
    void Core::getSaveGame() {
        // Add this message to the message queue
        //mf_addMessageToDataQueue( url, "getSaveGame_Done", cb );
        mf_httpGetRequest( API_GET_SAVEGAME, "GET", "getSaveGame_Done" );
    }


    /**
     * Callback function occurs when API_DELETE_SAVEGAME is successful.
     */
    void deleteSaveGame_Done( p_glSDKInfo sdkInfo ) {
        const char* json = sdkInfo.data.c_str();
        //sdkInfo.core->logMessage( "---------------------------" );
        //sdkInfo.core->logMessage( "deleteSaveGame_Done", json );
        //sdkInfo.core->logMessage( "---------------------------" );
        printf( "\n---------------------------\n" );
        printf( "deleteSaveGame_Done: %s", json );
        printf( "\n---------------------------\n" );
        
        json_t* root;
        json_error_t error;
        
        // Set the return message
        Const::Message returnMessage = Const::Message_DeleteGameSave;
        
        // Parse the JSON data from the response
        root = json_loads( json, 0, &error );
        if( root && json_is_object( root ) ) {
            // First, check for errors
            if( sdkInfo.core->mf_checkForJSONErrors( root ) ) {
                //returnMessage = Const::Message_Error;
            }
        }
        json_decref( root );
        
        // Push GameSave message
        sdkInfo.core->pushMessageStack( returnMessage, json );
    }

    /**
     * DeleteSaveGame function communicates with the server to get the save game data for the user.
     */
    void Core::deleteSaveGame() {
        // Add this message to the message queue
        //mf_addMessageToDataQueue( url, "deleteSaveGame_Done", cb );
        mf_httpGetRequest( API_DELETE_SAVEGAME, "DELETE", "deleteSaveGame_Done" );
    }


    //--------------------------------------
    //--------------------------------------
    //--------------------------------------
    /**
     * Callback function occurs when API_POST_ACHIEVEMENT is successful.
     */
    void saveAchievement_Done( p_glSDKInfo sdkInfo ) {
        const char* json = sdkInfo.data.c_str();
        //sdkInfo.core->logMessage( "---------------------------" );
        //sdkInfo.core->logMessage( "saveAchievement_Done", json );
        //sdkInfo.core->logMessage( "---------------------------" );
        //printf( "\n---------------------------\n" );
        //printf( "saveAchievement_Done: %s", json );
        //printf( "\n---------------------------\n" );
        
        json_t* root;
        json_error_t error;
        
        // Set the return message
        Const::Message returnMessage = Const::Message_SaveAchievement;
        
        // Parse the JSON data from the response
        root = json_loads( json, 0, &error );
        if( root && json_is_object( root ) ) {
            // First, check for errors
            if( sdkInfo.core->mf_checkForJSONErrors( root ) ) {
                returnMessage = Const::Message_Error;
            }
        }
        json_decref( root );
        
        // Push GameSave message
        sdkInfo.core->pushMessageStack( returnMessage );
    }

    /**
     * Functions saves an achievement event with the following information:
     *  - item
     *  - group
     *  - subGroup
     */
    void Core::saveAchievement( const char* item, const char* group, const char* subGroup ) {
        // Append the parameter information to the postdata
        string dataOut = "{\"item\":\"";
        dataOut += item;
        dataOut += "\",\"group\":\"";
        dataOut += group;
        dataOut += "\",\"subGroup\":\"";
        dataOut += subGroup;
        dataOut += "\"}";
        
        // Add this message to the message queue
        mf_addMessageToDataQueue( API_POST_ACHIEVEMENT, "POST", "saveAchievement_Done", dataOut, "application/json" );
    }


    //--------------------------------------
    //--------------------------------------
    //--------------------------------------
    /**
     * Callback function occurs when API_POST_PLAYERINFO is successful.
     *
     */
    void savePlayerInfo_Done( p_glSDKInfo sdkInfo ) {
        const char* json = sdkInfo.data.c_str();
        //sdkInfo.core->logMessage( "---------------------------" );
        //sdkInfo.core->logMessage( "savePlayerInfo_Done", json );
        //sdkInfo.core->logMessage( "---------------------------" );
        //printf( "\n---------------------------\n" );
        //printf( "savePlayerInfo_Done: %s", json );
        //printf( "\n---------------------------\n" );
        
        json_t* root;
        json_error_t error;
        
        // Set the return message
        Const::Message returnMessage = Const::Message_SavePlayerInfo;
        
        // Parse the JSON data from the response
        root = json_loads( json, 0, &error );
        if( root && json_is_object( root ) ) {
            // First, check for errors
            if( sdkInfo.core->mf_checkForJSONErrors( root ) ) {
                returnMessage = Const::Message_Error;
            }
        }
        json_decref( root );
        
        // Push SavePlayerInfo message
        sdkInfo.core->pushMessageStack( returnMessage );
    }
    
    /**
     * SavePlayerInfo function communicates with the server to save player info data.
     */
    void Core::savePlayerInfo() {
        // Get the player info JSON as string
        char* rootJSON = json_dumps( m_playerInfo, JSON_ENCODE_ANY | JSON_INDENT(3) | JSON_SORT_KEYS );
        string jsonOut = rootJSON;
        free( rootJSON );

        // Replace $totalTimePlayed$ with totalTimePlayed
        string totalTimePlayedTag = "$totalTimePlayed$";
        string::size_type n = jsonOut.find( totalTimePlayedTag );
        if( n != string::npos ) {
            jsonOut.replace( n, totalTimePlayedTag.size(), "totalTimePlayed" );
        }

        // Add this message to the queue
        //mf_addMessageToDataQueue( API_POST_PLAYERINFO, "POST", "savePlayerInfo_Done", jsonOut.c_str(), "application/json" );
    }


    //--------------------------------------
    //--------------------------------------
    //--------------------------------------
    /**
     * Callback function occurs when API_POST_PLAYERINFO is successful.
     *
     */
    void sendTotalTimePlayed_Done( p_glSDKInfo sdkInfo ) {
        const char* json = sdkInfo.data.c_str();
        //sdkInfo.core->logMessage( "---------------------------" );
        //sdkInfo.core->logMessage( "sendTotalTimePlayed_Done", json );
        //sdkInfo.core->logMessage( "---------------------------" );
        //printf( "\n---------------------------\n" );
        //printf( "sendTotalTimePlayed_Done: %s", json );
        //printf( "\n---------------------------\n" );
        
        json_t* root;
        json_error_t error;
        
        // Set the return message
        Const::Message returnMessage = Const::Message_SendTotalTimePlayed;
        
        // Parse the JSON data from the response
        root = json_loads( json, 0, &error );
        if( root && json_is_object( root ) ) {
            // First, check for errors
            if( sdkInfo.core->mf_checkForJSONErrors( root ) ) {
                returnMessage = Const::Message_Error;
            }
        }
        json_decref( root );
        
        // Push SavePlayerInfo message
        sdkInfo.core->pushMessageStack( returnMessage );
    }
    
    /**
     * SendTotalTimePlayed function communicates with the server to send totalTimePlayed data.
     */
    void Core::sendTotalTimePlayed() {
        // Append the totalTimePlayed to the postdata
        string dataOut = "";
        dataOut += "{\"setTime\":";
        // Append the timestamp to the postdata
        char t[21];
        sprintf(t, "%.2f", getTotalTimePlayed());
        dataOut += t;
        dataOut += "}";

        // Add this message to the queue
        mf_addMessageToDataQueue( API_POST_TOTAL_TIME_PLAYED, "POST", "sendTotalTimePlayed_Done", dataOut, "application/json" );
    }


    //--------------------------------------
    //--------------------------------------
    //--------------------------------------
    /**
     * Callback function occurs when API_POST_EVENTS is successful.
     *
     * Indicates successful telemetry event send.
     */
    void sendTelemEvent_Done( p_glSDKInfo sdkInfo ) {
        const char* json = NULL;
        json = sdkInfo.data.c_str();
        
        //sdkInfo.core->logMessage( "---------------------------" );
        //sdkInfo.core->logMessage( "sendTelemEvent_Done", json );
        //sdkInfo.core->logMessage( "---------------------------" );
        //printf( "\n---------------------------\n" );
        //printf( "sendTelemEvent_Done: \n%s", json );
        //printf( "\n---------------------------\n" );

        
        json_t* root;
        json_error_t error;

        // Set the return message
        Const::Message returnMessage = Const::Message_Event;
        
        // Parse the JSON data from the response
        root = json_loads( json, 0, &error );
        if( root && json_is_object( root ) ) {
            // First, check for errors
            if( sdkInfo.core->mf_checkForJSONErrors( root ) ) {
                returnMessage = Const::Message_Error;
            }
        }
        json_decref( root );
        
        // Push Event message
        // Since the client does nothing with Message_Event yet, we won't send this.
        // It tends to clog the queue.
        //sdkInfo.core->pushMessageStack( returnMessage );
    }

    /**
     * SendTelemEvents function writes all saved telemetry events to the message queue
     * for processing later. This function is generally called on update, so if no
     * events exist, it will call the callbacks normally.
     */
    void Core::sendTelemEvents() {
        // Get the current total time played for updating and setting in the internal database
        float newTime = getTotalTimePlayed();

        // First, increment the game timer if it is active
        if( m_gameTimerActive ) {
            // Get the current time
            time_t currentTime = time( NULL );
            
            // Measure the time between last game time and current (in seconds)
            float delta = difftime( currentTime, m_gameTimerLast );
            m_gameTimerLast = currentTime;

            // Increment the total time played and set it
            newTime += delta;
            updatePlayerInfoKey( "$totalTimePlayed$", newTime );
        }
        

        //printf( "send telem event\n%s\n%s", clientCB.c_str(), coreCB.c_str() );
        // Set the initial JSON postdata string
        string jsonOut = "";
        
        // Continue with the request if there is telemetry to send
        if( json_array_size( m_telemEvents ) > 0 ) {
            // Get the telemetry JSON
            char* rootJSON = json_dumps( m_telemEvents, JSON_ENCODE_ANY | JSON_INDENT(3) | JSON_SORT_KEYS );
            jsonOut = rootJSON;
            free( rootJSON );
         
            printf( "\n---------------------------\n" );
            printf( "sendTelemEvents Num of Events being sent: %lu\n", json_array_size(m_telemEvents) );
            printf( "sendTelemEvents: %s\n", jsonOut.c_str() );
            printf( "\n---------------------------\n" );
         
            // Add this message to the queue
            mf_addMessageToDataQueue( API_POST_EVENTS, "POST", "sendTelemEvent_Done", jsonOut.c_str(), "application/json" );
            
            // Reset all memebers in event list
            clearTelemEventValues();
            //printf( "sendTelemEvents Events after clear: %lu\n", json_array_size(m_telemEvents) );
        }
        // No telemetry exists, perform callbacks normally
        else {
            
            // If there is no telemetry, we won't bother doing the callback.
            // This ends up clogging the response queue

            /*p_glSDKInfo sdkInfo;
            sdkInfo.sdk = m_sdk; // exposed sdk
            sdkInfo.core = this; // hidden core
            sdkInfo.clientCB = getClientCallback( clientCB );
            
            CoreCallback_Func coreCallback = getCoreCallback( cb );//cb(sdkInfo);
            coreCallback( sdkInfo );*/
        }

        // Update the totalTimePlayed in the SQLite database
        m_dataSync->updatePlayerInfoFromDeviceId( m_deviceId, newTime, m_gameSessionEventOrder );

        // Attempt to dispatch the message queue
        attemptMessageDispatch();
    }
    
    /**
     * This function will force a call to flushMsgQ to ensure all requests are made to the server.
     * This is a useful function for games that store the database in memory, making it a temporary
     * entity. Just before closing the application, it would be useful to flush the remaining events
     * stored.
     */
    void Core::forceFlushTelemEvents() {
        sendTotalTimePlayed();
        
        // Only flush the queue if we are connected
        if( getConnectedState() ) {
            m_dataSync->flushMsgQ();
        }
    }

    /**
     * Function attempts to dispatch the telemetry events in the message queue, based on the
     * interval timer, minimum number of events, and maximum number of allowed events.
     */
    void Core::attemptMessageDispatch() {
        // Get the current time
        time_t currentTime = time( NULL );
        // Measure the time between last and current (in seconds)
        float secondsElapsed = difftime( currentTime, m_telemetryLastTime );
        //printf( "Current elapsed: %f\n", secondsElapsed );

        // If the seconds elapsed exceeds our interval, reset the current telemetry clock and
        // flush the message queue
        //if( secondsElapsed > config.eventsPeriodSecs ) {

            //printf("secondsElapsed: %f,  getMessageTableSize: %d, config.eventsMinSize: %d\n", secondsElapsed, m_dataSync->getMessageTableSize(), config.eventsMinSize);
            // Check that we exceed the minimum number of events to send data
            //if( m_dataSync->getMessageTableSize() > config.eventsMinSize ) {
                // In addition to flushing the message queue, do a POST on the totalTimePlayed
                sendTotalTimePlayed();

                printf( "Connected: %d, Seconds elapsed for flush %f with %i events\n", getConnectedState(), secondsElapsed, m_dataSync->getMessageTableSize() );

                // Only flush the queue if we are connected
                //if( getConnectedState() ) {
                    m_dataSync->flushMsgQ();
                //}
                m_telemetryLastTime = currentTime;
            //}
        //}
        // Or send events if we've exceeded the table size limit
        /*else if( m_dataSync->getMessageTableSize() > config.eventsMaxSize ) {
            printf( "reached max number of events: %i with %f elapsed with %i\n", m_dataSync->getMessageTableSize(), secondsElapsed, config.eventsPeriodSecs );
            m_telemetryLastTime = currentTime;
            m_dataSync->flushMsgQ();

            // In addition to flushing the message queue, do a POST on the player info
            savePlayerInfo();
        }*/
    }


    //--------------------------------------
    //--------------------------------------
    //--------------------------------------
    /**
     * Callback function occurs when an HTTP request is successful.
     *
     * 
     */
    void httpGetRequest_Done( struct evhttp_request* req, void* arg ) {
        
        // Cast the argument to a request info object
        p_glHttpRequest *request = (p_glHttpRequest *)arg;
        
        printf( "httpGetRequest_Done\n");
        // If the request object exists, parse the response
        if( req ) {
            // There should be request info, including callbacks and statuses
            if( request != NULL ) {
                size_t s;
                size_t inbuffer_size    = req->body_size + 1;
                char* inbuffer          = new char[inbuffer_size];
                s = evbuffer_remove( req->input_buffer, inbuffer, inbuffer_size - 1 );
                inbuffer[s] = '\0';
                
                // Get the cookie if it was passed along from the server
                const char* setCookie = evhttp_find_header(req->input_headers, "set-cookie" );
                if( setCookie ) {
                    request->core->setCookie( setCookie );
                }

                // Mark the status of the event as success to remove it from the table
                request->core->mf_updateMessageStatusInDataQueue( request->msgQRowId, "success" );

                
                // If the core callback exists, run it
                if( request->coreCBKey != "" ) {
                    if( request->core->getCoreCallbackCancelState( request->coreCBKey ) ) {
                        request->core->setCoreCallbackCancelState( request->coreCBKey, false );
                        request->core->logMessage( "\n\t\t request ignored because it was cancelled" );
                    }
                    else {
                        p_glSDKInfo sdkInfo;
                        sdkInfo.sdk = request->sdk;
                        sdkInfo.core = request->core;
                        sdkInfo.data = inbuffer;
                        sdkInfo.success = true;
                        request->core->getCoreCallback( request->coreCBKey )( sdkInfo );
                    }
                }

                // Delete the info buffer
                delete [] inbuffer;
            }
            // Request info was likely corrupt
            else {
                printf( "The HTTP request object was NULL, is there a possible corruption?" );
            }

            if(request) {
                // Terminate event_base_dispatch()
                event_base_loopbreak( request->base );
                
                //evhttp_connection_free(request->conn);
                //event_base_free(request->base);
                
                // We're done, clean up the request
                //delete request;
            }
        }
        // The request object did not exist, which is likely due to no internet connection
        else {
            // Mark the status of the event as failed
            if( request != NULL ) {
                request->core->displayWarning( "httpGetRequest_Done()", "The HTTP request object was NULL, is there a proper internet connection?" );
                request->core->mf_updateMessageStatusInDataQueue( request->msgQRowId, "failed" );

                string errorMessage = "{\"status\":\"error\",\"error\":\"request timed out\"}";
                // If the core callback exists, run it
                if( request->coreCBKey != "" ) {
                    if( request->core->getCoreCallbackCancelState( request->coreCBKey ) ) {
                        request->core->setCoreCallbackCancelState( request->coreCBKey, false );
                        request->core->logMessage( "\n\t\t request ignored because it was cancelled" );
                    }
                    else {
                        p_glSDKInfo sdkInfo;
                        sdkInfo.sdk = request->sdk;
                        sdkInfo.core = request->core;
                        sdkInfo.data = errorMessage.c_str();
                        sdkInfo.success = false;
                        request->core->getCoreCallback( request->coreCBKey )( sdkInfo );
                    }
                }

                if(request) {
                    // Terminate event_base_dispatch()
                    event_base_loopbreak( request->base );
                    
                    //evhttp_connection_free(request->conn);
                    //event_base_free(request->base);
                    
                    // We're done, clean up the request
                    //delete request;
                }
            }
        }
    }

    /**
     * HttpGetRequest function performs a GET/POST request to the server for
     * a single event extracted from the SQLite database.
     */
    void Core::mf_httpGetRequest( string path, string requestType, string coreCB, string postdata, const char* contentType, int rowId ) {
        // Set initial information to send to the server
        struct evhttp_uri* uri;
        int port;
        const char* host;
        string url, requestMethod;
        struct evbuffer* postdata_buffer = NULL;

		#ifdef _WIN32
		WSADATA WSAData;
		WSAStartup( 0x101, &WSAData );
		#endif

        // Set the URI, host, and port information
        url  = m_connectUri;
        uri  = evhttp_uri_parse( url.c_str() );
        port = evhttp_uri_get_port( uri );
        host = evhttp_uri_get_host( uri );
        // Default to port 80
        if( port == -1 ) {
            port = 80;
		}

        //req.api = req.api.split( ":gameId" ).join( m_clientId );
        // Update the path to remove all ":gameId" occurrences, replacing them with the actual gameId
        string gameIdTag = ":gameId";
        string::size_type n = 0;
        while( ( n = path.find( gameIdTag, n ) ) != string::npos ) {
            path.replace( n, gameIdTag.size(), m_gameId );
            n += m_gameId.size();
        }

        // Reset the cancel state of the callback
        setCoreCallbackCancelState( coreCB, false );

        // Create the HTTP request object and set appropriate information
        p_glHttpRequest *httpRequest = new p_glHttpRequest();
        httpRequest->sdk        = m_sdk;
        httpRequest->core       = this;
        httpRequest->coreCBKey  = coreCB;
        httpRequest->msgQRowId  = rowId;
        // Set additional information in the HTTP request
        httpRequest->base       = event_base_new();
        httpRequest->conn       = evhttp_connection_base_new( httpRequest->base, NULL, host, port );
        httpRequest->req        = evhttp_request_new( httpGetRequest_Done, (void *)httpRequest );

        // Only proceed if the HTTP request is valid
        if( httpRequest->req != NULL ) {
            // If the cookie already exists, pass it along
            if( m_cookie != "" ) {
                evhttp_add_header( httpRequest->req->output_headers, "cookie", m_cookie.c_str() );
            }
            
            // Set the request type to GET by default
            requestMethod = "GET";
            
            // add header if contentType set
            if( contentType != NULL && strlen(contentType) != 0 ) {
                evhttp_add_header( httpRequest->req->output_headers, "Content-type", contentType );
            }

            // If postdata exists in this request, ensure it has the correct information
            if( postdata.length() > 0 ) {
                postdata_buffer = evbuffer_new();
                evbuffer_add_printf( postdata_buffer, "%s", postdata.c_str() );

                // add default contentType
                if( contentType == NULL || strlen(contentType) == 0 ) {
                    evhttp_add_header( httpRequest->req->output_headers, "Content-type", "application/x-www-form-urlencoded" );
                }
                
                // add content length if post
                char t[255];
                sprintf(t, "%lu", postdata.length());
                evhttp_add_header( httpRequest->req->output_headers, "Content-Length", t);
                
                // Add the postdata to the request and reset the type to POST
                evbuffer_add_buffer( httpRequest->req->output_buffer, postdata_buffer );
                requestMethod = "POST";
            }
            
            // Setup the user agent and host
            char headerUserAgent[255];
            if(m_clientName.length() == 0) {
                sprintf(headerUserAgent, "GlassLab SDK v%s", SDK_VERSION);
            } else {
                sprintf(headerUserAgent, "GlassLab SDK v%s - Client \"%s\" v%s", SDK_VERSION, m_clientName.c_str(), m_clientVersion.c_str());
            }
            evhttp_add_header( httpRequest->req->output_headers, "User-Agent", headerUserAgent);
            char headerHost[255];
            sprintf(headerHost, "%s:%d", host, port);
            evhttp_add_header( httpRequest->req->output_headers, "Host", headerHost );
            evhttp_add_header( httpRequest->req->output_headers, "Accept", "*/*" );


            // Update the request type based on the parameter, if it exists
            if( requestType.c_str() != NULL ) {
                requestMethod = requestType;
            }

            // Get the correct evhttp_cmd_type
            evhttp_cmd_type requestCmd = EVHTTP_REQ_GET;
            if( strstr( requestMethod.c_str(), "POST" ) ) {
                requestCmd = EVHTTP_REQ_POST;
            }
            else if( strstr( requestMethod.c_str(), "DELETE" ) ) {
                requestCmd = EVHTTP_REQ_DELETE;
            }
            
            // Print the results
            printf("connect url: %s, method: %s, host: %s, port:%d, path: %s, cookie: %s\n", url.c_str(), requestMethod.c_str(), host, port, path.c_str(), m_cookie.c_str());

            // Dispatch the request
            evhttp_connection_set_timeout( httpRequest->conn, 600 );
            evhttp_make_request( httpRequest->conn, httpRequest->req, requestCmd, path.c_str() );
            event_base_dispatch( httpRequest->base );
            
            // Free the connection
            evhttp_connection_free(httpRequest->conn);
            event_base_free(httpRequest->base);
        }
        
        // Finished with the URI object, free it
        evhttp_uri_free( uri );
        
        // Free the postdata information if any existed
        if( postdata_buffer != NULL ) {
            evbuffer_free( postdata_buffer );
        }
    }

    /**
     * Function cancels a request based on key.
     */
    void Core::cancelRequest( const char* requestKey ) {
        //printf( "\n\t\t cancelling request: %s\n", requestKey );
        setCoreCallbackCancelState( requestKey, true );
    }


    //--------------------------------------
    //--------------------------------------
    //--------------------------------------
    /**
     * Store all callback functions in a map so the SQLite database has a reference.
     * TODO: include client callback functions
     */
    void Core::mf_setupCallbacks() {
        coreCallbackStructure getConnect_Structure;
        getConnect_Structure.coreCB = getConnect_Done;
        getConnect_Structure.cancel = false;
        m_coreCallbackMap[ "getConnect_Done" ] = getConnect_Structure;

        coreCallbackStructure getConfig_Structure;
        getConfig_Structure.coreCB = getConfig_Done;
        getConfig_Structure.cancel = false;
        m_coreCallbackMap[ "getConfig_Done" ] = getConfig_Structure;

        coreCallbackStructure deviceUpdate_Structure;
        deviceUpdate_Structure.coreCB = deviceUpdate_Done;
        deviceUpdate_Structure.cancel = false;
        m_coreCallbackMap[ "deviceUpdate_Done" ] = deviceUpdate_Structure;

        coreCallbackStructure authStatus_Structure;
        authStatus_Structure.coreCB = authStatus_Done;
        authStatus_Structure.cancel = false;
        m_coreCallbackMap[ "authStatus_Done" ] = authStatus_Structure;

        coreCallbackStructure register_Structure;
        register_Structure.coreCB = register_Done;
        register_Structure.cancel = false;
        m_coreCallbackMap[ "register_Done" ] = register_Structure;

        coreCallbackStructure getPlayerInfo_Structure;
        getPlayerInfo_Structure.coreCB = getPlayerInfo_Done;
        getPlayerInfo_Structure.cancel = false;
        m_coreCallbackMap[ "getPlayerInfo_Done" ] = getPlayerInfo_Structure;

        coreCallbackStructure getUserInfo_Structure;
        getUserInfo_Structure.coreCB = getUserInfo_Done;
        getUserInfo_Structure.cancel = false;
        m_coreCallbackMap[ "getUserInfo_Done" ] = getUserInfo_Structure;

        coreCallbackStructure login_Structure;
        login_Structure.coreCB = login_Done;
        login_Structure.cancel = false;
        m_coreCallbackMap[ "login_Done" ] = login_Structure;

        coreCallbackStructure logout_Structure;
        logout_Structure.coreCB = logout_Done;
        logout_Structure.cancel = false;
        m_coreCallbackMap[ "logout_Done" ] = logout_Structure;

        coreCallbackStructure enroll_Structure;
        enroll_Structure.coreCB = enroll_Done;
        enroll_Structure.cancel = false;
        m_coreCallbackMap[ "enroll_Done" ] = enroll_Structure;

        coreCallbackStructure unenroll_Structure;
        unenroll_Structure.coreCB = unenroll_Done;
        unenroll_Structure.cancel = false;
        m_coreCallbackMap[ "unenroll_Done" ] = unenroll_Structure;

        coreCallbackStructure getCourses_Structure;
        getCourses_Structure.coreCB = getCourses_Done;
        getCourses_Structure.cancel = false;
        m_coreCallbackMap[ "getCourses_Done" ] = getCourses_Structure;

        coreCallbackStructure startSession_Structure;
        startSession_Structure.coreCB = startSession_Done;
        startSession_Structure.cancel = false;
        m_coreCallbackMap[ "startSession_Done" ] = startSession_Structure;

        coreCallbackStructure endSession_Structure;
        endSession_Structure.coreCB = endSession_Done;
        endSession_Structure.cancel = false;
        m_coreCallbackMap[ "endSession_Done" ] = endSession_Structure;
        
        coreCallbackStructure saveGame_Structure;
        saveGame_Structure.coreCB = saveGame_Done;
        saveGame_Structure.cancel = false;
        m_coreCallbackMap[ "saveGame_Done" ] = saveGame_Structure;

        coreCallbackStructure getSaveGame_Structure;
        getSaveGame_Structure.coreCB = getSaveGame_Done;
        getSaveGame_Structure.cancel = false;
        m_coreCallbackMap[ "getSaveGame_Done" ] = getSaveGame_Structure;

        coreCallbackStructure deleteSaveGame_Structure;
        deleteSaveGame_Structure.coreCB = deleteSaveGame_Done;
        deleteSaveGame_Structure.cancel = false;
        m_coreCallbackMap[ "deleteSaveGame_Done" ] = deleteSaveGame_Structure;

        coreCallbackStructure saveAchievement_Structure;
        saveAchievement_Structure.coreCB = saveAchievement_Done;
        saveAchievement_Structure.cancel = false;
        m_coreCallbackMap[ "saveAchievement_Done" ] = saveAchievement_Structure;

        coreCallbackStructure savePlayerInfo_Structure;
        savePlayerInfo_Structure.coreCB = savePlayerInfo_Done;
        savePlayerInfo_Structure.cancel = false;
        m_coreCallbackMap[ "savePlayerInfo_Done" ] = savePlayerInfo_Structure;

        coreCallbackStructure sendTotalTimePlayed_Structure;
        sendTotalTimePlayed_Structure.coreCB = sendTotalTimePlayed_Done;
        sendTotalTimePlayed_Structure.cancel = false;
        m_coreCallbackMap[ "sendTotalTimePlayed_Done" ] = sendTotalTimePlayed_Structure;

        coreCallbackStructure sendTelemEvent_Structure;
        sendTelemEvent_Structure.coreCB = sendTelemEvent_Done;
        sendTelemEvent_Structure.cancel = false;
        m_coreCallbackMap[ "sendTelemEvent_Done" ] = sendTelemEvent_Structure;
    }

    /**
     * Function returns a Core Callback function from the map using the key parameter.
     */
    CoreCallback_Func Core::getCoreCallback( string key ) {
        // Callback function does not exist
        if( m_coreCallbackMap.find( key ) == m_coreCallbackMap.end() ) {
            return NULL;
        }
        // Callback function exists
        else {
            return m_coreCallbackMap[ key ].coreCB;
        }
    }
    /**
     * Function returns the cancel state of the Core Callback function requested.
     */
    bool Core::getCoreCallbackCancelState( string key ) {
        // Callback function does not exist
        if( m_coreCallbackMap.find( key ) == m_coreCallbackMap.end() ) {
            return true;
        }
        // Callback function exists
        else {
            return m_coreCallbackMap[ key ].cancel;
        }
    }
    /**
     * Function sets the cancel state of a core callback function.
     */
    void Core::setCoreCallbackCancelState( string key, bool state ) {
        // Callback function does not exist
        if( m_coreCallbackMap.find( key ) != m_coreCallbackMap.end() ) {
            m_coreCallbackMap[ key ].cancel = state;
        }
    }


    //--------------------------------------
    //--------------------------------------
    //--------------------------------------
    /**
     * Function adds a new message to the SQLite message queue.
     */
    void Core::mf_addMessageToDataQueue( string path, string requestType, string coreCB, string postdata, const char* contentType ) {
        // Only proceed if the data sync object exists
        if( m_dataSync != NULL ) {
            m_dataSync->addToMsgQ( m_deviceId, path, requestType, coreCB, postdata, contentType );
        }
        else {
            displayError( "Core::mf_addMessageToDataQueue()", "Tried to add a message to MSG_QUEUE but the sync object was NULL!" );
        }
    }
    /**
     * Function updates the status of a single message in the SQLite message queue.
     */
    void Core::mf_updateMessageStatusInDataQueue( int rowId, string status ) {
        // Only proceed if the data sync object exists
        if( m_dataSync != NULL && rowId != -1 ) {
            //printf( "Marking object at %i as %s\n", rowId, status.c_str() );
            m_dataSync->updateMessageStatus( rowId, status );
        }
    }

    /**
     * Function updates the totalTimePlayed for the user associated with the parameter devieceId 
     * in the SQLite session table.
     */
    void Core::mf_updateTotalTimePlayedInSessionTable( float totalTimePlayed ) {
        // Only proceed if the data sync object exists
        if( m_dataSync != NULL ) {
            m_dataSync->updatePlayerInfoFromDeviceId( m_deviceId, totalTimePlayed, m_gameSessionEventOrder );
        }
        else {
            displayError( "Core::mf_updateTotalTimePlayedInSessionTable()", "Tried to update the totalTimePlayed in the SESSION table but the sync object was NULL!" );
        }
    }


    //--------------------------------------
    //--------------------------------------
    //--------------------------------------
    /**
     * Append telemetry functions for all possible data types.
     * TODO: include bools
     */
    void Core::addTelemEventValue( const char* key, const char* value ) {
        json_object_set_new( m_telemEventValues, key, json_string( value ) );
    }
    void Core::addTelemEventValue( const char* key, int8_t value ) {
        json_object_set_new( m_telemEventValues, key, json_integer( value ) );
    }
    void Core::addTelemEventValue( const char* key, int16_t value ) {
        json_object_set_new( m_telemEventValues, key, json_integer( value ) );
    }
    void Core::addTelemEventValue( const char* key, int32_t value ) {
        json_object_set_new( m_telemEventValues, key, json_integer( value ) );
    }
    void Core::addTelemEventValue( const char* key, uint8_t value ) {
        json_object_set_new( m_telemEventValues, key, json_integer( value ) );
    }
    void Core::addTelemEventValue( const char* key, uint16_t value ) {
        json_object_set_new( m_telemEventValues, key, json_integer( value ) );
    }
    void Core::addTelemEventValue( const char* key, uint32_t value ) {
        json_object_set_new( m_telemEventValues, key, json_integer( value ) );
    }
    void Core::addTelemEventValue( const char* key, float value ) {
        json_object_set_new( m_telemEventValues, key, json_real( value ) );
    }
    void Core::addTelemEventValue( const char* key, double value ) {
        json_object_set_new( m_telemEventValues, key, json_real( value ) );
    }
    void Core::addTelemEventValue( const char* key, bool value ) {
        json_object_set_new( m_telemEventValues, key, json_boolean( value ) );
    }

    /**
     * Function clears all telemetry events stored in the JSON object.
     */
    void Core::clearTelemEventValues() {
        // Initialize events array
        int ret = json_array_clear(m_telemEvents);
        if( !m_telemEvents || ret == -1 ) {
            displayError( "Core::clearTelemEventValues()", "There was an error intializing a new telemetry document after clearing the old one." );
        }
    }

    /**
     * Functions saves a telemetry event by name with all default parameters,
     * including a timestamp, name, gameId, gameSessionId, deviceId, 
     * clientVersion, gameLevel, and the data itself.
     */
    void Core::saveTelemEvent( const char* name ) {
        // Create the JSON event object to populate
        json_t* event = json_object();
        if( event ) {
            // Time this event occurred
            time_t t = time(NULL);

            // Increment the session timer if it is active
            if( m_autoSessionManagement && m_sessionTimerActive ) {
                // Measure the time between last session time and current (in seconds)
                float delta = difftime( t, m_sessionTimerLast );
                m_sessionTimerLast = t;

                // If the time since last event is greater than the SESSION_TIMEOUT, start a new session
                if( delta >= SESSION_TIMEOUT ) {
                    endSession();
                    startSession();
                }
            }

            // Set default information
            json_object_set_new( event, "clientTimeStamp", json_integer( (int)t ) );
            json_object_set_new( event, "eventName", json_string( name ) );
            json_object_set_new( event, "gameId",  json_string( m_gameId.c_str() ) );
            json_object_set_new( event, "gameSessionId", json_string( "$gameSessionId$" ) );
            json_object_set_new( event, "gameSessionEventOrder", json_integer( m_gameSessionEventOrder++ ) );//"$gameSessionEventOrder$" ) );

            // Set the deviceId if it exists
            if( m_deviceId.length() > 0 ) {
                json_object_set_new( event, "deviceId", json_string( m_deviceId.c_str() ) );
            }
            // Set the clientVersion if it exists
            if( m_clientVersion.length() > 0 ) {
                json_object_set_new( event, "clientVersion", json_string( m_clientVersion.c_str() ) );
            }
            // Set the gameLevel if it exists
            if( m_gameLevel.length() > 0 ) {
                json_object_set_new( event, "gameLevel", json_string( m_gameLevel.c_str() ) );
            }
            // Set the eventData as a separate JSON document using the values
            json_object_set_new( event, "eventData", m_telemEventValues );

            // Get the total time played from the player info and set it (-1 indicates an error or it doesn't exist)
            float totalTimePlay = getTotalTimePlayed();
            json_object_set_new( event, "totalTimePlayed", json_real( totalTimePlay ) );
            
            // Append the final event structure to the telemetry events JSON object
            json_array_append_new( m_telemEvents, event );
            
            //string jsonOut = "";
            //char* rootJSON = json_dumps( event, JSON_SORT_KEYS );//JSON_ENCODE_ANY | JSON_INDENT(3) | JSON_SORT_KEYS );
            //jsonOut = rootJSON;
            //free( rootJSON );            
            //printf( "\n---------------------------\n" );
            //printf( "saveTelemEvent: \n%s", jsonOut.c_str() );
            //printf( "\n---------------------------\n" );
            
            // Reset all memebers in event object
            m_telemEventValues  = json_object();
        }
        // If the JSON object wasn't created properly, we have an error
        else {
            displayError( "Core::saveTelemEvent()", "Could not create a new event document, unable to send event." );
        }
    }


    //--------------------------------------
    //--------------------------------------
    //--------------------------------------
    /**
     * Allow the user to update the key/values in the user player data structure.
     */
    void Core::updatePlayerInfoKey( const char* key, const char* value ) {
        json_object_set_new( m_playerInfo, key, json_string( value ) );
    }
    void Core::updatePlayerInfoKey( const char* key, int8_t value ) {
        json_object_set_new( m_playerInfo, key, json_integer( value ) );
    }
    void Core::updatePlayerInfoKey( const char* key, int16_t value ) {
        json_object_set_new( m_playerInfo, key, json_integer( value ) );
    }
    void Core::updatePlayerInfoKey( const char* key, int32_t value ) {
        json_object_set_new( m_playerInfo, key, json_integer( value ) );
    }
    void Core::updatePlayerInfoKey( const char* key, uint8_t value ) {
        json_object_set_new( m_playerInfo, key, json_integer( value ) );
    }
    void Core::updatePlayerInfoKey( const char* key, uint16_t value ) {
        json_object_set_new( m_playerInfo, key, json_integer( value ) );
    }
    void Core::updatePlayerInfoKey( const char* key, uint32_t value ) {
        json_object_set_new( m_playerInfo, key, json_integer( value ) );
    }
    void Core::updatePlayerInfoKey( const char* key, float value ) {
        json_object_set_new( m_playerInfo, key, json_real( value ) );
    }
    void Core::updatePlayerInfoKey( const char* key, double value ) {
        json_object_set_new( m_playerInfo, key, json_real( value ) );
    }
    void Core::updatePlayerInfoKey( const char* key, bool value ) {
        json_object_set_new( m_playerInfo, key, json_boolean( value ) );
    }

    /**
     * Allow the user to remove a key/value pair from the user info data structure.
     */
    void Core::removePlayerInfoKey( const char* key ) {
        // Don't allow totalTimePlayed to be deleted
        if( strcmp(key, "$totalTimePlayed$") == 0 ) {
            return;
        }

        json_object_del( m_playerInfo, key );
    }

    /**
     * Set platform required default key-value pairs in the player info data structure.
     */
    void Core::setDefaultPlayerInfoKeys() {
        json_object_set_new( m_playerInfo, "$totalTimePlayed$", json_real( m_dataSync->getTotalTimePlayedFromDeviceId( m_deviceId ) ) );
    }

    /**
     * Function resets all player info values.
     */
    void Core::resetPlayerInfo() {
        // clear object memebers
        json_object_clear(m_playerInfo);

        // Set default keys in the blob
        setDefaultPlayerInfoKeys();
    }


    //--------------------------------------
    //--------------------------------------
    //--------------------------------------
    /**
     * Game timer function for starting. This function also resets the timer to be current.
     */
    void Core::startGameTimer() {
        // Only reset the last time if we were previously inactive
        if( !m_gameTimerActive ) {
            m_gameTimerActive = true;
            m_gameTimerLast = time( NULL );
        }
    }

    /**
     * Game timer function for stopping.
     */
    void Core::stopGameTimer() {
        m_gameTimerActive = false;
    }


    //--------------------------------------
    //--------------------------------------
    //--------------------------------------
    /**
     * Session timer function for starting. This function also resets the timer to be current.
     */
    void Core::startSessionTimer() {
        // Only reset the last time if we were previously inactive
        if( !m_sessionTimerActive ) {
            m_sessionTimerActive = true;
            m_sessionTimerLast = time( NULL );
        }
    }

    /**
     * Session timer function for stopping.
     */
    void Core::stopSessionTimer() {
        m_sessionTimerActive = false;
    }


    //--------------------------------------
    //--------------------------------------
    //--------------------------------------
    /**
     * Function will reset the internal database containing all CONFIG, SESSION,
     * and MESSAGE information.
     */
    void Core::resetDatabase() {
        if( m_dataSync != NULL ) {
            m_dataSync->resetDatabase();
        }
    }


    //--------------------------------------
    //--------------------------------------
    //--------------------------------------
    /**
     * Setters.
     */
    void Core::setConnectUri( const char* uri ) {
        m_connectUri = uri;
    }

    void Core::setName( const char* name ) {
        m_clientName = name;
    }

    void Core::setVersion( const char* version ) {
        m_clientVersion = version;
    }

    void Core::setGameLevel( const char* gameLevel ) {
        m_gameLevel = gameLevel;
    }

    void Core::setUserId( int userId ) {
        m_userId = userId;
    }
    
    void Core::setConfig( glConfig _config ){
        memcpy(&config, &_config, sizeof(glConfig));
    }
    
    void Core::setTime( time_t time ) {
        m_currentTime = time;
    }

    void Core::setPlayerHandle( const char* handle ) {
        printf( "player handle to set: %s\n" , handle );

        m_playerHandle = handle;

        // The device Id is handle_deviceId
        char newDeviceId[256];
        strcpy( newDeviceId, handle );
        strcat( newDeviceId, "_" );
        strcat( newDeviceId, m_baseDeviceId.c_str() );

        // Update the database with this information
        if( m_dataSync != NULL ) {
            printf( "setting new device Id using player handle: %s", newDeviceId );
            m_dataSync->updateSessionTableWithPlayerHandle( newDeviceId );

            // Get the cookie stored for this device Id
            m_cookie = m_dataSync->getCookieFromDeviceId( newDeviceId );
        }

        // Set the new device Id
        m_deviceId = newDeviceId;

        // Send the current player info and reset it for this user
        savePlayerInfo();
        resetPlayerInfo();

        // Get the game session event order to update
        m_gameSessionEventOrder = m_dataSync->getGameSessionEventOrderFromDeviceId( m_deviceId );

        // Call the update device Id API
        //deviceUpdate();
    }

    void Core::removePlayerHandle( const char* handle ) {
        logMessage( "attempting to remove player handle from db:", handle );

        // The device Id is handle_deviceId
        char deviceIdToRemove[256];
        strcpy( deviceIdToRemove, handle );
        strcat( deviceIdToRemove, "_" );
        strcat( deviceIdToRemove, m_baseDeviceId.c_str() );

        // Update the database with this information
        if( m_dataSync != NULL ) {
            logMessage( "device Id to remove:", deviceIdToRemove );
            m_dataSync->removeSessionWithDeviceId( deviceIdToRemove );
        }
    }

    void Core::setCookie( const char* cookie ) {
        m_cookie = cookie;

        // Set the cookie in the SESSION table
        if( m_dataSync != NULL ) {
            logMessage( "setting cookie:", m_cookie.c_str() );
            m_dataSync->updateSessionTableWithCookie( m_deviceId, m_cookie );
        }
    }

    void Core::setSessionId( const char* sessionId ) {
        m_sessionId = sessionId;

        // Set the gameSessionId in the SESSION table
        if( m_dataSync != NULL ) {
            logMessage( "setting game session Id:", m_sessionId.c_str() );
            m_dataSync->updateSessionTableWithGameSessionId( m_deviceId, m_sessionId );

            // Get the game session event order to update
            m_gameSessionEventOrder = m_dataSync->getGameSessionEventOrderFromDeviceId( m_deviceId );
        }
    }

    void Core::setAutoSessionManagement( bool state ) {
        m_autoSessionManagement = state;
    }


    //--------------------------------------
    //--------------------------------------
    //--------------------------------------
    /**
     * Getters.
     */
    const char* Core::getConnectUri() {
        return m_connectUri.c_str();
    }

    int Core::getUserId() {
        return m_userId;
    }

    const char* Core::getId() {
        return m_gameId.c_str();
    }

    const char* Core::getCookie() {
        return m_cookie.c_str();
    }

    const char* Core::getSessionId() {
        return m_sessionId.c_str();
    }

    Const::Status Core::getLastStatus() {
        return m_lastStatus;
    }

    float Core::getTotalTimePlayed() {
        // Get the JSON value from player info
        json_t* totalTimePlayedAsJSON = json_object_get( m_playerInfo, "$totalTimePlayed$" );
        // Verify it exists and is the right type
        if( totalTimePlayedAsJSON && json_is_real( totalTimePlayedAsJSON ) ) {
            // Return it
            return (float)json_real_value( totalTimePlayedAsJSON );
        }

        // Otherwise, return default
        return -1;
    }


    //--------------------------------------
    //--------------------------------------
    //--------------------------------------
    /**
     * Helper functions for the connected state.
     */
    void Core::setConnectedState( bool state ) {
        m_connected = state;
    }

    bool Core::getConnectedState() {
        return m_connected;
    }


    //--------------------------------------
    //--------------------------------------
    //--------------------------------------
    /**
     * Helper function for displaying warning messages.
     */
    void Core::displayWarning( string location, string warning ) {
        //printf( "\n\n\t\tWARNING:\n" );
        //printf( "\t\t\t%s\n\n", location.c_str() );
        //printf( "\t\t\t%s\n\n", warning.c_str() );
        logMessage( location.c_str(), warning.c_str() );
    }

    /**
     * Helper function for displaying error messages.
     */
    void Core::displayError( string location, string error ) {
        //printf( "\n\n\t\tERROR:\n" );
        //printf( "\t\t\t%s\n\n", location.c_str() );
        //printf( "\t\t\t%s\n\n", error.c_str() );
        logMessage( location.c_str(), error.c_str() );
    }

    /**
     * Helper function for recording and displaying general messages to be surfaced in the client.
     */
    void Core::logMessage( const char* message, const char* data ) {
        string concat;
        if( data == NULL ) {
            printf("%s\n", message);
            concat = string(message);
        }
        else {
            printf("%s %s\n", message, data);
            concat = string(message) + string(data);
        }
        
        m_logQueue.push( concat );
    }

    /**
     * Helper function to pop from the logging queue.
     */
    const char* Core::popLogQueue() {
        if( m_logQueue.empty() ) {
            return "";
        }
        else {
            std::string nextLog = m_logQueue.front();
            m_logQueue.pop();
            return nextLog.c_str();
        }
    }

    /**
     * Helper function for checking for errors within JSON response.
     */
    bool Core::mf_checkForJSONErrors( json_t* root ) {
        json_t* errorJSON = json_object_get( root, "error" );
        if( errorJSON && json_is_string( errorJSON ) ) {
            displayError( "http callback", json_string_value( errorJSON ) );
            return 1;
        }
        return 0;
    }

}; // end nsGlasslabSDK