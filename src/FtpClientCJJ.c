#include "FtpClientCJJ.h"

bool create_directories(const char *path)
{
    char *pp;
    char *slash;
    bool status = true;
    int len;

    // Make a mutable copy of the path
    char *path_copy = strdup(path);
    if (path_copy == NULL)
    {
        perror("Erro ao alocar memória para o caminho do diretório");
        return false;
    }

    len = strlen(path_copy);
    if (len == 0)
    {
        free(path_copy);
        return true; // Nothing to create
    }

    pp = path_copy;
    if (path_copy[0] == '/')
    { // Handle absolute paths
        pp++;
    }
    else if (path_copy[0] == '~')
    { // Handle ~ (home directory) if needed, but not common for FTP downloads directly
      // For simplicity, we won't expand ~ here. If needed, you'd get the home dir.
      // For now, treat ~ as a literal directory name or expect absolute paths.
    }

    while ((slash = strchr(pp, '/')) != NULL)
    {
        *slash = '\0'; // Temporarily null-terminate to create one directory at a time
        if (mkdir(path_copy, 0755) != 0)
        { // 0755 permissions (rwx for owner, rx for group/others)
            if (errno != EEXIST)
            { // If it's not just that the directory already exists
                perror("Erro ao criar diretório");
                status = false;
                break;
            }
        }
        *slash = '/'; // Restore the slash
        pp = slash + 1;
    }

    free(path_copy);
    return status;
}

bool parse_ip_part(const char *part_str, uint8_t *out)
{
    char *endptr;
    long val = strtol(part_str, &endptr, 10);
    if (*endptr || val < 0 || val > 255)
    {
        return false;
    }
    *out = (uint8_t)val;
    return true;
}

bool parse_port_part(const char *part_str, uint16_t *out)
{
    char *endptr;
    long val = strtol(part_str, &endptr, 10);
    if (*endptr || val < 0 || val > 65535)
    {
        return false;
    }
    *out = (uint16_t)val;
    return true;
}

FTPClient *FTPClient_create(const char *ftpUrl)
{
    if (!ftpUrl)
    {
        return NULL;
    }

    FTPClient *client = (FTPClient *)malloc(sizeof(FTPClient));
    if (!client)
    {
        perror("Erro ao alocar memória para FTPClient");
        return NULL;
    }
    memset(client, 0, sizeof(FTPClient));

    client->host = NULL;
    client->ip = NULL;
    client->port = 21;                             // Porta FTP padrão
    client->user = StrCJJ_create("anonymous");     // Usuário padrão
    client->password = StrCJJ_create("anonymous"); // Senha padrão
    client->urlPath = StrCJJ_create("Nao ha path de momento");
    client->filename = StrCJJ_create("no file selected");
    client->responseBuffer = StrCJJ_create("");
    client->controlSocket = -1;
    client->dataSocket = -1;

    // ftp://[<user>:<password>@]<host>/<url-path>
    StrCJJ *urlCJJ = StrCJJ_create(ftpUrl);
    if (!StrCJJ_startsWith(urlCJJ, "ftp://"))
    {
        fprintf(stderr, "URL FTP inválido: %s\n", ftpUrl);
        FTPClient_destroy(client);
        StrCJJ_free(urlCJJ);
        return NULL;
    }

    // Remover "ftp://"
    StrCJJ *remainingUrl = StrCJJ_substring(urlCJJ, 6, urlCJJ->len - 6);
    StrCJJ_free(urlCJJ);
    urlCJJ = remainingUrl;

    ssize_t atIndex = StrCJJ_findIndexOf_2(urlCJJ, "@");
    StrCJJ *hostAndPath;

    if (atIndex != -1)
    {
        StrCJJ *userPass = StrCJJ_substring(urlCJJ, 0, atIndex);
        size_t colonIndex = StrCJJ_findIndexOf_2(userPass, ":");
        if (colonIndex != -1)
        {
            StrCJJ_free(client->user);
            client->user = StrCJJ_substring(userPass, 0, colonIndex);
            StrCJJ_free(client->password);
            client->password = StrCJJ_substring(userPass, colonIndex + 1, userPass->len - (colonIndex + 1));
        }
        else
        {
            StrCJJ_free(client->user);
            client->user = StrCJJ_clone(userPass);
            StrCJJ_free(client->password);
        }
        StrCJJ_free(userPass);
        hostAndPath = StrCJJ_substring(urlCJJ, atIndex + 1, urlCJJ->len - (atIndex + 1));
    }
    else
    {
        hostAndPath = StrCJJ_clone(urlCJJ);
    }
    StrCJJ_free(urlCJJ);

    ssize_t slashIndex = StrCJJ_findIndexOf_2(hostAndPath, "/");
    if (slashIndex >= 0)
    {
        StrCJJ_free(client->host);
        client->host = StrCJJ_substring(hostAndPath, 0, slashIndex);
        StrCJJ_free(client->urlPath);
        client->urlPath = StrCJJ_substring(hostAndPath, slashIndex + 1, hostAndPath->len - (slashIndex + 1));
        StrCJJ_free(client->filename);
        client->filename = StrCJJ_basename(client->urlPath);
    }
    else
    {
        StrCJJ_free(client->host);
        client->host = StrCJJ_clone(hostAndPath);
    }
    StrCJJ_free(hostAndPath);

    return client;
}

