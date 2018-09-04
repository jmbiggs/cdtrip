//
//  main.cpp
//  cdtrip
//
//  Created by Michael Biggs on 8/25/18.
//

#include <iostream>
#include <ncurses.h>

bool IsTerminalAvailable = false; // Check this global variable before ncurses calls
static const int BorderWidth = 6;
static const int TickRate = 200;
static const int TripTickRate = 10;

int counter = 0;
char input1 = NULL;
char input2 = NULL;

int intensity;

enum Direction { down, right, up, left };
//struct coord { int x; int y; };

enum Content {
    blank = 0,
    spiral = 1,
    spiral2 = 2
};
int * board = NULL;

bool drawSpiral = false;
int spiralX;
int spiralY;
bool flipSpiral = false;
Direction spiralDir;

int backgroundColor = 0;

/// TRIP

int boardIndex(int x, int y) {
    return x + (y * COLS);
}

char charForContent(int content) {
    switch (content) {
        case spiral: {
            return '*';
        }
            
        case spiral2: {
            return '@';
        }
            
        case blank:
        default: {
            return ' ';
        }
    }
}

bool charCollides(int x, int y, char colChar) {
    if ((y < 0) || (y >= LINES) || (x < 0) || (x >= COLS)) {
        return true;
    }
    
    move(y, x);
    char cAtPos = inch() & A_CHARTEXT;
    return cAtPos == colChar;
}

void update() {
    if (drawSpiral) {
        char spiralChar = flipSpiral ? '@' : '*';
        
        if (charCollides(spiralX, spiralY - 1, spiralChar) &&
            charCollides(spiralX, spiralY + 1, spiralChar) &&
            charCollides(spiralX - 1, spiralY, spiralChar) &&
            charCollides(spiralX + 1, spiralY, spiralChar)) {
//            drawSpiral = false;
            flipSpiral = !flipSpiral;
            return;
        }

        switch (spiralDir) {
            case down: {
                if (charCollides(spiralX, spiralY + 1, spiralChar)) {
                    spiralDir = right;
                    update();
                    return;
                } else {
                    spiralY++;
                }
                break;
            }
            case right: {
                if (charCollides(spiralX + 1, spiralY, spiralChar)) {
                    spiralDir = up;
                    update();
                    return;
                } else {
                    spiralX++;
                }
                break;
            }
            case up: {
                if (charCollides(spiralX, spiralY - 1, spiralChar)) {
                    spiralDir = left;
                    update();
                    return;
                } else {
                    spiralY--;
                }
                break;
            }
            case left: {
                if (charCollides(spiralX - 1, spiralY, spiralChar)) {
                    spiralDir = down;
                    update();
                    return;
                } else {
                    spiralX--;
                }
                break;
            }
            default: {
                break;
            }
        }
        
        int idx = boardIndex(spiralX, spiralY);
        board[idx] = flipSpiral ? spiral2 : spiral;
    }
}

void colorBackground() {
    backgroundColor++;
    if (backgroundColor > 5) {
        backgroundColor = 0;
    }
    
    attron(COLOR_PAIR(backgroundColor));
}

void setupColor() {
    start_color();
    init_pair(0, COLOR_WHITE, COLOR_GREEN);
    init_pair(1, COLOR_WHITE, COLOR_CYAN);
    init_pair(2, COLOR_WHITE, COLOR_RED);
    init_pair(3, COLOR_WHITE, COLOR_BLUE);
    init_pair(4, COLOR_WHITE, COLOR_YELLOW);
    init_pair(5, COLOR_WHITE, COLOR_MAGENTA);
}

void drawScreen() {
    for (int i = 0; i < COLS; i++) {
        for (int j = 0; j < LINES; j++) {
            int idx = boardIndex(i, j);
            int content = board[idx];
            char cToDraw = charForContent(content);
            mvaddch(j, i, cToDraw);
        }
    }
    refresh();
}

void startSpiral() {
    drawSpiral = true;
    spiralX = 0;
    spiralY = -1;
    spiralDir = down;
}

