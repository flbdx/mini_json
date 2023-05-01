#include <cstdio>
#include <cstdlib>
#include <cmath>

#include <fstream>

#include <mini_json/mini_json.h>

int main(int argc, char **argv)
{
    using namespace MiniJSON;

    if (argc > 1) {
        std::ifstream ifs(argv[1], std::ios_base::in | std::ios_base::binary);
        if (!ifs.good()) {
            fprintf(stderr, "Can't open file %s", argv[1]);
            return 200;
        }
        std::string data = {std::istreambuf_iterator<char>(ifs), std::istreambuf_iterator<char>() };
        Parser parser;
        try {
            Value v = parser.parse(data);
            puts(v.to_string().c_str());
            return 0;
        } catch (std::exception &e) {
            fprintf(stderr, "%s\n", e.what());
            return 1;
        }
    }

    return 0;
}
