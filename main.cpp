// ASCIIcrush

// Sebastian Luciano Gallardo
// 17-12-18

// CandyCrush-like console videogame where you eliminate ASCII symbols, with a money system and a few special abilities. 
// The objective is to eliminate all the symbols of a kind from the board in the lowest amount of moves

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define MAX_MESSAGE_LENGTH 128
#define WIDTH_DEFAULT 10
#define NSYMB_DEFAULT 7
#define WIDTH_MIN 3
#define WIDTH_MAX 10
#define NSYMB_MAX 9
#define NSYMB_MIN 4

#define BOMB_COST 100
#define ROTATION_COST 50


//Text color modifiers made to work with GNOME terminals
#define TXT_NORMAL "\033[0m"
#define TXT_WHITE "\033[1;37m"
#define TXT_RED "\033[1;31m"
#define TXT_GREEN "\033[1;32m"
#define TXT_BLUE "\033[1;34m"
#define TXT_CYAN "\033[1;36m"
#define TXT_YELLOW "\033[1;33m"
#define TXT_MAGENTA "\033[1;35m"
#define TXT_GRAY "\033[1;90m"
#define TXT_SPECIAL "\033[5;37m"

unsigned points;
unsigned moves;
unsigned multiplier;
char sel;
char message[MAX_MESSAGE_LENGTH];

enum{RIGHT, UP, LEFT, DOWN};


// Here we define a board class. The whole game is played by defining a board and the player movements 
// are implemented as class functions.
class ASCIIboard{
    private:
        unsigned board_width;
        unsigned nsymb;
        unsigned **cell;
    public:
        char *symbols;
        unsigned width(void){return board_width;}
        unsigned nsymbols(void){return nsymb;}


        ASCIIboard(unsigned w, unsigned nsymbols){
            // Class constructor, takes board width and number (types) of symbols
            board_width = w;
            nsymb = nsymbols;

            // Reserve memory space for the board
            cell = new unsigned* [board_width];
            for(unsigned x=0; x<board_width; x++) cell[x] = new unsigned [board_width];

            //Generate the intial state of the board randomly. But make sure the initial state is not about to "crush"!
            do{
                for(unsigned x=0;  x<board_width; x++) for(unsigned y=0; y<board_width; y++) cell[x][y] = 0;
                refill();
            }while(crush(false) || checkWin());

            //We define a char vector with the characters corresponding to each symbol number.
            symbols = new char [nsymbols+1];
            symbols[0] = ' ';

            char c;
            bool existant;
            bool not_nice;

            //Now we randomly generate the symbol characters.
            for(unsigned i=1; i<=nsymbols; i++){
                do{
                    existant = false;
                    c = 33+rand()%94; //we only want characters in this range because these are the ones which look good.
                    symbols[i] = c;
                    for(unsigned k=1; k<i; k++) if(symbols[k] == c) existant = true;

                //Some characters are not very nice to work with, and also we don't want two symbol numbers to have the same character.
                //So we check for this. 
                //Here's the criterion for "non-niceness", where we exclude certain ASCII characters:
                not_nice = (c<= 59 && c >= 44) || (c <= 90 && c >= 65) || (c <= 119 && c>= 100) || (c <= 125 && c >= 123) || (c <= 41 && c >= 39) || (c <= 96 && c >= 93) || c==34 || c==91;
                
                }while(not_nice || existant);
            }
        }

        ~ASCIIboard(){
            for(unsigned x=0; x<board_width; x++) delete[] cell[x];
            delete[] cell;
            delete[] symbols;
        }

        void print();
        void printSymb(unsigned symb);
        void move();
        void rotateL();  //counterclockwise
        void rotateR();  //clockwise
        void bomb();
        bool crush(bool addPoints);
        void slide();
        void refill();
        unsigned checkWin();  //returns the symbol number that was eliminated from the board (or 0 if none was eliminated)
};

void playASCIIcrush(unsigned widthSel, unsigned nsymbSel);
void sendMessage(const char *send);
void showMessage(void);
void readU(unsigned *u);
void readDir(unsigned *u);

