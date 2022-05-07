set_property(TARGET mylangc PROPERTY CXX_STANDARD 17)

target_sources(mylangc PRIVATE
        tokenizer.cpp
        parser.cpp
        interpreter.cpp
)