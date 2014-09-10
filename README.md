GlassLab SDK cpp (iOS, OSX, Windows)
====================================

This iOS, OSX, and Windows compatible GlassLab SDK allows games and other applications to connect to the GlassLab Game Services (GLGS) platform and perform certain operations. The primary purpose of integrating this library into your project is to track game sessions and store telemetry for those sessions.

This package includes the libGlasslabSDK.a library file for iOS, the GlasslabSDK_x86_64.dylib library file for OSX, the GlassLabSDK.lib library file for Windows, source code, and some examples on how to use it.

Libraries used:
- libevent
- libjansson
- libsqlite


Integration
-----------

To integrate the SDK into your source, simply import the .a (iOS), .dylib (OSX), or .lib (Windows) into your source and include "glasslab_sdk.h". This header file is the high-level API into every SDK function your program will need. See below on creating an instance of the SDK and using its functions.


###C-Sharp Wrapper

Included in this project is a sample C# wrapper to the SDK that imports all necessary functions in the core library. The "GlasslabSDK.cs" wrapper can be found in ROOT/platform-support/unity/. If you are using Unity3D, include this wrapper and library in the "Plugins" directory of your Unity project.


Establish a connection
----------------------

The first thing you will need to do before recording any sessions or telemetry is to establish a connection to the server. A few parameters are required for a successful connection, including:
- internal database location
- game identifier
- device identifier
- server URI

The internal database location is used to store session and telemetry requests. These requests are dispatched in bulk rather than on demand in order to reduce server load. More on this below.

The game identifier is a simple string used to distinguish your game. Game identifiers are provided to developers when they register for GlassLab Game Services. The device identifier is an identifer sent with sessions and telmetry to distinguish their origin. See the Authentication section below for more information. Finally, the URI denotes the server you wish to connect to.

Should the connection be successful, you will receive a "Message_Connect" response, otherwise a "Message_Error" will be returned. The next section describes how to intercept this message in your game code.

There are two ways to make a connection to the server:
- Create a new instance of the GlassLabSDK
- Call the connect function directly

Note: if you are utilizing the C# wrapper, the instantiation and connect API calls are wrapped and exposed appropriately.


Dispatch and Response Messages
------------------------------

One of the SDK's primary functions is to maintain the communication channel between game client and server. In order to reduce potential server load, the SDK maintains a queue of messages that are dispatched at a defined interval. The server's response to these messages is generally fast but also unpredictable. Once the SDK intercepts the response, they will be placed in a response queue where they will remain until they are popped by a calling function. It is the client's responsibility to pop from this response queue to receive the response information, though it is not required.


###Dispatching Messages

Most server requests are dispatched to the server immediately. These server requests are made internally by the SDK for a subset of the exposed functions, including:
- connect
- authStatus
- login
- enroll
- logout
- getSaveGame
- deleteSaveGame

The SDK will intercept the server responses and push them into the response queue.

The following server requests are stored in an internal dispatch queue and fired at a defined interval:
- startSession
- endSession
- saveTelemEvent
- saveAchievementEvent
- saveGame

These dispatches are stored internally before sending to account for potential hiccups in internet connectivity. Should there be a sudden lapse in internet connection, these messages will simply remain in the queue until a connection is re-established, preserving the content. This is especially important for games that don't require online play. As such, you should not expect an immediate server response to these messsages.


###Intercepting Server Responses

All server requests will return with a response. These response messages are pushed to an internal queue and can be retrieved with the following calling function:

```
// Get the next response message (C++)
nsGlasslabSDK::Const::Response response = *glsdk->popMessageStack();
```

The response message contains two pieces of data:
- response type, as an enum
- response data, in JSON format


###Example

The following demonstrates an example of how to wrap the server response retrieval:

