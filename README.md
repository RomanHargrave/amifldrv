# amifldrv source code

***This repository is not maintained, and its utility has never been guaranteed. It may not work with newer AMI hardware. Your mileage may vary.***

This is the source code for the `amifldrv` kernel module used by the AMI
Aptios flashing tool for linux, `afulnx`. This is associated with the
_64-Bit_ version of the tool, and I _cannot_ guarantee that it will work
with the _32-Bit_ version.

This source code has been _modified_ so that it will compile and work (I
tested this by flashing with it (!)) on recent versions of Linux. This was
tested with and functioned on Linux 4.1.

# License issues

AMI _did not_ specify a license for this code in `readme.txt`,
`readme_afulnx.txt` or any other file, including the source code.
Furthermore, Linux kernel modules can be considered derivitave works of
the Linux kernel, which is licensed under the GNU General Public License.
The GPL _mandates_ that source code be made available. AMI appear to have
gone out of their way to make obtaining the _full_ module source rather
obfuse, as the tool that emits it will delete it immediately after
exiting.

I do not claim any ownership of this code.
