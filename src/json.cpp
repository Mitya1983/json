#include "json.hpp"
#include <stdexcept>
#include <iostream>
#include <stack>
#include <locale>
#include <cassert>

namespace {

std::vector<std::string> parseObject(std::string_view jsonData);
std::vector<std::string> parseArray(std::string_view jsonData);
size_t findCloseBracket(const std::string &jsonData, size_t openBracketPos, char openBracket);

} // namespace

json::JsonObject::JsonObject() : m_root(true), m_array(false), m_beautifyOutput(false)
{
    m_value = std::monostate();
}

json::JsonObject::JsonObject(std::string_view jsonData) : m_root(true), m_array(false), m_beautifyOutput(false)
{
    m_value = std::monostate();
    std::string data(jsonData);
    if (data.at(0) == '{'){
        this->_addChildsFromObject(data);
    }
    else if (data.at(0) == '['){
        m_array = true;
        this->_addChildsFromArray(data);
    }

}

json::JsonObject::JsonObject(std::string_view key, std::monostate) :
    m_key(key),
    m_array(false)
{

}

json::JsonObject::JsonObject(std::string_view key, std::string_view value) :
    m_key(key),
    m_array(false)
{
    m_value = std::make_unique<std::string>(value);
}

json::JsonObject::JsonObject(std::string_view key, double value) :
    m_key(key),
    m_array(false)
{
    m_value = value;
}

json::JsonObject::JsonObject(std::string_view key, int value) :
    m_key(key),
    m_array(false)
{
    m_value = value;
}

json::JsonObject::JsonObject(std::string_view key, bool value) :
    m_key(key),
    m_array(false)
{
    m_value = value;
}

void json::JsonObject::addObject(json::JsonObject &&object)
{
    object.m_root = false;
    if (!m_childs.empty()){
        for (const auto &child : m_childs){
            if (child->m_key == object.m_key && !m_array){
                std::string msg = "The key [" + object.m_key + "] is already present.\n" \
                                  "Dublicate Keys are not valid in this context.\n" \
                                  "Dublicate keys are valid only if JsonObject is set to Array.";
                throw std::invalid_argument(msg);
            }
        }
    }
    m_childs.emplace_back(std::make_shared<json::JsonObject>(std::move(object)));
}

void json::JsonObject::setValue(std::string_view value)
{
    if (!value.empty()){
        m_value = std::make_unique<std::string>(value);
    }
}

void json::JsonObject::setValue(double value)
{
    m_value = value;
}

void json::JsonObject::setValue(int value)
{
    m_value = value;
}

void json::JsonObject::setValue(bool value)
{
    m_value = value;
}

std::shared_ptr<json::JsonObject> json::JsonObject::getChildByName(std::string_view name) const
{
    if (!this->isObject()){
        throw std::runtime_error("const json::JsonObject &json::JsonObject::getChildByName(std::string_view name): [this] is not an Object");
    }
    size_t count = 0;
    for (const auto &child : m_childs){
        if (child->m_key == name){
            return child;
        }
        ++count;
    }
    return {};
}

const std::vector<std::shared_ptr<json::JsonObject>> &json::JsonObject::toArray() const
{
    if (!m_array){
        throw std::runtime_error("JsonObject is not an array");
    }
    return m_childs;
}

std::string json::JsonObject::toString() const
{
    if(std::holds_alternative<std::monostate>(m_value)){
        return "";
    }
    return *std::get<std::unique_ptr<std::string>>(m_value);
}

double json::JsonObject::toDouble() const
{
    return std::get<double>(m_value);
}

int json::JsonObject::toInt() const
{
    return std::get<int>(m_value);
}

bool json::JsonObject::toBool() const
{
    return std::get<bool>(m_value);
}

bool json::JsonObject::isObject() const
{
    if (!m_childs.empty() && !m_array){
        return true;
    }

    return false;
}

bool json::JsonObject::isArray() const
{
    return !m_childs.empty() && m_array;
}

bool json::JsonObject::isString() const
{
    return std::holds_alternative<std::unique_ptr<std::string>>(m_value);
}

bool json::JsonObject::isDouble() const
{
    return std::holds_alternative<double>(m_value);
}

bool json::JsonObject::isInt() const
{
    return std::holds_alternative<int>(m_value);
}

bool json::JsonObject::isBool() const
{
    return std::holds_alternative<bool>(m_value);
}

bool json::JsonObject::isNull() const
{
    return std::holds_alternative<std::monostate>(m_value);
}

