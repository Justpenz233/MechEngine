#include <string>
#include <vector>
#include <sstream>

namespace FString
{
    using std::vector;
    using std::string;

    inline vector<string> SpiltBy(const string& Input, char Delimiter)
    {
        std::vector<std::string> result;
        std::istringstream stream(Input);
        std::string token;

        while (std::getline(stream, token, Delimiter)) {
            result.push_back(token);
        }
        
        return result;
    }
};