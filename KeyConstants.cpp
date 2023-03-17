#include "KeyConstants.h"

/*
 * VK_0 - VK_9 are the same as ASCII '0' - '9' (0x30 - 0x39)
 * 0x3A - 0x40 : unassigned
 * VK_A - VK_Z are the same as ASCII 'A' - 'Z' (0x41 - 0x5A)
 */
std::wstring Keys::key(int lang, int key, bool shift, bool capital) {
	switch (lang) {
	case 67699721: //ENG
		switch (key) {
		case 0x41: { return std::wstring(capital ? (shift ? L"a" : L"A") : (shift ? L"A" : L"a")); break; }
		case 0x42: { return std::wstring(capital ? (shift ? L"b" : L"B") : (shift ? L"B" : L"b")); break; }
		case 0x43: { return std::wstring(capital ? (shift ? L"c" : L"C") : (shift ? L"C" : L"c")); break; }
		case 0x44: { return std::wstring(capital ? (shift ? L"d" : L"D") : (shift ? L"D" : L"d")); break; }
		case 0x45: { return std::wstring(capital ? (shift ? L"e" : L"E") : (shift ? L"E" : L"e")); break; }
		case 0x46: { return std::wstring(capital ? (shift ? L"f" : L"F") : (shift ? L"F" : L"f")); break; }
		case 0x47: { return std::wstring(capital ? (shift ? L"g" : L"G") : (shift ? L"G" : L"g")); break; }
		case 0x48: { return std::wstring(capital ? (shift ? L"h" : L"H") : (shift ? L"H" : L"h")); break; }
		case 0x49: { return std::wstring(capital ? (shift ? L"i" : L"I") : (shift ? L"I" : L"i")); break; }
		case 0x4A: { return std::wstring(capital ? (shift ? L"j" : L"J") : (shift ? L"J" : L"j")); break; }
		case 0x4B: { return std::wstring(capital ? (shift ? L"k" : L"K") : (shift ? L"K" : L"k")); break; }
		case 0x4C: { return std::wstring(capital ? (shift ? L"l" : L"L") : (shift ? L"L" : L"l")); break; }
		case 0x4D: { return std::wstring(capital ? (shift ? L"m" : L"M") : (shift ? L"M" : L"m")); break; }
		case 0x4E: { return std::wstring(capital ? (shift ? L"n" : L"N") : (shift ? L"N" : L"n")); break; }
		case 0x4F: { return std::wstring(capital ? (shift ? L"o" : L"O") : (shift ? L"O" : L"o")); break; }
		case 0x50: { return std::wstring(capital ? (shift ? L"p" : L"P") : (shift ? L"P" : L"p")); break; }
		case 0x51: { return std::wstring(capital ? (shift ? L"q" : L"Q") : (shift ? L"Q" : L"q")); break; }
		case 0x52: { return std::wstring(capital ? (shift ? L"r" : L"R") : (shift ? L"R" : L"r")); break; }
		case 0x53: { return std::wstring(capital ? (shift ? L"s" : L"S") : (shift ? L"S" : L"s")); break; }
		case 0x54: { return std::wstring(capital ? (shift ? L"t" : L"T") : (shift ? L"T" : L"t")); break; }
		case 0x55: { return std::wstring(capital ? (shift ? L"u" : L"U") : (shift ? L"U" : L"u")); break; }
		case 0x56: { return std::wstring(capital ? (shift ? L"v" : L"V") : (shift ? L"V" : L"v")); break; }
		case 0x57: { return std::wstring(capital ? (shift ? L"w" : L"W") : (shift ? L"W" : L"w")); break; }
		case 0x58: { return std::wstring(capital ? (shift ? L"x" : L"X") : (shift ? L"X" : L"x")); break; }
		case 0x59: { return std::wstring(capital ? (shift ? L"y" : L"Y") : (shift ? L"Y" : L"y")); break; }
		case 0x5A: { return std::wstring(capital ? (shift ? L"z" : L"Z") : (shift ? L"Z" : L"z")); break; }
		case 0x30: { return std::wstring(shift ? L")" : L"0"); break; }
		case 0x31: { return std::wstring(shift ? L"!" : L"1"); break; }
		case 0x32: { return std::wstring(shift ? L"@" : L"2"); break; }
		case 0x33: { return std::wstring(shift ? L"#" : L"3"); break; }
		case 0x34: { return std::wstring(shift ? L"$" : L"4"); break; }
		case 0x35: { return std::wstring(shift ? L"%" : L"5"); break; }
		case 0x36: { return std::wstring(shift ? L"^" : L"6"); break; }
		case 0x37: { return std::wstring(shift ? L"&" : L"7"); break; }
		case 0x38: { return std::wstring(shift ? L"*" : L"8"); break; }
		case 0x39: { return std::wstring(shift ? L"(" : L"9"); break; }

		case VK_OEM_1: { return std::wstring(shift ? L":" : L";"); break; }
		case VK_OEM_2: { return std::wstring(shift ? L"?" : L"/"); break; }
		case VK_OEM_3: { return std::wstring(shift ? L"~" : L"`"); break; }
		case VK_OEM_4: { return std::wstring(shift ? L"{" : L"["); break; }
		case VK_OEM_5: { return std::wstring(shift ? L"|" : L"\\"); break; }
		case VK_OEM_6: { return std::wstring(shift ? L"}" : L"]"); break; }
		case VK_OEM_7: { return std::wstring(shift ? L"\"" : L"'"); break; }
		case VK_OEM_PLUS: { return std::wstring(shift ? L"+" : L"="); break; }
		case VK_OEM_COMMA: { return std::wstring(shift ? L"<" : L","); break; }
		case VK_OEM_MINUS: { return std::wstring(shift ? L"_" : L"-"); break; }
		case VK_OEM_PERIOD: { return std::wstring(shift ? L">" : L"."); break; }
		default: break;
		}break;
	case 68748313://RUS
		switch (key) {
		case 0x41: { return std::wstring(capital ? (shift ? L"ô" : L"Ô") : (shift ? L"Ô" : L"ô")); break; }
		case 0x42: { return std::wstring(capital ? (shift ? L"è" : L"È") : (shift ? L"È" : L"è")); break; }
		case 0x43: { return std::wstring(capital ? (shift ? L"ñ" : L"Ñ") : (shift ? L"Ñ" : L"ñ")); break; }
		case 0x44: { return std::wstring(capital ? (shift ? L"â" : L"Â") : (shift ? L"Â" : L"â")); break; }
		case 0x45: { return std::wstring(capital ? (shift ? L"ó" : L"Ó") : (shift ? L"Ó" : L"ó")); break; }
		case 0x46: { return std::wstring(capital ? (shift ? L"à" : L"À") : (shift ? L"À" : L"à")); break; }
		case 0x47: { return std::wstring(capital ? (shift ? L"ï" : L"Ï") : (shift ? L"Ï" : L"ï")); break; }
		case 0x48: { return std::wstring(capital ? (shift ? L"ð" : L"Ð") : (shift ? L"Ð" : L"ð")); break; }
		case 0x49: { return std::wstring(capital ? (shift ? L"ø" : L"Ø") : (shift ? L"Ø" : L"ø")); break; }
		case 0x4A: { return std::wstring(capital ? (shift ? L"î" : L"Î") : (shift ? L"Î" : L"î")); break; }
		case 0x4B: { return std::wstring(capital ? (shift ? L"ë" : L"Ë") : (shift ? L"Ë" : L"ë")); break; }
		case 0x4C: { return std::wstring(capital ? (shift ? L"ä" : L"Ä") : (shift ? L"Ä" : L"ä")); break; }
		case 0x4D: { return std::wstring(capital ? (shift ? L"ü" : L"Ü") : (shift ? L"Ü" : L"ü")); break; }
		case 0x4E: { return std::wstring(capital ? (shift ? L"ò" : L"Ò") : (shift ? L"Ò" : L"ò")); break; }
		case 0x4F: { return std::wstring(capital ? (shift ? L"ù" : L"Ù") : (shift ? L"Ù" : L"ù")); break; }
		case 0x50: { return std::wstring(capital ? (shift ? L"ç" : L"Ç") : (shift ? L"Ç" : L"ç")); break; }
		case 0x51: { return std::wstring(capital ? (shift ? L"é" : L"É") : (shift ? L"É" : L"é")); break; }
		case 0x52: { return std::wstring(capital ? (shift ? L"ê" : L"Ê") : (shift ? L"Ê" : L"ê")); break; }
		case 0x53: { return std::wstring(capital ? (shift ? L"û" : L"Û") : (shift ? L"Û" : L"û")); break; }
		case 0x54: { return std::wstring(capital ? (shift ? L"å" : L"Å") : (shift ? L"Å" : L"å")); break; }
		case 0x55: { return std::wstring(capital ? (shift ? L"ã" : L"Ã") : (shift ? L"Ã" : L"ã")); break; }
		case 0x56: { return std::wstring(capital ? (shift ? L"ì" : L"Ì") : (shift ? L"Ì" : L"ì")); break; }
		case 0x57: { return std::wstring(capital ? (shift ? L"ö" : L"Ö") : (shift ? L"Ö" : L"ö")); break; }
		case 0x58: { return std::wstring(capital ? (shift ? L"÷" : L"×") : (shift ? L"×" : L"÷")); break; }
		case 0x59: { return std::wstring(capital ? (shift ? L"í" : L"Í") : (shift ? L"Í" : L"í")); break; }
		case 0x5A: { return std::wstring(capital ? (shift ? L"ÿ" : L"ß") : (shift ? L"ß" : L"ÿ")); break; }
		case 0x30: { return std::wstring(shift ? L")" : L"0"); break; }
		case 0x31: { return std::wstring(shift ? L"!" : L"1"); break; }
		case 0x32: { return std::wstring(shift ? L"\"" : L"2"); break; }
		case 0x33: { return std::wstring(shift ? L"¹" : L"3"); break; }
		case 0x34: { return std::wstring(shift ? L";" : L"4"); break; }
		case 0x35: { return std::wstring(shift ? L"%" : L"5"); break; }
		case 0x36: { return std::wstring(shift ? L":" : L"6"); break; }
		case 0x37: { return std::wstring(shift ? L"?" : L"7"); break; }
		case 0x38: { return std::wstring(shift ? L"*" : L"8"); break; }
		case 0x39: { return std::wstring(shift ? L"(" : L"9"); break; }

		case VK_OEM_1: { return std::wstring(capital ? (shift ? L"æ" : L"Æ") : (shift ? L"Æ" : L"æ")); break; }//L":" : L";"
		case VK_OEM_2: { return std::wstring(shift ? L"," : L"."); break; }//L"?" : L"/"
		case VK_OEM_3: { return std::wstring(shift ? L"¨" : L"¸"); break; }//L"~" : L"`"
		case VK_OEM_4: { return std::wstring(shift ? L"Õ" : L"õ"); break; }// L"{" : L"["
		case VK_OEM_5: { return std::wstring(shift ? L"/" : L"\\"); break; }//L"|" : L"\\"
		case VK_OEM_6: { return std::wstring(shift ? L"Ú" : L"ú"); break; }//L"}" : L"]"
		case VK_OEM_7: { return std::wstring(shift ? L"Ý" : L"ý"); break; }// L"\"" : L"'"
		case VK_OEM_PLUS: { return std::wstring(shift ? L"+" : L"="); break; }
		case VK_OEM_COMMA: { return std::wstring(shift ? L"Á" : L"á"); break; }
		case VK_OEM_MINUS: { return std::wstring(shift ? L"_" : L"-"); break; }
		case VK_OEM_PERIOD: { return std::wstring(shift ? L"Þ" : L"þ"); break; }
		default: break;
		}break;
	default: break;
	}
	switch (key)
	{
	case VK_SPACE: { return std::wstring(shift ? L" " : L" "); break; }
	case VK_RETURN: { return std::wstring(L"\n"); break; }
	case VK_TAB: { return std::wstring(L"\t"); break; }
	case VK_BACK: { return std::wstring(L"[BACKSPACE]"); break; }
	case VK_LEFT: { return std::wstring(L"[LEFT]"); break; }
	case VK_RIGHT: { return std::wstring(L"[RIGHT]"); break; }
	case VK_DELETE: { return std::wstring(L"[DELETE]"); break; }
	default:
		break;
	}
	return L"";
}