void json::JsonObject::_addChildsFromObject(std::string_view jsonData)
{
    auto childs = parseObject(jsonData);
    for (const auto &child : childs){
        json::JsonObject object;
        object.setKey(_getKey(child));
        std::string value = _getValue(child);
        if (value.at(0) == '{'){
            object._addChildsFromObject(value);
        }
        else if (value.at(0) == '['){
            object.setIsArray(true);
            object._addChildsFromArray(value);
        }
        else if (value.at(0) == '\"'){
            size_t start = 1;
            size_t end = value.size() - 1;
            object.setValue(value.substr(start, end - start));
        }
        else{
            if (value == "true"){
                object.setValue(true);
            }
            else if (value == "false"){
                object.setValue(false);
            }
            else if (value.at(0) >= '0' && value.at(0) <= '9'){
                if (value.find('.') != std::string::npos){
                    if (std::use_facet<std::numpunct<char>>(std::locale()).decimal_point() == ','){
                        value.replace(value.find('.'), 1, ",");
                    }
                    object.setValue(std::stod(value));
                }
                else{
                    object.setValue(std::stoi(value));
                }
            }
            else{
                if (value != "null"){
                    assert(false && "Some issue with json parsing");
                }
            }
        }
        this->addObject(std::move(object));
    }
}

void json::JsonObject::_addChildsFromArray(std::string_view jsonData)
{
    auto childs = parseArray(jsonData);
    for (const auto &child : childs){
        json::JsonObject object;
        if (child.at(0) == '{'){
            object._addChildsFromObject(child);
        }
        else{
            std::string value = _getValue(child);
            if (value.find("true") != std::string::npos){
                object.setValue(true);
            }
            else if (value.find("false") != std::string::npos){
                object.setValue(false);
            }
            else if (value.at(0) == '\"'){
                size_t start = 1;
                size_t end = value.size() - 1;
                object.setValue(value.substr(start, end - start));
            }
            else if (value.at(0) >= '0' && value.at(0) <= '9'){
                if (value.find('.') != std::string::npos){
                    if (std::use_facet< std::numpunct<char>>(std::locale()).decimal_point() == ','){
                        value.replace(value.find('.'), 1, ",");
                    }
                    object.setValue(std::stod(value));
                }
                else{
                    object.setValue(std::stoi(value));
                }
            }
            else{
                if (value != "null"){
                    assert(false && "Some issue with json parsing");
                }
            }
        }
        this->addObject(std::move(object));
    }
}

std::string json::JsonObject::_toStream() const
{
    std::string returnValue;
    bool returnJustValue = false;
    if (m_key.empty()){
        returnJustValue = true;
    }
    if (this->isObject()){
        size_t objectNumber = 1;
        size_t objectCount = m_childs.size();
        if (!returnJustValue){
            if (!m_root){
                returnValue += "\"" + m_key + "\":";
            }
        }
        returnValue += '{';
        for (const auto &JsonObject : m_childs){
            returnValue += JsonObject->_toStream();
            if (++objectNumber <= objectCount){
                returnValue += ",";
            }
        }
        returnValue += '}';
    }
    else if (this->isArray()){
        size_t objectNumber = 1;
        size_t objectCount = m_childs.size();
        if (!returnJustValue){
            returnValue += "\"" + m_key + "\":";
        }
        returnValue += '[';
        for (const auto &JsonObject : m_childs){
            returnValue += JsonObject->_toStream();
            if (++objectNumber <= objectCount){
                returnValue += ",";
            }
        }
        returnValue += ']';
    }
    else if (std::holds_alternative<std::unique_ptr<std::string>>(m_value)){
        if (!returnJustValue){
            returnValue += "\"" + m_key + "\":";
        }
        returnValue += "\"";
        returnValue += *std::get<std::unique_ptr<std::string>>(m_value);
        returnValue += "\"";
    }
    else if (std::holds_alternative<double>(m_value)){
        if (!returnJustValue){
            returnValue += "\"" + m_key + "\":";
        }
        std::string number = std::to_string(std::get<double>(m_value));
        auto pos = number.find(',');
        if (pos != std::string::npos){
            number.replace(pos, 1, ".");
        }
        returnValue += number;
    }
    else if (std::holds_alternative<int>(m_value)){
        if (!returnJustValue){
            returnValue += "\"" + m_key + "\":";
        }
        returnValue += std::to_string(std::get<int>(m_value));
    }
    else if (std::holds_alternative<bool>(m_value)){
        if (!returnJustValue){
            returnValue += "\"" + m_key + "\":";
        }
        returnValue += std::get<bool>(m_value) ? "true" : "false";
    }
    else if (std::holds_alternative<std::monostate>(m_value)){
        if (!returnJustValue){
            returnValue += "\"" + m_key + "\":";
        }
        if (m_array){
            returnValue += "[]";
        }
        else{
            returnValue += "null";
        }
    }

    return returnValue;
}

