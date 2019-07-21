
Rootkit Check-In Documentation
==============================

__Installation__

The rootkit functions as a Kernel Loadable Module. When installed, it
establishes an interface by hooking a common system call. This is used to
accept requests from authorised processes. In addition, the rootkit takes
measures to conceal its presence.

__Interface__

The rootkit provides an extendable interface for accepting requests from
authorised processes. The `mkdir` system call is hooked by the rootkit such
that an authorised process can perform `mkdir $HASH` to make a request. The
high randomness ensures that only authorised processes have access to the
rootkit's functionality.

__Privilege Escalation__

FreeBSD assigns user credentials on a [per-thread](https://wiki.freebsd.org/Per-Thread%20Credentials)
basis. Authorised threads can request the rootkit to escalate it's privilege.
This is implemented by modifying the permissions struct and setting the
threads user id and group id to that of root.

The thread data structure is defined in [proc.h](https://github.com/freebsd/freebsd/blob/master/sys/sys/proc.h).
Within the thread data structure the following can be found:

```c
struct ucred	*td_ucred;	/* (k) Reference to credentials. */
```

The fields for `td_ucred` indicate how permissions are handled for each thread.

```c
	uid_t	cr_uid;			/* effective user id */
	uid_t	cr_ruid;		/* real user id */
	uid_t	cr_svuid;		/* saved user id */
	int	cr_ngroups;		/* number of groups */
	gid_t	cr_rgid;		/* real group id */
	gid_t	cr_svgid;		/* saved group id */
```

These are updated by the rootkit to provide privilege escalation.
The hash required to instruct the rootkit to perform this operation can be
found in `config.h`.

__Concealment__

There are two primary things to hide to significantly increase the difficulty
of detecting the rootkit - the linker file and the module.

Refereces to these objects are stored in data structures in kernel memory.
These data structures can be found in
[sys/linker.h](https://github.com/freebsd/freebsd/blob/master/sys/sys/module.h),
[sys/module.h](https://github.com/freebsd/freebsd/blob/master/sys/sys/module.h),
[kern_linker.c](https://github.com/freebsd/freebsd/blob/master/sys/kern/kern_linker.c) and
[kern_module.c](https://github.com/freebsd/freebsd/blob/master/sys/kern/kern_module.c).

Using existing macros within FreeBSD the rootkit iterates over these data
structures and remove references to the linker file and the module.
Additionally, the rootkit removes associated counts. That is `nextid` and
`next_file_id`.
