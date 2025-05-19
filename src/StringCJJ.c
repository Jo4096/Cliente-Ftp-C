#include "StringCJJ.h"

StrCJJ *StrCJJ_create(const char *baseString)
{
    if (!baseString)
    {
        return NULL;
    }

    StrCJJ *ret = (StrCJJ *)malloc(sizeof(StrCJJ));
    if (!ret)
    {
        return NULL;
    }

    ret->len = strlen(baseString);
    ret->size = ret->len + 1;

    ret->str = (char *)malloc(ret->size);
    if (ret->str == NULL)
    {
        free(ret);
        return NULL;
    }

    memcpy(ret->str, baseString, ret->len);
    ret->str[ret->len] = '\0';
    return ret;
}

void StrCJJ_free(StrCJJ *str)
{
    if (str)
    {
        if (str->str)
        {
            // StrCJJ_print(str, "feeing -> ", "\n");
            free(str->str);
        }
        free(str);
    }
}

void StrCJJ_appendStr(StrCJJ *dest, StrCJJ *src)
{
    if (!dest || !src || !src->str)
    {
        return;
    }
    size_t newLen = dest->len + src->len;
    if (newLen + 1 > dest->size)
    {
        size_t newSize = newLen + 1;
        char *newStr = (char *)realloc(dest->str, newSize);
        if (!newStr)
        {
            return;
        }

        dest->str = newStr;
        dest->size = newSize;
    }
    memcpy(dest->str + dest->len, src->str, src->len);
    dest->len = newLen;
    dest->str[dest->len] = '\0';
}

void StrCJJ_appendChar(StrCJJ *dest, char c)
{
    if (!dest)
    {
        return;
    }
    size_t newLen = dest->len + 1;
    if (newLen + 1 > dest->size)
    {
        size_t newSize = newLen + 1;
        char *newStr = (char *)realloc(dest->str, newSize);
        if (!newStr)
        {
            return;
        }
        dest->str = newStr;
        dest->size = newSize;
    }
    dest->str[dest->len] = c;
    dest->len = newLen;
    dest->str[dest->len] = '\0';
}

void StrCJJ_appendFormat(StrCJJ *dest, const char *format, ...)
{
    if (!dest || !format)
    {
        return;
    }
    va_list args;
    va_start(args, format);
    va_list argsCopy;
    va_copy(argsCopy, args);
    int required = vsnprintf(NULL, 0, format, argsCopy);
    va_end(argsCopy);

    if (required < 0)
    {
        va_end(args);
        return;
    }

    size_t newLen = dest->len + (size_t)required;

    if (newLen + 1 > dest->size)
    {
        size_t newSize = newLen + 1;
        char *newStr = (char *)realloc(dest->str, newSize);
        if (!newStr)
        {
            va_end(args);
            return;
        }

        dest->str = newStr;
        dest->size = newSize;
    }

    vsnprintf(dest->str + dest->len, dest->size - dest->len, format, args);
    dest->len = newLen;

    va_end(args);
}

ssize_t StrCJJ_findIndexOf(StrCJJ *hay, StrCJJ *needle)
{
    if (!hay || !needle || !hay->str || !needle->str)
    {
        return -1;
    }

    if (needle->len == 0 || hay->len < needle->len)
    {
        return -1;
    }

    for (size_t i = 0; i <= hay->len - needle->len; ++i)
    {
        if (strncmp(hay->str + i, needle->str, needle->len) == 0)
        {
            return (ssize_t)i;
        }
    }

    return -1;
}

ssize_t StrCJJ_findIndexOf_2(StrCJJ *hay, const char *needle)
{
    if (!hay || !needle || !hay->str)
    {
        return -1;
    }

    size_t nlen = strlen(needle);

    if (nlen == 0 || hay->len < nlen)
    {
        return -1;
    }

    for (size_t i = 0; i <= hay->len - nlen; ++i)
    {
        if (strncmp(hay->str + i, needle, nlen) == 0)
        {
            return (ssize_t)i;
        }
    }

    return -1;
}

ssize_t StrCJJ_findLastIndexOf(StrCJJ *hay, StrCJJ *needle)
{
    if (!hay || !needle || !hay->str || !needle->str)
    {
        return -1;
    }

    if (needle->len == 0 || hay->len < needle->len)
    {
        return -1;
    }

    for (ssize_t i = (ssize_t)(hay->len - needle->len); i >= 0; i--)
    {
        if (strncmp(hay->str + i, needle->str, needle->len) == 0)
        {
            return i;
        }
    }

    return -1;
}

