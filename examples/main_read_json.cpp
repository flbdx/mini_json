#include <iostream>

#include <mini_json/mini_json.h>

static std::string document = R"MARKER(
{"menu": {
    "header": "SVG Viewer",
    "items": [
        {"id": "Open"},
        {"id": "OpenNew", "label": "Open New"},
        null,
        {"id": "ZoomIn", "label": "Zoom In"},
        {"id": "ZoomOut", "label": "Zoom Out"},
        {"id": "OriginalView", "label": "Original View"},
        null,
        {"id": "Quality"},
        {"id": "Pause"},
        {"id": "Mute"},
        null,
        {"id": "Find", "label": "Find..."},
        {"id": "FindAgain", "label": "Find Again"},
        {"id": "Copy"},
        {"id": "CopyAgain", "label": "Copy Again"},
        {"id": "CopySVG", "label": "Copy SVG"},
        {"id": "ViewSVG", "label": "View SVG"},
        {"id": "ViewSource", "label": "View Source"},
        {"id": "SaveAs", "label": "Save As"},
        null,
        {"id": "Help"},
        {"id": "About", "label": "About Adobe CVG Viewer..."}
    ]
}}
)MARKER";

int main() {
    using namespace MiniJSON;
    
    try {
        const Value &v = Parser().parse(document);
        
        
        if (!v.keyExists("menu")) {
            return 1;
        }
        const Value &menu = v["menu"];
        if (!(menu.keyExists("header") && menu.keyExists("items"))) {
            return 1;
        }
        
        std::cout << "header=" << menu["header"].get<Type::String>() << std::endl;
        
        const Value &items = menu["items"];
        for (const Value &item : items.get<Type::Array>()) {
            if (item.get_type() == Type::Null) {
                continue;
            }
            if (item.keyExists("label")) {
                std::cout << "- id=" << item["id"].get<Type::String>() << ", label=" << item["label"].get<Type::String>() << std::endl;
            }
            else {
                std::cout << "- id=" << item["id"].get<Type::String>() << std::endl;
            }
        }
    }
    catch (const std::exception &exc) {
        std::cerr << exc.what() << std::endl;
    }
}
