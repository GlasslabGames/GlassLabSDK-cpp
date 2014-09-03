using UnityEngine;
using System;
using System.Collections;
using System.Threading;
using System.Runtime.InteropServices;


public class GlasslabSDK {

	// ----------------------------
	/**
	 * Singleton for easy access.
	 */
	private static volatile GlasslabSDK mInstance;
	private static object syncRoot = new System.Object();
	
	public static GlasslabSDK Instance {
		get {
			if (mInstance == null) {
				lock (syncRoot) {
					if (mInstance == null) 
						mInstance = new GlasslabSDK();
				}
			}
			
			return mInstance; 
		}
	}
	
	// ----------------------------
	/**
	 * Member variables:
	 * - unmanaged instance pointer
	 * - update thread for handling SDK requests
	 * - callback function lists
	 * - message retrieval fields
	 */
	private System.IntPtr mInst;
	private bool mInstSet;

	private Thread mLoop;

	private ArrayList mConnect_CBList;
	private ArrayList mDeviceUpdate_CBList;
	private ArrayList mAuthStatus_CBList;
	private ArrayList mRegister_CBList;
	private ArrayList mLogin_CBList;
	private ArrayList mLogout_CBList;
	private ArrayList mEnroll_CBList;
	private ArrayList mUnenroll_CBList;
	private ArrayList mGetCourses_CBList;
	private ArrayList mStartSession_CBList;
	private ArrayList mEndSession_CBList;
	private ArrayList m_GameSave_CBList;
	private ArrayList m_GetGameSave_CBList;
	private ArrayList m_DeleteGameSave_CBList;
	private ArrayList m_GetUserInfo_CBList;

	private char[]    mMsgChars;
	private string    mMsgString;
	private int       mMsgCode;
	
	/**
	 * A message is appended to every SDK response indicating the type.
	 */
	public enum Message {
		None = 0,
		Connect,
		DeviceUpdate,
		AuthStatus,
		Register,
		Login,
		Logout,
		Enroll,
		Unenroll,
		GetCourses,
		StartSession,
		EndSession,
		GameSave,
		GetGameSave,
		DeleteGameSave,
		GetUserInfo,
		Event,
		Error
	};
	
	/**
	 * A status is appended to every SDK response indicating success or failure.
	 */
	public enum Status {
		Ok = 0,
		Error
	};
	
	/**
	 * The generic Response struct contains the message and data response. These
	 * are returned with each SDK callback.
	 */
	public struct Response {
		public Message m_message;
		public string m_data;
	};
	
	/**
	 * Define a generic Response Callback function containing response data. All
	 * SDK callbacks adhere to this.
	 */
	public delegate void ResponseCallback( string response = "" );
	public void ResponseCallback_Stub( string response = "" ) {}

	// ----------------------------
	/**
	 * The constructor initializes all callback lists and spawns a separate thread
	 * to handle communication between game and SDK/server.
	 */
	private GlasslabSDK() {
		mConnect_CBList 	 = new ArrayList();
		mDeviceUpdate_CBList = new ArrayList();
		mAuthStatus_CBList   = new ArrayList();
		mRegister_CBList     = new ArrayList();
		mLogin_CBList        = new ArrayList();
		mLogout_CBList       = new ArrayList();
		mEnroll_CBList       = new ArrayList();
		mUnenroll_CBList     = new ArrayList();
		mGetCourses_CBList   = new ArrayList();
		mStartSession_CBList = new ArrayList();
		mEndSession_CBList   = new ArrayList();
		m_GameSave_CBList 	 = new ArrayList();
		m_GetGameSave_CBList = new ArrayList();
		m_DeleteGameSave_CBList = new ArrayList();
		m_GetUserInfo_CBList = new ArrayList();
		mInstSet = false;
		
		mMsgCode   = 0;
		mMsgChars  = new char[1024];
		for(int i = 0; i < mMsgChars.Length; i++) {
			mMsgChars[i] = '-';
		}
		
		// Spawn the thread
		mLoop = new Thread( UpdateLoop );
		mLoop.Start ();
	}

	/**
	 * Simple cleanup.
	 */
	~GlasslabSDK(){
		GlasslabSDK_FreeInstance (mInst);
		mInstSet = false;
	}
	
