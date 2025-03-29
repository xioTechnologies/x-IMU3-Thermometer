/**
 * @file Json.c
 * @author Seb Madgwick
 * @brief Library for parsing of JSON strings.
 */

//------------------------------------------------------------------------------
// Includes

#include <ctype.h>
#include "Json.h"
#include <stdio.h>
#include <string.h>

//------------------------------------------------------------------------------
// Function declarations

static void SkipWhiteSpace(const char **const json);

static JsonResult CheckType(const char **const json, const JsonType expectedType);

static JsonResult ParseEscapeSequence(const char **const json, char *const destination, size_t *const index);

static JsonResult ParseHexEscapeSequence(const char **const json, char *const destination, size_t *const index);

static void WriteToDestination(char *const destination, size_t *const index, const char character);

static JsonResult ParseValue(const char **const json, const bool print, int *const indent);

static JsonResult ParseObject(const char **const json, const bool print, int *const indent);

static JsonResult ParseArray(const char **const json, const bool print, int *const indent);

//------------------------------------------------------------------------------
// Functions

/**
 * @brief Parses value type. The JSON pointer is not modified.
 * @param json JSON pointer.
 * @param type Type.
 * @return Result.
 */
JsonResult JsonParseType(const char **const json, JsonType *const type) {
    SkipWhiteSpace(json);
    switch (**json) {
        case '"':
            *type = JsonTypeString;
            return JsonResultOk;
        case '-':
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
            *type = JsonTypeNumber;
            return JsonResultOk;
        case '{':
            *type = JsonTypeObject;
            return JsonResultOk;
        case '[':
            *type = JsonTypeArray;
            return JsonResultOk;
        case 't':
        case 'f':
            *type = JsonTypeBoolean;
            return JsonResultOk;
        case 'n':
            *type = JsonTypeNull;
            return JsonResultOk;
        default:
            return JsonResultInvalidSyntax;
    }
}

/**
 * @brief Advances the JSON pointer to the first non-whitespace character.
 * @param json JSON pointer.
 */
static void SkipWhiteSpace(const char **const json) {
    while (true) {
        switch (**json) {
            case ' ':
            case '\t':
            case '\n':
            case '\r':
                (*json)++;
                break;
            default:
                return;
        }
    }
}

/**
 * @brief Checks that the type matches the expected type. The JSON pointer is
 * not modified.
 * @param json JSON pointer.
 * @param expectedType Expected type.
 * @return Result.
 */
static JsonResult CheckType(const char **const json, const JsonType expectedType) {
    JsonType type;
    const JsonResult result = JsonParseType(json, &type);
    if (result != JsonResultOk) {
        return result;
    }
    if (type != expectedType) {
        return JsonResultUnexpectedType;
    }
    return JsonResultOk;
}

/**
 * @brief Parses an object start. The JSON pointer is advanced to the first
 * non-whitespace character after the object start.
 * @param json JSON pointer.
 * @return Result.
 */
JsonResult JsonParseObjectStart(const char **const json) {
    const JsonResult result = CheckType(json, JsonTypeObject);
    if (result != JsonResultOk) {
        return result;
    }
    (*json)++;
    SkipWhiteSpace(json);
    return JsonResultOk;
}

/**
 * @brief Parses an object end. The JSON pointer is advanced to the first
 * character after the object end.
 * @param json JSON pointer.
 * @return Result.
 */
JsonResult JsonParseObjectEnd(const char **const json) {
    SkipWhiteSpace(json);
    if (**json != '}') {
        return JsonResultMissingObjectEnd;
    }
    (*json)++;
    return JsonResultOk;
}

/**
 * @brief Parses an array start. The JSON pointer is advanced to the first
 * non-whitespace character after the object start.
 * @param json JSON pointer.
 * @return Result.
 */
JsonResult JsonParseArrayStart(const char **const json) {
    const JsonResult result = CheckType(json, JsonTypeArray);
    if (result != JsonResultOk) {
        return result;
    }
    (*json)++;
    SkipWhiteSpace(json);
    return JsonResultOk;
}

