#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <string.h>
#include <sys/wait.h>
#include <iostream>
#include <fstream>
#include <algorithm>

#define PORT 2728
using namespace std;

extern int errno;

char A[9][9];
int vizA[4] = {0};
int vizB[4] = {0};
void CreateTable(char A[9][9]) {
    int i, j;
    for(i = 1; i <= 8; i++)
        for(j = 1; j <= 8; j++) A[i][j] = '-'; // umplu matricea cu -

    for(i = 1; i <= 8; i++) A[i][0] = '1' + i - 1; //bordare pe linie
    for(i = 1; i <= 8; i++) A[9][i] = 'a' + i - 1; // bordare pe coloana
    A[9][0] = '*';

    for(j = 1; j <= 8; j++) A[2][j] = 'P';
    for(j = 1; j <= 8; j++) A[7][j] = 'p';

    //turele
    A[1][1] = A[1][8] = 'T';
    A[8][1] = A[8][8] = 't';

    //cai
    A[1][2] = A[1][7] = 'C';
    A[8][2] = A[8][7] = 'c';

    //nebuni
    A[1][3] = A[1][6] = 'N';
    A[8][3] = A[8][6] = 'n';

    //Regina
    A[1][4] = 'Q';
    A[8][4] = 'q';

    //Rege
    A[1][5] = 'K';
    A[8][5] = 'k';
}

struct Player {
    int round;
    int fd;
};


string Convert(char A[9][9]) {
    int i, j;
    string s = "";
    for (i = 1; i <= 8; i++) {
        for (j = 1; j <= 8; j++) {
            s += A[i][j];
        }
    }
    return s;
}


int LC(int sr, int sc, int fr, int fc) { //parcurgerea pieselor pe linii si coloane (N S E V)
    int i, j;
    if(fr > sr && fc == sc) {
        i = sr + 1, j = sc;
        while(A[i][j] == '-' && i < fr) i++;
        if(i == fr && j == fc) return 1;
        else return 0;
    }
    else if(fr < sr && fc == sc) {
        i = sr - 1, j = sc;
        while(A[i][j] == '-' && i > fr) i--;
        if(i == fr && j == fc) return 1;
        else return 0;
    }
    else if(fc > sc && fr == sr) {
        i = sr, j = sc + 1;
        while(A[i][j] == '-' && j < fc) j++;
        if(i == fr && j == fc) return 1;
        else return 0;
    }
    else if(fc < sc && fr == sr) {
        i = sr, j = sc - 1;
        while(A[i][j] == '-' && j > fc) j--;
        if(i == fr && j == fc) return 1;
        else return 0;
    }
    else return 0;
}

int DIAG(int sr, int sc, int fr, int fc) { // parcurgere diagonale NE NV SE SV
    int i, j;
    if(fr > sr && fc > sc) {
        i = sr + 1, j = sc + 1;
        while(A[i][j] == '-' && i < fr && j < fc) i++, j++;
        if(i == fr && j == fc) return 1;
        else return 0;
    }
    else if(fr > sr && fc < sc) {
        i = sr + 1, j = sc - 1;
        while(A[i][j] == '-' && i < fr && j > fc) i++, j--;
        if(i == fr && j == fc) return 1;
        else return 0;
    }
    else if(fr < sr && fc > sc) {
        i = sr - 1, j = sc + 1;
        while(A[i][j] == '-' && i > fr && j < fc) i--, j++;
        if(i == fr && j == fc) return 1;
        else return 0;
    }
    else if(fr < sr && fc < sc) {
        i = sr - 1, j = sc - 1;
        while(A[i][j] == '-' && i > fr && j > fc) i--, j--;
        if(i == fr && j == fc) return 1;
        else return 0;
    }
    else return 0;
}

int bigLetter(char letter) {
    if(letter >= 'A' && letter <= 'Z') return 1;
    return 0;
}

int smallLetter(char letter) {
    if(letter >= 'a' && letter <= 'z') return 1;
    return 0;
}