	// ----------------------------
	/**
	 * Basic Connect function is used to create the SDK instance and attempt to the connect
	 * to the server. The dataPath is necessary for identifying where on the system queued
	 * up telemetry data should be stored. The clientId is a simple key used to identify the
	 * game and associated data. The URI indicates the server we should be connecting to.
	 * Optionally, you can specify a function to be called when the request is successful.
	 */
	public void Connect( string dataPath, string clientId, string uri = "", ResponseCallback cb = null ) {
		string deviceUUID = SystemInfo.deviceUniqueIdentifier;
		
		if (cb != null) {
			mConnect_CBList.Add (cb);
		} else {
			ResponseCallback tempCB = ResponseCallback_Stub;
			mConnect_CBList.Add (tempCB);
		}
		
		if( mInstSet ) {
			GlasslabSDK_Connect( mInst, clientId, uri );
		}
		else {
			mInst = GlasslabSDK_CreateInstance (clientId, deviceUUID, dataPath, uri);
			mInstSet = true;
			
			iPhone.SetNoBackupFlag( dataPath + "/glasslabsdk.db" );
		}
	}
	
	// ----------------------------
	/**
	 * The UpdateLoop function runs on a separate thread and intercepts all server responses
	 * that come through the SDK. Each response has a message code, defined by the Message
	 * enum above, and a JSON string resonse.
	 */
	private void UpdateLoop() {
		while(true) {
			
			// Get the message and response information
			mMsgCode = GlasslabSDK_ReadTopMessageCode (mInst);
			IntPtr responsePtr = GlasslabSDK_ReadTopMessageString (mInst);
			mMsgString = System.Runtime.InteropServices.Marshal.PtrToStringAuto( responsePtr );
			
			// Intercept the responses and fire the desired callback functions.
			switch(mMsgCode){
			case (int)GlasslabSDK.Message.Connect: {
				if(mConnect_CBList.Count > 0){
					ResponseCallback cb = (ResponseCallback)mConnect_CBList[0];
					mConnect_CBList.RemoveAt (0);
					cb( mMsgString );
				}
			} break;
				
			case (int)GlasslabSDK.Message.DeviceUpdate: {
				if(mDeviceUpdate_CBList.Count > 0){
					ResponseCallback cb = (ResponseCallback)mDeviceUpdate_CBList[0];
					mDeviceUpdate_CBList.RemoveAt (0);
					cb();
				}
			} break;
				
			case (int)GlasslabSDK.Message.AuthStatus: {
				if(mAuthStatus_CBList.Count > 0){
					ResponseCallback cb = (ResponseCallback)mAuthStatus_CBList[0];
					mAuthStatus_CBList.RemoveAt (0);
					cb( mMsgString );
				}
			} break;
				
			case (int)GlasslabSDK.Message.Register: {
				if(mRegister_CBList.Count > 0){
					ResponseCallback cb = (ResponseCallback)mRegister_CBList[0];
					mRegister_CBList.RemoveAt (0);
					cb();
				}
			} break;
				
			case (int)GlasslabSDK.Message.Login: {
				if(mLogin_CBList.Count > 0){
					ResponseCallback cb = (ResponseCallback)mLogin_CBList[0];
					mLogin_CBList.RemoveAt (0);
					cb( mMsgString );
				}
			} break;
				
			case (int)GlasslabSDK.Message.Logout: {
				if(mLogout_CBList.Count > 0){
					ResponseCallback cb = (ResponseCallback)mLogout_CBList[0];
					mLogout_CBList.RemoveAt (0);
					cb();
				}
			} break;
				
			case (int)GlasslabSDK.Message.Enroll: {
				if(mEnroll_CBList.Count > 0){
					ResponseCallback cb = (ResponseCallback)mEnroll_CBList[0];
					mEnroll_CBList.RemoveAt (0);
					cb( mMsgString );
				}
			} break;
				
			case (int)GlasslabSDK.Message.Unenroll: {
				if(mUnenroll_CBList.Count > 0){
					ResponseCallback cb = (ResponseCallback)mUnenroll_CBList[0];
					mUnenroll_CBList.RemoveAt (0);
					cb();
				}
			} break;
				
			case (int)GlasslabSDK.Message.GetCourses: {
				if(mGetCourses_CBList.Count > 0){
					ResponseCallback cb = (ResponseCallback)mGetCourses_CBList[0];
					mGetCourses_CBList.RemoveAt (0);
					cb( mMsgString );
				}
			} break;
				
			case (int)GlasslabSDK.Message.StartSession: {
				if(mStartSession_CBList.Count > 0){
					ResponseCallback cb = (ResponseCallback)mStartSession_CBList[0];
					mStartSession_CBList.RemoveAt (0);
					cb();
				}
			} break;
				
			case (int)GlasslabSDK.Message.EndSession: {
				if(mEndSession_CBList.Count > 0){
					ResponseCallback cb = (ResponseCallback)mEndSession_CBList[0];
					mEndSession_CBList.RemoveAt (0);
					cb();
				}
			} break;
				
			case (int)GlasslabSDK.Message.GameSave: {
				if(m_GameSave_CBList.Count > 0){
					ResponseCallback cb = (ResponseCallback)m_GameSave_CBList[0];
					m_GameSave_CBList.RemoveAt (0);
					cb();
				}
			} break;
				
			case (int)GlasslabSDK.Message.GetGameSave: {
				if(m_GetGameSave_CBList.Count > 0){
					ResponseCallback cb = (ResponseCallback)m_GetGameSave_CBList[0];
					m_GetGameSave_CBList.RemoveAt (0);
					cb( mMsgString );
				}
			} break;
				
			case (int)GlasslabSDK.Message.DeleteGameSave: {
				if(m_DeleteGameSave_CBList.Count > 0){
					ResponseCallback cb = (ResponseCallback)m_DeleteGameSave_CBList[0];
					m_DeleteGameSave_CBList.RemoveAt (0);
					cb( mMsgString );
				}
			} break;
				
			case (int)GlasslabSDK.Message.GetUserInfo: {
				if(m_GetUserInfo_CBList.Count > 0){
					ResponseCallback cb = (ResponseCallback)m_GetUserInfo_CBList[0];
					m_GetUserInfo_CBList.RemoveAt (0);
					cb( mMsgString );
				}
			} break;
				
			// do nothing
			default: break;
			}
			
			// Always pop the message stack because we're done with this message.
			GlasslabSDK_PopMessageStack (mInst);

			// This can happen at a certain interval but we always attempt to send our
			// telemetry to the server each loop. The SDK has throttling parameters that
			// control when that information is sent, including min event count, max event
			// count, and interval.
			GlasslabSDK_SendTelemEvents( mInst );
			
			// Sleep the thread for a short duration before firing again.
			Thread.Sleep( 100 );
		}
	}
	
