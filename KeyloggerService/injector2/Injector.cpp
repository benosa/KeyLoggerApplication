#include "Injector.h"
#include <Poco/Util/Application.h>
// variables for Privilege Escalation
HANDLE hToken;
int dwRetVal = RTN_OK;

LPCSTR ConvertWideCharToLPCSTR(WCHAR* wideStr)
{
	int bufferSize = WideCharToMultiByte(CP_ACP, 0, wideStr, -1, NULL, 0, NULL, NULL);
	char* buffer = new char[bufferSize];
	WideCharToMultiByte(CP_ACP, 0, wideStr, -1, buffer, bufferSize, NULL, NULL);
	LPCSTR narrowStr = buffer;
	return narrowStr;
}

int InjectDll(DWORD processId, WCHAR* inctingDll) {
	Poco::Util::Application::instance().logger().get("AppLogger").debug("InjectDll started");
	if (!processId) {
		processId = GetProcessId(GetCurrentProcess());
	}
	
	HANDLE hProcess = OpenProcess(
		PROCESS_QUERY_INFORMATION |
		PROCESS_CREATE_THREAD |
		PROCESS_VM_OPERATION |
		PROCESS_VM_WRITE,
		FALSE, processId);
	Poco::Util::Application::instance().logger().get("AppLogger").debug("OpenProcess called");
	//HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, PID);

	if (!hProcess) {
		return 1;
	}
	
	// disable SeDebugPrivilege
	SetPrivilege(hToken, SE_DEBUG_NAME, FALSE);
	Poco::Util::Application::instance().logger().get("AppLogger").debug("SetPrivilege started");
	// close handles
	CloseHandle(hToken);
	Poco::Util::Application::instance().logger().get("AppLogger").debug("CloseHandle(hToken) started");
	RtlCreateUsreThread_type5(hProcess, ConvertWideCharToLPCSTR(inctingDll));
	Poco::Util::Application::instance().logger().get("AppLogger").debug("RtlCreateUsreThread_type5 ended");
	if (hProcess == NULL || hProcess == INVALID_HANDLE_VALUE)
	{
		return 1;
	}

	//CloseHandle(hProcess);

	return 0;
}