ssize_t StrCJJ_findLastIndexOf_2(StrCJJ *hay, const char *needle)
{
    if (!hay || !needle || !hay->str)
    {
        return -1;
    }

    size_t nlen = strlen(needle);

    if (nlen == 0 || hay->len < nlen)
    {
        return -1;
    }

    for (ssize_t i = (ssize_t)(hay->len - nlen); i >= 0; i--)
    {
        if (strncmp(hay->str + i, needle, nlen) == 0)
        {
            return i;
        }
    }

    return -1;
}

void StrCJJ_change(StrCJJ *str, const char *newString)
{
    if (!str || !newString)
    {
        return;
    }

    size_t newLen = strlen(newString);

    if (newLen + 1 > str->size)
    {
        char *newStr = (char *)realloc(str->str, newLen + 1);
        if (!newStr)
        {
            return;
        }
        str->str = newStr;
        str->size = newLen + 1;
    }

    memcpy(str->str, newString, newLen);
    str->str[newLen] = '\0';
    str->len = newLen;
}

StrCJJ *StrCJJ_clone(StrCJJ *original)
{
    return (!original || !original->str) ? NULL : StrCJJ_create(original->str);
}

void StrCJJ_clear(StrCJJ *str)
{
    if (!str || !str->str)
    {
        return;
    }

    str->str[0] = '\0';
    str->len = 0;
}

void StrCJJ_trim(StrCJJ *str)
{
    if (!str || !str->str || str->len == 0)
    {
        return;
    }

    size_t start = 0;
    size_t end = str->len - 1;

    while (start < str->len && isspace((unsigned char)str->str[start]))
    {
        start++;
    }

    while (end > start && isspace((unsigned char)str->str[end]))
    {
        end--;
    }

    size_t newLen = end - start + 1;

    if (start > 0)
    {
        memmove(str->str, str->str + start, newLen);
    }

    str->str[newLen] = '\0';
    str->len = newLen;
}

void StrCJJ_removeAllSpaces(StrCJJ *str)

{
    if (!str || !str->str || str->len == 0)
    {
        return;
    }

    size_t writeIndex = 0;

    for (size_t readIndex = 0; readIndex < str->len; readIndex++)
    {
        if (!isspace((unsigned char)str->str[readIndex]))
        {
            str->str[writeIndex++] = str->str[readIndex];
        }
    }

    str->str[writeIndex] = '\0';
    str->len = writeIndex;
}

StrCJJ *StrCJJ_substring(StrCJJ *str, size_t start, size_t length)
{
    if (!str || !str->str || start >= str->len)
    {
        return NULL;
    }

    if (start + length > str->len)
    {
        length = str->len - start;
    }

    char *buffer = (char *)malloc(length + 1);
    if (!buffer)
    {
        return NULL;
    }

    memcpy(buffer, str->str + start, length);
    buffer[length] = '\0';

    StrCJJ *substr = StrCJJ_create(buffer);
    free(buffer);

    return substr;
}

bool StrCJJ_contains(StrCJJ *str, const char *substr)
{
    return (bool)(StrCJJ_findIndexOf_2(str, substr) >= 0);
}

void StrCJJ_replace(StrCJJ *str, const char *old, const char *newsub)
{
    if (!str || !str->str || !old || !newsub || strlen(old) == 0)
    {
        return;
    }

    size_t oldLen = strlen(old);
    size_t newLen = strlen(newsub);

    size_t count = 0;
    for (char *p = str->str; (p = strstr(p, old)); p += oldLen)
    {
        count++;
    }

    if (count == 0)
    {
        return;
    }

    size_t finalLen = str->len + count * (newLen - oldLen);
    char *newStr = (char *)malloc(finalLen + 1);
    if (!newStr)
    {
        return;
    }

    char *src = str->str;
    char *dst = newStr;

    while (*src)
    {
        char *pos = strstr(src, old);
        if (pos)
        {
            size_t segmentLen = pos - src;
            memcpy(dst, src, segmentLen);
            dst += segmentLen;

            memcpy(dst, newsub, newLen);
            dst += newLen;

            src = pos + oldLen;
        }
        else
        {
            strcpy(dst, src);
            break;
        }
    }

    free(str->str);
    str->str = newStr;
    str->len = finalLen;
    str->size = finalLen + 1;
}

