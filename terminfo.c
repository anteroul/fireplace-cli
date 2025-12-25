#include <unistd.h>
#include <sys/types.h>
#include "terminfo.h"

static pid_t get_ppid(pid_t);
static int get_comm(pid_t, char *, size_t);
static const char *match_terminal_binary(const char *);

#ifdef __linux__
#include <stdio.h>
#include <unistd.h>

static int read_ppid(pid_t pid)
{
    char path[64];
    snprintf(path, sizeof(path), "/proc/%d/stat", pid);

    FILE *f = fopen(path, "r");
    if (!f) return -1;

    int ppid;
    fscanf(f, "%*d %*s %*c %d", &ppid);
    fclose(f);
    return ppid;
}

static int read_comm(pid_t pid, char *buf, size_t sz)
{
    char path[64];
    snprintf(path, sizeof(path), "/proc/%d/comm", pid);

    FILE *f = fopen(path, "r");
    if (!f) return -1;

    fgets(buf, sz, f);
    buf[strcspn(buf, "\n")] = 0;
    fclose(f);
    return 0;
}
#endif

const char *detect_terminal_proc(void)
{
    pid_t pid = getpid();
    char name[64];

    for (int i = 0; i < 10 && pid > 1; i++) {
        if (read_comm(pid, name, sizeof(name)) == 0) {
            const char *hit = match_terminal_binary(name);
            if (hit) return hit;
        }
        pid = read_ppid(pid);
    }
    return NULL;
}

static const char *getenv_or(const char *name, const char *fallback)
{
    const char *v = getenv(name);
    return v ? v : fallback;
}

const char *detect_terminal_env(void)
{
    if (getenv("WT_SESSION")) return "Windows Terminal";
    if (getenv("KITTY_WINDOW_ID")) return "kitty";
    if (getenv("WEZTERM_EXECUTABLE")) return "WezTerm";
    if (getenv("ALACRITTY_LOG")) return "Alacritty";
    if (getenv("KONSOLE_VERSION")) return "Konsole";
    if (getenv("TERMINATOR_UUID")) return "Terminator";
    if (getenv("TILIX_ID")) return "Tilix";
    if (getenv("GNOME_TERMINAL_SCREEN")) return "GNOME Terminal";
    if (getenv("MATE_TERMINAL_SCREEN")) return "MATE Terminal";
    if (getenv("LXTERMINAL_UUID")) return "LXTerminal";
    if (getenv("VTE_VERSION") && getenv("XTERM_VERSION")) return "XFCE Terminal";
    if (getenv("VTE_VERSION")) return "VTE-based terminal";
    if (getenv("TMUX")) return "tmux";
    if (getenv("STY")) return "screen";
    // if none detected return null;
    printf("%sWARNING: Unknown terminal emulator detected.%s\n", ANSI_COLOR_YELLOW, ANSI_COLOR_RESET);
    return NULL;
}

const char *detect_shell(void)
{
    if (getenv("BASH_VERSION")) return "bash";
    if (getenv("ZSH_VERSION"))  return "zsh";
    if (getenv("FISH_VERSION")) return "fish";

    return getenv_or("SHELL", "unknown");
}

#ifdef __linux__
#include <unistd.h>
static pid_t get_ppid(pid_t pid) {
    char path[64];
    snprintf(path, sizeof(path), "/proc/%d/stat", pid);
    FILE *f = fopen(path, "r");
    if (!f) return -1;
    int ppid;
    fscanf(f, "%*d %*s %*c %d", &ppid);
    fclose(f);
    return ppid;
}

static int get_comm(pid_t pid, char *buf, size_t sz) {
    char path[64];
    snprintf(path, sizeof(path), "/proc/%d/comm", pid);
    FILE *f = fopen(path, "r");
    if (!f) return -1;
    fgets(buf, sz, f);
    buf[strcspn(buf, "\n")] = 0;
    fclose(f);
    return 0;
}
#endif

static const char *match_terminal_binary(const char *name)
{
    if (!strcmp(name, "xfce4-terminal")) return "XFCE Terminal";
    if (!strcmp(name, "gnome-terminal")) return "GNOME Terminal";
    if (!strcmp(name, "konsole")) return "Konsole";
    if (!strcmp(name, "alacritty")) return "Alacritty";
    if (!strcmp(name, "kitty")) return "kitty";
    if (!strcmp(name, "wezterm")) return "WezTerm";
    if (!strcmp(name, "foot")) return "foot";
    if (!strcmp(name, "xterm")) return "xterm";
    if (!strcmp(name, "urxvt")) return "rxvt-unicode";
    if (!strcmp(name, "Terminal")) return "Apple Terminal";
    if (!strcmp(name, "iTerm2")) return "iTerm2";
    return NULL;
}

const char *detect_terminal(void)
{
    const char *t;

    t = detect_terminal_env();
    if (t) return t;

    t = detect_terminal_proc();
    if (t) return t;

    /* fallback */
    return getenv("TERM") ? getenv("TERM") : "unknown";
}

#if defined(__APPLE__) || defined(__FreeBSD__) || defined(__OpenBSD__)
#include <sys/sysctl.h>

static pid_t get_ppid(pid_t pid) {
    struct kinfo_proc kp;
    size_t len = sizeof(kp);
    int mib[] = { CTL_KERN, KERN_PROC, KERN_PROC_PID, pid };

    if (sysctl(mib, 4, &kp, &len, NULL, 0) < 0)
        return -1;

#if defined(__APPLE__)
    return kp.kp_eproc.e_ppid;
#else
    return kp.p_ppid;
#endif
}

static int get_comm(pid_t pid, char *buf, size_t sz) {
    struct kinfo_proc kp;
    size_t len = sizeof(kp);
    int mib[] = { CTL_KERN, KERN_PROC, KERN_PROC_PID, pid };

    if (sysctl(mib, 4, &kp, &len, NULL, 0) < 0)
        return -1;

#if defined(__APPLE__)
    strlcpy(buf, kp.kp_proc.p_comm, sz);
#else
    strlcpy(buf, kp.p_comm, sz);
#endif
    return 0;
}
#endif

#ifdef _WIN32
const char *detect_terminal(void)
{
    return "Windows Console (PDCurses)";
}
#endif