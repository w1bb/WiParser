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

#ifndef _WI_UTILITIES_HPP_
#define _WI_UTILITIES_HPP_ "1.0.2b"

#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <any>


namespace wi {
// -----


bool string_starts_with(std::string s, std::string prefix, std::size_t index = 0);

std::vector<std::any> flatten_vector(std::any pot_v);

bool any_is_smart_string(std::any a);

std::string smart_string_any_cast(std::any a);


// -----


template<bool>
static std::string vector_to_string(std::vector<std::any>&);

// This function is limited
template<bool use_quotes = false>
static std::string any_to_string(const std::any& x)
{
    if (x.type() == typeid(std::vector<std::any>)) {
        std::vector<std::any> v = std::any_cast< std::vector<std::any> >(x);
        return vector_to_string<use_quotes>(v);
    } else if (any_is_smart_string(x)) {
        std::string aux = smart_string_any_cast(x);
        if constexpr (use_quotes) {
            return "\"" + aux + "\"";
        } else {
            return aux;
        }
    } else if (x.type() == typeid(int)) {
        return std::to_string(std::any_cast<int>(x));
    } else {
        // throw for now
        throw "any_to_string type: " + std::string(x.type().name());
    }
}

template<bool use_quotes = false>
static std::string vector_to_string(std::vector<std::any>& v)
{
    if (v.size() == 0)
        return "[]";
    std::stringstream ss;
    ss << "[";
    for(std::size_t i = 0; i < v.size() - 1; ++i)
        ss << any_to_string<use_quotes>(v[i]) << ", ";
    ss << any_to_string<use_quotes>(v[v.size() - 1]) << "]";
    return ss.str();
}

template<bool use_ellipsis = false>
static std::string string_at_most(std::string s, std::size_t at_most, std::size_t from = 0)
{
    if (at_most == 0 || from > s.size())
        return "";

    std::string result;
    for (std::size_t i = from; i < s.size(); ++i) {
        result.push_back(s[i]);
        if ((--at_most) == 0) {
            if constexpr (use_ellipsis) {
                if (i + 1 < s.size())
                    return result + "...";
            }
            return result;
        }
    }
    return result;
}


// -----
} // namespace wi
#endif  // _WI_UTILITIES_HPP_
