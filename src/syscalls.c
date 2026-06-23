#include <sys/stat.h>

// Resolve compile WARNINGS for GNU _write()
int _close(int file) { (void)file; return -1; }
int _fstat(int file, struct stat *st) { (void)file; st->st_mode = S_IFCHR; return 0; }
int _isatty(int file) { (void)file; return 1; }
int _lseek(int file, int ptr, int dir) { (void)file; (void)ptr; (void)dir; return 0; }
int _read(int file, char *ptr, int len) { (void)file; (void)ptr; (void)len; return 0; }
int _kill(int pid, int sig) { (void)pid; (void)sig; return -1; }
int _getpid(void) { return 1; }
int _exit(void) { return 1; }
int _sbrk(void) { return 1; }

// The "Magic" stub that sets you up for the future:
int _write(int file, char *ptr, int len) { 
    (void)file; (void)ptr;
    return len; 
}