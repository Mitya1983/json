    #ifndef JSON_HPP
#define JSON_HPP

#include <optional>
#include <variant>
#include <string>
#include <vector>
#include <memory>
#include <ostream>
#include <sstream>


namespace tristan::json {

    class JsonElement;

    using Children = std::vector< std::shared_ptr< JsonElement > >;

    /**
     * \brief Used to indicate type of JsonElement for \<explicit JsonElement(std::string string, ArgumentType argument_type = ArgumentType::KEY)\> constructor
     */
    enum class ArgumentType : uint8_t {
        KEY,
        VALUE
    };

    class JsonElement {
        friend class JsonDoc;
    public:
        /**
         * \brief  Default constructor
         */
        JsonElement();

        /**
         * \overload
         * \brief Creates object taking into account the second argument.
         * If ArgumentType::KEY is used then JsonElement will be initiated as key:value pair
         * If ArgumentType::VALUE is used then JsonElement will initiated keyless, aka for use in a json arrays
         * \param key std::string
         */
        explicit JsonElement(std::string string, ArgumentType argument_type = ArgumentType::KEY);
        /**
         * \overload
         * \brief Constructor
         * \param key std::string
         * \param value std::string
         */
        JsonElement(std::string key, std::string value);
        /**
         * \overload
         * \brief Constructor
         * \param key std::string
         * \param value double
         */
        JsonElement(std::string key, double value);
        /**
         * \overload
         * \brief Creates object which holds key as std::nullopt and thus may be used in array
         * \param value
         */
        explicit JsonElement(double value);
        /**
         * \overload
         * \brief Constructor
         * \param key std::string
         * \param value int64_t
         */
        JsonElement(std::string key, int64_t value);
        /**
         * \overload
         * \brief Creates object which holds key as std::nullopt and thus may be used in array
         * \param value
         */
        explicit JsonElement(int64_t value);
        /**
         * \overload
         * \brief Constructor
         * \param key std::string
         * \param value bool
         */
        JsonElement(std::string key, bool value);
        /**
         * \overload
         * \brief Creates object which holds key as std::nullopt and thus may be used in array
         * \param value
         */
        explicit JsonElement(bool value);
        /**
         * \brief Deleted copy constructor
         */
        JsonElement(const JsonElement&) = delete;
        /**
         * \brief Move constructor
         */
        JsonElement(JsonElement&&) = default;
        /**
         * \brief Deleted copy assignment operator
         */
        JsonElement& operator=(const JsonElement&) = delete;
        /**
         * \brief Move assignment operator
         * \return JsonElement&
         */
        JsonElement& operator=(JsonElement&&) = default;
        /**
         * \brief Destructor
         */
        ~JsonElement() = default;
        /**
         * \brief Adds element as a child
         * \param element JsonElement&&
         */
        void addElement(std::shared_ptr< JsonElement > element);
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
         * \brief Return key if JsonElement was instantiated as key:value pair or std::nullopt otherwise
         * \note Empty string is considered as allowed key
         * \return const std::optional<std::string>&
         */
        [[nodiscard]] auto key() const -> const std::optional<std::string>&;

        /**
         * \brief Returns list of children objects as an array if object is set as being an array.
         * \attention Element should be checked for emptiness using \<isNull()\> before trying to get an array
         * \return const std::vector< std::shared_ptr< JsonElement > >&
         * \throws std::runtime_error if json element is not set to be array
         * \throws std::bad_variant_access if json element is null
         */
        [[nodiscard]] auto toArray() const -> const Children&;
        /**
         * \brief Returns object as a string.
         * \attention Element should be checked for emptiness using \<isNull()\>  or \<isString()\> before trying to get a value
         * \return std::string
         * \throws std::bad_variant_access if value is not string
         */
        [[nodiscard]] auto toString() const -> const std::string&;
        /**
         * \brief Returns object as a double.
         * \attention Element should be checked for emptiness using \<isNull()\> or \<isDouble()\> before trying to get a value
         * \return double
         * \throws std::bad_variant_access if value is not double
         */
        [[nodiscard]] auto toDouble() const -> double;
        /**
         * \brief Returns object as an int64_t.
         * \attention Element should be checked for emptiness using \<isNull()\> or \<isInt()\> before trying to get a value
         * \return int64_t
         * \throws std::bad_variant_access if value is not int64_t
         */
        [[nodiscard]] auto toInt() const -> int64_t;
        /**
         * \brief Returns object as a bool.
         * \attention Element should be checked for emptiness using \<isNull()\> or \<isBool()\> before trying to get a value
         * \return bool
         * \throws std::bad_variant_access if value is not bool
         */
        [[nodiscard]] auto toBool() const -> bool;

