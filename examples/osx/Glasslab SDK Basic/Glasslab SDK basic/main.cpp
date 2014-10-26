#include <iostream>
#include "glasslab_sdk.h"



//
// A reference to the GlassLabSDK object, of which we'll create an instance.
//
GlasslabSDK *glsdk;

//
// Declare configuration variables
//
char host[]         =   "http://developer.playfully.org";

char gameId[]       =   "TEST";
char gameName[]     =   "Test Game";
char gameVersion[]  =   "1.0";
char gameLevel[]    =   "Level 1";

char username[]     =   "andrew";
char password[]     =   "glasslab";
char courseCode[]   =   "GLTH2";
char deviceId[]     =   "andrew_device";



int sleepTick = 100;


//
// SDK Response thread.
//
void* sdkResponses( void* threadId )
{
    
    int resCode;
    string resString;
    
    
    // Main thread loop
    while( true )
    {
        
        resCode   = glsdk->readTopMessageCode();
        resString = glsdk->readTopMessageString() || "";
        
        // handle here...
        switch( resCode )
        {
            case nsGlasslabSDK::Const::Message_Connect:
            {
                printf( "RESPONSE: Message_Connect\n" );
                printf( "We're connected to %s!\n", host );
                
                
                glsdk->login( username, password, NULL );
            } break;
                
                
                //
                // Authentication...
                //
            case nsGlasslabSDK::Const::Message_Login:
            {
                printf( "RESPONSE: Message_Login\n" );
                printf( "%s has successfully logged in!\n", username );
                
                
                glsdk->startSession();
                
                for( int i = 0; i < 10; i++ ) {
                    glsdk->addTelemEventValue( "index", i );
                    glsdk->saveTelemEvent( "For_Loop_Iteration" );
                }
                
                glsdk->addTelemEventValue( "health", 100.0f );
                glsdk->addTelemEventValue( "lives", 3 );
                glsdk->addTelemEventValue( "avatarName", "Lazarus" );
                glsdk->saveTelemEvent( "Player_State" );
                
                glsdk->addTelemEventValue( "health", 50.0f );
                glsdk->addTelemEventValue( "lives", 2 );
                glsdk->addTelemEventValue( "avatarName", "Anathema" );
                glsdk->saveTelemEvent( "Opponent_State" );
                
                glsdk->saveTelemEvent( "Turn_Elapsed" );
            } break;
                
                
                
                //
                // Session Management...
                //
            case nsGlasslabSDK::Const::Message_StartSession:
            {
                printf( "RESPONSE: Message_StartSession\n" );
                printf( "%s has started a new game session!\n", username );
                
                
                glsdk->saveAchievement( "Evidence Cadet", "CCSS.ELA-Literacy.WHST.6-8.1", "a" );
                
                glsdk->saveGame( "{ \"test\": \"test\" }" );
                
                
                glsdk->endSession();
                
            } break;
                
            case nsGlasslabSDK::Const::Message_EndSession:
            {
                printf( "RESPONSE: Message_EndSession\n" );
                printf( "%s has ended the current game session!\n", username );
                
                return NULL;
            } break;
                
                
            case nsGlasslabSDK::Const::Message_SaveAchievement:
            {
                printf( "RESPONSE: Message_SaveAchievement\n" );
                printf( "Achievement logged...\n" );
            } break;
                
                
                
                
            case nsGlasslabSDK::Const::Message_Error:
            {
                printf( "ERROR: A request encountered an error: %s\n", resString.c_str() );
            } break;
        }
        
        
        glsdk->popMessageStack();
        
        
        
        glsdk->sendTelemEvents();
        
        
        
        // Sleep a short duration
        usleep( sleepTick );
    }
    
    // End the thread
    return NULL;
}


//
// Main thread.
//
int main( int argc, const char * argv[] )
{
    // Create the SDK
    glsdk = new GlasslabSDK( gameId, deviceId, NULL, host );
    glsdk->setGameSecret( "646b502aa4fded5a0c1e24552e241c06d36b7eb405ea9d63b20a4bbfa2bccb3c" );
    
    
    //
    // Create the SDK Response Thread to start listening for messages
    //
    pthread_t sdkResponseThread;
    long t;
    if( pthread_create( &sdkResponseThread, NULL, sdkResponses, (void*)t ) ) {
        fprintf( stderr, "There was an error creating the SDK response thread.\n" );
        return 1;
    }
    
    
    //
    // Wait for the SDK Response Thread to complete
    //
    if( pthread_join( sdkResponseThread, NULL ) ) {
        fprintf( stderr, "There was an error joining the SDK response thread.\n" );
        return 2;
    }
    
    // Exit the program
    return 0;
}