#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdbool.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <unistd.h>
#include <string.h>

#define SIZE 8
#define SQUARE_SIZE 50
#define WINDOW_WIDTH (SIZE * SQUARE_SIZE)
#define WINDOW_HEIGHT (SIZE * SQUARE_SIZE + 50)
bool whiteInCheck = false;
bool blackInCheck = false;


char board[SIZE][SIZE] = {
    {'r', 'n', 'b', 'q', 'k', 'b', 'n', 'r'},
    {'p', 'p', 'p', 'p', 'p', 'p', 'p', 'p'},
    {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '},
    {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '},
    {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '},
    {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '},
    {'P', 'P', 'P', 'P', 'P', 'P', 'P', 'P'},
    {'R', 'N', 'B', 'Q', 'K', 'B', 'N', 'R'}
};
char xd[SIZE][SIZE] = {
    {' ', ' ', 'Q', ' ', ' ', 'b', 'n', 'r'},
    {' ', ' ', ' ', ' ', 'p', ' ', 'p', 'q'},
    {' ', ' ', ' ', ' ', ' ', 'p', 'k', 'r'},
    {' ', ' ', ' ', ' ', ' ', ' ', ' ', 'p'},
    {' ', ' ', ' ', ' ', ' ', ' ', ' ', 'B'},
    {' ', ' ', ' ', ' ', 'P', ' ', ' ', ' '},
    {'P', 'P', 'P', 'P', ' ', 'P', 'P', ' '},
    {'R', 'N', 'B', ' ', 'K', ' ', 'N', 'R'}
};
int enPassant[2][SIZE]={{0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0}};

char currentPlayer = 'W';

bool is_same_case(char a, char b) {
    return (isupper(a) && isupper(b)) || (islower(a) && islower(b));
}

//kopiuje szachownice
char** copyBoard(char original[SIZE][SIZE]) {

    char** newBoard = (char**)malloc(SIZE * sizeof(char*));
    for (int i = 0; i < SIZE; ++i) {
        newBoard[i] = (char*)malloc(SIZE * sizeof(char));
    }

    for (int i = 0; i < SIZE; ++i) {
        for (int j = 0; j < SIZE; ++j) {
            newBoard[i][j] = original[i][j];
        }
    }

    return newBoard;
}

void freeBoard(char** board) {
    for (int i = 0; i < SIZE; ++i) {
        free(board[i]);
    }
    free(board);
}

//kopiuje backup szachownicy na oryginalna
void revertBoard(char original[SIZE][SIZE], char** backup) {
    for (int i = 0; i < SIZE; ++i) {
        for (int j = 0; j < SIZE; ++j) {
            original[i][j] = backup[i][j];
        }
    }
}

bool isMoveInBounds(int srcX, int srcY, int destX, int destY) {
    return srcX >= 0 && srcX < SIZE && srcY >= 0 && srcY < SIZE &&
           destX >= 0 && destX < SIZE && destY >= 0 && destY < SIZE;
}

bool isOwnPiece(char piece) {
    return (currentPlayer == 'W' && isupper(piece)) || (currentPlayer == 'B' && islower(piece));
}

bool isOpponentPiece(char piece) {
    return (currentPlayer == 'W' && islower(piece)) || (currentPlayer == 'B' && isupper(piece));
}

//sprawdza czy pomiedzy pionkiem a docelowym polem znajdują się inne pionki
bool isPathClear(int srcX, int srcY, int destX, int destY) {
    int xDir = (destX > srcX) ? 1 : (destX < srcX) ? -1 : 0;
    int yDir = (destY > srcY) ? 1 : (destY < srcY) ? -1 : 0;
    int x = srcX + xDir, y = srcY + yDir;

    while (x != destX || y != destY) {
        if (board[x][y] != ' ') return false;
        x += xDir;
        y += yDir;
    }
    return true;
}

void forceMovePiece(int srcX, int srcY, int destX, int destY);
bool isWhiteInCheck();
bool isBlackInCheck();
void findKings();

bool whiteKingMoved = false;
bool blackKingMoved = false;
bool whiteLeftRookMoved = false;
bool whiteRightRookMoved = false;
bool blackLeftRookMoved = false;
bool blackRightRookMoved = false;

bool isValidMove(int srcX, int srcY, int destX, int destY, int status) {
    if (!isMoveInBounds(srcX, srcY, destX, destY)) return false;
    char piece = board[srcX][srcY];
    char dest = board[destX][destY];
    if (piece == ' ') return false;
    if (!isOwnPiece(piece)) return false;
    if (isOwnPiece(dest)) return false;
    int xDiff = abs(destX - srcX);
    int yDiff = abs(destY - srcY);
    findKings();

    //Sprawdzanie czy ruch powoduje zablokowanie szacha/czy nie odsloni krola na szach
    char **copied = copyBoard(board);
    forceMovePiece(srcX, srcY, destX, destY);
    if ((currentPlayer == 'B' && isBlackInCheck()) || (currentPlayer == 'W' && isWhiteInCheck())) {
        revertBoard(board, copied);
        freeBoard(copied);
        return false;
    } else {
        revertBoard(board, copied);
        freeBoard(copied);
    }

    //logika wszystkich figur
    // 
    switch (tolower(piece)) {
        case 'p': {
            int dir = (piece == 'P') ? -1 : 1;
            if ((destX == srcX + dir && destY == srcY && dest == ' ') ||
                (destX == srcX + dir && abs(destY - srcY) == 1 && isOpponentPiece(dest)) || 
                (piece == 'P' && srcX == 6 && destX == 4 && destY == srcY && board[5][srcY] == ' ' && dest == ' ') || 
                (piece == 'p' && srcX == 1 && destX == 3 && destY == srcY && board[2][srcY] == ' ' && dest == ' ') || 
                ((enPassant[currentPlayer == 'W' ? 0 : 1][destY] == 2) && (destX == srcX + dir) && (destY == srcY +1 || destY == srcY -1) && (destX == 2 || destX == 5))
                ) {
                    if((enPassant[currentPlayer == 'W' ? 0 : 1][destY] == 2) && (destX == srcX + dir) && (destY == srcY +1 || destY == srcY -1&& (destX == 2 || destX == 5))){
                        board[srcX][destY] = ' ';
                    }
                
                return true;
            }
            break;
        }
        case 'r':

            if(srcX != destX && srcY != destY) return false;
            
            if(srcX == destX || srcY == destY){ 
                if(isPathClear(srcX, srcY, destX, destY)){
                    if (srcX == 7 && srcY == 0) whiteLeftRookMoved = true;
                    if (srcX == 7 && srcY == 7) whiteRightRookMoved = true;
                    if ((srcX == 0 && srcY == 0) && (board[0][0] != 'r')) blackLeftRookMoved = true;
                    if (srcX == 0 && srcY == 7 && (board[0][7] != 'r')) blackRightRookMoved = true;
                    return true;
                }
            }
        case 'n':
            return (xDiff == 2 && yDiff == 1) || (xDiff == 1 && yDiff == 2);
        case 'b':
            return (xDiff == yDiff) && isPathClear(srcX, srcY, destX, destY);
        case 'q':
            return ((srcX == destX || srcY == destY) || (xDiff == yDiff)) && isPathClear(srcX, srcY, destX, destY);
        case 'k':
            if (xDiff <= 1 && yDiff <= 1) {
                if(piece == 'k') blackKingMoved = true;
                if(piece == 'K') whiteKingMoved = true;
                return true;
            }else if(piece == 'K'){ //roszada
                //roszada dluga bialego
                if(!whiteKingMoved && !whiteLeftRookMoved && destX == 7 && destY == 2 && board[7][1] == ' ' && board[7][2] == ' ' && board[7][3] == ' ' && board[7][0] == 'R' && !isWhiteInCheck()){//sprawdza czy krol i wieza nie wykonaly jeszcze ruchu +  czy nie ma figur pomiedzy krolem a wieza.
                     //sprawdza czy pola przez ktore przechodzi krol nie sa szachowane
                     char **copied = copyBoard(board);
                     forceMovePiece(7,4,7,3);
                     if(isWhiteInCheck()){
                        revertBoard(board, copied);
                        freeBoard(copied);
                        return false;
                     }else{
                        forceMovePiece(7,3,7,2);
                        if(isWhiteInCheck()){
                            revertBoard(board, copied);
                            freeBoard(copied);
                            return false;
                        }else{
                            revertBoard(board, copied);
                            freeBoard(copied);
                            board[7][0] = ' ';
                            board[7][3] = 'R';
                            return true;
                        }
                     }
                    return false;
                //roszada krotka bialego
                }else if(!whiteKingMoved && !whiteRightRookMoved && destX == 7 && destY == 6 && board[7][6] == ' ' && board[7][5] == ' '&& board[7][7] == 'R' && !isWhiteInCheck()){//sprawdza czy krol i wieza nie wykonaly jeszcze ruchu +  czy nie ma figur pomiedzy krolem a wieza.
                     //sprawdza czy pola przez ktore przechodzi krol nie sa szachowane
                     char **copied = copyBoard(board);
                     forceMovePiece(7,4,7,5);
                     if(isWhiteInCheck()){
                        revertBoard(board, copied);
                        freeBoard(copied);
                        return false;
                     }else{
                        forceMovePiece(7,5,7,6);
                        if(isWhiteInCheck()){
                            revertBoard(board, copied);
                            freeBoard(copied);
                            return false;
                        }else{
                            revertBoard(board, copied);
                            freeBoard(copied);
                            board[7][7] = ' ';
                            board[7][5] = 'R';
                            return true;
                        }
                     }
                    return false;
                }
            } else if(piece == 'k'){
                //roszada dluga czarnego
                //sprawdza czy krol i wieza nie wykonaly jeszcze ruchu +  czy nie ma figur pomiedzy krolem a wieza.
                if(!blackKingMoved && !blackLeftRookMoved && destX == 0 && destY == 2 && board[0][1] == ' ' && board[0][2] == ' ' && board[0][3] == ' ' && board[0][0] == 'r' && !isBlackInCheck()){
                    
                     //sprawdza czy pola przez ktore przechodzi krol nie sa szachowane
                     char **copied = copyBoard(board);
                     forceMovePiece(0,4,0,3);
                     if(isBlackInCheck()){
                        revertBoard(board, copied);
                        freeBoard(copied);
                        return false;
                     }else{
                        forceMovePiece(0,3,0,2);
                        if(isBlackInCheck()){
                            revertBoard(board, copied);
                            freeBoard(copied);
                            return false;
                        }else{
                            revertBoard(board, copied);
                            freeBoard(copied);
                            board[0][0] = ' ';
                            board[0][3] = 'r';
                            return true;
                        }
                     }
                    return false;
                //roszada krotka czarnego
                }else if(!blackKingMoved && !blackRightRookMoved && destX == 0 && destY == 6 && board[0][6] == ' ' && board[0][5] == ' '&& board[0][7] == 'r' && !isBlackInCheck()){//sprawdza czy krol i wieza nie wykonaly jeszcze ruchu +  czy nie ma figur pomiedzy krolem a wieza.
                     //sprawdza czy pola przez ktore przechodzi krol nie sa szachowane
                     char **copied = copyBoard(board);
                     forceMovePiece(0,4,0,5);
                     if(isBlackInCheck()){
                        revertBoard(board, copied);
                        freeBoard(copied);
                        return false;
                     }else{
                        forceMovePiece(0,5,0,6);
                        if(isBlackInCheck()){
                            revertBoard(board, copied);
                            freeBoard(copied);
                            return false;
                        }else{
                            revertBoard(board, copied);
                            freeBoard(copied);
                            board[0][7] = ' ';
                            board[0][5] = 'r';
                            return true;
                        }
                     }
                    return false;
                }
        }
    }
    return false;
}

bool movePiece(int srcX, int srcY, int destX, int destY) {
    if (isValidMove(srcX, srcY, destX, destY, 1)) {
        board[destX][destY] = board[srcX][srcY];
        if(board[srcX][srcY] == 'p' || board[srcX][srcY] == 'P'){
            enPassant[currentPlayer == 'W' ? 1 : 0][srcY] += enPassant[currentPlayer == 'W' ? 1 : 0][srcY] > 6 ? 0 : 1;
        }
        board[srcX][srcY] = ' ';
        
        return true;
    } else {
        printf("Invalid move\n");
        return false;
    }
}

//pomocnicza funkcja wymuszająca ruch, uzywana do szachowania
void forceMovePiece(int srcX, int srcY, int destX, int destY) {
        board[destX][destY] = board[srcX][srcY];
        board[srcX][srcY] = ' ';
}

void switchPlayer() {
    currentPlayer = (currentPlayer == 'W') ? 'B' : 'W';
}

//funkcja do tworzenia nowych nazw savea (board_state_n)
char *getUniqueFilename(const char *baseFilename, char *uniqueFilename) {
    char tempFilename[256]; // Zakładam maksymalną długość nazwy pliku
    strcpy(tempFilename, baseFilename);

    //sprawdzanie istnienia pliku o danej nazwie
    int counter = 0;
    while (access(tempFilename, F_OK) == 0) {
        counter++;
        sprintf(tempFilename, "%s_%d", baseFilename, counter);
    }

    strcpy(uniqueFilename, tempFilename);
    return uniqueFilename;
}

// Funkcja zapisująca stan gry do pliku
void saveBoard(const char *baseFilename, char board[8][8], char currentPlayer) {
    char uniqueFilename[256];
    getUniqueFilename(baseFilename, uniqueFilename);
    FILE *file = fopen(uniqueFilename, "w");
    if (file == NULL) {
        printf("Unable to open file for saving\n");
        return;
    }

    // Zapis aktualnego gracza
    fprintf(file, "Current Player: %c\n", currentPlayer);

    // Zapis stanu planszy
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            fprintf(file, "%c", board[i][j]);
        }
        fprintf(file, "\n");
    }
    //zapis tablicy ze stanem en passant
    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 8; j++) {
            fprintf(file, " %d", enPassant[i][j]);
        }
        fprintf(file, "\n");
    }
    fclose(file);
}


