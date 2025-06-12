#include <iostream>
#include <string>
#include <array>
#include <windows.h>
#include <conio.h>

class MenuScreen {
protected:
    MenuScreen() : m_Running(true), m_UpdateScreen(true) {}

    virtual void displayContent() = 0;
    virtual void handleInputs() { };

    void exitScreen() {
        m_Running = false;
        flushKeyboardBuffer();
    }

    void clearScreen() const {
        std::cout << "\033[2J\033[H";
    }

    void updateScreen() {
        m_UpdateScreen = true;
    }
    
    void enterScreen() {
        m_Running = true;
        m_UpdateScreen = true;

        while(m_Running) {
            flushKeyboardBuffer();

            if(m_UpdateScreen) {
                m_UpdateScreen = false;
                clearScreen();
                displayContent();
            }
            handleInputs();

            Sleep(60);
        }
    }

    void flushKeyboardBuffer() const {
        while (_kbhit()) {
            _getch();
        }
    }
private:
    bool m_Running;
    bool m_UpdateScreen;
};

class FlashCardApp : public MenuScreen {
public:
    FlashCardApp() = default;
    
    void run() {
        enterScreen();
    }
private:    
    int m_SelectedOption{0};

    enum MenuOperation {
        CREATE_CARD = 0x00,
        REVIEW_CARDS = 0x01,
        CHECK_PROGRESS = 0x02,
        LOAD_CARDS = 0x03,
        SAVE_CARDS = 0x04,
        EXIT = 0x05,

        OPERATION_COUNT = 0x06
    };
    
    static constexpr std::array<std::string_view, MenuOperation::OPERATION_COUNT> s_MenuOptions = {
        "Create Card",
        "Review Cards",
        "Check Progress",
        "Load Cards",
        "Save Cards",
        "Exit"
    };

    void displayContent() {
        clearScreen();
        std::cout << "Flash Card App" << std::endl;
        for (int i = 0; i < MenuOperation::OPERATION_COUNT; i++) {
            bool isSelected = (m_SelectedOption == i);

            if (isSelected) {
                std::cout << "\033[1;4m";
            }

            std::cout << i + 1 << ". " << s_MenuOptions[i] << std::endl;

            if (isSelected) {
                std::cout << "\033[0m";
            }
        }
    }

    void handleInputs() override {
        char input = _getch();
        if (input == '1' || input == '2' || input == '3' || input == '4' || input == '5' || input == '6') {
            m_SelectedOption = input - 48 - 1; // 48 is ascii value of 0
            updateScreen();
        }
    }

    void executeSelectedOption() {
        switch(m_SelectedOption) {
            case MenuOperation::CREATE_CARD:
                break;
            case MenuOperation::REVIEW_CARDS:
                break;
            case MenuOperation::CHECK_PROGRESS:
                break;
            case MenuOperation::LOAD_CARDS:
                break;
            case MenuOperation::SAVE_CARDS:
                break;
            case MenuOperation::EXIT:
                exitScreen();
                break;
        }
    }
};

int main() {
    FlashCardApp app;
    app.run();
    return 0;
}