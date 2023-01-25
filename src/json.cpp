#include "json.hpp"
#include <stdexcept>
#include <iostream>
#include <stack>
#include <locale>
#include <cassert>
#include <utility>

namespace {

    [[nodiscard]] auto getKey(const std::string& json_data) -> std::string;
    [[nodiscard]] auto getValue(const std::string& json_data) -> std::string;
    std::vector< std::string > parseObject(const std::string& json_data);
    std::vector< std::string > parseArray(const std::string& json_data);
    //    [[maybe_unused]] size_t
    //        findCloseBracket(const std::string& jsonData, size_t openBracketPos, char openBracket);

}  // namespace

tristan::json::JsonObject::JsonObject() :
    m_root(true),
    m_array(false),
    m_beautifyOutput(false) {
    m_value = std::monostate();
}

tristan::json::JsonObject::JsonObject(const std::string& jsonData) :
    m_root(true),
    m_array(false),
    m_beautifyOutput(false) {
    m_value = std::monostate();
    if (jsonData.at(0) == '{') {
        this->_addChildrenFromObject(jsonData);
    } else if (jsonData.at(0) == '[') {
        m_array = true;
        this->_addChildrenFromArray(jsonData);
    }
}

tristan::json::JsonObject::JsonObject(std::string key, std::monostate) :
    m_key(std::move(key)),
    m_root(true),
    m_array(false),
    m_beautifyOutput(false) { }

tristan::json::JsonObject::JsonObject(std::string key, std::string value) :
    m_key(std::move(key)),
    m_root(true),
    m_array(false),
    m_beautifyOutput(false) {
    m_value = std::make_unique< std::string >(std::move(value));
}

tristan::json::JsonObject::JsonObject(std::string key, double value) :
    m_key(std::move(key)),
    m_root(true),
    m_array(false),
    m_beautifyOutput(false) {
    m_value = value;
}

tristan::json::JsonObject::JsonObject(std::string key, int64_t value) :
    m_key(std::move(key)),
    m_root(true),
    m_array(false),
    m_beautifyOutput(false) {
    m_value = value;
}

tristan::json::JsonObject::JsonObject(std::string key, bool value) :
    m_key(std::move(key)),
    m_root(true),
    m_array(false),
    m_beautifyOutput(false) {
    m_value = value;
}

void tristan::json::JsonObject::addObject(json::JsonObject&& object) {
    object.m_root = false;
    if (!m_children.empty()) {
        for (const auto& child: m_children) {
            if (child->m_key == object.m_key && !m_array) {
                std::string msg = "The key [" + object.m_key
                                  + "] is already present.\n"
                                    "Duplicate Keys are not valid in this context.\n"
                                    "Duplicate keys are valid only if JsonObject is set to Array.";
                throw std::invalid_argument(msg);
            }
        }
    }
    m_children.emplace_back(std::make_shared< json::JsonObject >(std::move(object)));
}

void tristan::json::JsonObject::setKey(std::string key) { m_key = std::move(key); }

void tristan::json::JsonObject::setValue(std::string value) {
    if (!value.empty()) {
        m_value = std::make_unique< std::string >(std::move(value));
    }
}

void tristan::json::JsonObject::setValue(double value) { m_value = value; }

void tristan::json::JsonObject::setValue(int64_t value) { m_value = value; }

void tristan::json::JsonObject::setValue(bool value) { m_value = value; }

void tristan::json::JsonObject::setBeautify(bool value) { m_beautifyOutput = value; }

std::shared_ptr< tristan::json::JsonObject >
    tristan::json::JsonObject::getChildByName(const std::string& name) const {
    if (!this->isObject()) {
        throw std::runtime_error(
            "const json::JsonObject &json::JsonObject::getChildByName(const std::string& "
            "name): [this] is not an Object");
    }
    for (const auto& child: m_children) {
        if (child->m_key == name) {
            return child;
        }
    }
    return {};
}

const std::vector< std::shared_ptr< tristan::json::JsonObject > >&
    tristan::json::JsonObject::toArray() const {
    if (!m_array) {
        throw std::runtime_error("JsonObject is not an array");
    }
    return m_children;
}

std::string tristan::json::JsonObject::toString() const {
    if (std::holds_alternative< std::monostate >(m_value)) {
        return "";
    }
    return *std::get< std::unique_ptr< std::string > >(m_value);
}

double tristan::json::JsonObject::toDouble() const { return std::get< double >(m_value); }

int64_t tristan::json::JsonObject::toInt() const { return std::get< int64_t >(m_value); }

