#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <string.h>
#include <iostream>
#include <fstream>
/* codul de eroare returnat de anumite apeluri */
extern int errno;

using namespace std;

/* portul de conectare la server*/

void Convert(char a[9][9], string s) {
    int i, j, k = 0;
    for(i = 1; i <= 8; i++) {
        for(j = 1; j <= 8; j++)
            a[i][j] = s[k++];
    }
}

void Print(char A[9][9]) {

    int i, j;
    
    for(i = 1; i <= 8; i++) A[i][0] = '1' + i - 1; //bordare pe linie
    for(i = 1; i <= 8; i++) A[9][i] = 'a' + i - 1; // bordare pe coloana
    A[9][0] = '*';
      
    cout << "==============================================================================" << endl;

    for(i = 1; i <= 8; i++) {
        for(j = 0; j <= 8; j++)
            if(j) cout << " | " << A[i][j] << " | " << " ";
            else cout << A[i][j] << "     ";
        cout << endl;
    }
    cout << "==============================================================================" << endl;

    cout << "   *  ";
    for(j = 1; j <= 8; j++) cout << " | " << A[9][j] << " | " << " ";
    cout << endl;
}


int port;

int main (int argc, char *argv[])
{
    int sd;
    struct sockaddr_in server;
    char msg[100];

    if (argc != 3){
        printf ("[client] Sintaxa: %s <adresa_server> <port>\n", argv[0]);
        return -1;
    }

    port = atoi (argv[2]);

    if ((sd = socket (AF_INET, SOCK_STREAM, 0)) == -1){
        perror ("[client] Eroare la socket().\n");
        return errno;
    }

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = inet_addr(argv[1]);
    server.sin_port = htons (port);

    if (connect (sd, (struct sockaddr *) &server,sizeof (struct sockaddr)) == -1){
        perror ("[client]Eroare la connect().\n");
        return errno;
    }

    char a[9][9];

    int x = 0;
    string s;
    s.resize(100);
    if (read (sd, &s[0], s.size()) < 0)
    {
        perror ("[client]Eroare la read() de la server.\n");
        return errno;
    }
    Convert(a, s);
    Print(a);
    cout << endl;

    while(1) {

        bzero (msg, 100);
        printf ("[client]Introduceti mutarea dorita: ");
        fflush (stdout);
        read (0, msg, 100);

        if (write (sd, msg, 100) <= 0)
        {
            perror ("[client]Eroare la write() spre server.\n");
            return errno;
        }
        
        if(strcmp(msg, "surrender\n") == 0) break;


        if (read (sd, msg, 100) < 0)
        {
            perror ("[client]Eroare la read() de la server.\n");
            return errno;
        }
        cout << msg << endl;
		
		if(!strstr(msg, "invalid")){
		    s.resize(100);
		    if (read (sd, &s[0], s.size()) < 0)
		    {
		        perror ("[client]Eroare la read() de la server.\n");
		        return errno;
		    }
		    Convert(a, s);
		    Print(a);
		    cout << endl;
        }
    }
    close (sd);
}