	// ----------------------------
	/**
	 * Public functions for making requests to the server and receiving response callbacks.
	 * All functions below are setup to receive a callback, if requested. These callbacks
	 * are triggered in the UpdateLoop thread function above.
	 */
	public void DeviceUpdate(ResponseCallback cb = null) {
		if (cb != null) {
			mDeviceUpdate_CBList.Add (cb);
		} else {
			ResponseCallback tempCB = ResponseCallback_Stub;
			mDeviceUpdate_CBList.Add (tempCB);
		}
		
		GlasslabSDK_DeviceUpdate (mInst);
	}

	public void AuthStatus(ResponseCallback cb = null) {
		if (cb != null) {
			mAuthStatus_CBList.Add (cb);
		} else {
			ResponseCallback tempCB = ResponseCallback_Stub;
			mAuthStatus_CBList.Add (tempCB);
		}
		
		GlasslabSDK_AuthStatus (mInst);
	}

	public void RegisterStudent(string username, string password, string firstName, string lastInitial, ResponseCallback cb = null) {
		if (cb != null) {
			mRegister_CBList.Add (cb);
		} else {
			ResponseCallback tempCB = ResponseCallback_Stub;
			mRegister_CBList.Add (tempCB);
		}
		
		GlasslabSDK_RegisterStudent (mInst, username, password, firstName, lastInitial);
	}

	public void RegisterInstructor(string name, string email, string password, bool newsletter, ResponseCallback cb = null) {
		if (cb != null) {
			mRegister_CBList.Add (cb);
		} else {
			ResponseCallback tempCB = ResponseCallback_Stub;
			mRegister_CBList.Add (tempCB);
		}
		
		GlasslabSDK_RegisterInstructor (mInst, name, email, password, newsletter);
	}
	
