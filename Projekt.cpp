#define _CRT_SECURE_NO_WARNINGS
#include<stdio.h>
#include<malloc.h>
#include<stdlib.h>
#include"conio2.h"
#include"conio2.cpp"

// projekt ma około 780 linijek

#define SMALLSIZE 9 // 9X9
#define MEDIUMSIZE 13 // 13X13
#define LARGESIZE 19 // 19X19
#define NUMBEROFPLAYERS 2 // Tej stałej od ilości graczy lepiej nie zmieniać
#define BLACK 0 // stałe dotyczące zawartości pola (Field.stone)
#define WHITE 1
#define EMPTY 2
#define MENUXPOSITION 3 // przesunięcie menu w poziomie
#define XOFFSET 60 // przesunięcie planszy w poziomie
#define YOFFSET 2 // przesunięcie planszy w pionie
#define	TOPLEFTCORNER 218 // stałe dotyczące planszy
#define	TOPRIGHTCORNER 191
#define	TOPEREDGE 194
#define	BOTTOMEDGE 193
#define	BOTTOMLEFTCORNER 192
#define	BOTTOMRIGHTCORNER 217
#define	LEFTEDGE 195
#define	RIGHTEDGE 180
#define	INSIDE 197
#define ESC 27 // kody ASCII niektórych klawiszy
#define ENTER 13
#define SPACE 32

struct Field
{
	int koFlag;
	int stone;
	int status; // używane do sprawdzania czy grupa ma oddech
};

struct Game
{
	int size;
	int turn;
	float playerBScore;
	float playerWScore;
	Field** board;
};

// Tworzenie tablicy
int readPlayerInput(); // do pobrania od użytkownika dowolnego rozmiaru
Field** createBoard(int size); // należy pamietać  aby zwolnić potem pamięć przydzieloną przez tę funkcje
void destroyBoard(int size, Field** board); // do zwolnienia używać należy tej funkcji
void fillUpBoard(Field** board, int size); // wypełnia pola planszy (wszędzie daje 0)
void setUpHandicapGame(Field** board, int size, int xOnBoard, int yOnBoard); // ustawiamy sobie grę z handicapem

// Logika Gry
bool isInsideBoard(int currX, int currY, int size); // ta funkcja jest używana do tego abyśmy przypadkiem nie wyszli poza planszę
bool checkIfPlaceingAStoneIsPossible(int currX, int currY, Game game); // sprawdza czy pole spełnia warunki do połorzenia kamienia
int countBreaths(int currX, int currY, Field** board, int size); // liczy oddechy danego kamienia
bool checkIfNeighboursCanBeRemoved(int currX, int currY, int currPlayer, int enemyStone, Field** board, int size); // sprawdza czy sąsiedzi mają jeden oddech i czy będą zbici
bool checlIfGroupIsSurrounded(int currX, int currY, int currPlayer, Field** board, int size); // sprawdzi czy grupa jest otoczona i zmieni "status" kamieni
void removeGroup(int currX, int currY,Game game); // usuwa grupę kamieni (grupa może się składać z jednego kamienia)
void placeStone(int currX, int currY, int currPlayer, Field** board); // kładzie kamień na danej pozycji
void findStonesToRemove(Game game); // przeleci po całej planszy i sprawdzi które kamienia należy usunąć (zmniejszy też flagę ko)
float calculateScore(int colour, Field** board, int size); // liczy ilość kamieni danego koloru na planszy (wynik = zbite + ilość kamieni na planszy)
int calculateStonesAmount(Game game); // liczy ogólną ilość kamieni na planszy (używane do ustalenia ile było zbitych i ile dodać danemu graczowi)

// Wyświetlanie
void displayBoardSizeChoise(bool isHandicapOn); // przy wyborze rozmiaru jest używane
void displayBoard(Field** board, int size); // wyświetla planszę razem z ramką
void displayMenu(int zn,int x, int y, int scoreB, int scoreW); // wyświetla menu i aktualizuje pozycje kursora oraz wyniki graczy

// Inne
void runGame(Game game, bool isHandicapOn, bool isGameLoaded); // funkcja która w której w zasadzie wszystko się dzieje
void readFileName(char* name); // do wprowadzenia nazwy pliku z zapisem
void savaGame(Game game); // zapisywanie
void loadGame(); // wczytywanie

