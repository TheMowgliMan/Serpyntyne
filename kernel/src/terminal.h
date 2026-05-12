#ifndef TERMINAL_H_
#define TERMINAL_H_

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <limine.h>
#include <stdarg.h>

#define EOF (-1)

#define TTY_RESET "\033[0m"
#define TTY_RESETFG "\033[39m"
#define TTY_RESETBG "\033[49m"

#define TTY_BLACK "\e[0;30m"
#define TTY_RED "\e[0;31m"
#define TTY_GREEN "\e[0;32m"
#define TTY_BROWN "\e[0;33m"
#define TTY_BLUE "\e[0;34m"
#define TTY_MAGENTA "\e[0;35m"
#define TTY_CYAN "\e[0;36m"
#define TTY_GREY "\e[0;37m"

#define TTY_BLACKBG "\e[40m"
#define TTY_REDBG "\e[41m"
#define TTY_GREENBG "\e[42m"
#define TTY_BROWNBG "\e[43m"
#define TTY_BLUEBG "\e[44m"
#define TTY_MAGENTABG "\e[45m"
#define TTY_CYANBG "\e[46m"
#define TTY_GREYBG "\e[47m"

#define TTY_HIBLACK "\e[0;90m"
#define TTY_HIRED "\e[0;91m"
#define TTY_HIGREEN "\e[0;92m"
#define TTY_HIBROWN "\e[0;93m"
#define TTY_HIBLUE "\e[0;94m"
#define TTY_HIMAGENTA "\e[0;95m"
#define TTY_HICYAN "\e[0;96m"
#define TTY_HIGREY "\e[0;97m"

#define LOG_INFO 272
#define LOG_NOTICE 481
#define LOG_ERROR 44
#define LOG_PROC 21
#define LOG_WARN 123
#define LOG_SUCCESS 77

void termInit();

void kputs(const char* msg);
void kputs_unlocked(const char* msg);

void kerror(const char *msg);
char kputchar(int c);
int kprintf(const char* restrict format, ...);
int kvprintf(const char* restrict format, va_list prm);
int klog(int logstatus, const char* restrict format, ...);
void switch_to_panic_bg(void);
void pad_with_spaces(void);

void jailbreak_terminal(void);

#endif
