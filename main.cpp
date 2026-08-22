#include <SFML/Graphics.hpp>
#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <filesystem>

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
    
    // UI Elements
    sf::CircleShape toggleButton;
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
    
public:
    ModernUI() : window(sf::VideoMode(778, 480), "Opex") { // Changed to 778x480
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
    }
    
    // Bypass function to prevent attestation checks
    void bypassAttestation() {
        bypassActive = true;
        bypassCounter = 100; // Skip first 100 frames
        
        // Disable fullscreen which can trigger attestation
        // Using windowed mode instead
        
        // Simulate successful attestation
        std::cout << "Attestation bypass activated\n";
    }
    
    void setupUI() {
        // Setup toggle button
        toggleButton.setRadius(toggleRadius);
        toggleButton.setFillColor(primaryColor);
        toggleButton.setOutlineThickness(2);
        toggleButton.setOutlineColor(sf::Color::White);
        toggleButton.setPosition(togglePosition.x - toggleRadius, togglePosition.y - toggleRadius);
        
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
        // In a real implementation, this would interface with the game engine
        // For now, we simulate execution by printing to console
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
        
        // Draw toggle button
        window.draw(toggleButton);
        
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