	public void Login(string username, string password, string type = null, ResponseCallback cb = null) {
		if (cb != null) {
			mLogin_CBList.Add (cb);
		} else {
			ResponseCallback tempCB = ResponseCallback_Stub;
			mLogin_CBList.Add (tempCB);
		}
		
		GlasslabSDK_Login (mInst, username, password, type);
	}

	public void Login(string username, string password, ResponseCallback cb) {
		this.Login (username, password, null, cb);
	}
	
	public void Logout(ResponseCallback cb = null) {
		if (cb != null) {
			mLogout_CBList.Add (cb);
		} else {
			ResponseCallback tempCB = ResponseCallback_Stub;
			mLogout_CBList.Add (tempCB);
		}
		
		GlasslabSDK_Logout (mInst);
	}
	
	public void Enroll(string courseCode, ResponseCallback cb = null) {
		if (cb != null) {
			mEnroll_CBList.Add (cb);
		} else {
			ResponseCallback tempCB = ResponseCallback_Stub;
			mEnroll_CBList.Add (tempCB);
		}
		
		GlasslabSDK_Enroll (mInst, courseCode);
	}
	
	public void Unenroll(string courseId, ResponseCallback cb = null) {
		if (cb != null) {
			mUnenroll_CBList.Add (cb);
		} else {
			ResponseCallback tempCB = ResponseCallback_Stub;
			mUnenroll_CBList.Add (tempCB);
		}
		
		GlasslabSDK_UnEnroll (mInst, courseId);
	}
	
	public void GetCourses(ResponseCallback cb = null) {
		if (cb != null) {
			mGetCourses_CBList.Add (cb);
		} else {
			ResponseCallback tempCB = ResponseCallback_Stub;
			mGetCourses_CBList.Add (tempCB);
		}
		
		GlasslabSDK_GetCourses (mInst);
	}
	
	public void StartSession(ResponseCallback cb = null) {
		if (cb != null) {
			mStartSession_CBList.Add (cb);
		} else {
			ResponseCallback tempCB = ResponseCallback_Stub;
			mStartSession_CBList.Add (tempCB);
		}
		
		GlasslabSDK_StartSession (mInst);
	}
	
	public void EndSession(ResponseCallback cb = null) {
		if (cb != null) {
			mEndSession_CBList.Add (cb);
		} else {
			ResponseCallback tempCB = ResponseCallback_Stub;
			mEndSession_CBList.Add (tempCB);
		}
		
		GlasslabSDK_EndSession (mInst);
	}
	
	public void CancelRequest(string key) {
		GlasslabSDK_CancelRequest( mInst, key );
	}
	
	public void SaveGame( string gameData, ResponseCallback cb = null ) {
		if (cb != null) {
			m_GameSave_CBList.Add (cb);
		} else {
			ResponseCallback tempCB = ResponseCallback_Stub;
			m_GameSave_CBList.Add (tempCB);
		}
		
		GlasslabSDK_SaveGame( mInst, gameData );
	}
	
	public void GetSaveGame(ResponseCallback cb = null) {
		if (cb != null) {
			m_GetGameSave_CBList.Add (cb);
		} else {
			ResponseCallback tempCB = ResponseCallback_Stub;
			m_GetGameSave_CBList.Add (tempCB);
		}
		
		GlasslabSDK_GetSaveGame( mInst );
	}
	
	public void DeleteSaveGame(ResponseCallback cb = null) {
		if (cb != null) {
			m_DeleteGameSave_CBList.Add (cb);
		} else {
			ResponseCallback tempCB = ResponseCallback_Stub;
			m_DeleteGameSave_CBList.Add (tempCB);
		}
		
		GlasslabSDK_DeleteSaveGame( mInst );
	}
	
	public void GetUserInfo(ResponseCallback cb = null) {
		if (cb != null) {
			m_GetUserInfo_CBList.Add (cb);
		} else {
			ResponseCallback tempCB = ResponseCallback_Stub;
			m_GetUserInfo_CBList.Add (tempCB);
		}
		
		GlasslabSDK_GetUserInfo( mInst );
	}
	
