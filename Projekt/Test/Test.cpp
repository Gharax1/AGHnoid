#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <cmath>
#include <random>
#include <optional>
#include <vector>
#include <fstream>
#include <iostream>

// Definiujemy możliwe stany gry
enum class Stan { MENU, GRA, LEVEL, PAUSE, WIN, LOSE, OPCJE };

// Wybrany poziom muzyki
int music = 2;
int controls = 1;

// Zmienne do przechowywania aktualnej głośności (aby stosować je do nowych utworów)
float currentMenuVol = 30.f;
float currentGameVol = 10.f;

// Licznik punktów
int points = 0;
int wybranyPoziom = 1;

int combo = 0;

//Controls
sf::Keyboard::Key left = sf::Keyboard::Key::Left;
sf::Keyboard::Key right = sf::Keyboard::Key::Right;

//Szyfrowanie
int szyfrowanie(int dane) {
    return dane + 111;
}

int deszyfrowanie(int dane) {
    return dane - 111;
}

// LEVEL UP SYSTEM
int comboTempMax = 0; // maksymalna wartosc jednego combo
int combosum = 0; // suma wszystkich combo
int xpsum = points + 2 * combosum; // 1 xp za zniszczenie bloku, 2 xp za kazdy punkt combo
int score = 0; // 100 punktow za kazdy zniszczony blok, 5 punktów za kazdy punkt combo zwiększone z kazdym poziomem

struct Player {
    int level = 1;
    int xp = 0;
    int xpToNext = 100;
    int highScore = 0;
};

void levelUp(Player& p)
{
    p.level++;
    p.xpToNext = static_cast<int>(p.xpToNext * 1.25f);
}

void addXP(Player& p, int amount)
{
    // Jeśli xpToNext jest zepsute (<= 0), napraw je
    if (p.xpToNext <= 0) {
        p.xpToNext = 100;
    }

    p.xp += amount;

    // Pętla levelowania
    while (p.xp >= p.xpToNext)
    {
        p.xp -= p.xpToNext;
        levelUp(p);

        // Na wypadek gdyby levelUp nie zwiększył progu (mało prawdopodobne, ale bezpieczne)
        if (p.xpToNext <= 0) {
            p.xpToNext = 100;
            break;
        }
    }
}

void saveGame(const Player& p)
{
    int temp;
    std::ofstream file("save.txt");
    if (!file) return;

    

    temp = szyfrowanie(p.level);
    file << temp << "\n";
    temp = szyfrowanie(p.xp);
    file << temp << "\n";
    temp = szyfrowanie(p.xpToNext);
    file << temp << "\n";
    temp = szyfrowanie(p.highScore);
    file << temp << "\n";
    file.close();
}

bool loadGame(Player& p)
{
    int temp;
    std::ifstream file("save.txt");
    if (!file) return false;

    file >> temp;
    temp = deszyfrowanie(temp);
    p.level = temp;
    file >> temp;
    temp = deszyfrowanie(temp);
    p.xp = temp;
    file >> temp;
    temp = deszyfrowanie(temp);
    p.xpToNext = temp;
    file >> temp;
    temp = deszyfrowanie(temp);
    p.highScore = temp;
    file.close();
    return true;
}

