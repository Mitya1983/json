#ifndef JSON_HPP
#define JSON_HPP

#include <variant>
#include <string>
#include <vector>
#include <memory>
#include <ostream>
#include <sstream>

namespace tristan::json {

    class JsonObject {

        friend auto operator<<(std::ostream& output, const JsonObject& JsonObject) -> std::ostream&;
        friend auto operator<<(std::stringstream& output, const JsonObject& JsonObject) -> std::stringstream&;

    public:
        /**
         * \brief Default constructor
         * Object become the root, not array, will not beautify output and value is empty
         */
        JsonObject();
        /**
         * \overload
         * \brief Constructor
         * Parses json data provided as a string
         * \param jsonData const std::string&
         */
        explicit JsonObject(const std::string& jsonData);
        /**
         * \overload
         * \brief Constructor
         * \param key std::string
         */
        JsonObject(std::string key, std::monostate);
        /**
         * \overload
         * \brief Constructor
         * \param key std::string
         * \param value std::string
         */
        JsonObject(std::string key, std::string value);
        /**
         * \overload
         * \brief Constructor
         * \param key std::string
         * \param value double
         */
        JsonObject(std::string key, double value);
        /**
         * \overload
         * \brief Constructor
         * \param key std::string
         * \param value int64_t
         */
        JsonObject(std::string key, int64_t value);
        /**
         * \overload
         * \brief Constructor
         * \param key std::string
         * \param value bool
         */
        JsonObject(std::string key, bool value);
        JsonObject(const JsonObject&) = delete;
        /**
         * \brief Move constructor
         */
        JsonObject(JsonObject&&) = default;
        JsonObject& operator=(const JsonObject&) = delete;
        /**
         * \brief Move assignment operator
         * \return JsonObject&
         */
        JsonObject& operator=(JsonObject&&) = default;
        /**
         * \brief Destructor
         */
        ~JsonObject() = default;

        /**
         * \brief Adds object as a child
         * \param object JsonObject&&
         */
        void addObject(JsonObject&& object);

        /**
         * \brief Sets object as an array object which allows to have multiple elements with the same key
         * \param value bool. Default is true.
         */
        void setIsArray(bool value = true) { m_array = value; }

        /**
         * \brief Sets key
         * \param key std::string
         */
        void setKey(std::string key);
        /**
         * \brief Sets value
         * \param value std::string
         */
        void setValue(std::string value);
        /**
         * \brief Sets value
         * \param value double
         */
        void setValue(double value);
        /**
         * \brief Sets value
         * \param value int64_t
         */
        void setValue(int64_t value);
        /**
         * \brief Sets value
         * \param value bool
         */
        void setValue(bool value);
        /**
         * \brief Sets if beauty output should be provided by operator << and docToString()
         * \param value bool. Default is true
         */
        void setBeautify(bool value = true);

        /**
         * \brief Searches through the children.
         * \param name const std::string&
         * \return std::shared_ptr< JsonObject >
         */
        [[nodiscard]] auto getChildByName(const std::string& name) const -> std::shared_ptr< JsonObject >;

        /**
         * \brief Returns list of children objects as an array if object is set as being an array.
         * \return const std::vector< std::shared_ptr< JsonObject > >&
         * \throws std::runtime_error
         */
        [[nodiscard]] auto toArray() const -> const std::vector< std::shared_ptr< JsonObject > >&;
        /**
         * \brief Returns object as a string.
         * \return std::string
         * \throws std::bad_variant_access
         */
        [[nodiscard]] auto toString() const -> std::string;
        /**
         * \brief Returns object as a double.
         * \return double
         * \throws std::bad_variant_access
         */
        [[nodiscard]] auto toDouble() const -> double;
        /**
         * \brief Returns object as an int64_t.
         * \return int64_t
         * \throws std::bad_variant_access
         */
        [[nodiscard]] auto toInt() const -> int64_t;
        /**
         * \brief Returns object as a bool.
         * \return bool
         * \throws std::bad_variant_access
         */
        [[nodiscard]] auto toBool() const -> bool;
        /**
         * \brief Provides printable json document
         * \return std::string
         */
        [[nodiscard]] auto docToString() const -> std::string;

        /**
         * \brief Checks if instance is an object (aka holds child elements and is not an array)
         * \return bool
         */
        [[nodiscard]] auto isObject() const -> bool;
        /**
         * \brief Checks if instance is an array
         * \return bool
         */
        [[nodiscard]] auto isArray() const -> bool;
        /**
         * \brief Checks if json instance holds a value as a string
         * \return bool
         */
        [[nodiscard]] auto isString() const -> bool;
        /**
         * \brief Checks if json instance holds a value as a double
         * \return bool
         */
        [[nodiscard]] auto isDouble() const -> bool;
        /**
         * \brief Checks if json instance holds a value as an int64_t
         * \return bool
         */
        [[nodiscard]] auto isInt() const -> bool;
        /**
         * \brief Checks if json instance holds a value as a bool
         * \return bool
         */
        [[nodiscard]] auto isBool() const -> bool;
        /**
         * \brief Checks if json instance holds a value as an empty value
         * \return bool
         */
        [[nodiscard]] auto isNull() const -> bool;

    protected:
        void _addChildrenFromObject(const std::string& jsonData);
        void _addChildrenFromArray(const std::string& jsonData);

        [[nodiscard]] auto _toStream() const -> std::string;
        [[nodiscard]] auto _toStream_b(uint8_t level = 0) const -> std::string;

    private:
        std::string m_key;
        std::variant< std::monostate, std::unique_ptr< std::string >, double, int64_t, bool > m_value;
        std::vector< std::shared_ptr< JsonObject > > m_children;
        bool m_root;
        bool m_array;
        bool m_beautifyOutput;
    };

    auto operator<<(std::ostream& output, const JsonObject& jsonObject) -> std::ostream&;
    auto operator<<(std::stringstream& output, const JsonObject& jsonObject) -> std::stringstream&;

}  // namespace tristan::json

#endif  // JSON_HPP