bool StrCJJ_startsWith(StrCJJ *str, const char *prefix)
{
    if (!str || !str->str || !prefix)
    {
        return false;
    }

    size_t prefixLen = strlen(prefix);
    if (prefixLen > str->len)
    {
        return false;
    }

    return strncmp(str->str, prefix, prefixLen) == 0;
}

bool StrCJJ_endsWith(StrCJJ *str, const char *suffix)
{
    if (!str || !str->str || !suffix)
    {
        return false;
    }

    size_t suffixLen = strlen(suffix);
    if (suffixLen > str->len)
    {
        return false;
    }

    return strncmp(str->str + (str->len - suffixLen), suffix, suffixLen) == 0;
}

StrCJJ *StrCJJ_basename(StrCJJ *path)
{
    ssize_t index = StrCJJ_findLastIndexOf_2(path, "/");
    if (index < 0)
    {
        return StrCJJ_clone(path);
    }
    return StrCJJ_substring(path, index + 1, path->len - (index + 1));
}

StrCJJ *StrCJJ_dirname(StrCJJ *path)
{
    ssize_t index = StrCJJ_findLastIndexOf_2(path, "/");
    if (index < 0)
    {
        return StrCJJ_create("");
    }
    return StrCJJ_substring(path, 0, index + 1);
}

#include <stdlib.h>

StrCJJ **StrCJJ_split(StrCJJ *str, char delimiter, size_t *count)
{
    if (!str || !str->str || !count)
    {
        return NULL;
    }

    *count = 0;

    size_t parts = 1;
    for (size_t i = 0; i < str->len; ++i)
    {
        if (str->str[i] == delimiter)
        {
            parts++;
        }
    }

    StrCJJ **result = (StrCJJ **)malloc(sizeof(StrCJJ *) * parts);
    if (!result)
    {
        return NULL;
    }

    size_t start = 0;
    size_t partIndex = 0;

    for (size_t i = 0; i <= str->len; ++i)
    {
        if (i == str->len || str->str[i] == delimiter)
        {
            size_t segmentLen = i - start;

            StrCJJ *segment = StrCJJ_substring((StrCJJ *)str, start, segmentLen);
            if (!segment)
            {
                for (size_t j = 0; j < partIndex; ++j)
                {
                    StrCJJ_free(result[j]);
                }
                free(result);
                *count = 0;
                return NULL;
            }

            result[partIndex++] = segment;
            start = i + 1;
        }
    }

    *count = partIndex;
    return result;
}

bool StrCJJ_isEmpty(StrCJJ *str)
{
    return str->len != 0;
}

bool StrCJJ_equals(StrCJJ *a, StrCJJ *b)
{
    return strcmp(a->str, b->str) == 0;
}

void StrCJJ_reserve(StrCJJ *str, size_t newCapacity)
{
    if (!str)
    {
        return;
    }

    if (newCapacity <= str->size)
    {
        return;
    }

    char *newBuf = (char *)realloc(str->str, newCapacity);
    if (!newBuf)
    {
        return;
    }

    str->str = newBuf;
    str->size = newCapacity;
}

void StrCJJ_shrinkToFit(StrCJJ *str)
{
    if (!str)
    {
        return;
    }

    if (str->size == str->len + 1)
    {
        return;
    }

    char *newBuf = (char *)realloc(str->str, str->len + 1);
    if (!newBuf)
    {
        return;
    }

    str->str = newBuf;
    str->size = str->len + 1;
}

void StrCJJ_print(StrCJJ *str, const char *prefix, const char *suffix)
{
    if (prefix)
    {
        printf("%s", prefix);
    }
    if (str && str->str)
    {
        printf("%s ", str->str);
    }
    if (suffix)
    {
        printf("%s\n", suffix);
    }
}

StrCJJ *StrCJJ_input(size_t maxLen)
{
    if (maxLen == 0)
    {
        return NULL;
    }

    char *buffer = (char *)malloc(maxLen + 1);
    if (!buffer)
    {
        return NULL;
    }

    if (fgets(buffer, maxLen + 1, stdin) == NULL)
    {
        free(buffer);
        return NULL;
    }

    // Remove newline, if present
    size_t len = strlen(buffer);
    if (len > 0 && buffer[len - 1] == '\n')
    {
        buffer[len - 1] = '\0';
    }

    StrCJJ *inputStr = StrCJJ_create(buffer);
    free(buffer);
    return inputStr;
}

void StrCJJ_freeArray(StrCJJ **array, size_t count)
{
    if (array)
    {
        for (size_t i = 0; i < count; i++)
        {
            StrCJJ_free(array[i]);
        }
        free(array);
    }
}