std::string json::JsonObject::_toStream_b(uint8_t level) const
{
    std::string returnValue;
    const std::string spacer = "  ";
    bool returnJustValue = false;
    if (m_key.empty()){
        returnJustValue = true;
    }
    if (this->isObject()){
        for (int levelCount = 0; levelCount < level; ++levelCount){
            returnValue += spacer;
        }
        size_t objectNumber = 1;
        size_t objectCount = m_childs.size();
        if (!returnJustValue){
            if (!m_root){
                returnValue += "\"" + m_key + "\":";
            }
        }
        returnValue += "{\n";
        for (const auto &JsonObject : m_childs){
            returnValue += JsonObject->_toStream_b(level + 1);
            if (++objectNumber <= objectCount){
                returnValue += ",\n";
            }
        }
        returnValue += "\n";
        for (int levelCount = 0; levelCount < level; ++levelCount){
            returnValue += spacer;
        }
        returnValue += "}";
    }
    else if (this->isArray()){
        for (int levelCount = 0; levelCount < level; ++levelCount){
            returnValue += spacer;
        }
        size_t objectNumber = 1;
        size_t objectCount = m_childs.size();
        if (!returnJustValue){
            returnValue += "\"" + m_key + "\":";
        }
        returnValue += "[\n";
        for (const auto &JsonObject : m_childs){
            returnValue += JsonObject->_toStream_b(level + 1);
            if (++objectNumber <= objectCount){
                returnValue += ",\n";
            }
        }
        returnValue += "\n";
        for (int levelCount = 0; levelCount < level; ++levelCount){
            returnValue += spacer;
        }
        returnValue += "]";
    }
    else if (std::holds_alternative<std::unique_ptr<std::string>>(m_value)){
        for (int levelCount = 0; levelCount < level; ++levelCount){
            returnValue += spacer;
        }
        if (!returnJustValue){
            returnValue += "\"" + m_key + "\": ";
        }
        returnValue += "\"";
        returnValue += *std::get<std::unique_ptr<std::string>>(m_value);
        returnValue += "\"";
    }
    else if (std::holds_alternative<double>(m_value)){
        for (int levelCount = 0; levelCount < level; ++levelCount){
            returnValue += spacer;
        }
        if (!returnJustValue){
            returnValue += "\"" + m_key + "\": ";
        }
        std::string number = std::to_string(std::get<double>(m_value));
        auto pos = number.find(',');
        if (pos != std::string::npos){
            number.replace(pos, 1, ".");
        }
        returnValue += number;
    }
    else if (std::holds_alternative<int>(m_value)){
        for (int levelCount = 0; levelCount < level; ++levelCount){
            returnValue += spacer;
        }
        if (!returnJustValue){
            returnValue += "\"" + m_key + "\": ";
        }
        returnValue += std::to_string(std::get<int>(m_value));
    }
    else if (std::holds_alternative<bool>(m_value)){
        for (int levelCount = 0; levelCount < level; ++levelCount){
            returnValue += spacer;
        }
        if (!returnJustValue){
            returnValue += "\"" + m_key + "\": ";
        }
        returnValue += std::get<bool>(m_value) ? "true" : "false";
    }
    else if (std::holds_alternative<std::monostate>(m_value)){
        for (int levelCount = 0; levelCount < level; ++levelCount){
            returnValue += spacer;
        }
        if (!returnJustValue){
            returnValue += "\"" + m_key + "\": ";
        }
        if (m_array){
            returnValue += "[]";
        }
        else{
            returnValue += "null";
        }
    }

    return returnValue;
}

std::string json::JsonObject::_getKey(std::string_view jsonData)
{
    std::string data(jsonData);
    auto keyStartPos = data.find('\"') + 1;
    auto check = data.find('{');
    if (check == std::string::npos){
        check = data.find('[');
    }
    assert(keyStartPos < check && "Json key:value pair doesn't comprise key");
    auto keyEndPos = data.find('\"', keyStartPos);

    return data.substr(keyStartPos, keyEndPos - keyStartPos);
}

