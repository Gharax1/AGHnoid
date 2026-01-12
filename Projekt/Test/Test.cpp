#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <cmath>
#include <random>
#include <optional>
#include <vector>

// Definiujemy możliwe stany gry
enum class Stan { MENU, GRA, LEVEL, PAUSE };

// Licznik punktów
int points = 0;

int main() {
    const unsigned int szerokosc = 800;
    const unsigned int wysokosc = 600;

    sf::RenderWindow window(sf::VideoMode({ szerokosc, wysokosc }), "Arkanoid z Menu - SFML 3");
    window.setFramerateLimit(60);

    Stan aktualnyStan = Stan::MENU;

    // --- CZCIONKA ---
    // (Zostawiamy ładowanie, bo może się przydać do licznika punktów lub GameOver, ale usuwamy z przycisków)
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

    // ==================================================
    // --- ANIMOWANY TYTUŁ (SPRITESHEET) ---
    // ==================================================
    sf::Texture titleTexture;
    if (!titleTexture.loadFromFile("Sprites/title1.png")) {
        return -1;
    }
    titleTexture.setSmooth(true);

    int frameWidth = titleTexture.getSize().x / 2;
    int frameHeight = titleTexture.getSize().y;

    sf::Sprite titleSprite(titleTexture);

    // W SFML 3 IntRect wymaga sf::Vector2i
    titleSprite.setTextureRect(sf::IntRect(sf::Vector2i(0, 0), sf::Vector2i(frameWidth, frameHeight)));
    titleSprite.setOrigin(sf::Vector2f(frameWidth / 2.f, frameHeight / 2.f));
    titleSprite.setPosition(sf::Vector2f(szerokosc / 2.f, 100.f));
    titleSprite.setScale(sf::Vector2f(0.5f, 0.5f));

    sf::Clock titleClock;
    int currentTitleFrame = 0;

    // ==================================================

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
    // --- ELEMENTY PRZYCISKÓW (MENU & PAUSE) ---
    // ==================================================

    // 1. Ładowanie tekstur
    sf::Texture texBtnPlay;
    if (!texBtnPlay.loadFromFile("Sprites/btn_play.png")) { return -1; }

    sf::Texture texBtnExit;
    if (!texBtnExit.loadFromFile("Sprites/btn_exit.png")) { return -1; }

    // NOWA TEKSTURA DO PAUZY (Wznów)
    sf::Texture texBtnResume;
    if (!texBtnResume.loadFromFile("Sprites/btn_resume.png")) { return -1; }

    texBtnPlay.setSmooth(true);
    texBtnExit.setSmooth(true);
    texBtnResume.setSmooth(true);

    // --- PRZYCISKI MENU GŁÓWNEGO ---

    // Przycisk GRAJ
    sf::RectangleShape btnPlay(sf::Vector2f(350.f, 60.f));
    btnPlay.setTexture(&texBtnPlay);
    btnPlay.setFillColor(sf::Color::White);
    btnPlay.setOrigin(sf::Vector2f(175.f, 30.f));
    btnPlay.setPosition(sf::Vector2f(szerokosc / 2.f, wysokosc / 2.f - 30.f));

    // Przycisk WYJSCIE (Menu Główne)
    sf::RectangleShape btnExit(sf::Vector2f(250.f, 60.f));
    btnExit.setTexture(&texBtnExit);
    btnExit.setFillColor(sf::Color::White);
    btnExit.setOrigin(sf::Vector2f(125.f, 30.f));
    btnExit.setPosition(sf::Vector2f(szerokosc / 2.f, wysokosc / 2.f + 50.f));


    // --- ELEMENTY PAUZY ---

    // Przyciemnienie ekranu
    sf::RectangleShape dimmer(sf::Vector2f((float)szerokosc, (float)wysokosc));
    dimmer.setFillColor(sf::Color(0, 0, 0, 150)); // Półprzezroczysty czarny

    // Przycisk WZNÓW (PAUZA)
    sf::RectangleShape btnResume(sf::Vector2f(250.f, 60.f)); // Rozmiar przykładowy
    btnResume.setTexture(&texBtnResume);
    btnResume.setFillColor(sf::Color::White);
    btnResume.setOrigin(sf::Vector2f(125.f, 30.f));
    btnResume.setPosition(sf::Vector2f(szerokosc / 2.f, wysokosc / 2.f - 40.f));

    // Przycisk WYJŚCIE DO MENU (PAUZA) - używamy tej samej tekstury co btnExit
    sf::RectangleShape btnPauseExit(sf::Vector2f(250.f, 60.f));
    btnPauseExit.setTexture(&texBtnExit);
    btnPauseExit.setFillColor(sf::Color::White);
    btnPauseExit.setOrigin(sf::Vector2f(125.f, 30.f));
    btnPauseExit.setPosition(sf::Vector2f(szerokosc / 2.f, wysokosc / 2.f + 40.f));


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
    };
    std::vector<Bloczki> bloczki;

    sf::Texture blokTexture;
    if (blokTexture.loadFromFile("Sprites/bloczki_4.png")) {}

    auto resetGry = [&]() {
        points = 0;
        kulka.setPosition(sf::Vector2f(szerokosc / 2.f, wysokosc / 2.f + 50.f));
        paletka.setPosition(sf::Vector2f(szerokosc / 2.f, wysokosc - 40.f));
        predkosc = sf::Vector2f(0.f, 0.f);
        czyW_Ruchu = false;

        bloczki.clear();
        for (int rzad = 0; rzad < 3; ++rzad) {
            for (int kol = 0; kol < 10; ++kol) {
                Bloczki b;
                b.shape.setSize(sf::Vector2f(70.f, 25.f));
                b.shape.setTexture(&blokTexture);
                switch (rzad) {
                case 0: b.shape.setFillColor(sf::Color(0x00, 0x6b, 0x3b)); break;
                case 1: b.shape.setFillColor(sf::Color(0x23, 0x1f, 0x20)); break;
                case 2: b.shape.setFillColor(sf::Color(0xb0, 0x10, 0x28)); break;
                }
                b.shape.setPosition(sf::Vector2f(40.f + kol * 75.f, 50.f + rzad * 30.f));
                bloczki.push_back(b);
            }
        }
        };

    resetGry();
    std::random_device rd;
    std::mt19937 gen(rd());

    // --- PĘTLA GŁÓWNA ---
    while (window.isOpen()) {

        sf::Vector2f mousePos = window.mapPixelToCoords(sf::Mouse::getPosition(window));

        // --- AKTUALIZACJA ANIMACJI TYTUŁU (Tylko w Menu) ---
        if (aktualnyStan == Stan::MENU) {
            if (titleClock.getElapsedTime().asSeconds() >= 1.0f) {
                currentTitleFrame = (currentTitleFrame + 1) % 2;
                int left = currentTitleFrame * frameWidth;
                titleSprite.setTextureRect(sf::IntRect(
                    sf::Vector2i(left, 0),
                    sf::Vector2i(frameWidth, frameHeight)
                ));
                titleClock.restart();
            }
        }

        while (const std::optional event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) window.close();

            // OBSŁUGA KLAWISZA ESCAPE (PAUZA)
            if (event->is<sf::Event::KeyPressed>()) {
                if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Escape)) {
                    if (aktualnyStan == Stan::GRA) {
                        aktualnyStan = Stan::PAUSE;
                        gameMusic.pause(); // Zatrzymaj muzykę
                    }
                    else if (aktualnyStan == Stan::PAUSE) {
                        aktualnyStan = Stan::GRA;
                        gameMusic.play(); // Wznów muzykę
                    }
                }
            }

            if (aktualnyStan == Stan::MENU) {
                if (const auto* mouseEvent = event->getIf<sf::Event::MouseButtonPressed>()) {
                    if (mouseEvent->button == sf::Mouse::Button::Left) {

                        if (btnPlay.getGlobalBounds().contains(mousePos)) {
                            menuMusic.stop();
                            gameMusic.play();
                            aktualnyStan = Stan::GRA;
                            resetGry();
                        }

                        if (btnExit.getGlobalBounds().contains(mousePos)) {
                            window.close();
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
                // OBSŁUGA KLIKNIĘĆ W PAUZIE
                if (const auto* mouseEvent = event->getIf<sf::Event::MouseButtonPressed>()) {
                    if (mouseEvent->button == sf::Mouse::Button::Left) {

                        // Kliknięcie WZNÓW
                        if (btnResume.getGlobalBounds().contains(mousePos)) {
                            aktualnyStan = Stan::GRA;
                            gameMusic.play();
                        }

                        // Kliknięcie WYJŚCIE (do menu)
                        if (btnPauseExit.getGlobalBounds().contains(mousePos)) {
                            aktualnyStan = Stan::MENU;
                            gameMusic.stop();
                            menuMusic.play();
                        }
                    }
                }
            }
        }

        // --- LOGIKA HOVER (PODŚWIETLANIE) ---

        // 1. MENU
        if (aktualnyStan == Stan::MENU) {
            // GRAJ
            if (btnPlay.getGlobalBounds().contains(mousePos)) {
                btnPlay.setFillColor(sf::Color(220, 220, 220));
                btnPlay.setScale(sf::Vector2f(1.05f, 1.05f));
            }
            else {
                btnPlay.setFillColor(sf::Color::White);
                btnPlay.setScale(sf::Vector2f(1.0f, 1.0f));
            }

            // WYJSCIE
            if (btnExit.getGlobalBounds().contains(mousePos)) {
                btnExit.setFillColor(sf::Color(220, 220, 220));
                btnExit.setScale(sf::Vector2f(1.05f, 1.05f));
            }
            else {
                btnExit.setFillColor(sf::Color::White);
                btnExit.setScale(sf::Vector2f(1.0f, 1.0f));
            }
        }

        // 2. PAUZA (Hover)
        if (aktualnyStan == Stan::PAUSE) {
            // WZNÓW
            if (btnResume.getGlobalBounds().contains(mousePos)) {
                btnResume.setFillColor(sf::Color(220, 220, 220));
                btnResume.setScale(sf::Vector2f(1.05f, 1.05f));
            }
            else {
                btnResume.setFillColor(sf::Color::White);
                btnResume.setScale(sf::Vector2f(1.0f, 1.0f));
            }

            // WYJŚCIE DO MENU
            if (btnPauseExit.getGlobalBounds().contains(mousePos)) {
                btnPauseExit.setFillColor(sf::Color(220, 220, 220));
                btnPauseExit.setScale(sf::Vector2f(1.05f, 1.05f));
            }
            else {
                btnPauseExit.setFillColor(sf::Color::White);
                btnPauseExit.setScale(sf::Vector2f(1.0f, 1.0f));
            }
        }

        // --- LOGIKA STANU GRA ---
        // Wykonuje się tylko gdy NIE ma pauzy
        if (aktualnyStan == Stan::GRA) {
            float ruchX = 0.f;
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left)) ruchX = -8.f;
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right)) ruchX = 8.f;

            paletka.move(sf::Vector2f(ruchX, 0.f));

            sf::Vector2f pozycjaPaletki = paletka.getPosition();
            float polSzerokosci = paletka.getSize().x / 2.f;

            if (pozycjaPaletki.x - polSzerokosci < 0.f)
                paletka.setPosition(sf::Vector2f(polSzerokosci, pozycjaPaletki.y));
            else if (pozycjaPaletki.x + polSzerokosci > szerokosc)
                paletka.setPosition(sf::Vector2f(szerokosc - polSzerokosci, pozycjaPaletki.y));

            if (czyW_Ruchu) {
                kulka.move(predkosc);
                kulka.rotate(sf::degrees(predkosc.x * 3.0f));
                sf::Vector2f pos = kulka.getPosition();

                if (pos.x - promien <= 0 || pos.x + promien >= szerokosc) predkosc.x = -predkosc.x;
                if (pos.y - promien <= 0) predkosc.y = -predkosc.y;

                if (pos.y + promien >= wysokosc || points >= bloczki.size()) {
                    aktualnyStan = Stan::MENU;
                    gameMusic.stop();
                    menuMusic.play();
                }

                if (kulka.getGlobalBounds().findIntersection(paletka.getGlobalBounds())) {
                    float kat;
                    if (ruchX == 0) {
                        std::uniform_real_distribution<float> dist(220.f, 320.f);
                        kat = dist(gen);
                    }
                    else {
                        kat = (ruchX < 0) ? 220.f : 320.f;
                    }
                    float rad = kat * 3.14159f / 180.f;
                    predkosc = sf::Vector2f(std::cos(rad) * moc, std::sin(rad) * moc);
                    kulka.setPosition(sf::Vector2f(pos.x, paletka.getPosition().y - 15.f));
                }

                for (auto& b : bloczki) {
                    if (!b.zniszczony && kulka.getGlobalBounds().findIntersection(b.shape.getGlobalBounds())) {
                        b.zniszczony = true;
                        std::uniform_real_distribution<float> pitchDist(0.8f, 1.2f);
                        hitSound.setPitch(pitchDist(gen));
                        hitSound.play();
                        points++;
                        predkosc.y = -predkosc.y;
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
        else if (aktualnyStan == Stan::GRA) {
            window.draw(paletka);
            for (const auto& b : bloczki) if (!b.zniszczony) window.draw(b.shape);
            window.draw(kulka);
        }
        else if (aktualnyStan == Stan::PAUSE) {
            // W pauzie najpierw rysujemy grę pod spodem
            window.draw(paletka);
            for (const auto& b : bloczki) if (!b.zniszczony) window.draw(b.shape);
            window.draw(kulka);

            // Rysujemy przyciemnienie
            window.draw(dimmer);

            // Rysujemy przyciski pauzy (bez napisów)
            window.draw(btnResume);
            window.draw(btnPauseExit);
        }

        window.display();
    }
    return 0;
}