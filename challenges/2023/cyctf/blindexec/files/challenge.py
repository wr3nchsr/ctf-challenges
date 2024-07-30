import os

class BlindExec():
    def __init__(self):
        self.null_fds =  [os.open(os.devnull, os.O_RDWR) for _ in range(3)]
        self.saved_fds = [os.dup(0), os.dup(1), os.dup(2)]

    def __enter__(self):
        os.dup2(self.null_fds[0], 0)
        os.dup2(self.null_fds[1], 1)
        os.dup2(self.null_fds[2], 2)

    def __exit__(self, *_):
        os.dup2(self.saved_fds[0], 0)
        os.dup2(self.saved_fds[1], 1)
        os.dup2(self.saved_fds[2], 2)
        for fd in self.null_fds + self.saved_fds:
            os.close(fd)

def main():
    cmd = input("Enter command to execute: ")
    if len(cmd) > 30:
        print("Your command is too long!")
        exit(1)
    with BlindExec():
        os.system(cmd)
    print("Your command is done!")

if __name__ == "__main__":
    main()