/**
 * @brief Parses an array end. The JSON pointer is advanced to the first
 * character after the array end.
 * @param json JSON pointer.
 * @return Result.
 */
JsonResult JsonParseArrayEnd(const char **const json) {
    SkipWhiteSpace(json);
    if (**json != ']') {
        return JsonResultMissingArrayEnd;
    }
    (*json)++;
    return JsonResultOk;
}

/**
 * @brief Parses a comma. The JSON pointer is advanced to the first character
 * after the comma.
 * @param json JSON pointer.
 * @return Result.
 */
JsonResult JsonParseComma(const char **const json) {
    SkipWhiteSpace(json);
    if (**json != ',') {
        return JsonResultMissingComma;
    }
    (*json)++;
    return JsonResultOk;
}

/**
 * @brief Parses the key in a JSON object. The JSON pointer is advanced to the
 * character after the colon that separates the key/value pair.
 * @param json JSON pointer.
 * @param destination Destination. NULL if not required.
 * @param destinationSize Destination size.
 * @return Result.
 */
JsonResult JsonParseKey(const char **const json, char *const destination, const size_t destinationSize) {
    // Check type
    if (CheckType(json, JsonTypeString) != JsonResultOk) {
        return JsonResultMissingKey;
    }

    // Parse key
    const JsonResult result = JsonParseString(json, destination, destinationSize, NULL);
    if (result != JsonResultOk) {
        return result;
    }

    // Parse colon
    SkipWhiteSpace(json);
    if (**json != ':') {
        return JsonResultMissingColon;
    }
    (*json)++;
    return JsonResultOk;
}

/**
 * @brief Parse string. The JSON pointer is advanced to the first
 * character after the string.
 * @param json JSON pointer.
 * @param destination Destination. NULL if not required.
 * @param destinationSize Destination size.
 * @param numberOfBytes Number of bytes in string. NULL if not required.
 * @return Result.
 */
JsonResult JsonParseString(const char **const json, char *const destination, const size_t destinationSize, size_t *const numberOfBytes) {
    // Check type
    JsonResult result = CheckType(json, JsonTypeString);
    if (result != JsonResultOk) {
        return result;
    }
    (*json)++;

    // Parse string
    size_t index = 0;
    while (true) {
        if ((destination != NULL) && (index >= destinationSize)) {
            return JsonResultStringTooLong;
        }
        if (**json == '\0') {
            return JsonResultMissingStringEnd;
        }
        if ((**json >= 0) && (**json < 0x20)) {
            return JsonResultInvalidStringCharacter; // control charcaters must be escaped
        }
        if (**json == '\\') {
            result = ParseEscapeSequence(json, destination, &index);
            if (result != JsonResultOk) {
                return result;
            }
            continue;
        }
        if (**json == '\"') {
            (*json)++;
            WriteToDestination(destination, &index, '\0');
            if (numberOfBytes != NULL) {
                *numberOfBytes = index;
            }
            return JsonResultOk;
        }
        WriteToDestination(destination, &index, *(*json)++);
    }
}

/**
 * @brief Parse escape sequence. The JSON pointer is advanced to the first
 * character after the escape sequence.
 * @param json JSON pointer.
 * @param destination Destination.
 * @param index Index.
 * @return Result.
 */
static JsonResult ParseEscapeSequence(const char **const json, char *const destination, size_t *const index) {
    switch (*(*json + 1)) {
        case '\"':
            WriteToDestination(destination, index, '"');
            break;
        case '\\':
            WriteToDestination(destination, index, '\\');
            break;
        case '/':
            WriteToDestination(destination, index, '/');
            break;
        case 'b':
            WriteToDestination(destination, index, '\b');
            break;
        case 'f':
            WriteToDestination(destination, index, '\f');
            break;
        case 'n':
            WriteToDestination(destination, index, '\n');
            break;
        case 'r':
            WriteToDestination(destination, index, '\r');
            break;
        case 't':
            WriteToDestination(destination, index, '\t');
            break;
        case 'u':
            return ParseHexEscapeSequence(json, destination, index);
        default:
            return JsonResultInvalidStringEscapeSequence;
    }
    (*json) += 2;
    return JsonResultOk;
}

