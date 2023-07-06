#ifndef _HAPROXY_CPUSET_T_H
#define _HAPROXY_CPUSET_T_H

#define _GNU_SOURCE
#include <sched.h>

#if defined(__FreeBSD__) || defined(__DragonFly__) || defined(__NetBSD__)
#include <sys/param.h>
#ifdef __FreeBSD__
#include <sys/_cpuset.h>
#include <sys/cpuset.h>
#include <sys/sysctl.h>
#include <strings.h>
#endif
#endif

#include <haproxy/api-t.h>

#if defined(__linux__) || defined(__DragonFly__) || \
  (defined(__FreeBSD_kernel__) && defined(__GLIBC__))

# define CPUSET_REPR cpu_set_t
# define CPUSET_USE_CPUSET

#elif defined(__FreeBSD__) || defined(__NetBSD__)

# define CPUSET_REPR cpuset_t

# if defined(__FreeBSD__) && __FreeBSD_version >= 1301000
#  define CPUSET_USE_CPUSET
# else
#  define CPUSET_USE_FREEBSD_CPUSET
# endif

#elif defined(__APPLE__)

# define CPUSET_REPR unsigned long
# define CPUSET_USE_ULONG

#else

# error "No cpuset support implemented on this platform"

#endif

struct hap_cpuset {
	CPUSET_REPR cpuset;
};

struct cpu_map {
	struct hap_cpuset proc;                 /* list of CPU masks for the whole thread group */
	struct hap_cpuset thread[MAX_THREADS_PER_GROUP];  /* list of CPU masks for the 32/64 threads of this group */
};

/* hardware CPU state flags used with CPU topology detection */

#define HW_CPU_F_BOUND  0x0001  // the process was bound to this CPU at boot
#define HW_CPU_F_ONLINE 0x0002  // this CPU is online

/* hardware CPU descriptor. All the ID and IDX fields are initialized to -1
 * when not known. The identifiers there are mostly assigned on the fly using
 * increments and have no particular representation except the fact that CPUs
 * having the same ID there share the same designated resource. The flags are
 * preset to zero.
 */
struct hw_cpu {
	ushort st;   // state flags (HW_CPU_F_*)
	short idx;   // CPU index as passed to the OS. Initially the entry index.
	short l1_id; // L1 cache identifier
	short l2_id; // L2 cache identifier
	short l3_id; // L3 cache slice identifier
	short ts_id; // thread-set identifier (generally core number)
	short cl_id; // cluster identifier (group of more shortimate cores)
	short no_id; // NUMA node identifier
	short pk_id; // package identifier
	short tg_id; // thread group ID
};

#endif /* _HAPROXY_CPUSET_T_H */