int Pion(char type, int sr, int sc, int fr, int fc) {
    if(bigLetter(type)) {
        if(bigLetter(A[fr][fc])) return 0;
        else if(fr > 8 || fr < 1 || fc > 8 || fc < 1) return 0; //in afara matricii
        else if(fr <= sr) return 0; // nu are voie pionul sa fie dat inapoi
        else if(fr - sr == 2 && sr == 2 && fc == sc && A[fr][fc] == '-') {
            if(smallLetter(A[fr][fc])) return 0;
            else return 1;
        }
        else if(fr - sr == 1 && abs(fc - sc) == 1 && (smallLetter(A[fr][fc]))) return 1;
        else if(fr - sr == 1 && fc == sc && !smallLetter(A[fr][fc])) return 1;

    }
    else if(smallLetter(type)) {
        if(smallLetter(A[fr][fc])) return 0;
        else if(fr > 8 || fr < 1 || fc > 8 || fc < 1) return 0; //in afara matricii
        else if(fr >= sr) return 0; // nu are voie pionul sa fie dat inapoi
        else if(sr - fr == 2 && sr == 7 && fc == sc && A[fr][fc] == '-') {
            if(bigLetter(A[fr][fc])) return 0;
            else return 1;
        }
        else if(sr - fr == 1 && abs(fc - sc) == 1 && (bigLetter(A[fr][fc]))) return 1;
        else if(sr - fr == 1 && fc == sc && !bigLetter(A[fr][fc])) return 1;
    }
}

int Rege(char type, int sr, int sc, int fr, int fc) {
    if(bigLetter(type)) {
        if(bigLetter(A[fr][fc])) return 0;
        else if(fr > 8 || fr < 1 || fc > 8 || fc < 1) return 0; //in afara matricii
        else if((abs(fr - sr) == 1 || abs(fr - sr) == 0) && (abs(fc - sc) == 1 || abs(fc - sc) == 0)) return 1;
        else return 0;
    }
    else if(smallLetter(type)) {
        if(smallLetter(A[fr][fc])) return 0;
        else if(fr > 8 || fr < 1 || fc > 8 || fc < 1) return 0; //in afara matricii
        else if((abs(fr - sr) == 1 || abs(fr - sr) == 0) && (abs(fc - sc) == 1 || abs(fc - sc) == 0)) return 1;
        else return 0;
    }
}

int Cal(char type, int sr, int sc, int fr, int fc) {
    if(bigLetter(type)) {
        if(bigLetter(A[fr][fc])) return 0;
        else if(fr > 8 || fr < 1 || fc > 8 || fc < 1) return 0; //in afara matricii
        if((abs(fr - sr) == 1 || abs(fr - sr) == 2) && (abs(fc - sc) == 1 || abs(fc - sc) == 2)) return 1;
        else return 0;
    }
    else if(smallLetter(type)) {
        if(smallLetter(A[fr][fc])) return 0;
        else if(fr > 8 || fr < 1 || fc > 8 || fc < 1) return 0; //in afara matricii
        if((abs(fr - sr) == 1 || abs(fr - sr) == 2) && (abs(fc - sc) == 1 || abs(fc - sc) == 2)) return 1;
        else return 0;
    }
}

int Tura(char type, int sr, int sc, int fr, int fc) {
    if(bigLetter(type)) {
        if(bigLetter(A[fr][fc])) return 0;
        else if(fr > 8 || fr < 1 || fc > 8 || fc < 1) return 0; //in afara matricii
        else if(LC(sr, sc, fr, fc)) return 1;
        else return 0;
    }
    else if(smallLetter(type)) {
        if(smallLetter(A[fr][fc])) return 0;
        else if(fr > 8 || fr < 1 || fc > 8 || fc < 1) return 0; //in afara matricii
        else if(LC(sr, sc, fr, fc)) return 1;
        else return 0;
    }
}

int Nebun(char type, int sr, int sc, int fr, int fc) {
    if(bigLetter(type)) {
        if(bigLetter(A[fr][fc])) return 0;
        else if(fr > 8 || fr < 1 || fc > 8 || fc < 1) return 0;
        else if(DIAG(sr, sc, fr, fc)) return 1;
        else return 0;
    }
    else if(smallLetter(type)) {
        if(smallLetter(A[fr][fc])) return 0;
        else if(fr > 8 || fr < 1 || fc > 8 || fc < 1) return 0;
        else if(DIAG(sr, sc, fr, fc)) return 1;
        else return 0;
    }
}

