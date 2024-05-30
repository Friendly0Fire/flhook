#pragma once

class DebugTools
{
        inline static std::unordered_map<std::string, uint> hashMap;
        inline static std::allocator<BYTE> allocator;

        static uint CreateIdDetour(const char* str);

    public:
        DebugTools() = delete;
        static void Init();
};