/**
 * @brief Parse hex escape sequence. The JSON pointer is advanced to the
 * first character after the hex escape sequence.
 * @param json JSON pointer.
 * @param destination Destination.
 * @param index Index.
 * @return Result.
 */
static JsonResult ParseHexEscapeSequence(const char **const json, char *const destination, size_t *const index) {
    if (isxdigit((int) *(*json + 2)) == 0) {
        return JsonResultInvalidStringHexEscapeSequence;
    }
    if (isxdigit((int) *(*json + 3)) == 0) {
        return JsonResultInvalidStringHexEscapeSequence;
    }
    if (isxdigit((int) *(*json + 4)) == 0) {
        return JsonResultInvalidStringHexEscapeSequence;
    }
    if (isxdigit((int) *(*json + 5)) == 0) {
        return JsonResultInvalidStringHexEscapeSequence;
    }
    char string[5];
    snprintf(string, sizeof(string), "%s", *json + 2);
    unsigned int value;
    if (sscanf(string, "%x", &value) != 1) {
        return JsonResultUnableToParseStringHexEscapeSequence;
    }
    WriteToDestination(destination, index, (char) value);
    (*json) += 6;
    return JsonResultOk;
}

/**
 * @brief Writes character to destination and increments index, if destination is
 * not NULL.
 * @param destination Destination.
 * @param index Index.
 * @param character Character.
 */
static void WriteToDestination(char *const destination, size_t *const index, const char character) {
    if (destination != NULL) {
        destination[(*index)++] = character;
    }
}

/**
 * @brief Parses number. The JSON pointer is advanced to the first character
 * after the number.
 * @param json JSON pointer.
 * @param number Number. NULL if not required.
 * @return Result.
 */
JsonResult JsonParseNumber(const char **const json, float *const number) {
    // Check type
    const JsonResult result = CheckType(json, JsonTypeNumber);
    if (result != JsonResultOk) {
        return result;
    }

    // Parse sign
    const char *jsonCopy = *json;
    if (*jsonCopy == '-') {
        jsonCopy++;
        if (isdigit((int) *jsonCopy) == 0) {
            return JsonResultInvalidNumberFormat; // minus sign must be followed by digit
        }
    }

    // Parse first zero
    if (*jsonCopy == '0') {
        jsonCopy++;
        if (*jsonCopy == '0') {
            return JsonResultInvalidNumberFormat; // leading zeros are invalid
        }
    }

    // Parse integer
    while (isdigit((int) *jsonCopy) != 0) {
        jsonCopy++;
    }

    // Parse fraction
    if (*jsonCopy == '.') {
        jsonCopy++;
        if (isdigit((int) *jsonCopy) == 0) {
            return JsonResultInvalidNumberFormat; // decimal point must be followed by digit
        }
    }
    while (isdigit((int) *jsonCopy) != 0) {
        jsonCopy++;
    }

    // Parse exponent
    if ((*jsonCopy == 'e') || (*jsonCopy == 'E')) {
        jsonCopy++;
        if ((*jsonCopy == '+') || (*jsonCopy == '-')) {
            jsonCopy++;
        }
        if (isdigit((int) *jsonCopy) == 0) {
            return JsonResultInvalidNumberFormat; // exponent must be followed by digit
        }
    }
    while (isdigit((int) *jsonCopy) != 0) {
        jsonCopy++;
    }

    // Read number string
    if (number != NULL) {
        char string[32];
        const size_t numberOfBytes = jsonCopy - *json;
        if (numberOfBytes >= sizeof(string)) {
            return JsonResultNumberTooLong;
        }
        snprintf(string, sizeof(string), "%s", *json);
        if (sscanf(string, "%f", number) != 1) {
            return JsonResultUnableToParseNumber;
        }
    }
    *json = jsonCopy;
    return JsonResultOk;
}

