#include "json.hpp"
#include <stdexcept>
#include <iostream>
#include <stack>
#include <utility>
#include <queue>

tristan::json::JsonElement::JsonElement() :
        m_key(std::nullopt),
        m_value(std::monostate()),
        m_object(false),
        m_array(false) {}

tristan::json::JsonElement::JsonElement(std::string string, ArgumentType argument_type) :
        m_object(true),
        m_array(false) {
    if (argument_type == ArgumentType::KEY) {
        m_key = std::move(string);
        m_value = std::monostate();
    } else {
        m_key = std::nullopt;
        m_value = std::move(string);
    }
}

tristan::json::JsonElement::JsonElement(std::string key, std::string value) :
        m_key(std::move(key)),
        m_value(std::move(value)),
        m_object(false),
        m_array(false) {}

tristan::json::JsonElement::JsonElement(std::string key, double value) :
        m_key(std::move(key)),
        m_value(value),
        m_object(false),
        m_array(false) {}

tristan::json::JsonElement::JsonElement(double value) :
        m_key(std::nullopt),
        m_value(value),
        m_object(false),
        m_array(false) {}

tristan::json::JsonElement::JsonElement(std::string key, int64_t value) :
        m_key(std::move(key)),
        m_value(value),
        m_object(false),
        m_array(false) {}

tristan::json::JsonElement::JsonElement(int64_t value) :
        m_key(std::nullopt),
        m_value(value),
        m_object(false),
        m_array(false) {}

tristan::json::JsonElement::JsonElement(std::string key, bool value) :
        m_key(std::move(key)),
        m_value(value),
        m_object(false),
        m_array(false) {}

tristan::json::JsonElement::JsonElement(bool value) :
        m_key(std::nullopt),
        m_value(value),
        m_object(false),
        m_array(false) {}

void tristan::json::JsonElement::addElement(std::shared_ptr<JsonElement> element) {
    if (std::holds_alternative<std::monostate>(m_value)) {
        m_value = Children();
    }
    if (std::holds_alternative<Children>(m_value)) {
        auto &children = std::get<Children>(m_value);
        if (children.empty()) {
            children.emplace_back(std::move(element));
            return;
        }
        if (m_object) {
            if (not element->m_key) {
                throw std::invalid_argument("Trying to add child with key not set into json element");
            }
            for (auto &child: children) {
                if (child->m_key == element->m_key) {
                    child = element;
                    return;
                }
            }
        } else if (m_array) {
            if (element->m_key) {
                throw std::invalid_argument("Trying to add child with key into json array");
            }
        }
        children.emplace_back(std::move(element));
    }
}

void tristan::json::JsonElement::setValue(std::string value) {
    if (!value.empty()) {
        m_value = std::move(value);
    }
}

void tristan::json::JsonElement::setValue(double value) { m_value = value; }

void tristan::json::JsonElement::setValue(int64_t value) { m_value = value; }

void tristan::json::JsonElement::setValue(bool value) { m_value = value; }

auto tristan::json::JsonElement::key() const -> const std::optional<std::string> & { return m_key; }

auto tristan::json::JsonElement::toArray() const -> const std::vector<std::shared_ptr<tristan::json::JsonElement> > & {
    if (!m_array) {
        throw std::runtime_error("JsonElement is not an array");
    }
    return std::get<Children>(m_value);
}

auto tristan::json::JsonElement::toString() const -> const std::string & { return std::get<std::string>(m_value); }

auto tristan::json::JsonElement::toDouble() const -> double { return std::get<double>(m_value); }

auto tristan::json::JsonElement::toInt() const -> int64_t { return std::get<int64_t>(m_value); }

auto tristan::json::JsonElement::toBool() const -> bool { return std::get<bool>(m_value); }

auto tristan::json::JsonElement::isObject() const -> bool { return m_object; }

auto tristan::json::JsonElement::isArray() const -> bool { return m_array; }

auto tristan::json::JsonElement::isString() const -> bool { return std::holds_alternative<std::string>(m_value); }