bool tristan::json::JsonObject::toBool() const { return std::get< bool >(m_value); }

auto tristan::json::JsonObject::docToString() const -> std::string {
    if (m_beautifyOutput) {
        return _toStream_b();
    }
    return _toStream();
}

bool tristan::json::JsonObject::isObject() const {
    if (!m_children.empty() && !m_array) {
        return true;
    }

    return false;
}

bool tristan::json::JsonObject::isArray() const { return !m_children.empty() && m_array; }

bool tristan::json::JsonObject::isString() const {
    return std::holds_alternative< std::unique_ptr< std::string > >(m_value);
}

bool tristan::json::JsonObject::isDouble() const { return std::holds_alternative< double >(m_value); }

bool tristan::json::JsonObject::isInt() const { return std::holds_alternative< int64_t >(m_value); }

bool tristan::json::JsonObject::isBool() const { return std::holds_alternative< bool >(m_value); }

bool tristan::json::JsonObject::isNull() const { return std::holds_alternative< std::monostate >(m_value); }

void tristan::json::JsonObject::_addChildrenFromObject(const std::string& jsonData) {  //NOLINT
    auto children = parseObject(jsonData);
    for (const auto& child: children) {
        json::JsonObject object;
        object.setKey(getKey(child));
        std::string value = getValue(child);
        if (value.at(0) == '{') {
            object._addChildrenFromObject(value);
        } else if (value.at(0) == '[') {
            object.setIsArray(true);
            object._addChildrenFromArray(value);
        } else if (value.at(0) == '\"') {
            size_t start = 1;
            size_t end = value.size() - 1;
            object.setValue(value.substr(start, end - start));
        } else {
            if (value == "true") {
                object.setValue(true);
            } else if (value == "false") {
                object.setValue(false);
            } else if (value.at(0) >= '0' && value.at(0) <= '9') {
                if (value.find('.') != std::string::npos) {
                    if (std::use_facet< std::numpunct< char > >(std::locale()).decimal_point() == ',') {
                        value.replace(value.find('.'), 1, ",");
                    }
                    object.setValue(std::stod(value));
                } else {
                    object.setValue(std::stol(value));
                }
            } else {
                if (value != "null") {
                    assert(false && "Some issue with json parsing");
                }
            }
        }
        this->addObject(std::move(object));
    }
}

void tristan::json::JsonObject::_addChildrenFromArray(const std::string& jsonData) {  //NOLINT
    auto children = parseArray(jsonData);
    for (const auto& child: children) {
        json::JsonObject object;
        if (child.at(0) == '{') {
            object._addChildrenFromObject(child);
        } else {
            std::string value = getValue(child);
            if (value.find("true") != std::string::npos) {
                object.setValue(true);
            } else if (value.find("false") != std::string::npos) {
                object.setValue(false);
            } else if (value.at(0) == '\"') {
                size_t start = 1;
                size_t end = value.size() - 1;
                object.setValue(value.substr(start, end - start));
            } else if (value.at(0) >= '0' && value.at(0) <= '9') {
                if (value.find('.') != std::string::npos) {
                    if (std::use_facet< std::numpunct< char > >(std::locale()).decimal_point() == ',') {
                        value.replace(value.find('.'), 1, ",");
                    }
                    object.setValue(std::stod(value));
                } else {
                    object.setValue(std::stol(value));
                }
            } else {
                if (value != "null") {
                    assert(false && "Some issue with json parsing");
                }
            }
        }
        this->addObject(std::move(object));
    }
}

