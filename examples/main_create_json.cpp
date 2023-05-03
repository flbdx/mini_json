#include <iostream>

#include <mini_json/mini_json.h>

static void ex1() {
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
    
    std::cout << Generator::to_string_pretty(root) << std::endl;
}

static void ex2() {
    using namespace MiniJSON;
    
    Value root = Value::new_object();
    root["null_value"] = Value();
    root["bool_true"] = true;
    root["bool_false"] = false;
    root["int32_value"] = int32_t(-42);
    root["uint64_value"] = uint64_t(1)<<48;
    root["float_value"] = float(1./7.);
    root["double_value"] = 1./7.;
    root["string_value 𝅘𝅥𝅮"] = "𝄆𝄠𝄢";
    root["empty_object"] = Value::new_object();
    root["empty_array"] = Value::new_array();
    
    std::cout << Generator::to_string_pretty(root) << std::endl;
}

int main() {
    ex1();
    ex2();
}
