#ifdef _UNICODE
typedef wchar_t TCHAR;
#else
typedef char TCHAR;
#endif  // _UNICODE

typedef const TCHAR *LPCTSTR;

#include <Windows.h>
#include <stdlib.h> /* srand, rand */
#include <string.h>
#include <time.h>

#include <cmath>
#include <random>
#include <variant>
#include <vector>

#include "include/CasualLibrary.cpp"
#include "include/Core.cpp"
#include "include/MinHook.h"
#include "include/iniloader.cpp"
#include "include/obfuscator.h"
#include "include/rapidcsv.h"
#include "script.cpp"

using namespace std;

float sunVector[3] = {0, 0, 0};
float envColor[3] = {.99, .99, .99};

namespace {

template <typename T>
const inline MH_STATUS MH_CreateHookApiEx(LPCWSTR pszModule, LPCSTR pszProcName,
                                          LPVOID pDetour, T **ppOriginal) {
    return MH_CreateHookApi(pszModule, pszProcName, pDetour,
                            reinterpret_cast<LPVOID *>(ppOriginal));
}

typedef void(test)();
test *function_trampoline_original = NULL;

typedef void(test2)();
test2 *function_trampoline_original_camera = NULL;

void setSun(Address pointerAddress, float (&vect)[3]) {
    Memory::Internal::write<float>(pointerAddress - 0x50, vect[0]);
    Memory::Internal::write<float>(pointerAddress - 0x4c, vect[1]);
    Memory::Internal::write<float>(pointerAddress - 0x48, vect[2]);
}

void setEnvColor(Address pointerAddress, float (&vect)[3]) {
    Memory::Internal::write<float>(pointerAddress - 0x40, vect[0]);
    Memory::Internal::write<float>(pointerAddress - 0x3c, vect[1]);
    Memory::Internal::write<float>(pointerAddress - 0x38, vect[2]);
}

void setSunColor(Address pointerAddress, float (&vect)[3]) {
    Memory::Internal::write<float>(pointerAddress - 0x30, vect[0]);
    Memory::Internal::write<float>(pointerAddress - 0x2c, vect[1]);
    Memory::Internal::write<float>(pointerAddress - 0x28, vect[2]);
}

void getSun(Address pointerAddress, float (&vect)[3]) {
    vect[0] = Memory::Internal::read<float>(pointerAddress - 0x50);
    vect[1] = Memory::Internal::read<float>(pointerAddress - 0x4c);
    vect[2] = Memory::Internal::read<float>(pointerAddress - 0x48);
}

void function_detour() {
    __asm__(
        "push rax;"
        "push rbx;"
        "push rcx;"
        "push rdx;"
        "push rbp;"
        "push rdi;"
        "push rsi;"
        "push r8;"
        "push r9;"
        "push r10;"
        "push r11;"
        "push r12;"
        "push r13;"
        "push r14;"
        "push r15;");

    register unsigned long long temp asm("rbp");
    Address pointerAddress = Address(temp);

    if (sunVector[0] < -3) {
        sunVector[1] += 0.001;
        if( sunVector[1] >= 0 ) {
            sunVector[2] += 0.001;
            if( sunVector[2] >= 0 ) {
                sunVector[0] = 3;
                sunVector[1] = -0.5;
                sunVector[2] = -0.5;
            }
        }
    } else {
        sunVector[0] -= 0.001;
    }

    setSun(pointerAddress, sunVector);
    // setEnvColor(pointerAddress, envColor);

    printf("%.3f, %.3f, %.3f\n", sunVector[0], sunVector[1], sunVector[2]);

    __asm__(
        "pop r15;"
        "pop r14;"
        "pop r13;"
        "pop r12;"
        "pop r11;"
        "pop r10;"
        "pop r9;"
        "pop r8;"
        "pop rsi;"
        "pop rdi;"
        "pop rbp;"
        "pop rdx;"
        "pop rcx;"
        "pop rbx;"
        "pop rax;");
    return function_trampoline_original();
}

void function_detour_camera() {
    __asm__(
        "push rax;"
        "push rbx;"
        "push rcx;"
        "push rdx;"
        "push rbp;"
        "push rdi;"
        "push rsi;"
        "push r8;"
        "push r9;"
        "push r10;"
        "push r11;"
        "push r12;"
        "push r13;"
        "push r14;"
        "push r15;");

    register unsigned long long temp asm("rbp");
    Address pointerAddress = Address(temp);

    if (sunVector[0] < -3) {
        sunVector[1] += 0.001;
        if( sunVector[1] >= 0 ) {
            sunVector[2] += 0.001;
            if( sunVector[2] >= 0 ) {
                sunVector[0] = 3;
                sunVector[1] = -0.5;
                sunVector[2] = -0.5;
            }
        }
    } else {
        sunVector[0] -= 0.001;
    }

    setSun(pointerAddress, sunVector);
    // setEnvColor(pointerAddress, envColor);

    printf("%.3f, %.3f, %.3f\n", sunVector[0], sunVector[1], sunVector[2]);

    __asm__(
        "pop r15;"
        "pop r14;"
        "pop r13;"
        "pop r12;"
        "pop r11;"
        "pop r10;"
        "pop r9;"
        "pop r8;"
        "pop rsi;"
        "pop rdi;"
        "pop rbp;"
        "pop rdx;"
        "pop rcx;"
        "pop rbx;"
        "pop rax;");
    return function_trampoline_original_camera();
}

namespace Sandbox {
int init() {
    MH_Initialize();
    LPVOID add = (LPVOID)(Core::baseAddress + 0x98f64);
    LPVOID addressCamera = (LPVOID)(Core::baseAddress + 0x104B5B);

    if (MH_CreateHook(add, (LPVOID *)&function_detour,
                      (LPVOID *)&function_trampoline_original) != MH_OK)
        std::cout << AY_OBFUSCATE("Deu erro ao criar o hook") << std::endl;

    if (MH_CreateHook(addressCamera, (LPVOID *)&function_detour_camera,
                      (LPVOID *)&function_trampoline_original_camera) != MH_OK)
        std::cout << AY_OBFUSCATE("Deu erro ao criar o hook") << std::endl;

    if (MH_EnableHook(MH_ALL_HOOKS) != MH_OK) {
        std::cout << AY_OBFUSCATE("Deu erro ao habilitar o hook") << std::endl;
    }
    // ----------------------------------------------------------------

    while (true) {
        if (GetAsyncKeyState(VK_F8) & 0x8000) {
            /*if (MH_EnableHook(MH_ALL_HOOKS) != MH_OK) {
                    std::cout << AY_OBFUSCATE("Deu erro ao habilitar o hook") <<
            std::endl;
                }
            } else {
                if (MH_DisableHook(MH_ALL_HOOKS) != MH_OK) {
                    std::cout << AY_OBFUSCATE("Deu erro ao desabilitar o hook")
            << std::endl;
                }
            } */
        }
        initscript();
    }

    if (MH_DisableHook(MH_ALL_HOOKS)) MH_Uninitialize();
    return 0;
}

}  // namespace Sandbox

}  // namespace