#include "imgui.h"
#include "imgui-SFML.h"
#include "Profiler.hpp"

#include <SFML/Graphics.hpp>

#include "Balls.hpp"

int main() {
    sf::RenderWindow window;
    window.create(sf::VideoMode({ 1280, 720 }), "My window");
    window.setFramerateLimit(60);
    window.setVerticalSyncEnabled(true);

    if (!ImGui::SFML::Init(window))
        return -1;
    auto ballGame = BallGame(window.getSize());
    auto profiler = Profiler();
    auto gameSim = new GameSimulation();
    char buttonLabel = 'p';
    char* blp = &buttonLabel;


    sf::Clock deltaClock;
    while (window.isOpen())
    {
        // Event Polling
        while (const std::optional event = window.pollEvent())
        {
            ImGui::SFML::ProcessEvent(window, *event);

            // "close requested" event: we close the window
            if (event->is<sf::Event::Closed>())
                window.close();
        }

        // Update
        auto delta = deltaClock.restart();
        ImGui::SFML::Update(window, delta);
        // ImGui::ShowDemoWindow();
        // if (ImGui::Button("Profile"))
        // {
        //     gameSim->performIntensiveTask(profiler);
        // }

        ballGame.updateBalls(window.getSize(), delta.asSeconds(), profiler);

        // Render
        window.clear(sf::Color::Cyan);

        ballGame.drawBalls(window);
        profiler.renderImGui();
        ImGui::SFML::Render(window);


        window.display();
    }

	return 0;
}
