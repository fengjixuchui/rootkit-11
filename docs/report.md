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

## Reverse Shell
The rootkit provides a userland tool called `rshell_target`. This should be run
on the infected machine to establish a remote shell. `rshell_target` works by:
1. Polling for a TCP connection to a remote IP specified at infection time.
2. Executing a shell and redirecting standard input, output and error to the
socket used for the TCP connection. This allows the other end of the socket,
the remote user, to control the shell and recieve output.

## Keylogging
Similar to how the reverse shell is implemented, the contents of a file can be
sent over a TCP connection using a socket.
This is done by:
1. Polling for TCP connection
2. Polling for a `keylog_file`
3. Reading new data written to a keylog file
The rootkit collects only new data from a file by reading differences in file
sizes at regular intervals

## Persistence
The rootkit as a kernel module achieves persistence by installing the rootkit
module to `/boot/modules` and updating the `/boot/loader.conf` configuration
file to autoload the rootkit module. Details of how this is hidden are
discussed later in the report.

For features that depend on userland processes, persistence is managed by using
FreeBSD's `rc` which allows a system administrator to run scripts at startup.
Configuring these features to run at startup involved:
1. Creating an `/etc/rc.d/` entry which details what to do for a given "module".
This included:
    1. Requirements before booting our modules such as loading the file system.
    2. Scripts to run on load.
2. Adding an entry for each module inside `/etc/rc.conf`.
3. Installing these items on rootkit installation.
Each script must run in a non-blocking fashion, so a `fork` and `execute` model
was used.

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

Transparent and hidden files are used to facilitate persistence for the
rootkit. The rootkit is added to `/boot/loader.conf` which controls what
modules are loaded at boot. This is superior to loading the kernel once
the system has finished booting as a rootkit detector may load or run before
the rootkit has infected the kernel. The module itself is hidden in
`/boot/modules/`. This ensures that hidden files are hidden the moment
the system has finished booting.

## Bonus Marks
- The rootkit makes use of transparent files which allow users to interact
with files critical to the functioning of the rootkit *without* damaging the
data required there for the rootkit. An example of this is `/boot/loader.conf`
which must contain a line for the rootkit to ensure that it is loaded at boot.
- The rootkit uses inline function hooking which is much harder to detect than
simply setting function pointers in the system call function pointer array.

# Rootkit detection

The rootkit hooks a number of system calls. Some of the operations it needs to
perform are time intensive and doing analysis on the time taken to perform
certain system calls may reveal that the system calls have been extended. This
isn't very practical for most systems because of the margin of error in
measurements.

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

The extra features of the rootkit, specifically the remote shell, keylogging,
and data exfiltration increase the traces that the rootkit leaves on the
system. This makes it easier to detect. While the ports used are hidden by the
rootkit on the infected system, it would be possible to scan the system for
open ports from another machine on the same network. Furthermore, it would
be possible to capture the packets as they left the system and inspect their
contents.

## Improvements
To reduce the chance that a rootkit detector will detect changes to kernel
functions such as `sys_openat` it would be best to patch the function in
a more subtle way. Currently, the first instruction is made to be a jump.
This is blatant and obviously not the original implementation of the function.

It would be ideal if the remote shell and exfiltration features were not
constantly enabled to reduce the chance of being detected. A possible
improvement would be to icmp knocking to notify the system that the remote
machine was ready to recieve or send data. Additionally, there would be some
benefit to obscuring the data being sent and recieved along with sending it
pretending to be an application that *should* send and recieve data across the
internet.