//Rysowanie gry
void drawBoard(SDL_Renderer *renderer) {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    //fonty
    TTF_Font *font = TTF_OpenFont("CASEFONT.ttf", 24);
    TTF_Font *textFont = TTF_OpenFont("arial.ttf", 18);
    if (!font || !textFont) {
        printf("Failed to load font: %s\n", TTF_GetError());
        return;
    }

    SDL_Color textColor = {255, 255, 255};
    //current player
    char playerTurnText[20];
    sprintf(playerTurnText, "CURRENT PLAYER: %c", currentPlayer);
    SDL_Surface *playerTurnSurface = TTF_RenderText_Solid(textFont, playerTurnText, textColor);
    SDL_Texture *playerTurnTexture = SDL_CreateTextureFromSurface(renderer, playerTurnSurface);
    SDL_Rect playerTurnRect = {10, 10, 200, 30};
    SDL_RenderCopy(renderer, playerTurnTexture, NULL, &playerTurnRect);
    //zapis
     SDL_Surface *saveSurface = TTF_RenderText_Solid(textFont, "SAVE", textColor);
    SDL_Texture *saveTexture = SDL_CreateTextureFromSurface(renderer, saveSurface);
    SDL_Rect saveRect = {WINDOW_WIDTH - 110, 10, 60, 30};
    SDL_RenderCopy(renderer, saveTexture, NULL, &saveRect);

    SDL_DestroyTexture(playerTurnTexture);
    SDL_FreeSurface(playerTurnSurface);

    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            SDL_Rect rect = {j * SQUARE_SIZE, i * SQUARE_SIZE + 50, SQUARE_SIZE, SQUARE_SIZE};

            //kolory szachownicy
            if ((i + j) % 2 == 0) {
                SDL_SetRenderDrawColor(renderer, 20, 107, 45, 255); //zielony
            } else {
                SDL_SetRenderDrawColor(renderer, 151, 163, 124, 255); //niebieski
            }
            SDL_RenderFillRect(renderer, &rect);

            if (board[i][j] != ' ') {
                //kolory pionkow
                SDL_Color pieceColor = isupper(board[i][j]) ? (SDL_Color){255, 255, 255} : (SDL_Color){0, 0, 0};
                
                //wyswietlanie wszystkich pionkow jako uppercase (zeby lepiej dzialalo z tym fontem)
                char piece = tolower(board[i][j]);

                SDL_Surface *pieceSurface = TTF_RenderText_Solid(font, (char[2]){piece, '\0'}, pieceColor);
                SDL_Texture *pieceTexture = SDL_CreateTextureFromSurface(renderer, pieceSurface);

                SDL_Rect pieceRect = {j * SQUARE_SIZE + 10, i * SQUARE_SIZE + 60, SQUARE_SIZE - 20, SQUARE_SIZE - 20};
                SDL_RenderCopy(renderer, pieceTexture, NULL, &pieceRect);

                SDL_DestroyTexture(pieceTexture);
                SDL_FreeSurface(pieceSurface);
            }
        }
    }


    SDL_RenderPresent(renderer);

    TTF_CloseFont(font);
    TTF_CloseFont(textFont);
}

