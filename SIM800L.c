#include "SIM800L.h"

/** AT Command to initialize the module */
const char AT[5] = "AT\r\n\0";
/** AT Command to get the device's ip */
const char AT_CIFSR[11] = "AT+CIFSR\r\n\0";
/** AT Command to get status of GSM */
const char AT_CGATT[12] = "AT+CGATT?\r\n\0";
/** AT Command to check module's functionality */
const char AT_CFUN[11] = "AT+CFUN?\r\n\0";
/** AT Command to disable response prompt */
const char AT_CIPSPRT[15] = "AT+CIPSPRT=0\r\n\0";
/** AT Command to enable SSL on TCP */
const char AT_CIPSSL[15] = "AT+CIPSSL=%d\r\n\0";
/** AT Command to open tcp socket */
const char AT_CIPSTART[31] = "AT+CIPSTART=\"tcp\",\"%s\",\"%d\"\r\n";
/** AT Command to send data to tcp socket*/
const char AT_CIPSEND[18] = "AT+CIPSEND\r\n\0";
/** AT Command to close tcp socket */
const char AT_CIPCLOSE[15] = "AT+CIPCLOSE\r\n\0";
/** HTTP request packet content template */
const char HTTP_PACKET_TEMPLATE[140] = "%s %s HTTP/1.0\r\nHost: %s\r\nUser-Agent: TelsokitModule/2021.5.2\r\nContent-Type: application/json\r\nAccept: */*\r\nContent-Length: %llu\r\n\r\n%s\r\n\r\n\x1A\0";

// TODO: Implement error handling
int GSM_HTTP(char *method, char *url, char *body)
{
	char enable_ssl_buffer[12];
	char *trimmed_url;
	char *url_location;
	char *tcp_buffer;
	char *protocol_data;
	char *receive_buffer = "\0";
	int is_ssl;
	int body_length;

	trimmed_url = trim_url(url);
	printf("gola");
	url_location = get_url_location(url);
	is_ssl = is_url_ssl(url) ? 443 : 80;
	if (strlen(body) != 0)
		body_length = floor(log10(strlen(body))) + 1;
	else
		body_length = 0;
	// Setup initial config
	send_data(AT);
	read_data(&receive_buffer);
	printf("%s", receive_buffer);
	if (!strstr(receive_buffer, "OK"))
		return (1);

	send_data(AT_CIFSR);
	read_data(&receive_buffer);
	printf("%s", receive_buffer);
	if (strstr(receive_buffer, "ERROR"))
		return (1);

	send_data(AT_CGATT);
	read_data(&receive_buffer);
	printf("%s", receive_buffer);
	if (!strstr(receive_buffer, "OK"))
		return (1);

	send_data(AT_CFUN);
	read_data(&receive_buffer);
	printf("%s", receive_buffer);
	if (!strstr(receive_buffer, "OK"))
		return (1);

	send_data(AT_CIPSPRT);
	read_data(&receive_buffer);
	printf("%s", receive_buffer);
	if (!strstr(receive_buffer, "OK"))
		return (1);

	// Tell the module wether ssl is required
	sprintf(enable_ssl_buffer, AT_CIPSSL, is_ssl == 443);
	send_data(enable_ssl_buffer);
	read_data(&receive_buffer);
	printf("%s", receive_buffer);
	if (!strstr(receive_buffer, "OK"))
		return (1);

	// Stablish TCP socket
	tcp_buffer = malloc(sizeof(char) * (25 + strlen(trimmed_url) + ((is_ssl == 443) ? 3 : 2) + 1));
	sprintf(tcp_buffer, AT_CIPSTART, trimmed_url, is_ssl);
	send_data(tcp_buffer);
	read_data(&receive_buffer);
	printf("%s", receive_buffer);
	if (!strstr(receive_buffer, "OK"))
		return (1);

	// TODO: Await for succesful connection
	send_data(AT_CIPSEND);
	while (!strstr(receive_buffer, "CONNECT OK"))
	{
		read_data(&receive_buffer);
		printf("%s", receive_buffer);
	}

	// Send data using template
	protocol_data = malloc(sizeof(char) * (57 + strlen(method) + strlen(url_location) + strlen(trimmed_url) + body_length + 1));
	sprintf(protocol_data, HTTP_PACKET_TEMPLATE, method, url_location, trimmed_url, strlen(body), body);
	send_data(protocol_data);

	// TODO: Close only if not closed by request
	send_data(AT_CIPCLOSE);

	// Free buffers used
	free(tcp_buffer);
	free(protocol_data);
	free(trimmed_url);
	free(url_location);
	return 0;
}

int GSM_HTTP_PUT(char *url, char *body)
{
	return GSM_HTTP("PUT", url, body);
}

int GSM_HTTP_PATCH(char *url, char *body)
{
	return GSM_HTTP("PATCH", url, body);
}