	// ----------------------------
	/**
	 * Public functions for saving telemetry and achievement information in the SDK to be sent
	 * on the next SendTelemEvents call.
	 */
	public void AddTelemEventValue(string key, string value) {
		GlasslabSDK_AddTelemEventValue_ccp   (mInst, key, value);
	}
	public void AddTelemEventValue(string key, sbyte  value) {
		GlasslabSDK_AddTelemEventValue_int8  (mInst, key, value);
	}
	public void AddTelemEventValue(string key, short  value) {
		GlasslabSDK_AddTelemEventValue_int16 (mInst, key, value);
	}
	public void AddTelemEventValue(string key, int    value) {
		GlasslabSDK_AddTelemEventValue_int32 (mInst, key, value);
	}
	public void AddTelemEventValue(string key, byte   value) {
		GlasslabSDK_AddTelemEventValue_uint8 (mInst, key, value);
	}
	public void AddTelemEventValue(string key, ushort value) {
		GlasslabSDK_AddTelemEventValue_uint16(mInst, key, value);
	}
	public void AddTelemEventValue(string key, uint   value) {
		GlasslabSDK_AddTelemEventValue_uint32(mInst, key, value);
	}
	public void AddTelemEventValue(string key, float  value) {
		GlasslabSDK_AddTelemEventValue_float (mInst, key, value);
	}
	public void AddTelemEventValue(string key, double value) {
		GlasslabSDK_AddTelemEventValue_double(mInst, key, value);
	}
	public void AddTelemEventValue(string key, bool value) {
		GlasslabSDK_AddTelemEventValue_bool(mInst, key, value);
	}
	public void ClearTelemEventValues() {
		GlasslabSDK_ClearTelemEventValues (mInst);
	}
	public void SaveTelemEvent(string name) {
		GlasslabSDK_SaveTelemEvent (mInst, name);
	}
	public void SaveAchievement( string item, string group, string subGroup ) {
		GlasslabSDK_SaveAchievement(mInst, item, group, subGroup);
	}

	// ----------------------------
	/**
	 * Public functions for setting variables and states in the SDK.
	 */
	public void SetName(string name) {
		GlasslabSDK_SetName (mInst, name);
	}
	
	public void SetVersion(string version) {
		GlasslabSDK_SetVersion (mInst, version);
	}
	
	public void SetGameLevel(string gameLevel) {
		GlasslabSDK_SetGameLevel (mInst, gameLevel);
	}
	
	public void SetUserId(int userId) {
		GlasslabSDK_SetUserId (mInst, userId);
	}
	
	public void SetPlayerHandle(string handle) {
		GlasslabSDK_SetPlayerHandle( mInst, handle );
	}
	
	public void RemovePlayerHandle(string handle) {
		GlasslabSDK_RemovePlayerHandle( mInst, handle );
	}
	
	public void SetCookie(string cookie) {
		GlasslabSDK_SetCookie( mInst, cookie );
	}
	
	public void SetAutoSessionManagement(bool state) {
		GlasslabSDK_SetAutoSessionManagement( mInst, state );
	}
	
	// ----------------------------
	/**
	 * Public functions for getting variables and states in the SDK.
	 */
	public string GetConnectUri() {
		// Get the URI
		IntPtr uriPtr = GlasslabSDK_GetConnectUri( mInst );
		string uri = System.Runtime.InteropServices.Marshal.PtrToStringAuto( uriPtr );
		
		// Return the URI from SDK if it exists
		if( uri != null ) {
			return uri;
		}
		// Otherwise and empty string
		else {
			return "";
		}
	}
	
	public int GetUserId() {
		int userId = GlasslabSDK_GetUserId( mInst );
		return userId;
	}
	
	public string GetCookie( bool fullCookie = false ) {
		// Get the entire cookie string
		IntPtr cookiePtr = GlasslabSDK_GetCookie( mInst );
		string cookie = System.Runtime.InteropServices.Marshal.PtrToStringAuto( cookiePtr );
		
		if( fullCookie ) {
			return cookie;
		}
		
		// Parse the cookie portion between "connect.sid=" and ";"
		string parsedCookie = "";
		
		// Get the index of "connect.sid="
		int indexOfFirst = cookie.IndexOf( "connect.sid=" );
		if( indexOfFirst != -1 ) {
			string sub1 = cookie.Substring( indexOfFirst + 12 );
			
			// Get the next index of ";"
			int indexOfSecond = sub1.IndexOf( ";" );
			if( indexOfSecond != -1 ) {
				// Get the parsed cookie
				parsedCookie = sub1.Substring( 0, indexOfSecond );
				Debug.Log( "Parsed cookie is: " + parsedCookie );
			}
		}
		
		// Returned the parsed cookie
		return parsedCookie;
	}
	
