//
//  main.cpp
//  GlassLab SDK Hello World
//
//  This program will demonstrate the various API calls that can be made to the
//  GlassLabSDK. The GlassLabSDK is meant to provide a connection between the
//  game client and hosted server. It's primary function is to allow the user to
//  maintain and store telemetry streams within game sessions.
//
//  This program will step through each API call in a defined order, with a detailed
//  description of what is happening both client-side and server-side.
//
//  Created by Joseph Sutton on 1/21/14.
//  Modified by Ben Dapkiewicz on 5/20/14.
//  Copyright (c) 2014 GlassLab. All rights reserved.
//

#include <iostream>
#include "glasslab_sdk.h"


// A reference to the GlassLabSDK object, of which we'll create an instance
GlasslabSDK *glsdk;


int main( int argc, const char * argv[] )
{
    //
    // Declare temporary variables used for testing the SDK calls.
    //
    
    // These variables are required for making a connection to the server.
    // They include the URI, game Id, and location for the internal events
    // database.
    //char host[] = "http://stage.argubotacademy.org";
    //char host[] = "http://54.193.118.96";
    //char host[] = "http://192.168.1.101:8001";
    
    char host[] = "http://stage.playfully.org";
    //char host[] = "http://54.183.156.129";
    char gameId[] = "AA-1";
    
    // These variables are required for logging in and enrolling with a course.
    // Once a connection is successful, the server is open to accept login and
    // enroll requests
    //char username[] = "ben";
    //char password[] = "glasslab";
    char username[] = "annabellek";
    char password[] = "glasslab";
    char courseCode[] = "YW8XD";
    
    // The device Id is a way to uniquely identify a user with sessions and
    // associated events. It is used to distinguish users using the same device.
    char deviceId[ 256 ];
    sprintf( deviceId, "%s_test-device", username );
    
    // delay -> 1 sec = 1000 * 1000
    int mainLoopDelay = 100;
    int telemEventLoopDelay = 100;
    int numTelemEvents = 1;
    
    //
    // Create an instance of the GlassLab SDK and begin testing the SDK functions.
    //
    
    // A GlassLabSDK instnace requries a local database location to store queued
    // events to be dispatched to the server, a game identifier, the device Id, and
    // the URI to connect to.
    printf( "Basic: Creating SDK Instance connecting to %s\n", host );
    glsdk = new GlasslabSDK( gameId, deviceId, NULL, host );
    
    // Optionally set additional properties pertaining to the game, including the
    // name of the game, version number, and level.
    printf( "Setting game properties...\n" );
    glsdk->setName( "SimCityEDU" );
    glsdk->setVersion( "1.2.4156" );
    glsdk->setGameLevel( "Parktown" );
    
    //
    // The remainder of this program will iterate through each of the SDK calls and
    // print out the results. Errors will be captured with the "Message_Error" response.
    //
    
    int step = 0, resCode;
    string resString;
    while( true ) {
        
        // Get the next GlassLabSDK response object. This can be empty.
        // A response object contains the message, denoted by an enum, and
        // the response data as a JSON string.
        resCode   = glsdk->readTopMessageCode();
        resString = glsdk->readTopMessageString() || "";
        
        // Check the message type
        switch( resCode ) {
            //
            // The connect function can be called explicitly or can be triggered
            // when a new instance of the SDK is created. A successful connection
            // will trigger a "Message_Connect" message, otherwise "Message_Error".
            //
            case nsGlasslabSDK::Const::Message_Connect : {
                // Once we're connected, test login
                if( step == 0 ) {
                    printf( "** Logging in...\n" );
                    
                    nsGlasslabSDK::glConfig config;
                    config.eventsDetailLevel = 10;
                    config.eventsMaxSize     = 1000;
                    config.eventsMinSize     = 0;
                    config.eventsPeriodSecs  = 0;
                    glsdk->setConfig(config);
                    
                    glsdk->startGameTimer();
                    
                    glsdk->login( username, password, NULL );
                    step++;
                }
            } break;
            
            //
            // Login requires a username and a password in order to proceed. A
            // successful login will trigger a "Message_Login" message, otherwise
            // "Message_Error".
            //
            case nsGlasslabSDK::Const::Message_Login : {
                // Once we're logged in, test enrollment
                if( step == 1 ) {
                    printf( "** Start Enroll...\n" );
                    glsdk->enroll( courseCode );
                    glsdk->getUserInfo();
                    step++;
                }
            } break;
                
            //
            // Once we're logged in we can get user information. The return for this will
            // include the user Id, username, first name, last initial, type, and role. A
            // successful user info retrieval will trigger a "Message_GetUserInfo" message,
            // otherwise "Message_Error".
            //
            case nsGlasslabSDK::Const::Message_GetUserInfo : {
                // Print the user info returned
                printf( "** Get User Info: %s", resString.c_str() );
            } break;
                
            //
            // Enrollment requires a course code in string form. A course code is a
            // 5-character string containing letters and numbers. A successful enrollment
            // will trigger a "Message_Enroll" message, otherwise "Message_Error".
            //
            case nsGlasslabSDK::Const::Message_Enroll : {
                // Once we're enrolled, test course retrieval
                if( step == 2 ) {
                    printf( "** Get Courses...\n" );
                    glsdk->getCourses();
                    step++;
                }
            } break;
            
            //
            // Getting enrolled courses only requires that the user is logged in. Getting
            // the enrolled courses list otherwise will yield an error. The response data
            // will contain a JSON array of courses, including course Id, name, and grade
            // levels. A successful retrieval will trigger a "Message_GetCourses" message,
            // otherwise "Message_Error".
            //
            case nsGlasslabSDK::Const::Message_GetCourses : {
                // Once we have received our enrolled courses, test starting a new session
                if( step == 3 ) {
                    printf( "** Start Session...\n" );
                    glsdk->startSession();
                    step++;
                }
            } break;
                
            //
            // Starting a new session requires no additional information except that the
            // user is already logged in. The response data will contain the game Session
            // Id, which is used internally to associate telemetry events with. A successful
            // session start will trigger a "Message_StartSession" message, otherwise
            // "Message_Error".
            //
            case nsGlasslabSDK::Const::Message_StartSession : {
                // Once we've successfully established a new game session with the server,
                // test saving a new game state
                if( step == 4 ) {
                    printf( "** Saving Game...\n" );
                    glsdk->saveGame( "{\"a\":123,\"b\":4.31,\"c\":\"test\"}" );
                    step++;
                }
            } break;
                
            //
            // Saving the game state requires a JSON representation of the state information.
            // The server will not enforce the content of the save state but is responsible
            // for returning it upon request. A successful save will trigger a "Message_GameSave"
            // message, otherwise "Message_Error".
            //
            case nsGlasslabSDK::Const::Message_GameSave : {
                // Once we've saved the game state, test sending telemetry and achievements
                if( step == 5 ) {
                    for( int i = 0; i < numTelemEvents; i++ ) {
                        printf( "** Saving Event (%d)...\n", i );
                        
                        glsdk->addTelemEventValue( "string key", "asd" );
                        glsdk->addTelemEventValue( "int key", i );
                        glsdk->addTelemEventValue( "float key", i * 1.23 );
                        glsdk->saveTelemEvent( "GL_Scenario_Score" );
                        
                        //glsdk->saveAchievement( "Bot Champion", "CCSS.ELA-Literacy.WHST.6-8.1", "a" );
                        glsdk->saveAchievement( "Evidence Cadet", "CCSS.ELA-Literacy.WHST.6-8.1", "a" );
                        // NOT USED: glsdk->saveAchievement( "Core Cadet", "CCSS.ELA-Literacy.WHST.6-8.1", "b" );
                        //glsdk->saveAchievement( "Bot Defender", "CCSS.ELA-Literacy.WHST.6-8.1", "b" );
                        glsdk->saveAchievement( "Core Champion", "CCSS.ELA-Literacy.WHST.6-8.1", "b" );
                        
                        //glsdk->saveAchievement( "Deliberate", "21st.Century.Skills", "a" );
                        //glsdk->saveAchievement( "Bold", "21st.Century.Skills", "a" );
                        glsdk->saveAchievement( "Persistent", "21st.Century.Skills", "a" );
                        //glsdk->saveAchievement( "Curious", "21st.Century.Skills", "b" );
                        // NOT USED: glsdk->saveAchievement( "Empathetic", "21st.Century.Skills", "b" );
                        
                        // Sleep a short duration between telemetry events and achievements
                        //usleep( telemEventLoopDelay );
                    }
                    
                    step++;
                }
            } break;

            //
            // In order to send telemetry, the user must have started a new session. Telemetry
            // is only valid if it is called between start and end session APIs. Telemetry is
            // defined by any number of custom parameters, which can be any basic primitives,
            // and the name of the event. A successful telemetry request will trigger a "Message_Event"
            // message, otherwise "Message_Error".
            //
            case nsGlasslabSDK::Const::Message_Event : {
                // Once we've saved some telemetry, test ending the session
                if( step == 6 ) {
                    printf( "** End Session...\n" );

                    glsdk->endSession();
                    glsdk->stopGameTimer();
                    
                    step++;
                }
            } break;
                
            //
            // Ending a session requires a session to already be established to begin with. Once
            // a session is ended, no more additional telemetry can be captured until start session
            // is called again. A successful end session will trigger a "Message_EndSession" message,
            // otherwise "Message_Error".
            //
            case nsGlasslabSDK::Const::Message_EndSession : {
                // Once we've tested ending the session, simply delete the SDK
                // The simulation is complete.
                if( step == 7 ) {
                    printf( "** Get Save Game...\n" );
                    glsdk->getSaveGame();
                    step++;
                }
            } break;
                
            //
            // Saving the game state requires a JSON representation of the state information.
            // The server will not enforce the content of the save state but is responsible
            // for returning it upon request. A successful save will trigger a "Message_GameSave"
            // message, otherwise "Message_Error".
            //
            case nsGlasslabSDK::Const::Message_GetGameSave : {
                // Once we've saved the game state, test sending telemetry and achievements
                if( step == 8 ) {
                    printf( "Save game is: %s", resString.c_str() );
                    delete glsdk;
                    return 0;
                }
            } break;
                
            //
            // Any request to the server that results in failure will return a "Message_Error" message.
            // The response data will indicate the details of the error in JSON format.
            //
            case nsGlasslabSDK::Const::Message_Error : {
                //printf( "A request encountered an error: %s\n", response.m_data.c_str() );
            } break;
            
                
            // Do nothing
            default: break;
        }
        
        glsdk->popMessageStack();
        
        // Send any telemetry events that may be occupying the queue. The telemetry
        // stream could be empty, in which case nothing would happen.
        glsdk->sendTelemEvents();
        
        // Sleep a short duration between message extraction
        usleep( mainLoopDelay );
    }
    
    
    // End the main function
    return 0;
}
