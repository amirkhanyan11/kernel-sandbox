#include <stdbool.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>

int main(int ac, char **av)
{
  if (ac != 2) {
    printf("something went wrong\n");
    return 1;
  }
  printf("trying to open %s ...\n", av[1]);
  int fd = open(av[1], O_WRONLY);
  
  if (fd == -1) {
    printf("error while opening the device\n");
    return 1;
  }

  printf("called open from user space. fd : `%d`\n", fd);

  
  printf("trying to write something to file\n");

  int res = write(fd, "barev", strlen("barev"));

  printf("res: %d\n", res);

  close(fd);

  fd = open(av[1], O_RDONLY);
  
  if (fd == -1) {
    printf("error while opening the device\n");
    return 1;
  }
  
  printf("trying to read something from file\n");

  static char readbuf[1024] = {0};

  res = read(fd, readbuf, res + 1);

  printf("readbuf : %s\n", readbuf);

  printf("res: %d\n", res);
  
  return 0;
}


