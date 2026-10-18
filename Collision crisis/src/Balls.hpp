#pragma once

#include <vector>
#include <random>
#include <cmath>
#include <iostream>
#include <utility>

#include <SFML/Graphics.hpp>
#include <map>



struct Ball {
    sf::CircleShape shape;
    sf::Vector2f velocity;

    Ball(float x, float y, float radius, sf::Color color, float vx, float vy) {
        shape.setRadius(radius);
        shape.setPosition(sf::Vector2f(x, y));
        shape.setFillColor(color);
        shape.setOrigin(sf::Vector2f(radius, radius)); // Center origin
        velocity = sf::Vector2f(vx, vy);
    }
};

class BallGame {
private:
    // Create balls
    std::vector<Ball> balls;
    std::random_device rd;
    std::mt19937 gen;
    std::uniform_real_distribution<float> posDist;
    std::uniform_real_distribution<float> velDist;
    std::uniform_int_distribution<int> colorDist;
    std::uniform_real_distribution<float> radiusDist;

    // hashgrid vars
    std::map<std::pair<int, int>, std::vector<Ball*>> gridMap;
    int cellSize = 10;

public:
    BallGame(const sf::Vector2u& windowSize) {

        auto ballCount = 15000;
        auto ballSize = 2.5f;

        gen = std::mt19937(rd());
        posDist = std::uniform_real_distribution<float>(5.0f, 795.0f);
        velDist = std::uniform_real_distribution<float>(-200.0f, 200.0f);
        colorDist = std::uniform_int_distribution<int>(0, 255);
        radiusDist = std::uniform_real_distribution<float>(ballSize, ballSize);



        // Generate random balls
        for (int i = 0; i < ballCount; ++i) {
            sf::Color randomColor(colorDist(gen), colorDist(gen), colorDist(gen));
            balls.emplace_back(
                posDist(gen), posDist(gen),  // position
                radiusDist(gen),             // radius
                randomColor,                 // color
                velDist(gen), velDist(gen)   // velocity
            );
        }

        setupGrid(windowSize);
    }

    void setupGrid(const sf::Vector2u& windowSize)
    {
        auto gridSizeX = windowSize.x / cellSize;
        auto gridSizeY = windowSize.y / cellSize;

        for (int i = 0; i < gridSizeX; ++i)
        {
            for (int j = 0; j < gridSizeY; ++j)
            {
                std::pair<int,int> key = std::make_pair(i, j);
                gridMap[key] = std::vector<Ball*>();
            }
        }
    }

    void clearGridMap()
    {
        for (auto& [key, val] : gridMap)
        {
            val.clear();
        }
    }

    void populateGridMap()
    {
        for (auto& ball : balls)
        {
            auto* ptr = &ball;
            auto const pos = ball.shape.getPosition();
            int const posX = pos.x;
            int const posY = pos.y;
            auto const key = std::make_pair(posX/cellSize, posY/cellSize);
            auto& homeVector = gridMap[key];
            homeVector.push_back(ptr);
        }
    }

    void resolveCollisions(Ball& ball1, Ball& ball2)
    {
        sf::Vector2f pos1 = ball1.shape.getPosition();
        sf::Vector2f pos2 = ball2.shape.getPosition();
        float radius1 = ball1.shape.getRadius();
        float radius2 = ball2.shape.getRadius();

        // Calculate distance between centers
        sf::Vector2f delta = pos2 - pos1;
        float distance = std::sqrt(delta.x * delta.x + delta.y * delta.y);
        float minDistance = radius1 + radius2;

        if (distance < minDistance && distance > 0) {
            // Normalize collision vector
            sf::Vector2f normal = delta / distance;

            // Separate balls to prevent overlap
            float overlap = minDistance - distance;
            sf::Vector2f separation = normal * (overlap * 0.5f);
            ball1.shape.setPosition(pos1 - separation);
            ball2.shape.setPosition(pos2 + separation);

            // Calculate relative velocity
            sf::Vector2f relativeVel = ball2.velocity - ball1.velocity;
            float velAlongNormal = relativeVel.x * normal.x + relativeVel.y * normal.y;

            // Don't resolve if velocities are separating
            if (velAlongNormal > 0) return;

            // Apply collision response (elastic collision)
            float restitution = 0.0f; // Bounce factor (0 = no bounce, 1 = perfect bounce)
            float impulse = -(1 + restitution) * velAlongNormal;

            // Assume equal mass for simplicity
            sf::Vector2f impulseVector = impulse * normal;
            ball1.velocity -= impulseVector;
            ball2.velocity += impulseVector;
        }
    }

    void updateBalls(const sf::Vector2u& windowSize, float deltaTime, Profiler& profiler) {
        PROFILE(profiler, "Ball Simulation");

        clearGridMap();

        // Update positions
        for (auto& ball : balls) {
            ball.shape.move(ball.velocity * deltaTime);
        }

        populateGridMap();


        // Handle ball-to-ball collisions
        for (size_t i = 0; i < balls.size(); ++i) {
            const std::pair<int, int> neighborOffsets[4] = {
                {1, 0},
                {1, 1},
                {0, 1},
                {-1, 1}
            };

            auto const pos = balls[i].shape.getPosition();
            int const posX = pos.x;
            int const posY = pos.y;
            auto const key = std::make_pair(posX/cellSize, posY/cellSize);
            auto& gridCell = gridMap[key];

            for (auto& ball : gridCell) {
                Ball& ball1 = balls[i];
                Ball& ball2 = *ball;
                // std::cout << ball2.shape.getPosition().x << std::endl;

                resolveCollisions(ball1, ball2);
                }

            for (const auto& offset : neighborOffsets)
            {
                auto const neighborKey = std::make_pair(key.first + offset.first, key.second + offset.second);
                auto& neighborCell = gridMap[key];
                for (auto& ball : neighborCell) {
                    Ball& ball1 = balls[i];
                    Ball& ball2 = *ball;
                    // std::cout << ball2.shape.getPosition().x << std::endl;

                    resolveCollisions(ball1, ball2);
                }
            }

            }
        // Handle wall collisions
        for (auto& ball : balls) {
            sf::Vector2f pos = ball.shape.getPosition();
            float radius = ball.shape.getRadius();

            // Bounce off walls
            if (pos.x - radius <= 0 || pos.x + radius >= windowSize.x) {
                ball.velocity.x = -ball.velocity.x;
                // Clamp position to prevent sticking
                if (pos.x - radius <= 0) {
                    ball.shape.setPosition(sf::Vector2f(radius, pos.y));
                }
                else {
                    ball.shape.setPosition(sf::Vector2f(windowSize.x - radius, pos.y));
                }
            }

            if (pos.y - radius <= 0 || pos.y + radius >= windowSize.y) {
                ball.velocity.y = -ball.velocity.y;
                // Clamp position to prevent sticking
                if (pos.y - radius <= 0) {
                    ball.shape.setPosition(sf::Vector2f(pos.x, radius));
                }
                else {
                    ball.shape.setPosition(sf::Vector2f(pos.x, windowSize.y - radius));
                }
            }
        }
    }

    void drawBalls( sf::RenderWindow& window ) const
    {
        for (const auto& ball : balls) {
            window.draw(ball.shape);
        }
    }
};