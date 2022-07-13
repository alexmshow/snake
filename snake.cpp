#include <chrono>
#include <thread>
#include <Windows.h>
#include <vector>
#include <iostream>
#include <time.h>  

#pragma comment(lib, "user32.lib")

#define START_SNAKE_POS vec2(25, 5)
#define START_SNAKE_LEN 5

int MAXFOOD = 0;

class vec2
{
    public: 
        short x = 0;
        short y = 0;
        vec2(short a, short b) {
            x = a; y = b;
        }
        bool operator==(vec2 const &obj) {
            return this->x == obj.x && this->y == obj.y;
        }
        vec2 operator+(vec2 const &obj) {
            return vec2(this->x + obj.x, this->y + obj.y);
        }
        vec2 operator-(vec2 const &obj) {
            return vec2(this->x - obj.x, this->y - obj.y);
        }
        vec2 operator+(int num) {
            return vec2(this->x + num, this->y + num);
        }
        vec2 operator-(int num) {
            return vec2(this->x - num, this->y - num);
        }
};

void moveSnake(vec2 Move);
void createFood(COORD screenSize);
void createSnakePart(COORD screenSize);
bool checkForCollision();

std::vector<vec2> cells;
std::vector<vec2> food;

const unsigned FPS = 15;
std::vector<char> frameData;
short score = 0;

// Get the intial console buffer.
auto firstBuffer = GetStdHandle(STD_OUTPUT_HANDLE);

// Create an additional buffer for switching.
auto secondBuffer = CreateConsoleScreenBuffer(
    GENERIC_READ | GENERIC_WRITE,
    FILE_SHARE_WRITE | FILE_SHARE_READ,
    nullptr,
    CONSOLE_TEXTMODE_BUFFER,
    nullptr);

// Assign switchable back buffer.
HANDLE backBuffer = secondBuffer;
bool bufferSwitch = true;

// Returns current window size in rows and columns.
COORD getScreenSize()
{
    CONSOLE_SCREEN_BUFFER_INFO bufferInfo;
    GetConsoleScreenBufferInfo(firstBuffer, &bufferInfo);
    const auto newScreenWidth = bufferInfo.srWindow.Right - bufferInfo.srWindow.Left + 1;
    const auto newscreenHeight = bufferInfo.srWindow.Bottom - bufferInfo.srWindow.Top + 1;

    return COORD{ static_cast<short>(newScreenWidth), static_cast<short>(newscreenHeight) };
}

// Switches back buffer as active.
void swapBuffers()
{
    WriteConsole(backBuffer, &frameData.front(), static_cast<short>(frameData.size()), nullptr, nullptr);
    SetConsoleActiveScreenBuffer(backBuffer);
    backBuffer = bufferSwitch ? firstBuffer : secondBuffer;
    bufferSwitch = !bufferSwitch;
    std::this_thread::sleep_for(std::chrono::milliseconds(1000 / FPS));
}

// Draw horizontal line moving from top to bottom.
void drawFrame(COORD screenSize)
{
    for (auto i = 0; i < screenSize.Y; i++)
    {
        for (auto j = 0; j < screenSize.X; j++)
        {
            if (frameData[i * screenSize.X + j] != '#') {
                frameData[i * screenSize.X + j] = ' ';
            }
            for (auto I = 0; I < food.size();I++)
            {
                if (food[I].x == j && food[I].y == i)
                    frameData[i * screenSize.X + j] = '@';
            }
            for (auto I = 0; I < cells.size();I++)
            {
                if (cells[I].x == j && cells[I].y == i)
                    frameData[i * screenSize.X + j] = '#';
            }
        }
    }
}

void reset(COORD screenSize) {
    for (auto i = 0; i < screenSize.Y*screenSize.X; i++) {
        frameData[i] = ' ';
    }
}

