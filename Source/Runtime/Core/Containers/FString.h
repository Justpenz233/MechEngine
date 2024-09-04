#pragma once
#include <string>
#include <vector>
#include <sstream>

namespace FString
{
    using std::vector;
    using std::string;

    inline TArray<string> SpiltBy(const string& Input, char Delimiter)
    {
        TArray<std::string> result;
        std::istringstream stream(Input);
        std::string token;

        while (std::getline(stream, token, Delimiter)) {
            result.push_back(token);
        }
        
        return result;
    }
};