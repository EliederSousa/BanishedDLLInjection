#ifdef _UNICODE
typedef wchar_t TCHAR;
#else
typedef char TCHAR;
#endif  // _UNICODE

typedef const TCHAR* LPCTSTR;

#include <Windows.h>
#include <string.h>
#include <time.h>

#include "include/MinHook.h"
#include "include/CasualLibrary.cpp"
#include "include/iniloader.cpp"
#include "include/obfuscator.h"
#include "include/Core.cpp"

using namespace std;

namespace MemoryChecker {
    int init() {
        while ( !Core::quit ) {
            
            if (GetAsyncKeyState(VK_NUMPAD5) & 0x8000) {
                
            }
            Sleep( Core::delayTime );
        }
        return 0;
    }
}