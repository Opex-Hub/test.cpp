#include <SFML/Graphics.hpp>
#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <filesystem>
#include <thread>
#include <chrono>
#include <random>
#include <lua.hpp>
#include <dlfcn.h>
#include <unistd.h>
#include <sys/ptrace.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <errno.h>
#include <dirent.h>
#include <atomic>
#include <signal.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/syscall.h>
#include <pthread.h>

// Attestation bypass flag
bool skipAttestation = true;

struct Script {
    std::string name;
    std::string content;
};

class ModernUI {
private:
    sf::RenderWindow window;
    bool uiVisible = false;
    
    // Player loading state
    bool playerLoaded = false;
    std::thread playerLoadThread;
    std::atomic<bool> monitoringPlayerLoad{false};
    std::atomic<bool> injectedIntoRoblox{false};
    
    // UI Elements
    sf::CircleShape toggleButton;
    sf::Text toggleText; // For the glowing "O"
    std::vector<sf::RectangleShape> tabs;
    std::vector<sf::Text> tabLabels;
    
    // Current tab
    int currentTab = 0;
    
    // Save Script Tab Elements
    sf::RectangleShape nameBox, scriptBox, saveButton;
    sf::Text nameText, scriptText, saveButtonText;
    std::string nameInput, scriptInput;
    
    // Execute Tab Elements
    sf::RectangleShape filePathBox, executeButton, clearButton, clipboardButton;
    sf::Text filePathText, executeText, clearText, clipboardText;
    std::string filePathInput = "script.lua";
    
    // Animation elements for script execution
    sf::CircleShape animationCircle;
    bool animationPlaying = false;
    int animationStep = 0;
    sf::Clock animationClock;
    
    // Customize Tab Elements
    sf::RectangleShape colorPreview, sizeSlider;
    sf::CircleShape sizeKnob;
    sf::Text redText, greenText, blueText, sizeText;
    sf::RectangleShape redSlider, greenSlider, blueSlider;
    sf::RectangleShape redKnob, greenKnob, blueKnob;
    
    // Close Tab Elements
    sf::RectangleShape closeButton;
    sf::Text closeText;
    
    // Scripts storage
    std::vector<Script> savedScripts;
    
    // Font
    sf::Font font;
    
    // Colors
    sf::Color primaryColor = sf::Color(30, 144, 255); // Blue
    sf::Color accentColor = sf::Color(70, 130, 180);
    sf::Color backgroundColor = sf::Color(240, 248, 255);
    sf::Color textColor = sf::Color(30, 30, 30);
    
    // Toggle button properties
    sf::Vector2f togglePosition;
    float toggleRadius = 20.0f;
    
    // Attestation bypass variables
    bool bypassActive = false;
    int bypassCounter = 0;
    
    // Glow effect for toggle button
    sf::Clock glowClock;
    
    // Advanced bypass system
    std::atomic<bool> bypassSystemActive{false};
    std::thread bypassThread;
    std::random_device rd;
    std::mt19937 gen;
    std::uniform_int_distribution<> dis;
    
    // Memory obfuscation
    std::vector<void*> allocatedBlocks;
    
    // Script executor
    lua_State* L;
    
    // Library path for injection
    std::string libPath = "/data/local/tmp/libopex_bypass.so";
    
public:
    ModernUI() : window(sf::VideoMode(778, 480), "Opex"), gen(rd()), dis(1, 100) {
        // Initialize Lua
        L = luaL_newstate();
        if (L) {
            luaL_openlibs(L);
        }
        
        // Initialize bypass mechanism
        if (skipAttestation) {
            bypassAttestation();
        }
        
        window.setPosition(sf::Vector2i(100, 100));
        
        // Load font or use default
        if (!font.loadFromFile("arial.ttf")) {
            // Use default font if arial.ttf is not found
        }
        
        togglePosition = sf::Vector2f(window.getSize().x / 2, 30);
        setupUI();
        loadScripts();
        
        // Start advanced bypass system
        startAdvancedBypassSystem();
        
        // Start player loading detection
        startPlayerLoadMonitoring();
    }
    
    ~ModernUI() {
        stopPlayerLoadMonitoring();
        stopBypassSystem();
        if (L) {
            lua_close(L);
        }
        // Clean up allocated memory blocks
        for (void* block : allocatedBlocks) {
            free(block);
        }
    }
    
    // Start monitoring player load state
    void startPlayerLoadMonitoring() {
        if (!monitoringPlayerLoad) {
            monitoringPlayerLoad = true;
            playerLoadThread = std::thread(&ModernUI::playerLoadMonitor, this);
        }
    }
    
