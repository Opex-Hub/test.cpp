#include <iostream>
#include <fstream>
#include <string>
#include <lua.hpp>
#include <thread>
#include <chrono>

class ScriptExecutor {
private:
    lua_State* L;
    
public:
    ScriptExecutor() {
        L = luaL_newstate();
        if (L) {
            luaL_openlibs(L);
        }
    }
    
    ~ScriptExecutor() {
        if (L) {
            lua_close(L);
        }
    }
    
    // Execute script.lua file
    bool executeLuaScript() {
        // Load the script file
        std::ifstream file("script.lua");
        if (!file.is_open()) {
            std::cerr << "Error: Could not open script.lua" << std::endl;
            return false;
        }
        
        // Read the entire file content
        std::string scriptContent((std::istreambuf_iterator<char>(file)),
                                   std::istreambuf_iterator<char>());
        file.close();
        
        if (scriptContent.empty()) {
            std::cerr << "Error: script.lua is empty" << std::endl;
            return false;
        }
        
        // Execute the script in a separate thread to prevent UI blocking
        std::thread execThread([this, scriptContent]() {
            executeInSandbox(scriptContent);
        });
        
        execThread.detach();
        return true;
    }
    
private:
    // Execute script in a protected environment
    void executeInSandbox(const std::string& script) {
        if (!L) {
            std::cerr << "Error: Lua state not initialized" << std::endl;
            return;
        }
        
        // Create a sandboxed environment
        setupSandbox();
        
        // Execute the script
        int result = luaL_dostring(L, script.c_str());
        
        if (result != LUA_OK) {
            const char* error = lua_tostring(L, -1);
            std::cerr << "Script execution error: " << error << std::endl;
            lua_pop(L, 1); // Remove error message
        } else {
            std::cout << "Script executed successfully!" << std::endl;
        }
    }
    
    // Set up a restricted environment
    void setupSandbox() {
        // Create a new table for our sandbox
        lua_newtable(L);
        int sandboxTable = lua_gettop(L);
        
        // Copy safe functions from _G
        copySafeFunctions(sandboxTable);
        
        // Set the sandbox as the global environment
        lua_setglobal(L, "_G");
        
        // Also set it as the environment for the current thread
        lua_pushvalue(L, sandboxTable);
        lua_setupvalue(L, -2, 1); // Set as environment
        lua_pop(L, 1); // Remove sandbox table from stack
    }
    
    // Copy safe functions to sandbox environment
    void copySafeFunctions(int sandboxTable) {
        const char* safeFunctions[] = {
            "print", " pairs", "ipairs", "next",
            "tonumber", "tostring", "type", "assert",
            "error", "pcall", "xpcall",
            "select", "unpack", "rawequal", "rawget", "rawset",
            "setmetatable", "getmetatable", "table", "string",
            "math", "bit32", nullptr
        };
        
        for (int i = 0; safeFunctions[i]; i++) {
            lua_getglobal(L, safeFunctions[i]);
            if (!lua_isnil(L, -1)) {
                lua_setfield(L, sandboxTable, safeFunctions[i]);
            } else {
                lua_pop(L, 1); // Remove nil
            }
        }
        
        // Add custom safe functions if needed
        addCustomFunctions(sandboxTable);
    }
    
    // Add custom functions that are safe for our environment
    void addCustomFunctions(int sandboxTable) {
        // Add custom wait function
        lua_pushcfunction(L, lua_wait);
        lua_setfield(L, sandboxTable, "wait");
        
        // Add custom print function with timestamp
        lua_pushcfunction(L, lua_printWithTimestamp);
        lua_setfield(L, sandboxTable, "printTS");
    }
    
    // Custom wait function (in seconds)
    static int lua_wait(lua_State* L) {
        double seconds = luaL_optnumber(L, 1, 0.01);
        int milliseconds = static_cast<int>(seconds * 1000);
        std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));
        return 0;
    }
    
    // Custom print function with timestamp
    static int lua_printWithTimestamp(lua_State* L) {
        int nargs = lua_gettop(L);
        
        auto now = std::chrono::system_clock::now();
        auto time_t = std::chrono::system_clock::to_time_t(now);
        
        std::cout << "[" << time_t << "] ";
        
        for (int i = 1; i <= nargs; i++) {
            if (i > 1) std::cout << "\t";
            if (lua_isstring(L, i)) {
                std::cout << lua_tostring(L, i);
            } else {
                std::cout << lua_typename(L, lua_type(L, i));
            }
        }
        std::cout << std::endl;
        
        return 0;
    }
};

// Global executor instance
static ScriptExecutor* g_executor = nullptr;

// Initialize the script executor
extern "C" void initializeScriptExecutor() {
    if (!g_executor) {
        g_executor = new ScriptExecutor();
    }
}

// Execute script.lua when called from UI
extern "C" bool executeScriptFromUI() {
    if (!g_executor) {
        initializeScriptExecutor();
    }
    
    if (g_executor) {
        std::cout << "Executing script.lua..." << std::endl;
        return g_executor->executeLuaScript();
    }
    
    return false;
}

// Cleanup function
extern "C" void cleanupScriptExecutor() {
    if (g_executor) {
        delete g_executor;
        g_executor = nullptr;
    }
}