        /**
         * \brief Checks if instance is an object (aka holds child elements and is not an array)
         * May still be null if object is empty
         * \return bool
         */
        [[nodiscard]] auto isObject() const -> bool;
        /**
         * \brief Checks if instance is an array
         * May still be null if array is empty
         * \return bool
         */
        [[nodiscard]] auto isArray() const -> bool;
        /**
         * \brief Checks if json instance holds a value as a string
         * \note Empty string is considered as null value.
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

    private:
        [[nodiscard]] auto print() -> std::string;

        std::optional<std::string> m_key;
        std::variant< std::monostate, std::string, Children, double, int64_t, bool > m_value;
        bool m_object;
        bool m_array;
    };

    struct JsonError {
        JsonError();
        JsonError(uint64_t, char);
        uint64_t place = std::string::npos;
        std::string description;
        explicit operator bool() const;
    };

    class JsonDoc {

        friend auto operator<<(std::ostream& output, const JsonDoc& json_doc) -> std::ostream&;
        friend auto operator<<(std::stringstream& output, const JsonDoc& json_doc) -> std::stringstream&;

        /**
         * \private
         * \brief Default constructor
         */
        JsonDoc();
    public:

        auto static createJsonDocument() -> std::shared_ptr<JsonDoc>;

        /**
         * \overload
         * \brief Parses json data and returns parsed JsonDoc.
         * Checks during parsing the validity of json data and sets JsonError accordingly
         * It is a good practice to check if JsonError::place is equal std::string:npos which means json does not contain any errors
         * \param json_document const std::string&
         * \return std::shared_ptr<JsonDoc>
         */
        static auto createJsonDocument(const std::string& json_document) -> std::shared_ptr< JsonDoc >;
        /**
         * \brief Deleted copy constructor
         */
        JsonDoc(const JsonDoc&) = delete;
        /**
         * \brief Default move constructor
         */
        JsonDoc(JsonDoc&&) = default;
        /**
         * \brief Deleted copy assignment operator
         */
        JsonDoc& operator=(const JsonDoc&) = delete;

        /**
         * \brief Default move assignment operator
         * \return JsonDoc&
         */
        JsonDoc& operator=(JsonDoc&&) = default;

        ~JsonDoc() = default;

        /**
         * \brief Sets json document to be an array
         */
        void setAsArray() noexcept;

        /**
         * \brief Adds child to json document
         * \param element std::shared_ptr< JsonElement >
         * \throws std::invalid_argument if document is not an array and child key is not set
         */
        void addChild(std::shared_ptr< JsonElement > element);

        /**
         * \brief Sets if beauty output should be provided by operator << and docToString()
         * \param value bool. Default is true
         */
        void beautifyOutput(bool value = true) noexcept;

        /**
         * \brief Searches through the children.
         * If search is not recursive and JsonDoc is set to be an array, then search will always return empty object as json arrays can not contain key:value pairs
         * \param name const std::string&
         * \param recursively bool. Default value is false
         * \return std::shared_ptr< JsonElement > which is empty if child was not found
         * \note Recursive search should be used with caution as in case of multiple elements only the first one will be returned.
         */
        [[nodiscard]] auto getChildByName(const std::string& name, bool recursively = false) const -> std::shared_ptr< JsonElement >;

        /**
         * \brief Provides printable json document
         * \return std::string
         */
        [[nodiscard]] auto toString() const -> std::string;

        /**
         * \brief Returns error
         * \return const JsonError&
         */
        [[nodiscard]] auto error() const noexcept -> const JsonError&;

        /**
         * \brief Encodes the string value according to JSON standard, aka adds escape character
         * \param string_value
         * \return std::string
         */
        [[nodiscard]] static auto encodeStringValue(std::string string_value) -> std::string;

        /**
         * \brief Beatifies the resulting json doc. This function is invoked is beatifyOutput is set to true.
         * \param json_doc const std::string&
         * \param indent uint8_t. Number of spaces to use as indent
         * \return td::string
         */
        [[nodiscard]] static auto beautifyOutput(const std::string& json_doc, uint8_t indent = 2) -> std::string;

    protected:
    private:
        JsonError m_error;

        std::vector< std::shared_ptr< JsonElement > > m_children;

        bool m_object;
        bool m_array;
        bool m_beautify_output;
    };

    auto operator<<(std::ostream& output, const JsonDoc& json_doc) -> std::ostream&;
    auto operator<<(std::stringstream& output, const JsonDoc& json_doc) -> std::stringstream&;

}  // namespace tristan::json

#endif  // JSON_HPP
