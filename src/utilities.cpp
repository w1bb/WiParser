////////////////////////////////////////////////////////////////////////////////
// Copyright 2023 Valentin-Ioan Vintilă
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the “Software”), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
////////////////////////////////////////////////////////////////////////////////

#include "utilities.hpp"

namespace wi {
// -----


bool string_starts_with(std::string s, std::string prefix, std::size_t index)
{
    if (index + prefix.size() > s.size())
        return false;
    for (std::size_t i = 0; i < prefix.size(); ++i) {
        if (s[i + index] != prefix[i])
            return false;
    }
    return true;
}

std::vector<std::any> flatten_vector(std::any pot_v)
{
    std::vector<std::any> result;
    if (pot_v.type() != typeid(std::vector<std::any>)) {
        result.emplace_back(pot_v);
        return result;
    }
    std::vector<std::any> v = std::any_cast< std::vector<std::any> >(pot_v);
    for (std::any& a : v) {
        if (a.type() == typeid(std::vector<std::any>)) {
            std::vector<std::any> aux = flatten_vector(a);
            for (std::any& x : aux)
                result.emplace_back(x);
        } else {
            result.push_back(a);
        }
    }
    return result;
}

bool any_is_smart_string(std::any a)
{
    return a.type() == typeid(std::string) ||
           a.type() == typeid(char *) ||
           a.type() == typeid(const char *);
}

std::string smart_string_any_cast(std::any a)
{
    if (a.type() == typeid(std::string))
        return std::any_cast<std::string>(a);
    if (a.type() == typeid(char *))
        return std::string(std::any_cast<char *>(a));
    if (a.type() == typeid(const char *))
        return std::string(std::any_cast<const char *>(a));
    return "??"; // throw?
}


// -----
} // namespace wi