int connect_with_timeout(int sockfd, struct sockaddr *addr, socklen_t addrlen, int timeout_sec)
{
    fcntl(sockfd, F_SETFL, O_NONBLOCK);

    int res = connect(sockfd, addr, addrlen);
    if (res < 0 && errno != EINPROGRESS)
        return -1;

    if (res == 0)
        goto done;

    fd_set wfds;
    FD_ZERO(&wfds);
    FD_SET(sockfd, &wfds);
    struct timeval tv = {.tv_sec = timeout_sec, .tv_usec = 0};

    res = select(sockfd + 1, NULL, &wfds, NULL, &tv);
    if (res <= 0)
        return -1;

    int so_error;
    socklen_t len = sizeof(so_error);
    getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &so_error, &len);
    if (so_error != 0)
        return -1;

done:
    fcntl(sockfd, F_SETFL, fcntl(sockfd, F_GETFL) & ~O_NONBLOCK);
    return 0;
}

void FTPClient_destroy(FTPClient *client)
{
    if (client)
    {
        StrCJJ_free(client->host);
        StrCJJ_free(client->ip);
        StrCJJ_free(client->user);
        StrCJJ_free(client->password);
        StrCJJ_free(client->urlPath);
        StrCJJ_free(client->filename);
        StrCJJ_free(client->responseBuffer);
        if (client->controlSocket != -1)
        {
            close(client->controlSocket);
        }
        if (client->dataSocket != -1)
        {
            close(client->dataSocket);
        }
        free(client);
    }
}

bool FTPClient_sendCommand(int socket, const char *command, FTPClient *client)
{
    printf("Enviando comando: %s", command);
    ssize_t bytes_sent = send(socket, command, strlen(command), 0);
    if (bytes_sent == -1)
    {
        perror("Erro ao enviar comando");
        return false;
    }
    return true;
}

bool FTPClient_receiveResponse(int socket, FTPClient *client)
{
    if (!client)
    {
        printf("Erro: Client não criado\n");
        return false;
    }

    StrCJJ_clear(client->responseBuffer);
    char buffer[512] = {0};
    ssize_t bytes_read;
    while ((bytes_read = recv(socket, buffer, sizeof(buffer) - 1, 0)) > 0)
    {
        buffer[bytes_read] = '\0';
        StrCJJ *temp = StrCJJ_create(buffer);
        StrCJJ_appendStr(client->responseBuffer, temp);
        StrCJJ_free(temp);
        if (StrCJJ_contains(client->responseBuffer, "\r\n"))
        {
            break;
        }
    }

    if (bytes_read == -1)
    {
        printf("Erro ao receber resposta do servidor");
        return false;
    }

    StrCJJ_print(client->responseBuffer, "Resposta do Servidor:", "\n");

    return true;
}