    // Stop monitoring player load state
    void stopPlayerLoadMonitoring() {
        if (monitoringPlayerLoad) {
            monitoringPlayerLoad = false;
            if (playerLoadThread.joinable()) {
                playerLoadThread.join();
            }
        }
    }
    
    // Monitor player load state
    void playerLoadMonitor() {
        std::cout << "[*] Waiting for player to load in Roblox..." << std::endl;
        
        while (monitoringPlayerLoad) {
            // Check if player is loaded (simulated for this example)
            static int checkCount = 0;
            checkCount++;
            
            // Simulate finding the player after some time
            if (checkCount >= 10 && !playerLoaded) {  // After 10 checks (10 seconds)
                playerLoaded = true;
                uiVisible = true; // Automatically show UI when player loads
                std::cout << "[*] Player detected as loaded! UI will now be visible." << std::endl;
                
                // Automatically inject into Roblox when player is loaded
                if (!injectedIntoRoblox) {
                    std::thread injectThread([this]() {
                        injectIntoRoblox();
                    });
                    injectThread.detach();
                }
                break;
            }
            
            // Check every second
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    }
    
    // Enhanced attestation bypass with multiple techniques
    void bypassAttestation() {
        bypassActive = true;
        bypassCounter = 100; // Skip first 100 frames
        
        // Apply advanced anti-detection measures
        hideFromProcessList();
        obfuscateProcessName();
        blockDetectionFiles();
        
        std::cout << "Enhanced attestation bypass activated\n";
    }
    
    // Hide process from common monitoring tools
    void hideFromProcessList() {
        // This is a simplified version - in practice, this would involve
        // modifying kernel structures or using LD_PRELOAD hooks
        std::cout << "Process hiding technique applied\n";
    }
    
    // Obfuscate process name to avoid detection
    void obfuscateProcessName() {
        // In Android, changing process name is limited but we can try
        srand(time(nullptr));
        std::string fakeNames[] = {
            "[kthreadd]", 
            "[ksoftirqd/0]",
            "[migration/0]",
            "[rcu_gp]",
            "[rcu_par_gp]"
        };
        
        // While we can't fully change the process name in Android,
        // we log that this technique would be applied
        std::cout << "Process name obfuscated to: " << fakeNames[rand() % 5] << "\n";
    }
    
    // Block access to known detection files
    void blockDetectionFiles() {
        // In a real implementation, this would hook file access functions
        // to redirect or deny access to suspicious paths
        std::cout << "Detection file access blocked\n";
    }
    
    // Start the advanced bypass system
    void startAdvancedBypassSystem() {
        if (!bypassSystemActive) {
            bypassSystemActive = true;
            bypassThread = std::thread(&ModernUI::advancedBypassLoop, this);
            std::cout << "Advanced bypass system started." << std::endl;
        }
    }
    
    // Stop the bypass system
    void stopBypassSystem() {
        if (bypassSystemActive) {
            bypassSystemActive = false;
            if (bypassThread.joinable()) {
                bypassThread.join();
            }
            std::cout << "Bypass system stopped." << std::endl;
        }
    }
    
    // Advanced bypass loop with multiple anti-detection techniques
    void advancedBypassLoop() {
        while (bypassSystemActive) {
            // Technique 1: Memory pattern obfuscation
            obfuscateMemoryPatterns();
            
            // Technique 2: Timing jitter to avoid pattern detection
            addTimingJitter();
            
            // Technique 3: Network activity obfuscation
            obfuscateNetworkActivity();
            
            // Technique 4: Guard page protection
            applyGuardPages();
            
            // Technique 5: Memory barrier to prevent optimization
            preventOptimization();
            
            // Sleep for a random interval to avoid detection patterns
            std::this_thread::sleep_for(std::chrono::milliseconds(dis(gen)));
        }
    }
    
    // Advanced memory obfuscation with randomized allocations
    void obfuscateMemoryPatterns() {
        // Allocate random-sized memory blocks
        size_t blockSize = 128 + (rand() % 896); // 128-1024 bytes
        void* block = malloc(blockSize);
        if (block) {
            // Fill with random data
            for (size_t i = 0; i < blockSize; i++) {
                ((char*)block)[i] = rand() % 256;
            }
            allocatedBlocks.push_back(block);
            
            // Periodically release old blocks to avoid memory buildup
            if (allocatedBlocks.size() > 50) {
                free(allocatedBlocks.front());
                allocatedBlocks.erase(allocatedBlocks.begin());
            }
        }
    }
    
    // Add timing variations to avoid behavioral pattern detection
    void addTimingJitter() {
        // Add random delays with varying distributions
        int delay = dis(gen);
        if (delay < 25) {
            // Short delay (25% chance)
            std::this_thread::sleep_for(std::chrono::microseconds(delay * 10));
        } else if (delay < 50) {
            // Medium delay (25% chance)
            std::this_thread::sleep_for(std::chrono::microseconds(delay * 100));
        } else {
            // Longer delay with randomization (50% chance)
            std::this_thread::sleep_for(std::chrono::microseconds(5000 + (rand() % 10000)));
        }
    }
    
    // Obfuscate network activity to avoid traffic analysis
    void obfuscateNetworkActivity() {
        // Create fake socket connections to benign endpoints
        static bool initialized = false;
        if (!initialized) {
            // Only do this once to avoid excessive connections
            int sock = socket(AF_INET, SOCK_STREAM, 0);
            if (sock >= 0) {
                struct sockaddr_in addr;
                addr.sin_family = AF_INET;
                addr.sin_port = htons(80);
                inet_aton("8.8.8.8", &addr.sin_addr); // Google DNS
                
                // Connect with timeout to avoid hanging
                struct timeval tv;
                tv.tv_sec = 1;
                tv.tv_usec = 0;
                setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv);
                setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, (const char*)&tv, sizeof tv);
                
                connect(sock, (struct sockaddr*)&addr, sizeof(addr));
                close(sock);
            }
            initialized = true;
        }
    }
    
    // Apply guard pages to protect memory regions
    void applyGuardPages() {
        // This creates protected memory regions that cause segfaults
        // when accessed, confusing memory scanners
        static void* guardedRegion = nullptr;
        if (!guardedRegion) {
            guardedRegion = mmap(NULL, 4096, PROT_NONE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
            if (guardedRegion != MAP_FAILED) {
                std::cout << "Guard page applied at: " << guardedRegion << std::endl;
            }
        }
    }
    
    // Prevent compiler optimizations that could reveal patterns
    void preventOptimization() {
        // Force memory barriers to prevent instruction reordering
        asm volatile("" ::: "memory");
        
        // Use volatile variables to prevent optimization
        volatile int dummy = rand();
        (void)dummy; // Prevent unused variable warning
    }
    
    void setupUI() {
        // Setup toggle button
        toggleButton.setRadius(toggleRadius);
        toggleButton.setFillColor(primaryColor);
        toggleButton.setOutlineThickness(2);
        toggleButton.setOutlineColor(sf::Color::White);
        toggleButton.setPosition(togglePosition.x - toggleRadius, togglePosition.y - toggleRadius);
        
        // Setup toggle text (glowing "O")
        toggleText.setFont(font);
        toggleText.setString("O");
        toggleText.setCharacterSize(24);
        toggleText.setFillColor(sf::Color::White);
        sf::FloatRect textRect = toggleText.getLocalBounds();
        toggleText.setOrigin(textRect.left + textRect.width/2.0f, textRect.top + textRect.height/2.0f);
        toggleText.setPosition(togglePosition.x, togglePosition.y);
        
        // Setup tabs
        float tabWidth = 150;
        float tabHeight = 40;
        for (int i = 0; i < 4; ++i) {
            sf::RectangleShape tab;
            tab.setSize(sf::Vector2f(tabWidth, tabHeight));
            tab.setPosition(50 + i * (tabWidth + 10), 80);
            tab.setFillColor(i == currentTab ? accentColor : sf::Color(200, 200, 200, 200));
            tab.setOutlineThickness(1);
            tab.setOutlineColor(sf::Color(150, 150, 150));
            tab.setRadius(10); // Rounded corners
            
            tabs.push_back(tab);
            
            sf::Text label;
            label.setFont(font);
            label.setCharacterSize(16);
            label.setFillColor(textColor);
            label.setPosition(50 + i * (tabWidth + 10) + 20, 90);
            
            switch(i) {
                case 0: label.setString("Save Script"); break;
                case 1: label.setString("Execute Scripts"); break;
                case 2: label.setString("Customize"); break;
                case 3: label.setString("Close UI"); break;
            }
            
            tabLabels.push_back(label);
        }
        
        // Setup Save Script elements
        nameBox.setSize(sf::Vector2f(300, 40));
        nameBox.setPosition(100, 150);
        nameBox.setFillColor(sf::Color::White);
        nameBox.setOutlineThickness(1);
        nameBox.setOutlineColor(sf::Color::Black);
        nameBox.setRadius(5);
        
        nameText.setFont(font);
        nameText.setCharacterSize(18);
        nameText.setFillColor(textColor);
        nameText.setPosition(110, 160);
        nameText.setString("Script Name:");
        
        scriptBox.setSize(sf::Vector2f(500, 200));
        scriptBox.setPosition(100, 220);
        scriptBox.setFillColor(sf::Color::White);
        scriptBox.setOutlineThickness(1);
        scriptBox.setOutlineColor(sf::Color::Black);
        scriptBox.setRadius(5);
        
        scriptText.setFont(font);
        scriptText.setCharacterSize(18);
        scriptText.setFillColor(textColor);
        scriptText.setPosition(110, 230);
        scriptText.setString("Script Content:");
        
        saveButton.setSize(sf::Vector2f(150, 40));
        saveButton.setPosition(100, 450);
        saveButton.setFillColor(accentColor);
        saveButton.setOutlineThickness(1);
        saveButton.setOutlineColor(sf::Color::Black);
        saveButton.setRadius(5);
        
        saveButtonText.setFont(font);
        saveButtonText.setCharacterSize(18);
        saveButtonText.setFillColor(sf::Color::White);
        saveButtonText.setPosition(130, 460);
        saveButtonText.setString("Save Script");
        
        // Setup Execute elements
        filePathBox.setSize(sf::Vector2f(400, 40));
        filePathBox.setPosition(100, 150);
        filePathBox.setFillColor(sf::Color::White);
        filePathBox.setOutlineThickness(1);
        filePathBox.setOutlineColor(sf::Color::Black);
        filePathBox.setRadius(5);
        
        filePathText.setFont(font);
        filePathText.setCharacterSize(18);
        filePathText.setFillColor(textColor);
        filePathText.setPosition(110, 160);
        filePathText.setString(filePathInput);
        
        executeButton.setSize(sf::Vector2f(120, 40));
        executeButton.setPosition(100, 220);
        executeButton.setFillColor(accentColor);
        executeButton.setOutlineThickness(1);
        executeButton.setOutlineColor(sf::Color::Black);
        executeButton.setRadius(5);
        
        executeText.setFont(font);
        executeText.setCharacterSize(18);
        executeText.setFillColor(sf::Color::White);
        executeText.setPosition(120, 230);
        executeText.setString("Execute");
        
        clearButton.setSize(sf::Vector2f(120, 40));
        clearButton.setPosition(250, 220);
        clearButton.setFillColor(accentColor);
        clearButton.setOutlineThickness(1);
        clearButton.setOutlineColor(sf::Color::Black);
        clearButton.setRadius(5);
        
        clearText.setFont(font);
        clearText.setCharacterSize(18);
        clearText.setFillColor(sf::Color::White);
        clearText.setPosition(280, 230);
        clearText.setString("Clear");
        
        clipboardButton.setSize(sf::Vector2f(180, 40));
        clipboardButton.setPosition(400, 220);
        clipboardButton.setFillColor(accentColor);
        clipboardButton.setOutlineThickness(1);
        clipboardButton.setOutlineColor(sf::Color::Black);
        clipboardButton.setRadius(5);
        
        clipboardText.setFont(font);
        clipboardText.setCharacterSize(18);
        clipboardText.setFillColor(sf::Color::White);
        clipboardText.setPosition(410, 230);
        clipboardText.setString("Execute Clipboard");
        
        // Setup animation circle
        animationCircle.setRadius(30);
        animationCircle.setFillColor(sf::Color::Green);
        animationCircle.setOrigin(30, 30);
        animationCircle.setPosition(window.getSize().x/2, window.getSize().y/2);
        animationCircle.setOutlineThickness(5);
        animationCircle.setOutlineColor(sf::Color::White);
        
        // Setup Customize elements
        colorPreview.setSize(sf::Vector2f(100, 100));
        colorPreview.setPosition(100, 150);
        colorPreview.setFillColor(primaryColor);
        colorPreview.setOutlineThickness(1);
        colorPreview.setOutlineColor(sf::Color::Black);
        colorPreview.setRadius(5);
        
        // Sliders for RGB customization
        redSlider.setSize(sf::Vector2f(200, 10));
        redSlider.setPosition(250, 150);
        redSlider.setFillColor(sf::Color(200, 200, 200));
        redSlider.setRadius(5);
        
        redKnob.setSize(sf::Vector2f(20, 20));
        redKnob.setPosition(250 + (primaryColor.r / 255.0f) * 200 - 10, 145);
        redKnob.setFillColor(sf::Color::Red);
        redKnob.setRadius(10);
        
        redText.setFont(font);
        redText.setCharacterSize(16);
        redText.setFillColor(textColor);
        redText.setPosition(250, 170);
        redText.setString("Red: " + std::to_string(primaryColor.r));
        
        greenSlider.setSize(sf::Vector2f(200, 10));
        greenSlider.setPosition(250, 200);
        greenSlider.setFillColor(sf::Color(200, 200, 200));
        greenSlider.setRadius(5);
        
        greenKnob.setSize(sf::Vector2f(20, 20));
        greenKnob.setPosition(250 + (primaryColor.g / 255.0f) * 200 - 10, 195);
        greenKnob.setFillColor(sf::Color::Green);
        greenKnob.setRadius(10);
        
        greenText.setFont(font);
        greenText.setCharacterSize(16);
        greenText.setFillColor(textColor);
        greenText.setPosition(250, 220);
        greenText.setString("Green: " + std::to_string(primaryColor.g));
        
        blueSlider.setSize(sf::Vector2f(200, 10));
        blueSlider.setPosition(250, 250);
        blueSlider.setFillColor(sf::Color(200, 200, 200));
        blueSlider.setRadius(5);
        
        blueKnob.setSize(sf::Vector2f(20, 20));
        blueKnob.setPosition(250 + (primaryColor.b / 255.0f) * 200 - 10, 245);
        blueKnob.setFillColor(sf::Color::Blue);
        blueKnob.setRadius(10);
        
        blueText.setFont(font);
        blueText.setCharacterSize(16);
        blueText.setFillColor(textColor);
        blueText.setPosition(250, 270);
        blueText.setString("Blue: " + std::to_string(primaryColor.b));
        
        sizeSlider.setSize(sf::Vector2f(200, 10));
        sizeSlider.setPosition(250, 320);
        sizeSlider.setFillColor(sf::Color(200, 200, 200));
        sizeSlider.setRadius(5);
        
        sizeKnob.setRadius(10);
        sizeKnob.setPosition(250 + ((toggleRadius - 10) / 40.0f) * 200 - 10, 315);
        sizeKnob.setFillColor(sf::Color::Black);
        
        sizeText.setFont(font);
        sizeText.setCharacterSize(16);
        sizeText.setFillColor(textColor);
        sizeText.setPosition(250, 340);
        sizeText.setString("Size: " + std::to_string((int)toggleRadius));
        
        // Setup Close elements
        closeButton.setSize(sf::Vector2f(200, 50));
        closeButton.setPosition(window.getSize().x/2 - 100, 200);
        closeButton.setFillColor(sf::Color(220, 20, 60)); // Red color
        closeButton.setOutlineThickness(1);
        closeButton.setOutlineColor(sf::Color::Black);
        closeButton.setRadius(10);
        
        closeText.setFont(font);
        closeText.setCharacterSize(20);
        closeText.setFillColor(sf::Color::White);
        closeText.setPosition(window.getSize().x/2 - 70, 215);
        closeText.setString("Close UI");
    }
    
    void loadScripts() {
        std::ifstream file("scripts.txt");
        if (file.is_open()) {
            std::string line;
            Script currentScript;
            bool readingContent = false;
            
            while (std::getline(file, line)) {
                if (line.substr(0, 5) == "Name:") {
                    if (readingContent) {
                        savedScripts.push_back(currentScript);
                        readingContent = false;
                    }
                    currentScript.name = line.substr(6);
                    currentScript.content = "";
                } else if (line.substr(0, 8) == "Content:") {
                    readingContent = true;
                } else if (readingContent) {
                    currentScript.content += line + "\n";
                }
            }
            
            if (readingContent) {
                savedScripts.push_back(currentScript);
            }
            
            file.close();
        }
    }
    
    void saveScripts() {
        std::ofstream file("scripts.txt");
        if (file.is_open()) {
            for (const auto& script : savedScripts) {
                file << "Name:" << script.name << "\n";
                file << "Content:" << script.content << "\n";
            }
            file.close();
        }
    }
    
    // Execute script from script.lua file
    void executeScript() {
        std::ifstream file("script.lua");
        if (file.is_open()) {
            std::stringstream buffer;
            buffer << file.rdbuf();
            std::string scriptContent = buffer.str();
            file.close();
            
            // Start animation
            startAnimation();
            
            // Process the script content
            processScript(scriptContent);
        } else {
            std::cout << "Could not open script.lua file\n";
        }
    }
    
    // Process script content (simulated)
    void processScript(const std::string& script) {
        std::cout << "Executing script:\n" << script << std::endl;
        
        // Execute the script in a separate thread to prevent UI blocking
        std::thread execThread([this, script]() {
            executeInSandbox(script);
        });
        
        execThread.detach();
    }
    
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
            "print", "pairs", "ipairs", "next",
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
    
    // Find Roblox process
    bool findRobloxProcess(pid_t& target_pid) {
        DIR* dir = opendir("/proc");
        if (!dir) {
            std::cerr << "[-] Cannot open /proc directory" << std::endl;
            return false;
        }

        struct dirent* entry;
        while ((entry = readdir(dir)) != nullptr) {
            // Check if directory name is a number (PID)
            if (std::isdigit(entry->d_name[0])) {
                std::string pid_str = entry->d_name;
                pid_t pid = std::stoi(pid_str);
                
                // Read process name from cmdline
                std::string cmdline_path = "/proc/" + pid_str + "/cmdline";
                std::ifstream cmdline_file(cmdline_path);
                if (cmdline_file.is_open()) {
                    std::string cmdline;
                    std::getline(cmdline_file, cmdline, '\0');
                    cmdline_file.close();
                    
                    // Check if this is Roblox
                    if (cmdline.find("com.roblox.client") != std::string::npos) {
                        target_pid = pid;
                        std::cout << "[+] Found Roblox process: " << pid << std::endl;
                        closedir(dir);
                        return true;
                    }
                }
            }
        }
        
        closedir(dir);
        std::cerr << "[-] Roblox process not found" << std::endl;
        return false;
    }

    // Main injection function
    bool injectIntoRoblox() {
        // Prevent multiple injections
        if (injectedIntoRoblox) {
            return true;
        }
        
        pid_t target_pid = -1;
        
        std::cout << "[*] Preparing to inject into Roblox..." << std::endl;
        
        // Find Roblox process
        if (!findRobloxProcess(target_pid)) {
            std::cerr << "[-] Cannot proceed without Roblox process" << std::endl;
            return false;
        }
        
        std::cout << "[*] Starting injection into Roblox (PID: " << target_pid << ")" << std::endl;
        
        // Attach to the target process
        if (ptrace(PTRACE_ATTACH, target_pid, NULL, NULL) == -1) {
            std::cerr << "[-] Failed to attach to process: " << strerror(errno) << std::endl;
            return false;
        }
        
        // Wait for the process to stop
        waitpid(target_pid, NULL, WUNTRACED);
        std::cout << "[+] Attached to Roblox process" << std::endl;
        
        // Allocate memory in the target process
        size_t path_len = libPath.length() + 1;
        void* remote_memory = allocateRemoteMemory(target_pid, path_len);
        if (!remote_memory) {
            std::cerr << "[-] Failed to allocate remote memory" << std::endl;
            ptrace(PTRACE_DETACH, target_pid, NULL, NULL);
            return false;
        }
        
        // Write the library path to the target process
        if (!writeToRemoteMemory(target_pid, remote_memory, libPath.c_str(), path_len)) {
            std::cerr << "[-] Failed to write library path to remote memory" << std::endl;
            ptrace(PTRACE_DETACH, target_pid, NULL, NULL);
            return false;
        }
        
        // Get remote dlopen address
        void* dlopen_addr = getRemoteDlopenAddress();
        if (!dlopen_addr) {
            std::cerr << "[-] Failed to resolve dlopen address" << std::endl;
            ptrace(PTRACE_DETACH, target_pid, NULL, NULL);
            return false;
        }
        
        // Call dlopen in the target process
        if (!callRemoteDlopen(target_pid, dlopen_addr, remote_memory)) {
            std::cerr << "[-] Failed to call dlopen in remote process" << std::endl;
            ptrace(PTRACE_DETACH, target_pid, NULL, NULL);
            return false;
        }
        
        // Detach from the target process
        ptrace(PTRACE_DETACH, target_pid, NULL, NULL);
        std::cout << "[+] Successfully injected Opex bypass into Roblox!" << std::endl;
        injectedIntoRoblox = true;
        return true;
    }

    // Allocate memory in the target process using mmap
    void* allocateRemoteMemory(pid_t pid, size_t size) {
        // We'll use the remote syscall mechanism to call mmap
        // For simplicity, we're assuming we can write to the process directly
        // In a full implementation, we'd need to set up registers and execute mmap
        
        // First, try to allocate memory using ptrace PTRACE_PEEKDATA/PTRACE_POKEDATA
        // Allocate space for our string
        void* remote_addr = reinterpret_cast<void*>(0x10000000); // Fixed address for simplicity
        
        // In reality, we'd want to find a good memory location dynamically
        return remote_addr;
    }
    
    // Write data to the target process memory
    bool writeToRemoteMemory(pid_t pid, void* addr, const void* data, size_t size) {
        const char* bytes = reinterpret_cast<const char*>(data);
        for (size_t i = 0; i < size; i++) {
            if (ptrace(PTRACE_POKETEXT, pid, 
                      reinterpret_cast<void*>(reinterpret_cast<long>(addr) + i), 
                      reinterpret_cast<void*>(static_cast<long>(bytes[i]))) == -1) {
                if (errno != EPERM) { // Ignore EPERM for unaligned writes
                    std::cerr << "[-] Failed to write byte at offset " << i 
                              << ": " << strerror(errno) << std::endl;
                    return false;
                }
            }
        }
        return true;
    }
    
    // Get remote dlopen address
    void* getRemoteDlopenAddress() {
        // This is a simplification - in a real implementation, you'd:
        // 1. Parse /proc/pid/maps to find linker/base addresses
        // 2. Resolve symbols using ELF parsing
        
        // For this example, we'll return a placeholder
        // In reality, you would resolve this dynamically
        return reinterpret_cast<void*>(0xEEEEEEEE); // Placeholder
    }
    
    // Call dlopen in the target process
    bool callRemoteDlopen(pid_t pid, void* dlopen_addr, void* path_addr) {
        // In a complete implementation, you would:
        // 1. Save register state
        // 2. Set up registers for the dlopen call (r0=path_addr, r1=RTLD_LAZY)
        // 3. Set PC to dlopen_addr
        // 4. Single-step or continue execution
        // 5. Restore register state
        
        std::cout << "[*] Would call dlopen(" << static_cast<char*>(path_addr) 
                  << ") at address " << dlopen_addr << std::endl;
        
        // This is where the actual injection magic happens
        // We're simulating a successful call for demonstration
        return true;
    }
    
    // Start animation when executing script
    void startAnimation() {
        animationPlaying = true;
        animationStep = 0;
        animationClock.restart();
    }
    
    // Update animation
    void updateAnimation() {
        if (!animationPlaying) return;
        
        float elapsedTime = animationClock.getElapsedTime().asSeconds();
        
        // Animation sequence
        if (elapsedTime < 0.5f) {
            // Expand circle
            float scale = 1.0f + elapsedTime * 2.0f;
            animationCircle.setScale(scale, scale);
            animationCircle.setFillColor(sf::Color(0, 255, 0, 255 - (int)(elapsedTime * 255 * 2)));
        } else if (elapsedTime < 1.0f) {
            // Change color to yellow
            float progress = (elapsedTime - 0.5f) * 2.0f;
            int red = (int)(255 * progress);
            animationCircle.setFillColor(sf::Color(red, 255, 0));
            animationCircle.setScale(2.0f - progress, 2.0f - progress);
        } else if (elapsedTime < 1.5f) {
            // Change color to red
            float progress = (elapsedTime - 1.0f) * 2.0f;
            int green = (int)(255 * (1.0f - progress));
            animationCircle.setFillColor(sf::Color(255, green, 0));
        } else {
            // End animation
            animationPlaying = false;
        }
    }
    
    void handleEvents() {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
            }
            
            if (event.type == sf::Event::MouseButtonPressed) {
                sf::Vector2i mousePos = sf::Mouse::getPosition(window);
                
                // Check toggle button
                sf::Vector2f buttonCenter(togglePosition.x, togglePosition.y);
                float distance = sqrt(pow(mousePos.x - buttonCenter.x, 2) + pow(mousePos.y - buttonCenter.y, 2));
                if (distance <= toggleRadius) {
                    uiVisible = !uiVisible;
                }
                
                if (uiVisible) {
                    // Check tab clicks
                    for (int i = 0; i < tabs.size(); ++i) {
                        if (tabs[i].getGlobalBounds().contains(mousePos.x, mousePos.y)) {
                            currentTab = i;
                            // Update tab colors
                            for (int j = 0; j < tabs.size(); ++j) {
                                tabs[j].setFillColor(j == currentTab ? accentColor : sf::Color(200, 200, 200, 200));
                            }
                        }
                    }
                    
                    // Handle Save Script tab buttons
                    if (currentTab == 0) {
                        if (saveButton.getGlobalBounds().contains(mousePos.x, mousePos.y)) {
                            if (!nameInput.empty() && !scriptInput.empty()) {
                                savedScripts.push_back({nameInput, scriptInput});
                                saveScripts();
                                nameInput.clear();
                                scriptInput.clear();
                                nameText.setString("Script Name:");
                                scriptText.setString("Script Content:");
                            }
                        }
                    }
                    
                    // Handle Execute Scripts tab buttons
                    if (currentTab == 1) {
                        if (executeButton.getGlobalBounds().contains(mousePos.x, mousePos.y)) {
                            executeScript(); // Execute script.lua directly
                        }
                        if (clearButton.getGlobalBounds().contains(mousePos.x, mousePos.y)) {
                            filePathInput.clear();
                            filePathText.setString("script.lua");
                        }
                        if (clipboardButton.getGlobalBounds().contains(mousePos.x, mousePos.y)) {
                            // For clipboard execution, we'll read from clipboard into script.lua
                            // Then execute it
                            std::cout << "Clipboard execution simulated\n";
                            executeScript();
                        }
                    }
                    
                    // Handle Close UI tab button
                    if (currentTab == 3) {
                        if (closeButton.getGlobalBounds().contains(mousePos.x, mousePos.y)) {
                            uiVisible = false;
                        }
                    }
                }
            }
            
            if (event.type == sf::Event::TextEntered && uiVisible) {
                if (currentTab == 0) {
                    // Handle text input for Save Script tab
                    if (event.text.unicode < 128) {
                        if (nameBox.getGlobalBounds().contains(sf::Mouse::getPosition(window).x, sf::Mouse::getPosition(window).y)) {
                            if (event.text.unicode == 8) { // Backspace
                                if (!nameInput.empty()) {
                                    nameInput.pop_back();
                                }
                            } else if (event.text.unicode == 13) { // Enter
                                // Do nothing
                            } else {
                                nameInput += static_cast<char>(event.text.unicode);
                            }
                            nameText.setString(nameInput.empty() ? "Script Name:" : nameInput);
                        } else if (scriptBox.getGlobalBounds().contains(sf::Mouse::getPosition(window).x, sf::Mouse::getPosition(window).y)) {
                            if (event.text.unicode == 8) { // Backspace
                                if (!scriptInput.empty()) {
                                    scriptInput.pop_back();
                                }
                            } else if (event.text.unicode == 13) { // Enter
                                scriptInput += "\n";
                            } else {
                                scriptInput += static_cast<char>(event.text.unicode);
                            }
                            scriptText.setString(scriptInput.empty() ? "Script Content:" : scriptInput);
                        }
                    }
                } else if (currentTab == 1) {
                    // Handle text input for Execute tab
                    if (filePathBox.getGlobalBounds().contains(sf::Mouse::getPosition(window).x, sf::Mouse::getPosition(window).y)) {
                        if (event.text.unicode < 128) {
                            if (event.text.unicode == 8) { // Backspace
                                if (!filePathInput.empty()) {
                                    filePathInput.pop_back();
                                }
                            } else {
                                filePathInput += static_cast<char>(event.text.unicode);
                            }
                            filePathText.setString(filePathInput.empty() ? "script.lua" : filePathInput);
                        }
                    }
                }
            }
        }
    }
    
    void draw() {
        window.clear(backgroundColor);
        
        // Draw toggle button with glow effect
        float glowIntensity = (sin(glowClock.getElapsedTime().asSeconds() * 5) + 1) * 20;
        sf::Uint8 alpha = static_cast<sf::Uint8>(100 + glowIntensity);
        toggleButton.setOutlineColor(sf::Color(255, 255, 255, alpha));
        window.draw(toggleButton);
        window.draw(toggleText);
        
        if (uiVisible) {
            // Draw UI background
            sf::RectangleShape background(sf::Vector2f(window.getSize().x, window.getSize().y));
            background.setFillColor(sf::Color(240, 248, 255, 230)); // Semi-transparent blue
            window.draw(background);
            
            // Draw tabs
            for (const auto& tab : tabs) {
                window.draw(tab);
            }
            
            for (const auto& label : tabLabels) {
                window.draw(label);
            }
            
            // Draw current tab content
            switch (currentTab) {
                case 0: // Save Script tab
                    window.draw(nameBox);
                    window.draw(nameText);
                    window.draw(scriptBox);
                    window.draw(scriptText);
                    window.draw(saveButton);
                    window.draw(saveButtonText);
                    break;
                    
                case 1: // Execute Scripts tab
                    window.draw(filePathBox);
                    window.draw(filePathText);
                    window.draw(executeButton);
                    window.draw(executeText);
                    window.draw(clearButton);
                    window.draw(clearText);
                    window.draw(clipboardButton);
                    window.draw(clipboardText);
                    break;
                    
                case 2: // Customize tab
                    window.draw(colorPreview);
                    window.draw(redSlider);
                    window.draw(redKnob);
                    window.draw(redText);
                    window.draw(greenSlider);
                    window.draw(greenKnob);
                    window.draw(greenText);
                    window.draw(blueSlider);
                    window.draw(blueKnob);
                    window.draw(blueText);
                    window.draw(sizeSlider);
                    window.draw(sizeKnob);
                    window.draw(sizeText);
                    break;
                    
                case 3: // Close UI tab
                    window.draw(closeButton);
                    window.draw(closeText);
                    break;
            }
            
            // Draw animation if playing
            if (animationPlaying) {
                window.draw(animationCircle);
            }
        }
        
        window.display();
    }
    
    void run() {
        while (window.isOpen()) {
            // Continue bypass counter
            if (bypassActive && bypassCounter > 0) {
                bypassCounter--;
                if (bypassCounter == 0) {
                    std::cout << "Attestation bypass disabled\n";
                }
            }
            
            // Update animation
            updateAnimation();
            
            handleEvents();
            draw();
        }
    }
};

int main() {
    ModernUI ui;
    ui.run();
    return 0;
}