/**
 * @brief Parses boolean. The JSON pointer is advanced to the first character
 * after the boolean.
 * @param json JSON pointer.
 * @param boolean Boolean. NULL if not required.
 * @return Result.
 */
JsonResult JsonParseBoolean(const char **const json, bool *const boolean) {
    // Check type
    const JsonResult result = CheckType(json, JsonTypeBoolean);
    if (result != JsonResultOk) {
        return result;
    }

    // Parse true
    const char trueString[] = "true";
    if (strncmp(*json, trueString, sizeof(trueString) - 1) == 0) {
        (*json) += sizeof(trueString) - 1;
        if (boolean != NULL) {
            *boolean = true;
        }
        return JsonResultOk;
    }

    // Parse false
    const char falseString[] = "false";
    if (strncmp(*json, falseString, sizeof(falseString) - 1) == 0) {
        (*json) += sizeof(falseString) - 1;
        if (boolean != NULL) {
            *boolean = false;
        }
        return JsonResultOk;
    }
    return JsonResultInvalidSyntax;
}

/**
 * @brief Parses null. The JSON pointer is advanced to the first character
 * after the null.
 * @param json JSON pointer.
 * @return Result.
 */
JsonResult JsonParseNull(const char **const json) {
    // Check type
    const JsonResult result = CheckType(json, JsonTypeNull);
    if (result != JsonResultOk) {
        return result;
    }

    // Parse null
    const char nullString[] = "null";
    if (strncmp(*json, nullString, sizeof(nullString) - 1) == 0) {
        (*json) += sizeof(nullString) - 1;
        return JsonResultOk;
    }
    return JsonResultInvalidSyntax;
}

/**
 * @brief Parses any JSON and discards data. The JSON pointer is advanced to
 * the first character after the JSON.
 * @param json JSON pointer.
 * @return Result.
 */
JsonResult JsonParse(const char **const json) {
    int indent = 0;
    return ParseValue(json, false, &indent);
}

/**
 * @brief Prints the JSON structure and result message.
 * @param json_ JSON pointer.
 */
void JsonPrint(const char *json_) {
    int indent = 0;
    const char **const json = &json_;
    const JsonResult result = ParseValue(json, true, &indent);
    printf("%s\n", JsonResultToString(result));
}

/**
 * @brief Parses value and discards data. The JSON pointer is advanced to the
 * first character after the value.
 * @param json JSON pointer.
 * @param print True to print.
 * @param indent Indent.
 * @return Result.
 */
static JsonResult ParseValue(const char **const json, const bool print, int *const indent) {
    // Parse value type
    JsonType type;
    const JsonResult result = JsonParseType(json, &type);
    if (result != JsonResultOk) {
        return result;
    }

    // Print value type
    if (print) {
        const int actualIndent = 4 * *indent;
        switch (type) {
            case JsonTypeString:
                printf("%*sstring\n", actualIndent, "");
                break;
            case JsonTypeNumber:
                printf("%*snumber\n", actualIndent, "");
                break;
            case JsonTypeObject:
                printf("%*sobject\n", actualIndent, "");
                break;
            case JsonTypeArray:
                printf("%*sarray\n", actualIndent, "");
                break;
            case JsonTypeBoolean:
                printf("%*sboolean\n", actualIndent, "");
                break;
            case JsonTypeNull:
                printf("%*snull\n", actualIndent, "");
                break;
        }
    }

    // Parse value
    switch (type) {
        case JsonTypeString:
            return JsonParseString(json, NULL, 0, NULL);
        case JsonTypeNumber:
            return JsonParseNumber(json, NULL);
        case JsonTypeObject:
            return ParseObject(json, print, indent);
        case JsonTypeArray:
            return ParseArray(json, print, indent);
        case JsonTypeBoolean:
            return JsonParseBoolean(json, NULL);
        case JsonTypeNull:
            return JsonParseNull(json);
    }
    return JsonResultOk;
}