int kings[][2]={{-1, -1},{-1, -1}};
void findKings(){
    for (int i = 0; i< SIZE; i++){
        for (int j = 0; j< SIZE; j++){
            if(board[i][j] == 'K'){
                kings[0][0] = i;
                kings[0][1] = j;
            }
            if(board[i][j] == 'k'){
                kings[1][0] = i;
                kings[1][1] = j;
            }
        }
    }
}
bool helperCheckFunction(int srcX, int srcY, int destX, int destY) {
    if (!isMoveInBounds(srcX, srcY, destX, destY)) return false;
    char piece = board[srcX][srcY];
    char dest = board[destX][destY];
    if (piece == ' ') return false;
    if (is_same_case(piece, dest) && dest != ' ') return false;
    int xDiff = abs(destX - srcX);
    int yDiff = abs(destY - srcY);

    switch (tolower(piece)) {
    case 'p': {
        int dir = (piece == 'P') ? -1 : 1;
        if ((destX == srcX + dir && destY == srcY && dest == ' ') ||
            (destX == srcX + dir && abs(destY - srcY) == 1 && isOpponentPiece(dest)) ||
            (piece == 'P' && srcX == 6 && destX == 4 && destY == srcY && board[5][srcY] == ' ' && dest == ' ') ||
            (piece == 'p' && srcX == 1 && destX == 3 && destY == srcY && board[2][srcY] == ' ' && dest == ' ') || 
                ((enPassant[currentPlayer == 'W' ? 0 : 1][destY] == 2) && (destX == srcX + dir) && (destY == srcY +1 || destY == srcY -1))
                ) {
            return true;
        }
        break;
    }
    case 'r':
        return (srcX == destX || srcY == destY) && isPathClear(srcX, srcY, destX, destY);
    case 'n':
        return (xDiff == 2 && yDiff == 1) || (xDiff == 1 && yDiff == 2);
    case 'b':
        return (xDiff == yDiff) && isPathClear(srcX, srcY, destX, destY);
    case 'q':
        return ((srcX == destX || srcY == destY) || (xDiff == yDiff)) && isPathClear(srcX, srcY, destX, destY);
    case 'k':
        return xDiff <= 1 && yDiff <= 1;
    }
    return false;
}
bool isKingInCheck(int kingX, int kingY) {
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            char piece = board[i][j];
            if (helperCheckFunction(i, j, kingX, kingY)) {
                return true;
            }
        }
    }
    return false;
}