/* MAIN */

int main() {
	
#ifndef __cplusplus
	Conio2_Init();
#endif 
	int zn = 0;
	_setcursortype(_NOCURSOR);
	do {  // menu glówne
		settitle("PopGo");
		gotoxy(MENUXPOSITION, 1);
		cputs("Radoslaw Piotrowicz 193251");
		gotoxy(MENUXPOSITION, 2);
		cputs("a,b,c,d,e,f,g,h,i,j,k");
		gotoxy(MENUXPOSITION, 3);
		cputs("n = new game");
		gotoxy(MENUXPOSITION, 4);
		cputs("l = load game");
		gotoxy(MENUXPOSITION, 5);
		cputs("q = exit");
		zn = getch();
		if (zn == 'n')
		{	
			clrscr();
			gotoxy(MENUXPOSITION, 1);
			cputs("Do you want to play with handicap On 1 - yes 2 - no"); // ustalanie czy gramy z handicapem
			zn = getch();
			if(zn == '1')
				displayBoardSizeChoise(1);
			else if(zn == '2')
				displayBoardSizeChoise(0);
		}
		if (zn == 'l') // wczytywanie
		{
			clrscr();
			gotoxy(MENUXPOSITION, 1);
			cputs("Write your save file name");
			gotoxy(MENUXPOSITION, 2);
			cputs("enter confirm");
			gotoxy(MENUXPOSITION, 3);
			cputs("esc = cancel");
			loadGame();
		}
	} while (zn != 'q');	
	return 0;
}

/* TWORZENIE TABLICY */

Field** createBoard(int size) 
{
	Field** board = (Field**)malloc(size * sizeof(Field*));
	for (int i = 0; i < size; i++)
	{
		board[i] = (Field*)malloc(size * sizeof(Field)); // ostrrzeżenie o przepełnieniu 
	}
	return board;
}

void destroyBoard(int size, Field** board)
{
	for (int i = 0; i < size; i++)
	{
		free(board[i]);
	}
	free(board);
}

int readPlayerInput() 
{
	int zn = 0;
	int customValue = 0;
	do {
		zn = getch();
		if (zn >= 48 && zn <= 57) // zczytujemy tylko cyfry
		{
			putch(zn);
			int	currCh = zn - 48;
			customValue = customValue * 10 + currCh;
		}
	} while (zn != ESC && zn != ENTER);
	clrscr();
	if (zn == ENTER)
		return customValue;
	else
		return 0;
} 

void fillUpBoard(Field** board, int size)
{
	for (int i = 0; i < size; i++)
	{
		for (int j = 0; j < size; j++)
		{
			board[i][j].koFlag = 0;
			board[i][j].stone = EMPTY;
			board[i][j].status = 0;
		}
	}
}

void setUpHandicapGame(Field** board, int size, int xOnBoard, int yOnBoard)
{
	clrscr();
	int zn = 0;
	do {
		textbackground(BLACK);
		textcolor(LIGHTGRAY);
		gotoxy(MENUXPOSITION, 1);
		cputs("enter - finish"); // sterowanie
		gotoxy(MENUXPOSITION, 2);
		cputs("cursors = moving");
		gotoxy(MENUXPOSITION, 3);
		cputs("i = place a stone");
		displayBoard(board, size);
		textbackground(BLACK);
		textcolor(LIGHTGRAY);
		// kursor
		gotoxy(xOnBoard + XOFFSET + 1, yOnBoard + YOFFSET + 1);
		putch('*');
		zn = getch();
		if (zn == 0x48 && isInsideBoard(xOnBoard, yOnBoard - 1, size)) { yOnBoard--; }
		else if (zn == 0x50 && isInsideBoard(xOnBoard, yOnBoard + 1, size)) { yOnBoard++; }
		else if (zn == 0x4b && isInsideBoard(xOnBoard - 1, yOnBoard, size)) { xOnBoard--; }
		else if (zn == 0x4d && isInsideBoard(xOnBoard + 1, yOnBoard, size)) { xOnBoard++; }
		if (zn == 'i') // kładzenie kamieni
		{
			if (isInsideBoard(xOnBoard, yOnBoard, size) && board[xOnBoard][yOnBoard].stone == EMPTY)
				placeStone(xOnBoard, yOnBoard, BLACK, board);
		}
	} while (zn != ENTER);
	clrscr();
	return;
}

