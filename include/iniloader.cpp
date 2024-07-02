#ifndef _SANDBOXINILOADER_
#define _SANDBOXINILOADER_

#include <fstream>
#include <string.h>
#include "inipp.h"

using namespace std;

/**
    @brief This namespace has the functions used to read the contents of the .ini file.
    */
namespace {
    namespace IniLoader {
    inipp::Ini<char> ini;
    string fileUrl;
    bool needReload = true;

     /**
        @brief Loads the .ini file.
        @param The URL of the .ini file to be loaded.
    */
    void loadINIFile( char* fileUrlParam ) {
        ini.clear();
        fileUrl = fileUrlParam;
        std::ifstream is( fileUrlParam );
        ini.parse(is);
        ini.strip_trailing_comments();
        ini.default_section(ini.sections["default"]);
        ini.interpolate();
    }

     /**
        @brief Get the value of some variable in .ini file and write to an specified variable.
        @param the section in .ini file. Ex: "default".
        @param the name of the variable to be readed.
        @param the variable witch will receive the value. You need to declare the variable with the same type of the value you expect to be returned. Ex: if destinationVar was declared as int, the value written on it will be an int as well.
    */
    void getValue( const char* section, const char* variableName, auto& destinationVar ) {
        inipp::get_value(ini.sections[section], variableName, destinationVar);
    }

    /**
        @brief Get the value of some variable in .ini file and returns it.
        @param the section in .ini file. Ex: "default".
        @param the name of the variable to be readed.
    */
    template <typename T> T getValue( const char* section, const char* variableName ) {
        T tempVar;
        inipp::get_value(ini.sections[section], variableName, tempVar);
        return tempVar;
    }

};
}

#endif