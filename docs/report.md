# 2019T2 COMP6447
# Assignment: Rootkit

# kernelScRuBs
- Jasper Lowell (z5180332)
- Wesley Hamburger (z5017795)
- David Morris (z5115881)

# Rootkit functionality
## Rootkit installation

The rootkit is installed as a kernel module. The kernel module hooks a number
of system calls.

## How the rootkit hides itself

Generally, files are hidden by checking the filename against the file
currently being operated on via a system call such as `getdirentries`. The
file name is not the file path. This means that *all* files with the same
name, regardless of location are hidden. This rootkit traverses the vnode
provided by the FreeBSD VFS and evaluates the exact path of the files being
operated on to ensure that only the exact file is hidden. This is the most
thorough and technically correct approach.

# Rootkit detection

The rootkit hooks a number of system calls. Some of the operations it needs to
perform are time intensive and doing analysis on the time taken to perform
certain system calls may reveal that the system calls have been extended.