/*LOGIKA GRY*/


bool isInsideBoard(int currX, int currY, int size) // sprawdza w oparciu o rozmiary tablicy
{
	if (currX >= 0 && currX <= size - 1 && currY >= 0 && currY <= size - 1)
		return 1;
	else
		return 0;
}

bool checkIfPlaceingAStoneIsPossible(int currX, int currY, Game game)
{
	int enemyStone;
	if (game.turn == BLACK)
		enemyStone = WHITE;
	else
		enemyStone = BLACK;
	
	// gdy nie ma na polu flagi ko
	if (game.board[currX][currY].stone != EMPTY || game.board[currX][currY].koFlag != 0) 
		return 0;
	// gdy ma oddechy
	if (countBreaths(currX, currY, game.board, game.size) > 0)
		return 1;
	// gdy łańcuch może oddychać to możemy postawić
	else if (countBreaths(currX, currY, game.board, game.size) == 0 && checlIfGroupIsSurrounded(currX, currY, game.turn, game.board, game.size) == 0)
	{  
		return 1;
	}
	// gdy zbijemy przeciwnikowi łańcuch
	else if (countBreaths(currX, currY, game.board, game.size) == 0 && checlIfGroupIsSurrounded(currX, currY, enemyStone, game.board, game.size))
	{
		if (checlIfGroupIsSurrounded(currX, currY, game.turn, game.board, game.size) && checkIfNeighboursCanBeRemoved(currX, currY, game.turn, enemyStone, game.board, game.size))
		{
			game.board[currX][currY].status = 1;
			removeGroup(currX, currY,game);
			return 1;
		}
	}
	// gdy po położeniu kamienia nastąpi zbicie pojedyńczego kamienia przeciwnika
	else if (countBreaths(currX, currY, game.board, game.size) == 0 && checkIfNeighboursCanBeRemoved(currX, currY, game.turn, enemyStone, game.board, game.size))
	{
		return 1;
	}	
	else
		return 0;
} 

int countBreaths(int currX, int currY, Field** board, int size)
{
	int breathsCounter = 4; // obszary poza planszą liczymy tak samo jak przeciwników
	if (isInsideBoard(currX, currY - 1, size) != 1 || board[currX][currY - 1].stone != EMPTY)
		breathsCounter--;
	if (isInsideBoard(currX, currY + 1, size) != 1 || board[currX][currY + 1].stone != EMPTY)
		breathsCounter--;
	if (isInsideBoard(currX - 1, currY, size) != 1 || board[currX - 1][currY].stone != EMPTY)
		breathsCounter--;
	if (isInsideBoard(currX + 1, currY, size) != 1 || board[currX + 1][currY].stone != EMPTY)
		breathsCounter--;
	if (isInsideBoard(currX, currY, size))
		return breathsCounter;
	else
		return 0;
}

bool checkIfNeighboursCanBeRemoved(int currX, int currY, int currPlayer, int enemyStone, Field** board, int size) 
{ // sprawdzi z czerech stron i usunie kamień leżący tam jeżeli będzię on otoczony
	if (countBreaths(currX, currY - 1,  board, size) == 1 && board[currX][currY - 1].stone == enemyStone ) // od góry
	{
		placeStone(currX, currY, currPlayer, board);
		if (checlIfGroupIsSurrounded(currX, currY - 1, enemyStone, board, size))
		{
			board[currX][currY - 1].stone = EMPTY;
			board[currX][currY - 1].koFlag = 2;
		}	
		return 1;
	}
	else if (countBreaths(currX, currY + 1,  board, size) == 1 && board[currX][currY + 1].stone == enemyStone) // od dołu
	{
		placeStone(currX, currY, currPlayer, board);
		if (checlIfGroupIsSurrounded(currX, currY + 1, enemyStone, board, size))
		{
			board[currX][currY + 1].stone = EMPTY;
			board[currX][currY + 1].koFlag = 2;
		}	
		return 1;
	}
	else if (countBreaths(currX - 1, currY,  board, size) == 1 && board[currX - 1][currY].stone == enemyStone) // od lewej
	{
		placeStone(currX, currY, currPlayer, board);
		if (checlIfGroupIsSurrounded(currX - 1, currY, enemyStone, board, size))
		{
			board[currX - 1][currY].stone = EMPTY;
			board[currX - 1][currY].koFlag = 2;
		}	
		return 1;
	}
	else if (countBreaths(currX + 1, currY,  board, size) == 1 && board[currX + 1][currY].stone == enemyStone) // od prawej
	{
		placeStone(currX, currY, currPlayer, board);
		if (checlIfGroupIsSurrounded(currX + 1, currY, enemyStone, board, size))
		{
			board[currX + 1][currY].stone = EMPTY;
			board[currX + 1][currY].koFlag = 2;
		}
		return 1;
	}
	else
		return 0;
} 

