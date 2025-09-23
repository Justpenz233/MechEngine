//
// Created by MarvelLi on 2025/9/23.
//

#pragma once
#include "UIWidget.h"
#include "Math/FTransform.h"
#include <ImguiPlus.h>
#include "Math/Math.h"
#include "Game/Actor.h"

class SimpleTransformAnimationPlayer: public UIWidget
{
public:
    SimpleTransformAnimationPlayer() : UIWidget("SimpleTransformAnimationPlayer") {}

    void SetDuration(double InDuration) { Duration = InDuration; }

    void AddTrack(ObjectPtr<Actor> Actor, TArray<FTransform> Transforms)
    {
        Tracks.push_back({Actor, Transforms});
    }

    virtual void Draw() override
    {
        ImGui::Begin("SimpleTransformAnimationPlayer");

        // Animation Controls
        ImGui::Text("Animation Player");
        ImGui::Separator();

        // Duration control
        float durationFloat = static_cast<float>(Duration);
        if (ImGui::InputFloat("Duration (s)", &durationFloat, 0.1f, 1.0f, "%.2f"))
        {
            Duration = static_cast<double>(durationFloat);
        }

        // Progress slider
        float percentFloat = static_cast<float>(Percent);
        if (ImGui::SliderFloat("Progress", &percentFloat, 0.0f, 1.0f, "%.3f"))
        {
            Percent = static_cast<double>(percentFloat);
            ApplyCurrentFrame();
        }

        // Playback controls
        ImGui::Spacing();
        if (ImGui::Button("Play"))
        {
            lastTime = ImGui::GetTime();
            bIsPlaying = true;
        }
        ImGui::SameLine();
        if (ImGui::Button("Pause"))
        {
            bIsPlaying = false;
        }
        ImGui::SameLine();
        if (ImGui::Button("Stop"))
        {
            bIsPlaying = false;
            Percent = 0.0;
            ApplyCurrentFrame();
        }
        ImGui::SameLine();
        if (ImGui::Button("Reset"))
        {
            Percent = 0.0;
            bIsPlaying = false;
            ApplyCurrentFrame();
        }

        // Time display
        double currentTime = Percent * Duration;
        ImGui::Text("Time: %.2f / %.2f seconds", currentTime, Duration);

        // Track information
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Text("Tracks (%zu)", Tracks.size());

        // Display tracks
        for (size_t i = 0; i < Tracks.size(); ++i)
        {
            const Track& track = Tracks[i];

            ImGui::PushID(static_cast<int>(i));

            // Track header
            bool trackOpen = ImGui::TreeNode(("Track " + std::to_string(i)).c_str());

            // Track controls on same line
            ImGui::SameLine();
            if (ImGui::SmallButton("Remove"))
            {
                Tracks.erase(Tracks.begin() + i);
                ImGui::PopID();
                break; // Exit loop since we modified the container
            }

            if (trackOpen)
            {
                // Actor information
                if (track.Actor)
                {
                    ImGui::Text("Actor: Valid (%s)", track.Actor->GetName().c_str());
                }
                else
                {
                    ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.5f, 1.0f), "Actor: Invalid/Null");
                }

                // Transform count
                ImGui::Text("Keyframes: %zu", track.Transforms.size());

                // Show current transform if valid
                if (!track.Transforms.empty() && track.Actor)
                {
                    int currentKeyframe = static_cast<int>(Percent * (track.Transforms.size() - 1));
                    currentKeyframe = std::min(currentKeyframe, static_cast<int>(track.Transforms.size() - 1));

                    const FTransform& currentTransform = track.Transforms[currentKeyframe];
                    ImGui::Text("Current Keyframe: %d", currentKeyframe);
                }

                ImGui::TreePop();
            }

            ImGui::PopID();
        }

        // Update animation if playing
        if (bIsPlaying)
        {
            // Simple time update (you'd typically use a proper timer)
            double currentFrameTime = ImGui::GetTime();
            double deltaTime = currentFrameTime - lastTime;
            lastTime = currentFrameTime;

            Percent += deltaTime / Duration;

            if (Percent >= 1.0)
            {
                if (bLoop)
                {
                    Percent = 0.0;
                }
                else
                {
                    Percent = 1.0;
                    bIsPlaying = false;
                }
            }

            ApplyCurrentFrame();
        }

        // Animation settings
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Checkbox("Loop", &bLoop);

        ImGui::End();
    }

protected:
    struct Track {
        ObjectPtr<Actor> Actor;
        TArray<FTransform> Transforms;
    };

    double Duration = 2.0;
    double Percent = 0.0;
    TArray<Track> Tracks;
    double lastTime = 0.0;
    bool bIsPlaying = false;
    bool bLoop = false;

private:
    void ApplyCurrentFrame()
    {
        for (const Track& track : Tracks)
        {
            if (!track.Actor || track.Transforms.empty())
                continue;

            // Calculate interpolated transform
            FTransform interpolatedTransform = GetInterpolatedTransform(track.Transforms, Percent);

            // Apply transform to actor (assuming Actor has a SetTransform method)
            track.Actor->SetTransform(interpolatedTransform);
        }
    }

    FTransform GetInterpolatedTransform(const TArray<FTransform>& Transforms, double InPercent)
    {
        if (Transforms.empty())
            return FTransform::Identity();

        if (Transforms.size() == 1)
            return Transforms[0];

        // Calculate indices for interpolation
        double scaledPercent = InPercent * (Transforms.size() - 1);
        int lowerIndex = static_cast<int>(floor(scaledPercent));
        int upperIndex = lowerIndex + 1;

        // Clamp indices
        lowerIndex = std::max(0, std::min(lowerIndex, static_cast<int>(Transforms.size() - 1)));
        upperIndex = std::max(0, std::min(upperIndex, static_cast<int>(Transforms.size() - 1)));

        // If indices are the same, no interpolation needed
        if (lowerIndex == upperIndex)
            return Transforms[lowerIndex];

        // Calculate interpolation factor
        double t = scaledPercent - lowerIndex;

        // Interpolate between transforms
        const FTransform& transform1 = Transforms[lowerIndex];
        const FTransform& transform2 = Transforms[upperIndex];

        FTransform result;
        result.SetTranslation(Math::Lerp(transform1.GetTranslation(), transform2.GetTranslation(), t));
        result.SetRotation(transform1.GetRotation().slerp(t, transform2.GetRotation()));
        result.SetScale(Math::Lerp(transform1.GetScale(), transform2.GetScale(), t));

        return result;
    }
};
