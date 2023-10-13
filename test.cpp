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

#include <iostream>
#include <vector>
#include <regex>

#include "utilities.hpp"
#include "parser.hpp"

using namespace wi;

int main() {
    try {
        parser_state_t init_parser_state("[% (* 2 (- [+ 8 2] 4)) 5]");

        lazy_parser_t *p_lazy_function = new lazy_parser_t();

        parser_t *p_value = new choice_of_parser_t({
            new digits_parser_t(),
            p_lazy_function
        });

        parser_t *p_function = new between_parser_t(
            new sequence_of_parser_t({
                new choice_of_parser_t({
                    new exact_string_parser_t("("),
                    new exact_string_parser_t("["),
                }),
                new maybe_whitespaces_parser_t()
            }),
            new sequence_of_parser_t({
                new maybe_whitespaces_parser_t(),
                new choice_of_parser_t({
                    new exact_string_parser_t(")"),
                    new exact_string_parser_t("]"),
                })
            }),
            new sequence_of_parser_t({
                new choice_of_parser_t({
                    new exact_string_parser_t("+"),
                    new exact_string_parser_t("-"),
                    new exact_string_parser_t("*"),
                    new exact_string_parser_t("/"),
                    new exact_string_parser_t("%")
                }),
                new between_parser_t(
                    new whitespaces_parser_t(),
                    new whitespaces_parser_t(),
                    p_value
                ),
                p_value
            })
        );

        p_lazy_function->set_parser(p_function);
        parser_state_t ps = p_function->run(init_parser_state);

        std::function<int(std::any)> f = [&](std::any a) {
            if (a.type() == typeid(std::vector<std::any>)) {
                std::vector<std::any> v = std::any_cast< std::vector<std::any> >(a);
                char op = smart_string_any_cast(v[0])[0];
                int left = f(v[1]), right = f(v[2]);
                if (op == '+') return left + right;
                if (op == '-') return left - right;
                if (op == '*') return left * right;
                if (op == '/') return left / right;
                if (op == '%') return left % right;
                return 0;
            } else {
                return std::stoi(smart_string_any_cast(a));
            }
        };

        ps = ps.map_result(f);

        std::cout << ps.to_string() << std::endl;

    } catch (std::string s) {
        std::cout << s << std::endl;
    }
    return 0;
}