int Regina(char type, int sr, int sc, int fr, int fc) {
    int i, j;
    if(bigLetter(type)) {
        if(bigLetter(A[fr][fc])) return 0;
        else if(fr > 8 || fr < 1 || fc > 8 || fc < 1) return 0; //in afara matricii
        else if(DIAG(sr, sc, fr, fc) || LC(sr, sc, fr, fc)) return 1;
        else return 0;
    }
    else if(smallLetter(type)) {
        if(smallLetter(A[fr][fc])) return 0;
        else if(fr > 8 || fr < 1 || fc > 8 || fc < 1) return 0; //in afara matricii
        else if(DIAG(sr, sc, fr, fc) || LC(sr, sc, fr, fc)) return 1;
        else return 0;
    }
}

int validMove(char type, int sr, int sc, int fr, int fc) {
    switch(type) {
    case 'p':
        if(Pion(type, sr, sc, fr, fc)) return 1;
        else return 0;
    case 'P':
        if(Pion(type, sr, sc, fr, fc)) return 1;
        else return 0;
    case 'k':
        if(Rege(type, sr, sc, fr, fc)) return 1;
        else return 0;
    case 'K':
        if(Rege(type, sr, sc, fr, fc)) return 1;
        else return 0;
    case 'c':
        if(Cal(type, sr, sc, fr, fc)) return 1;
        else return 0;
    case 'C':
        if(Cal(type, sr, sc, fr, fc)) return 1;
        else return 0;
    case 't':
        if(Tura(type, sr, sc, fr, fc)) return 1;
        else return 0;
    case 'T':
        if(Tura(type, sr, sc, fr, fc)) return 1;
        else return 0;
    case 'n':
        if(Nebun(type, sr, sc, fr, fc)) return 1;
        else return 0;
    case 'N':
        if(Nebun(type, sr, sc, fr, fc)) return 1;
        else return 0;
    case 'q':
        if(Regina(type, sr, sc, fr, fc)) return 1;
        else return 0;
    case 'Q':
        if(Regina(type, sr, sc, fr, fc)) return 1;
        else return 0;
    }
}

void findMyKing(char type, int &fr, int &fc) {
    int i, j;
    for(i = 1; i <= 8; i++)
        for(j = 1; j <= 8; j++)
            if(A[i][j] == type) {
                fr = i;
                fc = j;
                break;
            }
}

int isAttacked(char type, int fr, int fc) {
    int i, j;
    if(fr < 1 || fr > 8 || fc < 1 || fc > 8) return 0;
    else if(bigLetter(type)) {
        for(i = 1; i <= 8; i++)
            for(j = 1; j <= 8; j++)
                if(smallLetter(A[i][j]))
                    if(validMove(A[i][j], i, j, fr, fc)) return 1;

        return 0;
    }
    else if(smallLetter(type)) {
        for(i = 1; i <= 8; i++)
            for(j = 1; j <= 8; j++)
                if(bigLetter(A[i][j]))
                    if(validMove(A[i][j], i, j, fr, fc)) return 1;

        return 0;
    }
}

int Sah(char type) {
    int fr, fc;
    findMyKing(type, fr, fc);
    if(isAttacked(type, fr, fc)) return 1;
    return 0;
}

void Copy(char B[9][9], char A[9][9]) {
    int i, j;
    for(i = 1; i <= 8; i++)
        for(j = 1; j <= 8; j++)
            B[i][j] = A[i][j];
}