auto tristan::json::JsonElement::isDouble() const -> bool { return std::holds_alternative<double>(m_value); }

auto tristan::json::JsonElement::isInt() const -> bool { return std::holds_alternative<int64_t>(m_value); }

auto tristan::json::JsonElement::isBool() const -> bool { return std::holds_alternative<bool>(m_value); }

auto tristan::json::JsonElement::isNull() const -> bool { return std::holds_alternative<std::monostate>(m_value); }

auto tristan::json::JsonElement::print() -> std::string {
    std::string result;
    if (m_key) {
        result += '\"';
        result += m_key.value();
        result += "\":";
    }
    std::visit(
            [&result, this](auto &&value) -> void {
                using ValueType = std::decay_t<decltype(value)>;
                if constexpr (std::is_same_v<ValueType, std::monostate>) {
                    result += "null";
                } else if constexpr (std::is_same_v<ValueType, std::string>) {
                    result += '\"';
                    result += tristan::json::JsonDoc::encodeStringValue(value);
                    result += '\"';
                } else if constexpr (std::is_same_v<ValueType, double> or std::is_same_v<ValueType, int64_t>) {
                    result += std::to_string(value);
                } else if constexpr (std::is_same_v<ValueType, bool>) {
                    result += value ? "true" : "false";
                } else if constexpr (std::is_same_v<ValueType, Children>) {
                    if (m_object) {
                        result += '{';
                    } else if (m_array) {
                        result += '[';
                    }
                    for (const auto &child: value) {
                        result += child->print();
                        if (child.get() == (value.end() - 1)->get()) {
                            break;
                        }
                        result += ',';
                    }
                    if (m_object) {
                        result += '}';
                    } else if (m_array) {
                        result += ']';
                    }
                }
            },
            m_value);
    return result;
}

tristan::json::JsonError::JsonError() :
        place(std::string::npos) {}

tristan::json::JsonError::JsonError(uint64_t place_, char symbol) :
        place(place_) {
    description = "Unexpected symbol \'";
    description += symbol;
    description += '\'';
}

tristan::json::JsonError::operator bool() const { return place != std::string::npos; }

std::ostream &tristan::json::operator<<(std::ostream &output, const json::JsonDoc &json_doc) {
    output << json_doc.toString();
    return output;
}

std::stringstream &tristan::json::operator<<(std::stringstream &output, const json::JsonDoc &json_doc) {
    output << json_doc.toString();
    return output;
}

tristan::json::JsonDoc::JsonDoc() :
        m_object(false),
        m_array(false),
        m_beautify_output(false) {}

auto tristan::json::JsonDoc::createJsonDocument() -> std::shared_ptr<JsonDoc> {
    return std::shared_ptr<JsonDoc>(new JsonDoc());
}

