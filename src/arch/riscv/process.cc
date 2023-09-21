/*
 * Copyright (c) 2004-2005 The Regents of The University of Michigan
 * Copyright (c) 2016 The University of Virginia
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met: redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer;
 * redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution;
 * neither the name of the copyright holders nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "arch/riscv/process.hh"

#include <algorithm>
#include <cstddef>
#include <iostream>
#include <iterator>
#include <map>
#include <string>
#include <vector>

#include "arch/riscv/isa.hh"
#include "arch/riscv/page_size.hh"
#include "arch/riscv/regs/int.hh"
#include "arch/riscv/regs/misc.hh"
#include "base/loader/elf_object.hh"
#include "base/loader/object_file.hh"
#include "base/logging.hh"
#include "base/random.hh"
#include "cpu/thread_context.hh"
#include "debug/Stack.hh"
#include "mem/page_table.hh"
#include "params/Process.hh"
#include "sim/aux_vector.hh"
#include "sim/process.hh"
#include "sim/process_impl.hh"
#include "sim/syscall_return.hh"
#include "sim/system.hh"
#include "sim/syscall_desc.hh"

namespace gem5
{

using namespace RiscvISA;

// std::map<int, SyscallDesc> RiscvProcess64::syscallDescs = {
//     {0,    SyscallDesc("io_setup")},
//     {1,    SyscallDesc("io_destroy")},
//     {2,    SyscallDesc("io_submit")},
//     {3,    SyscallDesc("io_cancel")},
//     {4,    SyscallDesc("io_getevents")},
//     {5,    SyscallDesc("setxattr")},
//     {6,    SyscallDesc("lsetxattr")},
//     {7,    SyscallDesc("fsetxattr")},
//     {8,    SyscallDesc("getxattr")},
//     {9,    SyscallDesc("lgetxattr")},
//     {10,   SyscallDesc("fgetxattr")},
//     {11,   SyscallDesc("listxattr")},
//     {12,   SyscallDesc("llistxattr")},
//     {13,   SyscallDesc("flistxattr")},
//     {14,   SyscallDesc("removexattr")},
//     {15,   SyscallDesc("lremovexattr")},
//     {16,   SyscallDesc("fremovexattr")},
//     {17,   SyscallDesc("getcwd", getcwdFunc)},
//     {18,   SyscallDesc("lookup_dcookie")},
//     {19,   SyscallDesc("eventfd2")},
//     {20,   SyscallDesc("epoll_create1")},
//     {21,   SyscallDesc("epoll_ctl")},
//     {22,   SyscallDesc("epoll_pwait")},
//     {23,   SyscallDesc("dup", dupFunc)},
//     {24,   SyscallDesc("dup3")},
//     {25,   SyscallDesc("fcntl", fcntl64Func)},
//     {26,   SyscallDesc("inotify_init1")},
//     {27,   SyscallDesc("inotify_add_watch")},
//     {28,   SyscallDesc("inotify_rm_watch")},
//     {29,   SyscallDesc("ioctl", ioctlFunc<RiscvLinux64>)},
//     {30,   SyscallDesc("ioprio_get")},
//     {31,   SyscallDesc("ioprio_set")},
//     {32,   SyscallDesc("flock")},
//     {33,   SyscallDesc("mknodat")},
//     {34,   SyscallDesc("mkdirat")},
//     {35,   SyscallDesc("unlinkat", unlinkatFunc<RiscvLinux64>)},
//     {36,   SyscallDesc("symlinkat")},
//     {37,   SyscallDesc("linkat")},
//     {38,   SyscallDesc("renameat", renameatFunc<RiscvLinux64>)},
//     {39,   SyscallDesc("umount2")},
//     {40,   SyscallDesc("mount")},
//     {41,   SyscallDesc("pivot_root")},
//     {42,   SyscallDesc("nfsservctl")},
//     {43,   SyscallDesc("statfs", statfsFunc<RiscvLinux64>)},
//     {44,   SyscallDesc("fstatfs", fstatfsFunc<RiscvLinux64>)},
//     {45,   SyscallDesc("truncate", truncateFunc)},
//     {46,   SyscallDesc("ftruncate", ftruncate64Func)},
//     {47,   SyscallDesc("fallocate", fallocateFunc)},
//     {48,   SyscallDesc("faccessat", faccessatFunc<RiscvLinux64>)},
//     {49,   SyscallDesc("chdir")},
//     {50,   SyscallDesc("fchdir")},
//     {51,   SyscallDesc("chroot")},
//     {52,   SyscallDesc("fchmod", fchmodFunc<RiscvLinux64>)},
//     {53,   SyscallDesc("fchmodat")},
//     {54,   SyscallDesc("fchownat")},
//     {55,   SyscallDesc("fchown", fchownFunc)},
//     {56,   SyscallDesc("openat", openatFunc<RiscvLinux64>)},
//     {57,   SyscallDesc("close", closeFunc)},
//     {58,   SyscallDesc("vhangup")},
//     {59,   SyscallDesc("pipe2")},
//     {60,   SyscallDesc("quotactl")},
//     {61,   SyscallDesc("getdents64")},
//     {62,   SyscallDesc("lseek", lseekFunc)},
//     {63,   SyscallDesc("read", readFunc<RiscvLinux64>)},
//     {64,   SyscallDesc("write", writeFunc<RiscvLinux64>)},
//     {66,   SyscallDesc("writev", writevFunc<RiscvLinux64>)},
//     {67,   SyscallDesc("pread64")},
//     {68,   SyscallDesc("pwrite64", pwrite64Func<RiscvLinux64>)},
//     {69,   SyscallDesc("preadv")},
//     {70,   SyscallDesc("pwritev")},
//     {71,   SyscallDesc("sendfile")},
//     {72,   SyscallDesc("pselect6")},
//     {73,   SyscallDesc("ppoll")},
//     {74,   SyscallDesc("signalfd64")},
//     {75,   SyscallDesc("vmsplice")},
//     {76,   SyscallDesc("splice")},
//     {77,   SyscallDesc("tee")},
//     {78,   SyscallDesc("readlinkat", readlinkatFunc<RiscvLinux64>)},
//     {79,   SyscallDesc("fstatat", fstatat64Func<RiscvLinux64>)},
//     {80,   SyscallDesc("fstat", fstat64Func<RiscvLinux64>)},
//     {81,   SyscallDesc("sync")},
//     {82,   SyscallDesc("fsync")},
//     {83,   SyscallDesc("fdatasync")},
//     {84,   SyscallDesc("sync_file_range2")},
//     {85,   SyscallDesc("timerfd_create")},
//     {86,   SyscallDesc("timerfd_settime")},
//     {87,   SyscallDesc("timerfd_gettime")},
//     {88,   SyscallDesc("utimensat")},
//     {89,   SyscallDesc("acct")},
//     {90,   SyscallDesc("capget")},
//     {91,   SyscallDesc("capset")},
//     {92,   SyscallDesc("personality")},
//     {93,   SyscallDesc("exit", exitFunc)},
//     {94,   SyscallDesc("exit_group", exitGroupFunc)},
//     {95,   SyscallDesc("waitid")},
//     {96,   SyscallDesc("set_tid_address", setTidAddressFunc)},
//     {97,   SyscallDesc("unshare")},
//     {98,   SyscallDesc("futex", futexFunc<RiscvLinux64>)},
//     {99,   SyscallDesc("set_robust_list", ignoreWarnOnceFunc)},
//     {100,  SyscallDesc("get_robust_list", ignoreWarnOnceFunc)},
//     {101,  SyscallDesc("nanosleep", ignoreWarnOnceFunc)},
//     {102,  SyscallDesc("getitimer")},
//     {103,  SyscallDesc("setitimer")},
//     {104,  SyscallDesc("kexec_load")},
//     {105,  SyscallDesc("init_module")},
//     {106,  SyscallDesc("delete_module")},
//     {107,  SyscallDesc("timer_create")},
//     {108,  SyscallDesc("timer_gettime")},
//     {109,  SyscallDesc("timer_getoverrun")},
//     {110,  SyscallDesc("timer_settime")},
//     {111,  SyscallDesc("timer_delete")},
//     {112,  SyscallDesc("clock_settime")},
//     {113,  SyscallDesc("clock_gettime", clock_gettimeFunc<RiscvLinux64>)},
//     {114,  SyscallDesc("clock_getres", clock_getresFunc<RiscvLinux64>)},
//     {115,  SyscallDesc("clock_nanosleep")},
//     {116,  SyscallDesc("syslog")},
//     {117,  SyscallDesc("ptrace")},
//     {118,  SyscallDesc("sched_setparam")},
//     {119,  SyscallDesc("sched_setscheduler")},
//     {120,  SyscallDesc("sched_getscheduler")},
//     {121,  SyscallDesc("sched_getparam")},
//     {122,  SyscallDesc("sched_setaffinity")},
//     {123,  SyscallDesc("sched_getaffinity")},
//     {124,  SyscallDesc("sched_yield", ignoreWarnOnceFunc)},
//     {125,  SyscallDesc("sched_get_priority_max")},
//     {126,  SyscallDesc("sched_get_priority_min")},
//     {127,  SyscallDesc("scheD_rr_get_interval")},
//     {128,  SyscallDesc("restart_syscall")},
//     {129,  SyscallDesc("kill")},
//     {130,  SyscallDesc("tkill")},
//     {131,  SyscallDesc("tgkill", tgkillFunc<RiscvLinux64>)},
//     {132,  SyscallDesc("sigaltstack")},
//     {133,  SyscallDesc("rt_sigsuspend", ignoreWarnOnceFunc)},
//     {134,  SyscallDesc("rt_sigaction", ignoreWarnOnceFunc)},
//     {135,  SyscallDesc("rt_sigprocmask", ignoreWarnOnceFunc)},
//     {136,  SyscallDesc("rt_sigpending", ignoreWarnOnceFunc)},
//     {137,  SyscallDesc("rt_sigtimedwait", ignoreWarnOnceFunc)},
//     {138,  SyscallDesc("rt_sigqueueinfo", ignoreWarnOnceFunc)},
//     {139,  SyscallDesc("rt_sigreturn", ignoreWarnOnceFunc)},
//     {140,  SyscallDesc("setpriority")},
//     {141,  SyscallDesc("getpriority")},
//     {142,  SyscallDesc("reboot")},
//     {143,  SyscallDesc("setregid")},
//     {144,  SyscallDesc("setgid")},
//     {145,  SyscallDesc("setreuid")},
//     {146,  SyscallDesc("setuid", setuidFunc)},
//     {147,  SyscallDesc("setresuid")},
//     {148,  SyscallDesc("getresuid")},
//     {149,  SyscallDesc("getresgid")},
//     {150,  SyscallDesc("getresgid")},
//     {151,  SyscallDesc("setfsuid")},
//     {152,  SyscallDesc("setfsgid")},
//     {153,  SyscallDesc("times", timesFunc<RiscvLinux64>)},
//     {154,  SyscallDesc("setpgid", setpgidFunc)},
//     {155,  SyscallDesc("getpgid")},
//     {156,  SyscallDesc("getsid")},
//     {157,  SyscallDesc("setsid")},
//     {158,  SyscallDesc("getgroups")},
//     {159,  SyscallDesc("setgroups")},
//     {160,  SyscallDesc("uname", unameFunc64)},
//     {161,  SyscallDesc("sethostname")},
//     {162,  SyscallDesc("setdomainname")},
//     {163,  SyscallDesc("getrlimit", getrlimitFunc<RiscvLinux64>)},
//     {164,  SyscallDesc("setrlimit", ignoreFunc)},
//     {165,  SyscallDesc("getrusage", getrusageFunc<RiscvLinux64>)},
//     {166,  SyscallDesc("umask", umaskFunc)},
//     {167,  SyscallDesc("prctl")},
//     {168,  SyscallDesc("getcpu")},
//     {169,  SyscallDesc("gettimeofday", gettimeofdayFunc<RiscvLinux64>)},
//     {170,  SyscallDesc("settimeofday")},
//     {171,  SyscallDesc("adjtimex")},
//     {172,  SyscallDesc("getpid", getpidFunc)},
//     {173,  SyscallDesc("getppid", getppidFunc)},
//     {174,  SyscallDesc("getuid", getuidFunc)},
//     {175,  SyscallDesc("geteuid", geteuidFunc)},
//     {176,  SyscallDesc("getgid", getgidFunc)},
//     {177,  SyscallDesc("getegid", getegidFunc)},
//     {178,  SyscallDesc("gettid", gettidFunc)},
//     {179,  SyscallDesc("sysinfo", sysinfoFunc<RiscvLinux64>)},
//     {180,  SyscallDesc("mq_open")},
//     {181,  SyscallDesc("mq_unlink")},
//     {182,  SyscallDesc("mq_timedsend")},
//     {183,  SyscallDesc("mq_timedrecieve")},
//     {184,  SyscallDesc("mq_notify")},
//     {185,  SyscallDesc("mq_getsetattr")},
//     {186,  SyscallDesc("msgget")},
//     {187,  SyscallDesc("msgctl")},
//     {188,  SyscallDesc("msgrcv")},
//     {189,  SyscallDesc("msgsnd")},
//     {190,  SyscallDesc("semget")},
//     {191,  SyscallDesc("semctl")},
//     {192,  SyscallDesc("semtimedop")},
//     {193,  SyscallDesc("semop")},
//     {194,  SyscallDesc("shmget")},
//     {195,  SyscallDesc("shmctl")},
//     {196,  SyscallDesc("shmat")},
//     {197,  SyscallDesc("shmdt")},
//     {198,  SyscallDesc("socket")},
//     {199,  SyscallDesc("socketpair")},
//     {200,  SyscallDesc("bind")},
//     {201,  SyscallDesc("listen")},
//     {202,  SyscallDesc("accept")},
//     {203,  SyscallDesc("connect")},
//     {204,  SyscallDesc("getsockname")},
//     {205,  SyscallDesc("getpeername")},
//     {206,  SyscallDesc("sendo")},
//     {207,  SyscallDesc("recvfrom")},
//     {208,  SyscallDesc("setsockopt")},
//     {209,  SyscallDesc("getsockopt")},
//     {210,  SyscallDesc("shutdown")},
//     {211,  SyscallDesc("sendmsg")},
//     {212,  SyscallDesc("recvmsg")},
//     {213,  SyscallDesc("readahead")},
//     {214,  SyscallDesc("brk", brkFunc)},
//     {215,  SyscallDesc("munmap", munmapFunc)},
//     {216,  SyscallDesc("mremap", mremapFunc<RiscvLinux64>)},
//     {217,  SyscallDesc("add_key")},
//     {218,  SyscallDesc("request_key")},
//     {219,  SyscallDesc("keyctl")},
//     {220,  SyscallDesc("clone", cloneFunc<RiscvLinux64>)},
//     {221,  SyscallDesc("execve", execveFunc<RiscvLinux64>)},
//     {222,  SyscallDesc("mmap", mmapFunc<RiscvLinux64>)},
//     {223,  SyscallDesc("fadvise64")},
//     {224,  SyscallDesc("swapon")},
//     {225,  SyscallDesc("swapoff")},
//     {226,  SyscallDesc("mprotect", ignoreFunc)},
//     {227,  SyscallDesc("msync", ignoreFunc)},
//     {228,  SyscallDesc("mlock", ignoreFunc)},
//     {229,  SyscallDesc("munlock", ignoreFunc)},
//     {230,  SyscallDesc("mlockall", ignoreFunc)},
//     {231,  SyscallDesc("munlockall", ignoreFunc)},
//     {232,  SyscallDesc("mincore", ignoreFunc)},
//     {233,  SyscallDesc("madvise", ignoreFunc)},
//     {234,  SyscallDesc("remap_file_pages")},
//     {235,  SyscallDesc("mbind", ignoreFunc)},
//     {236,  SyscallDesc("get_mempolicy")},
//     {237,  SyscallDesc("set_mempolicy")},
//     {238,  SyscallDesc("migrate_pages")},
//     {239,  SyscallDesc("move_pages")},
//     {240,  SyscallDesc("tgsigqueueinfo")},
//     {241,  SyscallDesc("perf_event_open")},
//     {242,  SyscallDesc("accept4")},
//     {243,  SyscallDesc("recvmmsg")},
//     {260,  SyscallDesc("wait4")},
//     {261,  SyscallDesc("prlimit64", prlimitFunc<RiscvLinux64>)},
//     {262,  SyscallDesc("fanotify_init")},
//     {263,  SyscallDesc("fanotify_mark")},
//     {264,  SyscallDesc("name_to_handle_at")},
//     {265,  SyscallDesc("open_by_handle_at")},
//     {266,  SyscallDesc("clock_adjtime")},
//     {267,  SyscallDesc("syncfs")},
//     {268,  SyscallDesc("setns")},
//     {269,  SyscallDesc("sendmmsg")},
//     {270,  SyscallDesc("process_vm_ready")},
//     {271,  SyscallDesc("process_vm_writev")},
//     {272,  SyscallDesc("kcmp")},
//     {273,  SyscallDesc("finit_module")},
//     {274,  SyscallDesc("sched_setattr")},
//     {275,  SyscallDesc("sched_getattr")},
//     {276,  SyscallDesc("renameat2")},
//     {277,  SyscallDesc("seccomp")},
//     {278,  SyscallDesc("getrandom")},
//     {279,  SyscallDesc("memfd_create")},
//     {280,  SyscallDesc("bpf")},
//     {281,  SyscallDesc("execveat")},
//     {282,  SyscallDesc("userfaultid")},
//     {283,  SyscallDesc("membarrier")},
//     {284,  SyscallDesc("mlock2")},
//     {285,  SyscallDesc("copy_file_range")},
//     {286,  SyscallDesc("preadv2")},
//     {287,  SyscallDesc("pwritev2")},
//     {1024, SyscallDesc("open", openFunc<RiscvLinux64>)},
//     {1025, SyscallDesc("link")},
//     {1026, SyscallDesc("unlink", unlinkFunc)},
//     {1027, SyscallDesc("mknod")},
//     {1028, SyscallDesc("chmod", chmodFunc<RiscvLinux64>)},
//     {1029, SyscallDesc("chown", chownFunc)},
//     {1030, SyscallDesc("mkdir", mkdirFunc)},
//     {1031, SyscallDesc("rmdir")},
//     {1032, SyscallDesc("lchown")},
//     {1033, SyscallDesc("access", accessFunc)},
//     {1034, SyscallDesc("rename", renameFunc)},
//     {1035, SyscallDesc("readlink", readlinkFunc)},
//     {1036, SyscallDesc("symlink")},
//     {1037, SyscallDesc("utimes", utimesFunc<RiscvLinux64>)},
//     {1038, SyscallDesc("stat", stat64Func<RiscvLinux64>)},
//     {1039, SyscallDesc("lstat", lstat64Func<RiscvLinux64>)},
//     {1040, SyscallDesc("pipe", pipeFunc)},
//     {1041, SyscallDesc("dup2", dup2Func)},
//     {1042, SyscallDesc("epoll_create")},
//     {1043, SyscallDesc("inotifiy_init")},
//     {1044, SyscallDesc("eventfd")},
//     {1045, SyscallDesc("signalfd")},
//     {1046, SyscallDesc("sendfile")},
//     {1047, SyscallDesc("ftruncate", ftruncate64Func)},
//     {1048, SyscallDesc("truncate", truncate64Func)},
//     {1049, SyscallDesc("stat", stat64Func<RiscvLinux64>)},
//     {1050, SyscallDesc("lstat", lstat64Func<RiscvLinux64>)},
//     {1051, SyscallDesc("fstat", fstat64Func<RiscvLinux64>)},
//     {1052, SyscallDesc("fcntl", fcntl64Func)},
//     {1053, SyscallDesc("fadvise64")},
//     {1054, SyscallDesc("newfstatat")},
//     {1055, SyscallDesc("fstatfs", fstatfsFunc<RiscvLinux64>)},
//     {1056, SyscallDesc("statfs", statfsFunc<RiscvLinux64>)},
//     {1057, SyscallDesc("lseek", lseekFunc)},
//     {1058, SyscallDesc("mmap", mmapFunc<RiscvLinux64>)},
//     {1059, SyscallDesc("alarm")},
//     {1060, SyscallDesc("getpgrp")},
//     {1061, SyscallDesc("pause")},
//     {1062, SyscallDesc("time", timeFunc<RiscvLinux64>)},
//     {1063, SyscallDesc("utime")},
//     {1064, SyscallDesc("creat")},
//     {1065, SyscallDesc("getdents")},
//     {1066, SyscallDesc("futimesat")},
//     {1067, SyscallDesc("select")},
//     {1068, SyscallDesc("poll")},
//     {1069, SyscallDesc("epoll_wait")},
//     {1070, SyscallDesc("ustat")},
//     {1071, SyscallDesc("vfork")},
//     {1072, SyscallDesc("oldwait4")},
//     {1073, SyscallDesc("recv")},
//     {1074, SyscallDesc("send")},
//     {1075, SyscallDesc("bdflush")},
//     {1076, SyscallDesc("umount")},
//     {1077, SyscallDesc("uselib")},
//     {1078, SyscallDesc("sysctl")},
//     {1079, SyscallDesc("fork")},
//     {2011, SyscallDesc("getmainvars")}
// };

// std::map<int, SyscallDesc> RiscvProcess32::syscallDescs = {
//     {0,    SyscallDesc("io_setup")},
//     {1,    SyscallDesc("io_destroy")},
//     {2,    SyscallDesc("io_submit")},
//     {3,    SyscallDesc("io_cancel")},
//     {4,    SyscallDesc("io_getevents")},
//     {5,    SyscallDesc("setxattr")},
//     {6,    SyscallDesc("lsetxattr")},
//     {7,    SyscallDesc("fsetxattr")},
//     {8,    SyscallDesc("getxattr")},
//     {9,    SyscallDesc("lgetxattr")},
//     {10,   SyscallDesc("fgetxattr")},
//     {11,   SyscallDesc("listxattr")},
//     {12,   SyscallDesc("llistxattr")},
//     {13,   SyscallDesc("flistxattr")},
//     {14,   SyscallDesc("removexattr")},
//     {15,   SyscallDesc("lremovexattr")},
//     {16,   SyscallDesc("fremovexattr")},
//     {17,   SyscallDesc("getcwd", getcwdFunc)},
//     {18,   SyscallDesc("lookup_dcookie")},
//     {19,   SyscallDesc("eventfd2")},
//     {20,   SyscallDesc("epoll_create1")},
//     {21,   SyscallDesc("epoll_ctl")},
//     {22,   SyscallDesc("epoll_pwait")},
//     {23,   SyscallDesc("dup", dupFunc)},
//     {24,   SyscallDesc("dup3")},
//     {25,   SyscallDesc("fcntl", fcntlFunc)},
//     {26,   SyscallDesc("inotify_init1")},
//     {27,   SyscallDesc("inotify_add_watch")},
//     {28,   SyscallDesc("inotify_rm_watch")},
//     {29,   SyscallDesc("ioctl", ioctlFunc<RiscvLinux32>)},
//     {30,   SyscallDesc("ioprio_get")},
//     {31,   SyscallDesc("ioprio_set")},
//     {32,   SyscallDesc("flock")},
//     {33,   SyscallDesc("mknodat")},
//     {34,   SyscallDesc("mkdirat")},
//     {35,   SyscallDesc("unlinkat", unlinkatFunc<RiscvLinux32>)},
//     {36,   SyscallDesc("symlinkat")},
//     {37,   SyscallDesc("linkat")},
//     {38,   SyscallDesc("renameat", renameatFunc<RiscvLinux32>)},
//     {39,   SyscallDesc("umount2")},
//     {40,   SyscallDesc("mount")},
//     {41,   SyscallDesc("pivot_root")},
//     {42,   SyscallDesc("nfsservctl")},
//     {43,   SyscallDesc("statfs", statfsFunc<RiscvLinux32>)},
//     {44,   SyscallDesc("fstatfs", fstatfsFunc<RiscvLinux32>)},
//     {45,   SyscallDesc("truncate", truncateFunc)},
//     {46,   SyscallDesc("ftruncate", ftruncateFunc)},
//     {47,   SyscallDesc("fallocate", fallocateFunc)},
//     {48,   SyscallDesc("faccessat", faccessatFunc<RiscvLinux32>)},
//     {49,   SyscallDesc("chdir")},
//     {50,   SyscallDesc("fchdir")},
//     {51,   SyscallDesc("chroot")},
//     {52,   SyscallDesc("fchmod", fchmodFunc<RiscvLinux32>)},
//     {53,   SyscallDesc("fchmodat")},
//     {54,   SyscallDesc("fchownat")},
//     {55,   SyscallDesc("fchown", fchownFunc)},
//     {56,   SyscallDesc("openat", openatFunc<RiscvLinux32>)},
//     {57,   SyscallDesc("close", closeFunc)},
//     {58,   SyscallDesc("vhangup")},
//     {59,   SyscallDesc("pipe2")},
//     {60,   SyscallDesc("quotactl")},
//     {61,   SyscallDesc("getdents64")},
//     {62,   SyscallDesc("lseek", lseekFunc)},
//     {63,   SyscallDesc("read", readFunc<RiscvLinux32>)},
//     {64,   SyscallDesc("write", writeFunc<RiscvLinux32>)},
//     {66,   SyscallDesc("writev", writevFunc<RiscvLinux32>)},
//     {67,   SyscallDesc("pread64")},
//     {68,   SyscallDesc("pwrite64", pwrite64Func<RiscvLinux32>)},
//     {69,   SyscallDesc("preadv")},
//     {70,   SyscallDesc("pwritev")},
//     {71,   SyscallDesc("sendfile")},
//     {72,   SyscallDesc("pselect6")},
//     {73,   SyscallDesc("ppoll")},
//     {74,   SyscallDesc("signalfd64")},
//     {75,   SyscallDesc("vmsplice")},
//     {76,   SyscallDesc("splice")},
//     {77,   SyscallDesc("tee")},
//     {78,   SyscallDesc("readlinkat", readlinkatFunc<RiscvLinux32>)},
//     {79,   SyscallDesc("fstatat")},
//     {80,   SyscallDesc("fstat", fstatFunc<RiscvLinux32>)},
//     {81,   SyscallDesc("sync")},
//     {82,   SyscallDesc("fsync")},
//     {83,   SyscallDesc("fdatasync")},
//     {84,   SyscallDesc("sync_file_range2")},
//     {85,   SyscallDesc("timerfd_create")},
//     {86,   SyscallDesc("timerfd_settime")},
//     {87,   SyscallDesc("timerfd_gettime")},
//     {88,   SyscallDesc("utimensat")},
//     {89,   SyscallDesc("acct")},
//     {90,   SyscallDesc("capget")},
//     {91,   SyscallDesc("capset")},
//     {92,   SyscallDesc("personality")},
//     {93,   SyscallDesc("exit", exitFunc)},
//     {94,   SyscallDesc("exit_group", exitGroupFunc)},
//     {95,   SyscallDesc("waitid")},
//     {96,   SyscallDesc("set_tid_address", setTidAddressFunc)},
//     {97,   SyscallDesc("unshare")},
//     {98,   SyscallDesc("futex", futexFunc<RiscvLinux32>)},
//     {99,   SyscallDesc("set_robust_list", ignoreWarnOnceFunc)},
//     {100,  SyscallDesc("get_robust_list", ignoreWarnOnceFunc)},
//     {101,  SyscallDesc("nanosleep")},
//     {102,  SyscallDesc("getitimer")},
//     {103,  SyscallDesc("setitimer")},
//     {104,  SyscallDesc("kexec_load")},
//     {105,  SyscallDesc("init_module")},
//     {106,  SyscallDesc("delete_module")},
//     {107,  SyscallDesc("timer_create")},
//     {108,  SyscallDesc("timer_gettime")},
//     {109,  SyscallDesc("timer_getoverrun")},
//     {110,  SyscallDesc("timer_settime")},
//     {111,  SyscallDesc("timer_delete")},
//     {112,  SyscallDesc("clock_settime")},
//     {113,  SyscallDesc("clock_gettime", clock_gettimeFunc<RiscvLinux32>)},
//     {114,  SyscallDesc("clock_getres", clock_getresFunc<RiscvLinux32>)},
//     {115,  SyscallDesc("clock_nanosleep")},
//     {116,  SyscallDesc("syslog")},
//     {117,  SyscallDesc("ptrace")},
//     {118,  SyscallDesc("sched_setparam")},
//     {119,  SyscallDesc("sched_setscheduler")},
//     {120,  SyscallDesc("sched_getscheduler")},
//     {121,  SyscallDesc("sched_getparam")},
//     {122,  SyscallDesc("sched_setaffinity")},
//     {123,  SyscallDesc("sched_getaffinity")},
//     {124,  SyscallDesc("sched_yield", ignoreWarnOnceFunc)},
//     {125,  SyscallDesc("sched_get_priority_max")},
//     {126,  SyscallDesc("sched_get_priority_min")},
//     {127,  SyscallDesc("scheD_rr_get_interval")},
//     {128,  SyscallDesc("restart_syscall")},
//     {129,  SyscallDesc("kill")},
//     {130,  SyscallDesc("tkill")},
//     {131,  SyscallDesc("tgkill", tgkillFunc<RiscvLinux32>)},
//     {132,  SyscallDesc("sigaltstack")},
//     {133,  SyscallDesc("rt_sigsuspend", ignoreWarnOnceFunc)},
//     {134,  SyscallDesc("rt_sigaction", ignoreWarnOnceFunc)},
//     {135,  SyscallDesc("rt_sigprocmask", ignoreWarnOnceFunc)},
//     {136,  SyscallDesc("rt_sigpending", ignoreWarnOnceFunc)},
//     {137,  SyscallDesc("rt_sigtimedwait", ignoreWarnOnceFunc)},
//     {138,  SyscallDesc("rt_sigqueueinfo", ignoreWarnOnceFunc)},
//     {139,  SyscallDesc("rt_sigreturn", ignoreWarnOnceFunc)},
//     {140,  SyscallDesc("setpriority")},
//     {141,  SyscallDesc("getpriority")},
//     {142,  SyscallDesc("reboot")},
//     {143,  SyscallDesc("setregid")},
//     {144,  SyscallDesc("setgid")},
//     {145,  SyscallDesc("setreuid")},
//     {146,  SyscallDesc("setuid", setuidFunc)},
//     {147,  SyscallDesc("setresuid")},
//     {148,  SyscallDesc("getresuid")},
//     {149,  SyscallDesc("getresgid")},
//     {150,  SyscallDesc("getresgid")},
//     {151,  SyscallDesc("setfsuid")},
//     {152,  SyscallDesc("setfsgid")},
//     {153,  SyscallDesc("times", timesFunc<RiscvLinux32>)},
//     {154,  SyscallDesc("setpgid", setpgidFunc)},
//     {155,  SyscallDesc("getpgid")},
//     {156,  SyscallDesc("getsid")},
//     {157,  SyscallDesc("setsid")},
//     {158,  SyscallDesc("getgroups")},
//     {159,  SyscallDesc("setgroups")},
//     {160,  SyscallDesc("uname", unameFunc32)},
//     {161,  SyscallDesc("sethostname")},
//     {162,  SyscallDesc("setdomainname")},
//     {163,  SyscallDesc("getrlimit", getrlimitFunc<RiscvLinux32>)},
//     {164,  SyscallDesc("setrlimit", ignoreFunc)},
//     {165,  SyscallDesc("getrusage", getrusageFunc<RiscvLinux32>)},
//     {166,  SyscallDesc("umask", umaskFunc)},
//     {167,  SyscallDesc("prctl")},
//     {168,  SyscallDesc("getcpu")},
//     {169,  SyscallDesc("gettimeofday", gettimeofdayFunc<RiscvLinux32>)},
//     {170,  SyscallDesc("settimeofday")},
//     {171,  SyscallDesc("adjtimex")},
//     {172,  SyscallDesc("getpid", getpidFunc)},
//     {173,  SyscallDesc("getppid", getppidFunc)},
//     {174,  SyscallDesc("getuid", getuidFunc)},
//     {175,  SyscallDesc("geteuid", geteuidFunc)},
//     {176,  SyscallDesc("getgid", getgidFunc)},
//     {177,  SyscallDesc("getegid", getegidFunc)},
//     {178,  SyscallDesc("gettid", gettidFunc)},
//     {179,  SyscallDesc("sysinfo", sysinfoFunc<RiscvLinux32>)},
//     {180,  SyscallDesc("mq_open")},
//     {181,  SyscallDesc("mq_unlink")},
//     {182,  SyscallDesc("mq_timedsend")},
//     {183,  SyscallDesc("mq_timedrecieve")},
//     {184,  SyscallDesc("mq_notify")},
//     {185,  SyscallDesc("mq_getsetattr")},
//     {186,  SyscallDesc("msgget")},
//     {187,  SyscallDesc("msgctl")},
//     {188,  SyscallDesc("msgrcv")},
//     {189,  SyscallDesc("msgsnd")},
//     {190,  SyscallDesc("semget")},
//     {191,  SyscallDesc("semctl")},
//     {192,  SyscallDesc("semtimedop")},
//     {193,  SyscallDesc("semop")},
//     {194,  SyscallDesc("shmget")},
//     {195,  SyscallDesc("shmctl")},
//     {196,  SyscallDesc("shmat")},
//     {197,  SyscallDesc("shmdt")},
//     {198,  SyscallDesc("socket")},
//     {199,  SyscallDesc("socketpair")},
//     {200,  SyscallDesc("bind")},
//     {201,  SyscallDesc("listen")},
//     {202,  SyscallDesc("accept")},
//     {203,  SyscallDesc("connect")},
//     {204,  SyscallDesc("getsockname")},
//     {205,  SyscallDesc("getpeername")},
//     {206,  SyscallDesc("sendo")},
//     {207,  SyscallDesc("recvfrom")},
//     {208,  SyscallDesc("setsockopt")},
//     {209,  SyscallDesc("getsockopt")},
//     {210,  SyscallDesc("shutdown")},
//     {211,  SyscallDesc("sendmsg")},
//     {212,  SyscallDesc("recvmsg")},
//     {213,  SyscallDesc("readahead")},
//     {214,  SyscallDesc("brk", brkFunc)},
//     {215,  SyscallDesc("munmap", munmapFunc)},
//     {216,  SyscallDesc("mremap", mremapFunc<RiscvLinux32>)},
//     {217,  SyscallDesc("add_key")},
//     {218,  SyscallDesc("request_key")},
//     {219,  SyscallDesc("keyctl")},
//     {220,  SyscallDesc("clone", cloneFunc<RiscvLinux32>)},
//     {221,  SyscallDesc("execve", execveFunc<RiscvLinux32>)},
//     {222,  SyscallDesc("mmap", mmapFunc<RiscvLinux32>)},
//     {223,  SyscallDesc("fadvise64")},
//     {224,  SyscallDesc("swapon")},
//     {225,  SyscallDesc("swapoff")},
//     {226,  SyscallDesc("mprotect", ignoreFunc)},
//     {227,  SyscallDesc("msync", ignoreFunc)},
//     {228,  SyscallDesc("mlock", ignoreFunc)},
//     {229,  SyscallDesc("munlock", ignoreFunc)},
//     {230,  SyscallDesc("mlockall", ignoreFunc)},
//     {231,  SyscallDesc("munlockall", ignoreFunc)},
//     {232,  SyscallDesc("mincore", ignoreFunc)},
//     {233,  SyscallDesc("madvise", ignoreFunc)},
//     {234,  SyscallDesc("remap_file_pages")},
//     {235,  SyscallDesc("mbind", ignoreFunc)},
//     {236,  SyscallDesc("get_mempolicy")},
//     {237,  SyscallDesc("set_mempolicy")},
//     {238,  SyscallDesc("migrate_pages")},
//     {239,  SyscallDesc("move_pages")},
//     {240,  SyscallDesc("tgsigqueueinfo")},
//     {241,  SyscallDesc("perf_event_open")},
//     {242,  SyscallDesc("accept4")},
//     {243,  SyscallDesc("recvmmsg")},
//     {260,  SyscallDesc("wait4")},
//     {261,  SyscallDesc("prlimit64", prlimitFunc<RiscvLinux32>)},
//     {262,  SyscallDesc("fanotify_init")},
//     {263,  SyscallDesc("fanotify_mark")},
//     {264,  SyscallDesc("name_to_handle_at")},
//     {265,  SyscallDesc("open_by_handle_at")},
//     {266,  SyscallDesc("clock_adjtime")},
//     {267,  SyscallDesc("syncfs")},
//     {268,  SyscallDesc("setns")},
//     {269,  SyscallDesc("sendmmsg")},
//     {270,  SyscallDesc("process_vm_ready")},
//     {271,  SyscallDesc("process_vm_writev")},
//     {272,  SyscallDesc("kcmp")},
//     {273,  SyscallDesc("finit_module")},
//     {274,  SyscallDesc("sched_setattr")},
//     {275,  SyscallDesc("sched_getattr")},
//     {276,  SyscallDesc("renameat2")},
//     {277,  SyscallDesc("seccomp")},
//     {278,  SyscallDesc("getrandom")},
//     {279,  SyscallDesc("memfd_create")},
//     {280,  SyscallDesc("bpf")},
//     {281,  SyscallDesc("execveat")},
//     {282,  SyscallDesc("userfaultid")},
//     {283,  SyscallDesc("membarrier")},
//     {284,  SyscallDesc("mlock2")},
//     {285,  SyscallDesc("copy_file_range")},
//     {286,  SyscallDesc("preadv2")},
//     {287,  SyscallDesc("pwritev2")},
//     {1024, SyscallDesc("open", openFunc<RiscvLinux32>)},
//     {1025, SyscallDesc("link")},
//     {1026, SyscallDesc("unlink", unlinkFunc)},
//     {1027, SyscallDesc("mknod")},
//     {1028, SyscallDesc("chmod", chmodFunc<RiscvLinux32>)},
//     {1029, SyscallDesc("chown", chownFunc)},
//     {1030, SyscallDesc("mkdir", mkdirFunc)},
//     {1031, SyscallDesc("rmdir")},
//     {1032, SyscallDesc("lchown")},
//     {1033, SyscallDesc("access", accessFunc)},
//     {1034, SyscallDesc("rename", renameFunc)},
//     {1035, SyscallDesc("readlink", readlinkFunc)},
//     {1036, SyscallDesc("symlink")},
//     {1037, SyscallDesc("utimes", utimesFunc<RiscvLinux32>)},
//     {1038, SyscallDesc("stat", statFunc<RiscvLinux32>)},
//     {1039, SyscallDesc("lstat", lstatFunc<RiscvLinux32>)},
//     {1040, SyscallDesc("pipe", pipeFunc)},
//     {1041, SyscallDesc("dup2", dup2Func)},
//     {1042, SyscallDesc("epoll_create")},
//     {1043, SyscallDesc("inotifiy_init")},
//     {1044, SyscallDesc("eventfd")},
//     {1045, SyscallDesc("signalfd")},
//     {1046, SyscallDesc("sendfile")},
//     {1047, SyscallDesc("ftruncate", ftruncateFunc)},
//     {1048, SyscallDesc("truncate", truncateFunc)},
//     {1049, SyscallDesc("stat", statFunc<RiscvLinux32>)},
//     {1050, SyscallDesc("lstat", lstatFunc<RiscvLinux32>)},
//     {1051, SyscallDesc("fstat", fstatFunc<RiscvLinux32>)},
//     {1052, SyscallDesc("fcntl", fcntlFunc)},
//     {1053, SyscallDesc("fadvise64")},
//     {1054, SyscallDesc("newfstatat")},
//     {1055, SyscallDesc("fstatfs", fstatfsFunc<RiscvLinux32>)},
//     {1056, SyscallDesc("statfs", statfsFunc<RiscvLinux32>)},
//     {1057, SyscallDesc("lseek", lseekFunc)},
//     {1058, SyscallDesc("mmap", mmapFunc<RiscvLinux32>)},
//     {1059, SyscallDesc("alarm")},
//     {1060, SyscallDesc("getpgrp")},
//     {1061, SyscallDesc("pause")},
//     {1062, SyscallDesc("time", timeFunc<RiscvLinux32>)},
//     {1063, SyscallDesc("utime")},
//     {1064, SyscallDesc("creat")},
//     {1065, SyscallDesc("getdents")},
//     {1066, SyscallDesc("futimesat")},
//     {1067, SyscallDesc("select")},
//     {1068, SyscallDesc("poll")},
//     {1069, SyscallDesc("epoll_wait")},
//     {1070, SyscallDesc("ustat")},
//     {1071, SyscallDesc("vfork")},
//     {1072, SyscallDesc("oldwait4")},
//     {1073, SyscallDesc("recv")},
//     {1074, SyscallDesc("send")},
//     {1075, SyscallDesc("bdflush")},
//     {1076, SyscallDesc("umount")},
//     {1077, SyscallDesc("uselib")},
//     {1078, SyscallDesc("sysctl")},
//     {1079, SyscallDesc("fork")},
//     {2011, SyscallDesc("getmainvars")}
// };

RiscvProcess::RiscvProcess(const ProcessParams &params,
        loader::ObjectFile *objFile) :
        Process(params,
                new EmulationPageTable(params.name, params.pid, PageBytes),
                objFile)
{
    fatal_if(params.useArchPT, "Arch page tables not implemented.");
}

RiscvProcess64::RiscvProcess64(const ProcessParams &params,
        loader::ObjectFile *objFile) :
        RiscvProcess(params, objFile)
{
    const Addr stack_base = 0x7FFFFFFFFFFFFFFFL;
    const Addr max_stack_size = 8 * 1024 * 1024;
    const Addr next_thread_stack_base = stack_base - max_stack_size;
    const Addr brk_point = roundUp(image.maxAddr(), PageBytes);
    const Addr mmap_end = 0x4000000000000000L;
    memState = std::make_shared<MemState>(this, brk_point, stack_base,
            max_stack_size, next_thread_stack_base, mmap_end);
}

RiscvProcess32::RiscvProcess32(const ProcessParams &params,
        loader::ObjectFile *objFile) :
        RiscvProcess(params, objFile)
{
    const Addr stack_base = 0x7FFFFFFF;
    const Addr max_stack_size = 8 * 1024 * 1024;
    const Addr next_thread_stack_base = stack_base - max_stack_size;
    const Addr brk_point = roundUp(image.maxAddr(), PageBytes);
    const Addr mmap_end = 0x40000000L;
    memState = std::make_shared<MemState>(this, brk_point, stack_base,
            max_stack_size, next_thread_stack_base, mmap_end);
}

void
RiscvProcess64::initState()
{
    Process::initState();

    argsInit<uint64_t>(PageBytes);
    for (ContextID ctx: contextIds)
        system->threads[ctx]->setMiscRegNoEffect(MISCREG_PRV, PRV_U);
}

void
RiscvProcess32::initState()
{
    Process::initState();

    argsInit<uint32_t>(PageBytes);
    for (ContextID ctx: contextIds) {
        auto *tc = system->threads[ctx];
        tc->setMiscRegNoEffect(MISCREG_PRV, PRV_U);
        PCState pc = tc->pcState();
        pc.rv32(true);
        tc->pcState(pc);
    }
}

template<class IntType> void
RiscvProcess::argsInit(int pageSize)
{
    const int RandomBytes = 16;
    const int addrSize = sizeof(IntType);

    auto *elfObject = dynamic_cast<loader::ElfObject*>(objFile);
    memState->setStackMin(memState->getStackBase());

    // Determine stack size and populate auxv
    Addr stack_top = memState->getStackMin();
    stack_top -= RandomBytes;
    for (const std::string& arg: argv)
        stack_top -= arg.size() + 1;
    for (const std::string& env: envp)
        stack_top -= env.size() + 1;
    stack_top &= -addrSize;

    std::vector<gem5::auxv::AuxVector<IntType>> auxv;
    if (elfObject != nullptr) {
        auxv.emplace_back(gem5::auxv::Entry, objFile->entryPoint());
        auxv.emplace_back(gem5::auxv::Phnum, elfObject->programHeaderCount());
        auxv.emplace_back(gem5::auxv::Phent, elfObject->programHeaderSize());
        auxv.emplace_back(gem5::auxv::Phdr, elfObject->programHeaderTable());
        auxv.emplace_back(gem5::auxv::Pagesz, PageBytes);
        auxv.emplace_back(gem5::auxv::Secure, 0);
        auxv.emplace_back(gem5::auxv::Random, stack_top);
        auxv.emplace_back(gem5::auxv::Null, 0);
    }
    stack_top -= (1 + argv.size()) * addrSize +
                   (1 + envp.size()) * addrSize +
                   addrSize + 2 * sizeof(IntType) * auxv.size();
    stack_top &= -2*addrSize;
    memState->setStackSize(memState->getStackBase() - stack_top);
    memState->mapRegion(roundDown(stack_top, pageSize),
                        roundUp(memState->getStackSize(), pageSize), "stack");

    // Copy random bytes (for AT_RANDOM) to stack
    memState->setStackMin(memState->getStackMin() - RandomBytes);
    uint8_t at_random[RandomBytes];
    std::generate(std::begin(at_random), std::end(at_random),
                  [&]{ return random_mt.random(0, 0xFF); });
    initVirtMem->writeBlob(memState->getStackMin(), at_random, RandomBytes);

    // Copy argv to stack
    std::vector<Addr> argPointers;
    for (const std::string& arg: argv) {
        memState->setStackMin(memState->getStackMin() - (arg.size() + 1));
        initVirtMem->writeString(memState->getStackMin(), arg.c_str());
        argPointers.push_back(memState->getStackMin());
        if (debug::Stack) {
            std::string wrote;
            initVirtMem->readString(wrote, argPointers.back());
            DPRINTFN("Wrote arg \"%s\" to address %p\n",
                    wrote, (void*)memState->getStackMin());
        }
    }
    argPointers.push_back(0);

    // Copy envp to stack
    std::vector<Addr> envPointers;
    for (const std::string& env: envp) {
        memState->setStackMin(memState->getStackMin() - (env.size() + 1));
        initVirtMem->writeString(memState->getStackMin(), env.c_str());
        envPointers.push_back(memState->getStackMin());
        DPRINTF(Stack, "Wrote env \"%s\" to address %p\n",
                env, (void*)memState->getStackMin());
    }
    envPointers.push_back(0);

    // Align stack
    memState->setStackMin(memState->getStackMin() & -addrSize);

    // Calculate bottom of stack
    memState->setStackMin(memState->getStackMin() -
            ((1 + argv.size()) * addrSize +
             (1 + envp.size()) * addrSize +
             addrSize + 2 * sizeof(IntType) * auxv.size()));
    memState->setStackMin(memState->getStackMin() & (-2 * addrSize));
    Addr sp = memState->getStackMin();
    const auto pushOntoStack =
        [this, &sp](IntType data) {
            initVirtMem->write(sp, data, ByteOrder::little);
            sp += sizeof(data);
        };

    // Push argc and argv pointers onto stack
    IntType argc = argv.size();
    DPRINTF(Stack, "Wrote argc %d to address %#x\n", argc, sp);
    pushOntoStack(argc);

    for (const Addr& argPointer: argPointers) {
        DPRINTF(Stack, "Wrote argv pointer %#x to address %#x\n",
                argPointer, sp);
        pushOntoStack(argPointer);
    }

    // Push env pointers onto stack
    for (const Addr& envPointer: envPointers) {
        DPRINTF(Stack, "Wrote envp pointer %#x to address %#x\n",
                envPointer, sp);
        pushOntoStack(envPointer);
    }

    // Push aux vector onto stack
    std::map<IntType, std::string> aux_keys = {
        {gem5::auxv::Entry, "gem5::auxv::Entry"},
        {gem5::auxv::Phnum, "gem5::auxv::Phnum"},
        {gem5::auxv::Phent, "gem5::auxv::Phent"},
        {gem5::auxv::Phdr, "gem5::auxv::Phdr"},
        {gem5::auxv::Pagesz, "gem5::auxv::Pagesz"},
        {gem5::auxv::Secure, "gem5::auxv::Secure"},
        {gem5::auxv::Random, "gem5::auxv::Random"},
        {gem5::auxv::Null, "gem5::auxv::Null"}
    };
    for (const auto &aux: auxv) {
        DPRINTF(Stack, "Wrote aux key %s to address %#x\n",
                aux_keys[aux.type], sp);
        pushOntoStack(aux.type);
        DPRINTF(Stack, "Wrote aux value %x to address %#x\n", aux.val, sp);
        pushOntoStack(aux.val);
    }

    ThreadContext *tc = system->threads[contextIds[0]];
    tc->setIntReg(StackPointerReg, memState->getStackMin());
    tc->pcState(getStartPC());

    memState->setStackMin(roundDown(memState->getStackMin(), pageSize));
}

RegVal
RiscvProcess::getSyscallArg(ThreadContext *tc, int &i)
{
    // If a larger index is requested than there are syscall argument
    // registers, return 0
    printf("Riscv process getting SyscallArg\n");
    RegVal retval = 0;
    if (i < SyscallArgumentRegs.size())
        retval = tc->readIntReg(SyscallArgumentRegs[i]);
    i++;
    return retval;
}

void
RiscvProcess::setSyscallReturn(ThreadContext *tc, SyscallReturn sysret)
{
    if (sysret.successful()) {
        // no error
        tc->setIntReg(SyscallPseudoReturnReg, sysret.returnValue());
    } else {
        // got an error, return details
        tc->setIntReg(SyscallPseudoReturnReg, sysret.encodedValue());
    }
}

// SyscallDesc*
// RiscvProcess::getDesc(int callnum)
// {
//     return syscallDescs.find(callnum) != syscallDescs.end() ?
//         &syscallDescs.at(callnum) : nullptr;
// }

} // namespace gem5
