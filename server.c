#include <stdio.h>
#include <stdlib.h>

#include <unistd.h>
#include <netinet/in.h>

#include <string.h>

static long getHandleLength(FILE *handle) {
	fseek(handle, 0, SEEK_END);
	return ftell(handle);
}

static void readHandle(FILE *handle, char *buffer, unsigned handleLength) {
	fseek(handle, 0, SEEK_SET);
	fread(buffer, sizeof(char), handleLength, handle);
}

int main() {
	struct sockaddr_in us_info;
	us_info.sin_family = AF_INET;
	us_info.sin_port = htons(1235);
	us_info.sin_addr.s_addr = INADDR_ANY;

	int us_sock = socket(AF_INET, SOCK_STREAM, 0);
	bind(us_sock, (struct sockaddr *)&us_info, sizeof us_info);
	listen(us_sock, 0);

	struct sockaddr_in them_info;
	unsigned them_info_size = sizeof them_info;

	// open the file
	FILE *handle = fopen("rot.gif", "r");
	// get length of file contents
	long handleLength = getHandleLength(handle);

	// create a buffer to be sent
	char *content = malloc(128);
	// append the header info
	sprintf(content, "HTTP/1.1 200 OK\r\nContent-Type: image/gif\r\nContent-Length: %li\r\n\r\n", handleLength);
	// get the current length of the string
	unsigned long templateLength = strlen(content);
	// reallocate enough memory for file contents
	content = realloc(content, templateLength + (unsigned)handleLength);
	// append file contents to content buffer (header + content)
	readHandle(handle, content + templateLength, (unsigned)handleLength);

	long status;
	char readBuffer[128];

	for (;;) {
		int them_sock = accept(us_sock, (struct sockaddr *)&them_info, &them_info_size);
		send(them_sock, content, templateLength + (unsigned)handleLength, 0);

		do {
			status = recv(them_sock, readBuffer, sizeof readBuffer, MSG_DONTWAIT);
			//char *i = strchr(readBuffer, '/');
			printf("%s ((((%li))))\n", readBuffer, status);
		}
		while (status > 0);

		close(them_sock);
	}
}
