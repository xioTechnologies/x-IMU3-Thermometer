/**
 * @file Json.h
 * @author Seb Madgwick
 * @brief Library for parsing of JSON strings.
 */

#ifndef JSON_H
#define JSON_H

#ifdef __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------
// Includes

#include <stdbool.h>
#include <stddef.h>

//------------------------------------------------------------------------------
// Definitions

/**
 * @brief Result.
 */
typedef enum {
    JsonResultOk,
    JsonResultInvalidSyntax,
    JsonResultUnexpectedType,
    JsonResultMissingObjectEnd,
    JsonResultMissingArrayEnd,
    JsonResultMissingComma,
    JsonResultMissingKey,
    JsonResultMissingColon,
    JsonResultMissingStringEnd,
    JsonResultStringTooLong,
    JsonResultInvalidStringCharacter,
    JsonResultInvalidStringEscapeSequence,
    JsonResultInvalidStringHexEscapeSequence,
    JsonResultUnableToParseStringHexEscapeSequence,
    JsonResultInvalidNumberFormat,
    JsonResultNumberTooLong,
    JsonResultUnableToParseNumber,
} JsonResult;

/**
 * @brief Type.
 */
typedef enum {
    JsonTypeString,
    JsonTypeNumber,
    JsonTypeObject,
    JsonTypeArray,
    JsonTypeBoolean,
    JsonTypeNull,
} JsonType;

//------------------------------------------------------------------------------
// Function declarations

JsonResult JsonParseType(const char **const json, JsonType *const type);

JsonResult JsonParseObjectStart(const char **const json);

JsonResult JsonParseObjectEnd(const char **const json);

JsonResult JsonParseArrayStart(const char **const json);

JsonResult JsonParseArrayEnd(const char **const json);

JsonResult JsonParseComma(const char **const json);

JsonResult JsonParseKey(const char **const json, char *const destination, const size_t destinationSize);

JsonResult JsonParseString(const char **const json, char *const destination, const size_t destinationSize, size_t *const numberOfBytes);

JsonResult JsonParseNumber(const char **const json, float *const number);

JsonResult JsonParseBoolean(const char **const json, bool *const boolean);

JsonResult JsonParseNull(const char **const json);

JsonResult JsonParse(const char **const json);

void JsonPrint(const char *json);

const char *JsonResultToString(const JsonResult result);

#ifdef __cplusplus
}
#endif

#endif

//------------------------------------------------------------------------------
// End of file
