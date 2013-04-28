#include "FREXinput.h"
#include <windows.h>
#include <XInput.h>
#include <string>       // std::string
#include <iostream>     // std::cout
#include <sstream>      // std::stringstream




// notation
// FRE***  - functions : bridge to AIR � must be filed in context initializer with name "***"
// DLL_*** - functions : alias for *** functions in XInput1_3.dll
// PL_***  - functions : payload

//struct ControllerStruct
//	{
//		unsigned long eventCount; //event counter, increases with every controller event,
//					  //but for some reason not by one.
//		__int16 btns; // button state bitfield
//		unsigned char lTrigger;  //Left Trigger
//		unsigned char rTrigger;  //Right Trigger
//		short lJoyY;  //Left Joystick Y
//		short lJoyX;  //Left Joystick X
//		short rJoyY;  //Right Joystick Y 
//		short rJoyX;  //Right Joystick X
//	};


typedef void (WINAPI *XInputGetState_t)(uint32_t,XINPUT_STATE*); 
typedef DWORD (WINAPI *XInputSetState_t)(uint32_t,XINPUT_VIBRATION*); 
HMODULE  hinstLib; 
XInputGetState_t DLL_XInputGetState;
XInputSetState_t DLL_XInputSetState;
BOOL fRunTimeLinkSuccess = FALSE; 

void XINPUT_STATE_to_JSON(XINPUT_STATE state, uint8_t** response){
	std::stringstream jsonState;
	jsonState << "{\"packetNumber\": "<< (uint32_t)state.dwPacketNumber << ",\"gamepad\":{\"sticks\":[{\"x\":\"" << state.Gamepad.sThumbLX << "\",\"y\":\"" << state.Gamepad.sThumbLY <<
		    "\"},{\"x\":\"" << state.Gamepad.sThumbRX << "\",\"y\":\"" << state.Gamepad.sThumbRY <<"\"}],\"buttons\":\"" <<
			state.Gamepad.wButtons << "\",\"triggers\":[\"" << (int)state.Gamepad.bLeftTrigger << "\",\"" << (int)state.Gamepad.bRightTrigger << "\"]}}";
	//jsonState << "{\"packetNumber\": "<< (uint32_t)state.eventCount << ",\"gamepad\":{\"sticks\":[{\"x\":\"" << state.lJoyX << "\",\"y\":\"" << state.lJoyY <<
	//			"\"},{\"x\":\"" << state.rJoyX << "\",\"y\":\"" << state.rJoyY<<"\"}],\"buttons\":\"" <<
	//		state.btns << "\",\"triggers\":[\"" << (int)state.lTrigger << "\",\"" << (int)state.rTrigger << "\"]}}";
	std::string s = jsonState.str();
	*response = (uint8_t*)strcpy((char*)malloc(s.length() + 1), s.c_str());
	return;
}

///////////////////////////////////////////////   ///   ///   ///   ///   ///   ///   ///   ///   ///   ///   ///   ///
/// FUNCTIONS WE EXPORT IN DLL FOR TESTING ///   ///   ///   ///   ///   ///   ///   ///   ///   ///   ///   ///   ///
/////////////////////////////////////////////   ///   ///   ///   ///   ///   ///   ///   ///   ///   ///   ///   /// 

void PL_XInputGetState(DWORD dwUserIndex, XINPUT_STATE* state){
	//ControllerStruct state;
	DLL_XInputGetState (dwUserIndex,state);
	return;
	
}
DWORD PL_XInputSetState(DWORD dwUserIndex, WORD left, WORD right){
	XINPUT_VIBRATION rumble;
	rumble.wLeftMotorSpeed = left;
	rumble.wRightMotorSpeed = right;
	return (DWORD) DLL_XInputSetState (dwUserIndex,&rumble);

}


BOOL PL_activate (){

	if (!fRunTimeLinkSuccess)
	{
		hinstLib = LoadLibrary(TEXT("XInput1_3.dll")); 
		if (hinstLib != NULL) 
		{ 
			DLL_XInputGetState = (XInputGetState_t) GetProcAddress(hinstLib, "XInputGetState"); 
			DLL_XInputSetState = (XInputSetState_t) GetProcAddress(hinstLib, "XInputSetState"); 
		    // If the function address is valid, call the function.
			// List of functions we need to use
		    if (NULL != DLL_XInputGetState &&
				NULL != DLL_XInputSetState ) 
		    {
		        fRunTimeLinkSuccess = TRUE; 
		    }
		}
	}
	return fRunTimeLinkSuccess;

}

