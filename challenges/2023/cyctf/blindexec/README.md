# BlindExec
The writeup for this challenge can be found [here](https://wr3nchsr.github.io/cyctf-blindexec-writeup).

## Description
I left a flag in `/flag.txt` but I was told that if you can't see the flag you can't get it, can you?
Flag format: `cyctf{[a-z0-9_1}`

## Solution
Use shell scripting to perform a side channel attack to leak the flag.