void logic(COORD screenSize) {
    static short left = 0;
    static short top = 0;
    static short nfood = 0;

    if (GetAsyncKeyState(VK_UP) && top != -1) {top = 1; left = 0;}
    else if (GetAsyncKeyState(VK_DOWN) && top != 1) {top = -1; left = 0;}
    else if (GetAsyncKeyState(VK_LEFT) && left != -1) {top = 0; left = 1;}
    else if (GetAsyncKeyState(VK_RIGHT) && left != 1) {top = 0; left = -1;}

    moveSnake(vec2(left, top));

    if (nfood < MAXFOOD) {
        while (nfood < MAXFOOD) {
            createFood(screenSize);
            nfood++;
        }
    }

    for (auto i = 0; i < cells.size(); i++)
    {
        for (auto j = 0; j < food.size(); j++)
        {
            if (cells.at(i) == food.at(j)) {
                food.erase(food.begin() + j);
                createSnakePart(screenSize);
                nfood--;
                score++;
            }
        }
    }

    if (checkForCollision()){
        std::cout << "Score: " << score << std::endl;
        system("pause");
        exit(0);
    }
        

    for (auto i = 0; i < cells.size(); i++)
    {
        if (cells.at(i).x > screenSize.X - 1)
            cells.at(i).x -= screenSize.X;
        if (cells.at(i).x < 0)
            cells.at(i).x += screenSize.X;
        if (cells.at(i).y >= screenSize.Y)
            cells.at(i).y -= screenSize.Y-1;
        if (cells.at(i).y < 1)
            cells.at(i).y += screenSize.Y-1;
    }
    
}

void createSnake(vec2 StartPos, int n) {
    for (auto i = 0; i < n; i++)
    {
        cells.push_back(vec2(StartPos.x + i, StartPos.y));
    }
}

void moveSnake(vec2 Move) {
    short top = Move.y;
    short left = Move.x;
    if (top == 1 && left == 0) {

        for (int i = cells.size() - 1; i > 0; i--)
        {
            cells.at(i).x = cells.at(i-1).x;
            cells.at(i).y = cells.at(i-1).y;
        }
        cells.at(0).y -= 1;
    }
    else if (top == -1 && left == 0) {
        for (int i = cells.size() - 1; i > 0; i--)
        {
            cells.at(i).x = cells.at(i-1).x;
            cells.at(i).y = cells.at(i-1).y;
        }
        cells.at(0).y += 1;
    }
    else if (top == 0 && left == 1) {
        for (int i = cells.size() - 1; i > 0; i--)
        {
            cells.at(i).x = cells.at(i-1).x;
            cells.at(i).y = cells.at(i-1).y;
        }
        cells.at(0).x -= 1;
    }
    else if (top == 0 && left == -1) {
        for (int i = cells.size() - 1; i > 0; i--)
        {
            cells.at(i).x = cells.at(i-1).x;
            cells.at(i).y = cells.at(i-1).y;
        }
        cells.at(0).x += 1;
    }
}

void createFood(COORD screenSize) {
    food.push_back(vec2(1+rand()%(screenSize.X-1), 1+rand()%(screenSize.Y-1)));
}

void createSnakePart(COORD screenSize) {
    vec2 direction = (cells.back() - cells.at(cells.size()-2));
    cells.push_back(vec2(cells.back().x + direction.x, cells.back().y + direction.y));
}

bool checkForCollision() {
    for (auto i = 0; i < cells.size(); i++)
    {
        for (auto j = 0; j < cells.size(); j++)
        {
            if (cells.at(i) == cells.at(j) && i != j) {
                return true;
            } 
        }
    }
    return false;
}

void gameloop(COORD screenSize) {
    while (true)
    {
        drawFrame(screenSize);
        logic(screenSize);
        swapBuffers();
        reset(screenSize);
    }
}

int main(int argc, char** argv)
{
    if (argc == 1) {MAXFOOD = 3;}
    else {MAXFOOD = atoi(argv[1]);}

    // init PRNG
    srand((unsigned)time(NULL));

    const auto screenSize = getScreenSize();
    SetConsoleScreenBufferSize(firstBuffer, screenSize);
    SetConsoleScreenBufferSize(secondBuffer, screenSize);
    frameData.resize(screenSize.X * screenSize.Y);

    createSnake(START_SNAKE_POS, START_SNAKE_LEN);
    gameloop(screenSize);
}
