#ifndef MT_JSON_HPP
#define MT_JSON_HPP

#include <variant>
#include <string>
#include <vector>
#include <memory>

namespace mt::json {

    class JsonError final : public std::exception {
      public:
        explicit JsonError(uint64_t p_position, char p_unexpected_symbol);

        [[nodiscard]] auto what() const noexcept -> const char * override;
        [[nodiscard]] auto position() const noexcept -> uint64_t;

      private:
        std::string m_description;
        uint64_t m_position;
    };

    class JsonValue {
        friend class Json;
      public:
        JsonValue() = default;

        explicit JsonValue(std::string p_value);
        explicit JsonValue(const char *p_value);
        explicit JsonValue(double p_value);
        explicit JsonValue(int64_t p_value);
        explicit JsonValue(bool p_value);

        JsonValue(const JsonValue&) = delete;
        JsonValue(JsonValue&&) = default;
        JsonValue& operator=(const JsonValue&) = delete;
        JsonValue& operator=(JsonValue&&) = default;


        [[nodiscard]] auto toString() const -> std::string;
        [[nodiscard]] auto toDouble() const -> double;
        [[nodiscard]] auto toInt() const -> int64_t;
        [[nodiscard]] auto toBool() const -> bool;
        [[nodiscard]] auto isString() const -> bool;
        [[nodiscard]] auto isDouble() const -> bool;
        [[nodiscard]] auto isInt() const -> bool;
        [[nodiscard]] auto isBool() const -> bool;
        [[nodiscard]] auto isNull() const -> bool;

        ~JsonValue() = default;

      private:
        [[nodiscard]] static auto fromString(const std::string& p_data, uint64_t& pos) -> JsonValue;
        std::variant< std::monostate, std::string, const char *, double, int64_t, bool> m_value{std::monostate()};
    };

    class Json {

      public:
        Json() = default;
        explicit Json(std::string p_key);
        explicit Json(JsonValue p_value);
        explicit Json(std::string p_key, JsonValue p_value);
        explicit Json(std::string p_key, Json&& p_value);
        explicit Json(std::string p_key, std::shared_ptr<Json> p_value);

        [[nodiscard]] static auto fromString(const std::string& p_json_data) -> Json;

        Json(const Json&) = delete;
        Json(Json&&) = default;
        Json& operator=(const Json&) = delete;
        Json& operator=(Json&&) = default;
        ~Json() = default;

        void addJson(std::shared_ptr< Json > p_json_element);
        void addJson(Json&& p_json_element);
        void setValue(JsonValue p_value);
        void setValue(std::shared_ptr< Json > p_json_element);
        void setValue(Json&& p_json_element);

        [[nodiscard]] auto isNull() const -> bool;
        [[nodiscard]] auto isObject() const -> bool;
        [[nodiscard]] auto isArray() const -> bool;

        [[nodiscard]] auto key() const -> const std::string&;
        [[nodiscard]] auto value() const -> const JsonValue&;
        [[nodiscard]] auto toObject() const -> std::shared_ptr<Json>;
        [[nodiscard]] auto toArray() const -> const std::vector< std::shared_ptr< Json > >&;
        [[nodiscard]] auto getChildByName(const std::string& p_name) const -> std::shared_ptr< Json >;
        [[nodiscard]] auto toString(bool p_beautify = false, uint8_t level = 0, bool in_array = false, const std::string& indent = "  ") const -> std::string;

      private:

        [[nodiscard]] static auto fromString(const std::string& p_data, uint64_t& pos) -> Json;

        std::variant< std::monostate, JsonValue, std::shared_ptr<Json>, std::vector< std::shared_ptr< Json > > > m_value{std::monostate()};
        std::string m_key;
        bool m_object{false};
        bool m_array{false};
    };

}  // namespace mt::utility::json

#endif  // MT_JSON_HPP
