# Cliente-Ftp-C

Cliente FTP escrito em C, desenvolvido para o projeto RCOM.

## Descrição

Este projeto é um cliente FTP simples implementado em C, destinado a comunicar com servidores FTP para operações básicas como listagem, download e upload de ficheiros.

---

## Compilação

Para compilar o programa, usa o `Makefile` incluído:
make

- correr o ./clientFtp sem parametros faz com que tenha que ser configurado manualmente as coisas.
- ./clientFtp ftp://example.com/ficheiro.txt -> extrai logo ficheiro que queremos, mas nao tem user nem palavra passe, tem de ser configurada.
- ./clientFtp ftp://demo:password@test.rebex.net/readme.txt -> extrai user, password, e ficheiro.