bool checlIfGroupIsSurrounded(int currX, int currY, int currPlayer, Field** board, int size)
{
	Field** boardCopy = createBoard(size); // tworzymy kopię kablicy
	for (int i = 0; i < size; i++)
	{
		for (int j = 0; j < size; j++)
		{	
			boardCopy[i][j].stone = board[i][j].stone; // kopiujemy zawartość
			boardCopy[i][j].status = 0;
		}
	}
	if (board[currX][currY].stone == EMPTY) // umieszczamy kamień 
	{
		boardCopy[currX][currY].stone = currPlayer;
		boardCopy[currX][currY].status = 0;
	}
	for (int i = 0; i < size; i++)
	{
		for (int j = 0; j < size; j++)
		{
			if (boardCopy[i][j].stone == BLACK || boardCopy[i][j].stone == WHITE) // sprawdzamy oddechy
			{
				if (countBreaths(i, j, boardCopy, size) > 0)
					boardCopy[i][j].status = 1;
				else
					boardCopy[i][j].status = 0;	
			}		
		}
	}
	// propagujemy oddechy dla grup robimy to w dwie strony aby wszędzie się poprawnie rozpropagowały
	for (int i = 0; i < size; i++) // w jedną stronę
	{
		for (int j = 0; j < size; j++)
		{
			if (isInsideBoard(i, j - 1, size) && (boardCopy[i][j - 1].status == 1 && boardCopy[i][j - 1].stone == boardCopy[i][j].stone))
				boardCopy[i][j].status = 1;
			else if (isInsideBoard(i, j + 1, size) && (boardCopy[i][j + 1].status == 1 && boardCopy[i][j + 1].stone == boardCopy[i][j].stone))
				boardCopy[i][j].status = 1;
			else if (isInsideBoard(i - 1, j, size) && (boardCopy[i - 1][j].status == 1 && boardCopy[i - 1][j].stone == boardCopy[i][j].stone))
				boardCopy[i][j].status = 1;
			else if (isInsideBoard(i + 1, j, size) && (boardCopy[i + 1][j].status == 1 && boardCopy[i + 1][j].stone == boardCopy[i][j].stone))
				boardCopy[i][j].status = 1;
		}
	}
	for (int i = size - 1; i >= 0; i--) // w drugą stronę
	{
		for (int j = size - 1; j >= 0; j--)
		{
			if (isInsideBoard(i, j - 1, size) && (boardCopy[i][j - 1].status == 1 && boardCopy[i][j - 1].stone == boardCopy[i][j].stone))
				boardCopy[i][j].status = 1;
			else if (isInsideBoard(i, j + 1, size) && (boardCopy[i][j + 1].status == 1 && boardCopy[i][j + 1].stone == boardCopy[i][j].stone))
				boardCopy[i][j].status = 1;
			else if (isInsideBoard(i - 1, j, size) && (boardCopy[i - 1][j].status == 1 && boardCopy[i - 1][j].stone == boardCopy[i][j].stone))
				boardCopy[i][j].status = 1;
			else if (isInsideBoard(i + 1, j, size) && (boardCopy[i + 1][j].status == 1 && boardCopy[i + 1][j].stone == boardCopy[i][j].stone))
				boardCopy[i][j].status = 1;
		}
	}
	for (int i = 0; i < size; i++) // kopiujemy statusy na orginalną tablicę
	{
		for (int j = 0; j < size; j++)
		{
			if (boardCopy[i][j].stone == BLACK || boardCopy[i][j].stone == WHITE)
			{
				board[i][j].status = boardCopy[i][j].status;
			}
		}
	}
	if (boardCopy[currX][currY].status == 1) // położymy kamień
	{	
		destroyBoard(size, boardCopy);
		return 0;
	}	
	else if(boardCopy[currX][currY].status == 0) // nie położymy kamienia
	{
		destroyBoard(size, boardCopy);
		return 1;
	}
}

