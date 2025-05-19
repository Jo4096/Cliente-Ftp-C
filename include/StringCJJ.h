#ifndef STRINGSCJJ_H
#define STRINGSCJJ_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include <sys/types.h>
#include <stdarg.h>

typedef struct StringCJJ
{
    char *str;
    size_t size;
    size_t len;
} StrCJJ;

StrCJJ *StrCJJ_create(const char *baseString);
void StrCJJ_free(StrCJJ *str);

void StrCJJ_appendStr(StrCJJ *dest, StrCJJ *src);
void StrCJJ_appendChar(StrCJJ *dest, char c);
void StrCJJ_appendFormat(StrCJJ *dest, const char *format, ...);

ssize_t StrCJJ_findIndexOf(StrCJJ *hay, StrCJJ *needle);
ssize_t StrCJJ_findIndexOf_2(StrCJJ *hay, const char *needle);
ssize_t StrCJJ_findLastIndexOf(StrCJJ *hay, StrCJJ *needle);
ssize_t StrCJJ_findLastIndexOf_2(StrCJJ *hay, const char *needle);

void StrCJJ_change(StrCJJ *str, const char *newString);
StrCJJ *StrCJJ_clone(StrCJJ *original);
void StrCJJ_clear(StrCJJ *str);
void StrCJJ_trim(StrCJJ *str);
void StrCJJ_removeAllSpaces(StrCJJ *str);
StrCJJ *StrCJJ_substring(StrCJJ *str, size_t start, size_t length);

bool StrCJJ_contains(StrCJJ *str, const char *substr);
void StrCJJ_replace(StrCJJ *str, const char *old, const char *newsub);
bool StrCJJ_startsWith(StrCJJ *str, const char *prefix);
bool StrCJJ_endsWith(StrCJJ *str, const char *suffix);

StrCJJ *StrCJJ_basename(StrCJJ *path);
StrCJJ *StrCJJ_dirname(StrCJJ *path);
StrCJJ **StrCJJ_split(StrCJJ *str, char delimiter, size_t *count);

bool StrCJJ_isEmpty(StrCJJ *str);
bool StrCJJ_equals(StrCJJ *a, StrCJJ *b);

void StrCJJ_reserve(StrCJJ *str, size_t newCapacity);
void StrCJJ_shrinkToFit(StrCJJ *str);

void StrCJJ_print(StrCJJ *str, const char *prefix, const char *suffix);
StrCJJ *StrCJJ_input(size_t maxLen);
void StrCJJ_freeArray(StrCJJ **array, size_t count);

#endif