auto tristan::json::JsonDoc::createJsonDocument(const std::string &json_document) -> std::shared_ptr<JsonDoc> {
    auto doc = createJsonDocument();

    bool key_parsed = false;
    bool string_value_parsed = false;
    bool number_value_parsed = false;
    bool number_has_dot = false;
    bool number_has_exponent = false;
    bool true_value_parsed = false;
    bool false_value_parsed = false;
    bool null_value_parsed = false;

    uint64_t current_parsing_pos = 0;

    char current_symbol = json_document[current_parsing_pos], previous_symbol;

    if (current_symbol == '{') {
        doc->m_object = true;
    } else if (current_symbol == '[') {
        doc->m_array = true;
    } else {
        doc->m_error = JsonError(current_parsing_pos, current_symbol);
        return doc;
    }

    ++current_parsing_pos;

    std::stack<std::shared_ptr<JsonElement> > json_objects_stack;

    json_objects_stack.push(nullptr);

    std::shared_ptr<JsonElement> current_object;

    for (auto n = json_document.size(); current_parsing_pos < n; ++current_parsing_pos) {

        if (json_document[current_parsing_pos] == ' ' or json_document[current_parsing_pos] == '\n' or
            json_document[current_parsing_pos] == '\t'
            or json_document[current_parsing_pos] == '\b' or json_document[current_parsing_pos] == '\f' or
            json_document[current_parsing_pos] == '\r') {
            continue;
        }
        previous_symbol = current_symbol;
        current_symbol = json_document[current_parsing_pos];

        if (json_objects_stack.empty()) {
            doc->m_error = JsonError(current_parsing_pos, current_symbol);
            return doc;
        }

        current_object = json_objects_stack.top();

        if (current_symbol == '{') {
            if (previous_symbol == '[' or (previous_symbol == ',' and current_object and current_object->m_array) or
                (not current_object and doc->m_array)) {
                auto child = std::make_shared<JsonElement>();
                child->m_object = true;
                json_objects_stack.emplace(child);
                if (current_object) {
                    current_object->addElement(std::move(child));
                } else {
                    doc->m_children.emplace_back(std::move(child));
                }
                continue;
            }
            if (previous_symbol == ':') {
                current_object->m_object = true;
                continue;
            }
            doc->m_error = JsonError(current_parsing_pos, current_symbol);
            return doc;
        }

        if (current_symbol == '[') {
            if (previous_symbol == '[' or (previous_symbol == ',' and current_object and current_object->m_array) or
                (not current_object and doc->m_array)) {
                auto child = std::make_shared<JsonElement>();
                child->m_array = true;
                json_objects_stack.emplace(child);
                current_object->addElement(std::move(child));
                continue;
            }
            if (previous_symbol == ':') {
                current_object->m_array = true;
                continue;
            }
            doc->m_error = JsonError(current_parsing_pos, current_symbol);
            return doc;
        }

        if (current_symbol == '}' or current_symbol == ']') {
            if (previous_symbol == '\"' and string_value_parsed) {
                string_value_parsed = false;
                json_objects_stack.pop();
                continue;
            }
            if (previous_symbol >= '0' and previous_symbol <= '9' and number_value_parsed) {
                number_value_parsed = false;
                json_objects_stack.pop();
                continue;
            }
            if (previous_symbol == 'e' and true_value_parsed and current_object->isBool() and
                current_object->toBool()) {
                true_value_parsed = false;
                json_objects_stack.pop();
                continue;
            }
            if (previous_symbol == 'e' and false_value_parsed and current_object->isBool() and
                not current_object->toBool()) {
                false_value_parsed = false;
                json_objects_stack.pop();
                continue;
            }
            if (previous_symbol == 'l' and null_value_parsed and current_object->isNull()) {
                null_value_parsed = false;
                json_objects_stack.pop();
                continue;
            }
            if ((current_symbol == '}' and (previous_symbol == '{' or previous_symbol == '}' or previous_symbol == ']'))
                or (current_symbol == ']' and
                    (previous_symbol == '}' or previous_symbol == ']' or previous_symbol == '['))) {
                if (current_object) {
                    if (std::holds_alternative<Children>(current_object->m_value) and
                        std::get<Children>(current_object->m_value).empty()) {
                        current_object->m_value = std::monostate();
                    }
                }
                json_objects_stack.pop();
                continue;
            }
            doc->m_error = JsonError(current_parsing_pos, current_symbol);
            return doc;
        }

        if (current_symbol == ':') {
            if (key_parsed) {
                key_parsed = false;
                continue;
            }
            doc->m_error = JsonError(current_parsing_pos, current_symbol);
            return doc;
        }

        if (current_symbol == ',') {
            if (previous_symbol == '\"' and string_value_parsed) {
                string_value_parsed = false;
                continue;
            } else if (previous_symbol >= '0' and previous_symbol <= '9' and number_value_parsed) {
                number_value_parsed = false;
                continue;
            } else if (previous_symbol == 'e' and true_value_parsed) {
                true_value_parsed = false;
                continue;
            } else if (previous_symbol == 'e' and false_value_parsed) {
                false_value_parsed = false;
                continue;
            } else if (previous_symbol == 'l' and null_value_parsed) {
                null_value_parsed = false;
                continue;
            } else if (previous_symbol == '}' or previous_symbol == ']') {
                continue;
            }
            doc->m_error = JsonError(current_parsing_pos, current_symbol);
            return doc;
        }

        if (current_symbol == '\"') {
            if (((current_object and current_object->m_object) or (not current_object and doc->m_object))
                and (previous_symbol == '{' or previous_symbol == ',')) {
                auto child = std::make_shared<JsonElement>();
                ++current_parsing_pos;
                std::string key;
                while (true) {
                    current_symbol = json_document[current_parsing_pos];
                    if (current_symbol == '\\') {
                        previous_symbol = current_symbol;
                        continue;
                    }
                    if (current_symbol == '\"' and previous_symbol != '\\') {
                        key_parsed = true;
                        child->m_key = std::move(key);
                        json_objects_stack.emplace(child);
                        if (current_object) {
                            current_object->addElement(std::move(child));
                        } else {
                            doc->m_children.emplace_back(std::move(child));
                        }
                        break;
                    }
                    key += current_symbol;
                    ++current_parsing_pos;
                }
                continue;
            }
            if ((((current_object and current_object->m_array) or (not current_object and doc->m_array)) and
                 (previous_symbol == '[' or previous_symbol == ','))
                or previous_symbol == ':') {
                std::string value;
                while (true) {
                    previous_symbol = current_symbol;
                    ++current_parsing_pos;
                    current_symbol = json_document[current_parsing_pos];
                    if (current_symbol == '\\') {
                        continue;
                    }
                    if (current_symbol == '\"' and previous_symbol != '\\') {
                        string_value_parsed = true;
                        current_object->m_value = std::move(value);
                        json_objects_stack.pop();
                        break;
                    }
                    value += current_symbol;
                }
                continue;
            }
            doc->m_error = JsonError(current_parsing_pos, current_symbol);
            return doc;
        }

        if (current_symbol == '-' or (current_symbol >= '0' and current_symbol <= '9')) {
            if (previous_symbol == ':'
                or (previous_symbol == ',' and
                    ((current_object and current_object->m_array) or (not current_object and doc->m_array)))) {
                std::string value;
                bool value_is_double = false;
                while (true) {
                    value += current_symbol;
                    ++current_parsing_pos;
                    previous_symbol = current_symbol;
                    current_symbol = json_document[current_parsing_pos];
                    if ((current_symbol == '-' or current_symbol == '+') and
                        not(previous_symbol == 'E' or previous_symbol == 'e')) {
                        doc->m_error = JsonError(current_parsing_pos, current_symbol);
                        return doc;
                    }
                    if (current_symbol == ',' or current_symbol == '}' or current_symbol == ']' or
                        current_symbol == ' ' or current_symbol == '\n'
                        or current_symbol == '\t' or current_symbol == '\b' or current_symbol == '\f' or
                        current_symbol == '\r') {
                        --current_parsing_pos;
                        current_symbol = json_document[current_parsing_pos];
                        break;
                    }
                    if (current_symbol >= '0' and current_symbol <= '9') {
                        continue;
                    }
                    if (current_symbol == '.') {
                        value_is_double = true;
                        if (number_has_dot) {
                            doc->m_error = JsonError(current_parsing_pos, current_symbol);
                            return doc;
                        }
                        number_has_dot = true;
                        continue;
                    }
                    if (current_symbol == 'E' or current_symbol == 'e') {
                        value_is_double = true;
                        if (number_has_exponent) {
                            doc->m_error = JsonError(current_parsing_pos, current_symbol);
                            return doc;
                        }
                        number_has_exponent = true;
                    }
                }
                number_value_parsed = true;
                if (value_is_double) {
                    current_object->m_value = std::stod(value);
                } else {
                    current_object->m_value = std::stoll(value);
                }
                json_objects_stack.pop();
                continue;
            } else {
                doc->m_error = JsonError(current_parsing_pos, current_symbol);
                return doc;
            }
        }

        if (current_symbol == 't') {
            if (previous_symbol == ':'
                or (previous_symbol == ',' and
                    ((current_object and current_object->m_array) or (not current_object and doc->m_array)))) {
                std::string value;
                while (true) {
                    value += current_symbol;
                    ++current_parsing_pos;
                    current_symbol = json_document[current_parsing_pos];
                    if (current_symbol == ',' or current_symbol == '}' or current_symbol == ']' or
                        current_symbol == ' ' or current_symbol == '\n'
                        or current_symbol == '\t' or current_symbol == '\b' or current_symbol == '\f' or
                        current_symbol == '\r') {
                        --current_parsing_pos;
                        current_symbol = json_document[current_parsing_pos];
                        break;
                    }
                    if ((current_symbol == 'r' and value.size() == 1) or (current_symbol == 'u' and value.size() == 2)
                        or (current_symbol == 'e' and value.size() == 3)) {
                        continue;
                    }
                    doc->m_error = JsonError(current_parsing_pos, current_symbol);
                    return doc;
                }
                true_value_parsed = true;
                current_object->m_value = true;
                json_objects_stack.pop();
                continue;
            } else {
                doc->m_error = JsonError(current_parsing_pos, current_symbol);
                return doc;
            }
        }

        if (current_symbol == 'f') {
            if (previous_symbol == ':'
                or (previous_symbol == ',' and
                    ((current_object and current_object->m_array) or (not current_object and doc->m_array)))) {
                std::string value;
                while (true) {
                    value += current_symbol;
                    ++current_parsing_pos;
                    current_symbol = json_document[current_parsing_pos];
                    if (current_symbol == ',' or current_symbol == '}' or current_symbol == ']' or
                        current_symbol == ' ' or current_symbol == '\n'
                        or current_symbol == '\t' or current_symbol == '\b' or current_symbol == '\f' or
                        current_symbol == '\r') {
                        --current_parsing_pos;
                        current_symbol = json_document[current_parsing_pos];
                        break;
                    }
                    if ((current_symbol == 'a' and value.size() == 1) or (current_symbol == 'l' and value.size() == 2)
                        or (current_symbol == 's' and value.size() == 3) or
                        (current_symbol == 'e' and value.size() == 4)) {
                        continue;
                    }
                    doc->m_error = JsonError(current_parsing_pos, current_symbol);
                    return doc;
                }
                false_value_parsed = true;
                current_object->m_value = false;
                json_objects_stack.pop();
                continue;
            } else {
                doc->m_error = JsonError(current_parsing_pos, current_symbol);
                return doc;
            }
        }

        if (current_symbol == 'n') {
            if (previous_symbol == ':'
                or (previous_symbol == ',' and
                    ((current_object and current_object->m_array) or (not current_object and doc->m_array)))) {
                std::string value;
                while (true) {
                    value += current_symbol;
                    ++current_parsing_pos;
                    current_symbol = json_document[current_parsing_pos];
                    if (current_symbol == ',' or current_symbol == '}' or current_symbol == ']' or
                        current_symbol == ' ' or current_symbol == '\n'
                        or current_symbol == '\t' or current_symbol == '\b' or current_symbol == '\f' or
                        current_symbol == '\r') {
                        --current_parsing_pos;
                        current_symbol = json_document[current_parsing_pos];
                        break;
                    }
                    if ((current_symbol == 'u' and value.size() == 1) or (current_symbol == 'l' and value.size() == 2)
                        or (current_symbol == 'l' and value.size() == 3)) {
                        continue;
                    }
                    doc->m_error = JsonError(current_parsing_pos, current_symbol);
                    return doc;
                }
                null_value_parsed = true;
                //                current_object->m_value = value;
                json_objects_stack.pop();
                continue;
            } else {
                doc->m_error = JsonError(current_parsing_pos, current_symbol);
                return doc;
            }
        }
    }

    return doc;
}

