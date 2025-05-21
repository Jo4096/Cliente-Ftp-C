#include "FtpClientMenu.h"

bool readYN()
{
    char yn = '\0';
    do
    {
        scanf(" %c", &yn);
        while (getchar() != '\n')
            ;
    } while (yn != 'y' && yn != 'Y' && yn != 'n' && yn != 'N');

    return yn == 'y' || yn == 'Y';
}

void printMenu(FTPClient *client)
{
    printf("\n---------------FTP_CLIENT--------------\n");
    if (!client)
    {
        printf("[CLI]: Client não foi criado ainda\n");
        return;
    }

    printf("[CLI]: INFO:\n");

    StrCJJ_print(client->urlPath, " -> URL Path\t", "");
    StrCJJ_print(client->host, " -> Host\t", "");
    if (client->ip)
    {
        if (strcmp(client->ip->str, "") == 0)
        {
            printf(" -> IP\t Conecta te a um server primeiro\n");
        }
        else
        {
            StrCJJ_print(client->ip, " -> IP\t", "");
        }
    }

    printf(" -> Port\t%d\n", client->port);
    StrCJJ_print(client->user, " -> User\t", "");
    printf("\n");

    printf("[CLI]: Escolha uma opção:\n");
    printf(" 1 - Configurar Client\n");
    printf(" 2 - Conectar ao servidor\n");
    printf(" 3 - Fazer login\n");
    printf(" 4 - Listar ficheiros\n");
    printf(" 5 - Descarregar ficheiro\n");
    printf(" 6 - Desconectar\n");
    printf(" 7 - Sair\n");
    printf("Opção: ");
}

void configure(FTPClient *client)
{
    if (!client)
    {
        printf("[CLI]: Client não foi criado ainda. Nada para configurar.\n");
        return;
    }

    printf("[CLI]: --- Configuração do Cliente ---\n");

    // Host
    if (client->host)
    {
        StrCJJ_free(client->host);
    }
    printf("[CLI]: Insira o Host (ex: ftp.example.com): ");
    StrCJJ *hostInput = StrCJJ_input(256);
    client->host = hostInput;

    // User
    if (client->user)
    {
        StrCJJ_free(client->user);
    }
    printf("[CLI]: Insira o Usuário: ");
    StrCJJ *userInput = StrCJJ_input(128);
    client->user = userInput;

    // Password
    if (client->password)
    {
        StrCJJ_free(client->password);
    }
    printf("[CLI]: Insira a Senha: ");
    StrCJJ *passwordInput = StrCJJ_input(128);
    client->password = passwordInput;

    // Port
    printf("[CLI]: Insira a Porta (padrão: 21): ");
    int portInput;
    if (scanf("%d", &portInput) == 1)
    {
        client->port = (uint16_t)portInput;
    }
    else
    {
        printf("[CLI]: Porta inválida. Usando a porta padrão (21).\n");
        client->port = 21;
        // Limpar o buffer de entrada em caso de erro no scanf
        while (getchar() != '\n')
            ;
    }

    // URL Path (opcional, pode ser configurado aqui ou inferido)
    printf("[CLI]: (Opcional) Insira o Path da URL (ex: /pub/file.txt): ");
    while (getchar() != '\n')
        ;
    StrCJJ *urlPathInput = StrCJJ_input(512);
    if (urlPathInput->str[0] != '\0')
    {
        if (client->urlPath)
        {
            StrCJJ_free(client->urlPath);
        }
        client->urlPath = urlPathInput;
        // Tentar inferir o nome do ficheiro se o urlPath foi dado
        if (client->filename)
        {
            StrCJJ_free(client->filename);
            client->filename = NULL;
        }
        char *lastSlash = strrchr(client->urlPath->str, '/');
        if (lastSlash != NULL && lastSlash[1] != '\0')
        {
            client->filename = StrCJJ_create(lastSlash + 1);
        }
    }
    else
    {
        StrCJJ_free(urlPathInput);
        if (!client->urlPath)
        {
            client->urlPath = StrCJJ_create(""); // Inicializar com string vazia se não fornecido
        }
    }

    // IP (será resolvido a partir do Host na conexão, então pode não ser configurado diretamente aqui)
    if (client->ip)
    {
        StrCJJ_free(client->ip);
        client->ip = NULL; // Será resolvido novamente na conexão
    }

    printf("[CLI]: Configuração concluída.\n");
}

bool handleMenu(FTPClient *client)
{
    if (!client)
    {
        return false;
    }

    int8_t choice = -1;
    do
    {
        scanf("%hhd", &choice);
        while (getchar() != '\n')
            ;
    } while (choice < 1 || choice > 7);

    switch (choice)
    {
    case 1:
        configure(client);
        break;
    case 2:
        if (FTPClient_connect(client))
        {
            printf("[CLI]: Conectei me ao server correctamente\n");
        }
        else
        {
            printf("[CLI]: Conectei me ao server...\n");
            sleep(3);
            printf("[CLI]:  Mentira lol, não me conectei tenta outra vez\n");
        }
        break;
    case 3:
        if (!FTPClient_checkForValidClient(client))
        {
            break;
        }

        if (FTPClient_login(client))
        {
            printf("[CLI]: Login foi feito\n");
        }
        else
        {
            printf("[CLI]: Login nao foi feito\n");
            sleep(1);
            printf("[CLI]:  Tenta força bruta, sei la\n");
        }
        break;
    case 4:
        if (!FTPClient_checkForValidClient(client))
        {
            break;
        }
        printf("[CLI]: Que path queres para listar?\n>");
        StrCJJ *input = StrCJJ_input(64);
        if (FTPClient_list(client, input->str))
        {
            printf("[CLI]: esta e a lista\n");
        }
        else
        {
            printf("[CLI]: Tentei nao deu para listar\n");
        }

        StrCJJ_free(input);
        break;
    case 5:
        if (!FTPClient_checkForValidClient(client))
        {
            break;
        }

        StrCJJ_print(client->filename, "[CLI]: Olha o filename é: '", "' queres mudar? [y/n]\n>");

        if (readYN())
        {
            StrCJJ_free(client->filename);
            printf("Novo filename: ");
            client->filename = StrCJJ_input(1024);
            StrCJJ_shrinkToFit(client->filename);
        }

        if (FTPClient_downloadFile(client))
        {
            printf("[CLI]: Download feito\n");
        }
        else
        {
            printf("[CLI]: Download não feito\n");
        }

        break;
    case 6:
        if (!FTPClient_checkForValidClient(client))
        {
            break;
        }
        printf("[CLI]: A desconectar\n");
        FTPClient_disconnect(client);
        printf("[CLI]: Desconectado\n");
        break;
    case 7:
        printf("[CLI]: Queres mesmo sair?[y/n]\n>");
        if (!readYN())
        {
            choice = 0;
        }

        break;
    default:
        printf("[CLI]: INPUT INVALIDO PA\n");
        break;
    }
    return choice != 7;
}

void app(const char *URL)
{
    FTPClient *client = FTPClient_create(URL);
    if (!client)
    {
        fprintf(stderr, "Erro ao criar cliente FTP para URL: %s\n", URL);
        return;
    }

    bool running = true;
    while (running && client)
    {
        printMenu(client);
        running = handleMenu(client);
        if (running)
        {
            printf("Pressiona algo para continuar...");
            getc(stdin);
        }
    }

    FTPClient_destroy(client);
}
