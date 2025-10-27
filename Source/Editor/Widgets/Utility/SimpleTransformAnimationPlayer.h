//
// Created by MarvelLi on 2025/9/23.
//

#pragma once
#include "UIWidget.h"
#include "Math/FTransform.h"
#include <ImguiPlus.h>
#include "Game/Actor.h"

class SimpleTransformAnimationPlayer: public UIWidget
{
public:
    SimpleTransformAnimationPlayer() : UIWidget("SimpleTransformAnimationPlayer") {}

    void SetDuration(double InDuration) { Duration = InDuration; }

    /* Add a track to the animation player.
     * @param Actor The actor to animate
    */
    void AddTrack(const ObjectPtr<Actor>& Actor, TArray<FTransform> Transforms = {});

    virtual void Draw() override;

    void ExportToGltf(std::string& Path, bool bBinary = true);

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
    void ApplyCurrentFrame();

    static FTransform GetInterpolatedTransform(const TArray<FTransform>& Transforms, double InPercent);
};
