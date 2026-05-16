#pragma once

#include <chrono>
#include <cstdint>
#include <string>
#include <unordered_map>

namespace core
{
    struct FrameStats
    {
        float deltaTime = 0.0f;
        float fps = 0.0f;
        float frameTimeMs = 0.0f;
    };

    struct ProfileStats
    {
        float lastMs = 0.0f;
        float averageMs = 0.0f;
        uint32_t callCount = 0;
    };

    class Time
    {
    public:
        Time()
            : m_lastTime(Clock::now())
        {
        }

        void Update()
        {
            auto now = Clock::now();

            std::chrono::duration<float> delta = now - m_lastTime;
            m_frameStats.deltaTime = delta.count();

            m_lastTime = now;

            m_frameStats.frameTimeMs = m_frameStats.deltaTime * 1000.0f;

            m_elapsedTime += m_frameStats.deltaTime;
            m_frameCount++;

            if (m_elapsedTime >= 1.0f)
            {
                m_frameStats.fps = static_cast<float>(m_frameCount) / m_elapsedTime;

                m_frameCount = 0;
                m_elapsedTime = 0.0f;
            }
        }

        void Begin(const std::string& category, const std::string& name)
        {
            m_activeProfiles[category][name] = Clock::now();
        }

        void End(const std::string& category, const std::string& name)
        {
            auto categoryIt = m_activeProfiles.find(category);
            if (categoryIt == m_activeProfiles.end())
                return;

            auto profileIt = categoryIt->second.find(name);
            if (profileIt == categoryIt->second.end())
                return;

            auto now = Clock::now();

            std::chrono::duration<float, std::milli> duration = now - profileIt->second;
            float elapsedMs = duration.count();

            ProfileStats& stats = m_profiles[category][name];

            stats.lastMs = elapsedMs;
            stats.callCount++;

            if (stats.callCount == 1)
            {
                stats.averageMs = elapsedMs;
            }
            else
            {
                constexpr float smoothing = 0.1f;

                stats.averageMs =
                    stats.averageMs * (1.0f - smoothing) +
                    elapsedMs * smoothing;
            }

            categoryIt->second.erase(profileIt);

            if (categoryIt->second.empty())
                m_activeProfiles.erase(categoryIt);
        }

        // Version compatible avec ton ancien code.
        void Begin(const std::string& name)
        {
            Begin("Default", name);
        }

        void End(const std::string& name)
        {
            End("Default", name);
        }

        const FrameStats& GetFrameStats() const
        {
            return m_frameStats;
        }

        float GetDeltaTime() const
        {
            return m_frameStats.deltaTime;
        }

        const std::unordered_map<std::string, std::unordered_map<std::string, ProfileStats>>& GetProfiles() const
        {
            return m_profiles;
        }

        const std::unordered_map<std::string, ProfileStats>* GetCategory(const std::string& category) const
        {
            auto it = m_profiles.find(category);

            if (it == m_profiles.end())
                return nullptr;

            return &it->second;
        }

        const ProfileStats* GetProfile(const std::string& category, const std::string& name) const
        {
            auto categoryIt = m_profiles.find(category);

            if (categoryIt == m_profiles.end())
                return nullptr;

            auto profileIt = categoryIt->second.find(name);

            if (profileIt == categoryIt->second.end())
                return nullptr;

            return &profileIt->second;
        }

        const ProfileStats* GetProfile(const std::string& name) const
        {
            return GetProfile("Default", name);
        }

        void ClearProfiles()
        {
            m_profiles.clear();
            m_activeProfiles.clear();
        }

    private:
        using Clock = std::chrono::steady_clock;

        Clock::time_point m_lastTime;

        FrameStats m_frameStats;

        float m_elapsedTime = 0.0f;
        uint32_t m_frameCount = 0;

        std::unordered_map<std::string, std::unordered_map<std::string, Clock::time_point>> m_activeProfiles;
        std::unordered_map<std::string, std::unordered_map<std::string, ProfileStats>> m_profiles;
    };
}