#pragma once

#include <string>
#include <vector>
#include <CasualLibrary.cpp>
#include <iniloader.cpp>

#include <Windows.h>
#include <Psapi.h>
#include <TlHelp32.h>
#include <string.h>
#include <stdio.h>

// #define RELEASE

namespace {
namespace Core {

    /**
     * @brief The base address of the module.
     */
    Address baseAddress = Memory::Internal::getModule(AY_OBFUSCATE("Application-steam-x64.exe"));
    bool isModEnabled = true;
    bool quit = false;
    bool modMainKey = false;
    int delayTime = 20;
    const char* str_modeEnabled[2] = {AY_OBFUSCATE("Mod disabled."), AY_OBFUSCATE("Mod enabled.")};

    /**
     * @brief Check if Cheat Engine is running in the system. It's not reliable; use other techniques together.
     * 
     * @return bool
     */
    bool IsCheatEngineRunning() {
        PROCESSENTRY32 pe32 = { sizeof(PROCESSENTRY32) };
        HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
        if (hSnapshot == INVALID_HANDLE_VALUE)
            return FALSE;
        if (Process32First(hSnapshot, &pe32)) {
            do {
                if ((strcmp(pe32.szExeFile, AY_OBFUSCATE("cheatengine-i386.exe") ) == 0)||(strcmp(pe32.szExeFile, AY_OBFUSCATE("cheatengine-x86_64.exe")) == 0)) {
                    CloseHandle(hSnapshot);
                    return TRUE;
                }
            } while (Process32Next(hSnapshot, &pe32));
        }
        CloseHandle(hSnapshot);
        return FALSE;
    }

    /**
     * @brief Checks if the value passed as a parameter is equal to the value in the memory address. 
     * 
     * @tparam T Example: checkMemoryValue<unsigned char>( 0x0012345, 0x90 )
     * @param address An valid address in the memory (exclude the base address of the module).
     * @param valueToCompare The value to be compared.
     * @return bool
     */
    template <typename T>
    bool checkMemoryValue( Address address, T valueToCompare ) {
        if (Memory::Internal::read<T>(Address(baseAddress + address)) != valueToCompare) {
            return false;
        }
        return true;
    }

    /**
     * @brief Loads an address in little endian format. Ex: an address in memory "03 9A 70 1A" will become 0x1A709A03
     * 
     * @param baseStart The memory address of first byte to be read.
     * @return Address The address reversed.
     */
    Address loadLittleEndianAddress( Address baseStart ) {
        baseStart = baseStart + Core::baseAddress;
        return 
            Memory::Internal::read<unsigned char>( baseStart + 3 ) << 24 |
            Memory::Internal::read<unsigned char>( baseStart + 2 ) << 16 |
            Memory::Internal::read<unsigned char>( baseStart + 1 ) << 8 |
            Memory::Internal::read<unsigned char>( baseStart );
    }

    /**
     * @brief Loads an address inside an assembly instruction that uses immediate/effective addresses. This can come from a mov, lea, call, jmp, etc. The address is in little endian format, so this method reverses it and returns in big endian format.
     * 
     * @param addressInstruction The first byte of the assembly instruction in memory.
     * @param ignoreBytes How many bytes must be ignored in the instruction to reach the immediate address.
     * @param offsetOfInstruction The number of bytes the instruction uses for internal process.
     * @example Consider this instruction: 
     *      0x546DF0    lea     rax, qword_AE8EA00  -> 48 8D 05 09 7C 94 0A
     *      This instruction has 3 bytes before the address to be ignored: 48 8D 05. The address is the next 4 bytes: 09 7C 94 0A. After reversing (0A947C09), it will be summed up with module base address and added the offset of the current instruction call (call instruction needs 6 bytes for example). So, for this example, you need to call the function as loadAddressFromInstruction( 0x0546DF0, 3, 6 );
     * @return Address 
     */
    Address loadAddressFromInstruction( Address addressInstruction, int ignoreBytes, int offsetOfInstruction ) {
        return (loadLittleEndianAddress(addressInstruction + ignoreBytes) + (baseAddress + addressInstruction) + offsetOfInstruction) - baseAddress;
    }

    /**
     * @brief Get an address by searching for an AOB (array of bytes). 
     * 
     * @param baseStart The start address to init the search.
     * @param baseEnd The last address to search.
     * @param sig The AOB (array of bytes) to be used in the search.
     * @param jumps Defines de nth result to be cased. Ex: if passed 2, the second occurrence of the search will be returned.
     * @return The address in the first byte of the result. 
     */
    Address getAddressBySig(Address baseStart, Address baseEnd, char* sig, int jumps=1 ) {
        Address ret;
        if ( jumps < 2 ) {
            ret = Memory::Internal::findSignature(baseAddress + baseStart, sig, baseEnd);
        } else {
            ret = Memory::Internal::findSignature(baseAddress + baseStart, sig, baseEnd);
            for( int w=0; w<jumps-1; w++ ) {
                ret = ret + 0x01;
                ret = Memory::Internal::findSignature(ret, sig, baseEnd);
            }
        }
        return (ret - baseAddress);
    }

  
    void printOffsets() {
        #ifndef RELEASE 
        //printf("================ OFFSETS ================\n");
        //printf("*Don't forget to add module base address*\n");
        //printf("%llx\n\n", add1, add2);
        #endif
        return;
    }
}
}