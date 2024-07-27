# Low Budget Router Trilogy
A series of three challenges I wrote for CyCTF finals 2023, intended as a practical introduction to IOT hacking.
The writeup for these challenges can be found [here](https://wr3nchsr.github.io/cyctf-low-budget-router-trilogy-writeup).

## Challenges
### Firmware Dump
#### Description
I bought a new router called Low Budget Router for a very cheap price and I think there might be a catch to that. So l dumped the firmware off of it using `SNANDer` but I cannot extract the filesystem. Someone told me a datasheet might help but I don't get it. Can you help?
Flag is at `/etc/`
#### Solution
Remove OOB data after identifying that ECC is disabled to fix the firmware binary and extract the filesystem.

### Management Portal
#### Description
The router appears to use a custom HTTP server. Can this be their downfall?
Note: This challenge depends on the filesystem extracted from `Firmware Dump`
#### Solution
Exploit a vanilla buffer overflow in the `POSTLogin` function resulting from the difference between the length being checked and the length being copied and overwrite the return address with the address of `readFlag` (ret2win).

### Management Console
#### Description
Some routers might have a backdoor but it can be tough to find. Do you think maybe this router will have one?
Note: This challenge depends on the filesystem extracted from `Firmware Dump`
##### Solution
Reverse engineer hidden commands inside the shared objects containing the logic of the `clid` binary and find the appropriate sequence to trigger an argument injection inside one of the hidden commands which will lead to arbitrary command execution.

## Build
Simply run the build script which will take care of everything (compile the binaries, build the file system, create the firmware blob and build the docker image).
> [!WARNING]
> The build script requires root permissions as it sets the files' permissions before making the jffs2 filesystem.
> Read the script carefully before executing it.

```sh
$ ./build.sh
```

## Files
### Provided to Players
* `./SNANDer_output.txt`
* `./W25N01GV.pdf`
* `./build/players/fw_dump.bin`

### Used to Deploy the Challenges
* `./build/challenge/Dockerfile`
* `./build/challenge/docker-compose.yml`
* `./build/challenge/setup.sh`
* `./build/challenge/fs.jffs2`

## Flags
* Flag for "Firmware Dump" is added in `./build.sh`
* Flags for "Management Portal" and "Management Console" are added in `./build/challenge/setup.sh`
