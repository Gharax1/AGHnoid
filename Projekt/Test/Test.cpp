#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <cmath>
#include <random>
#include <optional>
#include <vector>

// Definiujemy możliwe stany gry
enum class Stan { MENU, GRA, LEVEL, PAUSE, WIN, LOSE };


// Licznik punktów
int points = 0;
int wybranyPoziom = 1;


int main() {
    const unsigned int szerokosc = 800;
    const unsigned int wysokosc = 600;

    sf::RenderWindow window(sf::VideoMode({ szerokosc, wysokosc }), "Arkanoid z Menu - SFML 3");
    window.setFramerateLimit(60);

    Stan aktualnyStan = Stan::MENU;


    // --- CZCIONKA ---
    sf::Font font;
    if (!font.openFromFile("Fonts/arial.ttf")) {
        return -1;
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
    

    // ---- GAME OVER -------

    sf::Texture GameOverTexture;
    if(!GameOverTexture.loadFromFile("Sprites/game_over.png")){}
    GameOverTexture.setSmooth(true);
    
    float GOTwidth = static_cast<float>(GameOverTexture.getSize().x) / 2.f;
    float GOTheight = static_cast<float>(GameOverTexture.getSize().y) / 2.f;

    sf::Sprite GameOverSprite(GameOverTexture);
    GameOverSprite.setOrigin(sf::Vector2f(GOTwidth, GOTheight));
    GameOverSprite.setPosition(sf::Vector2f(szerokosc / 2.f, 150.f));
    GameOverSprite.setScale(sf::Vector2f(0.6f, 0.6f));


    // ------- WIN ----------
    sf::Texture WinTexture;
    if (!WinTexture.loadFromFile("Sprites/game_over.png")) {}
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


    // --- AUDIO ---
    sf::SoundBuffer hitBuffer;
    if (!hitBuffer.loadFromFile("Sounds/Hit/hit1.wav")) {}
    sf::Sound hitSound(hitBuffer);
    hitSound.setVolume(50.f);

    sf::Music menuMusic;
    if (menuMusic.openFromFile("Music/menu.mp3")) {
        menuMusic.setLooping(true);
        menuMusic.setVolume(30.f);
        menuMusic.play();
    }

    sf::Music gameMusic;
    if (gameMusic.openFromFile("Music/lvl1.mp3")) {
        gameMusic.setLooping(true);
        gameMusic.setVolume(10.f);
    }


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

    texBtnPlay.setSmooth(true);
    texBtnExit.setSmooth(true);
    texBtnResume.setSmooth(true);
    texBtnRestart.setSmooth(true);


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


    // --- ELEMENTY WYBORU POZIOMU ---

    sf::RectangleShape btnLvl1(sf::Vector2f(300.f, 60.f));
    btnLvl1.setTexture(&texBtnPlay);
    btnLvl1.setFillColor(sf::Color::White);
    btnLvl1.setOrigin(sf::Vector2f(150.f, 30.f));
    btnLvl1.setPosition(sf::Vector2f(szerokosc / 2.f, wysokosc / 2.f - 120.f)); 

    sf::Text textLvl1(font); 
    textLvl1.setString("POZIOM 1");
    textLvl1.setCharacterSize(30);
    textLvl1.setFillColor(sf::Color::Black);
    textLvl1.setOrigin(sf::Vector2f(textLvl1.getGlobalBounds().size.x / 2.f, textLvl1.getGlobalBounds().size.y / 2.f));
    textLvl1.setPosition(btnLvl1.getPosition());


    sf::RectangleShape btnLvl2(sf::Vector2f(300.f, 60.f));
    btnLvl2.setTexture(&texBtnPlay);
    btnLvl2.setFillColor(sf::Color::White);
    btnLvl2.setOrigin(sf::Vector2f(150.f, 30.f));
    btnLvl2.setPosition(sf::Vector2f(szerokosc / 2.f, wysokosc / 2.f - 40.f)); 

    sf::Text textLvl2(font);
    textLvl2.setString("POZIOM 2");
    textLvl2.setCharacterSize(30);
    textLvl2.setFillColor(sf::Color::Black);
    textLvl2.setOrigin(sf::Vector2f(textLvl2.getGlobalBounds().size.x / 2.f, textLvl2.getGlobalBounds().size.y / 2.f));
    textLvl2.setPosition(btnLvl2.getPosition());

    sf::RectangleShape btnLvl3(sf::Vector2f(300.f, 60.f));
    btnLvl3.setTexture(&texBtnPlay);
    btnLvl3.setFillColor(sf::Color::White);
    btnLvl3.setOrigin(sf::Vector2f(150.f, 30.f));
    btnLvl3.setPosition(sf::Vector2f(szerokosc / 2.f, wysokosc / 2.f + 40.f));

    sf::Text textLvl3(font); 
    textLvl3.setString("POZIOM 3");
    textLvl3.setCharacterSize(30);
    textLvl3.setFillColor(sf::Color::Black);
    textLvl3.setOrigin(sf::Vector2f(textLvl3.getGlobalBounds().size.x / 2.f, textLvl3.getGlobalBounds().size.y / 2.f));
    textLvl3.setPosition(btnLvl3.getPosition());


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
    btnGameOverRestart.setOrigin(sf::Vector2f(125.f,30.f));
    btnGameOverRestart.setPosition(sf::Vector2f(szerokosc / 2.f, wysokosc / 2.f - 30.f));


    sf::RectangleShape btnGameOverExit(sf::Vector2f(250.f, 60.f));
    btnGameOverExit.setTexture(&texBtnExit);
    btnGameOverExit.setFillColor(sf::Color::White);
    btnGameOverExit.setOrigin(sf::Vector2f(125.f, 30.f));
    btnGameOverExit.setPosition(sf::Vector2f(szerokosc / 2.f, wysokosc / 2.f + 50.f));


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
    float bonus = 1.0f;

    struct Bloczki {
        sf::RectangleShape shape;
        bool zniszczony = false;
        int health_points;
    };
    std::vector<Bloczki> bloczki;


    sf::Texture blokTexture;
    if (blokTexture.loadFromFile("Sprites/bloczki_4.png")) {}

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
                b.shape.setTexture(&blokTexture);
                if (lvl == 1) b.health_points = 1;
                if ((lvl == 2) && (rzad % 2 == 0)) b.health_points = 2;
                if ((lvl == 3) && (rzad % 3 == 0)) b.health_points = 2;
                if ((lvl == 3) && (rzad % 5 == 0)) b.health_points = 3;
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


    // --- PĘTLA GŁÓWNA ---
    while (window.isOpen()) {
        srand(time(NULL));

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
            if (event->is<sf::Event::Closed>()) window.close();

            if (event->is<sf::Event::KeyPressed>()) {
                if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Escape)) {
                    if (aktualnyStan == Stan::GRA) {
                        aktualnyStan = Stan::PAUSE;
                        gameMusic.pause();
                    }
                    else if (aktualnyStan == Stan::PAUSE) {
                        aktualnyStan = Stan::GRA;
                        gameMusic.play();
                    }
                }
            }


            if (aktualnyStan == Stan::MENU) {
                if (const auto* mouseEvent = event->getIf<sf::Event::MouseButtonPressed>()) {
                    if (mouseEvent->button == sf::Mouse::Button::Left) {
                        if (btnPlay.getGlobalBounds().contains(mousePos)) {
                            aktualnyStan = Stan::LEVEL;
                        }
                        if (btnExit.getGlobalBounds().contains(mousePos)) {
                            window.close();
                        }
                    }
                }
            }
            else if (aktualnyStan == Stan::LEVEL) {
                if (const auto* mouseEvent = event->getIf<sf::Event::MouseButtonPressed>()) {
                    if (mouseEvent->button == sf::Mouse::Button::Left) {
                        if (btnLvl1.getGlobalBounds().contains(mousePos)) {
                            wybranyPoziom = 1;
                            resetGry(1);
                            menuMusic.stop();
                            gameMusic.play();
                            aktualnyStan = Stan::GRA;
                        }
                        if (btnLvl2.getGlobalBounds().contains(mousePos)) {
                            wybranyPoziom = 2;
                            resetGry(2);
                            menuMusic.stop();
                            gameMusic.play();
                            aktualnyStan = Stan::GRA;
                        }
                        // DOPISANE KLIKNIĘCIE LVL 3
                        if (btnLvl3.getGlobalBounds().contains(mousePos)) {
                            wybranyPoziom = 3;
                            resetGry(3);
                            menuMusic.stop();
                            gameMusic.play();
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
                            gameMusic.play();
                        }
                        if (btnRestart.getGlobalBounds().contains(mousePos)) {
                            resetGry(wybranyPoziom);
                            aktualnyStan = Stan::GRA;
                            gameMusic.stop();
                            gameMusic.play();
                        }
                        if (btnPauseExit.getGlobalBounds().contains(mousePos)) {
                            aktualnyStan = Stan::MENU;
                            gameMusic.stop();
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
                            gameMusic.stop();
                            gameMusic.play();
                        }
                        if (btnGameOverExit.getGlobalBounds().contains(mousePos)) {
                            aktualnyStan = Stan::MENU;
                            gameMusic.stop();
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
            } else {
                btnPlay.setFillColor(sf::Color::White);
                btnPlay.setScale(sf::Vector2f(1.0f, 1.0f));
            }
            if (btnExit.getGlobalBounds().contains(mousePos)) {
                btnExit.setFillColor(sf::Color(220, 220, 220));
                btnExit.setScale(sf::Vector2f(1.05f, 1.05f));
            } else {
                btnExit.setFillColor(sf::Color::White);
                btnExit.setScale(sf::Vector2f(1.0f, 1.0f));
            }
        }


        if (aktualnyStan == Stan::LEVEL) {
            if (btnLvl1.getGlobalBounds().contains(mousePos)) {
                btnLvl1.setFillColor(sf::Color(220, 220, 220));
                btnLvl1.setScale(sf::Vector2f(1.05f, 1.05f));
            } else {
                btnLvl1.setFillColor(sf::Color::White);
                btnLvl1.setScale(sf::Vector2f(1.0f, 1.0f));
            }
            if (btnLvl2.getGlobalBounds().contains(mousePos)) {
                btnLvl2.setFillColor(sf::Color(220, 220, 220));
                btnLvl2.setScale(sf::Vector2f(1.05f, 1.05f));
            } else {
                btnLvl2.setFillColor(sf::Color::White);
                btnLvl2.setScale(sf::Vector2f(1.0f, 1.0f));
            }
    
            if (btnLvl3.getGlobalBounds().contains(mousePos)) {
                btnLvl3.setFillColor(sf::Color(220, 220, 220));
                btnLvl3.setScale(sf::Vector2f(1.05f, 1.05f));
            } else {
                btnLvl3.setFillColor(sf::Color::White);
                btnLvl3.setScale(sf::Vector2f(1.0f, 1.0f));
            }
            if (btnLvlBack.getGlobalBounds().contains(mousePos)) {
                btnLvlBack.setFillColor(sf::Color(220, 220, 220));
                btnLvlBack.setScale(sf::Vector2f(1.05f, 1.05f));
            } else {
                btnLvlBack.setFillColor(sf::Color::White);
                btnLvlBack.setScale(sf::Vector2f(1.0f, 1.0f));
            }
        }


        if (aktualnyStan == Stan::PAUSE) {
            if (btnResume.getGlobalBounds().contains(mousePos)) {
                btnResume.setFillColor(sf::Color(220, 220, 220));
                btnResume.setScale(sf::Vector2f(1.05f, 1.05f));
            } else {
                btnResume.setFillColor(sf::Color::White);
                btnResume.setScale(sf::Vector2f(1.0f, 1.0f));
            }
            if (btnRestart.getGlobalBounds().contains(mousePos)) {
                btnRestart.setFillColor(sf::Color(220, 220, 220));
                btnRestart.setScale(sf::Vector2f(1.05f, 1.05f));
            } else {
                btnRestart.setFillColor(sf::Color::White);
                btnRestart.setScale(sf::Vector2f(1.0f, 1.0f));
            }
            if (btnPauseExit.getGlobalBounds().contains(mousePos)) {
                btnPauseExit.setFillColor(sf::Color(220, 220, 220));
                btnPauseExit.setScale(sf::Vector2f(1.05f, 1.05f));
            } else {
                btnPauseExit.setFillColor(sf::Color::White);
                btnPauseExit.setScale(sf::Vector2f(1.0f, 1.0f));
            }
        }


        // --- LOGIKA GRY ---
        if (aktualnyStan == Stan::GRA) {
            float ruchX = 0.f;
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left)) ruchX = -8.f;
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right)) ruchX = 8.f;

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
                    gameMusic.stop();
                } else if (points >= (int)bloczki.size()) {
                    aktualnyStan = Stan::WIN;
                    gameMusic.stop();
                }

                if (kulka.getGlobalBounds().findIntersection(paletka.getGlobalBounds())) {
                    float kat = (ruchX == 0) ? 270.f : (ruchX < 0 ? 220.f : 320.f);
                    float rad = kat * 3.14159f / 180.f;
                    predkosc = sf::Vector2f(std::cos(rad) * moc, std::sin(rad) * moc);
                    kulka.setPosition(sf::Vector2f(pos.x, paletka.getPosition().y - 15.f));
                }


                for (auto& b : bloczki) {
                    if (!b.zniszczony && kulka.getGlobalBounds().findIntersection(b.shape.getGlobalBounds())) {
                        b.health_points--;
                        hitSound.play();
                        if (b.health_points == 0) {
                            points++;
                            b.zniszczony = true;
                            if (rand() % 2) {
                                Power pow;
                                pow.shape.setRadius(16.0f);
                                pow.shape.setFillColor(sf::Color::Red);
                                pow.shape.setOrigin(pow.shape.getGeometricCenter());
                                pow.shape.setPosition(b.shape.getPosition());
                                pow.moving = true;
                                if (rand() % 2) {
                                    pow.factor = moc * 0.15;
                                }
                                else {
                                    pow.factor = -moc * 0.15f;
                                }
                                power.push_back(pow);
                            }
                        }
                        predkosc.y = -predkosc.y;
                        break;
                    }
                }
                for (auto& pow : power) {
                    if (pow.moving) {
                        pow.shape.move({ 0.0f, 1.0f });
                    }
                    if (!pow.used && paletka.getGlobalBounds().findIntersection(pow.shape.getGlobalBounds())) {
                        pow.used = true;
                        hitSound.play();
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
            window.draw(titleSprite);
            window.draw(btnPlay);
            window.draw(btnExit);
        }
        else if (aktualnyStan == Stan::LEVEL) {
            window.draw(btnLvl1);
            window.draw(textLvl1);
            window.draw(btnLvl2);
            window.draw(textLvl2);
            window.draw(btnLvl3);
            window.draw(textLvl3);
            window.draw(btnLvlBack);
        }
        else if (aktualnyStan == Stan::GRA) {
            window.draw(paletka);
            for (const auto& b : bloczki) if (!b.zniszczony) window.draw(b.shape);
            for (const auto& pow : power) if (!pow.used) window.draw(pow.shape);
            window.draw(kulka);
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
            window.draw(dimmer);
            window.draw(GameOverSprite);
            window.draw(btnGameOverRestart);
            window.draw(btnGameOverExit);
        }
        else if (aktualnyStan == Stan::WIN) {
            window.draw(dimmer);
            window.draw(WinSprite);
            window.draw(btnGameOverRestart);
            window.draw(btnGameOverExit);
        }

        window.display();
    }
    return 0;
}