void removeGroup(int currX, int currY,Game game)
{
	for (int i = 0; i < game.size; i++)
	{
		for (int j = 0; j < game.size; j++)
		{
			if (game.board[i][j].status == 0) // usunie zarówno grupy jak i pojedyńczę kamienia
			{
				game.board[i][j].stone = EMPTY;
			}
				
		}
	}
}

void placeStone(int currX, int currY, int currPlayer, Field** board)
{
	if (currPlayer == BLACK)
	{
		board[currX][currY].stone = BLACK;
	}
	else if (currPlayer == WHITE)
	{
		board[currX][currY].stone = WHITE;
	}
}

void findStonesToRemove(Game game)
{
	for (int i = 0; i < game.size; i++)
	{
		for (int j = 0; j < game.size; j++)
		{
			if (game.board[i][j].koFlag > 0) // przy okazji zmniejsza flagę ko
				game.board[i][j].koFlag--;
			if (game.board[i][j].stone == BLACK || game.board[i][j].stone == WHITE)
			{
				if (checlIfGroupIsSurrounded(i, j, game.board[i][j].stone, game.board, game.size))	
					removeGroup(i, j, game);
			}			
		}
	}
}

float calculateScore(int colour, Field** board, int size)
{ 
	float score = 0;
	for (int i = 0; i < size; i++)
	{
		for (int j = 0; j < size; j++)
		{
			if (board[i][j].stone == colour)
				score++;
		}
	}
	return score;
}

int calculateStonesAmount(Game game)
{
	int stonesAmount = 0;
	for (int i = 0; i < game.size; i++)
	{
		for (int j = 0; j < game.size; j++)
		{
			if (game.board[i][j].stone == BLACK || game.board[i][j].stone == WHITE)
				stonesAmount++;
		}
	}
	return stonesAmount;
}

/* WYŚWIETLANIE*/

void displayBoardSizeChoise(bool isHandicapOn)
{
	clrscr();
	int zn = 0;
	Game game;
	game.turn = BLACK;
	do {		
		gotoxy(MENUXPOSITION, 1);
		cputs("esc = cancel");
		gotoxy(MENUXPOSITION, 2);
		cputs("1 = 9x9");
		gotoxy(MENUXPOSITION, 3);
		cputs("2 = 13x13");
		gotoxy(MENUXPOSITION, 4);
		cputs("3 = 19x19");
		gotoxy(MENUXPOSITION, 5);
		cputs("4 = Custom");	
		zn = getch();
		switch (zn)
		{
			case '1': // 9X9
				game.size = SMALLSIZE;
				runGame(game,isHandicapOn,0);
				break;
			case '2': // 13X13
				game.size = MEDIUMSIZE;
				runGame(game,isHandicapOn,0);
				break;
			case '3': // 19X19
				game.size = LARGESIZE;
				runGame(game, isHandicapOn,0);
				break;
			case '4': // "dowolny"
				clrscr();
				gotoxy(MENUXPOSITION, 1);
				cputs("esc = cancel");
				gotoxy(MENUXPOSITION, 2);
				cputs("enter = submit");
				gotoxy(MENUXPOSITION, 3);
				cputs("type your custom size");
				game.size = readPlayerInput();
				if (game.size > 0)
					runGame(game,isHandicapOn,0);
				break;
		}
		zn = ESC;
	} while (zn != ESC);
	clrscr();
	return;
}

