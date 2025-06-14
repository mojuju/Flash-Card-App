#include <iostream>
#include <string>
#include <array>
#include <vector>
#include <algorithm>
#include <filesystem>
#include <fstream>

#include <windows.h>
#include <conio.h>

class KeyPoller {
public:
    KeyPoller(int key) :  m_Key(key) {}

    void poll() {
        m_LastPolledState = m_PolledState;
        m_PolledState = GetKeyState(m_Key) & 0x8000;
    }

    bool keyDown() const {
        return m_PolledState & !m_LastPolledState;
    }
private:
    int m_Key;
    bool m_PolledState{};
    bool m_LastPolledState{};
};

class Card {
public:
    enum Difficulty : unsigned int {
        HARD = 0x01,
        MEDIUM = 0x02,
        EASY = 0x03,

        DIFFICULTY_COUNT = 0x04
    };

    std::string_view getFront() const { return m_Front; }
    std::string_view getBack() const { return m_Back; }
    unsigned int getScore() const { return m_Score; }
private:
    Card(const std::string& front, const std::string& back) 
    : m_Front(front), m_Back(back), m_Score(0) { }

    Card(const std::string& front, const std::string& back, unsigned int score) 
    : m_Front(front), m_Back(back), m_Score(score) { }

    std::string m_Front, m_Back;
    unsigned int m_Score;

    void addScore(Card::Difficulty difficulty) {
        m_Score += static_cast<unsigned int>(difficulty);
    }

    friend class CardManager;
    friend class ReviewCardsScreen;
};

class CardManager {
public:
    CardManager() = default;

    void addCard(const std::string& front, const std::string& back) {
        m_Cards.push_back(std::move(Card(front, back)));
    }

    void saveCards(const std::filesystem::path& path) {
        std::filesystem::create_directories(path.parent_path());
        std::ofstream file(path);
        if (!file) {
            std::cerr << "Failed to open file for saving: " << path << '\n';
            return;
        }

        for (const auto& card : m_Cards) {
            file << card.getFront() << "|||"
                 << card.getBack() << "|||"
                 << card.getScore() << '\n';
        }
    }

    void loadCards(const std::filesystem::path& path) {
        std::ifstream file(path);
        if (!file) {
            std::cerr << "Failed to open file for loading: " << path << std::endl;
            return;
        }

        m_Cards.clear();

        std::string line;
        while (std::getline(file, line)) {
            size_t firstSep = line.find("|||");
            size_t secondSep = line.find("|||", firstSep + 3);

            std::string front = line.substr(0, firstSep);
            std::string back = line.substr(firstSep + 3, secondSep - (firstSep + 3));
            std::string scoreStr = line.substr(secondSep + 3);

            unsigned int score = std::stoul(scoreStr);
            m_Cards.emplace_back(Card(front, back, score));
        }
    }

    std::vector<Card>& getCards() { return m_Cards; }
private:
    std::vector<Card> m_Cards;
};

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

            Sleep(30);
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

class CreateCardScreen : public MenuScreen {
private:
    CreateCardScreen(CardManager& cardmanager) : m_ParentCardManager(cardmanager) { }

    CardManager& m_ParentCardManager;

    void displayContent() override {
        std::string front, back;

        std::cout << "Input front of the card: ";
        takeStringInput(front);
        std::cout << "Input back of the card: ";
        takeStringInput(back);

        m_ParentCardManager.addCard(front, back);
        std::cout << "Card has been created!" << std::endl;
        Sleep(1000);
        exitScreen();
    }

    void takeStringInput(std::string& str) {
        while (true) {
            std::getline(std::cin, str);
            if (!str.empty()) break;
        }
    }

    friend class FlashCardApp;
};

class ReviewCardsScreen : public MenuScreen {
private:
    ReviewCardsScreen(CardManager& cardmanager) : m_ParentCardManager(cardmanager) { }

    CardManager& m_ParentCardManager;
    