	// ----------------------------
	/**
	 * Public functions for controlling the game throttle timer.
	 */
	public void StartGameTimer() {
		GlasslabSDK_StartGameTimer(mInst);
	}
	public void StopGameTimer() {
		GlasslabSDK_StopGameTimer(mInst);
	}


	// ----------------------------
	/**
	 * Expose all SDK functions below with the DllImport flag.
	 * GlassLab SDK instance management.
	 */
	[DllImport ("__Internal")]
	private static extern System.IntPtr GlasslabSDK_CreateInstance(string dataPath, string clientId, string deviceId, string uri);

	[DllImport ("__Internal")]
	private static extern void GlasslabSDK_FreeInstance(System.IntPtr inst);
	
	/**
	 * Response status and message retrieval. Responses are returned from HTTP requests
	 * to the server. These responses are handled in the Update function on a separate
	 * thread.
	 */
	[DllImport ("__Internal")]
	private static extern int GlasslabSDK_GetLastStatus(System.IntPtr inst);
	
	[DllImport ("__Internal")]
	private static extern void GlasslabSDK_PopMessageStack(System.IntPtr inst);
	
	[DllImport ("__Internal")]
	private static extern int GlasslabSDK_ReadTopMessageCode(System.IntPtr inst);
	
	[DllImport ("__Internal")]
	private static extern IntPtr GlasslabSDK_ReadTopMessageString(System.IntPtr inst);
	
	/**
	 * All functions below make HTTP requests to the server and have callback functions
	 * configured. Each callback returns a response indentifier and JSON message.
	 */
	[DllImport ("__Internal")]
	private static extern void GlasslabSDK_Connect(System.IntPtr inst, string gameId, string uri);

	[DllImport ("__Internal")]
	private static extern void GlasslabSDK_DeviceUpdate(System.IntPtr inst);

	[DllImport ("__Internal")]
	private static extern void GlasslabSDK_AuthStatus(System.IntPtr inst);

	[DllImport ("__Internal")]
	private static extern void GlasslabSDK_RegisterStudent(System.IntPtr inst, string username, string password, string firstName, string lastInitial);

	[DllImport ("__Internal")]
	private static extern void GlasslabSDK_RegisterInstructor(System.IntPtr inst, string name, string email, string password, bool newsletter);

	[DllImport ("__Internal")]
	private static extern void GlasslabSDK_GetUserInfo(System.IntPtr inst);

	[DllImport ("__Internal")]
	private static extern void GlasslabSDK_Login(System.IntPtr inst, string username, string password, string type);

	[DllImport ("__Internal")]
	private static extern void GlasslabSDK_Logout(System.IntPtr inst);

	[DllImport ("__Internal")]
	private static extern void GlasslabSDK_Enroll(System.IntPtr inst, string courseCode);

	[DllImport ("__Internal")]
	private static extern void GlasslabSDK_UnEnroll(System.IntPtr inst, string courseId);

	[DllImport ("__Internal")]
	private static extern void GlasslabSDK_GetCourses(System.IntPtr inst);
	
	[DllImport ("__Internal")]
	private static extern void GlasslabSDK_StartSession(System.IntPtr inst);
	
	[DllImport ("__Internal")]
	private static extern void GlasslabSDK_EndSession(System.IntPtr inst);

	[DllImport ("__Internal")]
	private static extern void GlasslabSDK_SaveGame(System.IntPtr inst, string gameData);

	[DllImport ("__Internal")]
	private static extern void GlasslabSDK_GetSaveGame(System.IntPtr inst);

	[DllImport ("__Internal")]
	private static extern void GlasslabSDK_DeleteSaveGame(System.IntPtr inst);
	
	/**
	 * Helper functions allows a request by key to be cancelled. This is useful for requests that
	 * are mid-stream but have not yet returned to fire a callback. It ensures that the callback
	 * will not be fired.
	 */
	[DllImport ("__Internal")]
	private static extern void GlasslabSDK_CancelRequest(System.IntPtr inst, string key);
	
