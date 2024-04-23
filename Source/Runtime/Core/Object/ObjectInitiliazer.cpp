//
// Created by MarvelLi on 2024/1/18.
//

#include "ObjectInitiliazer.h"

MObjectInitiliazer& MObjectInitiliazer::Get()
{
    static MObjectInitiliazer instance;
    return instance;
}