    void displayContent() override {
        if(m_ParentCardManager.getCards().empty()) {
            std::cout << "############## Review Flash Cards ##############" << std::endl << std::endl;
            std::cout << "              NO CARDS CREATED YET              " << std::endl << std::endl;
            std::cout << "################################################" << std::endl;
            std::cout << std::endl << "(Press ENTER to go back)" << std::endl;
            while (_getch() != '\r');
            exitScreen();
            return;
        }

        int i = 1;
        for(auto& card: m_ParentCardManager.getCards()) {
            clearScreen();
            std::cout << "############## Review Flash Cards ##############" << std::endl << std::endl;
            std::cout << " Question " << i << " : " << card.getFront() << std::endl << std::endl;
            std::cout << "################################################" << std::endl;
            std::cout << std::endl << "[PRESS ENTER] Answer: ";
            while (_getch() != '\r');
            std::cout << card.getBack();
            Sleep(1000);
            std::cout << std::endl << std::endl << "[1] Hard [2] Medium [3] Easy" << std::endl;

            char input;
            while (true) {
                input = _getch();
                if (input == '1' || input == '2' || input == '3') {
                    card.addScore(static_cast<Card::Difficulty>(input - '0'));
                    std::cout << std::endl << "Finished Reviewing card " << i++ << "." << std::endl;
                    std::cout << std::endl << "################################################" << std::endl;
                    Sleep(1000);
                    break;
                }
            }
        }

        exitScreen();
    }

    friend class FlashCardApp;
};

class CheckProgressScreen : public MenuScreen {
private:
    CheckProgressScreen(CardManager& cardmanager) 
    : m_ParentCardManager(cardmanager) { }

    CardManager& m_ParentCardManager;

    void displayContent() override {
        std::cout << "############## Flash Card Progress #############" << std::endl << std::endl;

        if(m_ParentCardManager.getCards().empty()) {
            std::cout << "              NO CARDS CREATED YET              " << std::endl << std::endl;
            std::cout << "################################################" << std::endl;
            std::cout << std::endl << "(Press ENTER to go back)" << std::endl;
            while (_getch() != '\r');
            exitScreen();
            return;
        }

        int i = 1;
        for(const auto& card : m_ParentCardManager.getCards()) {
            std::cout << i++ << ". " << card.getFront() << " | Score: " << card.getScore() << std::endl;
        }
        std::cout << std::endl << "################################################" << std::endl;
        std::cout << std::endl << "(Press ENTER to go back)" << std::endl;
        while (_getch() != '\r');
        exitScreen();
    }

    friend class FlashCardApp;
};


class FlashCardApp : public MenuScreen {
public:
    FlashCardApp() : 
    m_UpPoller(VK_UP), 
    m_DownPoller(VK_DOWN), 
    m_EnterPoller(VK_RETURN),
    m_EscPoller(VK_ESCAPE),
    m_IgnoreFirstEnter(true),
    m_CreateCardScreen(m_CardManager),
    m_ReviewCardsScreen(m_CardManager),
    m_CheckProgressScreen(m_CardManager) { }
    
    void run() {
        enterScreen();
    }
private:    
    int m_SelectedOption{0};
    KeyPoller m_UpPoller;
    KeyPoller m_DownPoller;
    KeyPoller m_EnterPoller;
    KeyPoller m_EscPoller;
    bool m_IgnoreFirstEnter;

    CardManager m_CardManager;

    CreateCardScreen m_CreateCardScreen;
    ReviewCardsScreen m_ReviewCardsScreen;
    CheckProgressScreen m_CheckProgressScreen;

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

    void displayContent() override {
        std::cout << "################ Flash Card App ################" << std::endl;
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
        std::cout << "################################################" << std::endl;
    }

    void handleInputs() override {
        m_UpPoller.poll();
        m_DownPoller.poll();
        m_EnterPoller.poll();
        m_EscPoller.poll();

        if(m_EscPoller.keyDown()) {
            exitScreen();
            return;
        }

        if(m_UpPoller.keyDown()) {
            m_SelectedOption--;
            updateScreen();
        } else if(m_DownPoller.keyDown()) {
            m_SelectedOption++;
            updateScreen();
        }

        m_SelectedOption = std::clamp(m_SelectedOption, 0, MenuOperation::OPERATION_COUNT - 1);

        if(m_EnterPoller.keyDown()) {
            executeSelectedOption();
        }
    }

    void executeSelectedOption() {
        if(m_IgnoreFirstEnter) {
            m_IgnoreFirstEnter = false;
            return;
        }

        switch(m_SelectedOption) {
            case MenuOperation::CREATE_CARD:
                m_CreateCardScreen.enterScreen();
                break;
            case MenuOperation::REVIEW_CARDS:
                m_ReviewCardsScreen.enterScreen();
                break;
            case MenuOperation::CHECK_PROGRESS:
                m_CheckProgressScreen.enterScreen();
                break;
            case MenuOperation::LOAD_CARDS:
                m_CardManager.loadCards("cards/deck.dat");
                break;
            case MenuOperation::SAVE_CARDS:
                m_CardManager.saveCards("cards/deck.dat");
                break;
            case MenuOperation::EXIT:
                exitScreen();
                break;
        }

        updateScreen();
    }
};

int main() {
    FlashCardApp app;
    app.run();
    return 0;
}