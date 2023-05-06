#include <iostream>
#include <random>
#include <cmath>
#include <list>
#include <functional>

#include <mini_json/mini_json.h>
#include <mini_json/utf_conv.h>

/*
 * This is a tester program for the generator and parser.
 * It generates random JSON documents, parses them and compare the trees.
 * The generated JSON document don't use the Float data type, as the parser
 * always parse floating point values as Double.
 */


/**
 * @brief The Random JSON Generator
 * 
 */
class RandomJsonGenerator {
public:
    RandomJsonGenerator () : rng() {
        std::random_device rd;
        std::seed_seq sseq {rd(), rd(), rd(), rd(), rd(), rd()};
        rng.seed(sseq);
    }
    
    /**
     * @brief Generate a random JSON Value
     * 
     * The generated document can be a single value or a complexe type.
     * It will contains up to max_n_nodes values.
     * 
     * @param max_n_nodes maximum number of values in the document
     * @return random JSON
     */
    MiniJSON::Value gen_json(size_t max_n_nodes = 100) {
        // list of arrays or objects to fill
        std::list<std::reference_wrapper<MiniJSON::Value>> to_fill;
        
        size_t n_nodes = 1;
        
        auto root = gen_something(true);
        if (root.get_type() & MiniJSON::MASK_TYPE_IS_CONTAINER) {
            to_fill.emplace_back(root);
        }
        
        std::normal_distribution<> distrib {5, 2}; // to generate the number of nodes in an array or an object
        while (!to_fill.empty() && n_nodes < max_n_nodes) {
            auto &n = to_fill.front().get();
            to_fill.pop_front();
            
            long int to_gen = (long int) distrib(rng);
            if (to_gen < 0) {
                to_gen = 0;
            }
            if (max_n_nodes - n_nodes < (size_t) to_gen) {
                to_gen = max_n_nodes - n_nodes;
            }
            
            if (n.get_type() == MiniJSON::Type::Array) {
                auto & array_content = n.get<MiniJSON::Array>();
                while (to_gen) {
                    array_content.push_back(gen_something(false));
                    auto &nnode = array_content.back();
                    if (nnode.get_type() & MiniJSON::MASK_TYPE_IS_CONTAINER) {
                        to_fill.emplace_back(nnode);
                    }
                    --to_gen;
                    ++n_nodes;
                }
            }
            else {
                while (to_gen) {
                    std::string key = std::to_string(to_gen);
                    n[key] = gen_something(false);
                    auto &nnode = n[key];
                    if (nnode.get_type() & MiniJSON::MASK_TYPE_IS_CONTAINER) {
                        to_fill.emplace_back(nnode);
                    }
                    --to_gen;
                    ++n_nodes;
                }
            }

        }
        
        return root;
    }
    
    
private:
    /**
     * @brief Generate a random boolean value
     * 
     * @return Value
     */
    MiniJSON::Value gen_boolean() {
        std::uniform_int_distribution<uint8_t> distrib(0);
        return MiniJSON::Value(bool(distrib(rng) & 1));
    }
    
    /**
     * @brief Generate a null value
     * 
     * @return Value
     */
    MiniJSON::Value gen_null() {
        return {};
    }
    
    /**
     * @brief Generate an unsigned 32 bits integer value 
     * 
     * @return Value
     */
    MiniJSON::Value gen_uint32() {
        std::uniform_int_distribution<uint32_t> distrib(0);
        return MiniJSON::Value(distrib(rng));
    }
    /**
     * @brief Generate a signed 32 bits integer value 
     * 
     * @return Value
     */
    MiniJSON::Value gen_int32() {
        std::uniform_int_distribution<int32_t> distrib(std::numeric_limits<int32_t>::min());
        return MiniJSON::Value(distrib(rng));
    }
    /**
     * @brief Generate an unsigned 64 bits integer value 
     * 
     * @return Value
     */
    MiniJSON::Value gen_uint64() {
        std::uniform_int_distribution<uint64_t> distrib(0);
        return MiniJSON::Value(distrib(rng));
    }
    /**
     * @brief Generate a signed 64 bits integer value 
     * 
     * @return Value
     */
    MiniJSON::Value gen_int64() {
        std::uniform_int_distribution<int64_t> distrib(std::numeric_limits<int64_t>::min());
        return MiniJSON::Value(distrib(rng));
    }
    
