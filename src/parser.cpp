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

#include "parser.hpp"

namespace wi {
// -----


parser_state_t::parser_state_t()
: target_string(),
  result(""),
  index(0),
  error()
{}

parser_state_t::parser_state_t(std::string _target_string)
: target_string(_target_string),
  result(""),
  index(0),
  error()

{}

parser_state_t& parser_state_t::set_target_string(std::string _target_string)
{
    target_string = _target_string;
    return *this;
}

parser_state_t& parser_state_t::set_result(std::any _result)
{
    result = _result;
    return *this;
}

parser_state_t& parser_state_t::set_index(std::size_t _index)
{
    index = _index;
    return *this;
}

parser_state_t& parser_state_t::set_error(std::string _error)
{
    error = _error;
    return *this;
}

parser_state_t& parser_state_t::unset_error()
{
    error.reset();
    return *this;
}

std::string parser_state_t::get_target_string() const
{
    return target_string;
}

std::any parser_state_t::get_result() const
{
    return result;
}

std::size_t parser_state_t::get_index() const
{
    return index;
}

std::optional<std::string> parser_state_t::get_error() const
{
    return error;
}


parser_state_t parser_state_t::map_result(std::function<std::any(std::any)> f) const
{
    if (this->error.has_value())
        return *this;
    parser_state_t parser_state = *this;
    return parser_state.set_result(f(parser_state.result));
}

parser_state_t parser_state_t::map_error(std::function<std::string(std::string)> f) const
{
    if (!(this->error.has_value()))
        return *this;
    parser_state_t parser_state = *this;
    return parser_state.set_error(f(parser_state.error.value()));
}

parser_state_t parser_state_t::map_nested_result(std::function<std::any(std::string)> f) const
{
    if (this->error.has_value())
        return *this;
    std::function<std::any(std::any)> g = [&](std::any x) {
        if (x.type() == typeid(std::vector<std::any>)) {
            std::vector<std::any> v = std::any_cast< std::vector<std::any> >(x);
            for (std::any& a : v)
                a = g(a);
            return std::any(v);
        } else { // string
            return f(std::any_cast<std::string>(x));
        }
    };
    return this->map_result(g);
}

parser_state_t parser_state_t::chain(std::function<parser_t*(std::any)> f) const
{
    if (this->error.has_value())
        return *this;
    parser_t* next_parser = f(this->result);
    return next_parser->run(*this);
}

parser_state_t parser_state_t::flatten_result() const
{
    if (this->error.has_value())
        return *this;
    parser_state_t parser_state = *this;
    return parser_state.set_result(flatten_vector(parser_state.result));
}


std::string parser_state_t::to_string() const
{
    std::stringstream ss;
    std::string aux;

    ss << "{ target_string: \"" << target_string << "\",\n";
    ss << "  index: " << std::to_string(index) << ",\n";
    try {
        aux = any_to_string<true>(result);
    } catch (std::string err) {
        aux = "ERROR - " + err;
    }
    ss << "  result: " << aux;
    
    if (error.has_value()) {
        ss << ",\n";
        ss << "  error: \"" << error.value() << "\" }";
    } else {
        ss << " }";
    }
    
    return ss.str();
}


// -----


parser_state_t parser_t::run([[maybe_unused]]parser_state_t parser_state) const
{
    throw "parser_t::run() should never be run on its own!";
}


// -----


do_nothing_parser_t::do_nothing_parser_t()
{}

parser_state_t do_nothing_parser_t::run(parser_state_t parser_state) const
{
    return parser_state;
}


// -----


lazy_parser_t::lazy_parser_t()
: parser(nullptr)
{}

lazy_parser_t::lazy_parser_t(parser_t *_parser)
: parser(_parser)
{}

parser_state_t lazy_parser_t::run(parser_state_t parser_state) const
{
    return parser->run(parser_state);
}

lazy_parser_t& lazy_parser_t::set_parser(parser_t *_parser)
{
    parser = _parser;
    return *this;
}


// -----


map_parser_t::map_parser_t()
: parser(new do_nothing_parser_t()),
  f([](std::any a) {return a;})
{}

map_parser_t::map_parser_t(parser_t *_parser, std::function<std::any(std::any)> _f)
: parser(_parser),
  f(_f)
{}

parser_state_t map_parser_t::run(parser_state_t parser_state) const
{
    if (parser_state.error.has_value())
        return parser_state;
    parser_state = parser->run(parser_state);
    return parser_state.map_result(f);
}

map_parser_t& map_parser_t::set_parser(parser_t *_parser)
{
    parser = _parser;
    return *this;
}

map_parser_t& map_parser_t::set_f(std::function<std::any(std::any)> _f)
{
    f = _f;
    return *this;
}


// -----


chain_parser_t::chain_parser_t()
: parser(new do_nothing_parser_t()),
  f([]([[maybe_unused]] std::any a) {return new do_nothing_parser_t();})
{}

chain_parser_t::chain_parser_t(parser_t *_parser, std::function<parser_t*(std::any)> _f)
: parser(_parser),
  f(_f)
{}

parser_state_t chain_parser_t::run(parser_state_t parser_state) const
{
    if (parser_state.error.has_value())
        return parser_state;
    parser_state = parser->run(parser_state);
    return parser_state.chain(f);
}

chain_parser_t& chain_parser_t::set_parser(parser_t *_parser)
{
    parser = _parser;
    return *this;
}

chain_parser_t& chain_parser_t::set_f(std::function<parser_t*(std::any)> _f)
{
    f = _f;
    return *this;
}


// -----


flatten_parser_t::flatten_parser_t()
: parser(nullptr)
{}

flatten_parser_t::flatten_parser_t(parser_t *_parser)
: parser(_parser)
{}

parser_state_t flatten_parser_t::run(parser_state_t parser_state) const
{
    if (parser_state.error.has_value())
        return parser_state;
    parser_state = parser->run(parser_state);
    return parser_state.flatten_result();
}

flatten_parser_t& flatten_parser_t::set_parser(parser_t *_parser)
{
    parser = _parser;
    return *this;
}


// -----


sequence_of_parser_t::sequence_of_parser_t()
: parsers()
{}

sequence_of_parser_t::sequence_of_parser_t(std::vector<parser_t*> _parsers)
: parsers(_parsers)
{}

parser_state_t sequence_of_parser_t::run(parser_state_t parser_state) const
{
    if (parser_state.error.has_value())
        return parser_state;

    std::vector<std::any> results;
    for (auto parser : this->parsers) {
        parser_state = parser->run(parser_state);
        results.emplace_back(parser_state.result);
    }

    if (parser_state.error.has_value())
        return parser_state;

    return parser_state.set_result(results);
}

void sequence_of_parser_t::add_parser(parser_t* parser)
{
    parsers.push_back(parser);
}


// -----


choice_of_parser_t::choice_of_parser_t(std::vector<parser_t*> _parsers)
: parsers(_parsers)
{}

parser_state_t choice_of_parser_t::run(parser_state_t parser_state) const
{
    if (parser_state.error.has_value())
        return parser_state;

    for (auto parser : this->parsers) {
        parser_state_t next_state = parser->run(parser_state);
        if (!next_state.error.has_value())
            return next_state;
    }

    return parser_state
        .set_result("")
        .set_error("choice_of_parser_t::run(): Unable to match with any parser the string \"" + string_at_most(parser_state.target_string, 10, parser_state.index) + "\"");
}


// -----


many_parser_t::many_parser_t(const parser_t* _parser)
: parser(_parser)
{}

parser_state_t many_parser_t::run(parser_state_t parser_state) const
{
    if (parser_state.error.has_value())
        return parser_state;

    parser_state_t next_state;
    std::vector<std::any> results;
    do {
        next_state = parser->run(parser_state);
        if (next_state.error.has_value())
            break;
        parser_state = next_state;
        results.emplace_back(next_state.result);
    } while (1);

    return parser_state.set_result(results);
}

many1_parser_t::many1_parser_t(const parser_t* _parser)
: many_parser_t(_parser)
{}

parser_state_t many1_parser_t::run(parser_state_t parser_state) const
{
    parser_state = many_parser_t::run(parser_state);
    if (!parser_state.error.has_value()) {
        std::vector<std::any> results = std::any_cast< std::vector<std::any> >(parser_state.result);
        if (results.size() == 0) {
            return parser_state_t()
                .set_result("")
                .set_error("many1_parser_t::run(): Unable to match any inputs using given parser for the string \"" + string_at_most(parser_state.target_string, 10, parser_state.index) + "\"");
        }
    }
    return parser_state;
}


// -----


between_parser_t::between_parser_t()
: left_parser(nullptr),
  right_parser(nullptr),
  content_parser(nullptr)
{}

between_parser_t::between_parser_t(parser_t* _left_parser, parser_t* _right_parser)
: left_parser(_left_parser),
  right_parser(_right_parser),
  content_parser(nullptr)
{}

between_parser_t::between_parser_t(parser_t* _left_parser, parser_t* _right_parser, parser_t* content_parser)
: left_parser(_left_parser),
  right_parser(_right_parser),
  content_parser(content_parser)
{}

parser_state_t between_parser_t::run(parser_state_t parser_state) const
{
    sequence_of_parser_t sequence_of_parser;
    sequence_of_parser.add_parser(left_parser);
    sequence_of_parser.add_parser(content_parser);
    sequence_of_parser.add_parser(right_parser);
    parser_state = sequence_of_parser.run(parser_state);
    if (!parser_state.error.has_value()) {
        std::vector<std::any> v = std::any_cast< std::vector<std::any> >(parser_state.result);
        if (v.size() == 0) {
            // TODO
            throw -1;
        }
        parser_state.set_result(v[1]);
    }
    return parser_state;
}


// -----


separated_by_parser_t::separated_by_parser_t()
: seaparator_parser(nullptr),
  value_parser(nullptr)
{}

separated_by_parser_t::separated_by_parser_t(parser_t* _seaparator_parser)
: seaparator_parser(_seaparator_parser),
  value_parser(nullptr)
{}

separated_by_parser_t::separated_by_parser_t(parser_t* _seaparator_parser, parser_t* _value_parser)
: seaparator_parser(_seaparator_parser),
  value_parser(_value_parser)
{}

parser_state_t separated_by_parser_t::run(parser_state_t parser_state) const
{
    if (parser_state.error.has_value())
        return parser_state;
    if (seaparator_parser == nullptr) {
        return parser_state_t()
            .set_result("")
            .set_error("separated_by_parser_t::run(): seaparator_parser is NULL");
    }
    if (value_parser == nullptr) {
        return parser_state_t()
            .set_result("")
            .set_error("separated_by_parser_t::run(): value_parser is NULL");
    }

    parser_state_t next_state = parser_state;
    std::vector<std::any> results;
    do {
        parser_state_t wanted_state = value_parser->run(next_state);
        if (wanted_state.error.has_value())
            break;
        results.emplace_back(wanted_state.result);
        next_state = wanted_state;
        parser_state_t separator_state = seaparator_parser->run(next_state);
        if (separator_state.error.has_value())
            break;
        next_state = separator_state;
    } while (1);

    return next_state.set_result(results);
}

separated_by_parser_t& separated_by_parser_t::set_seaparator_parser(parser_t* _seaparator_parser)
{
    seaparator_parser = _seaparator_parser;
    return *this;
}

separated_by_parser_t& separated_by_parser_t::set_value_parser(parser_t* _value_parser)
{
    value_parser = _value_parser;
    return *this;
}


// -----


string_parser_t::string_parser_t()
: s()
{}

string_parser_t::string_parser_t(std::string _s)
: s(_s)
{}

parser_state_t string_parser_t::run(parser_state_t parser_state) const
{
    if (parser_state.error.has_value())
        return parser_state;

    if (parser_state.target_string.size() == 0) {
        return parser_state
            .set_result("")
            .set_error("string_parser_t::run(): Unexpected end of string");
    }

    if (string_starts_with(parser_state.target_string, this->s, parser_state.index)) {
        return parser_state_t()
            .set_target_string(parser_state.target_string)
            .set_result(this->s)
            .set_index(parser_state.index + this->s.size());
    }

    return parser_state
        .set_result("")
        .set_error("string_parser_t::run(): Couldn't match \"" + this->s + "\" in \"" + string_at_most<true>(parser_state.target_string, 10, parser_state.index) + "\"");
}

string_parser_t& string_parser_t::set_string(std::string _s)
{
    s = _s;
    return *this;
}


// -----


choice_of_string_parser_t::choice_of_string_parser_t()
: words()
{}

choice_of_string_parser_t::choice_of_string_parser_t(std::vector<std::string> _words)
: words(_words)
{}

parser_state_t choice_of_string_parser_t::run(parser_state_t parser_state) const
{
    if (parser_state.error.has_value())
        return parser_state;

    string_parser_t parser;

    for (const std::string& word : this->words) {
        parser_state_t next_state = parser.set_string(word).run(parser_state);
        if (!next_state.error.has_value())
            return next_state;
    }

    return parser_state
        .set_result("")
        .set_error("choice_of_string_parser_t::run(): Unable to match with any parser the string \"" + string_at_most(parser_state.target_string, 10, parser_state.index) + "\"");
}


// -----


char_parser_t::char_parser_t(std::regex _rexp)
: rexp(_rexp)
{}

parser_state_t char_parser_t::run(parser_state_t parser_state) const
{
    if (parser_state.error.has_value())
        return parser_state;

    if (parser_state.target_string.size() == 0) {
        return parser_state
            .set_result("")
            .set_error("char_parser_t::run(): Unexpected end of string");
    }

    if (parser_state.index < parser_state.target_string.size()) {
        std::string first_char = std::string(1, parser_state.target_string[parser_state.index]);
        if (std::regex_search(first_char, rexp)) {
            return parser_state_t()
                .set_target_string(parser_state.target_string)
                .set_result(first_char)
                .set_index(parser_state.index + 1);
        }
    }

    return parser_state
        .set_result("")
        .set_error("char_parser_t::run(): Couldn't match any character TODO in \"" + string_at_most<true>(parser_state.target_string, 10, parser_state.index) + "\"");
}

letter_parser_t::letter_parser_t()
: char_parser_t(std::regex(R"([A-Za-z])"))
{}

digit_parser_t::digit_parser_t()
: char_parser_t(std::regex(R"([0-9])"))
{}

whitespace_parser_t::whitespace_parser_t()
: char_parser_t(std::regex(R"(\s)"))
{}


// -----


chars_parser_t::chars_parser_t(std::regex _rexp)
: char_parser(_rexp)
{}

parser_state_t chars_parser_t::run(parser_state_t parser_state) const
{
    parser_state = many1_parser_t(&char_parser).run(parser_state);
    if (parser_state.error.has_value())
        return parser_state;
    
    return parser_state.map_result(
        [](std::any x) {
            std::vector<std::any> v = std::any_cast< std::vector<std::any> >(x);
            std::string s;
            for (std::any& a : v)
                s += std::any_cast<std::string>(a);
            return s;
        }
    );
}

letters_parser_t::letters_parser_t()
: chars_parser_t(std::regex(R"([A-Za-z])"))
{}

digits_parser_t::digits_parser_t()
: chars_parser_t(std::regex(R"([0-9])"))
{}

whitespaces_parser_t::whitespaces_parser_t()
: chars_parser_t(std::regex(R"(\s)"))
{}


// -----


maybe_chars_parser_t::maybe_chars_parser_t(std::regex _rexp)
: char_parser(_rexp)
{}

parser_state_t maybe_chars_parser_t::run(parser_state_t parser_state) const
{
    parser_state = many_parser_t(&char_parser).run(parser_state);    
    return parser_state.map_result(
        [](std::any x) {
            std::vector<std::any> v = std::any_cast< std::vector<std::any> >(x);
            std::string s;
            for (std::any& a : v)
                s += std::any_cast<std::string>(a);
            return s;
        }
    );
}

maybe_letters_parser_t::maybe_letters_parser_t()
: maybe_chars_parser_t(std::regex(R"([A-Za-z])"))
{}

maybe_digits_parser_t::maybe_digits_parser_t()
: maybe_chars_parser_t(std::regex(R"([0-9])"))
{}

maybe_whitespaces_parser_t::maybe_whitespaces_parser_t()
: maybe_chars_parser_t(std::regex(R"(\s)"))
{}


// -----
} // namespace wi