std::string tristan::json::JsonObject::_toStream() const {  //NOLINT
    std::string returnValue;
    bool returnJustValue = false;
    if (m_key.empty()) {
        returnJustValue = true;
    }
    if (this->isObject()) {
        size_t objectNumber = 1;
        size_t objectCount = m_children.size();
        if (!returnJustValue) {
            if (!m_root) {
                returnValue += "\"" + m_key + "\":";
            }
        }
        returnValue += '{';
        for (const auto& JsonObject: m_children) {
            returnValue += JsonObject->_toStream();
            if (++objectNumber <= objectCount) {
                returnValue += ",";
            }
        }
        returnValue += '}';
    } else if (this->isArray()) {
        size_t objectNumber = 1;
        size_t objectCount = m_children.size();
        if (!returnJustValue) {
            returnValue += "\"" + m_key + "\":";
        }
        returnValue += '[';
        for (const auto& JsonObject: m_children) {
            returnValue += JsonObject->_toStream();
            if (++objectNumber <= objectCount) {
                returnValue += ",";
            }
        }
        returnValue += ']';
    } else if (std::holds_alternative< std::unique_ptr< std::string > >(m_value)) {
        if (!returnJustValue) {
            returnValue += "\"" + m_key + "\":";
        }
        returnValue += "\"";
        returnValue += *std::get< std::unique_ptr< std::string > >(m_value);
        returnValue += "\"";
    } else if (std::holds_alternative< double >(m_value)) {
        if (!returnJustValue) {
            returnValue += "\"" + m_key + "\":";
        }
        std::string number = std::to_string(std::get< double >(m_value));
        auto pos = number.find(',');
        if (pos != std::string::npos) {
            number.replace(pos, 1, ".");
        }
        returnValue += number;
    } else if (std::holds_alternative< int64_t >(m_value)) {
        if (!returnJustValue) {
            returnValue += "\"" + m_key + "\":";
        }
        returnValue += std::to_string(std::get< int64_t >(m_value));
    } else if (std::holds_alternative< bool >(m_value)) {
        if (!returnJustValue) {
            returnValue += "\"" + m_key + "\":";
        }
        returnValue += std::get< bool >(m_value) ? "true" : "false";
    } else if (std::holds_alternative< std::monostate >(m_value)) {
        if (!returnJustValue) {
            returnValue += "\"" + m_key + "\":";
        }
        if (m_array) {
            returnValue += "[]";
        } else {
            returnValue += "null";
        }
    }

    return returnValue;
}

std::string tristan::json::JsonObject::_toStream_b(uint8_t level) const {  //NOLINT
    std::string returnValue;
    const std::string spacer = "  ";
    bool returnJustValue = false;
    if (m_key.empty()) {
        returnJustValue = true;
    }
    if (this->isObject()) {
        for (int levelCount = 0; levelCount < level; ++levelCount) {
            returnValue += spacer;
        }
        size_t objectNumber = 1;
        size_t objectCount = m_children.size();
        if (!returnJustValue) {
            if (!m_root) {
                returnValue += "\"" + m_key + "\":";
            }
        }
        returnValue += "{\n";
        for (const auto& JsonObject: m_children) {
            returnValue += JsonObject->_toStream_b(level + 1);
            if (++objectNumber <= objectCount) {
                returnValue += ",\n";
            }
        }
        returnValue += "\n";
        for (int levelCount = 0; levelCount < level; ++levelCount) {
            returnValue += spacer;
        }
        returnValue += "}";
    } else if (this->isArray()) {
        for (int levelCount = 0; levelCount < level; ++levelCount) {
            returnValue += spacer;
        }
        size_t objectNumber = 1;
        size_t objectCount = m_children.size();
        if (!returnJustValue) {
            returnValue += "\"" + m_key + "\":";
        }
        returnValue += "[\n";
        for (const auto& JsonObject: m_children) {
            returnValue += JsonObject->_toStream_b(level + 1);
            if (++objectNumber <= objectCount) {
                returnValue += ",\n";
            }
        }
        returnValue += "\n";
        for (int levelCount = 0; levelCount < level; ++levelCount) {
            returnValue += spacer;
        }
        returnValue += "]";
    } else if (std::holds_alternative< std::unique_ptr< std::string > >(m_value)) {
        for (int levelCount = 0; levelCount < level; ++levelCount) {
            returnValue += spacer;
        }
        if (!returnJustValue) {
            returnValue += "\"" + m_key + "\": ";
        }
        returnValue += "\"";
        returnValue += *std::get< std::unique_ptr< std::string > >(m_value);
        returnValue += "\"";
    } else if (std::holds_alternative< double >(m_value)) {
        for (int levelCount = 0; levelCount < level; ++levelCount) {
            returnValue += spacer;
        }
        if (!returnJustValue) {
            returnValue += "\"" + m_key + "\": ";
        }
        std::string number = std::to_string(std::get< double >(m_value));
        auto pos = number.find(',');
        if (pos != std::string::npos) {
            number.replace(pos, 1, ".");
        }
        returnValue += number;
    } else if (std::holds_alternative< int64_t >(m_value)) {
        for (int levelCount = 0; levelCount < level; ++levelCount) {
            returnValue += spacer;
        }
        if (!returnJustValue) {
            returnValue += "\"" + m_key + "\": ";
        }
        returnValue += std::to_string(std::get< int64_t >(m_value));
    } else if (std::holds_alternative< bool >(m_value)) {
        for (int levelCount = 0; levelCount < level; ++levelCount) {
            returnValue += spacer;
        }
        if (!returnJustValue) {
            returnValue += "\"" + m_key + "\": ";
        }
        returnValue += std::get< bool >(m_value) ? "true" : "false";
    } else if (std::holds_alternative< std::monostate >(m_value)) {
        for (int levelCount = 0; levelCount < level; ++levelCount) {
            returnValue += spacer;
        }
        if (!returnJustValue) {
            returnValue += "\"" + m_key + "\": ";
        }
        if (m_array) {
            returnValue += "[]";
        } else {
            returnValue += "null";
        }
    }

    return returnValue;
}

