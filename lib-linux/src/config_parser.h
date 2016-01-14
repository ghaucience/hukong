#ifndef __CONFIG_PARSER__
#define __CONFIG_PARSER__

#include <map>
#include <fstream>
#include <sstream>
#include <assert.h>
#include <stdexcept>
#include "utility.h"

namespace lib_linux
{
    // simple config file
    // example: 
    // a=b
    // c=d
    class TxtConfigParser
    {
        public:
            typedef std::map<std::string, std::string> key_map;

        public:
            TxtConfigParser()
            {
            }

            std::string Get(std::string strKey)
            {
                if (m_keyMap.count(strKey) > 0)
                {
                    return m_keyMap[strKey];
                }
                else
                {
                    return std::string("");
                }
            }

            void Set(std::string strKey, std::string strValue)
            {
                m_keyMap[strKey] = strValue;
            }

            void Input(std::istream &in)
            {
                // init 
                m_keyMap.clear();

                std::string strLine;
                for (;std::getline(in, strLine);)
                {
                    strLine = Utility::Strip(strLine);
                    if (!strLine.empty())
                    {
                        char prefix = strLine.at(0);
                        switch (prefix)
                        {
                            case ';':
                            case '#':
                                break;
                            default:
                                {
                                    size_t pos = strLine.find_first_of('=');
                                    if (pos == std::string::npos)
                                    {
                                        throw std::runtime_error("simple config line not find =");
                                    }

                                    if (pos != 0)
                                    {
                                        std::string strKey(strLine, 0, pos);
                                        strKey = Utility::Strip(strKey);

                                        if (!strKey.empty())
                                        {
                                            if (pos+1 < strLine.length())
                                            {
                                                std::string strValue(strLine, pos+1, std::string::npos);
                                                m_keyMap[strKey] = Utility::Strip(strValue); 
                                            }
                                            else
                                            {
                                                m_keyMap[strKey] = "";
                                            }
                                            break;
                                        }
                                    }

                                    throw std::runtime_error("simple config line = position not correct");
                                }
                                break;
                        }
                    }
                }
            }

            // full format output
            void Output(std::ostream &out)
            {
                //can't call <<endl, because it can flush buffer to file.
                for (key_map::const_iterator it=m_keyMap.begin(); it!=m_keyMap.end(); it++)
                {
                    out<<it->first<<"="<<it->second<<"\n";
                }
                out<<"\n";
            }

        private:
            key_map m_keyMap;
    };


    std::ostream &operator<<(std::ostream &stream, TxtConfigParser &parser);
    std::istream &operator>>(std::istream &stream, TxtConfigParser &parser);

    // ini config file 
    // example:
    // [section]
    // a=b
    // c=d
    class IniConfigParser
    {
        public:
            typedef std::map<std::string, TxtConfigParser *>  key_map;

        public:
            IniConfigParser()
            {
            }

            ~IniConfigParser()
            {
                Clear();
            }

            void Clear()
            {
                for (key_map::const_iterator it=m_keyMap.begin(); it!=m_keyMap.end(); it++)
                {
                    delete it->second;
                }
                m_keyMap.clear();
            }

            void Input(std::istream &in)
            {
                // initiation
                Clear();

                std::string strLine;
                std::stringstream stream;
                std::string strKey;
                bool bStart = false;
                for (;std::getline(in, strLine);)
                {
                    strLine = Utility::Strip(strLine);
                    if (!strLine.empty())
                    {
                        char prefix = strLine.at(0);
                        switch (prefix)
                        {
                            case ';':
                            case '#':
                                break;
                            case '[':
                                if (strLine.at(strLine.length()-1) == ']')
                                {
                                    if (bStart)
                                    {
                                        // end of a section
                                        assert(!strKey.empty());
                                        TxtConfigParser *pImp = new TxtConfigParser;
                                        stream>>*pImp;
                                        stream.clear();
                                        m_keyMap[strKey] = pImp;
                                    }

                                    // extract section key
                                    strKey = strLine.substr(1, strLine.length()-2);
                                    bStart = true;
                                }
                                else
                                {
                                    throw std::runtime_error("[] not match");
                                }
                                break;
                            default:
                                if (bStart)
                                {
                                    // input to TxtConfigParser
                                    stream<<strLine<<std::endl;
                                }
                                break;
                        }
                    }
                }

                // last section
                assert(!strKey.empty());
                TxtConfigParser *pImp = new TxtConfigParser;
                stream>>*pImp;
                m_keyMap[strKey] = pImp;
            }

            void Output(std::ostream &out)
            {
                for (key_map::const_iterator it=m_keyMap.begin(); it!=m_keyMap.end(); it++)
                {
                    //can't call <<endl, because it can flush buffer to file.
                    out<<"["<<it->first<<"]"<<"\n";
                    it->second->Output(out);
                }
            }

            void Set(std::string strSection, std::string strKey, std::string strValue)
            {
                if (m_keyMap.count(strSection) > 0)
                {
                    m_keyMap[strSection]->Set(strKey, strValue);
                }
                else
                {
                    TxtConfigParser *pParser = new TxtConfigParser;
                    pParser->Set(strKey, strValue);
                    m_keyMap[strSection] = pParser;
                }
            }

            std::string Get(std::string strSection, std::string strKey)
            {
                if (m_keyMap.count(strSection) > 0)
                {
                    return m_keyMap[strSection]->Get(strKey);
                }
                else
                {
                    return std::string("");
                }
            }

        private:
            IniConfigParser(const IniConfigParser &);
            const IniConfigParser &operator=(const IniConfigParser &);

        private:
            key_map m_keyMap;
    };

    std::ostream &operator<<(std::ostream &stream, IniConfigParser &parser);
    std::istream &operator>>(std::istream &stream, IniConfigParser &parser);
}

#endif