    /**
     * @brief Generate a random double value
     * 
     * @return Value
     */
    MiniJSON::Value gen_double() {
        union {
            uint8_t u8[8];
            double d;
        } u;
        std::uniform_int_distribution<uint8_t> distrib(0);
        while (true) {
            // random 64 bits
            for (auto & b : u.u8) {
                b = distrib(rng);
            }
            if (std::isnormal(u.d) || u.d == 0.) { // avoid infinities and NaN
                return MiniJSON::Value(u.d);
            }
        }
    }
    
    /**
     * @brief Generate a random string value
     * 
     * The generated string is a valid UTF-8 sequence, but the chararacters may not
     * be defined.
     * 
     * @return Value
     */
    MiniJSON::Value gen_string() {
        std::string ret;
        size_t n = std::uniform_int_distribution<uint8_t>(0, 25)(rng);
        std::uniform_int_distribution<uint32_t> distrib(1, 0x1FF);
        uint32_t cp;
        for (size_t i = 0; i < n; ) {
            cp = distrib(rng);
            if (cp > 0x1D0) {
                // lift up part of the range to generate high values
                cp += 0x10000;
            }
            if (cp >= 0xD800 && cp <= 0xDFFF) {
                continue;
            }
            UTF::encode_utf8(&cp, 1, std::back_inserter(ret), NULL, NULL);
            ++i;
        }
        
        return MiniJSON::Value(ret);
    }
    
    /**
     * @brief Returns an empty array Value
     * 
     * @return Value
     */
    MiniJSON::Value gen_empty_array() {
        return MiniJSON::Value::new_array();
    }
    
    /**
     * @brief Returns an empty object Value
     * 
     * @return Value
     */
    MiniJSON::Value gen_empty_object() {
        return MiniJSON::Value::new_object();
    }
    
    /**
     * @brief Generate a random value
     * 
     * The returned value can be a simple value or en empty array or object
     * 
     * If top_level is true, then the probability to generate an array or an object is higher.
     * 
     * @param top_level if true, higher probability to generate an object or an array
     * @return Value
     */
    MiniJSON::Value gen_something(bool top_level) {
        std::uniform_real_distribution fdistrib(0.);
        std::uniform_int_distribution idistrib(0, 7);
        double r = fdistrib(rng);
        double base = top_level ? 0.4 : 0.20;
        if (r < base) {
            return gen_empty_array();
        }
        else if (r < 2*base) {
            return gen_empty_object();
        }
        else {
            int i = idistrib(rng);
            switch (i) {
                case 0:
                    return gen_null();
                case 1:
                    return gen_boolean();
                case 2:
                    return gen_uint32();
                case 3:
                    return gen_int32();
                case 4:
                    return gen_uint64();
                case 5:
                    return gen_int64();
                case 6:
                    return gen_double();
                default:
                    return gen_string();
            }
        }
    }
    
    std::mt19937 rng;
};

int main() {
    using namespace MiniJSON;
    
    RandomJsonGenerator rng;
    Parser parser;
    while (true) {
        Value json = rng.gen_json(500);
        std::string doc = Generator::to_string(json);
        Value o = parser.parse(doc);
        if (json != o) {
            puts(Generator::to_string(json).c_str());
            puts(Generator::to_string(o).c_str());
            break;
        }
        doc = Generator::to_string_pretty(json);
        o = parser.parse(doc);
        if (o != json) { // symmetric...
            puts(Generator::to_string(json).c_str());
            puts(Generator::to_string(o).c_str());
            break;
        }
    }

}
