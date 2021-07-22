#pragma once
 
#include <stdio.h>
#include <stdlib.h>
/*
#define UNW_LOCAL_ONLY 
#include <libunwind.h>
*/

#include <cxxabi.h>
#include <backtrace.h>

// or just go the gdb route
#include <signal.h>


#define fatal(msg) (printf("Fatal Error: %s (%s:%d)\n", msg, __FILE__, __LINE__), panic());

// how to get addresses of function arguments for syminfo?
// point of pcinfo?


void err_cb(void *data, const char *msg, int errnum) {
    printf("err %d %s\n", errnum, msg);
}

void syminfo_cb(void *data, uintptr_t pc,
        const char *symname,
        uintptr_t symval,
        uintptr_t symsize) {
    *(uintptr_t *)data += symsize;
    printf("syminfo cb name %s size %lu val %lu\n", symname, symsize, symval);
}

int full_cb(void *data, uintptr_t pc,
    	const char *filename, int lineno,
	    const char *function) {
    if (filename == NULL) {
        return 1;
    }

    int status;
    auto demangled = abi::__cxa_demangle(function, NULL, NULL, &status);
    if (demangled != NULL) {
        function = demangled;
    }

    printf("%s:%s:%d\n", filename, function, lineno);

    auto cursor = pc;
    while (true) {
        auto oldcursor = cursor;
        auto res = backtrace_syminfo((backtrace_state*)data, cursor, syminfo_cb, err_cb, &cursor);
        if (!res) break;
        if (cursor == oldcursor) break;
        //printf("test spicy: ");
        //backtrace_syminfo((backtrace_state*)data, oldcursor + 20, syminfo_cb, err_cb, &whocares);
    }

    return 0;
}

static void panic() {
    auto bt_state = backtrace_create_state(
        "/home/pat/repo/sweetlib/sweetlib-test", 
        0, err_cb, NULL
    );

    backtrace_full(bt_state, 1, full_cb, err_cb, bt_state);
    printf("panic time\n");
    raise(SIGTRAP);
}

/*
static void panic() {
    auto bt_state = backtrace_create_state(
        "/home/pat/repo/sweetlib/sweetlib-test", 
        0, err_cb, NULL
    );

    backtrace_full(bt_state, 1, full_cb, err_cb, bt_state);
}
*/
/*
static void panic()
{
  unw_cursor_t cursor;
  unw_context_t context;

  unw_getcontext(&context);
  unw_init_local(&cursor, &context);

  int n=0;
  while ( unw_step(&cursor) ) {
    unw_word_t ip, sp, off;

    unw_get_reg(&cursor, UNW_REG_IP, &ip);
    unw_get_reg(&cursor, UNW_REG_SP, &sp);

    char symbol[256] = {"<unknown>"};
    char *name = symbol;

    if ( !unw_get_proc_name(&cursor, symbol, sizeof(symbol), &off) ) {
      int status;
      if ( (name = abi::__cxa_demangle(symbol, NULL, NULL, &status)) == 0 )
        name = symbol;
    }

    printf("#%-2d 0x%016" PRIxPTR " sp=0x%016" PRIxPTR " %s + 0x%" PRIxPTR "\n",
        ++n,
        static_cast<uintptr_t>(ip),
        static_cast<uintptr_t>(sp),
        name,
        static_cast<uintptr_t>(off));

    if ( name != symbol )
      free(name);
  }

    exit(1);
}
*/