#include "json.hpp"

mt::json::JsonError::JsonError(const uint64_t p_position, const char p_unexpected_symbol) {
    m_description = "Unexpected symbol \'";
    m_description += p_unexpected_symbol;
    m_description += "\' on position ";
    m_description += std::to_string(p_position);
    m_position = p_position;
}

const char *mt::json::JsonError::what() const noexcept {
    return m_description.c_str();
}

auto mt::json::JsonError::position() const noexcept -> uint64_t {
    return m_position;
}

mt::json::JsonValue::JsonValue(std::string p_value) :
    m_value(std::move(p_value)) {
}

mt::json::JsonValue::JsonValue(const char *p_value) :
    m_value(std::string(p_value)) {
}

mt::json::JsonValue::JsonValue(double p_value) :
    m_value(p_value) {
}

mt::json::JsonValue::JsonValue(int64_t p_value) :
    m_value(p_value) {
}

mt::json::JsonValue::JsonValue(bool p_value) :
    m_value(p_value) {
}

auto mt::json::JsonValue::fromString(const std::string& p_data, uint64_t& pos) -> JsonValue {
    if (p_data[ pos ] == '"') {
        ++pos;
        std::string value;
        bool escape = false;
        for (const uint64_t size = p_data.size(); pos < size; ++pos) {
            if (p_data[ pos ] == '\"') {
                ++pos;
                return JsonValue{value};
            }
            if (escape) {
                switch (p_data[ pos ]) {
                    case '\\': {
                        value[ pos - 1 ] = '\\';
                        break;
                    }
                    case '\"': {
                        value[ pos - 1 ] = '\"';
                        break;
                    }
                    case '/': {
                        value[ pos - 1 ] = '/';
                        break;
                    }
                    case 'b': {
                        value[ pos - 1 ] = '\b';
                        break;
                    }
                    case 'f': {
                        value[ pos - 1 ] = '\f';
                        break;
                    }
                    case 'n': {
                        value[ pos - 1 ] = '\n';
                        break;
                    }
                    case 'r': {
                        value[ pos - 1 ] = '\r';
                        break;
                    }
                    case 't': {
                        value[ pos - 1 ] = '\t';
                        break;
                    }
                    default: {
                        throw JsonError{pos, p_data[ pos ]};
                    }
                }
                escape = false;
                continue;
            }
            if (p_data[ pos ] == '\\') {
                escape = true;
            }
            value += p_data[ pos ];
        }
    }

    if (p_data[ pos ] == 't') {
        if (p_data[ ++pos ] == 'r') {
            if (p_data[ ++pos ] == 'u') {
                if (p_data[ ++pos ] == 'e') {
                    ++pos;
                    return JsonValue{true};
                }
                throw JsonError{pos, p_data[ pos ]};
            }
            throw JsonError{pos, p_data[ pos ]};
        }
        throw JsonError{pos, p_data[ pos ]};
    }

    if (p_data[ pos ] == 'f') {
        if (p_data[ ++pos ] == 'a') {
            if (p_data[ ++pos ] == 'l') {
                if (p_data[ ++pos ] == 's') {
                    if (p_data[ ++pos ] == 'e') {
                        ++pos;
                        return JsonValue{false};
                    }
                    throw JsonError{pos, p_data[ pos ]};
                }
                throw JsonError{pos, p_data[ pos ]};
            }
            throw JsonError{pos, p_data[ pos ]};
        }
        throw JsonError{pos, p_data[ pos ]};
    }

    if (p_data[ pos ] == 'n') {
        if (p_data[ ++pos ] == 'u') {
            if (p_data[ ++pos ] == 'l') {
                if (p_data[ ++pos ] == 'l') {
                    ++pos;
                    return {};
                }
                throw JsonError{pos, p_data[ pos ]};
            }
            throw JsonError{pos, p_data[ pos ]};
        }
        throw JsonError{pos, p_data[ pos ]};
    }

    bool plus_minus_allowed = true;
    bool exponent_allowed = false;
    bool dot_allowed = false;
    bool floating_point_number = false;
    bool end_of_value_allowed = false;
    bool begin = true;
    std::string number;
    for (const auto size = p_data.size(); pos < size; ++pos) {
        if (p_data[ pos ] == '+') {
            if (plus_minus_allowed) {
                plus_minus_allowed = false;
                continue;
            }
        }
        if (p_data[ pos ] == '-') {
            if (plus_minus_allowed) {
                number.push_back(p_data[ pos ]);
                plus_minus_allowed = false;
                continue;
            }
        }
        if (p_data[ pos ] >= '0' && p_data[ pos ] <= '9') {
            number.push_back(p_data[ pos ]);
            if (begin) {
                dot_allowed = true;
                begin = false;
            }
            end_of_value_allowed = true;
            if (floating_point_number and not exponent_allowed) {
                exponent_allowed = true;
            }
            continue;
        }
        if (p_data[ pos ] == '.') {
            if (dot_allowed) {
                number.push_back(p_data[ pos ]);
                dot_allowed = false;
                floating_point_number = true;
                end_of_value_allowed = false;
                continue;
            }
        }
        if (p_data[ pos ] == 'E' || p_data[ pos ] == 'e') {
            if (exponent_allowed) {
                number.push_back(p_data[ pos ]);
                exponent_allowed = false;
                plus_minus_allowed = true;
                end_of_value_allowed = false;
                continue;
            }
        }
        if ((p_data[ pos ] == ',' || p_data[ pos ] == '}' || p_data[ pos ] == ']' || p_data[ pos ] == ' ' || p_data[ pos ] == '\n' || p_data[ pos ] == '\t')
            && end_of_value_allowed) {
            if (floating_point_number) {
                return JsonValue(std::stod(number));
            }
            return JsonValue(static_cast< int64_t >(std::stoll(number)));
        }
        break;
    }
    throw JsonError{pos, p_data[ pos ]};
}

