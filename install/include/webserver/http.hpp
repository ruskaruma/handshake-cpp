#pragma once
#include <string>
#include <map>
#include <cctype>
#include <sstream>
#include <optional>

namespace webserver
{
    inline std::string _lc(std::string s)
    {
        for (char& c : s)
            c = static_cast<char>( std::tolower(static_cast<unsigned char>(c)) );
        return s;
    }

    inline std::string _trim(const std::string& s)
    {
        size_t a = 0, b = s.size();
        while (a < b && std::isspace(static_cast<unsigned char>(s[a]))) ++a;
        while (b > a && std::isspace(static_cast<unsigned char>(s[b-1]))) --b;
        return s.substr(a, b - a);
    }

    struct HttpRequest
    {
        std::string method;
        std::string path;
        std::string version;
        std::map<std::string,std::string> headers; // lower-case keys
    };

    inline std::optional<HttpRequest> parse_http_request(const std::string& raw)
    {
        auto pos = raw.find("\r\n\r\n");
        if (pos == std::string::npos) return std::nullopt;

        std::istringstream ss(raw.substr(0, pos));
        std::string line;
        HttpRequest r;

        if (!std::getline(ss, line)) return std::nullopt;
        if (!line.empty() && line.back() == '\r') line.pop_back();
        {
            std::istringstream sl(line);
            if (!(sl >> r.method >> r.path >> r.version)) return std::nullopt;
        }

        while (std::getline(ss, line))
        {
            if (!line.empty() && line.back() == '\r') line.pop_back();
            if (line.empty()) break;
            auto c = line.find(':'); if (c == std::string::npos) continue;
            auto k = _lc(_trim(line.substr(0, c)));
            auto v = _trim(line.substr(c + 1));
            r.headers[k] = v;
        }
        return r;
    }

    inline bool _token_has(const std::string& v, const std::string& tok_lc)
    {
        size_t s = 0; std::string x = _lc(v);
        while (s < x.size())
        {
            size_t e = x.find(',', s); if (e == std::string::npos) e = x.size();
            if (_trim(x.substr(s, e - s)) == tok_lc) return true;
            s = (e == x.size() ? e : e + 1);
        }
        return false;
    }
}
