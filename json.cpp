#include <iostream>
#include <sstream>
#include <string>
#include <stdexcept>
#include <unordered_map>

//debug stuff
#include <typeinfo>

/* Helper macros */
#define WIDEN(STRING)   \
    L ## STRING

#define EXPAND_MACRO_WIDEN_IMPL(MACRO)  \
    WIDEN(#MACRO)

#define EXPAND_MACRO_WIDEN(MACRO) \
    EXPAND_MACRO_WIDEN_IMPL(MACRO)

/* Don't put this anywhere in your class definition unless you like reading horrific template
   errors */
#define JSON_VAR_DECORATOR \
    static constexpr JSON::__json_var

/* constructor macros */
#define JSON_CLASS_IMPL(CLASS_NAME, ...)                                                        \
    class CLASS_NAME {                                                                          \
        /* inject the stuff that we need to function. It shouldn't be public */                 \
        typedef CLASS_NAME __JSONClass;                                                         \
        /* This needs to be able to hit functions even if private */                            \
        template <class ThisClass, const wchar_t *const *classString, size_t offset>            \
        friend struct JSON::JSONVartoJSONFnInvoker;                                             \
                                                                                                \
        template <class ThisClass>                                                              \
        friend struct JSON::JSONTypetoJSONFnInvoker;                                            \
                                                                                                \
        /* This gives us the string for the class that we parse at compile time */              \
        static constexpr const wchar_t* __##CLASS_NAME = EXPAND_MACRO_WIDEN(__VA_ARGS__);       \
        static constexpr const wchar_t* __##CLASS_NAME##_str = WIDEN(#CLASS_NAME);              \
                                                                                                \
        /* This is the base template from which all variables will make their specializations   \
           uniqueID is generated by the __COUNTER__ macro and identifies each member of the     \
           class                                                                                \
         */                                                                                     \
        template<class ThisClass, unsigned int uniqueID>                                        \
        static void VarToJSON(const ThisClass&,                                                 \
                              JSON::JSONDataMap&,                                               \
                              const JSON::VarToJSONIdentifier<uniqueID>&);                      \
                                                                                                \
    public:                                                                                     \
        std::wstring ToJSON() const {                                                           \
            JSON::JSONDataMap jsonData;                                                         \
            std::wcout << L"Type invoke get" << std::endl;                                      \
            /* VarToJSON jsonify all members */                                                 \
            return L"";                                                                         \
        }                                                                                       \
                                                                                                \
    private:                                                                                    \
        /* Here we actually make the rest of the class for them */                              \
        __VA_ARGS__                                                                             \
    }

/* Do indirection so macros in the class body get invoked properly */
#define JSON_CLASS(CLASS_NAME, ...)  \
    JSON_CLASS_IMPL(CLASS_NAME, __VA_ARGS__)

#define JSON_VAR_IMPL(TYPE, VARNAME, JSONKEY, KEY, ...)                                     \
    /* This is the thing our tokenizer looks for, KEY gives it the ID for the               \
       specialization.                                                                      \
     */                                                                                     \
    JSON_VAR_DECORATOR KEY = EXPAND_MACRO_WIDEN(KEY);                                       \
                                                                                            \
    /* This function does the actual work */                                                \
    template<class ThisClass>                                                               \
    static void VarToJSON(const ThisClass& classFrom,                                       \
                          JSON::JSONDataMap& jsonData,                                      \
                          const JSON::VarToJSONIdentifier<                                  \
                                    JSON::JSONStringHasher<&KEY>::Hash()>&& key) {          \
        jsonData.insert(JSON::JSONDataType(JSONKEY,                                         \
                                           JSON::JSONTypetoJSONFnInvoker<TYPE>              \
                                                ::Invoke(classFrom.VARNAME)));              \
                                                                                            \
        std::wcout << typeid(__JSONClass).name() <<                                         \
                    WIDEN(#VARNAME) L" serialized to: " << jsonData[JSONKEY] << std::endl;  \
                                                                                            \
    }                                                                                       \
    __VA_ARGS__ TYPE VARNAME

/* This makes sure our JSON key (KEY_ARG) for hashing and searching are the same */
#define JSON_VAR_HELPER0(TYPE, VARNAME, JSONKEY, KEY_ARG, ...)   \
    JSON_VAR_IMPL(TYPE, VARNAME, JSONKEY, __##VARNAME##KEY_ARG, __VA_ARGS__)

/* Indirection here so that KEY_ARG (__COUNTER__) will become a number */
#define JSON_VAR_HELPER(TYPE, VARNAME, JSONKEY, KEY_ARG, ...)   \
    JSON_VAR_HELPER0(TYPE, VARNAME, JSONKEY, KEY_ARG, __VA_ARGS__)

/* Make a variable, the varargs will become the type qualifiers if specified */
#define JSON_VAR(TYPE, VARNAME, JSONKEY, ...)    \
    JSON_VAR_HELPER(TYPE, VARNAME, JSONKEY, __COUNTER__, __VA_ARGS__)

/* holy shit so many templates */
namespace JSON {
    /* Important constants used in parsing */

    //The decorator
    //  - typedef lets us use it in class defs and still find the unique string
    typedef const wchar_t* __json_var;

    typedef std::unordered_map<std::wstring, std::wstring> JSONDataMap;
    typedef std::pair<std::wstring, std::wstring> JSONDataType;

    //This is used to match against when searching the stringy class.
    // It's defined in terms of what we use to make the identifier, so hopefully
    //  it doesn't break everytime we change things
    constexpr const wchar_t* DECORATOR_STR = EXPAND_MACRO_WIDEN(JSON_VAR_DECORATOR);
    constexpr const size_t DECORATOR_STR_LEN = sizeof(EXPAND_MACRO_WIDEN(JSON_VAR_DECORATOR)) / sizeof(wchar_t);

    /* Helpers for the template programs */
    template<int uniqueID>
    struct VarToJSONIdentifier {
        const static int help = uniqueID;
    };

    template<wchar_t testChar,
             wchar_t candidateChar>
    struct JSONTokenTester {
        static constexpr bool Equal() {
            return testChar == candidateChar;
        }

        static constexpr bool NotEqual() {
            return testChar != candidateChar;
        }
    };

    template<class Type>
    struct JSONPODParser {
        static Type FromJSON(const std::wstring& json) {
            std::wstringstream wss;
            wss << json;

            Type value;
            wss >> value;

            if(wss.fail()) {
                throw std::invalid_argument("bad json value for key"); 
            }

            return value;
        }

        static std::wstring ToJSON(const Type& value) {
            // return std::to_wstring(value);
            //Cygwin won't let me use the above...
            std::wstringstream wss;
            wss << value;
            return wss.str();
        }
    };

////////////////////////////////////////////////////////////////////////////////
// JSONIsNullOrWhitespace implementation
////
    template<wchar_t testChar>
    struct JSONIsNullOrWhitespace {
        static constexpr bool Check() {
            /* The specializations do the check for valid chars
               so if it hits this it can't be
             */
            return false;
        }
    };

    template<>
    struct JSONIsNullOrWhitespace<L' '> {
        static constexpr bool Check() {
            return true;
        }
    };

    template<>
    struct JSONIsNullOrWhitespace<L'\t'> {
        static constexpr bool Check() {
            return true;
        }
    };

    template<>
    struct JSONIsNullOrWhitespace<L'\0'> {
        static constexpr bool Check() {
            return true;
        }
    };

////////////////////////////////////////////////////////////////////////////////
// JSONStringHasher implementation
/////
    template<const wchar_t *const *string,
             size_t offset = 0,
             unsigned int hashValue = 0,
             bool endOfString = false>
    struct JSONStringHasher {
        static constexpr unsigned int Hash() {
            return JSONStringHasher<string,
                                    offset + 1,
                                    hashValue * 131 + string[0][offset],
                                    JSONIsNullOrWhitespace<string[0][offset + 1]>::Check()
                                   >::Hash();
        }
    };

    template<const wchar_t *const *string,
             size_t offset,
             unsigned int hashValue>
    struct JSONStringHasher<string,
                            offset,
                            hashValue,
                            /* endOfString */ true> {
        static constexpr unsigned int Hash() {
            return hashValue;
        }
    };

////////////////////////////////////////////////////////////////////////////////
// JSONTokenMatcherPart implementation
////
    //This case handles successful comparisons and recurses
    template<const wchar_t* const* candidateWord,
             const wchar_t* const* testWord,
             size_t candidateOffset,
             size_t testOffset = 0,
             bool matchFailed = false,
             bool endCandidateWord = false,
             bool endTestWord = false>
    struct JSONTokenMatcherPart { 
        static constexpr bool MatchToken() {
            return JSONTokenMatcherPart<candidateWord,
                                        testWord,
                                        candidateOffset + 1,
                                        testOffset + 1,
                                        JSONTokenTester<testWord[0][testOffset],
                                                        candidateWord[0][candidateOffset]
                                                       >::NotEqual(),
                                        JSONTokenTester<candidateWord[0][candidateOffset + 1],
                                                        L'\0'
                                                       >::Equal(),
                                        JSONTokenTester<testWord[0][testOffset + 1],
                                                        L'\0'
                                                       >::Equal()
                                       >::MatchToken();
        }
    };

    //This is the fail case, no need to continue
    template<const wchar_t* const* candidateWord,
             const wchar_t* const* testWord,
             size_t candidateOffset,
             size_t testOffset,
             bool endCandidateWord,
             bool endTestWord>
    struct JSONTokenMatcherPart<candidateWord,
                                testWord,
                                candidateOffset,
                                testOffset,
                                /* matchFailed */ true,
                                endCandidateWord,
                                endTestWord> {
        static constexpr bool MatchToken() {
            return false;
        }
    };

    //The string isn't long enough
    template<const wchar_t* const* candidateWord,
             const wchar_t* const* testWord,
             size_t candidateOffset,
             size_t testOffset,
             bool matchFailed,
             bool endTestWord>
    struct JSONTokenMatcherPart<candidateWord,
                                testWord,
                                candidateOffset,
                                testOffset,
                                matchFailed,
                                /* endCandidateWord */ true,
                                endTestWord> {
        static constexpr bool MatchToken() {
            return false;
        }
    };

    //Also not long enough, also it didn't match
    template<const wchar_t* const* candidateWord,
             const wchar_t* const* testWord,
             size_t candidateOffset,
             size_t testOffset,
             bool endTestWord>
    struct JSONTokenMatcherPart<candidateWord,
                                testWord,
                                candidateOffset,
                                testOffset,
                                /* matchFailed */ true,
                                /* endCandidateWord */ true,
                                endTestWord> {
        static constexpr bool MatchToken() {
            return false;
        }
    };

    //YAY! A Match!
    template<const wchar_t* const* candidateWord,
             const wchar_t* const* testWord,
             size_t candidateOffset,
             size_t testOffset,
             bool matchFailed,
             bool endCandidateWord>
    struct JSONTokenMatcherPart<candidateWord,
                                testWord,
                                candidateOffset,
                                testOffset,
                                matchFailed,
                                endCandidateWord,
                                /* endTestWord */ true> {
        static constexpr bool MatchToken() {
            return true;
        }
    };

    //Also a match, but full length
    template<const wchar_t* const* candidateWord,
             const wchar_t* const* testWord,
             size_t candidateOffset,
             size_t testOffset,
             bool matchFailed>
    struct JSONTokenMatcherPart<candidateWord,
                                testWord,
                                candidateOffset,
                                testOffset,
                                matchFailed,
                                /* endCandidateWord */ true,
                                /* endTestWord */ true> {
        static constexpr bool MatchToken() {
            return true;
        }
    };

////////////////////////////////////////////////////////////////////////////////
// JSONTagMatcher implementation
////
    //This class checks if a string at an offset matches the decorator
    template<const wchar_t *const *classInfo,
             unsigned int offset>
    struct JSONTagMatcher {
        static constexpr bool MatchJSONVarTag() {
            return JSONTokenMatcherPart<classInfo,
                                        &DECORATOR_STR,
                                        offset
                                       >::MatchToken();
        }
    };

////////////////////////////////////////////////////////////////////////////////
// JSONClassParserTokenFinder implementation
////
    //This recursively searches the input string until it finds a matching token
    //  or fails to find one
    template<const wchar_t *const *classInfo,
             unsigned int offset,
             bool foundToken,
             bool endOfInput>
    struct JSONClassParserTokenFinder {
        static constexpr const size_t FindJSONToken() {
            return JSONClassParserTokenFinder<classInfo,
                                              offset + 1,
                                              JSONTagMatcher<classInfo,
                                                             offset
                                                            >::MatchJSONVarTag(),
                                              classInfo[0][offset + 1] != L'\0'
                                             >::FindJSONToken();
        }
    };

    //This is the termination case when a recursive search finds a token
    template<const wchar_t *const *classInfo,
             unsigned int offset,
             bool endOfInput>
    struct JSONClassParserTokenFinder<classInfo,
                                      offset,
                                      /* foundToken*/ true,
                                      endOfInput> {
        static constexpr size_t FindJSONToken() {
            //The offset is increased before we know if we found the end, so we
            //  need to decrease it here to get the real offset
            return offset - 1;
        }
    };

    //Also a termination case
    template<const wchar_t *const *classInfo,
             unsigned int offset>
    struct JSONClassParserTokenFinder<classInfo,
                                      offset,
                                      /* foundToken*/ true,
                                      /* endOfInput */ false> {
        static constexpr size_t FindJSONToken() {
            //The offset is increased before we know if we found the end, so we
            //  need to decrease it here to get the real offset
            return offset - 1;
        }
    };

    //This handles the recursive search failing to locate a token 
    template<const wchar_t *const *classInfo,
             unsigned int offset,
             bool foundToken>
    struct JSONClassParserTokenFinder<classInfo,
                                      offset,
                                      foundToken,
                                      /* endOfInput */ false> {
        static constexpr const size_t FindJSONToken() {
            return offset - 1;
        }
    };

////////////////////////////////////////////////////////////////////////////////
// JSONVartoJSONFnInvoker implementation
////
    template<class classOn,
             const wchar_t *const *classString,
             size_t offset>
    struct JSONVartoJSONFnInvoker {
        static void Invoke(classOn& classFrom, JSONDataMap& jsonData) {
            constexpr unsigned int id = JSONStringHasher<classString, offset>::Hash();
            classOn::VarToJSON(classFrom, jsonData, JSON::VarToJSONIdentifier<id>());
        }
    };

////////////////////////////////////////////////////////////////////////////////
// JSONTypetoJSONFnInvoker implementation
////
    template<class classOn>
    struct JSONTypetoJSONFnInvoker {
        static std::wstring Invoke(const classOn& classFrom) {
            return classFrom.FromJSON();
        }
    };

    template<>
    struct JSONTypetoJSONFnInvoker<int> {
        static std::wstring Invoke(const int& classFrom) {
            return JSONPODParser<int>::ToJSON(classFrom);
        }
    };

    template<>
    struct JSONTypetoJSONFnInvoker<long> {
        static std::wstring Invoke(const long& classFrom) {
            return JSONPODParser<long>::ToJSON(classFrom);
        }
    };

    template<>
    struct JSONTypetoJSONFnInvoker<long long> {
        static std::wstring Invoke(const long long& classFrom) {
            return JSONPODParser<long long>::ToJSON(classFrom);
        }
    };

    template<>
    struct JSONTypetoJSONFnInvoker<unsigned int> {
        static std::wstring Invoke(const unsigned int& classFrom) {
            return JSONPODParser<unsigned int>::ToJSON(classFrom);
        }
    };

    template<>
    struct JSONTypetoJSONFnInvoker<unsigned long> {
        static std::wstring Invoke(const unsigned long& classFrom) {
            return JSONPODParser<unsigned long>::ToJSON(classFrom);
        }
    };

    template<>
    struct JSONTypetoJSONFnInvoker<unsigned long long> {
        static std::wstring Invoke(const unsigned long long& classFrom) {
            return JSONPODParser<unsigned long long>::ToJSON(classFrom);
        }
    };

    template<>
    struct JSONTypetoJSONFnInvoker<float> {
        static std::wstring Invoke(const float& classFrom) {
            return JSONPODParser<float>::ToJSON(classFrom);
        }
    };

    template<>
    struct JSONTypetoJSONFnInvoker<double> {
        static std::wstring Invoke(const double& classFrom) {
            return JSONPODParser<double>::ToJSON(classFrom);
        }
    };

    template<>
    struct JSONTypetoJSONFnInvoker<long double> {
        static std::wstring Invoke(const long double& classFrom) {
            return JSONPODParser<long double> ::ToJSON(classFrom);
        }
    };

////////////////////////////////////////////////////////////////////////////////
// JSONClassParser implementation
////
    template<const wchar_t *const *classInfo>
    struct JSONClassParser {
        static constexpr const size_t FindNextJSONToken() {
            return JSONClassParserTokenFinder<classInfo, 0, false, true>::FindJSONToken();
        }
    };

    constexpr const wchar_t* test = L"10";

    template<class classFor,
             const wchar_t *const *classInfo>
    struct JSONParser {
        static void FromJSON() {
            // bool t = JSONTagMatcher<classInfo, 0, true>::MatchJSONVarTag();
            // std::wcout << t << std::endl;
            std::wcout << L"Class info:" << std::endl;
            std::wcout << *classInfo << std::endl << std::endl;

            std::wcout << L"Decorator:" << std::endl;
            std::wcout << DECORATOR_STR << std::endl << std::endl;

            std::wcout << L"Result:" << std::endl;
            constexpr size_t first_pos = JSONClassParser<classInfo>::FindNextJSONToken();
            std::wcout << &(*classInfo)[first_pos] << std::endl << std::endl;

            std::wcout << L"Test var ID parsing:" << std::endl;
            std::wcout << L"offset: " << first_pos + DECORATOR_STR_LEN << std::endl;
            std::wcout << L"String: " << &(*classInfo)[first_pos + DECORATOR_STR_LEN] << std::endl;
            unsigned int varValue = JSONStringHasher<classInfo, first_pos + DECORATOR_STR_LEN>::Hash();
            std::wcout << L"Hashed out: " << varValue << std::endl << std::endl;

            std::wcout << L"Test Fn execution:" << std::endl;
            JSONDataMap jsonData;
            classFor a;
            a.__json = 20;
            JSONVartoJSONFnInvoker<classFor, classInfo, first_pos + DECORATOR_STR_LEN>::Invoke(a, jsonData);
            std::wcout << L"#Items in map: " << jsonData.size() << std::endl;

            a.__json = 10;
            a.ToJSON();
        }
    };
}

JSON_CLASS(Test, 
public:
     /* variable type, variable name, json key */
     JSON_VAR(int, __json, L"test");
);

int main() {

    JSON::JSONParser<Test, &Test::__Test>::FromJSON();

    return 0;
}
