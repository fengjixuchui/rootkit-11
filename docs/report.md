# 2019T2 COMP6447
# Assignment: Rootkit

# kernelScRuBs
- Jasper Lowell (z5180332)
- Wesley Hamburger (z5017795)
- David Morris (z5115881)

# Rootkit functionality

## Rootkit installation
The rootkit is installed as a kernel module and requires root privileges.

## Privilege Escalation
Privilege escalation is provided through the rootkit API by making a RKCALL.
RKCALLs are defined in `config.h`. An RKCALL can be made by calling `mkdir`
with a specific name for the directory.

## Concealment
To ensure that the rootkit API is not accidentally triggered by
unsuspecting users, long 256-bit hashes are used as identification keys
for commands. The high entropy provides reliable authentication.

The rootkit modifies in kernel memory objects to prevent the rootkit
kernel module from being detected through commands such as `kldstat`. The
in kernel memory objects includes the loaded module list and the linker list.
When iterating through these lists the nodes identifying the rootkit can
be removed using macros.

The rootkit makes use of system call hooking. Simple system call hooking can
be detected easily by iterating through the system call function pointer
table and comparing the function pointers against the legitiment values.
This is especially easy considering that symbols such as `sys_mkdir` are
available. To avoid being detected so easily, the rootkit uses live in-line
patching of the legitiment functions to transfer control to rootkit. This
ensures that should a rootkit detector iterate through the system call
function pointer table, it will not detect the hooking.

Generally, files are hidden by checking the filename against the file
currently being operated on via a system call such as `getdirentries`. The
file name is not the file path. This means that *all* files with the same
name, regardless of location are hidden. This rootkit traverses the vnode
provided by the FreeBSD VFS and evaluates the exact path of the files being
operated on to ensure that only the exact file is hidden. This is the most
thorough and technically correct approach.

On FreeBSD it is possible to read directories as if they are files. Hooking
the `getdirentries` system call is not enough to hide the existence of files.
Should the parent directory of a hidden file be read, the name of the hidden
file will be exposed. This will not match the contents of `ls` on the parent
directory and will expose the existence of the rootkit. Along with hooking
the `getdirentries` system call we also hook `read` and modify the returned
buffer when the file being read is a parent directory of a hidden file.

Along with hidden files, the rootkit provides something called "transparent
files". The rootkit relies on files such as `/boot/loader.conf` to provide
features like persistence. Hiding these files completely would give away to
a user that a rootkit has infected the system. Instead, the contents of
these files are hidden. This introduces a new problem, what happens when a
user edits them? If the edits aren't visible then the user may suspect that
a rootkit has infected the system. To solve this, the rootkit hooks the
opening system calls for these transparent files, closes them, and opens
a file that is called the same but has `.transparent` appended to the file
name. The `.transparent` file is completely hidden but is accessed by the
user when modifying the original file. This ensures that the contents of
transparent files like `loader.conf`, which contain data important for the
rootkit to maintain functionality, is not corrupted.

# Rootkit detection

The rootkit hooks a number of system calls. Some of the operations it needs to
perform are time intensive and doing analysis on the time taken to perform
certain system calls may reveal that the system calls have been extended.

While inline patching makes it more difficult for a rootkit detector to detect
the rootkit, it's not undetectable. Should a rootkit detector inspect the
memory of functions such as `sys_openat` and check if one of the first
instructions is a jump, then it will detect the rootkit. Alternatively,
the rootkit detector could also compare a hash of what should be in memory
for those functions along with a hash of what is in memory at runtime.

FreeBSD does not track paths along with the files they belong to. The rootkit
uses the VFS cache to generate a path for a vnode. This is not guaranteed to be
successful due to limitations in the kernel. If files such as
`/boot/loader.conf` are under heavy editing, its possible that the changes
will not reflect in the file. This can happen when file descriptors are cloned
rather than created via `sys_openat`. There are no good solutions to this
problem. The typical solution of handling hidden files is to hide all files
with a specific name. This, however, hides *all* files with that name
regardless of where they are in the file system. This is not ideal because
legitiment files created by the user could be hidden.
