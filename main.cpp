#include <iostream>
#include <string>
#include <array>
#include <windows.h>

class FlashCardApp {
public:
    FlashCardApp() = default;
    
    void run() {
        while(m_Running) {
            displayContent();
            Sleep(30);
        }
    }
private:
    int m_SelectedOption{0};
    bool m_Running{true};

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

    void exitScreen() {
        m_Running = false;
    }

    void clearScreen() const {
        std::cout << "\033[2J\033[H";
    }
};

int main() {
    FlashCardApp app;
    app.run();
    return 0;
}