void displayBoard(Field** board, int size) // wyświetla razem z ramką
{
	textbackground(RED);
	textcolor(LIGHTRED);
	for (int i = 0; i < size+2; i++)
	{
		for (int j = 0; j < size+2; j++)
		{
			if (i > 0 && i <= size && j >0 && j <= size)
			{			
				// tło
				textbackground(BROWN);
				if (board[i - 1][j - 1].stone == BLACK)
					textbackground(BLACK);
				else if (board[i - 1][j - 1].stone == WHITE)
					textbackground(WHITE + 14);
				gotoxy(i + XOFFSET, j + YOFFSET);
				if (j == 1) // wyświetlanie znaków
				{
					if (i == 1)
						putch(TOPLEFTCORNER);
					else if (i == size)
						putch(TOPRIGHTCORNER);
					else
						putch(TOPEREDGE);
				}
				else if (j == size)
				{
					if (i == 1)
						putch(BOTTOMLEFTCORNER);
					else if (i == size)
						putch(BOTTOMRIGHTCORNER);
					else
						putch(BOTTOMEDGE);
				}
				else if (i == 1)
					putch(LEFTEDGE);
				else if (i == size)
					putch(RIGHTEDGE);
				else
					putch(INSIDE);
			}
			// ramka
			if (i == size + 1 || j == size + 1 || i == 0 || j == 0)
			{
				gotoxy(i + XOFFSET, j + YOFFSET);
				putch(SPACE);
				textbackground(RED);
			}	
			textbackground(RED);
		}
	}
}

void displayMenu(int zn, int x, int y, float scoreB, float scoreW)
{
	char buffer[20];
	gotoxy(MENUXPOSITION, 1); // sterowanie
	cputs("q = exit");
	gotoxy(MENUXPOSITION, 2);
	cputs("cursors = moving");
	gotoxy(MENUXPOSITION, 3);
	cputs("i = place a stone");
	gotoxy(MENUXPOSITION, 4);
	cputs("esc = cancel");
	gotoxy(MENUXPOSITION, 5);
	cputs("enter = confirm");
	gotoxy(MENUXPOSITION, 6);
	cputs("s = save game");
	gotoxy(MENUXPOSITION, 7);
	clreol();
	cputs("Current position: "); // pozycja kursora
	sprintf(buffer, "X: %d", x);
	cputs(buffer);
	cputs(" ");
	sprintf(buffer, "Y: %d", y);
	cputs(buffer);
	gotoxy(MENUXPOSITION, 8);
	clreol();
	cputs("Current score: "); // wyniki graczy
	sprintf(buffer, "Black: %.1f", scoreB);
	cputs(buffer);
	cputs(" ");
	sprintf(buffer, "White: %.1f", scoreW);
	cputs(buffer);
}

/* INNE */

