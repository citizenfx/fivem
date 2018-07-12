LPVOID allocatePoolMemory_ori;

// #include "memmath.h"
// Hook(Scan("48 83 ec 28 83 3d ?? ?? ?? ?? 00 75 23")
//         .add(0x20)
//         .rip(4)
//         .as<void*>(), 
//     allocatePoolMemory, 
//     &allocatePoolMemory_ori
// );

int allocatePoolMemory(UINT_PTR a1, unsigned int hash, int size) {
    // Obviously in FiveM, you would do a map lookup rather that using a switch statement.
    switch (hash) {
        case JOAAT("CGameScriptHandler"):
            trace("Resized ScriptHandler Pool");
            return std::max(256, size);

        case JOAAT("atDScriptObjectNode"):
            trace("Resized ScriptObject Pool");
            return 3072;

        case JOAAT("fwScriptGuid"): // EntityPool
            trace("Resized Entity Pool");
            return 3072;

        case JOAAT("CScriptEntityExtension"): // MissionPool
            trace("Resized Mission Pool");
            return 3072;

        case 0xf2e37be0:
            // The function is called continuously throughout the game with this 1 single hash. 
            // Avoid wasting time processing it. The function is otherwise uncalled except on 
            // initial creation of a pool.
        default:
            return reinterpret_cast<decltype(&allocatePoolMemory)>
                (allocatePoolMemory_ori)(a1, hash, size);
    }
}
