#include "assembler.h"
#include "opcodes.h"
#include "modloader_bytes.h"

#include <limits.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

namespace N64 {

namespace {

bool Parse_Number_Text(const char* text, u32 length, u32* out_value) {
    u32 value = 0;
    u32 base = 10;
    u32 index = 0;

    if (length == 0) {
        return false;
    }

    if (length > 2 && text[0] == '0' && (text[1] == 'x' || text[1] == 'X')) {
        base = 16;
        index = 2;
    }

    for (; index < length; index++) {
        char character = text[index];
        u32 digit;

        if (character >= '0' && character <= '9') {
            digit = static_cast<u32>(character - '0');
        }
        else if (base == 16 && character >= 'a' && character <= 'f') {
            digit = static_cast<u32>(character - 'a' + 10);
        }
        else if (base == 16 && character >= 'A' && character <= 'F') {
            digit = static_cast<u32>(character - 'A' + 10);
        }
        else {
            return false;
        }
        if (value > (UINT32_MAX - digit) / base) {
            return false;
        }

        value = value * base + digit;
    }

    *out_value = value;
    return true;
}

s32 Parse_Register_Number(const char* text, u32 length) {
    u32 number;
    return Parse_Number_Text(text, length, &number) && number < 32 ? static_cast<s32>(number) : -1;
}

s32 Parse_Gpr(const char* text, u32 length) {
    if (length > 0 && text[0] == '$') {
        text++;
        length--;
    }

    s32 number = Parse_Register_Number(text, length);
    if (number >= 0) {
        return number;
    }

    for (u32 index = 0; index < 32; index++) {
        if (Names_Equal(text, length, gGprNames[index])) {
            return static_cast<s32>(index);
        }
    }

    if (Names_Equal(text, length, "s8")) {
        return 30;
    }

    return -1;
}

s32 Parse_Fpr(const char* text, u32 length) {
    if (length > 0 && text[0] == '$') {
        text++;
        length--;
    }

    if (length > 1 && (text[0] == 'f' || text[0] == 'F')) {
        return Parse_Register_Number(text + 1, length - 1);
    }

    return -1;
}

s32 Parse_Cop0(const char* text, u32 length) {
    if (length > 0 && text[0] == '$') {
        s32 number = Parse_Register_Number(text + 1, length - 1);
        if (number >= 0) {
            return number;
        }
    }

    for (u32 index = 0; index < 32; index++) {
        if (Names_Equal(text, length, gCop0Names[index])) {
            return static_cast<s32>(index);
        }
    }

    return -1;
}

s32 Parse_Fcr(const char* text, u32 length) {
    if (length > 0 && text[0] == '$') {
        text++;
        length--;
    }

    if (length > 3 && Names_Equal(text, 3, "fcr")) {
        text += 3;
        length -= 3;
    }

    return Parse_Register_Number(text, length);
}

constexpr u32 gMaxOperands = 8;

struct Label {
    const char* name;
    u32 length;
    u32 value;
};

struct Assembly_State {
    const char* source;
    Symbol_Resolver resolver;
    void* resolverUser;
    Label* labels;
    u32 labelCount;
    u32 labelCapacity;
    u32 origin;
    u64 pc;
    u32 line;
    bool finalPass;
    bool failed;
    char error[256];
    u8* output;
    u32 outputSize;
    u32 outputCapacity;
    u32 outputBase; // address of output[0]
    bool outputBaseSet;
};

struct Text_Span {
    const char* text;
    u32 length;
};

void Fail(Assembly_State* assembler, const char* format, ...) {
    va_list arguments;

    if (assembler->failed) {
        return;
    }
    
    assembler->failed = true;
    va_start(arguments, format);
    vsnprintf(assembler->error, sizeof(assembler->error), format, arguments);
    va_end(arguments);
}

Text_Span Trim_Span(Text_Span span) {
    while (span.length > 0 && (span.text[0] == ' ' || span.text[0] == '\t')) {
        span.text++;
        span.length--;
    }

    while (span.length > 0 && (span.text[span.length - 1] == ' ' || span.text[span.length - 1] == '\t' || span.text[span.length - 1] == '\r')) {
        span.length--;
    }
    return span;
}

Label* Find_Label(Assembly_State* assembler, const char* name, u32 length) {
    for (u32 index = 0; index < assembler->labelCount; index++) {
        if (assembler->labels[index].length == length && strncmp(assembler->labels[index].name, name, length) == 0) {
            return &assembler->labels[index];
        }
    }

    return nullptr;
}

void Define_Label(Assembly_State* assembler, const char* name, u32 length, u32 value) {
    Label* label = Find_Label(assembler, name, length);

    if (label != nullptr) {
        if (!assembler->finalPass && label->value != value) {
            Fail(assembler, "'%.*s' is defined twice", static_cast<int>(length), name);
        }
        label->value = value;
        return;
    }

    if (assembler->labelCount == assembler->labelCapacity) {
        u64 capacity = assembler->labelCapacity == 0 ? 64 : static_cast<u64>(assembler->labelCapacity) * 2;

        if (capacity > UINT32_MAX) {
            capacity = UINT32_MAX;
        }

        if (capacity <= assembler->labelCapacity || capacity > SIZE_MAX / sizeof(Label)) {
            Fail(assembler, "too many labels");
            return;
        }

        Label* grown = static_cast<Label*>(realloc(assembler->labels, static_cast<size_t>(capacity) * sizeof(Label)));
        if (grown == nullptr) {
            Fail(assembler, "out of memory");
            return;
        }

        assembler->labels = grown;
        assembler->labelCapacity = static_cast<u32>(capacity);
    }

    assembler->labels[assembler->labelCount++] = { name, length, value };
}

u32 Closing_Parenthesis(Text_Span span, u32 open) {
    u32 depth = 0;

    for (u32 index = open; index < span.length; index++) {
        if (span.text[index] == '(') {
            depth++;
        }
        else if (span.text[index] == ')') {
            if (--depth == 0) {
                return index;
            }
        }
    }
    return span.length;
}

bool Evaluate(Assembly_State* assembler, Text_Span span, s64* out_value, bool* out_symbolic) {
    s64 total = 0;
    bool symbolic = false;
    u32 index = 0;
    s32 sign = 1;
    bool expect_term = true;

    span = Trim_Span(span);
    while (index < span.length) {
        char character = span.text[index];
        s64 term = 0;

        if (character == ' ' || character == '\t') {
            index++;
            continue;
        }

        if (!expect_term) {
            if (character == '+') {
                sign = 1;
            }
            else if (character == '-') {
                sign = -1;
            }
            else {
                Fail(assembler, "unexpected '%c' in '%.*s'", character, static_cast<int>(span.length), span.text);
                return false;
            }

            index++;
            expect_term = true;
            continue;
        }

        if (character == '-') {
            sign = -sign;
            index++;
            continue;
        }

        if (character == '+') {
            index++;
            continue;
        }

        if (character == '%') {
            u32 start = index + 1;
            u32 end = start;
            Text_Span function;
            u32 close;
            s64 inner = 0;
            bool inner_symbolic = false;
            u32 value;

            while (end < span.length && span.text[end] != '(') {
                end++;
            }

            function = { span.text + start, end - start };
            close = Closing_Parenthesis(span, end);
            if (close >= span.length) {
                Fail(assembler, "bad %%hi/%%lo in '%.*s'", static_cast<int>(span.length), span.text);
                return false;
            }

            if (!Evaluate(assembler, { span.text + end + 1, close - end - 1 }, &inner, &inner_symbolic)) {
                return false;
            }

            symbolic |= inner_symbolic;
            value = static_cast<u32>(inner);
            if (Names_Equal(function.text, function.length, "hi")) {
                term = static_cast<s64>((value + 0x8000u) >> 16);
            }
            else if (Names_Equal(function.text, function.length, "lo")) {
                term = static_cast<s16>(value & 0xFFFF);
            }
            else {
                Fail(assembler, "unknown operator %%%.*s", static_cast<int>(function.length), function.text);
                return false;
            }

            index = close + 1;
        }
        else if (character == '(') {
            u32 close = Closing_Parenthesis(span, index);
            bool inner_symbolic = false;

            if (close >= span.length || !Evaluate(assembler, { span.text + index + 1, close - index - 1 }, &term, &inner_symbolic)) {
                Fail(assembler, "bad parentheses in '%.*s'", static_cast<int>(span.length), span.text);
                return false;
            }

            symbolic |= inner_symbolic;
            index = close + 1;
        }
        else {
            u32 start = index;
            const char* token;
            u32 number;
            u32 length;

            while (index < span.length && span.text[index] != '+' && span.text[index] != '-' && span.text[index] != ' ' && span.text[index] != '\t' &&
                   span.text[index] != ')') {
                index++;
            }

            token = span.text + start;
            length = index - start;
            if (length == 1 && token[0] == '.') {
                term = assembler->pc;
            }
            else if (Parse_Number_Text(token, length, &number)) {
                term = number;
            }
            else if (length != 0 && token[0] >= '0' && token[0] <= '9') {
                Fail(assembler, "invalid 32-bit number '%.*s'", static_cast<int>(length), token);
                return false;
            }
            else if (length == 3 && token[0] == '\'' && token[2] == '\'') {
                term = static_cast<u8>(token[1]);
            }
            else {
                const Label* label;
                u32 value = 0;

                symbolic = true;
                label = Find_Label(assembler, token, length);
                if (label != nullptr) {
                    term = label->value;
                }
                else if (assembler->resolver != nullptr && assembler->resolver(assembler->resolverUser, token, length, &value)) {
                    term = value;
                }
                else if (assembler->finalPass) {
                    Fail(assembler, "unknown symbol '%.*s'", static_cast<int>(length), token);
                    return false;
                }
            }
        }

        if (sign < 0) {
            if (term == INT64_MIN) {
                Fail(assembler, "expression overflow");
                return false;
            }
            term = -term;
        }

        if ((term > 0 && total > INT64_MAX - term) || (term < 0 && total < INT64_MIN - term)) {
            Fail(assembler, "expression overflow");
            return false;
        }

        total += term;
        sign = 1;
        expect_term = false;
    }

    if (expect_term) {
        Fail(assembler, "missing value in '%.*s'", static_cast<int>(span.length), span.text);
        return false;
    }

    *out_value = total;
    if (out_symbolic != nullptr) {
        *out_symbolic = symbolic;
    }

    return true;
}

void Emit_Bytes(Assembly_State* assembler, const u8* bytes, u32 size) {
    if (assembler->failed || size == 0) {
        return;
    }

    if (assembler->pc > UINT32_MAX || size > (u64{1} << 32) - assembler->pc) {
        Fail(assembler, "output exceeds the 32-bit address range");
        return;
    }

    if (assembler->finalPass) {
        u32 position;

        if (!assembler->outputBaseSet) {
            assembler->outputBase = static_cast<u32>(assembler->pc);
            assembler->outputBaseSet = true;
        }

        if (assembler->pc < assembler->outputBase) {
            Fail(assembler, ".org went before the output origin");
            return;
        }

        position = static_cast<u32>(assembler->pc - assembler->outputBase);
        if (size > UINT32_MAX - position) {
            Fail(assembler, "output size exceeds 32 bits");
            return;
        }

        u32 end = position + size;
        if (end > assembler->outputCapacity) {
            u64 capacity = assembler->outputCapacity < 256 ? 256 : assembler->outputCapacity;
            u8* grown;

            while (capacity < end) {
                capacity *= 2;
            }

            if (capacity > UINT32_MAX) {
                capacity = UINT32_MAX;
            }

            grown = static_cast<u8*>(realloc(assembler->output, static_cast<size_t>(capacity)));
            if (grown == nullptr) {
                Fail(assembler, "out of memory");
                return;
            }

            memset(grown + assembler->outputCapacity, 0, static_cast<size_t>(capacity) - assembler->outputCapacity);
            assembler->output = grown;
            assembler->outputCapacity = static_cast<u32>(capacity);
        }
        
        if (bytes != nullptr) {
            memcpy(assembler->output + position, bytes, size);
        }
        else {
            memset(assembler->output + position, 0, size);
        }

        if (end > assembler->outputSize) {
            assembler->outputSize = end;
        }
    }
    assembler->pc += size;
}

void Emit_Byte(Assembly_State* assembler, u8 value) {
    Emit_Bytes(assembler, &value, 1);
}

void Emit_Word(Assembly_State* assembler, u32 value) {
    u8 bytes[sizeof(value)];
    ModLoader::Bytes::Write_Be32(bytes, value);
    Emit_Bytes(assembler, bytes, sizeof(bytes));
}

u32 Split_Operands(Assembly_State* assembler, Text_Span span, Text_Span* out_operands) {
    u32 count = 0;
    u32 depth = 0;
    u32 start = 0;
    bool in_string = false;

    span = Trim_Span(span);
    if (span.length == 0) {
        return 0;
    }

    for (u32 index = 0; index <= span.length; index++) {
        char character = index < span.length ? span.text[index] : ',';

        if (character == '\\' && in_string && index + 1 < span.length) {
            index++;
            continue;
        }

        if (character == '"') {
            in_string = !in_string;
        }

        if (in_string) {
            continue;
        }

        if (character == '(') {
            depth++;
        }
        else if (character == ')') {
            if (depth == 0) {
                Fail(assembler, "unexpected ')'");
                return 0;
            }
            depth--;
        }
        else if (character == ',' && depth == 0) {
            if (count == gMaxOperands) {
                Fail(assembler, "too many operands");
                return 0;
            }
            out_operands[count++] = Trim_Span({ span.text + start, index - start });
            start = index + 1;
        }
    }

    if (in_string || depth != 0) {
        Fail(assembler, in_string ? "unterminated string" : "missing ')'");
        return 0;
    }
    return count;
}

s32 Operand_Gpr(Assembly_State* assembler, Text_Span operand) {
    s32 value = Parse_Gpr(operand.text, operand.length);

    if (value < 0) {
        Fail(assembler, "expected a register, got '%.*s'", static_cast<int>(operand.length), operand.text);
        return 0;
    }

    return value;
}

s32 Operand_Fpr(Assembly_State* assembler, Text_Span operand) {
    s32 value = Parse_Fpr(operand.text, operand.length);
    if (value < 0) {
        Fail(assembler, "expected an FPU register, got '%.*s'", static_cast<int>(operand.length), operand.text);
        return 0;
    }

    return value;
}

s64 Operand_Value(Assembly_State* assembler, Text_Span operand, bool* out_symbolic = nullptr) {
    s64 value = 0;
    Evaluate(assembler, operand, &value, out_symbolic);
    return value;
}

void Operand_Memory(Assembly_State* assembler, Text_Span operand, s32* out_offset, s32* out_base) {
    s32 open = -1;
    u32 close;
    Text_Span base;
    Text_Span offset;

    for (u32 index = 0; index < operand.length; index++) {
        if (operand.text[index] == '(') {
            open = static_cast<s32>(index);
        }
    }

    *out_base = 0;
    *out_offset = 0;
    if (open < 0) {
        *out_offset = static_cast<s32>(Operand_Value(assembler, operand));
        return;
    }

    close = Closing_Parenthesis(operand, static_cast<u32>(open));
    if (close == operand.length) {
        Fail(assembler, "missing ')' in '%.*s'", static_cast<int>(operand.length), operand.text);
        return;
    }

    base = Trim_Span({ operand.text + open + 1, close - static_cast<u32>(open) - 1 });
    *out_base = Operand_Gpr(assembler, base);
    offset = Trim_Span({ operand.text, static_cast<u32>(open) });
    if (offset.length != 0) {
        *out_offset = static_cast<s32>(Operand_Value(assembler, offset));
    }
}

u32 Branch_Offset(Assembly_State* assembler, Text_Span operand) {
    s64 target = Operand_Value(assembler, operand);
    s64 delta = target - static_cast<u32>(assembler->pc + 4);

    if (assembler->finalPass && ((delta & 3) != 0 || delta < -0x20000 || delta > 0x1FFFC)) {
        Fail(assembler, "branch target 0x%08X is out of range", static_cast<u32>(target));
    }

    return static_cast<u32>(delta >> 2) & 0xFFFF;
}

void Check_Signed(Assembly_State* assembler, s64 value) {
    if (assembler->finalPass && (value < -0x8000 || value > 0x7FFF)) {
        Fail(assembler, "immediate %lld does not fit 16 signed bits", static_cast<long long>(value));
    }
}

void Check_Unsigned(Assembly_State* assembler, s64 value) {
    if (assembler->finalPass && (value < 0 || value > 0xFFFF)) {
        Fail(assembler, "immediate %lld does not fit 16 unsigned bits", static_cast<long long>(value));
    }
}

u32 Encode(Assembly_State* assembler, const Opcode_Entry* entry, Text_Span* operands, u32 count) {
    u32 word = entry->match;
    auto expect = [&](u32 wanted) {
        if (count != wanted) {
            Fail(assembler, "%s takes %u operands", entry->mnemonic, wanted);
            return false;
        }
        return true;
    };

    switch (entry->format) {
    case Format_None:
        expect(0);
        break;
    case Format_D_S_T:
        if (expect(3)) {
            word |= (Operand_Gpr(assembler, operands[0]) << 11) | (Operand_Gpr(assembler, operands[1]) << 21) | (Operand_Gpr(assembler, operands[2]) << 16);
        }
        break;
    case Format_D_T_S:
        if (expect(3)) {
            word |= (Operand_Gpr(assembler, operands[0]) << 11) | (Operand_Gpr(assembler, operands[1]) << 16) | (Operand_Gpr(assembler, operands[2]) << 21);
        }
        break;
    case Format_D_T_Shift:
        if (expect(3)) {
            s64 shift = Operand_Value(assembler, operands[2]);

            if (shift < 0 || shift > 31) {
                Fail(assembler, "shift amount %lld is out of range", static_cast<long long>(shift));
            }
            word |= (Operand_Gpr(assembler, operands[0]) << 11) | (Operand_Gpr(assembler, operands[1]) << 16) | ((static_cast<u32>(shift) & 31) << 6);
        }
        break;
    case Format_S_T:
        if (expect(2)) {
            word |= (Operand_Gpr(assembler, operands[0]) << 21) | (Operand_Gpr(assembler, operands[1]) << 16);
        }
        break;
    case Format_D:
        if (expect(1)) {
            word |= Operand_Gpr(assembler, operands[0]) << 11;
        }
        break;
    case Format_S:
        if (expect(1)) {
            word |= Operand_Gpr(assembler, operands[0]) << 21;
        }
        break;
    case Format_Jalr:
        if (count == 1) {
            word |= (31u << 11) | (Operand_Gpr(assembler, operands[0]) << 21);
        }
        else if (expect(2)) {
            word |= (Operand_Gpr(assembler, operands[0]) << 11) | (Operand_Gpr(assembler, operands[1]) << 21);
        }
        break;
    case Format_T_S_Signed:
    case Format_T_S_Unsigned:
        if (expect(3)) {
            s64 value = Operand_Value(assembler, operands[2]);

            if (entry->format == Format_T_S_Signed) {
                Check_Signed(assembler, value);
            }
            else {
                Check_Unsigned(assembler, value);
            }
            word |= (Operand_Gpr(assembler, operands[0]) << 16) | (Operand_Gpr(assembler, operands[1]) << 21) | (static_cast<u32>(value) & 0xFFFF);
        }
        break;
    case Format_T_Upper:
        if (expect(2)) {
            s64 value = Operand_Value(assembler, operands[1]);

            Check_Unsigned(assembler, value);
            word |= (Operand_Gpr(assembler, operands[0]) << 16) | (static_cast<u32>(value) & 0xFFFF);
        }
        break;
    case Format_T_Memory:
    case Format_Ft_Memory:
    case Format_Cache:
        if (expect(2)) {
            s32 offset;
            s32 base;
            u32 rt;

            Operand_Memory(assembler, operands[1], &offset, &base);
            Check_Signed(assembler, offset);
            if (entry->format == Format_T_Memory) {
                rt = static_cast<u32>(Operand_Gpr(assembler, operands[0]));
            }
            else if (entry->format == Format_Ft_Memory) {
                rt = static_cast<u32>(Operand_Fpr(assembler, operands[0]));
            }
            else {
                rt = static_cast<u32>(Operand_Value(assembler, operands[0])) & 31;
            }
            word |= (static_cast<u32>(base) << 21) | (rt << 16) | (static_cast<u32>(offset) & 0xFFFF);
        }
        break;
    case Format_S_T_Branch:
        if (expect(3)) {
            word |= (Operand_Gpr(assembler, operands[0]) << 21) | (Operand_Gpr(assembler, operands[1]) << 16) | Branch_Offset(assembler, operands[2]);
        }
        break;
    case Format_S_Branch:
        if (expect(2)) {
            word |= (Operand_Gpr(assembler, operands[0]) << 21) | Branch_Offset(assembler, operands[1]);
        }
        break;
    case Format_Branch:
        if (expect(1)) {
            word |= Branch_Offset(assembler, operands[0]);
        }
        break;
    case Format_Jump:
        if (expect(1)) {
            u32 target = static_cast<u32>(Operand_Value(assembler, operands[0]));

            if (assembler->finalPass && ((target & 0xF0000000u) != ((assembler->pc + 4) & 0xF0000000u) || (target & 3) != 0)) {
                Fail(assembler, "jump target 0x%08X is outside this 256 MiB region", target);
            }
            word |= (target >> 2) & 0x3FFFFFF;
        }
        break;
    case Format_Code:
        if (count == 1) {
            word |= (static_cast<u32>(Operand_Value(assembler, operands[0])) & 0xFFFFF) << 6;
        }
        else {
            expect(0);
        }
        break;
    case Format_T_Cop0:
        if (expect(2)) {
            s32 cop0 = Parse_Cop0(operands[1].text, operands[1].length);

            if (cop0 < 0) {
                Fail(assembler, "unknown COP0 register '%.*s'", static_cast<int>(operands[1].length), operands[1].text);
            }
            word |= (Operand_Gpr(assembler, operands[0]) << 16) | ((static_cast<u32>(cop0) & 31) << 11);
        }
        break;
    case Format_T_Fs:
        if (expect(2)) {
            word |= (Operand_Gpr(assembler, operands[0]) << 16) | (Operand_Fpr(assembler, operands[1]) << 11);
        }
        break;
    case Format_T_Fcr:
        if (expect(2)) {
            s32 fcr = Parse_Fcr(operands[1].text, operands[1].length);

            if (fcr < 0) {
                Fail(assembler, "unknown FPU control register '%.*s'", static_cast<int>(operands[1].length), operands[1].text);
            }
            word |= (Operand_Gpr(assembler, operands[0]) << 16) | ((static_cast<u32>(fcr) & 31) << 11);
        }
        break;
    case Format_Fd_Fs_Ft:
        if (expect(3)) {
            word |= (Operand_Fpr(assembler, operands[0]) << 6) | (Operand_Fpr(assembler, operands[1]) << 11) | (Operand_Fpr(assembler, operands[2]) << 16);
        }
        break;
    case Format_Fd_Fs:
        if (expect(2)) {
            word |= (Operand_Fpr(assembler, operands[0]) << 6) | (Operand_Fpr(assembler, operands[1]) << 11);
        }
        break;
    case Format_Fs_Ft:
        if (expect(2)) {
            word |= (Operand_Fpr(assembler, operands[0]) << 11) | (Operand_Fpr(assembler, operands[1]) << 16);
        }
        break;
    case Format_S_Immediate:
        if (expect(2)) {
            s64 value = Operand_Value(assembler, operands[1]);

            Check_Signed(assembler, value);
            word |= (Operand_Gpr(assembler, operands[0]) << 21) | (static_cast<u32>(value) & 0xFFFF);
        }
        break;
    }
    return word;
}

bool Is_Pseudo(Assembly_State* assembler, Text_Span mnemonic, Text_Span* operands, u32 count) {
    auto is = [&](const char* name) {
        return Names_Equal(mnemonic.text, mnemonic.length, name);
    };

    if ((is("move") || is("not") || is("neg") || is("negu")) && count == 2) {
        bool negate = is("neg") || is("negu");
        Special_Function function = negate ? (is("neg") ? Special_Function::Sub : Special_Function::Subu) :
            (is("move") ? Special_Function::Addu : Special_Function::Nor);
        u32 destination = static_cast<u32>(Operand_Gpr(assembler, operands[0]));
        u32 source = static_cast<u32>(Operand_Gpr(assembler, operands[1]));

        Emit_Word(assembler, Special(function) | (destination << 11) | (source << (negate ? 16 : 21)));
        return true;
    }

    if (is("b") && count == 1) {
        Emit_Word(assembler, Op(Opcode::Beq) | Branch_Offset(assembler, operands[0]));
        return true;
    }

    if (is("bal") && count == 1) {
        Emit_Word(assembler, Regimm(Regimm_Function::Bgezal) | Branch_Offset(assembler, operands[0]));
        return true;
    }

    if ((is("beqz") || is("bnez") || is("beqzl") || is("bnezl")) && count == 2) {
        Opcode opcode = is("beqz") ? Opcode::Beq : (is("bnez") ? Opcode::Bne : (is("beqzl") ? Opcode::Beql : Opcode::Bnel));
        u32 word = Op(opcode) | (static_cast<u32>(Operand_Gpr(assembler, operands[0])) << 21);

        Emit_Word(assembler, word | Branch_Offset(assembler, operands[1]));
        return true;
    }

    if ((is("blt") || is("bge") || is("bgt") || is("ble") || is("bltu") || is("bgeu") || is("bgtu") || is("bleu")) && count == 3) {
        bool swap = is("bgt") || is("ble") || is("bgtu") || is("bleu");
        bool branch_if_set = is("blt") || is("bgt") || is("bltu") || is("bgtu");
        bool is_unsigned = is("bltu") || is("bgeu") || is("bgtu") || is("bleu");
        Text_Span left = swap ? operands[1] : operands[0];
        Text_Span right = swap ? operands[0] : operands[1];
        u32 source = static_cast<u32>(Operand_Gpr(assembler, left));
        u32 target = static_cast<u32>(Operand_Gpr(assembler, right));

        Emit_Word(assembler, Special(is_unsigned ? Special_Function::Sltu : Special_Function::Slt) | (source << 21) | (target << 16) | (1u << 11));
        u32 word = Op(branch_if_set ? Opcode::Bne : Opcode::Beq) | (1u << 21);
        Emit_Word(assembler, word | Branch_Offset(assembler, operands[2]));
        return true;
    }

    if (is("li") && count == 2) {
        s32 rt = Operand_Gpr(assembler, operands[0]);
        bool symbolic = false;
        s64 value = Operand_Value(assembler, operands[1], &symbolic);
        u32 bits = static_cast<u32>(value);

        if (!symbolic && value >= -0x8000 && value <= 0x7FFF) {
            Emit_Word(assembler, Op(Opcode::Addiu) | (static_cast<u32>(rt) << 16) | (bits & 0xFFFF));
        }
        else if (!symbolic && value >= 0 && value <= 0xFFFF) {
            Emit_Word(assembler, Op(Opcode::Ori) | (static_cast<u32>(rt) << 16) | (bits & 0xFFFF));
        }
        else {
            Emit_Word(assembler, Op(Opcode::Lui) | (static_cast<u32>(rt) << 16) | (bits >> 16));
            if (symbolic || (bits & 0xFFFF) != 0) {
                Emit_Word(assembler, Op(Opcode::Ori) | (static_cast<u32>(rt) << 21) | (static_cast<u32>(rt) << 16) | (bits & 0xFFFF));
            }
        }
        return true;
    }

    if (is("la") && count == 2) {
        s32 rt = Operand_Gpr(assembler, operands[0]);
        u32 value = static_cast<u32>(Operand_Value(assembler, operands[1]));

        Emit_Word(assembler, Op(Opcode::Lui) | (static_cast<u32>(rt) << 16) | (((value + 0x8000u) >> 16) & 0xFFFF));
        Emit_Word(assembler, Op(Opcode::Addiu) | (static_cast<u32>(rt) << 21) | (static_cast<u32>(rt) << 16) | (value & 0xFFFF));
        return true;
    }
    return false;
}

void Parse_String_Operand(Assembly_State* assembler, Text_Span operand, bool terminate) {
    if (operand.length < 2 || operand.text[0] != '"' || operand.text[operand.length - 1] != '"') {
        Fail(assembler, "expected a quoted string");
        return;
    }

    for (u32 index = 1; index + 1 < operand.length; index++) {
        char character = operand.text[index];

        if (character == '\\' && index + 2 < operand.length) {
            char escaped;

            index++;
            escaped = operand.text[index];
            character = escaped == 'n' ? '\n' : (escaped == 't' ? '\t' : (escaped == '0' ? '\0' : escaped));
        }
        Emit_Byte(assembler, static_cast<u8>(character));
    }

    if (terminate) {
        Emit_Byte(assembler, 0);
    }
}

void Directive(Assembly_State* assembler, Text_Span name, Text_Span* operands, u32 count) {
    auto is = [&](const char* candidate) {
        return Names_Equal(name.text, name.length, candidate);
    };

    if (is(".org")) {
        if (count != 1) {
            Fail(assembler, ".org takes an address");
            return;
        }
        s64 address = Operand_Value(assembler, operands[0]);

        if (address < INT32_MIN || address > UINT32_MAX) {
            Fail(assembler, ".org address exceeds 32 bits");
            return;
        }
        assembler->pc = static_cast<u32>(address);
    }
    else if (is(".word")) {
        for (u32 index = 0; index < count; index++) {
            Emit_Word(assembler, static_cast<u32>(Operand_Value(assembler, operands[index])));
        }
    }
    else if (is(".half") || is(".hword") || is(".short")) {
        for (u32 index = 0; index < count; index++) {
            u8 bytes[sizeof(u16)];
            ModLoader::Bytes::Write_Be16(bytes, static_cast<u16>(Operand_Value(assembler, operands[index])));
            Emit_Bytes(assembler, bytes, sizeof(bytes));
        }
    }
    else if (is(".byte")) {
        for (u32 index = 0; index < count; index++) {
            Emit_Byte(assembler, static_cast<u8>(Operand_Value(assembler, operands[index])));
        }
    }
    else if (is(".ascii") || is(".asciiz") || is(".asciz")) {
        for (u32 index = 0; index < count; index++) {
            Parse_String_Operand(assembler, operands[index], !is(".ascii"));
        }
    }
    else if (is(".align")) {
        s64 power = count == 1 ? Operand_Value(assembler, operands[0]) : 2;

        if (power < 0 || power > 31) {
            Fail(assembler, ".align power must be between 0 and 31");
            return;
        }
        u64 alignment = u64{1} << power;

        Emit_Bytes(assembler, nullptr, static_cast<u32>((alignment - assembler->pc) & (alignment - 1)));
    }
    else if (is(".space") || is(".skip")) {
        s64 size = count >= 1 ? Operand_Value(assembler, operands[0]) : 0;

        if (size < 0 || size > UINT32_MAX) {
            Fail(assembler, "space size must fit 32 unsigned bits");
            return;
        }
        Emit_Bytes(assembler, nullptr, static_cast<u32>(size));
    }
    else if (is(".equ") || (is(".set") && count == 2)) {
        if (count != 2) {
            Fail(assembler, ".equ takes a name and a value");
            return;
        }
        Define_Label(assembler, operands[0].text, operands[0].length, static_cast<u32>(Operand_Value(assembler, operands[1])));
    }
    else if (
        is(".set") || is(".globl") || is(".global") || is(".text") || is(".data") || is(".section") || is(".type") || is(".size") || is(".ent") || is(".end") ||
        is(".frame") || is(".mask") || is(".fmask")
    ) {
        // stub
    }
    else {
        Fail(assembler, "unknown directive %.*s", static_cast<int>(name.length), name.text);
    }
}

void Assemble_Line(Assembly_State* assembler, Text_Span line) {
    bool in_string = false;
    u32 split = 0;
    Text_Span mnemonic;
    Text_Span operands[gMaxOperands];
    u32 count;
    const Opcode_Entry* entry;

    for (u32 index = 0; index < line.length; index++) {
        char character = line.text[index];

        if (character == '\\' && in_string && index + 1 < line.length) {
            index++;
            continue;
        }

        if (character == '"') {
            in_string = !in_string;
        }

        if (!in_string && (character == '#' || character == ';' || (character == '/' && index + 1 < line.length && line.text[index + 1] == '/'))) {
            line.length = index;
            break;
        }
    }

    line = Trim_Span(line);
    while (true) {
        u32 index = 0;

        while (index < line.length &&
               (line.text[index] == '_' || line.text[index] == '.' || (line.text[index] >= 'a' && line.text[index] <= 'z') ||
                (line.text[index] >= 'A' && line.text[index] <= 'Z') || (line.text[index] >= '0' && line.text[index] <= '9'))) {
            index++;
        }

        if (index > 0 && index < line.length && line.text[index] == ':') {
            if (assembler->pc > UINT32_MAX) {
                Fail(assembler, "label exceeds the 32-bit address range");
                return;
            }

            Define_Label(assembler, line.text, index, static_cast<u32>(assembler->pc));
            line = Trim_Span({ line.text + index + 1, line.length - index - 1 });
            continue;
        }
        break;
    }
    if (line.length == 0) {
        return;
    }

    {
        u32 name_end = 0;
        while (name_end < line.length && line.text[name_end] != ' ' && line.text[name_end] != '\t' && line.text[name_end] != '=' &&
               line.text[name_end] != ',') {
            name_end++;
        }

        u32 equals = name_end;
        while (equals < line.length && (line.text[equals] == ' ' || line.text[equals] == '\t')) {
            equals++;
        }

        if (name_end > 0 && equals < line.length && line.text[equals] == '=') {
            Text_Span value = Trim_Span({ line.text + equals + 1, line.length - equals - 1 });

            Define_Label(assembler, line.text, name_end, static_cast<u32>(Operand_Value(assembler, value)));
            return;
        }
    }

    while (split < line.length && line.text[split] != ' ' && line.text[split] != '\t') {
        split++;
    }

    mnemonic = { line.text, split };
    count = Split_Operands(assembler, { line.text + split, line.length - split }, operands);
    if (assembler->failed) {
        return;
    }

    if (mnemonic.text[0] == '.') {
        Directive(assembler, mnemonic, operands, count);
        return;
    }

    if ((assembler->pc & 3) != 0) {
        Fail(assembler, "instructions must be word aligned");
        return;
    }

    if (Is_Pseudo(assembler, mnemonic, operands, count)) {
        return;
    }

    entry = Find_Mnemonic(mnemonic.text, mnemonic.length);
    if (entry == nullptr) {
        Fail(assembler, "unknown instruction '%.*s'", static_cast<int>(mnemonic.length), mnemonic.text);
        return;
    }

    Emit_Word(assembler, Encode(assembler, entry, operands, count));
}

bool Run_Pass(Assembly_State* assembler, bool final_pass) {
    const char* cursor;

    assembler->finalPass = final_pass;
    assembler->pc = assembler->origin;
    assembler->line = 0;
    cursor = assembler->source;
    while (*cursor != '\0' && !assembler->failed) {
        const char* end = strchr(cursor, '\n');
        size_t length = end != nullptr ? static_cast<size_t>(end - cursor) : strlen(cursor);

        assembler->line++;
        if (length > INT_MAX) {
            Fail(assembler, "source line is too long");
            return false;
        }
        Assemble_Line(assembler, { cursor, static_cast<u32>(length) });
        if (end == nullptr) {
            break;
        }
        cursor = end + 1;
    }
    return !assembler->failed;
}

} // namespace

Assembler::Assembler(Symbol_Resolver symbol_resolver, void* resolver_context)
    : resolver(symbol_resolver), context(resolver_context) {
}

Assembler::~Assembler() {
    Clear();
}

bool Assembler::Assemble(const char* source, u32 address) {
    Assembly_State state = {};

    Clear();

    if (source == nullptr) {
        snprintf(error, sizeof(error), "source is null");
        return false;
    }

    if (strlen(source) > UINT32_MAX) {
        snprintf(error, sizeof(error), "source size exceeds 32 bits");
        return false;
    }

    state.source = source;
    state.resolver = resolver;
    state.resolverUser = context;
    state.origin = address;
    bool success = Run_Pass(&state, false) && Run_Pass(&state, true);

    free(state.labels);
    if (!success) {
        snprintf(error, sizeof(error), "%s", state.error);
        errorLine = state.line;
        free(state.output);
        return false;
    }
    
    bytes = state.output;
    size = state.outputSize;
    origin = state.outputBaseSet ? state.outputBase : address;
    return true;
}

void Assembler::Clear() {
    free(bytes);
    bytes = nullptr;
    size = 0;
    origin = 0;
    errorLine = 0;
    error[0] = '\0';
}

const u8* Assembler::Data() const {
    return bytes;
}

u32 Assembler::Size() const {
    return size;
}

u32 Assembler::Origin() const {
    return origin;
}

u32 Assembler::Error_Line() const {
    return errorLine;
}

const char* Assembler::Error() const {
    return error;
}

} // namespace N64
