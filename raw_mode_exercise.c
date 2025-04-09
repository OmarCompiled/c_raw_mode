#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/syscall.h>
#include <termios.h>
#include <unistd.h>

struct termios original_attrs;

void
disable_raw_mode(void)
{
	tcsetattr(STDIN_FILENO, TCSAFLUSH, &original_attrs);
}

void
enable_raw_mode(void)
{
	tcgetattr(STDIN_FILENO, &original_attrs);
	atexit(disable_raw_mode);

	struct termios raw = original_attrs;
	raw.c_lflag &= ~(ECHO | ICANON | ISIG);
	tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

void
clear_screen(void)
{
	syscall(SYS_write, STDOUT_FILENO, "\033[H\033[J", 6);
}

int 
main(void)
{
	enable_raw_mode();
	clear_screen();
	char c;
	int char_count = 0;
	while(read(STDIN_FILENO, &c, 1) == 1) {
		if(c == 127 && char_count > 0) {
			printf("\b \b");
			char_count--;
		} else if(c == 27) {
			char ctrl[2];
			if(read(STDIN_FILENO, &ctrl[0], 1) 
			&& read(STDIN_FILENO, &ctrl[1], 1)) {
				printf("\033%c%c", ctrl[0], ctrl[1]);
			}
		} else if(iscntrl(c) && c != 10) {
			printf("%d\n", c);
	  } else if(c == ':') {
			printf("\n:");
			fflush(stdout);
			char command;
			disable_raw_mode();
			read(STDIN_FILENO, &command, 1);
			if(command == 'q') {
				clear_screen();
				exit(0);
			} else if(command == 'c') {
				printf("\033[1A\033[2K");
			}
			enable_raw_mode();
	  } else {
			printf("%c", c);
		}
		char_count++;
		fflush(stdout);
	}
}