bool isWhiteInCheck() {
    findKings();
    return isKingInCheck(kings[0][0], kings[0][1]);
}

bool isBlackInCheck() {
    findKings();
    return isKingInCheck(kings[1][0], kings[1][1]);
}

bool checkMateFlag = false;

bool helperCheckMateFunction(int srcX, int srcY) {
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            int destX = i;
            int destY = j;
            if (isValidMove(srcX, srcY, destX, destY, 0)) {
                char** copied = copyBoard(board);
                forceMovePiece(srcX, srcY, destX, destY);
                if ((currentPlayer == 'W' && !isBlackInCheck()) || (currentPlayer == 'B' && !isWhiteInCheck())) {
                    revertBoard(board, copied);
                    freeBoard(copied);
                    return true;
                }
                revertBoard(board, copied);
                freeBoard(copied);
            }
        }
    }
    return false;
}


//checks every possilbe move of every opponents piece :-)
bool isCheckMate() {
    if (isBlackInCheck() || isWhiteInCheck()) {
        char opponentPlayer = (currentPlayer == 'W') ? 'B' : 'W';
        for (int i = 0; i < SIZE; i++) {
            for (int j = 0; j < SIZE; j++) {
                char piece = board[i][j];
                if ((isupper(piece) == (opponentPlayer == 'B')) || (islower(piece) == (opponentPlayer == 'W'))) {
                    if (helperCheckMateFunction(i, j)) {
                        return false;
                    }
                }
            }
        }
        return true;
    }
    return false;
}