int main(){
    srand(time(NULL));
    message[0] = 0;
    unsigned widthSel = WIDTH_DEFAULT, nsymbSel = NSYMB_DEFAULT;

    while(true){
        //Main menu loop, we won't get out of here until the player chooses to exit or something goes wrong.

        system("clear");
        printf(TXT_NORMAL"ASCIIcrush\nSebastian Luciano Gallardo\n19-12-18\n\n");
        printf(TXT_WHITE"1: Play\n2: How to play\n3: Settings\n0: Exit\n");
        showMessage();
        scanf(" %c",&sel);

        //Player selection switch
        switch(sel){

            case '1':   //Play
                playASCIIcrush(widthSel,nsymbSel);
                break;
            case '2':   //How to play
                system("clear");
                getchar();
                printf(TXT_WHITE"How to play\n");
                printf(TXT_NORMAL"The objective of this game is to eliminate one type of symbol from the board.\n\nTo input directions:\nw: up\ns: down\na: left\nd: right\n");
                getchar();
                break;
            case '3':   //Configuracion
                unsigned input;
                configuracion:
                system("clear");
                printf(TXT_WHITE "Settings\n\n1: Width/Height\t" TXT_NORMAL"%u\n" TXT_WHITE"2: Symbols\t" TXT_NORMAL"%u\n" TXT_WHITE"3: Use standard settings\n0: Go to main menu\n",widthSel,nsymbSel);
                showMessage();
                scanf(" %c",&sel);
                switch(sel){

                    case '1':   //Modify width/height
                        ingresaAncho:
                        system("clear");
                        printf(TXT_WHITE"Set new width\n(Min: %u, Max: %u)\n",WIDTH_MIN,WIDTH_MAX);
                        printf("%s",message);
                        message[0] = 0;
                        readU(&input);
                        if(input < WIDTH_MIN || input > WIDTH_MAX){
                            sendMessage(TXT_NORMAL"\nPlease set a valid width.\n");
                            goto ingresaAncho;
                        }
                        widthSel = input;
                        goto configuracion;


                    case '2':   //Modify symbol number
                        ingresaNsymb:
                        system("clear");
                        printf(TXT_WHITE"Set new symbol number\n(Min: %u, Max: %u)\n",NSYMB_MIN,NSYMB_MAX);
                        printf("%s",message);
                        message[0] = 0;
                        readU(&input);
                        if(input < NSYMB_MIN || input > NSYMB_MAX){
                            sendMessage(TXT_NORMAL"\nPlease set a valid symbol number.\n");
                            goto ingresaNsymb;
                        }
                        nsymbSel = input;
                        goto configuracion;

                    case '3':   //Opciones por defecto
                        widthSel = WIDTH_DEFAULT;
                        nsymbSel = NSYMB_DEFAULT;
                        sendMessage(TXT_NORMAL"\nSettings have been set to their standard values.\n");
                        goto configuracion;

                    case '0':
                        break;

                    default:
                        sendMessage(TXT_NORMAL"\nInvalid selection.\n");
                        goto configuracion;
                }
                break;

            case '0':   //Salir
                system("clear");
                printf(TXT_WHITE"Are you sure you want to exit?\n\n1: Yes\n0: No\n");
                scanf(" %c",&sel);
                switch(sel){
                    case '1':
                        printf(TXT_NORMAL);
                        system("clear");
                        return 0;
                    default:
                        break;
                }
                break;
            default:
            sendMessage(TXT_NORMAL"\nInvalid selection.\n");
            break;
        }

    }
}

