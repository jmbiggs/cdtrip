//
//  main.cpp
//  cdtrip
//
//  Created by Michael Biggs on 8/25/18.
//

#include <iostream>
#include <cstring>
#include <cstdlib>
#include <ncurses.h>

bool IsTerminalAvailable = false; // Check this global variable before ncurses calls
static const int BorderWidth = 6;
static const int TickRate = 200;
static const int TripTickRate = 10;

int counter = 0;
char input1 = NULL;
char input2 = NULL;

int intensity;

enum Direction { Dirdown, Dirright, Dirup, Dirleft };
enum Content {
    blank = 0,
    spiral = 1,
    spiral2 = 2,
    ring = 3,
};
int * board = NULL;
int * boardWithRings = NULL;

bool drawSpiral = false;
int spiralX;
int spiralY;
bool flipSpiral = false;
Direction spiralDir;

struct Ring { int x; int y; int dist; bool activated; };
const int ringWidth = 3;
const int ringHeight = 2;
const int ringCount = 10;
struct Ring rings[ringCount];
int ringCounter = 0;
const int ringSlowdown = 2;
const int ringSpacing = 5;
int lastRingAdded = -1;
int nextRingCounter = 0;

int backgroundColor = 1;
int colorCounter = 0;
const int colorSlowdown = 30;
int colorFlashCounter = 0; // 0,1 ring shows for 1/2 slowdown, 2,3 ring shows for full slowdown
const int colorFlashMax = 4;

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
            
        case ring: {
//            return '0';
        }
            
        case blank: {
//            return '0' + backgroundColor;
        }
            
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

bool inRing(Ring * r, int x, int y) {
    int ringLeftXMax = r->x - r->dist;
    int ringLeftXMin = ringLeftXMax - ringWidth;
    int ringRightXMin = r->x + r->dist;
    int ringRightXMax = ringRightXMin + ringWidth;
    
    bool inXStrict = (x > ringLeftXMin && x < ringLeftXMax) || (x > ringRightXMin && x < ringRightXMax);
    bool inXLoose = (x > ringLeftXMin && x < ringRightXMax);
    
    int ringTopYMax = r->y - r->dist;
    int ringTopYMin = ringTopYMax - ringHeight;
    int ringBottomYMin = r->y + r->dist;
    int ringBottomYMax = ringBottomYMin + ringHeight;

    bool inYStrict = (y > ringTopYMin && y < ringTopYMax) || (y > ringBottomYMin && y < ringBottomYMax);
    bool inYLoose = (y > ringTopYMin && y < ringBottomYMax);
    
    return (inXLoose && inYStrict) || (inXStrict && inYLoose);
}

void addRing() {
    lastRingAdded++;
    if (lastRingAdded >= ringCount) {
        lastRingAdded = 0;
    }
    
    Ring * r = &rings[lastRingAdded];
    r->x = COLS / 2;
    r->y = LINES / 2;
    r->dist = 0;
    r->activated = true;
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
            case Dirdown: {
                if (charCollides(spiralX, spiralY + 1, spiralChar)) {
                    spiralDir = Dirright;
                    update();
                    return;
                } else {
                    spiralY++;
                }
                break;
            }
            case Dirright: {
                if (charCollides(spiralX + 1, spiralY, spiralChar)) {
                    spiralDir = Dirup;
                    update();
                    return;
                } else {
                    spiralX++;
                }
                break;
            }
            case Dirup: {
                if (charCollides(spiralX, spiralY - 1, spiralChar)) {
                    spiralDir = Dirleft;
                    update();
                    return;
                } else {
                    spiralY--;
                }
                break;
            }
            case Dirleft: {
                if (charCollides(spiralX - 1, spiralY, spiralChar)) {
                    spiralDir = Dirdown;
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
    
    memcpy(boardWithRings, board, sizeof(int) * LINES * COLS);

    ringCounter++;
    if (ringCounter > ringSlowdown) {
        ringCounter = 0;
    
        nextRingCounter++;
        if (nextRingCounter > ringSpacing) {
            addRing();
            nextRingCounter = 0;
        }
        
        for (int i = 0; i < ringCount; i++) {
            Ring * r = &rings[i];
            if (r->activated) {
                r->dist++;
            }
        }
    }
    
    for (int i = 0; i < ringCount; i++) {
        Ring * r = &rings[i];
        if (r->activated) {
            for (int i = 0; i < COLS; i++) {
                for (int j = 0; j < LINES; j++) {
                    bool in = inRing(r, i, j);
                    int idx = boardIndex(i, j);
                    
                    if (in) {
                        boardWithRings[idx] = ring;
                    }
                }
            }
        }
    }
}

void updateBackgroundColor() {
    colorCounter++;
    int slowdown = colorFlashCounter < colorFlashMax / 2 ? colorSlowdown / 2 : colorSlowdown;
    if (colorCounter < slowdown) {
        return;
    }
    
    colorCounter = 0;
    
    colorFlashCounter++;
    if (colorFlashCounter >= colorFlashMax) {
        colorFlashCounter = 0;
    }
    
    backgroundColor++;
    if (backgroundColor > 5) {
        backgroundColor = 1;
    }
}

void drawScreen() {
    for (int i = 0; i < COLS; i++) {
        for (int j = 0; j < LINES; j++) {
            int idx = boardIndex(i, j);
            int content = boardWithRings[idx];
            char cToDraw = charForContent(content);
            
            if (content == ring) {
                attron(COLOR_PAIR(6));
            } else {
                attron(COLOR_PAIR(backgroundColor));
            }
            
            mvaddch(j, i, cToDraw);
        }
    }
    refresh();
}

void startSpiral() {
    drawSpiral = true;
    spiralX = 0;
    spiralY = -1;
    spiralDir = Dirdown;
}

void setupBoard() {
    board = (int*)malloc(sizeof(int) * LINES * COLS);
    for (int i = 0; i < COLS; i++) {
        for (int j = 0; j < LINES; j++) {
            int idx = boardIndex(i, j);
            board[idx] = 0;
        }
    }
    boardWithRings = (int*)malloc(sizeof(int) * LINES * COLS);    
    memcpy(boardWithRings, board, sizeof(int) * LINES * COLS);
}

void setupColor() {
    start_color();
    init_pair(1, COLOR_BLACK, COLOR_CYAN);
    init_pair(2, COLOR_WHITE, COLOR_RED);
    init_pair(3, COLOR_WHITE, COLOR_BLUE);
    init_pair(4, COLOR_BLACK, COLOR_GREEN);
    init_pair(5, COLOR_WHITE, COLOR_MAGENTA);
    init_pair(6, COLOR_BLACK, COLOR_YELLOW);
}

void runTrip() {
    timeout(TripTickRate);
    curs_set(0); // turn off cursor
    counter = 0;
    
    setupBoard();
//    startSpiral();
    setupColor();
    drawScreen();
    
    int ch;
    while ((ch = getch()) != 'q') {
        updateBackgroundColor();
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