bool helperStalemateFunction(int srcX, int srcY) {
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            int destX = i;
            int destY = j;
            if (isValidMove(srcX, srcY, destX, destY, 0)) {
                char** copied = copyBoard(board);
                forceMovePiece(srcX, srcY, destX, destY);
                if ((currentPlayer == 'B' && !isBlackInCheck()) || (currentPlayer == 'W' && !isWhiteInCheck())) {
                    revertBoard(board, copied);
                    freeBoard(copied);
                    return false;
                }
                revertBoard(board, copied);
                freeBoard(copied);
            }
        }
    }
    return true;
}

bool isStaleMate() {
    if (isBlackInCheck() || isWhiteInCheck()) {
        return false; //jest szach -> nie ma patu
    }

    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            if (!helperStalemateFunction(i, j)) {
                return false; //jest ruch
            }
        }
    }
    return true;
}

void drawCheckMateScreen(SDL_Renderer *renderer, const char *winnerColor) {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    TTF_Font *font = TTF_OpenFont("arial.ttf", 48);
    if (!font) {
        printf("Failed to load font: %s\n", TTF_GetError());
        return;
    }

    SDL_Color textColor = {255, 255, 255};

    char winnerText[30];
    sprintf(winnerText, "%s PLAYER WINS!", winnerColor);
    SDL_Surface *winnerSurface = TTF_RenderText_Solid(font, winnerText, textColor);
    SDL_Texture *winnerTexture = SDL_CreateTextureFromSurface(renderer, winnerSurface);
    SDL_Rect winnerRect = {WINDOW_WIDTH / 2 - 200, WINDOW_HEIGHT / 2 - 50, 400, 100};
    SDL_RenderCopy(renderer, winnerTexture, NULL, &winnerRect);

    SDL_DestroyTexture(winnerTexture);
    SDL_FreeSurface(winnerSurface);
    TTF_CloseFont(font);

    SDL_RenderPresent(renderer);
}