auto mt::json::JsonValue::toString() const -> std::string {

    std::string result;
    std::visit(
        [ &result ]< typename ValueType >(const ValueType& value) {
            if constexpr (std::is_same_v< ValueType, std::string >) {
                result = value;
            } else if constexpr (std::is_same_v< ValueType, double > or std::is_same_v< ValueType, int64_t >) {
                result = std::to_string(value);
            } else if constexpr (std::is_same_v< ValueType, bool >) {
                result = value ? "true" : "false";
            } else {
                result = "null";
            }
        },
        m_value);
    return result;
}

auto mt::json::JsonValue::toDouble() const -> double {
    return std::get< double >(m_value);
}

auto mt::json::JsonValue::toInt() const -> int64_t {
    return std::get< int64_t >(m_value);
}

auto mt::json::JsonValue::toBool() const -> bool {
    return std::get< bool >(m_value);
}

auto mt::json::JsonValue::isString() const -> bool {
    return std::holds_alternative< std::string >(m_value);
}

auto mt::json::JsonValue::isDouble() const -> bool {
    return std::holds_alternative< double >(m_value);
}

auto mt::json::JsonValue::isInt() const -> bool {
    return std::holds_alternative< int64_t >(m_value);
}

auto mt::json::JsonValue::isBool() const -> bool {
    return std::holds_alternative< bool >(m_value);
}

auto mt::json::JsonValue::isNull() const -> bool {
    return std::holds_alternative< std::monostate >(m_value);
}

mt::json::Json::Json(std::string p_key) :
    m_key(std::move(p_key)) {
}

mt::json::Json::Json(JsonValue p_value) :
    m_value(std::move(p_value)) {
}

mt::json::Json::Json(std::string p_key, JsonValue p_value) :
    m_value(std::move(p_value)),
    m_key(std::move(p_key)) {
}

mt::json::Json::Json(std::string p_key, Json&& p_value) :
    m_value(std::make_shared< Json >(std::move(p_value))),
    m_key(std::move(p_key)) {
}

mt::json::Json::Json(std::string p_key, std::shared_ptr< Json > p_value) :
    m_value(std::move(p_value)),
    m_key(std::move(p_key)) {
}

auto mt::json::Json::fromString(const std::string& p_json_data) -> Json {
    uint64_t position{0};
    return Json::fromString(p_json_data, position);
}