std::string json::JsonObject::_getValue(std::string_view jsonData)
{
    std::string data(jsonData);
    auto separatorPos = data.find(':');
    if (separatorPos == std::string::npos){
        separatorPos = 0;
    }
    else{
        ++separatorPos;
    }
    auto valueStartPos = data.find('[', separatorPos);
    auto valueEndPos = valueStartPos;
    if (valueStartPos != std::string::npos){
        valueEndPos = data.find_last_of(']') + 1;
    }
    else{
        valueStartPos = data.find('{', separatorPos);
        if (valueStartPos != std::string::npos){
            valueEndPos = data.find_last_of('}') + 1;
        }
        else{
            valueStartPos = data.find('\"', separatorPos);
            if (valueStartPos != std::string::npos){
                valueEndPos = data.find_last_of('\"') + 1;
            }
            else{
                valueStartPos = data.find_first_not_of(' ', separatorPos);
                valueEndPos = data.find_last_not_of(" \r\n\t") + 1;
            }
        }
    }
    return data.substr(valueStartPos, valueEndPos - valueStartPos);
}

std::ostream &json::operator<<(std::ostream &output, const json::JsonObject &jsonObject)
{
    if (jsonObject.m_beautifyOutput){
        output << jsonObject._toStream_b();
    }
    else{
        output << jsonObject._toStream();
    }
    return output;
}

std::stringstream &json::operator<<(std::stringstream &output, const json::JsonObject &jsonObject)
{
    if (jsonObject.m_beautifyOutput){
        output << jsonObject._toStream_b();
    }
    else{
        output << jsonObject._toStream();
    }
    return output;
}


namespace {

std::vector<std::string> parseObject(std::string_view jsonData){
    std::string data(jsonData);
    std::vector<std::string> childs;
    u_int8_t level = 0;
    uint8_t initialLevel = 0;
    size_t start = 0, end = 0;
    for (size_t i = 0, n = data.size(); i < n; ++i){
        if (data.at(i) == '\"' && initialLevel == 0){
            initialLevel = level;
            start = i;
        }
        else if (data.at(i) == '{' || data.at(i) == '['){
            ++level;
        }
        else if(data.at(i) == ','){
            if (level == initialLevel){
                end = i;
                childs.emplace_back(data.substr(start, end - start));
                initialLevel = 0;
            }
        }
        else if (data.at(i) == '}' || data.at(i) == ']'){
            --level;
            if (level == 0){
                end = i;
                childs.emplace_back(data.substr(start, end - start));
            }
        }
    }
    return childs;
}

std::vector<std::string> parseArray(std::string_view jsonData){
    std::string data(jsonData);
    std::vector<std::string> childs;
    size_t childStart = 1;
    bool breakOnAdd = false;
    while (true) {
        childStart = data.find_first_not_of(" \r\n\t", childStart);
        if (childStart == data.size() - 1){
            break;
        }
        auto childEnd = childStart;
        if (data.at(childStart) == '{'){
            childEnd = data.find("},", childStart);
            if (childEnd == std::string::npos){
                childEnd = data.find_last_of('}');
                breakOnAdd = true;
            }
            ++childEnd;
        }
        else{
            childEnd = data.find(',', childStart);
            if (childEnd == std::string::npos){
                childEnd = data.find_last_not_of(" \r\n\t", data.size() - 2);
                ++childEnd;
            }
        }
        childs.emplace_back(data.substr(childStart, childEnd - childStart));
        if (breakOnAdd){
            break;
        }
        childStart = childEnd + 1;
    }
    return childs;
}

size_t findCloseBracket(const std::string jsonData, size_t openBracketPos, char openBracket){

    size_t returnValue = 0;
    char closeBracket;
    switch (openBracket){
    case '{':{
        closeBracket = '}';
        break;
    }
    case '[':{
        closeBracket = ']';
        break;
    }
    default:{
        throw std::invalid_argument("size_t findCloseBracket(const std::string jsonData, size_t openBracketPos, char openBracket): openBracket has invalid value");
        break;
    }
    }

    std::stack<size_t> openBrackets;
    for (int i = openBracketPos, n = jsonData.size(); i < n; ++i){
        if (jsonData.at(i) == openBracket){
            openBrackets.push(i);
        }
        if (jsonData.at(i) == closeBracket){
            openBrackets.pop();
            if (openBrackets.empty()){
                returnValue = i;
                break;
            }
        }
    }
    return returnValue;
}

} // namespace
