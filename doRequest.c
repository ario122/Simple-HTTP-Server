


char *doRequest(char *start, char** headers, char *body)
{

	int i = 0, j = 0;
	int len = 0;
	while (start[i]!='\0') i++;
	len = len + i;
	i = 0;
	while (body[i]!='\0') i++;
	len = len + i;

	while (headers[j]!='\0')
	{
		i = 0;
		while(headers[j][i]!='\0')
			i++;
		len = len + i;
		j++;
	}

	char* result = (char*)malloc(len + 300);

	strcat(result, "HTTP/1.1 200 OK\r\n\r\n\r\n<HTML><BODY>");
	strcat(result, start);
	strcat(result, "<br>");
	i = 0;

	while (headers[i] != '\0')
	{
		strcat(result, "<b>");
		strcat(result, headers[i++]);
		strcat(result, "</b>: ");
		strcat(result, headers[i++]);
		strcat(result, "<br>");
	}
	strcat(result, "<br><br>");
	strcat(result, body);
	strcat(result, "</BODY></HTML>");
	return result;
}
