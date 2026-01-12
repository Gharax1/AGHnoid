#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <cmath>
#include <random>
#include <optional>
#include <vector>

// Definiujemy możliwe stany gry
enum class Stan { MENU, GRA, LEVEL };

// Licznik punktów (zmienna globalna dla uproszczenia)
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

    // --- TŁO (BACKGROUND) ---
    sf::Texture backgroundTexture;
    if (!backgroundTexture.loadFromFile("Sprites/main_menu.png")) {
        return -1;
    }

    // --- MUZYKA, EFEKTY DŹWIĘKOWE ---
    sf::SoundBuffer hitBuffer;
    if (!hitBuffer.loadFromFile("Sounds/Hit/hit1.wav")) {
        sf::err() << "Nie udalo sie zaladowac hit.wav" << std::endl;
    }

    // Tworzymy odtwarzacz i przypisujemy mu bufor
    sf::Sound hitSound(hitBuffer);
    hitSound.setVolume(50.f);

    // Menu Music
    sf::Music menuMusic;
    if (!menuMusic.openFromFile("Music/menu.mp3")) {
        return -1;
    }
    menuMusic.setLooping(true);   // Zapętlanie
    menuMusic.setVolume(30.f); // Głośność (0-100)

    sf::Music gameMusic;
    if (!gameMusic.openFromFile("game_music.ogg")) {
        return -1;
    }
    gameMusic.setLooping(true);
    gameMusic.setVolume(40.f);

    // Od razu włączamy muzykę menu na starcie programu
    menuMusic.play();

    // Tworzymy sprite od razu z teksturą (wymóg SFML 3)
    sf::Sprite backgroundSprite(backgroundTexture);

    // Skalowanie tła do rozmiaru okna
    sf::Vector2u rozmiarObrazka = backgroundTexture.getSize();
    float skalaX = static_cast<float>(szerokosc) / rozmiarObrazka.x;
    float skalaY = static_cast<float>(wysokosc) / rozmiarObrazka.y;
    backgroundSprite.setScale({ skalaX, skalaY });

    // --- TEKST MENU ---
    sf::Text menuText(font);
    menuText.setString("\tARKANOID\nNacisnij ENTER aby zaczac");
    menuText.setCharacterSize(40);
    menuText.setFillColor(sf::Color::White);

    // Centrowanie tekstu
    sf::FloatRect textBounds = menuText.getLocalBounds();
    menuText.setOrigin({ textBounds.size.x / 2.f, textBounds.size.y / 2.f });
    menuText.setPosition({ szerokosc / 2.f, wysokosc / 2.f });

    // --- KULKA ---
    sf::Texture kulkaTexture;
    if (!kulkaTexture.loadFromFile("Sprites/kulka_1.png")) {
        return -1;
    }
    kulkaTexture.setSmooth(true);

    float promien = 8.f;
    sf::CircleShape kulka(promien);
    kulka.setTexture(&kulkaTexture);     // Przypisanie tekstury
    kulka.setFillColor(sf::Color::White); // Kolor biały (neutralny dla tekstury)
    kulka.setOrigin({ promien, promien });

    // --- PALETKA ---
    sf::Texture paletkaTexture;
    if (!paletkaTexture.loadFromFile("Sprites/paletka.png")) {
        return -1;
    }

    // paletkaTexture.setSmooth(true); // Wygładzanie krawędzi

    sf::Vector2f paletkaRozmiar(120.f, 15.f);
    sf::RectangleShape paletka(paletkaRozmiar);

    // Nakładamy teksturę
    paletka.setTexture(&paletkaTexture);

    // Ustawiamy kolor na Biały (żeby wyświetlić oryginalne kolory z pliku png)
    paletka.setFillColor(sf::Color::White);

    paletka.setOrigin({ paletkaRozmiar.x / 2.f, (paletkaRozmiar.y / 2.f) + 0.5f });

    sf::Vector2f predkosc(0.f, 0.f);
    bool czyW_Ruchu = false;
    float moc = 6.0f;

    // --- BLOCZKI ---
    struct Bloczki {
        sf::RectangleShape shape;
        bool zniszczony = false;
    };
    std::vector<Bloczki> bloczki;

    // 1. Ładowanie tekstury bloczka
    sf::Texture blokTexture;
    if (!blokTexture.loadFromFile("Sprites/bloczki_4.png")) {
        return -1;
    }

    // Funkcja resetująca grę
    auto resetGry = [&]() {
        points = 0;
        kulka.setPosition({ szerokosc / 2.f, wysokosc / 2.f + 50.f });
        paletka.setPosition({ szerokosc / 2.f, wysokosc - 40.f });
        predkosc = { 0.f, 0.f };
        czyW_Ruchu = false;

        bloczki.clear();
        for (int rzad = 0; rzad < 3; ++rzad) {
            for (int kol = 0; kol < 10; ++kol) {
                Bloczki b;
                b.shape.setSize({ 70.f, 25.f });

                // Przypisanie tekstury do każdego bloczka
                b.shape.setTexture(&blokTexture);

                //  b.shape.setFillColor(rzad == 0 ? sf::Color(0x00, 0x6b, 0x3b) : sf::Color(0x23, 0x1f, 0x20));

                switch (rzad) {
                    case 0:
                        b.shape.setFillColor(sf::Color(0x00, 0x6b, 0x3b));
                        break;
                    case 1:
                        b.shape.setFillColor(sf::Color(0x23, 0x1f, 0x20));
                        break;
                    case 2:
                        b.shape.setFillColor(sf::Color(0xb0, 0x10, 0x28));
                        break;
                }

                b.shape.setPosition({ 40.f + kol * 75.f, 50.f + rzad * 30.f });
                bloczki.push_back(b);
            }
        }
    };

    // Pierwsze uruchomienie (inicjalizacja bloczków)
    resetGry();

    std::random_device rd;
    std::mt19937 gen(rd());

    // --- PĘTLA GŁÓWNA ---
    while (window.isOpen()) {
        while (const std::optional event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) window.close();

            if (aktualnyStan == Stan::MENU) {
                if (event->is<sf::Event::KeyPressed>()) {
                    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Enter)) {

                        menuText.setString("\tARKANOID\nNacisnij ENTER aby zaczac");

                        // Ponowne centrowanie tekstu (bo zmieniła się jego długość)
                        sf::FloatRect bounds = menuText.getLocalBounds();
                        menuText.setOrigin({ bounds.size.x / 2.f, bounds.size.y / 2.f });
                        menuText.setPosition({ szerokosc / 2.f, wysokosc / 2.f });

                        aktualnyStan = Stan::GRA;
                        resetGry();
                    }
                }
            }
            else if (aktualnyStan == Stan::GRA) {
                // Start kulki (spacja lub strzałka, tutaj dowolny klawisz jeśli nie w ruchu)
                if (event->is<sf::Event::KeyPressed>() && !czyW_Ruchu) {
                    predkosc = { 0.f, -moc };
                    czyW_Ruchu = true;
                }
            }
        }

        if (aktualnyStan == Stan::GRA) {
            // --- RUCH PALETKI I BLOKADA ---
            float ruchX = 0.f;
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left)) ruchX = -8.f;
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right)) ruchX = 8.f;

            paletka.move({ ruchX, 0.f });

            // Blokada wyjeżdżania za ekran (Clamp)
            sf::Vector2f pozycjaPaletki = paletka.getPosition();
            float polSzerokosci = paletka.getSize().x / 2.f;

            if (pozycjaPaletki.x - polSzerokosci < 0.f) {
                paletka.setPosition({ polSzerokosci, pozycjaPaletki.y });
            }
            else if (pozycjaPaletki.x + polSzerokosci > szerokosc) {
                paletka.setPosition({ szerokosc - polSzerokosci, pozycjaPaletki.y });
            }

            // --- FIZYKA KULKI ---
            if (czyW_Ruchu) {
                kulka.move(predkosc);
                kulka.rotate(sf::degrees(predkosc.x * 3.0f));
                sf::Vector2f pos = kulka.getPosition();

                // Odbicia od ścian
                if (pos.x - promien <= 0 || pos.x + promien >= szerokosc) predkosc.x = -predkosc.x;
                if (pos.y - promien <= 0) predkosc.y = -predkosc.y;

                // --- PRZEGRANA ---
                if (pos.y + promien >= wysokosc) {
                    aktualnyStan = Stan::MENU;
                    menuText.setString("GAME OVER\nNacisnij ENTER");

                    // Centrujemy tekst przegranej
                    sf::FloatRect bounds = menuText.getLocalBounds();
                    menuText.setOrigin({ bounds.size.x / 2.f, bounds.size.y / 2.f });
                    menuText.setPosition({ szerokosc / 2.f, wysokosc / 2.f });
                }

                // --- WYGRANA ---
                // Jeśli punkty równe liczbie wszystkich bloczków
                if (points >= bloczki.size()) {
                    aktualnyStan = Stan::MENU;
                    menuText.setString("WYGRALES!\nNacisnij ENTER");

                    // Centrujemy tekst wygranej
                    sf::FloatRect bounds = menuText.getLocalBounds();
                    menuText.setOrigin({ bounds.size.x / 2.f, bounds.size.y / 2.f });
                    menuText.setPosition({ szerokosc / 2.f, wysokosc / 2.f });
                }

                // Odbicie od paletki
                if (kulka.getGlobalBounds().findIntersection(paletka.getGlobalBounds())) {
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
                    kulka.setPosition({ pos.x, paletka.getPosition().y - 15.f });
                }

                // Zbijanie bloczków
                for (auto& b : bloczki) {
                    if (!b.zniszczony && kulka.getGlobalBounds().findIntersection(b.shape.getGlobalBounds())) {
                        b.zniszczony = true;
                        
                        // Hit sound
                        std::uniform_real_distribution<float> pitchDist(0.8f, 1.2f);
                        hitSound.setPitch(pitchDist(gen));
                        hitSound.play();

                        points++; // Dodajemy punkt
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
            window.draw(menuText);
        }
        else {
            // W grze rysujemy obiekty
            window.draw(paletka);
            for (const auto& b : bloczki) if (!b.zniszczony) window.draw(b.shape);
            window.draw(kulka);
        }

        window.display();
    }
    return 0;
}