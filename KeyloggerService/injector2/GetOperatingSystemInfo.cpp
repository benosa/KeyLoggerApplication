#include "Injector.h"

#define		WIN_NT_3_1		"Windows NT 3.1 (1993)"
#define		WIN_NT_3_5		"Windows NT 3.5 (1994)"
#define		WIN_NT_3_51		"Windows NT 3.51 (1995)"
#define		WIN_NT_4_0		"Windows NT 4.0 (1996)"
#define		WIN_NT_5_0		"Windows NT 5.0 (Windows 2000) (1997-1999)"
#define		WIN_NT_5_1		"Windows NT 5.1 (Windows XP) (2001)"
#define		WIN_NT_5_2		"Windows NT 5.2 (Windows Server 2003, Windows XP x64) (2003)"
#define		WIN_NT_6_0		"Windows NT 6.0 (Windows Vista, Windows Server 2008) (2006)"
#define		WIN_NT_6_1		"Windows NT 6.1 (Windows 7, Windows Server 2008 R2) (2009)"
#define		WIN_NT_6_2		"Windows NT 6.2 (Windows 8, Windows Server 2012) (2012)"
#define		WIN_NT_6_3		"Windows NT 6.3 (Windows 8.1, Windows Server 2012 R2) (2013)"
#define		WIN_NT_10		"Windows NT 10.0 (Windows 10, Windows Server 2016) (2015)"

bool GetOSInfo() {
    printf("Определение версии Windows NT и архитектуры...\n");

    bool is64bit;
    is64bit = (sizeof(void*) != 4);

    if (is64bit)
        printf("Архитектура ОС: 64-бит\n");
    else
        printf("Архитектура ОС: 32-бит\n");

    RTL_OSVERSIONINFOW osVersion;
    ZeroMemory(&osVersion, sizeof(RTL_OSVERSIONINFOW));
    osVersion.dwOSVersionInfoSize = sizeof(osVersion);

    HMODULE hNtdll = GetModuleHandleW(L"ntdll.dll");
    if (hNtdll) {
        RtlGetVersionPtr rtlGetVersion = (RtlGetVersionPtr)GetProcAddress(hNtdll, "RtlGetVersion");
        if (rtlGetVersion && rtlGetVersion(&osVersion) == 0) {
            switch (osVersion.dwMajorVersion) {
            case 3:
                switch (osVersion.dwMinorVersion) {
                case 1:
                    printf("Windows NT 3.1\n");
                    return true;
                case 5:
                    printf("Windows NT 3.5\n");
                    return true;
                case 51:
                    printf("Windows NT 3.51\n");
                    return true;
                }
                break;

            case 4:
                switch (osVersion.dwMinorVersion) {
                case 0:
                    printf("Windows NT 4.0\n");
                    return true;
                }
                break;

            case 5:
                switch (osVersion.dwMinorVersion) {
                case 0:
                    printf("Windows 2000\n");
                    return true;
                case 1:
                    printf("Windows XP\n");
                    return true;
                case 2:
                    printf("Windows Server 2003, Windows XP x64\n");
                    return true;
                }
                break;

            case 6:
                switch (osVersion.dwMinorVersion) {
                case 0:
                    printf("Windows Vista, Windows Server 2008\n");
                    return true;
                case 1:
                    printf("Windows 7, Windows Server 2008 R2\n");
                    return true;
                case 2:
                    printf("Windows 8, Windows Server 2012\n");
                    return true;
                case 3:
                    printf("Windows 8.1, Windows Server 2012 R2\n");
                    return true;
                }
                break;

            case 10:
                switch (osVersion.dwMinorVersion) {
                case 0:
                    printf("Windows 10, Windows Server 2016\n");
                    return true;
                }
                break;
            }
        }
    }

    return false;
}

//bool GetOSInfo() {
//
//	/*
//	* Windows NT 3.1 (1993)
//	* Windows NT 3.5 (1994)
//	* Windows NT 3.51 (1995)
//	* Windows NT 4.0 (1996)
//	* Windows NT 5.0 (Windows 2000) (1997-1999)
//	* Windows NT 5.1 (Windows XP) (2001)
//	* Windows NT 5.2 (Windows Server 2003, Windows XP x64) (2003)
//	* Windows NT 6.0 (Windows Vista, Windows Server 2008) (2006)
//	* Windows NT 6.1 (Windows 7, Windows Server 2008 R2) (2009)
//	* Windows NT 6.2 (Windows 8, Windows Server 2012) (2012)
//	* Windows NT 6.3 (Windows 8.1, Windows Server 2012 R2) (2013)
//	* Windows NT 10.0 (Windows 10, Windows Server 2016) (2015)
//	*/
//
//	printf("Determining Windows NT version and architecture...\n");
//
//	bool is64bit;
//
//	is64bit = (sizeof(void*) != 4);
//
//	if (is64bit)
//		printf("The architecture of OS : 64bit\n");
//	else
//		printf("The architecture of OS : 32bit\n");
//
//
//	OSVERSIONINFO osVersion;
//	
//	ZeroMemory(&osVersion, sizeof(OSVERSIONINFO));
//	osVersion.dwOSVersionInfoSize = sizeof(osVersion);
//	
//	if (GetVersionEx(&osVersion) && osVersion.dwPlatformId == 2) {
//		
//		switch (osVersion.dwMajorVersion){
//
//			case 3:
//				switch (osVersion.dwMinorVersion) {
//
//					case 1:
//						printf("%s\n", WIN_NT_3_1);
//						return true;
//					case 5:
//						printf("%s\n", WIN_NT_3_5);
//						return true;
//					case 51:
//						printf("%s\n", WIN_NT_3_51);
//						return true;
//				}
//				break;
//
//
//			case 4:
//				switch (osVersion.dwMinorVersion) {
//					case 0:
//						printf("%s\n", WIN_NT_4_0);
//						return true;
//				}
//				break;
//
//			case 5:
//				switch (osVersion.dwMinorVersion) {
//
//					case 0:
//						printf("%s\n", WIN_NT_5_0);
//						return true;
//					case 1:
//						printf("%s\n", WIN_NT_5_1);
//						return true;
//					case 2:
//						printf("%s\n", WIN_NT_5_2);
//						return true;
//				}
//				break;
//
//			case 6:
//				switch (osVersion.dwMinorVersion) {
//
//				case 0:
//					printf("%s\n", WIN_NT_6_0);
//					return true;
//				case 1:
//					printf("%s\n", WIN_NT_6_1);
//					return true;
//				case 2:
//					printf("%s\n", WIN_NT_6_2);
//					return true;
//				case 3:
//					printf("%s\n", WIN_NT_6_3);
//					return true;
//				}
//				break;
//
//			case 10:
//				switch (osVersion.dwMinorVersion) {
//					case 0:
//						printf("%s\n", WIN_NT_10);
//						return true;
//				}
//				break;
//		}
//	}
//	
//	return false;
//}