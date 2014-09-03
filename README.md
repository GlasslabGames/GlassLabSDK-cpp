GlassLab SDK cpp (iOS, OSX, Windows)
====================================

This iOS/OSX-compatible GlassLab SDK allows games and other applications to connect to the GlassLab Games Services (GLGS) platform and perform certain operations.. The primary purpose of integrating this library into your project is to track game sessions and store telemetry for those sessions.

This package includes the libGlasslabSDK.a library file for iOS, the GlasslabSDK_x86_64.dylib library file for OSX, source code, and some examples on how to use it.

Libraries used:
- libevent
- libjansson
- libsqlite


Integration
-----------

To integrate the SDK into your source, simply import the .dylib (OSX) or .a (iOS) into your source and include "glasslab_sdk.h". This header file is the high-level API into every SDK function your program will need. See below on creating an instance of the SDK and using its functions.


###C-Sharp Wrapper

Included in this project is a sample C# wrapper to the SDK that imports all necessary functions in the core library. The "GlasslabSDK.cs" wrapper can be found in ROOT/platforms/unity/. If you are using Unity3D, include this wrapper and library in the "Plugins" directory of your Unity project.


Establish a connection
----------------------

The first thing you will need to do before recording any sessions or telemetry is to establish a connection to the server. A few parameters are required for a successful connection, including:
- internal database location
- game identifier
- device identifier
- URI

The internal database location is used to store session and telemetry requests. These requests are dispatched in bulk rather than on demand in order to reduce server load. More on this below.

The game identifier is a simple string used to define your game. A short code or abbreviation will suffice. The device identifier is used to distinguish players using the same device (useful for games that support multiple save files). Finally, the URI denotes the server you wish to connect to.

Should the connection be successful, you will receive a "Message_Connect" response, otherwise a "Message_Error" will be returned. The next section describes how to intercept this message in your game code.

There are two ways to make a connection to the server:
- Create a new instance of the GlassLabSDK
- Call the connect function directly

Note: if you are utilizing the C# wrapper, the instantiation and connect API calls are wrapped and exposed appropriately.


Dispatch and Response Messages
------------------------------

One of the SDK's primary functions is to maintain the communication channel between game client and server. In order to reduce potential server load, the SDK maintains a queue of messages that are dispatched at a defined interval. The server's response to these messages is generally fast but also unpredictable. Once the SDK intercepts the response, they will be placed in a response queue where they will remain until they are popped by a calling function. It is the client's responsibility to pop this response queue to receive the response information, though it is not required.


###Dispatching Messages

Most server requests are dispatched to the server immediately. These server requests are made internally by the SDK for a subset of the exposed functions, including:
- connect
- authStatus
- login
- enroll
- logout

The SDK will intercept the server responses and push them into the response queue.

The following server requests are stored in an internal dispatch queue and fired at a defined interval:
- startSession
- endSession
- saveTelemEvent
- saveAchievementEvent

These dispatches are stored internally before sending to account for potential hiccups in internet connectivity. Should there be a sudden lapse in internet connection, these messages will simply remain in the queue until a connection is re-establishing, preserving the content. This is especially important for games that don't require online play. As such, you should not expect an immediate server response to these messsages.


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
| connect(gameId, uri) | establish a connection to the server | MESSAGE_CONNECT |
| authStatus() | check the authentication status for a given user | MESSAGE_AUTHSTATUS |
| login(username, password) | attempt to log a user into the system | MESSAGE_LOGIN |
| enroll(courseCode) | attempt to enroll the logged in user to a course denoted by a 5-character code | MESSAGE_ENROLL |
| getCourses() | retrieve a list of enrolled courses for the current authenticated user | MESSAGE_GET_COURSES |
| logout() | attempt to log the current authenticated user out | MESSAGE_LOGOUT |
| startSession() | attempt to start a new session for the authenticated user | MESSAGE_STARTSESSION |
| endSession() | attempt to end the current session | MESSAGE_ENDSESSION |
| saveTelemEvent(eventName) | record a new telemetry event with previously appended data | MESSAGE_EVENT |
| saveAchievementEvent(item, group, subgroup) | record a new achievement | N/A |

The above repsonse messages assume a valid and successful request. If the request was unsuccessful, which could either be due to internet connection state or invalid data, the server will respond with "MESSAGE_ERROR".

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

Note: you can only have one session active at a time per device Id. As explained above, device Ids are used to identify a single user within the game or app. Typically, the users name is prepended to an identifier that defines the device being used.


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

If your game supports GLGS-linked achievements, you can send achievements via the SDK using the following function:

```
// The player completed 10 argument battles with his/her opponent, trigger the achievement
SDK->saveAchievementEvent( "Battle Master", CCSS.ELA-Literacy.WHST.6-8.1", "a" );
```

Three parameters are required for achievements, they include:
- item (the name of the achievement)
- group (the primary standard this achievement is associated with)
- subgroup (the standard sub-type)

Each of these three parameters must correspond to entries in the server in order for them to be recognized and captured. Please coordinate with a GlassLab representative on this.


Sample Projects and Wrapper
---------------------------

A sample OSX project is included to demonstrate how to use the SDK in C++ and Xcode. This sample utilizes the majority of the SDK functions described in this document and implements a simple listener to intercept server responses. This project can be found at ROOT/examples/osx/Glasslab SDK Basic/.

A sample Unity project is also included to demonstrate how to use the SDK in C#. This sample can be found at ROOT/examples/unity/. The C# wrapper, which is required for the Unity sample, can be found at ROOT/platforms/unity/.