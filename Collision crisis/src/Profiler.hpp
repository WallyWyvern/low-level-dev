#include <SFML/Graphics.hpp>
#include <imgui.h>
#include <imgui-SFML.h>
#include <chrono>
#include <vector>
#include <string>
#include <algorithm>
#include <cmath>
#include <random>

// Macro for easy profiling
#define PROFILE(profiler, name) Profiler::ScopedTimer timer(profiler, name)

class Profiler {
private:
    struct ProfileData {
        std::string name;
        float duration_ms;
        std::vector<float> history;
        static constexpr size_t MAX_HISTORY = 120; // 2 seconds at 60fps
    };

    std::vector<ProfileData> profiles;

public:
    class ScopedTimer {
        Profiler& profiler;
        std::string name;
        std::chrono::high_resolution_clock::time_point start_time;

    public:
        ScopedTimer(Profiler& p, const std::string& n)
            : profiler(p), name(n), start_time(std::chrono::high_resolution_clock::now()) {
        }

        ~ScopedTimer() {
            auto end_time = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);
            profiler.addSample(name, duration.count() / 1000.0f); // Convert to milliseconds
        }
    };

    void addSample(const std::string& name, float duration_ms) {
        // Find or create profile data
        auto it = std::find_if(profiles.begin(), profiles.end(),
            [&name](const ProfileData& data) { return data.name == name; });

        if (it == profiles.end()) {
            profiles.emplace_back();
            it = profiles.end() - 1;
            it->name = name;
        }

        it->duration_ms = duration_ms;
        it->history.push_back(duration_ms);

        // Keep history bounded
        if (it->history.size() > ProfileData::MAX_HISTORY) {
            it->history.erase(it->history.begin());
        }
    }

    void renderImGui() {
        ImGui::Begin("Performance Profiler");

        float total_frame_time = 0.0f;
        for (const auto& profile : profiles) {
            total_frame_time += profile.duration_ms;
        }

        ImGui::Text("Total Frame Time: %.3f ms (%.1f FPS)",
            total_frame_time, 1000.0f / std::max(total_frame_time, 0.001f));

        ImGui::Separator();

        for (const auto& profile : profiles) {
            if (profile.history.empty()) continue;

            // Calculate statistics
            float min_time = *std::min_element(profile.history.begin(), profile.history.end());
            float max_time = *std::max_element(profile.history.begin(), profile.history.end());
            float avg_time = 0.0f;
            for (float t : profile.history) avg_time += t;
            avg_time /= profile.history.size();

            // Display current time and statistics
            ImGui::Text("%s: %.3f ms", profile.name.c_str(), profile.duration_ms);
            ImGui::SameLine();
            ImGui::TextDisabled("(avg: %.3f, min: %.3f, max: %.3f)", avg_time, min_time, max_time);

            // Create the graph
            std::string graph_id = "##" + profile.name;
            ImGui::PlotLines(graph_id.c_str(),
                profile.history.data(),
                profile.history.size(),
                0,
                nullptr,
                0.0f,
                max_time * 1.2f, // Scale graph to show spikes
                ImVec2(0, 80));
        }

        ImGui::End();
    }

    void clear() {
        for (auto& profile : profiles) {
            profile.history.clear();
        }
    }
};

class GameSimulation {
    std::vector<sf::Vector2f> particles;
    std::vector<sf::Vector2f> velocities;
    std::vector<sf::CircleShape> shapes;
    std::mt19937 rng{ std::random_device{}() };

public:
    void initialize(int count) {
        particles.clear();
        velocities.clear();
        shapes.clear();

        std::uniform_real_distribution<float> pos_dist(0.0f, 800.0f);
        std::uniform_real_distribution<float> vel_dist(-100.0f, 100.0f);

        for (int i = 0; i < count; ++i) {
            particles.emplace_back(pos_dist(rng), pos_dist(rng));
            velocities.emplace_back(vel_dist(rng), vel_dist(rng));

            sf::CircleShape shape(2.0f);
            shape.setFillColor(sf::Color(rand() % 255, rand() % 255, rand() % 255));
            shapes.emplace_back(shape);
        }
    }

    void update(float dt, Profiler& profiler) {
        PROFILE(profiler, "Physics Update");

        // Update particle positions
        for (size_t i = 0; i < particles.size(); ++i) {
            particles[i] += velocities[i] * dt;

            // Bounce off walls
            if (particles[i].x < 0 || particles[i].x > 800) {
                velocities[i].x = -velocities[i].x;
                particles[i].x = std::clamp(particles[i].x, 0.0f, 800.0f);
            }
            if (particles[i].y < 0 || particles[i].y > 600) {
                velocities[i].y = -velocities[i].y;
                particles[i].y = std::clamp(particles[i].y, 0.0f, 600.0f);
            }
        }
    }

    void render(sf::RenderWindow& window, Profiler& profiler) {
        PROFILE(profiler, "Particle Rendering");

        for (size_t i = 0; i < particles.size(); ++i) {
            shapes[i].setPosition(particles[i]);
            window.draw(shapes[i]);
        }
    }

    void performIntensiveTask(Profiler& profiler, int iterations = 1000000) {
        PROFILE(profiler, "Intensive Task");

        // Simulate expensive computation - calculate lots of sine/cosine operations
        volatile float result = 0.0f; // volatile prevents optimization
        for (int i = 0; i < iterations; ++i) {
            float x = static_cast<float>(i) * 0.001f;
            result += std::sin(x) * std::cos(x) * std::sqrt(x + 1.0f);
        }
    }

    size_t getParticleCount() const { return particles.size(); }
};