```
while( true ) {
        
    // Get the next GlassLabSDK response object. This can be empty.
    // A response object contains the message, denoted by an enum, and
    // the response data as a JSON string.
    nsGlasslabSDK::Const::Response response = *glsdk->popMessageStack();
    
    // Check the message type
    switch( response.m_message ) {

        case nsGlasslabSDK::Const::Message_Connect : {
        	// Run code for successful connection
        } break;

        case nsGlasslabSDK::Const::Message_Login : {
        	// Run code for successful login
        } break;

        case nsGlasslabSDK::Const::Message_Enroll : {
        	// Run code for successful enrollment
        } break;
        
        case nsGlasslabSDK::Const::Message_GetCourses : {
        	// Run code for successful course retrieval
        } break;

        case nsGlasslabSDK::Const::Message_StartSession : {
        	// Run code for successful session start
        } break;

        case nsGlasslabSDK::Const::Message_Event : {
        	// Run code for successful event record
        } break;
            
        case nsGlasslabSDK::Const::Message_EndSession : {
        	// Run code for successful end session
        } break;

        case nsGlasslabSDK::Const::Message_Error : {
        	// Intecept errors and respond accordingly
        } break;
        
        default: break;
    }
    
    // Send any telemetry events that may be occupying the queue. The telemetry
    // stream could be empty, in which case nothing would happen.
    glsdk->sendTelemEvents();
    
    // Sleep a short duration between message extraction
    usleep( 2000 );
}
```

It is a good practice to run this on a separate thread so your program is free to run the game code without any blocking calls. Also, note the "sendTelemEvents" function call that occurs at the end. This is necessary for flushing the stored data queue, that is the queue containing all start session, end session, and telemetry calls.


API Format and Examples
-----------------------

The GlassLabSDK exposes many functions that communicate with the server to perform some operation, whether it is managing sessions, recording data, or enrolling a student in a course. The table below details the functions that are exposed, the information required for dispatch, and what will be returned with the server response.

| SDK Function | Purpose | Response Message |
| ------------ | ------- | ---------------- |
| connect(gameId, uri) | establish a connection to the server | Message_Connect |
| authStatus() | check the authentication status for a given user | Message_AuthStatus |
| login(username, password) | attempt to log a user into the system | Message_Login |
| logout() | attempt to log the current authenticated user out | Message_Logout |
| enroll(courseCode) | attempt to enroll the logged in user to a course denoted by a 5-character code | Message_Enroll |
| getUserInfo() | retrieve user information for the current authenticated user | Message_GetUserInfo |
| getPlayerInfo() | retrieve player information for the current authenticated user, including total time played | Message_GetPlayerInfo |
| getCourses() | retrieve a list of enrolled courses for the current authenticated user | Message_GetCourses |
| startSession() | attempt to start a new session for gathering telemetry | Message_StartSession |
| endSession() | attempt to end the current session | Message_EndSession |
| saveTelemEvent(eventName) | record a new telemetry event with previously appended data | Message_Event |
| saveAchievementEvent(item, group, subgroup) | record a new achievement | Message_SaveAchievement |
| saveGame(gameData) | records a JSON-formatted save game blob | Message_GameSave |
| getSaveGame() | retrieves the save game for the current user | Message_GetGameSave |
| deleteSaveGame() | deletes the save game record for the current user | Message_DeleteGameSave |

The above repsonse messages assume a valid and successful request. If the request was unsuccessful, which could either be due to internet connection state or invalid data, the server will respond with "Message_Error" and attach an error message indicating the failure.

Detailed below are examples of how to use some of the main SDK functions, including starting and ending sessions, sending telemetry, and sending achievements.


###Session Management

A game session Id is required for sending telemetry events and can only be obtained by a successful start session request. A session is typically used to describe a specific activity in the game, but there are no restrictions on how to use it. Here is how you can start a new session:

```
// Start a new session
SDK->startSession();
```

To end a session, and thus reset the game session Id, you can call:

```
// End the current session
SDK->endSession();
```

Note: you can only have one session active at a time per device Id. As explained above, device Ids are used to identify a single user within the game or app. Typically, the user's name is prepended to an identifier that defines the device being used.


###Authentication

User authentication is not required for managing sessions and sending telemetry. The session and telemetry information are identified by the deviceId that is set at the creation of the SDK instance. Usually, the deviceId is the unique identifier of the device being used. This Id is how we identify the origin of the events. However, user authentication is required for playfully.org student reporting. Services like achievements, SOWO (shout out, watch out), total time played, and game saves will only succeed if the user is authenticated. The SDK will report these failures back to the client but will not halt the app in any way.

A common conflict with using just the device's unique identifier is shared data: a game that supports multiple saves, and thus multiple users, will share that device Id, session info, and telemetry. A good practice to avoid this is to prepend the device identifier with a user-identifying value, which can be accessed from successful authentication.

