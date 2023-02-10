#ifndef TEST_H
#define TEST_H

#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>

bool assert(bool b, const char* format, ...);
void print_test_report();
void format_binary_string(char body[17], uint16_t n);
void print_binary(uint16_t n);

#endif