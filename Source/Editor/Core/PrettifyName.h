//
// Created by MarvelLi on 2024/5/10.
//

#pragma once
#include "ContainerTypes.h"

namespace MechEngine::UI
{
/**
 * Pretify the name of object name
 * Add space between the words by identifying the upper case letter
 * Would remove the prefix 'b' of the bool name
 * @param Name the name of the object
 * @return the pretified name
 */
inline String PretifyUIName(const String& Name)
{
	String Result;
	for (auto i = 0; i < Name.size(); i++)
	{
		if (i > 0 && isupper(Name[i]) && islower(Name[i - 1]))
		{
			Result += ' ';
		}
		Result += Name[i];
	}
	if (Result[0] == 'b')
		return Result.substr(1);
	return Result;
}
};