# WiParser

**WiParser** is a modern parser combinator written from scratch in plain C++ 17. It is intended to provide a simple API that can be easily extended.

This repository was contributed by [**Valentin-Ioan Vintilă**](https://www.v-vintila.com) and it is provided under the [MIT License](./LICENSE). For more info about the author, check out his [personal website](https://www.v-vintila.com) or his [other projects](https://www.github.com/w1bb).

## Usage

The repository contains an example [**Makefile**](./Makefile) that compiles the [test.cpp](./test.cpp) file. You can thus extract the required (generic) compilation process.

Simply run `make` and a new file, `./test`, should be created.

### Examples

A few examples should be provided in the [test.cpp](./test.cpp) file. These should give the programmer a good understanding of the basics, without reading any documentation.

## The API

The rich API provided by the current version of the project, **1.0.2b**, will be explained in the following paragraphs

### parser_state_t

The `parser_state_t` structure contains the information that will be passed from one (sub)parser to another. Think of it as a log of wood that will be processed into a dinner table by various machines (in our case, parsers) which work in an ordered manner.

The structure contains the following information:

- `target_string (std::string)` - The string that has to be parsed.
- `result (std::any)` - In our analogy, the _dinner table_ (or any product in between the log of wood and the dinner table). Currently, the `result` can only be a `std::string` or a `std::vector<std::any>`, but this will be addressed in the future so that more types will be included.
- `index (std::size_t)` - The index of the character that will be processed next, `target_string[index]`.
- `error (std::optional<std::string>)` - This will hold no value if no error occured and it will hold a detailed string in case something went wrong.

There are setters and getters for each of the parameters explained above.

### parser_t

TODO

### do_nothing_parser_t

TODO

### lazy_parser_t

TODO

### sequence_of_parser_t

TODO

### choice_of_parser_t

TODO

### many_parser_t

TODO

### many1_parser_t

TODO

### between_parser_t

TODO

### separated_by_parser_t

TODO

### string_parser_t

TODO

### choice_of_string_parser_t

TODO

### char_parser_t

TODO

### letter_parser_t

TODO

### digit_parser_t

TODO

### whitespace_parser_t

TODO

### chars_parser_t

TODO

### letters_parser_t

TODO

### digits_parser_t

TODO

### whitespaces_parser_t

TODO

### maybe_chars_parser_t

TODO

### maybe_letters_parser_t

TODO

### maybe_digits_parser_t

TODO

### maybe_whitespaces_parser_t

TODO