void drawStalemateScreen(SDL_Renderer *renderer) {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    TTF_Font *font = TTF_OpenFont("arial.ttf", 48);
    if (!font) {
        printf("Failed to load font: %s\n", TTF_GetError());
        return;
    }

    SDL_Color textColor = {255, 255, 255};

    char winnerText[30];
    sprintf(winnerText, "Stalemate.");
    SDL_Surface *winnerSurface = TTF_RenderText_Solid(font, winnerText, textColor);
    SDL_Texture *winnerTexture = SDL_CreateTextureFromSurface(renderer, winnerSurface);
    SDL_Rect winnerRect = {WINDOW_WIDTH / 2 - 200, WINDOW_HEIGHT / 2 - 50, 400, 100};
    SDL_RenderCopy(renderer, winnerTexture, NULL, &winnerRect);

    SDL_DestroyTexture(winnerTexture);
    SDL_FreeSurface(winnerSurface);
    TTF_CloseFont(font);

    SDL_RenderPresent(renderer);
}

void loadBoard(const char *filename, char board[][SIZE]) {
    FILE *file = fopen(filename, "r");
    if (file != NULL) {
        //wczytaj gracza
        fscanf(file, "Current Player: %c\n", &currentPlayer);
        
        //wczytaj pionki
        for (int i = 0; i < SIZE; i++) {
            for (int j = 0; j < SIZE; j++) {
                fscanf(file, "%c", &board[i][j]);
            }
            fseek(file, 1, SEEK_CUR);
        }
        for (int i = 0; i < 2; i++) {
            for (int j = 0; j < 8; j++) {
                fscanf(file, " %d", &enPassant[i][j]);
            }
            fseek(file, 1, SEEK_CUR);
        }

        fclose(file);
    } else {
        printf("Could not open file: %s\n", filename);
    }
}

