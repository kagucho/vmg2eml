#include <iostream>
#include <string>
#include <algorithm>
#include <cctype>

#include <boost/utility/string_view.hpp>
#include "libmbfl++.hpp"
namespace literals{
    BOOST_CONSTEXPR boost::string_view operator"" _sv (const char* s, std::size_t n)
    {
        return {s, n};
    }
}
mbfl::string convert_ja_mobile_phone_to_utf8(const std::string& s, mbfl_no_language from) {
    mbfl::memory_device dev(s.capacity(), s.size() / 60);
    dev.strcat(s.c_str(), s.size());//mbfl_no_language_japanese, mbfl_no_encoding_utf8
    mbfl::string s(std:move(dev), mbfl_no_language_japanese, from);
    mbfl::buffer_converter conv(from, mbfl_no_encoding_utf8);
    return conv.feed_result(s);
}
// template<std::size_t N>
// bool space_ignore_comp(boost::string_view s, const std::string& string_space_ignore)
// {
//     auto s_it = std::begin(s);
//     auto t_it = std::begin(string_space_ignore);
//     auto skip_space = [&string_space_ignore](std::string::iterator it){
//         while(std::end(string_space_ignore) != *it && ' ' == *it) ++it;
//         return it;
//     };
//     for(
//         t_it = skip_space(t_it);
//         std::end(s) != s_it && std::end(string_space_ignore) != t_it && *s_it == *t_it;
//         ++s_it, t_it = skip_space(t_it)
//     );
//     return std::end(s) == s_it;
// }
bool forward_comp(boost::string_view prefix, const std::string& arg){
    return (prefix.begin() == std::mismatch(prefix.begin(), prefix.end(), arg.begin()).first);
}
void vmg2eml(std::istream& vmg_stream, std::ostream& eml_stream, mbfl_no_language from) {
    using namespace literals;
    bool in_vbody = false;
    bool is_body = false;
    for(std::string buf; std::getline(vmg_stream, buf);){
        if(!in_vbody && "BEGIN:VBODY" == buf)){
            in_vbody = true;
            continue;
        }

        if("END:VBODY" ==, buf)){
            in_vbody = false;
            break;//read first detect vbody block only
        }
        else if(!is_body && (buf.empty() || '\r' == buf.back())){
            is_body = true;
            continue;
        }

        if(!is_body){
            if(buf.empty() || '\r' == buf.back()){
                is_body = true;
                continue;
            }
            else if(forward_comp("Content-Type", buf)){
                eml_stream << "Content-Type: text/plain; charset=UTF-8\r\n";
            }
            else if(forward_comp("Content-Transfer-Encoding", buf)){
                eml_stream << "Content-Transfer-Encoding: Base64\r\n";
            }
            else{
                auto first_not_alpha = std::find_if_not(buf.begin(), buf.end(), [](char c){ return 0 != std::isalpha(c); });
                if(buf.end() != first_not_alpha && ':' == *first_not_alpha && ' ' == first_not_alpha[1]){
                    
                }
            }
        }
    }
}
int main(int argc, char **argv)
{
    std::cout << "Hello World" << std::endl;
    return 0;
}
