//
// Created by MarvelLi on 2024/4/30.
//

#pragma once


/**
* Enum to specify the view buffer mode
*/
enum ViewMode : int
{
    FrameBuffer = 0,
    DepthBuffer = 1,
    NormalWorldBuffer = 2,
    BaseColorBuffer = 3,
    InstanceIDBuffer = 4,
    // MaterialIDBuffer,
};