int SahMat(char type) {
    const int dir[8] = {0};
    char B[9][9], C[9][9];
    Copy(B, A);
    Copy(C, A);
    int i, j, fr, fc;
    findMyKing(type, fr, fc);
    if(smallLetter(type)) {
        if(C[fr - 1][fc] == '-') A[fr - 1][fc] = 'K';
        if(C[fr + 1][fc] == '-') A[fr + 1][fc] = 'K';
        if(C[fr - 1][fc - 1] == '-') A[fr - 1][fc - 1] = 'K';
        if(C[fr - 1][fc + 1] == '-') A[fr - 1][fc + 1] = 'K';
        if(C[fr + 1][fc + 1] == '-') A[fr + 1][fc + 1] = 'K';
        if(C[fr + 1][fc - 1] == '-') A[fr + 1][fc - 1] = 'K';
        if(C[fr][fc - 1] == '-') A[fr][fc - 1] = 'K';
        if(C[fr][fc + 1] == '-') A[fr][fc + 1] = 'K';
    }
    else if(bigLetter(type)) {
        if(C[fr - 1][fc] == '-') A[fr - 1][fc] = 'k';
        if(C[fr + 1][fc] == '-') A[fr + 1][fc] = 'k';
        if(C[fr - 1][fc - 1] == '-') A[fr - 1][fc - 1] = 'k';
        if(C[fr - 1][fc + 1] == '-') A[fr - 1][fc + 1] = 'k';
        if(C[fr + 1][fc + 1] == '-') A[fr + 1][fc + 1] = 'k';
        if(C[fr + 1][fc - 1] == '-') A[fr + 1][fc - 1] = 'k';
        if(C[fr][fc - 1] == '-') A[fr][fc - 1] = 'k';
        if(C[fr][fc + 1] == '-') A[fr][fc + 1] = 'k';
    }

    if(Sah(type)) {
        int nr = 1;
        if(isAttacked(type, fr - 1, fc) && B[fr - 1][fc] == '-') B[fr - 1][fc] = 'x', nr++;
        if(isAttacked(type, fr + 1, fc) && B[fr + 1][fc] == '-') B[fr + 1][fc] = 'x', nr++;
        if(isAttacked(type, fr - 1, fc - 1) && B[fr - 1][fc - 1] == '-') B[fr - 1][fc - 1] = 'x', nr++;
        if(isAttacked(type, fr - 1, fc + 1) && B[fr - 1][fc + 1] == '-') B[fr - 1][fc + 1] = 'x', nr++;
        if(isAttacked(type, fr + 1, fc + 1) && B[fr + 1][fc + 1] == '-') B[fr + 1][fc + 1] = 'x', nr++;
        if(isAttacked(type, fr + 1, fc - 1) && B[fr + 1][fc - 1] == '-') B[fr + 1][fc - 1] = 'x', nr++;
        if(isAttacked(type, fr, fc - 1) && B[fr][fc - 1] == '-') B[fr][fc - 1] = 'x', nr++;
        if(isAttacked(type, fr, fc + 1) && B[fr][fc + 1] == '-') B[fr][fc + 1] = 'x', nr++;
        Copy(A, C);
        if(B[fr][fc + 1] == '-' ||  B[fr][fc - 1] == '-' || B[fr + 1][fc - 1] == '-' || B[fr + 1][fc + 1] == '-' || B[fr - 1][fc + 1] == '-' || B[fr - 1][fc - 1] == '-' || B[fr + 1][fc] == '-' || 
        B[fr - 1][fc] == '-') return 0;
        if(nr > 1) return 1;
    }
    return 0;
}