void incrementEnPassant(){
    for(int i = 0; i < 2; i++){
        for (int j = 0; j < 8; j++){
            if (enPassant[i][j] > 0 && enPassant[i][j] < 7){
                enPassant[i][j] += 1;
            }
        }
    }
}

int main(int argc, char *argv[]) {
    char *filename;
    if (argc > 1) {
        filename = argv[1];
        loadBoard(filename, board);
    }

    char baseFilename[] = "board_state";
    char uniqueFilename[256];
    FILE *file = fopen(uniqueFilename, "w");
    SDL_Init(SDL_INIT_VIDEO);
    TTF_Init();

    SDL_Window *window = SDL_CreateWindow("Chess", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN);
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    bool quit = false;
    SDL_Event event;
    int srcX = -1, srcY = -1, destX = -1, destY = -1;
    bool awaitingMove = false;
    bool gameOver = false;
    bool stalemate = false;
    while (!quit && !gameOver) {
        isCheckMate();
        if (isCheckMate()) {
            const char *winnerColor = (currentPlayer == 'W') ? "WHITE" : "BLACK";
            gameOver = true;
        }
        if (currentPlayer == 'B' && isCheckMate()) {
        const char *winnerColor = (currentPlayer == 'W') ? "WHITE" : "BLACK";
        gameOver = true;
    }
        if (isStaleMate()){
            gameOver = true;
            stalemate = true;
        }
        while (SDL_PollEvent(&event) != 0) {
            if (event.type == SDL_QUIT) {
                quit = true;
            } else if (event.type == SDL_MOUSEBUTTONDOWN && awaitingMove) {
                
                int x = event.button.x / SQUARE_SIZE;
                int y = (event.button.y - 50) / SQUARE_SIZE;
                if (event.button.x >= WINDOW_WIDTH - 110 && event.button.x <= WINDOW_WIDTH - 50 && event.button.y  >= 10 && event.button.y  <= 40) {
                    saveBoard("board_state", board, currentPlayer);
                    printf("Game state saved!\n");
                    goto flag;
                }
                if (srcX == -1 && srcY == -1) {
                    srcX = y;
                    srcY = x;
                } else {
                    destX = y;
                    destY = x;
                    isWhiteInCheck();
                    isBlackInCheck();
                    bool moveSuccessful = movePiece(srcX, srcY, destX, destY);
                    if (!moveSuccessful) {
                        printf("Invalid move. Try again.\n");
                        findKings();
                    } else {
                        incrementEnPassant();
                        switchPlayer();
                        awaitingMove = false;
                        
                    }
                    srcX = -1;
                    srcY = -1;
                    destX = -1;
                    destY = -1;
                }
            }
        }

        if (!awaitingMove) {
            drawBoard(renderer);
            awaitingMove = true;
            
        }
        flag:
        if(true){}
    }
     if (gameOver && !stalemate) {
        const char *winnerColor = (currentPlayer == 'W') ? "BLACK" : "WHITE";
        drawCheckMateScreen(renderer, winnerColor);
        SDL_Event exitEvent;
        while (SDL_WaitEvent(&exitEvent) && exitEvent.type != SDL_QUIT) {
        }
    }
    if (stalemate){
        drawStalemateScreen(renderer);
        SDL_Event exitEvent;
        while (SDL_WaitEvent(&exitEvent) && exitEvent.type != SDL_QUIT) {
        }
    }
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    TTF_Quit();

    return 0;
}
