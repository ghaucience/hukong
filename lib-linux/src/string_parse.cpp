#include <sstream>
#include <iterator>
#include "string_parse.h"

namespace lib_linux
{
    StringParse::StringParse(const std::string &str)
    {
        std::stringstream stream(str);
        std::copy(std::istream_iterator<std::string>(stream),
                std::istream_iterator<std::string>(),
                back_inserter(m_strings));
    }

    StringParse::iterator StringParse::begin() const
    {
        return m_strings.begin();
    }

    StringParse::iterator StringParse::end() const
    {
        return m_strings.end();
    }
}
