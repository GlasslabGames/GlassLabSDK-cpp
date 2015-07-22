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

using UnityEngine;

public class ButtonHandlers : MonoBehaviour
{
	/** The output text. */
	[SerializeField] protected UnityEngine.UI.Text Output = null;

	/** The username input text. */
	[SerializeField] protected UnityEngine.UI.InputField Username = null;

	/** The password input text. */
	[SerializeField] protected UnityEngine.UI.InputField Password = null;

	/** The password input text. */
	[SerializeField] protected UnityEngine.UI.InputField CourseID = null;

	/** Handles when the player hits the connect button.  Should be the first button hit. */
	public void OnConnect()
	{
		Output.text = "Connect: ";

		SDK.Connect(Application.persistentDataPath, "TEST", "http://developer.playfully.org", CommandDone);
		SDK.SetName("My Awesome Game");
		SDK.SetVersion("1.0");
	}

	public void OnLogin()
	{
		Output.text = "Login...";
		SDK.Login(Username.text, Password.text, CommandDone);
	}

	public void OnLogout()
	{
		Output.text = "Logout...";
		SDK.Logout(CommandDone);
	}

	public void OnAuthStatus()
	{
		Output.text = "AuthStatus...";
		SDK.AuthStatus(CommandDone);
	}

	public void OnGetUserInfo()
	{
		Output.text = "GetUserInfo...";
		SDK.GetUserInfo(CommandDone);
	}

	public void OnEnroll()
	{
		Output.text = "Enroll...";
		SDK.Enroll(CourseID.text, CommandDone);
	}

	public void OnGetCourses()
	{
		Output.text = "GetCourses...";
		SDK.GetCourses(CommandDone);
	}

	public void OnStartSession()
	{
		Output.text = "StartSession...";
		SDK.StartSession();
	}

	public void OnSaveTelemEvent()
	{
		Output.text = "AddTelemEventValue (will be flushed later)";
		SDK.AddTelemEventValue("FakeData", "data");
		SDK.SaveTelemEvent("telem name");
	}

	public void OnEndSession()
	{
		Output.text = "EndSession...";
		SDK.EndSession();
	}

	public void OnSaveAchievement()
	{
		Output.text = "SaveAchievement (will be flushed later)";
		SDK.SaveAchievement("item", "group", "subGroup");
	}

	public void OnSaveGame()
	{
		Output.text = "SaveGame...";
		SDK.SaveGame("{\"test\": \"test val\"}", CommandDone);
	}

	public void OnGetSaveGame()
	{
		Output.text = "GetSaveGame...";
		SDK.GetSaveGame(CommandDone);
	}

	/** Grabs the glass lab SDK singleton. */
	void Start()
	{
		SDK = GlasslabSDK.Instance;
	}

	void Update()
	{
		if (NextText != "")
		{
			Output.text = NextText;
			NextText = "";
		}
	}

	/** Callback with the response string for all SDK commands. */
	void CommandDone(string response = "")
	{
		NextText = Output.text + " " + response;
	}

	private string NextText = "";

	/** The SDK instance. */
	private GlasslabSDK SDK = null;

}