#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <cmath>
#include <random>
#include <optional>
#include <vector>

// Definiujemy możliwe stany gry
enum class Stan { MENU, GRA, LEVEL };

// Licznik punktów
int points = 0;

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

    // ==================================================
    // --- ANIMOWANY TYTUŁ (SPRITESHEET) ---
    // ==================================================
    sf::Texture titleTexture;
    // Ładujemy obrazek tytułu (spritesheet: 2 klatki obok siebie)
    if (!titleTexture.loadFromFile("Sprites/title1.png")) {
        return -1;
    }
    titleTexture.setSmooth(true);

    // Obliczamy wymiary jednej klatki
    // Skoro są 2 kolumny, dzielimy szerokość całej tekstury przez 2
    int frameWidth = titleTexture.getSize().x / 2;
    int frameHeight = titleTexture.getSize().y;

    sf::Sprite titleSprite(titleTexture);

    // Ustawiamy początkowy wycinek (pierwsza klatka: x=0, y=0)
    // W SFML 3 IntRect wymaga sf::Vector2i
    titleSprite.setTextureRect(sf::IntRect(sf::Vector2i(0, 0), sf::Vector2i(frameWidth, frameHeight)));

    // Ustawiamy origin na środek KLATKI (nie całej tekstury)
    titleSprite.setOrigin(sf::Vector2f(frameWidth / 2.f, frameHeight / 2.f));

    // Pozycja: Góra ekranu (np. Y = 100)
    titleSprite.setPosition(sf::Vector2f(szerokosc / 2.f, 100.f));
    titleSprite.setScale(sf::Vector2f(0.5f, 0.5f));

    // Zmienne do animacji
    sf::Clock titleClock;       // Mierzy czas
    int currentTitleFrame = 0;  // 0 = pierwsza klatka, 1 = druga klatka

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
    // --- ELEMENTY PRZYCISKÓW MENU ---
    // ==================================================

    // 1. Ładowanie tekstur przycisków (NOWOŚĆ)
    sf::Texture texBtnPlay;
    if (!texBtnPlay.loadFromFile("Sprites/btn_play.png")) { return -1; }

    sf::Texture texBtnExit;
    if (!texBtnExit.loadFromFile("Sprites/btn_exit.png")) { return -1; }

    // Włączenie wygładzania dla ładniejszego wyglądu przy skalowaniu
    texBtnPlay.setSmooth(true);
    texBtnExit.setSmooth(true);

    // 2. Konfiguracja przycisku GRAJ
    sf::RectangleShape btnPlay(sf::Vector2f(350.f, 60.f));
    btnPlay.setTexture(&texBtnPlay);          // <--- Przypisanie tekstury
    btnPlay.setFillColor(sf::Color::White);   // <--- Kolor Biały (żeby widać było teksturę)

    btnPlay.setOrigin(sf::Vector2f(175.f, 30.f));
    btnPlay.setPosition(sf::Vector2f(szerokosc / 2.f, wysokosc / 2.f - 30.f));

    sf::Text txtPlay(font);
    txtPlay.setFillColor(sf::Color::White);

    sf::FloatRect boundsPlay = txtPlay.getLocalBounds();
    txtPlay.setOrigin(sf::Vector2f(boundsPlay.size.x / 2.f, boundsPlay.size.y / 2.f));
    txtPlay.setPosition(sf::Vector2f(btnPlay.getPosition().x, btnPlay.getPosition().y - 5.f));

    // 3. Konfiguracja przycisku WYJSCIE
    sf::RectangleShape btnExit(sf::Vector2f(250.f, 60.f));
    btnExit.setTexture(&texBtnExit);          // <--- Przypisanie tekstury
    btnExit.setFillColor(sf::Color::White);   // <--- Kolor Biały

    btnExit.setOrigin(sf::Vector2f(125.f, 30.f));
    btnExit.setPosition(sf::Vector2f(szerokosc / 2.f, wysokosc / 2.f + 50.f));

    sf::Text txtExit(font);
    txtExit.setFillColor(sf::Color::White);

    sf::FloatRect boundsExit = txtExit.getLocalBounds();
    txtExit.setOrigin(sf::Vector2f(boundsExit.size.x / 2.f, boundsExit.size.y / 2.f));
    txtExit.setPosition(sf::Vector2f(btnExit.getPosition().x, btnExit.getPosition().y - 5.f));

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

        // --- AKTUALIZACJA ANIMACJI TYTUŁU ---
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
        }

        // --- LOGIKA STANU MENU (HOVER) ---
        if (aktualnyStan == Stan::MENU) {
            // GRAJ
            if (btnPlay.getGlobalBounds().contains(mousePos)) {
                btnPlay.setFillColor(sf::Color(220, 220, 220)); // Lekkie przyciemnienie
                btnPlay.setScale(sf::Vector2f(1.05f, 1.05f));
            }
            else {
                btnPlay.setFillColor(sf::Color::White); // Pełna widoczność tekstury
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

        // --- LOGIKA STANU GRA ---
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
            window.draw(txtPlay); 

            window.draw(btnExit);
            window.draw(txtExit); 
        }
        else {
            window.draw(paletka);
            for (const auto& b : bloczki) if (!b.zniszczony) window.draw(b.shape);
            window.draw(kulka);
        }

        window.display();
    }
    return 0;
}