/**
 * @brief Parses object and discards data. The JSON pointer is advanced to the
 * first character after the object.
 * @param json JSON pointer.
 * @param print True to print.
 * @param indent Indent.
 * @return Result.
 */
static JsonResult ParseObject(const char **const json, const bool print, int *const indent) {
    // Parse object start
    JsonResult result = JsonParseObjectStart(json);
    if (result != JsonResultOk) {
        return result;
    }

    // Parse object end
    result = JsonParseObjectEnd(json);
    if (result == JsonResultOk) {
        return JsonResultOk;
    }

    // Loop through each key/value pair
    (*indent)++;
    while (true) {
        // Parse key
        char key[64];
        result = JsonParseKey(json, key, sizeof(key));
        if (result != JsonResultOk) {
            return result;
        }

        // Parse value
        result = ParseValue(json, print, indent);
        if (result != JsonResultOk) {
            return result;
        }

        // Parse comma
        result = JsonParseComma(json);
        if (result == JsonResultOk) {
            continue;
        }

        // Parse object end
        result = JsonParseObjectEnd(json);
        if (result != JsonResultOk) {
            return result;
        }
        break;
    }
    (*indent)--;
    return JsonResultOk;
}

/**
 * @brief Parses array and discards data. The JSON pointer is advanced to the
 * first character after the array.
 * @param json JSON pointer.
 * @param print True to print.
 * @param indent Indent.
 * @return Result.
 */
static JsonResult ParseArray(const char **const json, const bool print, int *const indent) {
    // Parse array start
    JsonResult result = JsonParseArrayStart(json);
    if (result != JsonResultOk) {
        return result;
    }

    // Parse array end
    result = JsonParseArrayEnd(json);
    if (result == JsonResultOk) {
        return JsonResultOk;
    }

    // Loop through each value
    (*indent)++;
    while (true) {
        // Parse value
        result = ParseValue(json, print, indent);
        if (result != JsonResultOk) {
            return result;
        }

        // Parse comma
        result = JsonParseComma(json);
        if (result == JsonResultOk) {
            continue;
        }

        // Parse array end
        result = JsonParseArrayEnd(json);
        if (result != JsonResultOk) {
            return result;
        }
        break;
    }
    (*indent)--;
    return JsonResultOk;
}

/**
 * @brief Returns the result message.
 * @param result Result.
 * @return Result message.
 */
const char *JsonResultToString(const JsonResult result) {
    switch (result) {
        case JsonResultOk:
            return "OK";
        case JsonResultInvalidSyntax:
            return "Invalid syntax";
        case JsonResultUnexpectedType:
            return "Unexpected type";
        case JsonResultMissingObjectEnd:
            return "Missing object end";
        case JsonResultMissingArrayEnd:
            return "Missing array end";
        case JsonResultMissingComma:
            return "Missing comma";
        case JsonResultMissingKey:
            return "Missing key";
        case JsonResultMissingColon:
            return "Missing colon";
        case JsonResultMissingStringEnd:
            return "Missing string end";
        case JsonResultStringTooLong:
            return "String too long";
        case JsonResultInvalidStringCharacter:
            return "Invalid string character";
        case JsonResultInvalidStringEscapeSequence:
            return "Invalid string escape sequence";
        case JsonResultInvalidStringHexEscapeSequence:
            return "Invalid string hex escape sequence";
        case JsonResultUnableToParseStringHexEscapeSequence:
            return "Unable to parse string hex escape sequence";
        case JsonResultInvalidNumberFormat:
            return "Invalid number format";
        case JsonResultNumberTooLong:
            return "Number too long";
        case JsonResultUnableToParseNumber:
            return "Unable to parse number";
    }
    return ""; // avoid compiler warning
}

//------------------------------------------------------------------------------
// End of file
