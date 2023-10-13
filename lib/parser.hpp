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

#ifndef _WI_PARSER_HPP_
#define _WI_PARSER_HPP_ "1.0.1a"


namespace wi {
class parser_state_t;
class parser_t;
class lazy_parser_t;
class sequence_of_parser_t;
class choice_of_parser_t;
class many_parser_t;
class many1_parser_t;
class between_parser_t;
class separated_by_parser_t;
class exact_string_parser_t;
class char_parser_t;
class letter_parser_t;
class digit_parser_t;
class whitespace_parser_t;
class chars_parser_t;
class letters_parser_t;
class digits_parser_t;
class whitespaces_parser_t;
class maybe_chars_parser_t;
class maybe_letters_parser_t;
class maybe_digits_parser_t;
class maybe_whitespaces_parser_t;
}


// -----


#include "utilities.hpp"

#include <functional>
#include <cstdint>
#include <sstream>
#include <string>
#include <vector>
#include <regex>
#include <any>


namespace wi {
// -----


class parser_state_t {
public:
    std::string target_string;
    std::any result;
    std::size_t index;
    bool is_error;
    std::string error;

    parser_state_t();
    parser_state_t(std::string _target_string);

    parser_state_t& set_target_string(std::string _target_string);
    parser_state_t& set_result(std::any _result);
    parser_state_t& set_index(std::size_t _index);
    parser_state_t& set_is_error(bool _is_error);
    parser_state_t& set_error(std::string _error);

    parser_state_t map_result(std::function<std::any(std::any)> f) const;
    parser_state_t map_nested_result(std::function<std::any(std::string)> f) const;
    parser_state_t map_error(std::function<std::string(std::string)> f) const;

    parser_state_t chain(std::function<parser_t*(std::any)> f) const;

    parser_state_t flatten_result() const;

    std::string to_string() const;
};


// -----


class parser_t {
public:
    virtual parser_state_t run(parser_state_t parser_state) const;
};


// -----


class do_nothing_parser_t : public parser_t {
public:
    do_nothing_parser_t();
    parser_state_t run(parser_state_t parser_state) const;
};


// -----


class lazy_parser_t : public parser_t {
    parser_t *parser;

public:
    lazy_parser_t();
    lazy_parser_t(parser_t *_parser);
    parser_state_t run(parser_state_t parser_state) const;

    lazy_parser_t& set_parser(parser_t *_parser);
};


// -----


class map_parser_t : public parser_t {
    parser_t *parser;
    std::function<std::any(std::any)> f;

public:
    map_parser_t();
    map_parser_t(parser_t *_parser, std::function<std::any(std::any)> _f);

    parser_state_t run(parser_state_t parser_state) const;

    map_parser_t& set_parser(parser_t *_parser);
    map_parser_t& set_f(std::function<std::any(std::any)> _f);
};


// -----


class chain_parser_t : public parser_t {
    parser_t *parser;
    std::function<parser_t*(std::any)> f;

public:
    chain_parser_t();
    chain_parser_t(parser_t *_parser, std::function<parser_t*(std::any)> f);

    parser_state_t run(parser_state_t parser_state) const;

    chain_parser_t& set_parser(parser_t *_parser);
    chain_parser_t& set_f(std::function<parser_t*(std::any)> _f);
};


// -----


class flatten_parser_t : public parser_t {
    parser_t *parser;

public:
    flatten_parser_t();
    flatten_parser_t(parser_t *_parser);

    parser_state_t run(parser_state_t parser_state) const;

    flatten_parser_t& set_parser(parser_t *_parser);
};


// -----


class sequence_of_parser_t : public parser_t {
    std::vector<parser_t*> parsers;

public:
    sequence_of_parser_t();
    sequence_of_parser_t(std::vector<parser_t*> _parsers);
    parser_state_t run(parser_state_t parser_state) const;

    void add_parser(parser_t* parser);
};


// -----


class choice_of_parser_t : public parser_t {
    std::vector<parser_t*> parsers;

public:
    choice_of_parser_t(std::vector<parser_t*> _parsers);
    parser_state_t run(parser_state_t parser_state) const;
};


// -----


class many_parser_t : public parser_t {
    const parser_t* parser;

public:
    many_parser_t(const parser_t* parser);
    parser_state_t run(parser_state_t parser_state) const;
};


// -----

class many1_parser_t : public many_parser_t {
public:
    many1_parser_t(const parser_t* parser);
    parser_state_t run(parser_state_t parser_state) const;
};


// -----


class between_parser_t : public parser_t {
    parser_t *left_parser, *right_parser, *content_parser;

public:
    between_parser_t();
    between_parser_t(parser_t* _left_parser, parser_t* _right_parser);
    between_parser_t(parser_t* _left_parser, parser_t* _right_parser, parser_t* content_parser);

    parser_state_t run(parser_state_t parser_state) const;

    between_parser_t& set_left_parser(parser_t* _left_parser);
    between_parser_t& set_right_parser(parser_t* _right_parser);
    between_parser_t& set_content_parser(parser_t* _content_parser);
};


// -----


class separated_by_parser_t : public parser_t {
    parser_t *seaparator_parser, *value_parser;

public:
    separated_by_parser_t();
    separated_by_parser_t(parser_t* _seaparator_parser);
    separated_by_parser_t(parser_t* _seaparator_parser, parser_t* _value_parser);

    parser_state_t run(parser_state_t parser_state) const;

    separated_by_parser_t& set_seaparator_parser(parser_t* _seaparator_parser);
    separated_by_parser_t& set_value_parser(parser_t* _value_parser);
};


// -----


class exact_string_parser_t : public parser_t {
    std::string s;

public:
    exact_string_parser_t(std::string _s);
    parser_state_t run(parser_state_t parser_state) const;
};


// -----


class char_parser_t : public parser_t {
    std::regex rexp;

public:
    char_parser_t(std::regex rexp);
    parser_state_t run(parser_state_t parser_state) const;
};

class letter_parser_t : public char_parser_t {
public:
    letter_parser_t();
};

class digit_parser_t : public char_parser_t {
public:
    digit_parser_t();
};

class whitespace_parser_t : public char_parser_t {
public:
    whitespace_parser_t();
};


// -----


class chars_parser_t : public parser_t {
    char_parser_t char_parser;

public:
    chars_parser_t(std::regex _rexp);
    parser_state_t run(parser_state_t parser_state) const;
};

class letters_parser_t : public chars_parser_t {
public:
    letters_parser_t();
};

class digits_parser_t : public chars_parser_t {
public:
    digits_parser_t();
};

class whitespaces_parser_t : public chars_parser_t {
public:
    whitespaces_parser_t();
};


// -----


class maybe_chars_parser_t : public parser_t {
    char_parser_t char_parser;

public:
    maybe_chars_parser_t(std::regex _rexp);
    parser_state_t run(parser_state_t parser_state) const;
};

class maybe_letters_parser_t : public maybe_chars_parser_t {
public:
    maybe_letters_parser_t();
};

class maybe_digits_parser_t : public maybe_chars_parser_t {
public:
    maybe_digits_parser_t();
};

class maybe_whitespaces_parser_t : public maybe_chars_parser_t {
public:
    maybe_whitespaces_parser_t();
};


// -----
} // namespace wi
#endif // _WI_PARSER_HPP_