void playASCIIcrush(unsigned widthSel, unsigned nsymbSel){
    //Function where we play the game.

    ASCIIboard board(widthSel,nsymbSel);
    ::points = 0;
    ::moves = 0;

    int winningSymbol;
    do{
        //Print board and player controls
        board.print();
        printf(TXT_WHITE"\nm: mover\n");
        if(points >= ROTATION_COST){
            printf("e: rotate clockwise (Cost: %u)\n",ROTATION_COST);
            printf("q: rotate counterclockwise (Cost: %u)\n",ROTATION_COST);
        }
        if(points >= BOMB_COST) printf("b: use bomb (Cost: %u)\n",BOMB_COST);
        showMessage();
        scanf(" %c",&sel);

        //Interpret player command
        switch(sel){
            case 'm':
            case 'M':
                board.move();
                break;
            case 'e':
            case 'E':
                board.rotateR();
                break;
            case 'q':
            case 'Q':
                board.rotateL();
                break;
            case 'b':
            case 'B':
                board.bomb();
                break;
            case '0':
                system("clear");
                printf(TXT_WHITE"Are you sure you want to exit?\n1:Yes\n0:No\n");
                printf(TXT_NORMAL);
                scanf(" %c", &sel);
                switch(sel){
                    case '1':
                        return;
                }
            default:
                sendMessage(TXT_NORMAL"\nInvalid command.\n");
                break;
        }

        //check for crush, then slide and refill the board until we reach an equilibrium configuration.
        //the more times we crush from a single move, the more the multiplier increases, multiplying the potential score from the move.
        ::multiplier = 1;
        while(board.crush(true)){
            board.slide();
            board.refill();
            board.print();
            getchar();
            multiplier++;
        }

    }while(!(winningSymbol = board.checkWin()));

    printf(TXT_SPECIAL TXT_WHITE"You've won by eliminating ");
    board.printSymb(winningSymbol);
    printf(TXT_WHITE" in %u movements!\n",moves);
    printf(TXT_NORMAL);
    getchar();
}

void sendMessage(const char *send){
    int i=0;
    do{
        ::message[i] = send[i];
        i++;
    }while(send[i-1] != 0);
}

void showMessage(void){
    printf("%s",message);
    message[0] = 0;
}

void readU(unsigned *u){
    printf(TXT_NORMAL);
    char next;
    while(true){
        if(scanf("%u",u) == 1) break;
        else{
            do next = getchar();
            while(next != EOF && next != '\n');
            clearerr(stdin);
        }
    }
}

void ASCIIboard::print(void){
    //Function to print the board.
    system("clear");
    printf(TXT_NORMAL" ");
    for(unsigned x=0; x<board_width; x++) printf(" %u",x);
    for(unsigned y=board_width-1; y<board_width; y--){
        printf(TXT_NORMAL"\n%u",y);
        for(unsigned x=0; x<board_width; x++){
            printf(" ");
            printSymb(cell[x][y]);
        }
        printf(TXT_NORMAL" %u",y);
    }
    printf("\n" TXT_NORMAL" ");
    for(unsigned x=0; x<board_width; x++) printf(" %u",x);
    printf("\n");
    printf(TXT_NORMAL"[Pts: %u]  [Mov: %u]\n",points,moves);
}