int main() {
    const unsigned int szerokosc = 800;
    const unsigned int wysokosc = 600;

    // srand(time(NULL));

    Player player;
    loadGame(player);
    sf::RenderWindow window(sf::VideoMode({ szerokosc, wysokosc }), "AGHnoid");
    window.setFramerateLimit(60);

    Stan aktualnyStan = Stan::MENU;

    // --- POWER UPY ---
    sf::Texture powerUp;
    if (!powerUp.loadFromFile("Sprites/power_up.png")) {
        // return -1; // Opcjonalnie obsługa błędu
    }
    powerUp.setSmooth(true);


    // --- CZCIONKA ---
    sf::Font font;
    if (!font.openFromFile("Fonts/arial.ttf")) {
        // return -1;
    }
    sf::Font PixelFont;
    if (!PixelFont.openFromFile("Fonts/PixelifySans-Regular.ttf")) {
        // return -1;
    }

    // --- TŁO ---
    sf::Texture backgroundTexture;
    if (!backgroundTexture.loadFromFile("Sprites/main_menu.png")) {
        return -1;
    }
    sf::Sprite backgroundSprite(backgroundTexture);
    sf::Vector2u rozmiarObrazka = backgroundTexture.getSize();

    backgroundSprite.setScale(sf::Vector2f(
        static_cast<float>(szerokosc) / rozmiarObrazka.x,
        static_cast<float>(wysokosc) / rozmiarObrazka.y
    ));
    // -------- XP BAR ------------
    int level = player.level;
    int xp = player.xp;
    int xpToNext = player.xpToNext;

    float percent = static_cast<float>(xp) / xpToNext;
    percent = std::clamp(percent, 0.f, 1.f);

    sf::RectangleShape xpBarBg;
    xpBarBg.setSize(sf::Vector2f(310.f, 30.f));
    xpBarBg.setFillColor(sf::Color(0x00, 0x6b, 0x3b));
    xpBarBg.setPosition(sf::Vector2f(15.f, wysokosc - 50.f));
    xpBarBg.setOutlineColor(sf::Color::Black);
    xpBarBg.setOutlineThickness(5.f);

    sf::RectangleShape xpBarBg1;
    xpBarBg1.setSize(sf::Vector2f(300.f, 40.f));
    xpBarBg1.setFillColor(sf::Color(0x00, 0x6b, 0x3b));
    xpBarBg1.setPosition(sf::Vector2f(20.f, wysokosc - 55.f));
    xpBarBg1.setOutlineColor(sf::Color::Black);
    xpBarBg1.setOutlineThickness(5.f);

    sf::RectangleShape xpBarBg2;
    xpBarBg2.setSize(sf::Vector2f(320.f, 20.f));
    xpBarBg2.setFillColor(sf::Color(0x00, 0x6b, 0x3b));
    xpBarBg2.setPosition(sf::Vector2f(10.f, wysokosc - 45.f));
    xpBarBg2.setOutlineColor(sf::Color::Black);
    xpBarBg2.setOutlineThickness(5.f);

    sf::RectangleShape xpBarFill;
    xpBarFill.setSize(sf::Vector2f(300.f * percent, 20.f));
    xpBarFill.setFillColor(sf::Color(0xb0, 0x10, 0x28));
    xpBarFill.setPosition(sf::Vector2f(20.f, wysokosc - 45.f));

    sf::Text levelText(PixelFont);
    levelText.setString("LEVEL " + std::to_string(level));
    levelText.setCharacterSize(32.f);
    levelText.setFillColor(sf::Color(0xb0, 0x10, 0x28));
    levelText.setOutlineColor(sf::Color::Black);
    levelText.setOutlineThickness(5.f);
    levelText.setPosition(sf::Vector2f(10.f, wysokosc - 100.f));

    sf::Text HighscoreText(PixelFont);
    HighscoreText.setString("Highscore: " + std::to_string(player.highScore));
    HighscoreText.setCharacterSize(32.f);
    HighscoreText.setFillColor(sf::Color(0xb0, 0x10, 0x28));
    HighscoreText.setOutlineColor(sf::Color::Black);
    HighscoreText.setOutlineThickness(5.f);
    HighscoreText.setPosition(sf::Vector2f(szerokosc - 250.f, wysokosc - 50.f));

    // ---- GAME OVER -------

    sf::Texture GameOverTexture;
    if (!GameOverTexture.loadFromFile("Sprites/game_over.png")) {}
    GameOverTexture.setSmooth(true);

    float GOTwidth = static_cast<float>(GameOverTexture.getSize().x) / 2.f;
    float GOTheight = static_cast<float>(GameOverTexture.getSize().y) / 2.f;

    sf::Sprite GameOverSprite(GameOverTexture);
    GameOverSprite.setOrigin(sf::Vector2f(GOTwidth, GOTheight));
    GameOverSprite.setPosition(sf::Vector2f(szerokosc / 2.f, 150.f));
    GameOverSprite.setScale(sf::Vector2f(0.6f, 0.6f));


    // ------- WIN ----------
    sf::Texture WinTexture;
    if (!WinTexture.loadFromFile("Sprites/win.png")) {}
    WinTexture.setSmooth(true);

    float WinTwidth = static_cast<float>(WinTexture.getSize().x) / 2.f;
    float WinTheight = static_cast<float>(WinTexture.getSize().y) / 2.f;

    sf::Sprite WinSprite(WinTexture);
    WinSprite.setOrigin(sf::Vector2f(WinTwidth, WinTheight));
    WinSprite.setPosition(sf::Vector2f(szerokosc / 2.f, 150.f));
    WinSprite.setScale(sf::Vector2f(0.6f, 0.6f));


    // ==================================================
    // --- ANIMOWANY TYTUŁ ---
    // ==================================================
    sf::Texture titleTexture;
    if (!titleTexture.loadFromFile("Sprites/title1.png")) {
        return -1;
    }
    titleTexture.setSmooth(true);

    int frameWidth = titleTexture.getSize().x / 2;
    int frameHeight = titleTexture.getSize().y;

    sf::Sprite titleSprite(titleTexture);
    titleSprite.setTextureRect(sf::IntRect(sf::Vector2i(0, 0), sf::Vector2i(frameWidth, frameHeight)));
    titleSprite.setOrigin(sf::Vector2f(frameWidth / 2.f, frameHeight / 2.f));
    titleSprite.setPosition(sf::Vector2f(szerokosc / 2.f, 100.f));
    titleSprite.setScale(sf::Vector2f(0.5f, 0.5f));

    sf::Clock titleClock;
    int currentTitleFrame = 0;


    // ==================================================
    // --- AUDIO ---
    // ==================================================
    sf::SoundBuffer hitBuffer;
    if (!hitBuffer.loadFromFile("Sounds/Hit/hit1.wav")) {}
    sf::Sound hitSound(hitBuffer);
    hitSound.setVolume(50.f);

    sf::SoundBuffer powerBuffer;
    if (!powerBuffer.loadFromFile("Sounds/PowerUp/powerUp.wav")) {}
    sf::Sound powerSound(powerBuffer);
    powerSound.setVolume(50.f);

    // --- MUZYKA ---
    sf::Music menuMusic;
    if (menuMusic.openFromFile("Music/menu.mp3")) {
        menuMusic.setLooping(true);
        menuMusic.setVolume(currentMenuVol);
        menuMusic.play();
    }

    // Muzyka dla poziomów
    sf::Music musicLvl1;
    if (musicLvl1.openFromFile("Music/lvl1.mp3")) {
        musicLvl1.setLooping(true);
        musicLvl1.setVolume(currentGameVol);
    }

    sf::Music musicLvl2;
    if (musicLvl2.openFromFile("Music/lvl2.mp3")) {
        musicLvl2.setLooping(true);
        musicLvl2.setVolume(currentGameVol);
    }

    sf::Music musicLvl3;
    if (musicLvl3.openFromFile("Music/lvl3.mp3")) {
        musicLvl3.setLooping(true);
        musicLvl3.setVolume(currentGameVol);
    }

    // Muzyka Win / Lose
    sf::Music musicWin;
    if (musicWin.openFromFile("Music/win.mp3")) {
        musicWin.setLooping(false); 
        musicWin.setVolume(currentGameVol);
    }

    sf::Music musicLose;
    if (musicLose.openFromFile("Music/lose.mp3")) {
        musicLose.setLooping(false);
        musicLose.setVolume(currentGameVol);
    }

    // Wskaźnik, który będzie trzymał aktualnie graną muzykę poziomu
    sf::Music* currentBgMusic = &musicLvl1;

    // Funkcja pomocnicza do aktualizacji głośności WSZYSTKICH muzyczek gry (nie menu)
    auto updateGameVolume = [&](float vol) {
        currentGameVol = vol;
        musicLvl1.setVolume(vol);
        musicLvl2.setVolume(vol);
        musicLvl3.setVolume(vol);
        musicWin.setVolume(vol);
        musicLose.setVolume(vol);
        };


    // ==================================================
    // --- ELEMENTY PRZYCISKÓW ---
    // ==================================================

    // 1. Ładowanie tekstur
    sf::Texture texBtnPlay;
    if (!texBtnPlay.loadFromFile("Sprites/btn_play.png")) { return -1; }

    sf::Texture texBtnExit;
    if (!texBtnExit.loadFromFile("Sprites/btn_exit.png")) { return -1; }

    sf::Texture texBtnResume;
    if (!texBtnResume.loadFromFile("Sprites/btn_resume.png")) { return -1; }

    sf::Texture texBtnRestart;
    if (!texBtnRestart.loadFromFile("Sprites/btn_restart.png")) { return -1; }

    sf::Texture texBtnOptions;
    if (!texBtnOptions.loadFromFile("Sprites/btn_options.png")) { return -1; }

    texBtnPlay.setSmooth(true);
    texBtnExit.setSmooth(true);
    texBtnResume.setSmooth(true);
    texBtnRestart.setSmooth(true);
    texBtnOptions.setSmooth(true);


    // --- PRZYCISKI MENU GŁÓWNEGO ---

    sf::RectangleShape btnPlay(sf::Vector2f(350.f, 60.f));
    btnPlay.setTexture(&texBtnPlay);
    btnPlay.setFillColor(sf::Color::White);
    btnPlay.setOrigin(sf::Vector2f(175.f, 30.f));
    btnPlay.setPosition(sf::Vector2f(szerokosc / 2.f, wysokosc / 2.f - 30.f));

    sf::RectangleShape btnExit(sf::Vector2f(250.f, 60.f));
    btnExit.setTexture(&texBtnExit);
    btnExit.setFillColor(sf::Color::White);
    btnExit.setOrigin(sf::Vector2f(125.f, 30.f));
    btnExit.setPosition(sf::Vector2f(szerokosc / 2.f, wysokosc / 2.f + 50.f));

    sf::RectangleShape btnOptions(sf::Vector2f(220.f, 60.f));
    btnOptions.setTexture(&texBtnOptions);
    btnOptions.setFillColor(sf::Color::White);
    btnOptions.setOrigin(sf::Vector2f(110.f, 30.f));
    btnOptions.setPosition(sf::Vector2f(szerokosc / 2.f, wysokosc / 2.f + 130.f));

    // --- PRZYCISKI OPCJI ---

    sf::Texture texBtnMute;
    if (!texBtnMute.loadFromFile("Sprites/btn_mute.png")) { return -1; }
    texBtnMute.setSmooth(true);

    sf::Texture texBtnLow;
    if (!texBtnLow.loadFromFile("Sprites/btn_low.png")) { return -1; }
    texBtnLow.setSmooth(true);

    sf::Texture texBtnMedium;
    if (!texBtnMedium.loadFromFile("Sprites/btn_medium.png")) { return -1; }
    texBtnMedium.setSmooth(true);

    sf::Texture texBtnLoud;
    if (!texBtnLoud.loadFromFile("Sprites/btn_loud.png")) { return -1; }
    texBtnLoud.setSmooth(true);

    sf::Texture texBtnAD;
    if (!texBtnAD.loadFromFile("Sprites/btn_ad.png")) { return -1; }
    texBtnAD.setSmooth(true);

    sf::Texture texBtnARR;
    if (!texBtnARR.loadFromFile("Sprites/btn_arr.png")) { return -1; }
    texBtnARR.setSmooth(true);

    sf::Texture texBtnMusic;
    if (!texBtnMusic.loadFromFile("Sprites/btn_music.png")) { return -1; }
    texBtnMusic.setSmooth(true);

    sf::Texture texBtnCtrl;
    if (!texBtnCtrl.loadFromFile("Sprites/btn_ctrl.png")) { return -1; }
    texBtnCtrl.setSmooth(true);

    sf::RectangleShape btnMusic(sf::Vector2f(250.f, 60.f));
    btnMusic.setTexture(&texBtnMusic);
    btnMusic.setFillColor(sf::Color::White);
    btnMusic.setOrigin(sf::Vector2f(125.f, 30.f));
    btnMusic.setPosition(sf::Vector2f(125.f, wysokosc / 2.f));

    sf::RectangleShape btnTurnedOff(sf::Vector2f(60.f, 60.f));
    btnTurnedOff.setTexture(&texBtnMute);
    btnTurnedOff.setFillColor(sf::Color::White);
    btnTurnedOff.setOrigin(sf::Vector2f(30.f, 30.f));
    btnTurnedOff.setPosition(sf::Vector2f(szerokosc / 2.0f - 112.5f, wysokosc / 2.f));

    sf::RectangleShape btnQuiet(sf::Vector2f(60.f, 60.f));
    btnQuiet.setTexture(&texBtnLow);
    btnQuiet.setFillColor(sf::Color::White);
    btnQuiet.setOrigin(sf::Vector2f(30.f, 30.f));
    btnQuiet.setPosition(sf::Vector2f(szerokosc / 2.0f - 37.5f, wysokosc / 2.f));

    sf::RectangleShape btnMedium(sf::Vector2f(60.f, 60.f));
    btnMedium.setTexture(&texBtnMedium);
    btnMedium.setFillColor(sf::Color::White);
    btnMedium.setOrigin(sf::Vector2f(30.f, 30.f));
    btnMedium.setPosition(sf::Vector2f(szerokosc / 2.0f + 37.5f, wysokosc / 2.f));

    sf::RectangleShape btnLoud(sf::Vector2f(60.f, 60.f));
    btnLoud.setTexture(&texBtnLoud);
    btnLoud.setFillColor(sf::Color::White);
    btnLoud.setOrigin(sf::Vector2f(30.f, 30.f));
    btnLoud.setPosition(sf::Vector2f(szerokosc / 2.0f + 112.5f, wysokosc / 2.f));

    sf::RectangleShape btnBack(sf::Vector2f(250.f, 60.f));
    btnBack.setTexture(&texBtnExit);
    btnBack.setFillColor(sf::Color::White);
    btnBack.setOrigin(sf::Vector2f(125.f, 30.f));
    btnBack.setPosition(sf::Vector2f(szerokosc / 2.f, wysokosc / 2.f + 130.f));

    sf::RectangleShape btnControls(sf::Vector2f(250.f, 60.f));
    btnControls.setTexture(&texBtnCtrl);
    btnControls.setFillColor(sf::Color::White);
    btnControls.setOrigin(sf::Vector2f(125.f, 30.f));
    btnControls.setPosition(sf::Vector2f(125.0f, wysokosc / 2.f - 70.0f));

    sf::RectangleShape btnWSAD(sf::Vector2f(100.f, 60.f));
    btnWSAD.setTexture(&texBtnAD);
    btnWSAD.setFillColor(sf::Color::White);
    btnWSAD.setOrigin(sf::Vector2f(50.f, 30.f));
    btnWSAD.setPosition(sf::Vector2f(szerokosc / 2.f - 60.0f, wysokosc / 2.f - 70.0f));

    sf::RectangleShape btnArrows(sf::Vector2f(100.f, 60.f));
    btnArrows.setTexture(&texBtnARR);
    btnArrows.setFillColor(sf::Color::White);
    btnArrows.setOrigin(sf::Vector2f(50.f, 30.f));
    btnArrows.setPosition(sf::Vector2f(szerokosc / 2.f + 60.0f, wysokosc / 2.f - 70.0f));

    // --- ELEMENTY WYBORU POZIOMU ---

    sf::Texture texBtnLvl1;
    if (!texBtnLvl1.loadFromFile("Sprites/btn_lvl1.png")) { return -1; }
    texBtnLvl1.setSmooth(true);

    sf::Texture texBtnLvl2;
    if (!texBtnLvl2.loadFromFile("Sprites/btn_lvl2.png")) { return -1; }
    texBtnLvl2.setSmooth(true);

    sf::Texture texBtnLvl3;
    if (!texBtnLvl3.loadFromFile("Sprites/btn_lvl3.png")) { return -1; }
    texBtnLvl3.setSmooth(true);

    sf::RectangleShape btnLvl1(sf::Vector2f(250.f, 60.f));
    btnLvl1.setTexture(&texBtnLvl1);
    btnLvl1.setFillColor(sf::Color::White);
    btnLvl1.setOrigin(sf::Vector2f(125.f, 30.f));
    btnLvl1.setPosition(sf::Vector2f(szerokosc / 2.f, wysokosc / 2.f - 120.f));

    sf::RectangleShape btnLvl2(sf::Vector2f(250.f, 60.f));
    btnLvl2.setTexture(&texBtnLvl2);
    btnLvl2.setFillColor(sf::Color::White);
    btnLvl2.setOrigin(sf::Vector2f(125.f, 30.f));
    btnLvl2.setPosition(sf::Vector2f(szerokosc / 2.f, wysokosc / 2.f - 40.f));

    sf::RectangleShape btnLvl3(sf::Vector2f(250.f, 60.f));
    btnLvl3.setTexture(&texBtnLvl3);
    btnLvl3.setFillColor(sf::Color::White);
    btnLvl3.setOrigin(sf::Vector2f(125.f, 30.f));
    btnLvl3.setPosition(sf::Vector2f(szerokosc / 2.f, wysokosc / 2.f + 40.f));


    sf::RectangleShape btnLvlBack(sf::Vector2f(250.f, 60.f));
    btnLvlBack.setTexture(&texBtnExit);
    btnLvlBack.setFillColor(sf::Color::White);
    btnLvlBack.setOrigin(sf::Vector2f(125.f, 30.f));
    btnLvlBack.setPosition(sf::Vector2f(szerokosc / 2.f, wysokosc / 2.f + 120.f));




    // --- ELEMENTY PAUZY ---

    sf::RectangleShape dimmer(sf::Vector2f((float)szerokosc, (float)wysokosc));
    dimmer.setFillColor(sf::Color(0, 0, 0, 150));

    sf::RectangleShape btnResume(sf::Vector2f(250.f, 60.f));
    btnResume.setTexture(&texBtnResume);
    btnResume.setFillColor(sf::Color::White);
    btnResume.setOrigin(sf::Vector2f(125.f, 30.f));
    btnResume.setPosition(sf::Vector2f(szerokosc / 2.f, wysokosc / 2.f - 80.f));


    sf::RectangleShape btnRestart(sf::Vector2f(250.f, 60.f));
    btnRestart.setTexture(&texBtnRestart);
    btnRestart.setFillColor(sf::Color::White);
    btnRestart.setOrigin(sf::Vector2f(125.f, 30.f));
    btnRestart.setPosition(sf::Vector2f(szerokosc / 2.f, wysokosc / 2.f));


    sf::RectangleShape btnPauseExit(sf::Vector2f(250.f, 60.f));
    btnPauseExit.setTexture(&texBtnExit);
    btnPauseExit.setFillColor(sf::Color::White);
    btnPauseExit.setOrigin(sf::Vector2f(125.f, 30.f));
    btnPauseExit.setPosition(sf::Vector2f(szerokosc / 2.f, wysokosc / 2.f + 80.f));


    // ------------- PRZYCISKI GAME OVER/WIN ---------------- 

    sf::RectangleShape btnGameOverRestart(sf::Vector2f(250.f, 60.f));
    btnGameOverRestart.setTexture(&texBtnRestart);
    btnGameOverRestart.setFillColor(sf::Color::White);
    btnGameOverRestart.setOrigin(sf::Vector2f(125.f, 30.f));
    btnGameOverRestart.setPosition(sf::Vector2f(szerokosc / 2.f, wysokosc / 2.f + 40.f));


    sf::RectangleShape btnGameOverExit(sf::Vector2f(250.f, 60.f));
    btnGameOverExit.setTexture(&texBtnExit);
    btnGameOverExit.setFillColor(sf::Color::White);
    btnGameOverExit.setOrigin(sf::Vector2f(125.f, 30.f));
    btnGameOverExit.setPosition(sf::Vector2f(szerokosc / 2.f, wysokosc / 2.f + 120.f));

    sf::Text ScoreText(PixelFont);
    ScoreText.setCharacterSize(32.f);
    ScoreText.setFillColor(sf::Color(0xb0, 0x10, 0x28));
    ScoreText.setOutlineColor(sf::Color::Black);
    ScoreText.setOutlineThickness(5.0f);
    ScoreText.setString("Score: " + std::to_string(score));

    sf::Text xpText(PixelFont);
    xpText.setCharacterSize(32.f);
    xpText.setFillColor(sf::Color(0xb0, 0x10, 0x28));
    xpText.setOutlineColor(sf::Color::Black);
    xpText.setOutlineThickness(5.0f);
    xpText.setPosition(sf::Vector2f(szerokosc / 2 - 85.f, wysokosc / 2 - 40.f));
    xpText.setString("XP gained: " + std::to_string(xpsum));

    // -------------- COMBO ----------------------
    sf::Text ComboText(PixelFont);
    ComboText.setCharacterSize(32);
    ComboText.setFillColor(sf::Color(0xb0, 0x10, 0x28));
    ComboText.setOutlineColor(sf::Color::Black);
    ComboText.setOutlineThickness(5.0f);

    // --- GRACZ I OBIEKTY ---
    sf::Texture kulkaTexture;
    if (kulkaTexture.loadFromFile("Sprites/kulka_1.png")) {
        kulkaTexture.setSmooth(true);
    }
    float promien = 8.f;
    sf::CircleShape kulka(promien);
    kulka.setTexture(&kulkaTexture);
    kulka.setFillColor(sf::Color::White);
    kulka.setOrigin(sf::Vector2f(promien, promien));

    sf::Texture paletkaTexture;
    if (paletkaTexture.loadFromFile("Sprites/paletka.png")) {}
    sf::Vector2f paletkaRozmiar(120.f, 15.f);
    sf::RectangleShape paletka(paletkaRozmiar);
    paletka.setTexture(&paletkaTexture);
    paletka.setFillColor(sf::Color::White);
    paletka.setOrigin(sf::Vector2f(paletkaRozmiar.x / 2.f, (paletkaRozmiar.y / 2.f) + 0.5f));


    sf::Vector2f predkosc(0.f, 0.f);
    bool czyW_Ruchu = false;
    float moc = 6.0f;

    struct Bloczki {
        sf::RectangleShape shape;
        bool zniszczony = false;
        int health_points;
        bool moving = false;
    };
    std::vector<Bloczki> bloczki;


    sf::Texture blokTexture;
    if (blokTexture.loadFromFile("Sprites/bloczki_4.png")) {}

    sf::Texture Blok1hp;
    if (Blok1hp.loadFromFile("Sprites/bloczki_4_1hp.png")) {}

    sf::Texture Blok2hp;
    if (Blok2hp.loadFromFile("Sprites/bloczki_4_2hp.png")) {}

    sf::Texture Blok3hp;
    if (Blok3hp.loadFromFile("Sprites/bloczki_4_3hp.png")) {}

    // Power upy
    struct Power {
        sf::CircleShape shape;
        bool used = false;
        float factor;
        bool moving = false;
    };
    std::vector<Power> power;


    auto resetGry = [&](int lvl) {
        moc = 6.0f;
        points = 0;
        combo = 0;
        combosum = 0;
        comboTempMax = 0;
        score = 0;
        kulka.setPosition(sf::Vector2f(szerokosc / 2.f, wysokosc / 2.f + 50.f));
        paletka.setPosition(sf::Vector2f(szerokosc / 2.f, wysokosc - 40.f));
        predkosc = sf::Vector2f(0.f, 0.f);
        czyW_Ruchu = false;

        power.clear();
        bloczki.clear();

        // LOGIKA DLA LVL 3
        int rzedy = 3;
        if (lvl == 2) rzedy = 5;
        if (lvl == 3) rzedy = 8;


        for (int rzad = 0; rzad < rzedy; ++rzad) {
            for (int kol = 0; kol < 10; ++kol) {
                Bloczki b;
                b.shape.setSize(sf::Vector2f(70.f, 25.f));
                //b.shape.setTexture(&blokTexture);
                if (lvl == 1) b.health_points = 1;
                if ((lvl == 2) && (rzad % 2 == 0)) b.health_points = 2;
                if ((lvl == 3) && (rzad % 3 == 0)) b.health_points = 2;
                if ((lvl == 3) && (rzad % 5 == 0)) b.health_points = 3;

                if (b.health_points == 1) b.shape.setTexture(&Blok1hp);
                else if (b.health_points == 2) b.shape.setTexture(&Blok2hp);
                else if (b.health_points == 3) b.shape.setTexture(&Blok3hp);

                switch (rzad % 3) {
                case 0: b.shape.setFillColor(sf::Color(0x00, 0x6b, 0x3b)); break;
                case 1: b.shape.setFillColor(sf::Color(0x23, 0x1f, 0x20)); break;
                case 2: b.shape.setFillColor(sf::Color(0xb0, 0x10, 0x28)); break;
                }
                b.shape.setPosition(sf::Vector2f(40.f + kol * 75.f, 50.f + rzad * 30.f));
                bloczki.push_back(b);
            }
        }
        };


    resetGry(1);
    std::random_device rd;
    std::mt19937 gen(rd());

    srand(time(NULL));




    // --- PĘTLA GŁÓWNA ---
    while (window.isOpen()) {
        


        sf::Vector2f mousePos = window.mapPixelToCoords(sf::Mouse::getPosition(window));

        if (aktualnyStan == Stan::MENU) {
            if (titleClock.getElapsedTime().asSeconds() >= 1.0f) {
                currentTitleFrame = (currentTitleFrame + 1) % 2;
                int left = currentTitleFrame * frameWidth;
                titleSprite.setTextureRect(sf::IntRect(sf::Vector2i(left, 0), sf::Vector2i(frameWidth, frameHeight)));
                titleClock.restart();
            }
        }


        while (const std::optional event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) {
                window.close();
                saveGame(player);
            }
            if (event->is<sf::Event::KeyPressed>()) {
                if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Escape)) {
                    if (aktualnyStan == Stan::GRA) {
                        aktualnyStan = Stan::PAUSE;
                        // Zatrzymaj muzykę tła
                        if (currentBgMusic) currentBgMusic->pause();
                    }
                    else if (aktualnyStan == Stan::PAUSE) {
                        aktualnyStan = Stan::GRA;
                        // Wznów muzykę tła
                        if (currentBgMusic) currentBgMusic->play();
                    }
                }
            }


            if (aktualnyStan == Stan::MENU) {
                loadGame(player);
                if (const auto* mouseEvent = event->getIf<sf::Event::MouseButtonPressed>()) {
                    if (mouseEvent->button == sf::Mouse::Button::Left) {
                        if (btnPlay.getGlobalBounds().contains(mousePos)) {
                            aktualnyStan = Stan::LEVEL;
                        }
                        if (btnExit.getGlobalBounds().contains(mousePos)) {
                            window.close();
                            saveGame(player);
                        }
                        if (btnOptions.getGlobalBounds().contains(mousePos)) {
                            aktualnyStan = Stan::OPCJE;
                        }
                        if (levelText.getGlobalBounds().contains(mousePos)) {
                            std::cout << "Well done, you discovered an easter egg." << std::endl << "You gain *drums* nothing at all, but congratulations nonetheless." << std::endl;
                        }
                    }
                }
            }
            else if (aktualnyStan == Stan::OPCJE) {
                if (const auto* mouseEvent = event->getIf<sf::Event::MouseButtonPressed>()) {
                    if (mouseEvent->button == sf::Mouse::Button::Left) {
                        if (btnBack.getGlobalBounds().contains(mousePos)) {
                            aktualnyStan = Stan::MENU;
                        }
                        if (btnTurnedOff.getGlobalBounds().contains(mousePos)) {
                            menuMusic.setVolume(0.0f);
                            currentMenuVol = 0.0f;
                            updateGameVolume(0.0f);
                            music = 0;
                        }
                        if (btnQuiet.getGlobalBounds().contains(mousePos)) {
                            menuMusic.setVolume(15.0f);
                            currentMenuVol = 15.0f;
                            updateGameVolume(5.0f);
                            music = 1;
                        }
                        if (btnMedium.getGlobalBounds().contains(mousePos)) {
                            menuMusic.setVolume(30.0f);
                            currentMenuVol = 30.0f;
                            updateGameVolume(10.0f);
                            music = 2;
                        }
                        if (btnLoud.getGlobalBounds().contains(mousePos)) {
                            menuMusic.setVolume(45.0f);
                            currentMenuVol = 45.0f;
                            updateGameVolume(15.0f);
                            music = 3;
                        }
                        if (btnWSAD.getGlobalBounds().contains(mousePos)) {
                            left = sf::Keyboard::Key::A;
                            right = sf::Keyboard::Key::D;
                            controls = 0;

                        }
                        if (btnArrows.getGlobalBounds().contains(mousePos)) {
                            sf::Keyboard::Key left = sf::Keyboard::Key::Left;
                            sf::Keyboard::Key right = sf::Keyboard::Key::Right;
                            controls = 1;
                        }
                    }
                }
            }
            else if (aktualnyStan == Stan::LEVEL) {
                if (const auto* mouseEvent = event->getIf<sf::Event::MouseButtonPressed>()) {
                    if (mouseEvent->button == sf::Mouse::Button::Left) {
                        bool levelSelected = false;

                        if (btnLvl1.getGlobalBounds().contains(mousePos)) {
                            wybranyPoziom = 1;
                            currentBgMusic = &musicLvl1; // Wybierz muzykę lvl 1
                            levelSelected = true;
                        }
                        else if (btnLvl2.getGlobalBounds().contains(mousePos)) {
                            wybranyPoziom = 2;
                            currentBgMusic = &musicLvl2; // Wybierz muzykę lvl 2
                            levelSelected = true;
                        }
                        else if (btnLvl3.getGlobalBounds().contains(mousePos)) {
                            wybranyPoziom = 3;
                            currentBgMusic = &musicLvl3; // Wybierz muzykę lvl 3
                            levelSelected = true;
                        }

                        if (levelSelected) {
                            resetGry(wybranyPoziom);
                            menuMusic.stop();

                            // Zatrzymaj poprzednie i włącz nową muzykę
                            musicLvl1.stop();
                            musicLvl2.stop();
                            musicLvl3.stop();
                            if (currentBgMusic) currentBgMusic->play();

                            aktualnyStan = Stan::GRA;
                        }

                        if (btnLvlBack.getGlobalBounds().contains(mousePos)) {
                            aktualnyStan = Stan::MENU;
                        }
                    }
                }
            }
            else if (aktualnyStan == Stan::GRA) {
                if (event->is<sf::Event::KeyPressed>() && !czyW_Ruchu) {
                    predkosc = sf::Vector2f(0.f, -moc);
                    czyW_Ruchu = true;
                }
            }
            else if (aktualnyStan == Stan::PAUSE) {
                if (const auto* mouseEvent = event->getIf<sf::Event::MouseButtonPressed>()) {
                    if (mouseEvent->button == sf::Mouse::Button::Left) {
                        if (btnResume.getGlobalBounds().contains(mousePos)) {
                            aktualnyStan = Stan::GRA;
                            if (currentBgMusic) currentBgMusic->play();
                        }
                        if (btnRestart.getGlobalBounds().contains(mousePos)) {
                            resetGry(wybranyPoziom);
                            aktualnyStan = Stan::GRA;
                            if (currentBgMusic) {
                                currentBgMusic->stop();
                                currentBgMusic->play();
                            }
                        }
                        if (btnPauseExit.getGlobalBounds().contains(mousePos)) {
                            aktualnyStan = Stan::MENU;
                            if (currentBgMusic) currentBgMusic->stop();
                            menuMusic.play();
                        }
                    }
                }
            }
            else if (aktualnyStan == Stan::LOSE || aktualnyStan == Stan::WIN) {
                if (const auto* mouseEvent = event->getIf<sf::Event::MouseButtonPressed>()) {
                    if (mouseEvent->button == sf::Mouse::Button::Left) {
                        if (btnGameOverRestart.getGlobalBounds().contains(mousePos)) {
                            resetGry(wybranyPoziom);
                            aktualnyStan = Stan::GRA;

                            // Stop Win/Lose music i start level music
                            musicWin.stop();
                            musicLose.stop();
                            if (currentBgMusic) {
                                currentBgMusic->stop();
                                currentBgMusic->play();
                            }
                        }
                        if (btnGameOverExit.getGlobalBounds().contains(mousePos)) {
                            aktualnyStan = Stan::MENU;
                            // Stop Win/Lose music
                            musicWin.stop();
                            musicLose.stop();
                            menuMusic.play();
                        }
                    }
                }
            }
        }


        // --- HOVER LOGIC ---
        if (aktualnyStan == Stan::MENU) {
            if (btnPlay.getGlobalBounds().contains(mousePos)) {
                btnPlay.setFillColor(sf::Color(220, 220, 220));
                btnPlay.setScale(sf::Vector2f(1.05f, 1.05f));
            }
            else {
                btnPlay.setFillColor(sf::Color::White);
                btnPlay.setScale(sf::Vector2f(1.0f, 1.0f));
            }
            if (btnExit.getGlobalBounds().contains(mousePos)) {
                btnExit.setFillColor(sf::Color(220, 220, 220));
                btnExit.setScale(sf::Vector2f(1.05f, 1.05f));
            }
            else {
                btnExit.setFillColor(sf::Color::White);
                btnExit.setScale(sf::Vector2f(1.0f, 1.0f));
            }
            if (btnOptions.getGlobalBounds().contains(mousePos)) {
                btnOptions.setFillColor(sf::Color(220, 220, 220));
                btnOptions.setScale(sf::Vector2f(1.05f, 1.05f));
            }
            else {
                btnOptions.setFillColor(sf::Color::White);
                btnOptions.setScale(sf::Vector2f(1.0f, 1.0f));
            }
        }


        if (aktualnyStan == Stan::LEVEL) {
            if (btnLvl1.getGlobalBounds().contains(mousePos)) {
                btnLvl1.setFillColor(sf::Color(220, 220, 220));
                btnLvl1.setScale(sf::Vector2f(1.05f, 1.05f));
            }
            else {
                btnLvl1.setFillColor(sf::Color::White);
                btnLvl1.setScale(sf::Vector2f(1.0f, 1.0f));
            }
            if (btnLvl2.getGlobalBounds().contains(mousePos)) {
                btnLvl2.setFillColor(sf::Color(220, 220, 220));
                btnLvl2.setScale(sf::Vector2f(1.05f, 1.05f));
            }
            else {
                btnLvl2.setFillColor(sf::Color::White);
                btnLvl2.setScale(sf::Vector2f(1.0f, 1.0f));
            }

            if (btnLvl3.getGlobalBounds().contains(mousePos)) {
                btnLvl3.setFillColor(sf::Color(220, 220, 220));
                btnLvl3.setScale(sf::Vector2f(1.05f, 1.05f));
            }
            else {
                btnLvl3.setFillColor(sf::Color::White);
                btnLvl3.setScale(sf::Vector2f(1.0f, 1.0f));
            }
            if (btnLvlBack.getGlobalBounds().contains(mousePos)) {
                btnLvlBack.setFillColor(sf::Color(220, 220, 220));
                btnLvlBack.setScale(sf::Vector2f(1.05f, 1.05f));
            }
            else {
                btnLvlBack.setFillColor(sf::Color::White);
                btnLvlBack.setScale(sf::Vector2f(1.0f, 1.0f));
            }
        }


        if (aktualnyStan == Stan::PAUSE) {
            if (btnResume.getGlobalBounds().contains(mousePos)) {
                btnResume.setFillColor(sf::Color(220, 220, 220));
                btnResume.setScale(sf::Vector2f(1.05f, 1.05f));
            }
            else {
                btnResume.setFillColor(sf::Color::White);
                btnResume.setScale(sf::Vector2f(1.0f, 1.0f));
            }
            if (btnRestart.getGlobalBounds().contains(mousePos)) {
                btnRestart.setFillColor(sf::Color(220, 220, 220));
                btnRestart.setScale(sf::Vector2f(1.05f, 1.05f));
            }
            else {
                btnRestart.setFillColor(sf::Color::White);
                btnRestart.setScale(sf::Vector2f(1.0f, 1.0f));
            }
            if (btnPauseExit.getGlobalBounds().contains(mousePos)) {
                btnPauseExit.setFillColor(sf::Color(220, 220, 220));
                btnPauseExit.setScale(sf::Vector2f(1.05f, 1.05f));
            }
            else {
                btnPauseExit.setFillColor(sf::Color::White);
                btnPauseExit.setScale(sf::Vector2f(1.0f, 1.0f));
            }
        }
        if (aktualnyStan == Stan::LOSE) {
            if (btnGameOverExit.getGlobalBounds().contains(mousePos)) {
                btnGameOverExit.setFillColor(sf::Color(220, 220, 220));
                btnGameOverExit.setScale(sf::Vector2f(1.05f, 1.05f));
            }
            else {
                btnGameOverExit.setFillColor(sf::Color::White);
                btnGameOverExit.setScale(sf::Vector2f(1.0f, 1.0f));
            }
            if (btnGameOverRestart.getGlobalBounds().contains(mousePos)) {
                btnGameOverRestart.setFillColor(sf::Color(220, 220, 220));
                btnGameOverRestart.setScale(sf::Vector2f(1.05f, 1.05f));
            }
            else {
                btnGameOverRestart.setFillColor(sf::Color::White);
                btnGameOverRestart.setScale(sf::Vector2f(1.0f, 1.0f));
            }
        }
        if (aktualnyStan == Stan::WIN) {
            if (btnGameOverExit.getGlobalBounds().contains(mousePos)) {
                btnGameOverExit.setFillColor(sf::Color(220, 220, 220));
                btnGameOverExit.setScale(sf::Vector2f(1.05f, 1.05f));
            }
            else {
                btnGameOverExit.setFillColor(sf::Color::White);
                btnGameOverExit.setScale(sf::Vector2f(1.0f, 1.0f));
            }
            if (btnGameOverRestart.getGlobalBounds().contains(mousePos)) {
                btnGameOverRestart.setFillColor(sf::Color(220, 220, 220));
                btnGameOverRestart.setScale(sf::Vector2f(1.05f, 1.05f));
            }
            else {
                btnGameOverRestart.setFillColor(sf::Color::White);
                btnGameOverRestart.setScale(sf::Vector2f(1.0f, 1.0f));
            }
        }
        if (aktualnyStan == Stan::OPCJE) {
            if (btnBack.getGlobalBounds().contains(mousePos)) {
                btnBack.setFillColor(sf::Color(220, 220, 220));
                btnBack.setScale(sf::Vector2f(1.05f, 1.05f));
            }
            else {
                btnBack.setFillColor(sf::Color::White);
                btnBack.setScale(sf::Vector2f(1.0f, 1.0f));
            }
            if (btnTurnedOff.getGlobalBounds().contains(mousePos) && music != 0) {
                btnTurnedOff.setFillColor(sf::Color(220, 220, 220));
                btnTurnedOff.setScale(sf::Vector2f(1.05f, 1.05f));
            }
            else {
                btnTurnedOff.setFillColor(sf::Color::White);
                btnTurnedOff.setScale(sf::Vector2f(1.0f, 1.0f));
            }
            if (btnQuiet.getGlobalBounds().contains(mousePos) && music != 1) {
                btnQuiet.setFillColor(sf::Color(220, 220, 220));
                btnQuiet.setScale(sf::Vector2f(1.05f, 1.05f));
            }
            else {
                btnQuiet.setFillColor(sf::Color::White);
                btnQuiet.setScale(sf::Vector2f(1.0f, 1.0f));
            }
            if (btnMedium.getGlobalBounds().contains(mousePos) && music != 2) {
                btnMedium.setFillColor(sf::Color(220, 220, 220));
                btnMedium.setScale(sf::Vector2f(1.05f, 1.05f));
            }
            else {
                btnMedium.setFillColor(sf::Color::White);
                btnMedium.setScale(sf::Vector2f(1.0f, 1.0f));
            }
            if (btnLoud.getGlobalBounds().contains(mousePos) && music != 3) {
                btnLoud.setFillColor(sf::Color(220, 220, 220));
                btnLoud.setScale(sf::Vector2f(1.05f, 1.05f));
            }
            else {
                btnLoud.setFillColor(sf::Color::White);
                btnLoud.setScale(sf::Vector2f(1.0f, 1.0f));
            }
            if (btnWSAD.getGlobalBounds().contains(mousePos) && controls != 0) {
                btnWSAD.setFillColor(sf::Color(220, 220, 220));
                btnWSAD.setScale(sf::Vector2f(1.05f, 1.05f));
            }
            else {
                btnWSAD.setFillColor(sf::Color::White);
                btnWSAD.setScale(sf::Vector2f(1.0f, 1.0f));
            }
            if (btnArrows.getGlobalBounds().contains(mousePos) && controls != 1) {
                btnArrows.setFillColor(sf::Color(220, 220, 220));
                btnArrows.setScale(sf::Vector2f(1.05f, 1.05f));
            }
            else {
                btnArrows.setFillColor(sf::Color::White);
                btnArrows.setScale(sf::Vector2f(1.0f, 1.0f));
            }

            if (music == 0) {
                btnTurnedOff.setFillColor(sf::Color(180, 180, 180));
                btnTurnedOff.setScale(sf::Vector2f(.90f, .90f));
            }
            else if (music == 1) {
                btnQuiet.setFillColor(sf::Color(180, 180, 180));
                btnQuiet.setScale(sf::Vector2f(.90f, .90f));
            }
            else if (music == 2) {
                btnMedium.setFillColor(sf::Color(180, 180, 180));
                btnMedium.setScale(sf::Vector2f(.90f, .90f));
            }
            else if (music == 3) {
                btnLoud.setFillColor(sf::Color(180, 180, 180));
                btnLoud.setScale(sf::Vector2f(.90f, .90f));
            }

            if (controls == 0) {
                btnWSAD.setFillColor(sf::Color(180, 180, 180));
                btnWSAD.setScale(sf::Vector2f(.90f, .90f));
            }
            else if (controls == 1) {
                btnArrows.setFillColor(sf::Color(180, 180, 180));
                btnArrows.setScale(sf::Vector2f(.90f, .90f));
            }
        }


        // --- LOGIKA GRY ---
        if (aktualnyStan == Stan::GRA) {
            float ruchX = 0.f;
            if (sf::Keyboard::isKeyPressed(left)) ruchX = -8.f;
            if (sf::Keyboard::isKeyPressed(right)) ruchX = 8.f;

            paletka.move(sf::Vector2f(ruchX, 0.f));

            if (paletka.getPosition().x - 60.f < 0.f)
                paletka.setPosition(sf::Vector2f(60.f, paletka.getPosition().y));
            else if (paletka.getPosition().x + 60.f > szerokosc)
                paletka.setPosition(sf::Vector2f(szerokosc - 60.f, paletka.getPosition().y));



            if (czyW_Ruchu) {
                kulka.move(predkosc);
                kulka.rotate(sf::degrees(predkosc.x * 3.0f));
                sf::Vector2f pos = kulka.getPosition();

                if (pos.x - promien <= 0 || pos.x + promien >= szerokosc) predkosc.x = -predkosc.x;
                if (pos.y - promien <= 0) predkosc.y = -predkosc.y;

                if (pos.y + promien >= wysokosc) {
                    aktualnyStan = Stan::LOSE;

                    // ZMIANA MUZYKI NA PRZEGRANĄ
                    if (currentBgMusic) currentBgMusic->stop();
                    musicLose.play();

                    combosum += comboTempMax;
                    xpsum = points + (2 * combosum);
                    score = (points * 100) + (5 * level * combosum);
                    addXP(player, xpsum);
                    if (score > player.highScore) player.highScore = score;
                    saveGame(player);
                }
                else if (points >= (int)bloczki.size()) {
                    aktualnyStan = Stan::WIN;

                    // ZMIANA MUZYKI NA WYGRANĄ
                    if (currentBgMusic) currentBgMusic->stop();
                    musicWin.play();

                    combosum += comboTempMax;
                    xpsum = points + (2 * combosum);
                    score = (points * 100) + (5 * level * combosum);
                    addXP(player, xpsum);
                    if (score > player.highScore) player.highScore = score;
                    saveGame(player);
                }


                // logika odbić
                if (kulka.getGlobalBounds().findIntersection(paletka.getGlobalBounds())) {
                    combo = 0;
                    combosum += comboTempMax;
                    comboTempMax = 0;
                    score = (points * 100) + (5 * level * combosum);
                    ScoreText.setString("Score: " + std::to_string(score));
                    float kat;

                    if (ruchX == 0) {
                        // Jeśli paletka stoi, losujemy kąt z przedziału 220 (lewo-góra) do 320 (prawo-góra)
                        std::uniform_real_distribution<float> dist(220.f, 320.f);
                        kat = dist(gen);
                    }
                    else {
                        // Jeśli paletka się rusza, narzucamy stały kąt w stronę ruchu
                        kat = (ruchX < 0) ? 220.f : 320.f;
                    }

                    float rad = kat * 3.14159f / 180.f;
                    predkosc = { std::cos(rad) * moc, std::sin(rad) * moc };

                    // Poprawka pozycji, żeby kulka nie utknęła w paletce
                    kulka.setPosition({ pos.x, paletka.getPosition().y - 20.f });
                }


                for (auto& b : bloczki) {
                    if (!b.zniszczony && kulka.getGlobalBounds().findIntersection(b.shape.getGlobalBounds())) {
                        combo++;
                        b.health_points--;
                        if (b.health_points == 1) b.shape.setTexture(&Blok1hp);
                        else if (b.health_points == 2) b.shape.setTexture(&Blok2hp);
                        hitSound.play();
                        if (rand() % 100 < 40) {
                            b.moving = true;
                        }
                        if (b.health_points == 0) {
                            points++;
                            b.zniszczony = true;
                            if (rand() % 2 == 0) {
                                Power pow;
                                pow.shape.setRadius(16.0f);
                                pow.shape.setTexture(&powerUp);
                                pow.shape.setFillColor(sf::Color::White);
                                pow.shape.setOrigin(pow.shape.getGeometricCenter());
                                pow.shape.setPosition(b.shape.getPosition());
                                pow.moving = true;
                                if (rand() % 2 == 0) {
                                    pow.factor = moc * 0.15f;
                                }
                                else if (moc >= 6.0f) { // kulka nie porusza sie wolniej niz poczatkowa predkosc
                                    pow.factor = -moc * 0.15f;
                                }
                                power.push_back(pow);
                            }
                        }
                        predkosc.y = -predkosc.y;
                        break;
                    }
                    if (b.moving == true) {
                        b.shape.move({ 0.0f, 0.25f });
                    }
                    if (!b.zniszczony && b.shape.getGlobalBounds().findIntersection(paletka.getGlobalBounds())) {
                        aktualnyStan = Stan::LOSE;

                        // ZMIANA MUZYKI NA LOSE
                        if (currentBgMusic) currentBgMusic->stop();
                        musicLose.play();

                        combosum += comboTempMax;
                        xpsum = points + (2 * combosum);
                        score = (points * 100) + (5 * level * combosum);
                        addXP(player, xpsum);
                        if (score > player.highScore) player.highScore = score;
                        saveGame(player);
                    }
                    if (!b.zniszczony && b.shape.getPosition().y >= paletka.getPosition().y) {
                        b.health_points = 0;
                        b.zniszczony = true;
                        points++;
                    }
                }
                for (auto& pow : power) {
                    if (pow.moving) {
                        pow.shape.move({ 0.0f, 3.0f }); // predkosc power upu
                    }
                    if (!pow.used && paletka.getGlobalBounds().findIntersection(pow.shape.getGlobalBounds())) {
                        pow.used = true;
                        powerSound.play();
                        moc += pow.factor;
                        if (moc < 3.0f) moc = 5.0f;
                        break;
                    }
                }
            }
        }


        // --- RENDEROWANIE ---
        window.clear(sf::Color(30, 30, 30));
        window.draw(backgroundSprite);

        if (aktualnyStan == Stan::MENU) {
            float percent = static_cast<float>(player.xp) / player.xpToNext;
            percent = std::clamp(percent, 0.f, 1.f);
            xpBarFill.setSize(sf::Vector2f(300.f * percent, 20.f));
            levelText.setString("LEVEL " + std::to_string(player.level));
            HighscoreText.setString("Highscore: " + std::to_string(player.highScore));
            window.draw(titleSprite);
            window.draw(btnPlay);
            window.draw(btnExit);
            window.draw(btnOptions);
            window.draw(xpBarBg2);
            window.draw(xpBarBg1);
            window.draw(xpBarBg);
            window.draw(xpBarFill);
            window.draw(levelText);
            window.draw(HighscoreText);
        }
        else if (aktualnyStan == Stan::OPCJE) {
            window.draw(btnMusic);
            window.draw(btnTurnedOff);
            window.draw(btnQuiet);
            window.draw(btnMedium);
            window.draw(btnLoud);
            window.draw(btnBack);
            window.draw(btnControls);
            window.draw(btnWSAD);
            window.draw(btnArrows);
        }
        else if (aktualnyStan == Stan::LEVEL) {
            window.draw(btnLvl1);
            window.draw(btnLvl2);
            window.draw(btnLvl3);
            window.draw(btnLvlBack);
        }
        else if (aktualnyStan == Stan::GRA) {
            window.draw(paletka);
            for (const auto& b : bloczki) if (!b.zniszczony) window.draw(b.shape);
            for (const auto& pow : power) if (!pow.used) window.draw(pow.shape);
            window.draw(kulka);
            ScoreText.setPosition(sf::Vector2f(10.f, wysokosc - 50.f));
            window.draw(ScoreText);
            if (combo >= 3)
            {
                comboTempMax = combo;
                ComboText.setString("COMBO x" + std::to_string(combo));
                ComboText.setPosition(sf::Vector2f(szerokosc - 200.f, wysokosc / 2 - 100.f));
                window.draw(ComboText);
            }
        }
        else if (aktualnyStan == Stan::PAUSE) {
            window.draw(paletka);
            for (const auto& b : bloczki) if (!b.zniszczony) window.draw(b.shape);
            for (const auto& pow : power) if (!pow.used) window.draw(pow.shape);
            window.draw(kulka);
            window.draw(dimmer);
            window.draw(btnResume);
            window.draw(btnRestart);
            window.draw(btnPauseExit);
        }
        else if (aktualnyStan == Stan::LOSE) {
            ScoreText.setPosition(sf::Vector2f(szerokosc / 2 - 85.f, wysokosc / 2 - 90.f));
            ScoreText.setString("Score: " + std::to_string(score));
            xpText.setString("XP gained: " + std::to_string(xpsum));
            window.draw(dimmer);
            window.draw(GameOverSprite);
            window.draw(btnGameOverRestart);
            window.draw(btnGameOverExit);
            window.draw(ScoreText);
            window.draw(xpText);
        }
        else if (aktualnyStan == Stan::WIN) {
            ScoreText.setPosition(sf::Vector2f(szerokosc / 2 - 85.f, wysokosc / 2 - 90.f));
            ScoreText.setString("Score: " + std::to_string(score));
            xpText.setString("XP gained: " + std::to_string(xpsum));
            window.draw(dimmer);
            window.draw(WinSprite);
            window.draw(btnGameOverRestart);
            window.draw(btnGameOverExit);
            window.draw(ScoreText);
            window.draw(xpText);
        }

        window.display();
    }
    return 0;
}