	/**
	 * The following helper functions allow for preparing a telemetry blob to be sent to the server.
	 * Call GlasslabSDK_SaveTelemEvent after adding the event information you need.
	 */
	[DllImport ("__Internal")]
	private static extern void GlasslabSDK_AddTelemEventValue_ccp   (System.IntPtr inst, string key, string value);

	[DllImport ("__Internal")]
	private static extern void GlasslabSDK_AddTelemEventValue_int8  (System.IntPtr inst, string key, sbyte value);

	[DllImport ("__Internal")]
	private static extern void GlasslabSDK_AddTelemEventValue_int16 (System.IntPtr inst, string key, short value);

	[DllImport ("__Internal")]
	private static extern void GlasslabSDK_AddTelemEventValue_int32 (System.IntPtr inst, string key, int value);

	[DllImport ("__Internal")]
	private static extern void GlasslabSDK_AddTelemEventValue_uint8 (System.IntPtr inst, string key, byte value);

	[DllImport ("__Internal")]
	private static extern void GlasslabSDK_AddTelemEventValue_uint16(System.IntPtr inst, string key, ushort value);

	[DllImport ("__Internal")]
	private static extern void GlasslabSDK_AddTelemEventValue_uint32(System.IntPtr inst, string key, uint value);

	[DllImport ("__Internal")]
	private static extern void GlasslabSDK_AddTelemEventValue_float (System.IntPtr inst, string key, float value);

	[DllImport ("__Internal")]
	private static extern void GlasslabSDK_AddTelemEventValue_double(System.IntPtr inst, string key, double value);

	[DllImport ("__Internal")]
	private static extern void GlasslabSDK_AddTelemEventValue_bool	(System.IntPtr inst, string key, bool value);

	[DllImport ("__Internal")]
	private static extern void GlasslabSDK_ClearTelemEventValues(System.IntPtr inst);

	[DllImport ("__Internal")]
	private static extern void GlasslabSDK_SaveTelemEvent(System.IntPtr inst, string name);
	
	/**
	 * Additional request functions for flushing the telemetry queue and saving an achievement.
	 */
	[DllImport ("__Internal")]
	private static extern void GlasslabSDK_SendTelemEvents(System.IntPtr inst);

	[DllImport ("__Internal")]
	private static extern void GlasslabSDK_SaveAchievement(System.IntPtr inst, string item, string group, string subGroup);


	/**
	 * These functions allow for control over the game timer which is necessary for throttling
	 * telemetry to the server.
	 */
	[DllImport ("__Internal")]
	private static extern void GlasslabSDK_StartGameTimer(System.IntPtr inst);

	[DllImport ("__Internal")]
	private static extern void GlasslabSDK_StopGameTimer(System.IntPtr inst);


	/**
	 * Variable and state getter functions.
	 */
	[DllImport ("__Internal")]
	private static extern IntPtr GlasslabSDK_GetConnectUri(System.IntPtr inst);

	[DllImport ("__Internal")]
	private static extern int GlasslabSDK_GetUserId(System.IntPtr inst);

	[DllImport ("__Internal")]
	private static extern IntPtr GlasslabSDK_GetCookie(System.IntPtr inst);

	/**
	 * Variable and state setter functions.
	 */
	[DllImport ("__Internal")]
	private static extern void GlasslabSDK_SetName(System.IntPtr inst, string name);
	
	[DllImport ("__Internal")]
	private static extern void GlasslabSDK_SetVersion(System.IntPtr inst, string version);
	
	[DllImport ("__Internal")]
	private static extern void GlasslabSDK_SetGameLevel(System.IntPtr inst, string gameLevel);
	
	[DllImport ("__Internal")]
	private static extern void GlasslabSDK_SetUserId(System.IntPtr inst, int userId);
	
	[DllImport ("__Internal")]
	private static extern void GlasslabSDK_SetPlayerHandle(System.IntPtr inst, string handle);
	
	[DllImport ("__Internal")]
	private static extern void GlasslabSDK_RemovePlayerHandle(System.IntPtr inst, string handle);
	
	[DllImport ("__Internal")]
	private static extern void GlasslabSDK_SetCookie(System.IntPtr inst, string cookie);
	
	[DllImport ("__Internal")]
	private static extern void GlasslabSDK_SetAutoSessionManagement(System.IntPtr inst, bool state);
}