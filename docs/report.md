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

The rootkit modifies in kernel memory objects to prevent the rootkit
kernel module from being detected through commands such as `kldstat`.

To ensure that the rootkit API is not accidentally triggered by
unsuspecting users, long 256-bit hashes are used as identification keys
for commands. The high entropy provides reliable authentication.

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