/////////////////////////////////////////   ///   ///   ///   ///   ///   ///   ///   ///   ///   ///   ///   ///   ///
/// FUNCTIONS WE EXPORT IN EXTENSION ///   ///   ///   ///   ///   ///   ///   ///   ///   ///   ///   ///   ///   /// 
///////////////////////////////////////   ///   ///   ///   ///   ///   ///   ///   ///   ///   ///   ///   ///   ///  

FREObject FRE_XInputGetState(FREContext ctx, void* funcData, uint32_t argc, FREObject argv[])
{
	uint32_t dwUserIndex = 0;
	FREGetObjectAsUint32(argv[0], &dwUserIndex);
	uint8_t* json= NULL;
	
	XINPUT_STATE state;
	PL_XInputGetState((DWORD) dwUserIndex,&state);
	XINPUT_STATE_to_JSON(state,&json);

	FREObject result;
	//FRENewObjectFromUint32(state,&result);
	FRENewObjectFromUTF8(strlen((const char*)json),(const uint8_t*)json,&result);
	return result;
}

FREObject FRE_XInputSetState(FREContext ctx, void* funcData, uint32_t argc, FREObject argv[])
{
	uint32_t dwUserIndex = 0;
	uint32_t lRumble = 0;
	uint32_t rRumble = 0;
	uint32_t errorCode = 0;
	FREGetObjectAsUint32(argv[0],&dwUserIndex);
	FREGetObjectAsUint32(argv[1],&lRumble);
	FREGetObjectAsUint32(argv[2],&rRumble);
	errorCode = (uint32_t) PL_XInputSetState(dwUserIndex,(WORD)lRumble,(WORD)rRumble);
	FREObject result;
	FRENewObjectFromUint32(errorCode, &result);
	return result;
}



FREObject FRE_activate(FREContext ctx, void* funcData, uint32_t argc, FREObject argv[])
{
	PL_activate();
	FREObject result;
	FRENewObjectFromBool(fRunTimeLinkSuccess,&result);
	return result;
}

FREObject FRE_isSupported(FREContext ctx, void* funcData, uint32_t argc, FREObject argv[])
{
	FREObject result;
	FRENewObjectFromBool(TRUE,&result);
	return result;
}

////////////////////////   ///   ///   ///   ///   ///   ///   ///   ///   ///   ///   ///   ///   ///   ///   ///   //
/// RESTRICTED AREA ///   ///   ///   ///   ///   ///   ///   ///   ///   ///   ///   ///   ///   ///   ///   ///   ///
//////////////////////   ///   ///   ///   ///   ///   ///   ///   ///   ///   ///   ///   ///   ///   ///   ///   /// 

void contextInitializer(void* extData, const uint8_t* ctxType, FREContext ctx, uint32_t* numFunctions, const FRENamedFunction** functions)
{
	*numFunctions = 4;

	FRENamedFunction* func = (FRENamedFunction*) malloc(sizeof(FRENamedFunction) * (*numFunctions));

	func[0].name = (const uint8_t*) "isSupported";
	func[0].functionData = NULL;
	func[0].function = &FRE_isSupported;

	func[1].name = (const uint8_t*) "activate";
	func[1].functionData = NULL;
	func[1].function = &FRE_activate;

	func[2].name = (const uint8_t*) "XInputGetState";
	func[2].functionData = NULL;
	func[2].function = &FRE_XInputGetState;

	func[3].name = (const uint8_t*) "XInputSetState";
	func[3].functionData = NULL;
	func[3].function = &FRE_XInputSetState;

	*functions = func;
}

void contextFinalizer(FREContext ctx)
{
	FreeLibrary(hinstLib); 
	return;
}

void FREXInputInitializer(void** extData, FREContextInitializer* ctxInitializer, FREContextFinalizer* ctxFinalizer)
{
	*ctxInitializer = &contextInitializer;
	*ctxFinalizer = &contextFinalizer;
}

void FREXInputFinalizer(void* extData)
{
	return;
}