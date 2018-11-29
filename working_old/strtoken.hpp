#include <iostream>
#include <string>
#include <vector>
#include <algorithm>

void tokenize(const std::string& s, const char delim,
              std::vector<std::string>& out)
{
    std::string::size_type beg = 0;
    for (auto end = 0; (end = s.find(delim, end)) != std::string::npos; ++end)
    {
        out.push_back(s.substr(beg, end - beg));
        beg = end + 1;
    }

    out.push_back(s.substr(beg));
}