void setupBoard() {
    board = (int*)malloc(sizeof(int) * LINES * COLS);
    for (int i = 0; i < COLS; i++) {
        for (int j = 0; j < LINES; j++) {
            int idx = boardIndex(i, j);
            board[idx] = 0;
        }
    }
}

void runTrip() {
    timeout(TripTickRate);
    curs_set(0); // turn off cursor
    counter = 0;
    
    setupBoard();
    startSpiral();
    setupColor();
    drawScreen();
    
    int ch;
    while ((ch = getch()) != 'q') {
        colorBackground();
        update();
        drawScreen();
    }
    endwin();
}

/// MENU

char charForCoord(int x, int y) {
    char c;
    switch (counter) {
        case 0:
            c = '|';
            break;
        case 1:
            c = '/';
            break;
        case 2:
            c = '-';
            break;
        case 3:
            c = '\\';
            break;
        default:
            c = ' ';
            break;
    }
    
    if ((y < BorderWidth) || (y >= LINES - BorderWidth)) {
        // cover top 2 and bottom 2
        return c;
    } else if ((x < BorderWidth) || (x >= COLS - BorderWidth)) {
        // just print the edges
        return c;
    } else {
        return ' ';
    }
}

void updateBorder() {
    counter++;
    if (counter > 3) {
        counter = 0;
    }
    
    for (int i = 0; i < COLS; i++) {
        for (int j = 0; j < LINES; j++) {
            mvaddch(j, i, charForCoord(i, j));
        }
    }
}

void printTitles() {
    char welcome[] = "Welcome to CD-TRIP"; // 18
    char quit[] = "Press 'q' to quit"; // 18
    char how[] = "How strong do you want your trip to be today?"; // 45
    char prompt[] = "Enter a number 1-10 (1=weakest, 10=strongest):";
    
    int wy = BorderWidth + 2;
    int wx = COLS / 2 - 9;
    
    int hy = wy + 2;
    int hx = COLS / 2 - 23;
    
    int py = hy + 4;
    int px = BorderWidth + 2;
    
    int qy = LINES - BorderWidth - 2;
    int qx = BorderWidth + 2;

    attron(A_BOLD);
    mvaddstr(wy, wx, welcome);
    attroff(A_BOLD);
    
    mvaddstr(qy, qx, quit);
    mvaddstr(hy, hx, how);
    mvaddstr(py, px, prompt);
    if (input1) {
        addch(input1);
    }
    if (input2) {
        addch(input2);
    }
}

void execute() {
    // parse intensity from input
    if (input2) {
        intensity = 10;
    } else {
        intensity = (int)input1;
    }
    
    runTrip();
}

void runMenu() {
    int ch;
    while ((ch = getch()) != 'q') {
        updateBorder();
        printTitles();
        
        // handle number input
        if ((ch >= '0') && (ch <= '9') && !input2) {
            addch(ch);
            if (!input1) {
                input1 = ch;
            } else {
                input2 = ch;
            }
        }
        
        // handle delete/backspace
        if (ch == 8 || ch == 127) {
            if (input1 || input2) {
                int cury;
                int curx;
                getyx(stdscr, cury, curx);
                curx--;
                move(cury, curx);
                addch(' ');
                move(cury, curx);
            }
            
            if (input2) {
                input2 = NULL;
            } else if (input1) {
                input1 = NULL;
            }
        }
        
        // handle execute
        if (ch == '\n' && input1) {
            execute();
            return;
        }
        
        refresh();
    }
    
    endwin();
}

int main(int argc, char *argv[]) {
    
    for (int argi = 1; argi < argc; argi++)
    {
        if (strcmp(argv[argi], "--debug-in-terminal") == 0)
        {
            printf("Debugging in terminal enabled\n");
            getchar(); // Without this call debugging will be skipped
            break;
        }
    }
    
    initscr();
    cbreak();
    timeout(TickRate);
    noecho();
    
    runMenu();
}

