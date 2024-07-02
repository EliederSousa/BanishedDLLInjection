#include <Windows.h>
#include "include/obfuscator.h"
#include "include/iniloader.cpp"
#include "sandbox.cpp"
//#include "offsets.cpp"  // Descomente se quiser fazer só a pesquisa por AOBs

static void getConsoleOutput() {
    AllocConsole();
    FILE *f = new FILE();
    freopen_s(&f, AY_OBFUSCATE("CONOUT$"), "w", stdout);
    std::cout.clear();
    //std::cout << AY_OBFUSCATE(" MOD ENABLED! \n") << std::endl;
}                                                                       

static DWORD WINAPI MainThread( LPVOID param ) {
    //IniLoader::loadINIFile(AY_OBFUSCATE("minimap.ini"));
    getConsoleOutput();
    Sandbox::init();
    return 0;
}

bool APIENTRY DllMain( HMODULE hModule, DWORD dwReason, LPVOID lpReserved ) {
    if ( dwReason == DLL_PROCESS_ATTACH ) {
        HANDLE hThread = CreateThread( 0, 0, MainThread, hModule, 0, 0 );
        CloseHandle(hThread);
    }
    return true;
}