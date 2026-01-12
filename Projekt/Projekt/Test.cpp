#include <SFML/Graphics.hpp>
#include <cmath>
#include <random>
#include <optional>
#include <vector>

// Definiujemy możliwe stany gry
enum class Stan { MENU, GRA };

int main() {
    const unsigned int szerokosc = 800;
    const unsigned int wysokosc = 600;
    sf::RenderWindow window(sf::VideoMode({ szerokosc, wysokosc }), "Arkanoid z Menu - SFML 3");
    window.setFramerateLimit(60);

    Stan aktualnyStan = Stan::MENU;

    // --- CZCIONKA I TEKST MENU ---
    sf::Font font;
    if (!font.openFromFile("arial.ttf")) { // Upewnij się, że masz ten plik!
        return -1;
    }

    sf::Text menuText(font);
    menuText.setString("ARKANOID\nNacisnij ENTER aby zaczac");
    menuText.setCharacterSize(40);
    menuText.setFillColor(sf::Color::White);
    // Centrowanie tekstu
    sf::FloatRect textBounds = menuText.getLocalBounds();
    menuText.setOrigin({ textBounds.size.x / 2.f, textBounds.size.y / 2.f });
    menuText.setPosition({ szerokosc / 2.f, wysokosc / 2.f });

    // --- ZMIENNE GRY (KULKA, PALETKA, BLOCZKI) ---
    float promien = 8.f;
    sf::CircleShape kulka(promien);
    kulka.setFillColor(sf::Color::Cyan);
    kulka.setOrigin({ promien, promien });

    sf::Vector2f paletkaRozmiar(120.f, 15.f);
    sf::RectangleShape paletka(paletkaRozmiar);
    paletka.setFillColor(sf::Color::White);
    paletka.setOrigin({ paletkaRozmiar.x / 2.f, paletkaRozmiar.y / 2.f });

    sf::Vector2f predkosc(0.f, 0.f);
    bool czyW_Ruchu = false;
    float moc = 6.0f;

    struct Bloczki {
        sf::RectangleShape shape;
        bool zniszczony = false;
    };
    std::vector<Bloczki> bloczki;

    // Funkcja resetująca grę
    auto resetGry = [&]() {
        kulka.setPosition({ szerokosc / 2.f, wysokosc / 2.f + 50.f });
        paletka.setPosition({ szerokosc / 2.f, wysokosc - 40.f });
        predkosc = { 0.f, 0.f };
        czyW_Ruchu = false;
        bloczki.clear();
        for (int rzad = 0; rzad < 2; ++rzad) {
            for (int kol = 0; kol < 10; ++kol) {
                Bloczki b;
                b.shape.setSize({ 70.f, 25.f });
                b.shape.setFillColor(rzad == 0 ? sf::Color::Red : sf::Color::Yellow);
                b.shape.setPosition({ 40.f + kol * 75.f, 50.f + rzad * 30.f });
                bloczki.push_back(b);
            }
        }
        };

    resetGry();
    std::random_device rd;
    std::mt19937 gen(rd());

    while (window.isOpen()) {
        while (const std::optional event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) window.close();

            if (aktualnyStan == Stan::MENU) {
                if (event->is<sf::Event::KeyPressed>()) {
                    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Enter)) {
                        aktualnyStan = Stan::GRA;
                        resetGry();
                    }
                }
            }
            else if (aktualnyStan == Stan::GRA) {
                if (event->is<sf::Event::KeyPressed>() && !czyW_Ruchu) {
                    predkosc = { 0.f, -moc };
                    czyW_Ruchu = true;
                }
            }
        }

        if (aktualnyStan == Stan::GRA) {
            // --- LOGIKA GRY ---
            float ruchX = 0.f;
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left)) ruchX = -8.f;
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right)) ruchX = 8.f;
            paletka.move({ ruchX, 0.f });

            if (czyW_Ruchu) {
                kulka.move(predkosc);
                sf::Vector2f pos = kulka.getPosition();

                if (pos.x - promien <= 0 || pos.x + promien >= szerokosc) predkosc.x = -predkosc.x;
                if (pos.y - promien <= 0) predkosc.y = -predkosc.y;
                if (pos.y + promien >= wysokosc) aktualnyStan = Stan::MENU; // Powrót do menu po przegranej

                if (kulka.getGlobalBounds().findIntersection(paletka.getGlobalBounds())) {
                    // Twoja logika kątów (skrócona dla przejrzystości)
                    float kat = (ruchX == 0) ? 270.f : (ruchX < 0 ? 220.f : 320.f);
                    float rad = kat * 3.14159f / 180.f;
                    predkosc = { std::cos(rad) * moc, std::sin(rad) * moc };
                    kulka.setPosition({ pos.x, paletka.getPosition().y - 15.f });
                }

                for (auto& b : bloczki) {
                    if (!b.zniszczony && kulka.getGlobalBounds().findIntersection(b.shape.getGlobalBounds())) {
                        b.zniszczony = true;
                        predkosc.y = -predkosc.y;
                        break;
                    }
                }
            }
        }

        // --- RENDEROWANIE ---
        window.clear(sf::Color(30, 30, 30));

        if (aktualnyStan == Stan::MENU) {
            window.draw(menuText);
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