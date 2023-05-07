#include <iostream>

#include <mini_json/mini_json.h>

// {
//     "glossary": {
//         "title": "example glossary",
//         "GlossDiv": {
//             "title": "S",
//             "GlossList": {
//                 "GlossEntry": {
//                     "ID": "SGML",
//                     "SortAs": "SGML",
//                     "GlossTerm": "Standard Generalized Markup Language",
//                     "Acronym": "SGML",
//                     "Abbrev": "ISO 8879:1986",
//                     "GlossDef": {
//                         "para": "A meta-markup language, used to create markup languages such as DocBook.",
//                         "GlossSeeAlso": ["GML", "XML"]
//                     },
//                     "GlossSee": "markup"
//                 }
//             }
//         }
//     }
// }
static MiniJSON::Value ex1() {   
    using namespace MiniJSON;
      
    Value root = Value::new_object();
    
    Value &glossary = root["glossary"] = Value::new_object();
    glossary["title"] = "example glossary";
    
    Value &glossDiv = glossary["GlossDiv"] = Value::new_object();
    glossDiv["title"] = "S";
    glossDiv["GlossList"] = Value::new_object();
    
    Value &glossEntry = glossDiv["GlossList"]["GlossEntry"] = Value::new_object();
    glossEntry["ID"] = "SGML";
    glossEntry["SortAs"] = "SGML";
    glossEntry["GlossTerm"] = "Standard Generalized Markup Language";
    glossEntry["Acronym"] = "SGML";
    glossEntry["Abbrev"] = "ISO 8879:1986";
    
    Value &glossDef = glossEntry["GlossDef"] = Value::new_object();
    glossDef["para"] = "A meta-markup language, used to create markup languages such as DocBook.";
    
    Value & glossSeeAlso = glossDef["GlossSeeAlso"] = Value::new_array();
    glossSeeAlso.get<Type::Array>().emplace_back("GML");
    glossSeeAlso.get<Type::Array>().emplace_back("XML");
    
    glossEntry["GlossSee"] = "markup";
    
    return root;
}

static MiniJSON::Value ex2()
{
    using namespace MiniJSON;
    
    Value root = {
        {"glossary", {
            {"title", "example glossary"},
            {"GlossDiv", {
                {"title", "S"},
                {"GlossList", {
                    {"GlossEntry", {
                        {"ID", "SGML"},
                        {"SortAs", "SGML"},
                        {"GlossTerm", "Standard Generalized Markup Language"},
                        {"Acronym", "SGML"},
                        {"Abbrev", "ISO 8879:1986"},
                        {"GlossDef", {
                            {"para", "A meta-markup language, used to create markup languages such as DocBook."},
                            {"GlossSeeAlso", ArrayValues{"GML", "XML"}}
                        }},
                        {"GlossSee", "markup"}
                    }}
                }}
            }}
        }}
    };

    return root;
}

static MiniJSON::Value ex3() {
    using namespace MiniJSON;
    
    Value root = Value::new_object();
    root["null_value"] = Value();
    root["bool_true"] = true;
    root["bool_false"] = false;
    root["int64_value"] = int64_t(-42);
    root["uint64_value"] = uint64_t(1)<<48;
    root["double_value"] = 1./7.;
    root["string_value 𝅘𝅥𝅮"] = "𝄆𝄠𝄢";
    root["empty_object"] = Value::new_object();
    root["empty_array"] = Value::new_array();
    
    return root;
}

static MiniJSON::Value ex4() {
    using namespace MiniJSON;
    
    Value root = {
        {"null_value", nullptr},
        {"bool_true", true},
        {"bool_false", false},
        {"int64_value", int64_t(-42)},
        {"uint64_value", uint64_t(1)<<48},
        {"double_value", 1./7.},
        {"string_value 𝅘𝅥𝅮", "𝄆𝄠𝄢"},
        {"empty_object", ObjectValues{}},
        {"empty_array", ArrayValues{}}
    };

    return root;
}

int main() {
    MiniJSON::Value v1 = ex1();
    MiniJSON::Value v2 = ex2();
    MiniJSON::Value v3 = ex3();
    MiniJSON::Value v4 = ex4();
    std::cout << v1.to_string(2) << std::endl;
    std::cout << v2.to_string(2) << std::endl;
    std::cout << "v1 == v2 ? " << bool(v1 == v2) << std::endl;
    std::cout << v3.to_string(4) << std::endl;
    std::cout << v4.to_string(4) << std::endl;
    std::cout << "v3 == v4 ? " << bool(v3 == v4) << std::endl;
}