void tristan::json::JsonDoc::setAsArray() noexcept { m_array = true; }

void tristan::json::JsonDoc::addChild(std::shared_ptr<JsonElement> element) {
    if (not m_object and not m_array) {
        if (element->m_key) {
            m_object = true;
        } else {
            m_array = true;
        }
    }
    if (m_object) {
        if (not element->m_key) {
            throw std::invalid_argument("Trying to add child with key not set into json object");
        }
        if (m_children.empty()) {
            m_children.emplace_back(std::move(element));
            return;
        }
        for (auto &child: m_children) {
            if (child->m_key == element->m_key) {
                child = element;
                return;
            }
        }
    } else if (m_array) {
        if (element->m_key) {
            throw std::invalid_argument("Trying to add child with key into json array");
        }
    }
    m_children.emplace_back(std::move(element));
}

void tristan::json::JsonDoc::beautifyOutput(bool value) noexcept { m_beautify_output = value; }

auto tristan::json::JsonDoc::getChildByName(const std::string &name,
                                            bool recursively) const -> std::shared_ptr<JsonElement> {

    if (m_children.empty()) {
        return {};
    }
    if (not recursively) {
        if (m_array) {
            return {};
        }
        for (auto child: m_children) {
            if (child->m_key == name) {
                return child;
            }
        }
    } else {
        std::queue<std::shared_ptr<JsonElement> > search_queue;

        for (auto child: m_children) {
            if (m_object) {
                if (child->m_key == name) {
                    return child;
                }
                if (child->m_object or child->m_array) {
                    search_queue.push(child);
                }
            } else {
                if (child->m_object or child->m_array) {
                    search_queue.push(std::move(child));
                }
            }
        }
        while (not search_queue.empty()) {
            auto child = search_queue.front();
            search_queue.pop();
            if (std::holds_alternative<Children>(child->m_value)) {
                const auto &sub_children = std::get<Children>(child->m_value);
                for (auto sub_child: sub_children) {
                    if (sub_child->key() == name) {
                        return sub_child;
                    }
                    if (sub_child->m_object or sub_child->m_array) {
                        search_queue.push(std::move(sub_child));
                    }
                }
            }
        }
    }

    return {};
}

