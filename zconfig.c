/* Public domain */
/* gcc zconfig.c -o zconfig */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>

#define BUFSIZE 1024
#define MSGSIZE 1000
#define PERM "0666"
#define LISTEN_BACKLOG 50
#define handle_error(function) { perror(function); exit(EXIT_FAILURE); }

int main()
{
	/* Parent process start*/
	pid_t child_pid;
	child_pid = fork ();
	if (child_pid == 0) {

	/* Child process start */
	FILE *fp;
	int server_socket, accepted_socket, n;
	struct sockaddr_un my_addr, peer_addr;
	socklen_t peer_addr_size;
	char recvd_msg[BUFSIZE];
	char my_path[BUFSIZE], sock_addr[BUFSIZE];
	char command[BUFSIZE];
	char out_str[BUFSIZE];
	char tmp_str[BUFSIZE];
	mode_t mode = strtol(PERM, 0, 8);


	/* Find the location of the executable */
	readlink("/proc/self/exe", my_path, BUFSIZE);
	dirname(my_path);
	printf("my path is: %s\n", my_path);

	/* Create socket file address related to executable location*/
	strcpy(sock_addr, my_path);
	strcat(sock_addr,"/zconfig.sock");
	printf("socket address: %s\n", sock_addr);

	/* Creating a Unix domain socket */
	server_socket = socket(AF_UNIX, SOCK_STREAM, 0);
	if (server_socket == -1)
		handle_error("socket");

	/* Binding the socket to a file */
	memset(&my_addr, 0, sizeof(struct sockaddr_un));
	my_addr.sun_family = AF_UNIX;
	strncpy(my_addr.sun_path, sock_addr, sizeof(my_addr.sun_path) - 1);
	unlink(my_addr.sun_path);
	if (bind(server_socket, (struct sockaddr *) &my_addr, 
				sizeof(struct sockaddr_un)) == -1)
		handle_error("bind");

	/* Instructing the socket to listen for incoming connections */
	if (listen(server_socket, LISTEN_BACKLOG) == -1) 
		handle_error("listen");

	/* Changing permission of socket the file */
	if (chmod(sock_addr, mode) == -1)
		handle_error("chmod");

	/* Child infinite loop start*/ 
	for(;;) 
	{
		/* Accepting incoming connections one at a time */
		printf("Waiting for a connection...\n"); fflush (stdout);
		peer_addr_size = sizeof(struct sockaddr_un);
		accepted_socket = accept(server_socket, 
			(struct sockaddr *) &peer_addr, &peer_addr_size);
		if (accepted_socket == -1)
			handle_error("accept");

		printf("Connected.\n"); fflush (stdout);

		/* Receiving data */
		n = recv(accepted_socket, recvd_msg, MSGSIZE, 0);
		if (n < 0)
			handle_error("recv");
		recvd_msg[n] = '\0';

		/* Printing received data */
		printf("Server got this message: \"%s\"\n", recvd_msg); 
		fflush (stdout);

		/* Create command */
		/* TODO: remove dots */
		strcpy(command, my_path);
		strcat(command, "/");
		strcat(command, recvd_msg);
		printf("\"%s\"\n",command); fflush (stdout);

		/* Run command and captrue the output */
		fp = popen(command, "r");
		if (fp == NULL)
			printf("Failed to run %s\n", command);

		/* Fill output to a single string */
		memset(&out_str, 0, sizeof(out_str));
		while (fgets(tmp_str, sizeof(tmp_str)-1, fp) != NULL)
			strcat(out_str,tmp_str);

		/* Sending data */
		if ( send(accepted_socket, out_str, strlen(out_str), 0) < 0 )
			handle_error("recv");

		/* Cleaning up */
		close(accepted_socket);

	/* Child infinite loop end */ 
	}

	/* Child process end */
	}

	/* Parent process end*/
	return 0;
}