std::ostream& tristan::json::operator<<(std::ostream& output, const json::JsonObject& jsonObject) {
    if (jsonObject.m_beautifyOutput) {
        output << jsonObject._toStream_b();
    } else {
        output << jsonObject._toStream();
    }
    return output;
}

std::stringstream& tristan::json::operator<<(std::stringstream& output, const json::JsonObject& jsonObject) {
    if (jsonObject.m_beautifyOutput) {
        output << jsonObject._toStream_b();
    } else {
        output << jsonObject._toStream();
    }
    return output;
}

namespace {

    auto getKey(const std::string& json_data) -> std::string {

        auto keyStartPos = json_data.find('\"') + 1;
        auto check = json_data.find('{');
        if (check == std::string::npos) {
            check = json_data.find('[');
        }
        assert(keyStartPos < check && "Json key:value pair doesn't comprise key");
        auto keyEndPos = json_data.find('\"', keyStartPos);

        return json_data.substr(keyStartPos, keyEndPos - keyStartPos);
    }

    auto getValue(const std::string& json_data) -> std::string {
        auto separatorPos = json_data.find(':');
        if (separatorPos == std::string::npos) {
            separatorPos = 0;
        } else {
            ++separatorPos;
        }
        auto valueStartPos = json_data.find('[', separatorPos);
        auto valueEndPos = valueStartPos;
        if (valueStartPos != std::string::npos) {
            valueEndPos = json_data.find_last_of(']') + 1;
        } else {
            valueStartPos = json_data.find('{', separatorPos);
            if (valueStartPos != std::string::npos) {
                valueEndPos = json_data.find_last_of('}') + 1;
            } else {
                valueStartPos = json_data.find('\"', separatorPos);
                if (valueStartPos != std::string::npos) {
                    valueEndPos = json_data.find_last_of('\"') + 1;
                } else {
                    valueStartPos = json_data.find_first_not_of(' ', separatorPos);
                    valueEndPos = json_data.find_last_not_of(" \r\n\t") + 1;
                }
            }
        }
        return json_data.substr(valueStartPos, valueEndPos - valueStartPos);
    }

    std::vector< std::string > parseObject(const std::string& json_data) {
        std::vector< std::string > children;
        u_int8_t level = 0;
        uint8_t initialLevel = 0;
        size_t start = 0, end = 0;
        for (size_t i = 0, n = json_data.size(); i < n; ++i) {
            if (json_data.at(i) == '\"' && initialLevel == 0) {
                initialLevel = level;
                start = i;
            } else if (json_data.at(i) == '{' || json_data.at(i) == '[') {
                ++level;
            } else if (json_data.at(i) == ',') {
                if (level == initialLevel) {
                    end = i;
                    children.emplace_back(json_data.substr(start, end - start));
                    initialLevel = 0;
                }
            } else if (json_data.at(i) == '}' || json_data.at(i) == ']') {
                --level;
                if (level == 0) {
                    end = i;
                    children.emplace_back(json_data.substr(start, end - start));
                }
            }
        }
        return children;
    }

    std::vector< std::string > parseArray(const std::string& json_data) {
        std::vector< std::string > children;
        size_t childStart = 1;
        bool breakOnAdd = false;
        while (true) {
            childStart = json_data.find_first_not_of(" \r\n\t", childStart);
            if (childStart == json_data.size() - 1) {
                break;
            }
            auto childEnd = childStart;
            if (json_data.at(childStart) == '{') {
                childEnd = json_data.find("},", childStart);
                if (childEnd == std::string::npos) {
                    childEnd = json_data.find_last_of('}');
                    breakOnAdd = true;
                }
                ++childEnd;
            } else {
                childEnd = json_data.find(',', childStart);
                if (childEnd == std::string::npos) {
                    childEnd = json_data.find_last_not_of(" \r\n\t", json_data.size() - 2);
                    ++childEnd;
                }
            }
            children.emplace_back(json_data.substr(childStart, childEnd - childStart));
            if (breakOnAdd) {
                break;
            }
            childStart = childEnd + 1;
        }
        return children;
    }

}  // namespace
