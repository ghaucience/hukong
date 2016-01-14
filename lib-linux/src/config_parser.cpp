#include "config_parser.h"

namespace lib_linux
{
    std::ostream &operator<<(std::ostream &stream, TxtConfigParser &parser)
    {
        parser.Output(stream);
        return stream;
    }

    std::istream &operator>>(std::istream &stream, TxtConfigParser &parser)
    {
        parser.Input(stream);
        return stream;
    }

    std::ostream &operator<<(std::ostream &stream, IniConfigParser &parser)
    {
        parser.Output(stream);
        return stream;
    }

    std::istream &operator>>(std::istream &stream, IniConfigParser &parser)
    {
        parser.Input(stream);
        return stream;
    }
}