void mt::json::Json::addJson(std::shared_ptr< Json > p_json_element) {
    if (std::holds_alternative< JsonValue >(m_value)) {
        throw std::invalid_argument("Trying to add child to json which already has a value");
    }
    if (std::holds_alternative< std::monostate >(m_value)) {
        m_value = p_json_element;
        return;
    }
    if (std::holds_alternative< std::shared_ptr< Json > >(m_value)) {
        auto value = std::get< std::shared_ptr< Json > >(m_value);
        m_value = std::vector< std::shared_ptr< Json > >{};
        value->m_key.empty() ? m_array = true : m_object = true;
        std::get< std::vector< std::shared_ptr< Json > > >(m_value).push_back(std::move(value));
    }
    if (std::holds_alternative< std::vector< std::shared_ptr< Json > > >(m_value)) {
        auto& children = std::get< std::vector< std::shared_ptr< Json > > >(m_value);
        if (children.empty()) {
            p_json_element->m_key.empty() ? m_array = true : m_object = true;
            children.emplace_back(std::move(p_json_element));
            return;
        }
        if (m_object) {
            if (p_json_element->m_key.empty()) {
                throw std::invalid_argument("Trying to add child with key not set into json p_json_element");
            }
            for (auto& child: children) {
                if (child->m_key == p_json_element->m_key) {
                    child = p_json_element;
                    return;
                }
            }
        } else if (m_array) {
            if (not p_json_element->m_key.empty()) {
                throw std::invalid_argument("Trying to add child with key into json array");
            }
        }
        children.emplace_back(std::move(p_json_element));
    }
}

void mt::json::Json::addJson(Json&& p_json_element) {
    this->addJson(std::make_shared< mt::json::Json >(std::move(p_json_element)));
}

void mt::json::Json::setValue(JsonValue p_value) {
    m_value = std::move(p_value);
}

void mt::json::Json::setValue(std::shared_ptr< Json > p_json_element) {
    m_value = std::move(p_json_element);
}

void mt::json::Json::setValue(Json&& p_json_element) {
    m_value = std::make_shared< Json >(std::move(p_json_element));
}

auto mt::json::Json::isNull() const -> bool {
    if (std::holds_alternative< std::monostate >(m_value)) {
        return true;
    }
    if (std::holds_alternative< JsonValue >(m_value)) {
        return std::get< JsonValue >(m_value).isNull();
    }
    return false;
}

auto mt::json::Json::isObject() const -> bool {
    return m_object;
}

auto mt::json::Json::isArray() const -> bool {
    return m_array;
}

auto mt::json::Json::key() const -> const std::string& {
    return m_key;
}

auto mt::json::Json::value() const -> const JsonValue& {
    return std::get< JsonValue >(m_value);
}

auto mt::json::Json::toObject() const -> std::shared_ptr< Json > {
    return std::get< std::shared_ptr< Json > >(m_value);
}

auto mt::json::Json::toArray() const -> const std::vector< std::shared_ptr< mt::json::Json > >& {
    return std::get< std::vector< std::shared_ptr< Json > > >(m_value);
}

auto mt::json::Json::getChildByName(const std::string& p_name) const -> std::shared_ptr< mt::json::Json > {
    if (m_array) {
        return {};
    }
    for (const auto& children = std::get< std::vector< std::shared_ptr< Json > > >(m_value); const auto& child: children) {
        if (child->m_key == p_name) {
            return child;
        }
    }
    return {};
}

auto mt::json::Json::toString(const bool p_beautify, const uint8_t level, bool in_array, const std::string& indent) const -> std::string {
    std::string result;
    if (not m_key.empty() && m_key != "Root") {
        if (p_beautify) {
            for (uint8_t count = 0; count < level; ++count) {
                result += indent;
            }
        }
        result += '\"';
        result += m_key;
        result += "\":";
        if (p_beautify) {
            result += ' ';
        }
    }
    std::visit(
        [ this, &result, p_beautify, level, in_array, &indent ]< typename ValueType >(const ValueType& value) {
            if constexpr (std::is_same_v< ValueType, JsonValue >) {
                if (value.isString()) {
                    result += '\"';
                }
                result += value.toString();
                if (value.isString()) {
                    result += '\"';
                }
            } else if constexpr (std::is_same_v< ValueType, std::shared_ptr< Json > >) {
                if (value->isNull()) {
                    if (value->isObject()) {
                        result += "{}";
                    } else if (value->isArray()) {
                        result += "[]";
                    } else {
                        result += "null";
                    }
                } else {
                    result += value->toString(p_beautify, level, m_array, indent);
                }
            } else if constexpr (std::is_same_v< ValueType, std::vector< std::shared_ptr< Json > > >) {
                if (m_object) {
                    if (in_array) {
                        if (p_beautify) {
                            for (uint8_t count = 0; count < level; ++count) {
                                result += indent;
                            }
                        }
                    }
                    result += '{';
                } else if (m_array) {
                    if (in_array) {
                        if (p_beautify) {
                            for (uint8_t count = 0; count < level; ++count) {
                                result += indent;
                            }
                        }
                    }
                    result += '[';
                } else {
                    throw std::runtime_error("Should not get here");
                }
                if (p_beautify) {
                    result += '\n';
                }
                for (auto iter = value.begin(), end = value.end() - 1; iter < end; ++iter) {
                    result += iter->get()->toString(p_beautify, level + 1, m_array, indent);
                    result += ',';
                    if (p_beautify) {
                        result += '\n';
                    }
                }
                result += (value.end() - 1)->get()->toString(p_beautify, level + 1, m_array, indent);
                if (p_beautify) {
                    result += '\n';
                }
                if (p_beautify) {
                    for (uint8_t count = 0; count < level; ++count) {
                        result += indent;
                    }
                }
                result += m_object ? '}' : ']';
            } else {
                result += "null";
            }
        },
        m_value);
    return result;
}