void ASCIIboard::printSymb(unsigned symb){
    //Function to print symbols
    if(symb > nsymb){
        printf(TXT_NORMAL"?");
        return;
    }
    switch(symb){
        case 0:
            break;
        case 1:
            printf(TXT_CYAN);
            break;
        case 2:
            printf(TXT_YELLOW);
            break;
        case 3:
            printf(TXT_MAGENTA);
            break;
        case 4:
            printf(TXT_RED);
            break;
        case 5:
            printf(TXT_GREEN);
            break;
        case 6:
            printf(TXT_BLUE);
            break;
        case 7:
            printf(TXT_GRAY);
            break;
        default:
            printf(TXT_WHITE);
            break;
    }
    printf("%c",symbols[symb]);
}
void ASCIIboard::move(void){
    //Function to ask the player for coordinates and direction to make a movement, and then do that movement on the board.
    system("clear");
    print();
    printf("\n");

    unsigned xSel, ySel, dirSel;

    do{
        printf(TXT_NORMAL"x = ");
        readU(&xSel);
    }while(xSel < 0 || xSel >= board_width);
    do{
    printf("y = ");
    readU(&ySel);
    }while(ySel < 0 || ySel >= board_width);
    do{
        printf("dir = ");
        readDir(&dirSel);
    }while(dirSel >=  4);


    unsigned aux = cell[xSel][ySel];
    switch(dirSel){
        case RIGHT:
            cell[xSel][ySel] = cell[xSel+1][ySel];
            cell[xSel+1][ySel] = aux;
            break;
        case UP:
            cell[xSel][ySel] = cell[xSel][ySel+1];
            cell[xSel][ySel+1] = aux;
            break;
        case LEFT:
            cell[xSel][ySel] = cell[xSel-1][ySel];
            cell[xSel-1][ySel] = aux;
            break;
        case DOWN:
            cell[xSel][ySel] = cell[xSel][ySel-1];
            cell[xSel][ySel-1] = aux;
            break;
    }

    if((xSel == 0 && dirSel == LEFT) || (xSel == board_width-1 && dirSel == RIGHT) || (ySel == 0 && dirSel == DOWN) || (ySel == board_width-1 && dirSel == UP) || !crush(false)){
        sendMessage(TXT_NORMAL"\nInvalid movement.\n");
        aux = cell[xSel][ySel];
        switch(dirSel){
            case RIGHT:
                cell[xSel][ySel] = cell[xSel+1][ySel];
                cell[xSel+1][ySel] = aux;
                break;
            case UP:
                cell[xSel][ySel] = cell[xSel][ySel+1];
                cell[xSel][ySel+1] = aux;
                break;
            case LEFT:
                cell[xSel][ySel] = cell[xSel-1][ySel];
                cell[xSel-1][ySel] = aux;
                break;
            case DOWN:
                cell[xSel][ySel] = cell[xSel][ySel-1];
                cell[xSel][ySel-1] = aux;
                break;
        }
    }else{
        moves++;
        getchar();
        print();
        getchar();
    }
}
void ASCIIboard::rotateR(void){  //clockwise

    if(points < ROTATION_COST){
        sendMessage(TXT_NORMAL"\nYou don't have enough points to rotate.\n");
        return;
    }

    points -= ROTATION_COST;
    moves++;

    unsigned **copyCell;
    copyCell = new unsigned* [board_width];
    for(unsigned x=0; x<board_width; x++) copyCell[x] = new unsigned [board_width];
    for(unsigned x=0; x<board_width; x++) for(unsigned y=0; y<board_width; y++) copyCell[x][y] = cell[x][y];

    for(unsigned x=0; x<board_width; x++) for(unsigned y=0; y<board_width; y++) cell[x][y] = copyCell[board_width-1-y][x];

    for(unsigned x=0; x<board_width; x++) delete[] copyCell[x];
    delete[] copyCell;

}
void ASCIIboard::rotateL(){  //counterclockwise
    if(points < ROTATION_COST){
        sendMessage(TXT_NORMAL"\nYou don't have enough points to rotate.\n");
        return;
    }

    points -= ROTATION_COST;
    moves++;

    unsigned **copyCell;
    copyCell = new unsigned* [board_width];
    for(unsigned x=0; x<board_width; x++) copyCell[x] = new unsigned [board_width];
    for(unsigned x=0; x<board_width; x++) for(unsigned y=0; y<board_width; y++) copyCell[x][y] = cell[x][y];

    for(unsigned x=0; x<board_width; x++) for(unsigned y=0; y<board_width; y++) cell[x][y] = copyCell[y][board_width-1-x];

    for(unsigned x=0; x<board_width; x++) delete[] copyCell[x];
    delete[] copyCell;

}
void ASCIIboard::bomb(){
    if(points < BOMB_COST){
        sendMessage(TXT_NORMAL"\nYou don't have enough points to use a bomb.\n");
        return;
    }

    unsigned xSel, ySel;
    print();
    printf(TXT_NORMAL"\n");
    do{
        printf("x = ");
        readU(&xSel);
    }while(xSel >= board_width);

    do{
        printf("y = ");
        readU(&ySel);
    }while(ySel >= board_width);
    getchar();

    points -= BOMB_COST;
    moves++;
    cell[xSel][ySel] = 0;
    print();
    getchar();
    slide();
    refill();
    print();
    getchar();
}
bool ASCIIboard::crush(bool addPoints){
    bool **removeBoard;
    bool result = false;
    unsigned addedPoints = 0;
    removeBoard = new bool* [board_width];
    for(unsigned x=0; x<board_width; x++) removeBoard[x] = new bool [board_width];
    for(unsigned x=0; x<board_width; x++) for(unsigned y=0; y<board_width; y++) removeBoard[x][y] = false;

    unsigned counter = 0, currentColor, previousColor;

    //horizontal
    for(unsigned y=0; y<board_width; y++){
        previousColor = 0;
        for(unsigned x=0; x<board_width; x++){
            if((currentColor = cell[x][y]) != 0 && currentColor == previousColor) counter++;
            else{
                if(counter >= 3){
                    if(!addPoints) return true;
                    else{
                        result = true;
                        for(unsigned k = x-counter; k<x; k++) removeBoard[k][y] = true;
                        addedPoints += counter*counter;
                    }
                }
                previousColor = currentColor;
                counter = 1;
            }
        }
        if(counter >= 3){
            if(!addPoints) return true;
            else{
                result = true;
                for(unsigned k = board_width-counter; k<board_width; k++) removeBoard[k][y] = true;
                addedPoints += counter*counter;
                counter = 1;
            }
        }


    }

    //vertical
    for(unsigned x=0; x<board_width; x++){
        previousColor = 0;
        for(unsigned y=0; y<board_width; y++){
            if((currentColor = cell[x][y]) != 0 && currentColor == previousColor) counter++;
            else{
                if(counter >= 3){
                    if(!addPoints) return true;
                    else{
                        result = true;
                        for(unsigned k = y-counter; k<y; k++) removeBoard[x][k] = true;
                        addedPoints += counter*counter;
                    }
                }
                previousColor = currentColor;
                counter = 1;
            }
        }
        if(counter >= 3){
            if(!addPoints) return true;
            else{
                result = true;
                for(unsigned k = board_width-counter; k<board_width; k++) removeBoard[x][k] = true;
                addedPoints += counter*counter;
                counter = 1;
            }
        }
    }

    //remove
    for(unsigned x=0; x<board_width; x++) for(unsigned y=0; y<board_width; y++) if(removeBoard[x][y]) cell[x][y] = 0;
    for(unsigned x=0; x<board_width; x++) delete[] removeBoard[x];
    delete[] removeBoard;

    if(!addPoints) return false;

    if(addedPoints){
        points += multiplier*addedPoints;
        print();
        printf("\n");
        printf(TXT_WHITE);
        if(multiplier == 1) printf("+%u",addedPoints);
        else{
            printf("+%u*%u",multiplier,addedPoints);
            for(unsigned i=0; i<multiplier-1; i++) printf("!");
            printf("  (%u)",multiplier*addedPoints);
        }
        printf("\n" TXT_NORMAL);
        getchar();
    }

    return result;
}
void ASCIIboard::slide(){
    bool finished = false;
    unsigned aux;

    while(!finished){
        finished = true;
        for(unsigned x=0; x<board_width; x++){
            for(unsigned y=board_width-2; y<board_width; y--){
                if(cell[x][y] == 0 && cell[x][y+1] != 0){
                    aux = cell[x][y];
                    cell[x][y] = cell[x][y+1];
                    cell[x][y+1] = aux;
                    finished = false;
                }
            }
        }
    }

    print();
    getchar();
}
void ASCIIboard::refill(){
    for(unsigned x=0; x<board_width; x++) for(unsigned y=0; y<board_width; y++) if(cell[x][y] == 0) cell[x][y] = rand()%nsymb + 1;
}
unsigned ASCIIboard::checkWin(){
    bool *present = new bool [nsymb];
    for(unsigned i=0; i<nsymb; i++) present[i] = false;
    for(unsigned x=0; x<board_width; x++) for(unsigned y=0; y<board_width; y++) present[cell[x][y]-1] = true;
    for(unsigned i=0; i<nsymb; i++) if(!present[i]){
        delete[] present;
        return i+1;
    }
    delete[] present;
    return 0;
}


void readDir(unsigned *u){
    char c;
    scanf(" %c",&c);
    switch(c){
        case 'd':
        case 'D':
            *u = RIGHT;
            break;
        case 'w':
        case 'W':
            *u = UP;
            break;
        case 'a':
        case 'A':
            *u = LEFT;
            break;
        case 's':
        case 'S':
            *u = DOWN;
            break;
        default:
            *u = 4;
            break;
    }

}
