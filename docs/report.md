
Rootkit Design 
==========================

__Installation__

Our rootkit installs itself by using a Kernel Loadable Module. On installation it sets up two system call hooks which will be described below. These are used to achieve the target functionality of privledge escalation. In order to evade detection we remove linker files and modules. This is done in the `MOD_LOAD` section of the load callback. 

__Privledge Escalation__

The FreeBSD implementation assigns user credentials on [per-thread](https://wiki.freebsd.org/Per-Thread%20Credentials) basis. This indicates that in order to modify the privledges of a running thread we need to locate the data structure which contains the user privledges and provide a method that modifies these fields.

The thread data structure is defined in [proc.h](https://github.com/freebsd/freebsd/blob/master/sys/sys/proc.h). Inside the structure you find the following:

```c
struct ucred	*td_ucred;	/* (k) Reference to credentials. */
```

Following this chain we get [ucred.h](https://github.com/freebsd/freebsd/blob/master/sys/sys/ucred.h). Which reveals the ucred data structure. 

```c
	uid_t	cr_uid;			/* effective user id */
	uid_t	cr_ruid;		/* real user id */
	uid_t	cr_svuid;		/* saved user id */
	int	cr_ngroups;		/* number of groups */
	gid_t	cr_rgid;		/* real group id */
	gid_t	cr_svgid;		/* saved group id */
```

Therefore in order to modify the credentials of the thread we would need to modify these items within the data strucutre. 

In the end we created some functionality which hooked `mkdir`. Upon recieving a call `mkdir HASH`, where hash is a sha256 hash which uniquely identifies the escalation request. The running process would become a root user by modifying the data structure above to contain the id of root.

__Concealment__ 

Our rootkit design is simple. We hook system calls to achieve the desired functionality and load a KLM in order to run our rootkit. In this case there are many ways to detect this functionality. When loading a KLM it creates a linker file and module. Refereces to these objects are stored in data structures in kernel memory. These data structures can be found in [sys/linker.h](https://github.com/freebsd/freebsd/blob/master/sys/sys/module.h), [sys/module.h](https://github.com/freebsd/freebsd/blob/master/sys/sys/module.h), [kern_linker.c](https://github.com/freebsd/freebsd/blob/master/sys/kern/kern_linker.c) and [kern_module.c](https://github.com/freebsd/freebsd/blob/master/sys/kern/kern_module.c).  

Using the macros provided by FreeBSD we are able to iterate over these data structures and remove references to the linker file  and modules that indiciate the presence of our rootkit. Not only do we need to remove the data structures from these lists but some associated counts. That is `nextid` and `next_file_id`. 