bool FTPClient_connect(FTPClient *client)
{
    if (!client || !client->host || !client->host->str)
    {
        printf("Erro: Cliente ou host inválido.\n");
        return false;
    }

    struct hostent *he;
    struct sockaddr_in server_addr;

    if ((he = gethostbyname(client->host->str)) == NULL)
    {
        herror("Erro ao obter informações do host");
        return false;
    }

    if ((client->controlSocket = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        perror("Erro ao criar o socket de controlo");
        return false;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(client->port);
    server_addr.sin_addr = *((struct in_addr *)he->h_addr_list[0]);
    bzero(&(server_addr.sin_zero), 8);

    /*if (connect(client->controlSocket, (struct sockaddr *)&server_addr, sizeof(struct sockaddr)) < 0)
    {
        perror("Erro ao conectar ao servidor FTP");
        close(client->controlSocket);
        client->controlSocket = -1;
        return false;
    }*/

    if (connect_with_timeout(client->controlSocket, (struct sockaddr *)&server_addr, sizeof(struct sockaddr), TIMEOUT) < 0)
    {
        perror("Erro ao conectar ao servidor FTP");
        close(client->controlSocket);
        client->controlSocket = -1;
        return false;
    }

    // Ler a resposta inicial do servidor
    StrCJJ_clear(client->responseBuffer);
    char buffer[512] = {0};
    ssize_t bytes_read;
    while ((bytes_read = recv(client->controlSocket, buffer, sizeof(buffer) - 1, 0)) > 0)
    {
        buffer[bytes_read] = '\0';
        StrCJJ *temp = StrCJJ_create(buffer);
        StrCJJ_appendStr(client->responseBuffer, temp);
        StrCJJ_free(temp);
        if (StrCJJ_findIndexOf_2(client->responseBuffer, "\r\n") != -1)
        {
            break;
        }
    }

    if (bytes_read == -1)
    {
        perror("Erro ao receber resposta inicial do servidor");
        close(client->controlSocket);
        client->controlSocket = -1;
        return false;
    }

    printf("Resposta do servidor: %s\n", client->responseBuffer->str);
    return true;
}

bool FTPClient_login(FTPClient *client)
{
    if (!client)
    {
        printf("Erro: Client não criado\n");
        return false;
    }

    if (client->controlSocket == -1)
    {
        printf("Erro: Não conectado ao servidor.\n");
        return false;
    }

    // Enviar comando USER
    StrCJJ *userCmd = StrCJJ_create("");
    StrCJJ_appendFormat(userCmd, "USER %s\r\n", client->user->str);
    if (!FTPClient_sendCommand(client->controlSocket, userCmd->str, client))
    {
        StrCJJ_free(userCmd);
        return false;
    }
    StrCJJ_free(userCmd);

    if (!FTPClient_receiveResponse(client->controlSocket, client))
    {
        return false;
    }

    if (!StrCJJ_startsWith(client->responseBuffer, "331"))
    {
        fprintf(stderr, "Erro: Servidor não aceitou o nome de usuário.\n%s\n", client->responseBuffer->str);
        return false;
    }

    StrCJJ *passCmd = StrCJJ_create("");
    StrCJJ_appendFormat(passCmd, "PASS %s\r\n", client->password->str);
    if (!FTPClient_sendCommand(client->controlSocket, passCmd->str, client))
    {
        StrCJJ_free(passCmd);
        return false;
    }
    StrCJJ_free(passCmd);

    if (!FTPClient_receiveResponse(client->controlSocket, client))
    {
        return false;
    }
    if (!StrCJJ_startsWith(client->responseBuffer, "230"))
    {
        fprintf(stderr, "Erro: Falha na autenticação.\n%s\n", client->responseBuffer->str);
        return false;
    }

    printf("Autenticação bem-sucedida.\n");
    return true;
}

bool FTPClient_enterPassiveMode(FTPClient *client)
{
    if (!client)
    {
        printf("Erro: Client não criado\n");
        return false;
    }

    if (client->controlSocket == -1)
    {
        printf("Erro: Não conectado ao servidor.\n");
        return false;
    }

    // Enviar comando PASV
    if (!FTPClient_sendCommand(client->controlSocket, "PASV\r\n", client))
    {
        return false;
    }

    // Receber e verificar resposta
    if (!FTPClient_receiveResponse(client->controlSocket, client))
    {
        return false;
    }

    if (!StrCJJ_startsWith(client->responseBuffer, "227"))
    {
        fprintf(stderr, "Erro: Falha ao entrar no modo passivo. Resposta: %s\n", client->responseBuffer->str);
        return false;
    }

    // Parse the IP and port from the PASV response
    // Response format: "227 Entering Passive Mode (h1,h2,h3,h4,p1,p2)"
    StrCJJ *response = StrCJJ_clone(client->responseBuffer);
    ssize_t start = StrCJJ_findIndexOf_2(response, "(");
    ssize_t end = StrCJJ_findLastIndexOf_2(response, ")");

    if (start == -1 || end == -1 || start >= end)
    {
        printf("Erro: Formato inválido na resposta PASV: %s\n", response->str);
        StrCJJ_free(response);
        return false;
    }

    StrCJJ *dataAddressStrCJJ = StrCJJ_substring(response, start + 1, end - start - 1);
    StrCJJ_free(response);

    size_t count = 0;
    StrCJJ **parts = StrCJJ_split(dataAddressStrCJJ, ',', &count);
    StrCJJ_free(dataAddressStrCJJ);

    if (count != 6)
    {
        printf("Erro: Número incorreto de partes na resposta PASV.\n");
        printf("Partes obtidas:\n");
        for (size_t i = 0; i < count; i++)
        {
            StrCJJ_print(parts[i], " - ", "\n");
        }
        StrCJJ_freeArray(parts, count);
        return false;
    }

    uint8_t ip_parts[4];
    uint16_t port_parts[2];

    for (int i = 0; i < 4; i++)
    {
        if (!parse_ip_part(parts[i]->str, &ip_parts[i]))
        {
            fprintf(stderr, "Erro: Parte do IP inválida: %s\n", parts[i]->str);
            StrCJJ_freeArray(parts, count);
            return false;
        }
    }

    for (int i = 0; i < 2; i++)
    {
        if (!parse_port_part(parts[i + 4]->str, &port_parts[i]))
        {
            fprintf(stderr, "Erro: Parte da porta inválida: %s\n", parts[i + 4]->str);
            StrCJJ_freeArray(parts, count);
            return false;
        }
    }

    client->port = (port_parts[0] << 8) | port_parts[1];

    StrCJJ_free(client->ip);
    client->ip = StrCJJ_create("");
    StrCJJ_appendFormat(client->ip, "%d.%d.%d.%d", ip_parts[0], ip_parts[1], ip_parts[2], ip_parts[3]);

    StrCJJ_freeArray(parts, count);

    printf("Modo Passivo ativado. IP: %s, Porta: %u\n", client->ip->str, client->port);
    return true;
}

bool FTPClient_downloadFile(FTPClient *client)
{
    if (!client)
    {
        printf("Erro: Client não criado\n");
        return false;
    }

    if (client->controlSocket == -1)
    {
        printf("Erro: Não conectado ao servidor.\n");
        return false;
    }

    if (!FTPClient_enterPassiveMode(client))
    {
        printf("Erro: Não entrou no modo passivo\n");
        return false;
    }

    // Criar socket para dados
    if ((client->dataSocket = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        perror("Erro ao criar socket de dados");
        return false;
    }

    struct sockaddr_in data_addr;
    data_addr.sin_family = AF_INET;
    data_addr.sin_port = htons(client->port);

    if (inet_pton(AF_INET, client->ip->str, &data_addr.sin_addr) <= 0)
    {
        perror("Erro ao converter IP");
        close(client->dataSocket);
        client->dataSocket = -1;
        return false;
    }

    memset(data_addr.sin_zero, 0, sizeof(data_addr.sin_zero));

    if (connect_with_timeout(client->dataSocket, (struct sockaddr *)&data_addr, sizeof(data_addr), TIMEOUT) < 0) //
    {
        perror("Erro ao conectar socket de dados");
        close(client->dataSocket);
        client->dataSocket = -1;
        return false;
    }

    if (!FTPClient_sendCommand(client->controlSocket, "TYPE I\r\n", client))
    {
        printf("Erro ao definir modo de transferência para binário\n");
        close(client->dataSocket);
        client->dataSocket = -1;
        return false;
    }
    if (!FTPClient_receiveResponse(client->controlSocket, client))
    {
        printf("Erro ao receber resposta do comando TYPE I\n");
        close(client->dataSocket);
        client->dataSocket = -1;
        return false;
    }

    if (!StrCJJ_startsWith(client->responseBuffer, "200"))
    {
        fprintf(stderr, "Erro: Servidor não aceitou TYPE I. Resposta: %s\n", client->responseBuffer->str);
        close(client->dataSocket);
        client->dataSocket = -1;
        return false;
    }

    // --- NEW LOGIC: Create directories if they don't exist ---
    StrCJJ *full_local_path_str = StrCJJ_create(client->filename->str);
    StrCJJ *dirname_str = StrCJJ_dirname(full_local_path_str); // Get the directory part
    StrCJJ_free(full_local_path_str);                          // Free the temporary string

    if (dirname_str && dirname_str->len > 0)
    {
        if (!create_directories(dirname_str->str))
        {
            fprintf(stderr, "Erro ao criar diretórios para o caminho local: %s\n", dirname_str->str);
            StrCJJ_free(dirname_str);
            close(client->dataSocket);
            client->dataSocket = -1;
            return false;
        }
    }
    StrCJJ_free(dirname_str); // Free the directory string after use
    // --- END NEW LOGIC ---

    StrCJJ *retrCmd = StrCJJ_create("");
    StrCJJ_print(client->filename, "Filename: ", "\n");
    StrCJJ_appendFormat(retrCmd, "RETR %s\r\n", client->filename->str);
    if (!FTPClient_sendCommand(client->controlSocket, retrCmd->str, client))
    {
        StrCJJ_free(retrCmd);
        return false;
    }
    StrCJJ_free(retrCmd);

    if (!FTPClient_receiveResponse(client->controlSocket, client))
    {
        return false;
    }

    if (!StrCJJ_startsWith(client->responseBuffer, "150"))
    {
        fprintf(stderr, "Erro ao iniciar transferência do ficheiro: %s\n", client->responseBuffer->str);
        return false;
    }

    // Ler dados do dataSocket
    FILE *fp = fopen(client->filename->str, "wb");
    if (!fp)
    {
        perror("Erro ao abrir ficheiro para escrita");
        return false;
    }

    char buffer[1024] = {0};
    ssize_t bytes_read;
    while ((bytes_read = recv(client->dataSocket, buffer, sizeof(buffer), 0)) > 0)
    {
        fwrite(buffer, 1, bytes_read, fp);
    }

    fclose(fp);
    close(client->dataSocket);
    client->dataSocket = -1;

    if (!FTPClient_receiveResponse(client->controlSocket, client))
    {
        return false;
    }

    if (!StrCJJ_startsWith(client->responseBuffer, "226"))
    {
        fprintf(stderr, "Transferência pode não ter sido concluída corretamente: %s\n", client->responseBuffer->str); //
        return false;
    }

    printf("Download concluído com sucesso!\n");
    return true;
}

void FTPClient_disconnect(FTPClient *client)
{
    if (!client)
    {
        return;
    }

    if (client->controlSocket != -1)
    {
        FTPClient_sendCommand(client->controlSocket, "QUIT\r\n", client);
        FTPClient_receiveResponse(client->controlSocket, client);
        close(client->controlSocket);
        client->controlSocket = -1;
    }

    if (client->dataSocket != -1)
    {
        close(client->dataSocket);
        client->dataSocket = -1;
    }

    printf("Desconectado do servidor FTP.\n");
}

bool FTPClient_list(FTPClient *client, const char *pathname)
{
    if (!client)
    {
        printf("Erro: Client não criado\n");
        return false;
    }

    if (client->controlSocket == -1)
    {
        printf("Erro: Não conectado ao servidor.\n");
        return false;
    }

    // Enter passive mode
    if (!FTPClient_enterPassiveMode(client))
    {
        printf("Erro: Não entrou no modo passivo\n");
        return false;
    }

    // Create data socket
    int dataSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (dataSocket == -1)
    {
        perror("Erro ao criar socket de dados");
        return false;
    }

    struct sockaddr_in data_addr;
    data_addr.sin_family = AF_INET;
    data_addr.sin_port = htons(client->port);

    if (inet_pton(AF_INET, client->ip->str, &data_addr.sin_addr) <= 0)
    {
        perror("Erro ao converter IP");
        close(dataSocket);
        return false;
    }
    memset(data_addr.sin_zero, 0, sizeof(data_addr.sin_zero));

    /*if (connect(dataSocket, (struct sockaddr *)&data_addr, sizeof(data_addr)) < 0)
    {
        perror("Erro ao conectar socket de dados");
        close(dataSocket);
        return false;
    }*/
    if (connect_with_timeout(dataSocket, (struct sockaddr *)&data_addr, sizeof(data_addr), TIMEOUT) < 0)
    {
        perror("Erro ao conectar socket de dados");
        close(dataSocket);
        return false;
    }

    // Send LIST command
    StrCJJ *listCmd = StrCJJ_create("LIST ");
    if (pathname && strlen(pathname) > 0)
    {
        StrCJJ *temp = StrCJJ_create(pathname);
        StrCJJ_appendStr(listCmd, temp);
        StrCJJ_free(temp);
    }
    StrCJJ *rn = StrCJJ_create("\r\n");
    StrCJJ_appendStr(listCmd, rn);
    StrCJJ_free(rn);

    if (!FTPClient_sendCommand(client->controlSocket, listCmd->str, client))
    {
        StrCJJ_free(listCmd);
        close(dataSocket);
        return false;
    }
    StrCJJ_free(listCmd);

    // Receive and check LIST response
    if (!FTPClient_receiveResponse(client->controlSocket, client))
    {
        close(dataSocket);
        return false;
    }

    if (!StrCJJ_startsWith(client->responseBuffer, "150"))
    {
        fprintf(stderr, "Erro ao solicitar lista de ficheiros: %s\n", client->responseBuffer->str);
        close(dataSocket);
        return false;
    }

    // Receive and print the file list
    char buffer[1024] = {0};
    ssize_t bytesRead;
    printf("Lista de ficheiros:\n");
    while ((bytesRead = recv(dataSocket, buffer, sizeof(buffer), 0)) > 0)
    {
        buffer[bytesRead] = '\0';
        printf("%s", buffer);
    }

    if (bytesRead < 0)
    {
        perror("Erro ao receber lista de ficheiros");
        close(dataSocket);
        return false;
    }

    close(dataSocket);

    // Receive and check final response
    if (!FTPClient_receiveResponse(client->controlSocket, client))
    {
        return false;
    }

    if (!StrCJJ_startsWith(client->responseBuffer, "226") && !StrCJJ_startsWith(client->responseBuffer, "225"))
    {
        fprintf(stderr, "Erro ao finalizar lista de ficheiros: %s\n", client->responseBuffer->str);
        return false;
    }

    return true;
}

bool FTPClient_checkForValidClient(FTPClient *client)
{
    bool isValid = true;

    if (client == NULL)
    {
        printf("Error: FTPClient pointer is NULL.\n");
        return false;
    }

    if (client->host == NULL)
    {
        printf("Error: FTPClient->host is NULL.\n");
        isValid = false;
    }
    /*if (client->ip == NULL)
    {
        printf("Error: FTPClient->ip is NULL.\n");
        isValid = false;
    }*/
    if (client->user == NULL)
    {
        printf("Error: FTPClient->user is NULL.\n");
        isValid = false;
    }
    if (client->password == NULL)
    {
        printf("Error: FTPClient->password is NULL.\n");
        isValid = false;
    }
    if (client->urlPath == NULL)
    {
        printf("Error: FTPClient->urlPath is NULL.\n");
        isValid = false;
    }
    if (client->filename == NULL)
    {
        printf("Error: FTPClient->filename is NULL.\n");
        isValid = false;
    }
    /*if (client->responseBuffer == NULL)
    {
        printf("Error: FTPClient->responseBuffer is NULL.\n");
        isValid = false;
    }

    if (client->dataSocket < 0)
    {
        printf("Error: FTPClient->dataSocket < 0\n");
        isValid = false;
    }

    if (client->controlSocket < 0)
    {
        printf("Error: FTPClient->controlSocket < 0\n");
        isValid = false;
    }*/

    return isValid;
}