auto tristan::json::JsonDoc::toString() const -> std::string {
    std::string json;

    if (m_object) {
        json += '{';
    } else {
        json += '[';
    }

    for (const auto &child: m_children) {
        json += child->print();
        if (child.get() == (m_children.end() - 1)->get()) {
            break;
        }
        json += ',';
    }

    if (m_object) {
        json += '}';
    } else {
        json += ']';
    }
    if (m_beautify_output){
        json = JsonDoc::beautifyOutput(json);
    }
    return json;
}

auto tristan::json::JsonDoc::error() const noexcept -> const tristan::json::JsonError & { return m_error; }

auto tristan::json::JsonDoc::encodeStringValue(std::string string_value) -> std::string {
    auto result(std::move(string_value));
    for (auto iter = result.begin(); iter != result.end();) {
        if ((*iter == '\"' or *iter == '\\' or *iter == '/') and (iter == result.begin() or *(iter - 1) != '\\')) {
            iter = result.insert(iter, '\\');
            ++iter;
        }
        ++iter;
    }
    return result;
}

auto tristan::json::JsonDoc::beautifyOutput(const std::string& json_doc, uint8_t indent) -> std::string {
    std::string result;
    result.reserve(json_doc.size());
    uint8_t level = 0;
    bool inside_string = false;
    for (auto character = json_doc.begin(); character != json_doc.end(); ++character){
        if (*character == '\"' and *(character - 1) != '\\'){
            inside_string = !inside_string;
        }
        if (inside_string){
            result += *character;
            continue;
        }
        if (*character == '{' or *character == '[' or *character == ','){
            result += *character;
            result += '\n';
            if (*character == '{' or *character == '[') {
                ++level;
            }
            for (int i = 0; i < level; ++i) {
                for (int j = 0; j < indent; ++j){
                    result += ' ';
                }
            }
        } else if (*character == '}' or *character == ']'){
            result += '\n';
            --level;
            for (int i = 0; i < level; ++i) {
                for (int j = 0; j < indent; ++j){
                    result += ' ';
                }
            }
            result += *character;
        } else{
            result += *character;
        }
    }
    return result;
}