int Message(int fd, int &verify, char move)
{
	string s;
    char buffer[100];
    int bytes;
    char msg[100], save;
    char msgrasp[100]=" ";
    
    bytes = read (fd, msg, sizeof (buffer));
    if (bytes < 0)
    {
        perror ("Eroare la read() de la client.\n");
        return 0;
    }
    
    int sr, sc, fr, fc;
    char type, c1, c2, transform;
    strcpy(msgrasp, msg);
    int i, nr = 0;
    // Preluare coordonate.
    for(i = 0; msgrasp[i]; i++) {
        if(msgrasp[i] != ' ') {
            switch(nr) {
            case 0: {
                sc = (int)msgrasp[i] - 96;
                c1 = msgrasp[i];
            }
            case 1:
                sr = (int)msgrasp[i] - 48;
            case 2: {
                fc = (int)msgrasp[i] - 96;
                c2 = msgrasp[i];
            }
            case 3:
                fr = (int)msgrasp[i] - 48;
            
            case 4:
            	transform = msgrasp[i];
           	}
            nr++;
        }
    }

    save = A[fr][fc];
    type = A[sr][sc];
    //Vizitari piese pentru a verifica daca se poate executa rocada sau nu. Daca piesele au fost mutate rocada nu mai poate avea loc.
    if(type == 'K' && transform != 'F') vizA[2] = 1;
    else if(type == 'T' && sc == 1 && transform != 'F') vizA[1] = 1;
    else if(type == 'T' && sc == 8 && transform != 'F') vizA[3] = 1;
    else if(type == 'k' && transform != 'F') vizB[2] = 1;
    else if(type == 't' && sc == 1 && transform != 'F') vizB[1] = 1;
    else if(type == 't' && sc == 8 && transform != 'F') vizB[3] = 1;
    
    cout << type << " " << sr << " " << sc << " " << fr << " " << fc << " " << transform << endl;

    if(strcmp(msg, "surrender\n") == 0) return -1;
    else if(move == 'a' && SahMat('K')) {
        strcpy(msg, "Castigatorul este jucatorul B");
        write(fd, msg, strlen(msg));
        return -1;
    }
    else if(move == 'b' && SahMat('k')) {
        strcpy(msg, "Castigatorul este jucatorul A");
        write(fd, msg, strlen(msg));
        return -1;
    }
    else if(move == 'a' && smallLetter(type)) {
        strcpy(msg, "Mutare invalida");
        write(fd, msg, strlen(msg));
        return -2;
    }
    else if(move == 'b' && bigLetter(type)) {
        strcpy(msg, "Mutare invalida");
        write(fd, msg, strlen(msg));
        return -2;
    }
    else if(A[sr][sc] == '-')
    {
        strcpy(msg, "Mutare invalida");
        write(fd, msg, strlen(msg));
        return -2;
    }
	else if(transform == 'F'){ //ROCADA
		int ok = 0;
    	if(sr == 1 && fr == 1){
			char t1, t2;
			t1 = A[sr][sc], t2 = A[fr][fc];
			if(sc == 1 && fc == 5){
				if(bigLetter(t1) && bigLetter(t2)){
					if(!isAttacked(t1, fr, fc - 1) && !isAttacked(t2, fr, fc - 2)){
						if(A[sr][sc + 1] == '-' && A[sr][sc + 2] == '-' && A[sr][sc + 3] == '-')
							if(!vizA[1] && !vizA[2]){
								A[sr][sc] = '-';
								A[fr][fc] = '-';
								A[sr][sc + 2] = t2;
								A[sr][fc - 1] = t1;
								vizA[1] = 1;
								vizA[2] = 1;
								ok = 1;
							}
					}
				}
			}
			else if(sc == 5 && fc == 8){
				if(bigLetter(t1) && bigLetter(t2)){
					if(!isAttacked(t1, fr, sc + 1) && !isAttacked(t2, fr, sc + 2)){
						if(A[sr][sc + 1] == '-' && A[sr][sc + 2] == '-')
							if(!vizA[2] && !vizA[3]){
								A[sr][sc] = '-';
								A[fr][fc] = '-';
								A[sr][sc + 1] = t2;
								A[sr][fc - 1] = t1;
								vizA[2] = 1;
								vizA[3] = 1;
								ok = 1;
							}
					}
				}
			}
    	}
    	else if(sr == 8 && fr == 8){
    		char t1, t2;
			t1 = A[sr][sc], t2 = A[fr][fc];
			if(sc == 1 && fc == 5){
				if(smallLetter(t1) && smallLetter(t2)){
					if(!isAttacked(t1, fr, fc - 1) && !isAttacked(t2, fr, fc - 2)){
						if(A[sr][sc + 1] == '-' && A[sr][sc + 2] == '-' && A[sr][sc + 3] == '-')
							if(!vizB[1] && !vizB[2]){
								A[sr][sc] = '-';
								A[fr][fc] = '-';
								A[sr][sc + 2] = t2;
								A[sr][fc - 1] = t1;
								vizB[1] = 1;
								vizB[2] = 1;
								ok = 1;
							}
					}
				}
			}
			else if(sc == 5 && fc == 8){
				if(smallLetter(t1) && smallLetter(t2)){
					if(!isAttacked(t1, fr, sc + 1) && !isAttacked(t2, fr, sc + 2)){
						if(A[sr][sc + 1] == '-' && A[sr][sc + 2] == '-')
							if(!vizB[2] && !vizB[3]){
								A[sr][sc] = '-';
								A[fr][fc] = '-';
								A[sr][sc + 1] = t2;
								A[sr][fc - 1] = t1;
								vizB[2] = 1;
								vizB[3] = 1;
								ok = 1;
							}
					}
				}
			}
    	}
    	
    	if(!ok) {
    		strcpy(msg, "Nu se poate efectua rocada!");
    		write(fd, msg, strlen(msg));
    		return -2;
    	}
    	else if(ok){
    		//PrintTable(A);
    		s = Convert(A);
    		write(fd, s.c_str(), s.size());
    		cout << "Rocarda efectuata!" << endl;
    		verify = 1;
    		return s.size();
    	}
    }
    else if(!validMove(type, sr, sc, fr, fc)) {
        strcpy(msg, "Mutare invalida");
        write(fd, msg, strlen(msg));
        return -2;
    }
    else if(validMove(type, sr, sc, fr, fc)) {
        A[sr][sc] = '-';
        A[fr][fc] = type;
        if(move == 'a' && Sah('K')) {
            strcpy(msg, "Mutare invalida! Sah!");
            write(fd, msg, strlen(msg));
            A[sr][sc] = type;
            A[fr][fc] = save;
            return -2;
        }
        else if(move == 'b' && Sah('k')) {
            strcpy(msg, "Mutare invalida! Sah!");
            write(fd, msg, strlen(msg));
            A[sr][sc] = type;
            A[fr][fc] = save;
            return -2;
        }
        
        // transformare pion in alta piesa cand ajunge pe ultima linie a inamicului
     	if(type == 'p' && fr == 1 && smallLetter(transform) && (transform == 'n' || transform == 'c' || transform == 'q' || transform == 't')) A[sr][sc] = '-', A[fr][fc] = transform;	
		else if(type == 'P' && fr == 8 && bigLetter(transform) && (transform == 'N' || transform == 'C' || transform == 'Q' || transform == 'T')) A[sr][sc] = '-', A[fr][fc] = transform;
		else if((transform != 'n' || transform != 'c' || transform != 'q' || transform != 't' || transform != 'N' || transform != 'C' || transform != 'Q' || transform != 'T') && (type == 'p' || type == 'P')){
			if(fr == 1 || fr == 8){
				strcpy(msg, "Transformare invalida!");
				write(fd, msg, strlen(msg));
				return -2;
    		}
		}
		
        if(move == 'a' && Sah('k')){
        	strcpy(msg, "Mutare efecuata! Regele inamicului este in sah!");
       		s = Convert(A);
       		bytes = s.size();
        }
        else if(move == 'b' && Sah('K')){ 
        	strcpy(msg, "Mutare efecuata! Regele inamicului este in sah!");
       		s = Convert(A);
       		bytes = s.size();
		}
		else{
			strcpy(msg, "Mutare efecuata!");
       		s = Convert(A);
       		bytes = s.size();
		}
		
		//PrintTable(A);

        cout << type << " a fost mutat de pe pozitia " << c1 << " " << sr << " la pozitia " << c2 << " " << fr << endl;
        cout << "Se asteapta mutarea celuilalt jucator!" << endl;
		
		if(fd % 2 == 0){
		    if (bytes && write (fd + 1, s.c_str(), bytes) < 0)
		    {
		        perror ("[server] Eroare la write() catre client.\n");
		        return 0;
		    }
		    
		    if (strlen(msg) && write(fd, msg, strlen(msg)) < 0){
		    	perror ("[server] Eroare la write() catre client.\n");
		        return 0;
		    }
        }
        else if(fd % 2 == 1){
        	if (bytes && write (fd - 1, s.c_str(), bytes) < 0)
		    {
		        perror ("[server] Eroare la write() catre client.\n");
		        return 0;
		    }
		    
		    if (strlen(msg) && write(fd, msg, strlen(msg)) < 0){
		    	perror ("[server] Eroare la write() catre client.\n");
		        return 0;
		    }
        }
        verify = 1;
        return bytes;
    }
}