void runGame(Game game , bool isHandicapOn, bool isGameLoaded)
{
	int zn = 0;
	int xOnBoard = (game.size / 2); // tworzenie zmiennych
	int yOnBoard = (game.size / 2);
	if (isGameLoaded == 0) // wypełnie planszy oraz jej tworzenie przy nowej grze
	{
		game.board = createBoard(game.size);
		fillUpBoard(game.board, game.size);
		if (isHandicapOn) // ustalanie wyników
		{
			game.playerWScore = 0.5;
			setUpHandicapGame(game.board, game.size, xOnBoard, yOnBoard);
		}
		else
			game.playerWScore = 6.5;
		game.playerBScore = 0.0;
	}	
	_setcursortype(_NOCURSOR);
	
	clrscr();
	do {	
		/*Sekcja od wyświetlania*/
		displayMenu(zn,xOnBoard+1, yOnBoard+1, game.playerBScore + calculateScore(BLACK,game.board,game.size), game.playerWScore + calculateScore(WHITE, game.board, game.size));
		displayBoard(game.board, game.size);
		textbackground(BLACK);
		textcolor(LIGHTGRAY);
		gotoxy(xOnBoard + XOFFSET + 1, yOnBoard + YOFFSET + 1);
		putch('*');
		zn = getch(); // przesuwanie kursora
		if (zn == 0x48 && isInsideBoard(xOnBoard, yOnBoard - 1, game.size)) { yOnBoard--; }
		else if (zn == 0x50 && isInsideBoard(xOnBoard, yOnBoard + 1, game.size)) { yOnBoard++; }
		else if (zn == 0x4b && isInsideBoard(xOnBoard - 1, yOnBoard, game.size)) { xOnBoard--;}
		else if (zn == 0x4d && isInsideBoard(xOnBoard + 1, yOnBoard, game.size)) {xOnBoard++; }
		/*obsługa klawiszy*/
		if (zn == 'i') 
		{
			do {
				zn = getch();
				if (zn == ENTER) // to potwierdzanie jest nie wygodne
				{		
					if (checkIfPlaceingAStoneIsPossible(xOnBoard, yOnBoard, game))
					{
						int before = calculateStonesAmount(game);
						placeStone(xOnBoard, yOnBoard, game.turn, game.board);
						findStonesToRemove(game);
						if (game.turn == BLACK)
							game.turn = WHITE;
						else
							game.turn = BLACK;
						int after = calculateStonesAmount(game);
						if (after <= before)
						{
							if (game.turn == BLACK)
								game.playerWScore += before - after + 1;
							else
								game.playerBScore += before - after + 1;
						}
					}		
				}
			} while (zn != ENTER && zn != ESC);		
		}
		else if (zn == 's')
		{
			clrscr();
			gotoxy(MENUXPOSITION, 1);
			cputs("Write your save file name");
			gotoxy(MENUXPOSITION, 2);
			cputs("enter confirm");
			gotoxy(MENUXPOSITION, 3);
			cputs("esc = cancel");
			savaGame(game);
		}
	} while (zn != 'q');
	destroyBoard(game.size, game.board); // zwalnianie pamięci przydzielonej na tablice
	textbackground(BLACK);
	textcolor(LIGHTGRAY);
	clrscr();
	return;
}

void readFileName(char* fileName)
{
	int zn = 0;
	int i = 0;
	gotoxy(MENUXPOSITION, 4);
	do {
		if (zn != ENTER)
		{
			zn = getch();
			putch(zn);
			fileName[i] = zn;
			i++;
		}	
	} while (zn != ENTER && zn != ESC);
	if (zn == ENTER)
		fileName[i - 1] = '\0'; // koniec nazwy
	else
		fileName[0] = '\0';	
	clrscr();
	return;
}

void savaGame(Game game) // dane w pliku będą ułożone w kolejnych wierszach
{
	FILE *save;
	char fileName[32]; // zakładamy że nazwa nie będzie jakaś strasznie długa
	readFileName(fileName);
	save = fopen(fileName, "wt");
	if (save != NULL)
	{
		fprintf(save, "%d\n", game.size);
		fprintf(save, "%d\n", game.turn);
		fprintf(save, "%.1f\n", game.playerBScore);
		fprintf(save, "%.1f\n", game.playerWScore);
		for (int i = 0; i < game.size; i++)
		{
			for (int j = 0; j < game.size; j++)
			{
				fprintf(save, "%d ", game.board[i][j].koFlag);
				fprintf(save, "%d ", game.board[i][j].stone);
				fprintf(save, "%d\n", game.board[i][j].status);
			}
		}
		fclose(save);
	}	
}

void loadGame() // dosłownie odwrócenie procesu zapisywania
{
	Game game;
	FILE* savedGame;
	char fileName[32];
	readFileName(fileName);
	savedGame = fopen(fileName, "r");
	if (savedGame != NULL)
	{
		fscanf(savedGame, "%d", &game.size);
		fscanf(savedGame, "\n");
		fscanf(savedGame, "%d", &game.turn);
		fscanf(savedGame, "\n");
		fscanf(savedGame, "%f", &game.playerBScore);
		fscanf(savedGame, "\n");
		fscanf(savedGame, "%f", &game.playerWScore);
		fscanf(savedGame, "\n");
		game.board = createBoard(game.size);
		for (int i = 0; i < game.size; i++)
		{
			for (int j = 0; j < game.size; j++)
			{
				fscanf(savedGame, "%d", &game.board[i][j].koFlag);
				fscanf(savedGame, "%d", &game.board[i][j].stone);
				fscanf(savedGame, "%d", &game.board[i][j].status);
				fscanf(savedGame, "\n");
			}
		}
		fclose(savedGame);
		runGame(game, 0, 1);
	}
	return;
}