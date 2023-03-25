#include "Injector.h"
#include <Poco/Util/Application.h>
bool RtlCreateUsreThread_type5(HANDLE hProcess, LPCSTR DllPath) {

	LPVOID LoadLibraryAddr = GetProcAddress(GetModuleHandle(L"kernel32.dll"), "LoadLibraryA");
	Poco::Util::Application::instance().logger().get("AppLogger").debug("GetProcAddress ended");
	if (!LoadLibraryAddr) {
		printf("Could note get real address of LoadLibraryA from kernel32.dll!\n");
		printf("LastError : 0X%x\n", GetLastError());
		Poco::Util::Application::instance().logger().get("AppLogger").debug("Could note get real address of LoadLibraryA from kernel32.dll! Error: " + GetLastError());
		return false;
	}

	printf("LoadLibraryA is located at real address: 0X%p\n", (void*)LoadLibraryAddr);
	Poco::Util::Application::instance().logger().get("AppLogger").debug("LoadLibraryA is located at real address: ");
	Sleep(1000);

	LPVOID pDllPath = VirtualAllocEx(hProcess, 0, strlen(DllPath), MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
	Poco::Util::Application::instance().logger().get("AppLogger").debug("VirtualAllocEx called ");
	if (!pDllPath) {
		printf("Could not allocate Memory in target process\n");
		Poco::Util::Application::instance().logger().get("AppLogger").debug("Could not allocate Memory in target process");
		printf("LastError : 0X%x\n", GetLastError());
		return false;
	}

	printf("Dll path memory allocated at: 0X%p\n", (void*)pDllPath);
	Sleep(1000);

	BOOL written = WriteProcessMemory(hProcess, pDllPath, (LPVOID)DllPath, strlen(DllPath), NULL);

	if (!written) {
		Poco::Util::Application::instance().logger().get("AppLogger").debug("Could not write into the allocated memory");
		printf("Could not write into the allocated memory\n");
		printf("LastError : 0X%x\n", GetLastError());
		return false;
	}
	Poco::Util::Application::instance().logger().get("AppLogger").debug("Dll path memory was written at address");
	printf("Dll path memory was written at address : 0x%p\n", (void*)pDllPath);
	Sleep(1000);

	HMODULE modNtDll = GetModuleHandle(L"ntdll.dll");
	Poco::Util::Application::instance().logger().get("AppLogger").debug("GetModuleHandle(L\"ntdll.dll\")");
	if (!modNtDll) {
		Poco::Util::Application::instance().logger().get("AppLogger").debug("Failed to get module handle for ntdll.dll");
		printf("Failed to get module handle for ntdll.dll\n");
		printf("LastError : 0X%x\n", GetLastError());
		return false;
	}

	pRtlCreatUserThread pfunc_RtlCreateUserThread = (pRtlCreatUserThread)GetProcAddress(modNtDll, "RtlCreateUserThread");
	Poco::Util::Application::instance().logger().get("AppLogger").debug("pfunc_RtlCreateUserThread");
	if (!pfunc_RtlCreateUserThread) {
		Poco::Util::Application::instance().logger().get("AppLogger").debug("Failed to get RtlCreateThreadEx function address from ntdll.dll");
		printf("Failed to get RtlCreateThreadEx function address from ntdll.dll\n");
		printf("LastError: 0X%x\n", GetLastError());
		return false;
	}
	Sleep(1000);

	HANDLE hThread = NULL;

	pfunc_RtlCreateUserThread(
		hProcess,
		NULL,
		0,
		0,
		0,
		0,
		LoadLibraryAddr,
		pDllPath,
		&hThread,
		NULL
	);
	Poco::Util::Application::instance().logger().get("AppLogger").debug("pfunc_RtlCreateUserThread");
	if (!hThread) {
		Poco::Util::Application::instance().logger().get("AppLogger").debug("RtlCreateUserThreadEx failed");
		printf("\nRtlCreateUserThreadEx failed\n");
		printf("LastError: 0X%x\n", GetLastError());

		if (VirtualFreeEx(hProcess, pDllPath, 0, MEM_RELEASE)) {
			printf("Memory was freed in target process\n");
			Sleep(1000);
		}
		return false;
	}
	Poco::Util::Application::instance().logger().get("AppLogger").debug("Thread started with RtlCreateUserThread");
	printf("Thread started with RtlCreateUserThread\n");
	Sleep(1000);
	Poco::Util::Application::instance().logger().get("AppLogger").debug("WaitForSingleObject started");
	WaitForSingleObject(hThread, INFINITE);
	Poco::Util::Application::instance().logger().get("AppLogger").debug("WaitForSingleObject ended");
	if (VirtualFreeEx(hProcess, pDllPath, 0, MEM_RELEASE)) {
		//VirtualFreeEx(hProc, reinterpret_cast<int*>(pDllPath) + 0X010000, 0, MEM_RELEASE);
		printf("Memory was freed in target process\n");
		Sleep(1000);
	}

	CloseHandle(hThread);
	Poco::Util::Application::instance().logger().get("AppLogger").debug("CloseHandle(hThread)");
	CloseHandle(hProcess);
	Poco::Util::Application::instance().logger().get("AppLogger").debug("CloseHandle(hProcess)");
	return true;
}