char * conv_addr (struct sockaddr_in address)
{
    static char str[25];
    char port[7];

    strcpy (str, inet_ntoa (address.sin_addr));
    bzero (port, 7);
    sprintf (port, ":%d", ntohs (address.sin_port));
    strcat (str, port);
    return (str);
}

int main ()
{
    struct sockaddr_in server;
    struct sockaddr_in from;
    fd_set readfds;
    fd_set actfds;
    struct timeval tv;
    int sd, client;
    int optval=1;
    int fd;
    int nfds;
    pid_t childpid;
    unsigned int len;
    int v[250] = {0};

    if ((sd = socket (AF_INET, SOCK_STREAM, 0)) == -1)
    {
        perror ("[server] Eroare la socket().\n");
        return errno;
    }

    setsockopt(sd, SOL_SOCKET, SO_REUSEADDR,&optval,sizeof(optval));

    bzero (&server, sizeof (server));

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = htonl (INADDR_ANY);
    server.sin_port = htons (PORT);

    if (bind (sd, (struct sockaddr *) &server, sizeof (struct sockaddr)) == -1)
    {
        perror ("[server] Eroare la bind().\n");
        return errno;
    }

    if (listen (sd, 10) == -1)
    {
        perror ("[server] Eroare la listen().\n");
        return errno;
    }

    FD_ZERO (&actfds);
    FD_SET (sd, &actfds);

    tv.tv_sec = 1;
    tv.tv_usec = 0;

    nfds = sd;

    printf ("[server] Asteptam la portul %d...\n", PORT);
    fflush (stdout);
    while (1)
    {
        bcopy ((char *) &actfds, (char *) &readfds, sizeof (readfds));

        if (select (nfds+1, &readfds, NULL, NULL, &tv) < 0)
        {
            perror ("[server] Eroare la select().\n");
            return errno;
        }

        if (FD_ISSET (sd, &readfds))
        {
            len = sizeof (from);
            bzero (&from, sizeof (from));

            client = accept (sd, (struct sockaddr *) &from, &len);

            if (client < 0)
            {
                perror ("[server] Eroare la accept().\n");
                continue;
            }

            if (nfds < client)  nfds = client;

            FD_SET (client, &actfds);
            printf("[server] S-a conectat clientul cu descriptorul %d, de la adresa %s.\n",client, conv_addr (from));
            fflush (stdout);
        }

        if(nfds % 2 == 1 && nfds > 3 && !v[nfds - 1] && !v[nfds]) {
			//cout << a.fd << " " << b.fd << endl;
			v[nfds - 1] = 1;
			v[nfds] = 1;
            if((childpid = fork()) == 0) {
            	CreateTable(A);
				//PrintTable(A);
				cout << "Jocul a inceput!" << endl;
				Player a;
				Player b;
				a.round = 1;
				a.fd = nfds - 1;
				b.round = 0;
				b.fd = nfds;
				close(sd);
				int ft = 0;
                while(1) {
                    if(a.round == 1) {
                        fd = a.fd;
                        int verify = 0;
                        if (fd != sd)
                        {	
                        	if(!ft){
                        		string s;
								s = Convert(A);
								int bytes = s.size();
								if (bytes && write (fd, s.c_str(), bytes) < 0)
								{
									 perror ("[server] Eroare la write() catre client.\n");
									 return 0;
								}
								ft = 1;													
                        	}
                            if(Message(fd, verify, 'a') == -1) {
                                close (fd);
                                FD_CLR (fd, &actfds);
                                close(fd + 1);
                                FD_CLR(fd + 1, &actfds);
								v[a.fd] = 0;
								v[b.fd] = 0;
								exit(0);
                                //cout << "Jocul a luat sfarsit! Jucatorul B a castigat!" << endl;
                            }
                            else if(verify == 1) {
                                a.round = 0;
                                b.round = 1;
                                verify = 0;
                            }
                        }
                    }
                    if(b.round == 1) {
                        fd = b.fd;
                        int verify = 0;
                        if (fd != sd)
                        {	
                            if(Message(fd, verify, 'b') == -1) {
                                close (fd);
                                FD_CLR (fd, &actfds);
                                close(fd - 1);
                                FD_CLR(fd - 1, &actfds);
                                v[a.fd] = 0;
								v[b.fd] = 0;
								exit(0);
                                //cout << "Jocul a luat sfarsit! Jucatorul A a castigat!" << endl;
                            }
                            else if(verify == 1) {
                                b.round = 0;
                                a.round = 1;
                                verify = 0;
                            }
                        }
                    }
                }
            }
        }
    }
}
