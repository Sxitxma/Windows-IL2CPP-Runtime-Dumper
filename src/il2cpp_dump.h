//
// Created by Perfare on 2020/7/4.
//

#ifndef RIRU_IL2CPPDUMPER_IL2CPP_H
#define RIRU_IL2CPPDUMPER_IL2CPP_H

#define MODULENAME "GameAssembly.dll"

// true = dump-GAME_VERSION.cs
// false = dump.cs
#define FORMATED_FILE_NAME true

void il2cpp_dump(void* handle, char* outDir, const char* il2cppModuleName);

#endif //RIRU_IL2CPPDUMPER_IL2CPP_H
