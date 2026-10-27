#include <iostream>

#include "imgui.h"
#include "imgui-SFML.h"

#include <SFML/Graphics.hpp>
#include <thread>

#include "ConcurrentVector.h"


//std::vector<int> something;
ConcurrentVector<int> something = ConcurrentVector<int>();

void DoSomething()
{
    for (int i = 0; i < 10000; i++)
    {
        something.PushBack(10);
    }
}

void DoSomething2()
{
    int counter = 0;
    for (int i = 0; i < something.Size() - 1; i++)
    {
        std::cout << something.TryGet(i) << std::endl;
        counter++;
    }
    std::cout << counter << std::endl;
}

int main() {
    sf::RenderWindow window;
    window.create(sf::VideoMode({ 1280, 720 }), "My window");
    window.setFramerateLimit(60);
    window.setVerticalSyncEnabled(true);

    if (!ImGui::SFML::Init(window))
        return -1;

    // thread testing
    std::thread t(DoSomething);
    std::thread t2(DoSomething);

    t.join();
    t2.join();

    DoSomething2();



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
        ImGui::SFML::Update(window, deltaClock.restart());
        ImGui::ShowDemoWindow();

        // Render
        window.clear(sf::Color::Cyan);

        ImGui::SFML::Render(window);

        window.display();
    }

	return 0;
}
