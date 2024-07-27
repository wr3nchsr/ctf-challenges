
fw_file_noob = "fw_dump_without_oob.bin"
fw_file = "fw_dump.bin"
uboot = "extra/uboot-uImage"
kernel = "extra/kernel-uImage"
fs = "extra/fs.jffs2"

fw_header = b"1337SSSS1337AAAA\x00Low Budget Router Fake Firmware v1.0 by wr3nchsr\x00"
fw_disclaimer = b"Disclaimer: This firmware is an attempt to replicate how a real world firmware might look like. Some aspect may not be realistic and/or working for an actual embedded device\x00"

def add_oob():
    fw_noob = open(fw_file_noob, "rb")
    fw = open(fw_file, "wb")
    for i in range(65536):
        fw_noob.seek(i * 2048)
        content = fw_noob.read(2048)

        fw.seek((i * 2048) + (64 * i))
        fw.write(content)
        fw.write(b"\xff" * 64)
    
    fw_noob.close()
    fw.close()


def main():
    open(fw_file_noob, "wb").write(b"\xff" * 134217728)
    fw = open(fw_file_noob, "r+b")
    fw.write(fw_header)
    fw.write(fw_disclaimer)

    # Write uboot
    fw.seek(0x200)
    fw.write(open(uboot, "rb").read())
    fw.seek(0x60200)
    fw.write(open(uboot, "rb").read())

    # Write kernel
    fw.seek(0x1320000)
    fw.write(open(kernel, "rb").read())
    # Write filesystem
    fw.seek(0x20a0000)
    fw.write(open(fs, "rb").read())

    # Write kernel
    fw.seek(0x3320000)
    fw.write(open(kernel, "rb").read())
    # Write filesystem
    fw.seek(0x40a0000)
    fw.write(open(fs, "rb").read())

    fw.close()
    add_oob()


if __name__ == "__main__":
    main()