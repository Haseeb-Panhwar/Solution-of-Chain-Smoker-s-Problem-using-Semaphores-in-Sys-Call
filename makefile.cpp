// Make FIle code
obj-m += smoker_module.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean

 sudo insmod smoker_module.ko

//

#include <stdio.h>
#include <unistd.h>
#include <sys/syscall.h>

#define __NR_chain_smokers 333  // Update the system call number as per your setup

int main() {
    pid_t agent_pid = fork();

    if (agent_pid == -1) {
        perror("Fork failed.");
        return 1;
    }

    if (agent_pid == 0) {
        // Child process becomes the agent
        agent_process();
    } else {
        // Parent process calls the system call
        syscall(__NR_chain_smokers);
    }

    return 0;
}

