#pragma once

#include "types.h"

namespace N64 {

using Symbol_Resolver = bool (*)(void* context, const char* name, u32 name_length, u32* out_value);

class Assembler {
public:
    Assembler(Symbol_Resolver resolver = nullptr, void* context = nullptr);
    ~Assembler();
    Assembler(const Assembler&) = delete;
    Assembler& operator=(const Assembler&) = delete;

    bool Assemble(const char* source, u32 origin);
    void Clear();
    const u8* Data() const;
    u32 Size() const;
    u32 Origin() const;
    u32 Error_Line() const;
    const char* Error() const;

private:
    Symbol_Resolver resolver;
    void* context;
    u8* bytes = nullptr;
    u32 size = 0;
    u32 origin = 0;
    u32 errorLine = 0;
    char error[256] = {};
};

}