There are two API calls used for authentication-related services:
- login( username, password )
- logout()

The login API function requires the username and password as its parameters:

```
// Perform login with the following credentials
char username[] = "ben";
char password[] = "glasslab";
SDK->login( username, password );
```

Once the request has been routed to the server and returned as successful, the "nsGlasslabSDK::Const::Message_Login" message will be waiting in the response queue. The response data is a JSON object containing the following important fields:
- id
- username
- last name initial
- first name
- email
- role
- enrolled course list

As mentioned above, it is a good practice to use the "id" field returned on successful login with the deviceId for sessions and telemetry.

<b>Login and registration modals:</b>

Registration and login generally require modals. As a developer, you can either build these UI modules into the game yourself or you can use web view technology and link to modals we have created on the playfully.org site.

The login webview page can be found [here](http://developer.playfully.org/sdk/login/ "Login.org Registration Webview"). The registration webview page can be found [here](http://developer.playfully.org/sdk/login/ "Playfully.org Registration Webview").


###Telemetry

Adding custom telemetry to the server message queue is fairly straightforward and allows for some flexibility. Telemetry events adhere to a specific data schema that the SDK will construct automatically. When creating a new telemetry event to be sent to the system, the user specifies the name of the event and as many custom parameters as necessary. Telemetry events can be as  simple as triggers, containing no custom paramters:
- "Player_jump" : {}
 
Or they can be more descriptive and reusable:
- "Player_take_damage" : { amount : "10" }

The SDK functions required to write telemetry events are as follows:
- addTelemEventValue( key, value )
- saveTelemEvent( eventName )

The "addTelemEventValue" functions allow for custom parameters to be sent with each telemetry event and are therefore not required. In the above example, we created a parameter keyed as "amount" with the value "10". It is important to note that the telemetry parameters must be added before a telemetry event is saved. These parameters are simply stored locally until "saveTelemEvent( eventName )" is called, at which point they are appended to the event named "eventName" and then reset.

The code below demonstrates how we can write the aforementioned telemetry examples:

```
// Send the "Player_take_damage" event with amount parameter
SDK->addTelemEventValue( "amount", 10 );
SDK->saveTelemEvent( "Player_take_damage" );

// Send the "Player_jump" event
SDK->saveTelemEvent( "Player_jump" );
```

Note that the parameter "amount" with value "10" will not be sent along with the "Player_jump" event because it was flushed after the "Player_take_damage" event was saved.


###Achievements

Achievements are one piece of the reporting on GlassLab Game Services. This information will come from the game and all logic governing achievements must be defined there, but the SDK can be used to send recorded achievements to the server. They go through the same pipeline as telemetry and can be triggered with the "saveAchievement" function. The SDK requires three unique parameters for each achievement:
- item (the name of the achievement)
- group (the primary standard this achievement is associated with)
- subgroup (the standard sub-type)

The developer is responsible for defining the achievements and must register them through the developer portal with their game. Successful registration will return the appropriate item, group, and subGroup values that can be used in-game and that the server will accept as valid.

The sample API has an example achievement defined that the server currently accepts for GlassLab's Mars Generation One iPad game. It is written as such:

```
// The player completed 10 argument battles with his/her opponent, trigger the achievement
SDK->saveAchievementEvent( "Core Cadet", "CCSS.ELA-Literacy.WHST.6-8.1", "b" );
```


###Game Saves

You can also record and access game saves per user with two simple functions:
- getSaveGame()
- postSaveGame( const char* data )

Note that you must be authenticated with the server in order to get and send save games.


Sample Projects and Wrapper
---------------------------

A sample OSX project is included to demonstrate how to use the SDK in C++ and Xcode. This sample utilizes the majority of the SDK functions described in this document and implements a simple listener to intercept server responses. This project can be found at ROOT/examples/osx/Glasslab SDK Basic/. There is also a Windows sample using Visual Studio, which mocks the same functionality as the OSX project.

A sample Unity project is also included to demonstrate how to use the SDK in C#. This sample can be found at ROOT/examples/unity/. The C# wrapper, which is required for the Unity sample, can be found at ROOT/platform-support/unity/.