auto mt::json::Json::fromString(const std::string& p_data, uint64_t& pos) -> Json {  //NOLINT
    if (p_data[ pos ] == '}') {
        Json element;
        element.m_object = true;
        return element;
    }
    if (p_data[ pos ] == ']') {
        Json element;
        element.m_array = true;
        return element;
    }
    if (p_data[ pos ] == '{') {
        Json element;
        if (pos == 0) {
            element = Json{"Root"};
        }
        ++pos;
        while (true) {
            pos = p_data.find_first_not_of(" \n\t", pos);
            element.addJson(Json::fromString(p_data, pos));
            pos = p_data.find_first_not_of(" \n\t", pos);
            if (p_data[ pos ] == ',') {
                ++pos;
                continue;
            }
            if (p_data[ pos ] == '}') {
                ++pos;
                return element;
            }
            throw JsonError{pos, p_data[ pos ]};
        }
    }

    if (p_data[ pos ] == '[') {
        Json element;
        if (pos == 0) {
            element = Json{"Root"};
        }
        ++pos;
        while (true) {
            pos = p_data.find_first_not_of(" \n\t", pos);
            element.addJson(Json::fromString(p_data, pos));
            pos = p_data.find_first_not_of(" \n\t", pos);
            if (p_data[ pos ] == ',') {
                ++pos;
                continue;
            }
            if (p_data[ pos ] == ']') {
                ++pos;
                return element;
            }
            throw JsonError{pos, p_data[ pos ]};
        }
    }

    if (p_data[ pos ] == '"') {
        std::string key;
        ++pos;
        bool escape = false;
        for (const uint64_t size = p_data.size(); pos < size; ++pos) {
            if (p_data[ pos ] == '\"') {
                ++pos;
                break;
            }
            if (escape) {
                if (p_data[ pos ] == 'u' || p_data[ pos ] == 'U') {
                    throw std::runtime_error("Not implemented");
                }
                switch (p_data[ pos ]) {
                    case '\\': {
                        key[ pos - 1 ] = '\\';
                        break;
                    }
                    case '\"': {
                        key[ pos - 1 ] = '\"';
                        break;
                    }
                    case '/': {
                        key[ pos - 1 ] = '/';
                        break;
                    }
                    case 'b': {
                        key[ pos - 1 ] = '\b';
                        break;
                    }
                    case 'f': {
                        key[ pos - 1 ] = '\f';
                        break;
                    }
                    case 'n': {
                        key[ pos - 1 ] = '\n';
                        break;
                    }
                    case 'r': {
                        key[ pos - 1 ] = '\r';
                        break;
                    }
                    case 't': {
                        key[ pos - 1 ] = '\t';
                        break;
                    }
                    default: {
                        throw JsonError{pos, p_data[ pos ]};
                    }
                }
                escape = false;
                continue;
            }
            if (p_data[ pos ] == '\\') {
                escape = true;
            }
            key += p_data[ pos ];
        }
        pos = p_data.find_first_not_of("\n\t ", pos);
        if (p_data[ pos ] != ':') {
            throw JsonError{pos, p_data[ pos ]};
        }
        ++pos;
        pos = p_data.find_first_not_of("\n\t ", pos);
        Json element{std::move(key)};
        if (p_data[ pos ] == '{' || p_data[ pos ] == '[') {
            element.setValue(Json::fromString(p_data, pos));
        } else {
            element.setValue(JsonValue::fromString(p_data, pos));
        }
        return element;
    }
    throw JsonError{pos, p_data[ pos ]};
}