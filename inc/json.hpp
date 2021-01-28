#ifndef JSON_HPP
#define JSON_HPP

#include <variant>
#include <string>
#include <vector>
#include <memory>
#include <ostream>
#include <sstream>

namespace json {

class JsonObject
{

    friend std::ostream &operator<<(std::ostream &output, const JsonObject &JsonObject);
    friend std::stringstream &operator<<(std::stringstream &output, const JsonObject &JsonObject);
public:
    //CONSTRUCTORS
    JsonObject();
    JsonObject(std::string_view jsonData);
    JsonObject(std::string_view key, std::variant<std::monostate, std::string, double, int, bool> value);
    JsonObject(const JsonObject&) = delete;
    JsonObject(JsonObject&&) = default;
    //OPERATORS
    JsonObject& operator=(const JsonObject&) = delete;
    JsonObject& operator=(JsonObject&&) = default;
    //DESTRUCTOR
    ~JsonObject() = default;

    //API

    void addObject(JsonObject &&object);

    std::shared_ptr<JsonObject> getChildByName(std::string_view name) const;

    ///Returns const reference to std::shared_ptr<JsonObject> or throws std::runtime_error otherwise.
    const std::vector<std::shared_ptr<JsonObject>> &toArray() const;
    ///Returns const reference to std::string std::bad_variant_access otherwise.
    ///This function returns only JsonObject:Value as a string and should not be used for file or stream generation. Use operator<<() instead.
    const std::string &toString() const;
    ///Returns const double or throws std::bad_variant_access otherwise.
    double toDouble() const;
    ///Returns const int or throws std::bad_variant_access otherwise.
    int toInt() const;
    ///Returns const int or throws std::bad_variant_access otherwise.
    bool toBool() const;

    bool isObject() const;
    bool isArray() const;
    bool isString() const;
    bool isDouble() const;
    bool isInt() const;
    bool isBool() const;

protected:
    void _addChildsFromObject(std::string_view jsonData);
    void _addChildsFromArray(std::string_view jsonData);

    std::string _toStream() const;
    std::string _toStream_b(uint8_t level = 0) const;
    std::string _getKey(std::string_view jsonData);
    std::string _getValue(std::string_view jsonData);

private:
    std::string m_key;
    std::variant<std::monostate, std::string, double, int, bool> m_value;
    std::vector<std::shared_ptr<JsonObject>> m_childs;
    bool m_root;
    bool m_array;
    bool m_beautifyOutput;
    //SETTERS AND GETTERS
public:
    void setKey(std::string_view key) {m_key = key;}
    void setIsArray(bool value) {m_array = value;}
    void setValue(std::variant<std::monostate, std::string, double, int, bool> value) {m_value = value;}
    void setBeautify(bool value) {m_beautifyOutput = value;}
};

std::ostream &operator<<(std::ostream &output, const JsonObject &jsonObject);
std::stringstream &operator<<(std::stringstream &output, const JsonObject &jsonObject);

} // namespace json

#endif // JSON_HPP
