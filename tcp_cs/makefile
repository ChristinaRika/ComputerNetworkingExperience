all : srv cli

srv : tcp_echo_srv.c
	gcc -o srv tcp_echo_srv.c

cli : tcp_echo_cli.c
	gcc -o cli tcp_echo_cli.c