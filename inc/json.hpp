#ifndef JSON_HPP
#define JSON_HPP

#include <variant>
#include <string>
#include <vector>
#include <memory>
#include <ostream>
#include <sstream>

namespace tristan::json {

class JsonObject
{

    friend std::ostream &operator<<(std::ostream &output, const JsonObject &JsonObject);
    friend std::stringstream &operator<<(std::stringstream &output, const JsonObject &JsonObject);
public:
    //CONSTRUCTORS
    JsonObject();
    explicit JsonObject(std::string_view jsonData);
    JsonObject(std::string_view key, std::monostate);
    JsonObject(std::string_view key, std::string_view value);
    JsonObject(std::string_view key, double value);
    JsonObject(std::string_view key, int value);
    JsonObject(std::string_view key, bool value);
    JsonObject(const JsonObject&) = delete;
    JsonObject(JsonObject&&) = default;
    //OPERATORS
    JsonObject& operator=(const JsonObject&) = delete;
    JsonObject& operator=(JsonObject&&) = default;
    //DESTRUCTOR
    ~JsonObject() = default;

    //MODIFY API

    void addObject(JsonObject &&object);
    void setKey(std::string_view key) {m_key = key;}
    void setIsArray(bool value = true) {m_array = value;}
    void setValue(std::string_view value);
    void setValue(double value);
    void setValue(int value);
    void setValue(bool value);
    void setBeautify(bool value = true) {m_beautifyOutput = value;}
    //READ API


    [[nodiscard]] auto getChildByName(std::string_view name) const -> std::shared_ptr<JsonObject>;

    ///Returns const reference to std::shared_ptr<JsonObject> or throws std::runtime_error otherwise.
    [[nodiscard]] auto toArray() const -> const std::vector<std::shared_ptr<JsonObject>>&;
    ///Returns const std::string, which is empty if value in json was [null] and std::bad_variant_access if value is not string.
    ///This function returns only JsonObject:Value as a string and should not be used for file or stream generation. Use operator<<() instead.
    [[nodiscard]] auto toString() const -> std::string;
    ///Returns const double or throws std::bad_variant_access otherwise.
    [[nodiscard]] auto toDouble() const -> double;
    ///Returns const int or throws std::bad_variant_access otherwise.
    [[nodiscard]] auto toInt() const -> int;
    ///Returns const int or throws std::bad_variant_access otherwise.
    [[nodiscard]] auto toBool() const -> bool;
    
    [[nodiscard]] auto isObject() const -> bool;
    [[nodiscard]] auto isArray() const -> bool;
    [[nodiscard]] auto isString() const -> bool;
    [[nodiscard]] auto isDouble() const -> bool;
    [[nodiscard]] auto isInt() const -> bool;
    [[nodiscard]] auto isBool() const -> bool;
    [[nodiscard]] auto isNull() const -> bool;

protected:
    void _addChildsFromObject(std::string_view jsonData);
    void _addChildsFromArray(std::string_view jsonData);

    [[nodiscard]] auto _toStream() const -> std::string;
    [[nodiscard]] auto _toStream_b(uint8_t level = 0) const -> std::string;
    [[nodiscard]] auto _getKey(std::string_view jsonData) -> std::string;
    [[nodiscard]] auto _getValue(std::string_view jsonData) -> std::string;

private:
    std::string m_key;
    std::variant<std::monostate, std::unique_ptr<std::string>, double, int, bool> m_value;
    std::vector<std::shared_ptr<JsonObject>> m_childs;
    bool m_root{};
    bool m_array;
    bool m_beautifyOutput{};
};

auto operator<<(std::ostream &output, const JsonObject &jsonObject) -> std::ostream &;
auto operator<<(std::stringstream &output, const JsonObject &jsonObject) -> std::stringstream &;

} // namespace tristan::json

#endif // JSON_HPP
