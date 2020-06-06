#include<iostream>
#include<windows.h>
#include<time.h>
#include<stdio.h>
#include<conio.h>
#include<thread>
#include<io.h>
#include<fcntl.h>
#include<fstream>
#include<string>
#include<vector>
#include<iterator>
#include<algorithm>
#pragma comment(lib, "Winmm.lib") //add this library to play music for game
#pragma warning(disable:4996)
using namespace std;

#define MAX_SIZE_SNAKE 25 //increase this value to avoid bug after level up snake
#define MAX_SIZE_FOOD 4
#define MAX_SPEED 3

struct Tunnel
{
	POINT middle_of_tunnel; //point middle of tunnel for snake go through
	int direction; //direction of tunnel 
} tunnel;

POINT snake[25]; 
POINT food[4];
int CHAR_LOCK;
int MOVING;
int SPEED;
int HEIGH_CONSOLE, WIDTH_CONSOLE;
int FOOD_INDEX;
int SIZE_SNAKE;
int STATE;
char student_id[8] = {'1', '8', '1', '2', '7', '0', '7', '0'};
//cordinate draw tunnel
char BTIME[15] = {205, 218, 196, 191, 179, 195, 180, 194, 193, 217, 192, 220, 223, 219};
int cordinate_y[4][5] = { {-1,0,0,0,-1}, {-1,-1,0,1,1}, {1,0,0,0,1}, { -1,-1,0,1,1 } };
int cordinate_x[4][5] = { {-1,-1,0,1,1}, {1,0,0,0,1}, {-1,-1,0,1,1}, { -1,0,0,0,-1 } };
// obstacles
vector<POINT> obstacles;

//mode through wall
#define NEW_WIDTH_CONSOLE 60
#define NEW_HEIGH_CONSOLE 30
enum keyInput{Up, Down, Left, Right, Exit};

struct SnakeThroughWall
{
	string info_snake;
	int x, y, dx, dy; //cur pos and previous pos
};
//mode through wall

/*splash screen*/
#define ColorCode_Back			0
#define ColorCode_DarkBlue		1
#define ColorCode_DarkGreen		2
#define ColorCode_DarkCyan		3
#define ColorCode_DarkRed		4
#define ColorCode_DarkPink		5
#define ColorCode_DarkYellow	6
#define ColorCode_DarkWhite		7
#define ColorCode_Grey			8
#define ColorCode_Blue			9
#define ColorCode_Green			10
#define ColorCode_Cyan			11
#define ColorCode_Red			12
#define ColorCode_Pink			13
#define ColorCode_Yellow		14
#define ColorCode_White			15
#define default_ColorCode		7

#define key_Up		1072
#define key_Down	1080
#define key_Left	1075
#define key_Right	1077
#define key_none	-1
/*splash screen*/

//remove blinking underscore on console
//https://stackoverflow.com/questions/18028808/remove-blinking-underscore-on-console-cmd-prompt
void ShowConsoleCursor(bool showFlag)
{
	HANDLE out = GetStdHandle(STD_OUTPUT_HANDLE);
	CONSOLE_CURSOR_INFO cursorInfo;
	GetConsoleCursorInfo(out, &cursorInfo);
	cursorInfo.dwSize = 1; //set cursorInfo.dwSize to value between 1 and 100  else SetConsoleCursorInfo will fail
	cursorInfo.bVisible = showFlag;
	SetConsoleCursorInfo(out, &cursorInfo);
}

//fix console screen avoid stretching from user
void FixConsoleWindow()
{
	HWND consoleWindow = GetConsoleWindow();
	LONG style = GetWindowLong(consoleWindow, GWL_STYLE);
	style = style & ~(WS_MAXIMIZEBOX) & ~(WS_THICKFRAME);
	SetWindowLong(consoleWindow, GWL_STYLE, style);
	system("mode 120,43");
}

void GotoXY(int x, int y)
{
	COORD coord;
	coord.X = x;
	coord.Y = y;
	SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
}

//Check the location of coordinates
bool IsValid(int x, int y)
{
	for (int i = 0; i < SIZE_SNAKE; i++) 
	{
		if (snake[i].x == x && snake[i].y == y) return false;
	}
	for (int i = 0; i < obstacles.size(); i++) 
	{
		if (obstacles[i].x == x && obstacles[i].y == y) return false;
	}
	return true;
}

//initialize the position value for the food
void GenerateFood()
{
	int x, y;
	srand(time(NULL));
	for (int i = 0; i < MAX_SIZE_FOOD; i++) 
	{
		do 
		{
			x = rand() % (WIDTH_CONSOLE - 5) + 2; //default: -1 +1
			y = rand() % (HEIGH_CONSOLE - 5) + 2; //default: -1 +1
		} while (!IsValid(x, y));
		food[i] = { x,y };
	}
}

//Tunnel t
bool isValidTunnel(Tunnel tunnel)
{
	for(auto v = 0; v < 5; v++)
	{
		int i = tunnel.middle_of_tunnel.x + cordinate_x[tunnel.direction - 1][v], j = tunnel.middle_of_tunnel.y + cordinate_y[tunnel.direction - 1][v];
		if(!IsValid(i, j)) return false;
	}
	return true;
}

void createTunnel()
{
	srand(time(NULL));
	tunnel.middle_of_tunnel.x = rand() % (WIDTH_CONSOLE - 5) + 2; //default: -3 +2
	tunnel.middle_of_tunnel.y = rand() % (HEIGH_CONSOLE - 5) + 2; //default: -3 +2
	do 
	{
		tunnel.direction = rand() % 4 + 1;
	} while (!isValidTunnel(tunnel));
}

//reset data to original
void ResetData()
{
	CHAR_LOCK = 'A', MOVING = 'D', SPEED = 1, FOOD_INDEX = 0, WIDTH_CONSOLE = 70, HEIGH_CONSOLE = 20, SIZE_SNAKE = 6;
	snake[0] = { 10, 5 }; 
	snake[1] = { 11, 5 }; 
	snake[2] = { 12, 5 }; 
	snake[3] = { 13, 5 };
	snake[4] = { 14, 5 }; 
	snake[5] = { 15, 5 };
	GenerateFood(); 
	createTunnel();
}

//draw obstacles
void SetObstacles(int width, int height) 
{
	for (int i = 8; i <= width - 8; i++) 
	{
		obstacles.push_back(POINT({ i, 4 }));
		obstacles.push_back(POINT({ i, height - 4 }));
	}
	for (int i = 5; i <= 7; i++) 
	{
		obstacles.push_back(POINT({ 8, i }));
		obstacles.push_back(POINT({ width - 8, i }));
	}
	for (int i = height - 7; i <= height - 5; i++) 
	{
		obstacles.push_back(POINT({ 8, i }));
		obstacles.push_back(POINT({ width - 8, i }));
	}
	for (int i = 0; i < 17; i++) 
	{
		obstacles.push_back(POINT({ 15 + i, height / 2 }));
		obstacles.push_back(POINT({ width - 15 - i, height / 2 }));
	}
	for (int i = 1; i < 4; i++) 
	{
		obstacles.push_back(POINT({ 23, height / 2 + i }));
		obstacles.push_back(POINT({ width - 23, height / 2 - i }));
	}
}

// deleteObtacles
void DeleteObstacles () 
{
	obstacles.clear();
}
//draw obstacles

void DrawBoardNotObstacle(int x, int y, int width, int height, int curPosX = 0, int curPosY = 0)
{
	GotoXY(x, y); cout << (char)220;
	for (int i = 1; i < width; i++) cout << (char)220;
	cout << (char)220;
	GotoXY(x, height + y); cout << (char)220;
	for (int i = 1; i < width; i++) cout << (char)220;
	cout << (char)220;
	for (int i = y + 1; i < height + y + 1; i++) 
	{
		GotoXY(x, i); cout << (char)219;
		GotoXY(x + width, i); cout << (char)219;
	}
	GotoXY(curPosX, curPosY);
}

//mode obstacle
void DrawBoardObstacle(int x, int y, int width, int height, int curPosX = 0, int curPosY = 0)
{
	GotoXY(x, y); cout << (char)220;
	for (int i = 1; i < width; i++) cout << (char)220;
	cout << (char)220;
	GotoXY(x, height + y); cout << (char)220;
	for (int i = 1; i < width; i++) cout << (char)220;
	cout << (char)220;
	for (int i = y + 1; i < height + y + 1; i++) 
	{
		GotoXY(x, i); cout << (char)219;
		GotoXY(x + width, i); cout << (char)219;
	}
	GotoXY(curPosX, curPosY);
	//for obstacle
	for (int i = 8; i <= width - 8; i++) 
	{
		GotoXY(i, 4); 
		cout << (char)220;
		GotoXY(i, height - 4); 
		cout << (char)223;
	}
	for (int i = 5; i <= 7; i++) 
	{
		GotoXY(8, i); 
		cout << char(219);
		GotoXY(width - 8, i); 
		cout << char(219);
	}
	for (int i = height - 7; i <= height - 5; i++) 
	{
		GotoXY(8, i); 
		cout << char(219);
		GotoXY(width - 8, i); 
		cout << char(219);
	}
	for (int i = 0; i < 17; i++) 
	{
		GotoXY(15 + i, height / 2); 
		cout << (char)220;
		GotoXY(width - 15 - i, height / 2); 
		cout << (char)223;
	}
	for (int i = 1; i < 4; i++) 
	{
		GotoXY(23, height/2 + i); 
		cout << (char)219;
		GotoXY(width - 23, height / 2 - i); 
		cout << (char)219;
	}
}

void playSoundTheme();
void playSoundGameOver();
void drawInfoGame();
void StartGameObstacle()
{
	system("cls");
	ResetData();
	DrawBoardObstacle(0, 0, WIDTH_CONSOLE, HEIGH_CONSOLE);
	drawInfoGame();
	playSoundTheme();
	STATE = 1;
}
//mode obstacle

void playSoundTheme()
{
	bool isPlay = PlaySound(TEXT("background_music.wav"), NULL, SND_ASYNC);
}

void playSoundGameOver()
{
	bool isPlay = PlaySound(TEXT("gameover_music.wav"), NULL, SND_ASYNC);
}

void drawInfoGame()
{
	HANDLE consoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleTextAttribute(consoleHandle, 12);
	GotoXY(WIDTH_CONSOLE + 1, 0);
	for (int i = 1; i < 120 - WIDTH_CONSOLE; i++) cout << (char)220;
	GotoXY(WIDTH_CONSOLE + 1, HEIGH_CONSOLE);
	for (int i = 1; i < 120 - WIDTH_CONSOLE; i++) cout << (char)220;
	for (int i = 0; i < HEIGH_CONSOLE; i++)
	{
		GotoXY(WIDTH_CONSOLE + 1, i + 1); cout << (char)219;
		GotoXY(119, i + 1); cout << (char)219;
	}
	GotoXY(WIDTH_CONSOLE + 17, 3); 
	cout << "Snake Game";
	GotoXY(WIDTH_CONSOLE + 17, 5); 
	cout << "T (upload game)";
	GotoXY(WIDTH_CONSOLE + 17, 6); 
	cout << "L (save game)";
	GotoXY(WIDTH_CONSOLE + 17, 7); 
	cout << "P (pause game)";
	GotoXY(WIDTH_CONSOLE + 17, 8); 
	cout << "Enter (continue game)";
	GotoXY(WIDTH_CONSOLE + 17, 9); 
	cout << "M (turn off music)";
	GotoXY(WIDTH_CONSOLE + 17, 10); 
	cout << "N (turn on music)";
	GotoXY(WIDTH_CONSOLE + 17, 11); 
	cout << "Esc (exit game)";
	GotoXY(WIDTH_CONSOLE + 23, 13); 
	cout << "w: " << char(30);
	GotoXY(WIDTH_CONSOLE + 17, 14); 
	cout << "a: " << char(17);
	GotoXY(WIDTH_CONSOLE + 28, 14);
	cout << "d: " << char(16);
	GotoXY(WIDTH_CONSOLE + 23, 15);
	cout << "s: " << char(31);
	GotoXY(0, 0);
}

void StartGame()
{
	system("cls");
	ResetData();
	DrawBoardNotObstacle(0, 0, WIDTH_CONSOLE, HEIGH_CONSOLE);
	drawInfoGame();
	playSoundTheme();
	STATE = 1;
}

void ExitGame(HANDLE t)
{
	system("cls");
	TerminateThread(t, NULL);
}

void PauseGame(HANDLE t)
{
	SuspendThread(t);
}

void DrawTunnel(const char *str)
{
	for (auto v = 0; v < 5; v++) 
	{
		int i = tunnel.middle_of_tunnel.x + cordinate_x[tunnel.direction - 1][v], j = tunnel.middle_of_tunnel.y + cordinate_y[tunnel.direction - 1][v];
		GotoXY(i, j);
		printf(str);
	}
}

void Eat()
{
	//if snake eat 4 food, the tunnel will appear and snake will go through it
	if (FOOD_INDEX == MAX_SIZE_FOOD) snake[SIZE_SNAKE] = tunnel.middle_of_tunnel;
	else snake[SIZE_SNAKE] = food[FOOD_INDEX];
	if (FOOD_INDEX == MAX_SIZE_FOOD - 1) 
	{
		createTunnel();
		FOOD_INDEX++;
	}
	else if (FOOD_INDEX == MAX_SIZE_FOOD) 
	{
		DrawTunnel(" ");
		FOOD_INDEX = 0;
		if (SPEED == MAX_SPEED) 
		{
			SPEED = 1;
			SIZE_SNAKE = 6;
		}
		else SPEED++;
		GenerateFood();
	}
	else 
	{
		FOOD_INDEX++; 
		SIZE_SNAKE++;
	}
}

void TextColor(int color);
void DrawSnakeAndFood(const char *str)
{
	if (FOOD_INDEX < MAX_SIZE_FOOD) 
	{
		GotoXY(food[FOOD_INDEX].x, food[FOOD_INDEX].y);
		printf(str);
	}
	for (int i = 0; i < SIZE_SNAKE; i++) 
	{
		GotoXY(snake[i].x, snake[i].y);
		TextColor(1 + rand() % 15);
		printf("%c", student_id[i % 8]);
	}
}

//DrawSnakeAndFood
void ClearSnakeAndFood(const char *str)
{
	GotoXY(food[FOOD_INDEX].x, food[FOOD_INDEX].y);
	printf(str);
	for (int i = 0; i < SIZE_SNAKE; i++) 
	{
		GotoXY(snake[i].x, snake[i].y);
		printf(str);
	}
}

//check collision wall
bool isCollisionWall(int x, int y)
{
	for (auto i = 0; i < SIZE_SNAKE; i++) 
	{
		if (snake[i].x == x && snake[i].y == y) return true;
		for (auto j = 0; j < obstacles.size(); j++)
		if (x == obstacles[j].x && y == obstacles[j].y) return true;
	}
	return false;
}

bool isCollisionTunnel(int x, int y)
{
	for(auto v = 0; v < 5; v++)
	{
		int i = tunnel.middle_of_tunnel.x + cordinate_x[tunnel.direction - 1][v], j = tunnel.middle_of_tunnel.y + cordinate_y[tunnel.direction - 1][v];
		if(x == i && y == j) return true;
	}
	return false;
}

void printInfo()
{
	//need optimize for avoiding dropping frame 
	/*GotoXY(5, 23);
	TextColor(10);
	cout << char(220) << char(220) << char(220) << char(220) << char(220) << char(220) << char(220) << char(220) ;
	TextColor(13); 
	cout << " Score ";
	TextColor(10);
	cout  << char(220)  << char(220) << char(220) << char(220) << char(220) << char(220) << char(220);
	GotoXY(7, 24);
	TextColor(14);
	cout << " Food       Level";
	for(int i = 0; i <= 21; i++)
	{
		GotoXY(5 + i, 4 + 18 + 6);
		TextColor(10);
		cout << char(223);
	}
	for(int i = 4 ; i <= 7; i++)
	{
		GotoXY(5, i + 20);
		cout << char(219);
		GotoXY(5 + 16 + 5, i + 20);
		cout << char(219);
		GotoXY(5 + 8 + 2, i + 20);
		cout << char(219);
	} 
	GotoXY(10, 26);
	cout << FOOD_INDEX;
	GotoXY(21, 26);
	cout << SPEED;*/
	TextColor(ColorCode_Cyan);
	GotoXY(5, 23);
	cout << "Food: " << FOOD_INDEX;
	GotoXY(50, 23);
	cout << "Level: " << SPEED;
}

void clearInfo()
{
	GotoXY(WIDTH_CONSOLE / 2, HEIGH_CONSOLE + 3);
	printf("               ");
}

void TextColor(int color);
void drawGameOver()
{
	GotoXY(8, 15);
	cout << " .----------------.  .----------------.   .----------------.  .----------------.  .-----------------." << endl;
	GotoXY(8, 16);
	cout << "| .--------------. || .--------------. | | .--------------. || .--------------. || .--------------. |" << endl;
	TextColor(1 + rand() % 15);
	GotoXY(8, 17);
	cout << "| |    _______      ___     .___  ___. _______        ______  ____    ____ _______ .______        | |"<<endl;
	GotoXY(8, 18);
	cout << "| |   /  _____|    /   \\    |   \\/   ||   ____|      /  __  \\ \\   \\  /   /|   ____||   _  \\	  | |"<<endl;
	GotoXY(8, 19);
	cout << "| |  |  |  __     /  ^  \\   |  \\  /  ||  |__        |  |  |  | \\   \\/   / |  |__   |  |_)  |      | |"<<endl;
	GotoXY(8, 20);
	cout << "| |  |  | |_ |   /  /_\\  \\  |  |\\/|  ||   __|       |  |  |  |  \\      /  |   __|  |      /       | |"<<endl;
	GotoXY(8, 21);
	cout << "| |  |  |__| |  /  _____  \\ |  |  |  ||  |____      |  `--'  |   \\    /   |  |____ |  |\\  \\----.  | |"<<endl;
	GotoXY(8, 22);
	cout << "| |   \\______| /__/     \\__\\|__|  |__||_______|      \\______/     \\__/    |_______|| _| `._____|  | |"<<endl;
	TextColor(1 + rand() % 15);
	GotoXY(8, 23);
	cout << "| '--------------' || '--------------' | | '--------------' || '--------------' || '--------------' |" << endl;
	GotoXY(8, 24);
	cout << " '----------------'  '----------------'   '----------------'  '----------------'  '----------------' " << endl;
}

void drawBarGameOver()
{
	system("cls");
	int i = 6, j = 12;
	GotoXY(i, j);
	for(int i = 0; i < 110; i++)
	{
		TextColor(rand() % 15 + 1);
		Sleep(1);
		cout << char(178);
	}
	GotoXY(i, j + 22);
	for(int i = 0; i < 110; i++)
	{
		TextColor(rand() % 15 + 1);
		Sleep(1);
		cout << char(178);
	}
	TextColor(ColorCode_Cyan);
	GotoXY(50, j + 14);
	cout << "Y: Play Again";
	GotoXY(50, j + 15);
	cout << "T: Upload Game";
	GotoXY(50, j + 16);
	cout << "H: High Score";
	GotoXY(50, j + 17);
	cout << "C: Credit"; 
	GotoXY(50, j + 18);
	cout << "Z: Classic mode";
	GotoXY(50, j + 19);
	cout << "X: Obstacle mode";
	GotoXY(50, j + 20);
	cout << "Esc: Exit";
}

void ProcessDead()
{
	STATE = 0;
	//play sound when game over
	playSoundGameOver();
	//pause for user lost
	clearInfo();
	ClearSnakeAndFood(" ");
	//ClearSnakeAndFood("x");
	int k = 0;
	while(k < 30)
	{
		TextColor(1 + rand() % 15);
		int j = 0;
		for(int i = 0; i < SIZE_SNAKE; i++)
		{
			GotoXY(snake[i].x, snake[i].y);
			printf("%c", student_id[j++]);
			if(j > 6) j = 0;
			Sleep(50);
			k++;
		}
	}
	//splash goodbye
	Sleep(1000);
	int checkDraw = 1;
	drawBarGameOver();
	while(checkDraw < 5)
	{
		drawGameOver();
		Sleep(200);
		checkDraw++;
	}
	//splash goodbye
	/*high score*/
	ofstream fout;
	fout.open("data_score.txt", ios::app);
	if(!fout.is_open())
	{
		cerr << "file can't open";
		exit(0);
	}
	fout << SIZE_SNAKE << endl;
	fout.close();
	/*high score*/
	/*high score sorted*/
	fstream myfile("data_score.txt");
	vector<int> x(istream_iterator<int>(myfile), {});
	sort(x.begin(), x.end(), greater<int>());
	ofstream fout1;
	fout1.open("highscore_sorted.txt");
	if(!fout1.is_open())
	{
		cerr << "file can't open";
		exit(0);
	}
	for(const auto& element: x) fout1 << "Player unknown archieved score: " << element << endl;
	fout1.close();
	/*high score sorted*/
}

//handle movement of snake
void MoveRight()
{
	if (snake[SIZE_SNAKE - 1].x + 1 >= WIDTH_CONSOLE) ProcessDead();
	else if (isCollisionWall(snake[SIZE_SNAKE - 1].x + 1, snake[SIZE_SNAKE - 1].y)) ProcessDead(); 
	else if (FOOD_INDEX == MAX_SIZE_FOOD) 
	{
		if (snake[SIZE_SNAKE - 1].x + 1 == tunnel.middle_of_tunnel.x && snake[SIZE_SNAKE - 1].y == tunnel.middle_of_tunnel.y && tunnel.direction == 4) Eat();
		else if (isCollisionTunnel(snake[SIZE_SNAKE - 1].x + 1, snake[SIZE_SNAKE - 1].y)) ProcessDead();
	}
	else if (snake[SIZE_SNAKE - 1].x + 1 == food[FOOD_INDEX].x && snake[SIZE_SNAKE - 1].y == food[FOOD_INDEX].y) Eat();
	for (int i = 0; i < SIZE_SNAKE - 1; i++) 
	{
		snake[i].x = snake[i + 1].x;
		snake[i].y = snake[i + 1].y;
	}
	snake[SIZE_SNAKE - 1].x++;
}

void MoveLeft()
{
	if (snake[SIZE_SNAKE - 1].x - 1 <= 0) ProcessDead();
	else if (isCollisionWall(snake[SIZE_SNAKE - 1].x - 1, snake[SIZE_SNAKE - 1].y)) ProcessDead();
	else if (FOOD_INDEX == MAX_SIZE_FOOD) 
	{
		if (snake[SIZE_SNAKE - 1].x - 1 == tunnel.middle_of_tunnel.x && snake[SIZE_SNAKE - 1].y == tunnel.middle_of_tunnel.y && tunnel.direction == 2) Eat();
		else if (isCollisionTunnel(snake[SIZE_SNAKE - 1].x - 1, snake[SIZE_SNAKE - 1].y)) ProcessDead();
	}
	else if (snake[SIZE_SNAKE - 1].x - 1 == food[FOOD_INDEX].x && snake[SIZE_SNAKE - 1].y == food[FOOD_INDEX].y) Eat();
	for (int i = 0; i < SIZE_SNAKE - 1; i++) 
	{
		snake[i].x = snake[i + 1].x;
		snake[i].y = snake[i + 1].y;
	}
	snake[SIZE_SNAKE - 1].x--;
}

void MoveDown()
{
	if (snake[SIZE_SNAKE - 1].y + 1 >= HEIGH_CONSOLE) ProcessDead();
	else if (isCollisionWall(snake[SIZE_SNAKE - 1].x, snake[SIZE_SNAKE - 1].y + 1)) ProcessDead();
	else if (FOOD_INDEX == MAX_SIZE_FOOD) 
	{
		if (snake[SIZE_SNAKE - 1].x == tunnel.middle_of_tunnel.x && snake[SIZE_SNAKE - 1].y + 1 == tunnel.middle_of_tunnel.y && tunnel.direction == 1) Eat();
		else if (isCollisionTunnel(snake[SIZE_SNAKE - 1].x, snake[SIZE_SNAKE - 1].y + 1)) ProcessDead();
	}
	else if (snake[SIZE_SNAKE - 1].x == food[FOOD_INDEX].x && snake[SIZE_SNAKE - 1].y + 1== food[FOOD_INDEX].y) Eat();
	for (int i = 0; i < SIZE_SNAKE - 1; i++) 
	{
		snake[i].x = snake[i + 1].x;
		snake[i].y = snake[i + 1].y;
	}
	snake[SIZE_SNAKE - 1].y++;
}

void MoveUp()
{
	if (snake[SIZE_SNAKE - 1].y - 1 <= 0) ProcessDead();
	else if (isCollisionWall(snake[SIZE_SNAKE - 1].x, snake[SIZE_SNAKE - 1].y - 1)) ProcessDead();
	else if (FOOD_INDEX == MAX_SIZE_FOOD) 
	{
		if (snake[SIZE_SNAKE - 1].x == tunnel.middle_of_tunnel.x && snake[SIZE_SNAKE - 1].y - 1 == tunnel.middle_of_tunnel.y && tunnel.direction == 3) Eat();
		else if (isCollisionTunnel(snake[SIZE_SNAKE - 1].x, snake[SIZE_SNAKE - 1].y - 1)) ProcessDead();
	}
	else if (snake[SIZE_SNAKE - 1].x == food[FOOD_INDEX].x && snake[SIZE_SNAKE - 1].y - 1 == food[FOOD_INDEX].y) Eat();
	for (int i = 0; i < SIZE_SNAKE - 1; i++) 
	{
		snake[i].x = snake[i + 1].x;
		snake[i].y = snake[i + 1].y;
	}
	snake[SIZE_SNAKE - 1].y--;
}

//load and save game
void saveGame(char *myFile)
{	
	ofstream fout;
	fout.open(myFile, ios::binary);
	if(!fout.is_open())
	{
		cerr << "File can't open";
		exit(0);
	}
	fout.write((char*)&SIZE_SNAKE, sizeof(int));
	fout.write((char*)&snake, SIZE_SNAKE * sizeof(POINT));
	fout.write((char*)&FOOD_INDEX, sizeof(int));
	fout.write((char*)&food, MAX_SIZE_FOOD * sizeof(POINT));
	fout.write((char*)&tunnel, sizeof(Tunnel));
	fout.write((char*)&SPEED, sizeof(int));
	fout.write((char*)&CHAR_LOCK, sizeof(char));
	fout.write((char*)&MOVING, sizeof(char));
	fout.close();
}

bool loadGame(char *myFile)
{
	ifstream fin;
	fin.open(myFile, ios::binary);
	if(!fin.is_open()) return false;
	StartGame();
	fin.read((char*)&SIZE_SNAKE, sizeof(int));
	fin.read((char*)&snake, SIZE_SNAKE * sizeof(POINT));
	fin.read((char*)&FOOD_INDEX, sizeof(int));
	fin.read((char*)&food, MAX_SIZE_FOOD * sizeof(POINT));
	fin.read((char*)&tunnel, sizeof(Tunnel));
	fin.read((char*)&SPEED, sizeof(int));
	fin.read((char*)&CHAR_LOCK, sizeof(char));
	fin.read((char*)&MOVING, sizeof(char));
	fin.close();
	return true;
}
//load and save game

//splash screen
int inputKey()
{
	if (_kbhit())
	{
		int key = _getch();
		if (key == 224)	
		{
			key = _getch();
			return key + 1000;
		}
		return key;
	}
	else
	{
		return key_none;
	}
	return key_none;
}

void clrscr()
{
	CONSOLE_SCREEN_BUFFER_INFO csbiInfo;
	HANDLE	hConsoleOut;
	COORD	Home = { 0,0 };
	DWORD	dummy;
	hConsoleOut = GetStdHandle(STD_OUTPUT_HANDLE);
	GetConsoleScreenBufferInfo(hConsoleOut, &csbiInfo);
	FillConsoleOutputCharacter(hConsoleOut, ' ', csbiInfo.dwSize.X * csbiInfo.dwSize.Y, Home, &dummy);
	csbiInfo.dwCursorPosition.X = 0;
	csbiInfo.dwCursorPosition.Y = 0;
	SetConsoleCursorPosition(hConsoleOut, csbiInfo.dwCursorPosition);
}

int whereX()
{
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	if (GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi))
	return csbi.dwCursorPosition.X;
	return -1;
}

int whereY()
{
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	if (GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi))
	return csbi.dwCursorPosition.Y;
	return -1;
}

void TextColor(int color)
{
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), color);
}

void resizeConsole(int width, int height)
{
	HWND console = GetConsoleWindow();
	RECT r;
	GetWindowRect(console, &r);
	MoveWindow(console, r.left, r.top, width, height, TRUE);
}

int key()
{
	int c = _getch();
	if(c == 8) return 1; // Backspace
	else if(c == 9) return 2; // Tab
	else if(c == 13) return 3; // Enter
	else if(c == 27)  return 4; // Esc
	else if(c == 224)
	{
		c = _getch();
		if(c == 72) return 5; // Keyup
		else if(c == 80) return 6; // KeyDown
		else if(c == 75) return 7; // KeyLeft
		else if(c == 77) return 8; // KeyRight
		else if(c == 83) return 9; // Delete
	}
	else if(c == 0)
	{
		c = _getch();
		if(c == 60) return 12; // F2
		if(c == 62) return 13; // F4
	}
}

void Intro()
{
	ShowConsoleCursor(false);
	short i, n = 10;
	GotoXY(38, 5);
	for (i = 0; i <= 40; i++)
	{
		TextColor(9);
		printf("%c", 219); 
		Sleep(n);
	}
	for (i = 3; i <= 23; i++) 
	{
		GotoXY(38, i + 2);
		Sleep(n);
		TextColor(10);
		printf("%c", 219);
	}
	GotoXY(38, 25);
	for (i = 0; i <= 40; i++)
	{
		TextColor(12);
		printf("%c", 219); 
		Sleep(n);
	}
	for (i = 3; i <= 23; i++) 
	{
		GotoXY(78, i + 2);
		Sleep(n);
		TextColor(14);
		printf("%c", 219);
	}
	Sleep(300);
	_setmode(_fileno(stdout), _O_WTEXT);
	TextColor(ColorCode_Cyan);
	int x = 53;
	int y = 12;
	GotoXY(x, y + 2);  
	wcout << L"╦─╦╔╗╦─╦─╔╗";
	GotoXY(x, y + 3);  
	wcout << L"╠─╣╠─║─║─║║";
	GotoXY(x, y + 4);  
	wcout << L"╩─╩╚╝╚╝╚╝╚╝";
	Sleep(500);
	system("cls");
	ShowConsoleCursor(true);
}
//splash screen

void ThreadFunc()
{
	while (1) 
	{
		if (STATE == 1) 
		{
			ClearSnakeAndFood(" ");
			printInfo();
			switch (MOVING)
			{
				case 'A':
				{
					CHAR_LOCK = 'D';
					MoveLeft();
					break;
				}
				case 'D':
				{
					CHAR_LOCK = 'A';
					MoveRight();
					break;
				}
				case 'S':
				{
					CHAR_LOCK = 'W';
					MoveDown();
					break;
				}
				case 'W':
				{
					CHAR_LOCK = 'S';
					MoveUp();
					break;
				}
			}
			if (STATE == 1) 
			{
				if (FOOD_INDEX == MAX_SIZE_FOOD) 
				{
					DrawTunnel("\xDC");
					DrawSnakeAndFood(" ");
				}
				else DrawSnakeAndFood("o");
			}
			Sleep(150 / SPEED);
		}
	}
}

void typeWritter()
{
	GotoXY(40, 15);
	string title = "This game is made by Group 1 - HCMUS\n\n\t\t\t\t           Thanks for playing our game !!!";
	int x = 0;
	while(title[x] != '\0')
	{
		cout << title[x];
		Sleep(35);
		x++;
	};
	Sleep(300);
	system("cls");
}

//splash text snake
void splashStartSnake(char Text, int x, int y)
{
	if(Text == 65 || Text == 97)
	{
		GotoXY(x, y);     
		cout << "   00  ";     
		Sleep(40);
		GotoXY(x, y + 1); 
		cout << " 00  00";     
		Sleep(40);
		GotoXY(x, y + 2); 
		cout << " 00  00";     
		Sleep(40);
		GotoXY(x, y + 3); 
		cout << " 000000";     
		Sleep(40);
		GotoXY(x, y + 4); 
		cout << " 00  00";     
		Sleep(40);
	}
	else if(Text == 69 || Text == 101)
	{
		GotoXY(x, y);     
		cout << " 000000";     
		Sleep(40);
		GotoXY(x, y + 1); 
		cout << " 00    ";     
		Sleep(40);
		GotoXY(x, y + 2); 
		cout << " 000000";     
		Sleep(40);
		GotoXY(x, y + 3); 
		cout << " 00    ";     
		Sleep(40);
		GotoXY(x, y + 4); 
		cout << " 000000";     
		Sleep(40);
	}
	else if(Text == 75 || Text == 107)
	{
		GotoXY(x, y);     
		cout << "00    00";   
		Sleep(40);
		GotoXY(x, y + 1); 
		cout << "00   00 ";   
		Sleep(40);
		GotoXY(x, y + 2); 
		cout << "00 0 0  ";   
		Sleep(40);
		GotoXY(x, y + 3); 
		cout << "00  00  ";   
		Sleep(40);
		GotoXY(x, y + 4); 
		cout << "00    00";   
		Sleep(40);
	}
	else if (Text == 78 || Text == 110)
	{
		GotoXY(x, y);     
		cout << "00     00";  
		Sleep(40);
		GotoXY(x, y + 1); 
		cout << "00 0   00";  
		Sleep(40);
		GotoXY(x, y + 2); 
		cout << "00  0  00";  
		Sleep(40);
		GotoXY(x, y + 3); 
		cout << "00   0 00";  
		Sleep(40);
		GotoXY(x, y + 4); 
		cout << "00     00";  
		Sleep(40);
	}
	else if (Text == 83 || Text == 115)
	{
		GotoXY(x, y);     
		cout << " 000000";     
		Sleep(40);
		GotoXY(x, y + 1); 
		cout << " 00    ";     
		Sleep(40);
		GotoXY(x, y + 2); 
		cout << " 000000";     
		Sleep(40);
		GotoXY(x, y + 3); 
		cout << "     00.";     
		Sleep(40);
		GotoXY(x, y + 4); 
		cout << " 000000";     
		Sleep(40);
	}
}

//loading bar for splash start snake
void wait(float seconds)
{
	clock_t endwait = clock() + seconds * CLOCKS_PER_SEC;
	while(clock() < endwait) {}
}

void M(int n)
{
	for(int i = 1; i <= n; i++) cout << BTIME[2];
}

void starLoadingBar()
{
	TextColor(ColorCode_White);
	GotoXY(13, 13);
	cout << BTIME[1]; 
	M(91); 
	cout << BTIME[3] << endl;
	GotoXY(13, 14);
	cout << BTIME[4]; 
	GotoXY(105, 14); 
	cout << BTIME[4];
	GotoXY(13, 15);
	cout << BTIME[10]; 
	M(91); 
	cout << BTIME[9];
	TextColor(ColorCode_Cyan);
	for(int i = 10; i <= 100; i++)
	{
		GotoXY(4 + i, 14); 
		cout << BTIME[13]; 
		wait(0.1); 
		GotoXY(107, 14); 
		cout << ends << i << " %";
		GotoXY(53, 16); 
		cout << "...LOADING...";
		if(i == 100)
		{
			TextColor(10);
			GotoXY(53, 16); 
			cout << "...COMPLETE...";
			Sleep(500);
		}
	}
}
//loading bar for splash start

void drawSplashStartSnake()
{
	system("cls");
	TextColor(1 + rand() % 15);
	splashStartSnake('S', 37, 2);
	TextColor(1 + rand() % 15);
	splashStartSnake('N', 46, 2);
	TextColor(1 + rand() % 15);
	splashStartSnake('A', 56, 2);
	TextColor(1 + rand() % 15);
	splashStartSnake('K', 66, 2);
	TextColor(1 + rand() % 15);
	splashStartSnake('E', 75, 2);
	starLoadingBar();
}
//splash text snake

//copyright
void TextBT(int xGoc, int yGoc, int xDich, int yDich, char *InPut, int Delay, int Color, int Space)
{
	int i, j = 0;
	TextColor(Color);
	if (xGoc == xDich && yGoc > yDich && Space == 1)
	{
		for (j = 0; j <= strlen(InPut); j++)
		{
			if (InPut[j] != 32)
			{
				for (i = yGoc; i >= yDich; i--)
				{
					GotoXY(xGoc, i);       
					_putch(InPut[j]);
					GotoXY(xGoc, i + 1);   
					_putch(' ');
					Sleep(Delay);
				}
			}
			else
			{
				GotoXY(xGoc, yDich);       
				_putch(' ');
			}
			xGoc++;
		}
	}
	if (yDich == yGoc && xGoc > xDich && Space == 1)
	{
		for (j = 0; j <= strlen(InPut); j++)
		{
			if (InPut[j] != 32)
			{
				for (i = xGoc; i >= xDich; i--)
				{
					GotoXY(i, yGoc);     
					cout << InPut[i];
					Sleep(Delay);
				}
			}
			else 
			{ 
				GotoXY(xGoc, yDich);     
				_putch(' '); 
			}
			xDich++;
		}
	}
}

void copyRight()
{
	char text1[40] = "Copyright 2020 - Ban quyen thuoc ve", text2[30] = "GROUP 1 - HCMUS";
	TextBT(43, 10, 43, 2, text1, 10, 1 + rand() % 15, 1);
	TextBT(43, 12, 43, 4, text2, 10, 1 + rand() % 15, 1);
	Sleep(100);
}
//copyright

//splash goodbye
void splashBye()
{
	Sleep(150);
	TextColor(1 + rand() % 15);
	GotoXY(25, 15);
	cout << "  _______   ______    ______   _______  .______  ____    ____ _______ "<<endl;
	GotoXY(25, 16);
	cout << " /  _____| /  __  \\  /  __  \\ |       \\ |   _  \\ \\   \\  /   /|   ____|"<<endl;
	Sleep(150);
	TextColor(1 + rand() % 15);
	GotoXY(25, 17);
	cout << "|  |  __  |  |  |  ||  |  |  ||  .--.  ||  |_)  | \\   \\/   / |  |__   "<<endl;
	GotoXY(25, 18);
	cout << "|  | |_ | |  |  |  ||  |  |  ||  |  |  ||   _  <   \\_    _/  |   __|  "<<endl;
	Sleep(150);
	TextColor(1 + rand() % 15);
	GotoXY(25, 19);
	cout << "|  |__| | |  `--'  ||  `--'  ||  '--'  ||  |_)  |    |  |    |  |____ "<<endl;
	GotoXY(25, 20);
	cout << " \\______|  \\______/  \\______/ |_______/ |______/     |__|    |_______|"<<endl;
	Sleep(500);
	system("cls");
}
//splash goodbye

//mode through wall
void DrawBoardOriginal(int x, int y, int width, int height, int curPosX = 0, int curPosY = 0)
{
	GotoXY(x, y); cout << (char)220;
	for (int i = 1; i < width; i++) cout << (char)220;
	cout << (char)220;
	GotoXY(x, height + y); cout << (char)220;
	for (int i = 1; i < width; i++) cout << (char)220;
	cout << (char)220;
	for (int i = y + 1; i < height + y + 1; i++) 
	{
		GotoXY(x, i); cout << (char)219;
		GotoXY(x + width, i); cout << (char)219;
	}
	GotoXY(curPosX, curPosY);
}

void initGame(vector<SnakeThroughWall> &snake, SnakeThroughWall &food, SnakeThroughWall &direction, bool &checkGameOver, int Point)
{
	checkGameOver = false;
	SnakeThroughWall bodySnake;
	//print point
	GotoXY(10, 35);
	TextColor(10);
	cout << "Point: " << Point;
	//create snake
	int tmp = 4;
	for(auto i = 0; i < 6; i++)
	{
		//head of snake
		if(i == 0)
		{
			bodySnake.info_snake = char(220);
			//original position of snake is (5, 5);
			bodySnake.x = 5;
			bodySnake.y = 5;
			snake.push_back(bodySnake);
		}
		//body of snake
		else if(i != 0)
		{
			bodySnake.info_snake = char(220);
			bodySnake.x = tmp;
			bodySnake.y = 0;
			snake.push_back(bodySnake);
			tmp--;
		}
	}
	//create food
	srand(time(NULL));
	food.x = 2 + rand() % (NEW_WIDTH_CONSOLE - 4);
	food.y = 2 + rand() % (NEW_HEIGH_CONSOLE - 4);
	//setup original movement of snake is from left to right
	direction.x = 1;
	direction.y = 0;
}

void handleMovement(vector<SnakeThroughWall> &snake, SnakeThroughWall direction, SnakeThroughWall &food, bool &checkGameOver, int &Point)
{
	SnakeThroughWall addLengthSnake;
	if(direction.x == 0 && direction.y == 0)
	{
		for(auto i = 0; i < snake.size(); i++)
		{
			snake[0].x += direction.x;
			snake[0].y += direction.y;
		}
	}
	else
	{
		for(auto i = 0; i < snake.size(); i++)
		{
			//move head of snake
			if(i == 0)
			{
				snake[0].dx = snake[0].x;
				snake[0].dy = snake[0].y;
				snake[0].x += direction.x;
				snake[0].y += direction.y;
			}
			//move body of snake
			else
			{
				snake[i].dx = snake[i].x;
				snake[i].dy = snake[i].y;
				snake[i].x = snake[i - 1].dx;
				snake[i].y = snake[i - 1].dy;
			}
			//handle go through wall
			if(snake[i].x >= NEW_WIDTH_CONSOLE) snake[i].x = 0;
			if(snake[i].x < 0) snake[i].x = NEW_WIDTH_CONSOLE - 1;
			if(snake[i].y >= NEW_HEIGH_CONSOLE) snake[i].y = 0;
			if(snake[i].y < 0) snake[i].y = NEW_HEIGH_CONSOLE - 1;
			//check body collision
			if(i != 0 && snake[0].x == snake[i].x && snake[0].y == snake[i].y) checkGameOver = true;
		}
	}
	//print food
	GotoXY(food.x, food.y);
	cout << "o";
	//eat food
	if(snake[0].x == food.x && snake[0].y == food.y)
	{
		//add length of snake after eating food
		addLengthSnake.info_snake = char(220);
		addLengthSnake.x = snake[snake.size() - 1].dx;
		addLengthSnake.y = snake[snake.size() - 1].dy;
		snake.push_back(addLengthSnake);
		//print current point 
		Point++;
		GotoXY(10, 35);
		TextColor(10);
		cout << "Point: " << Point;
		//create new food after eating food
		srand(time(NULL));
		food.x = 2 + rand() % (NEW_WIDTH_CONSOLE - 4);
		food.y = 2 + rand() % (NEW_HEIGH_CONSOLE - 4);
	}
}

void drawSnake(vector<SnakeThroughWall> &snake)
{
	for(auto i = 0; i < snake.size(); i++)
	{
		GotoXY(snake[i].x, snake[i].y);
		cout << snake[i].info_snake;
	}
	GotoXY(snake[snake.size() - 1].dx, snake[snake.size() - 1].dy);
	cout << " ";
}

void mainGameSnakeThroughWall(keyInput &movement, keyInput &statusExit, vector<SnakeThroughWall> &snake, SnakeThroughWall &direction, SnakeThroughWall &food, bool &checkGameOver, int &Point)
{
	handleMovement(snake, direction, food, checkGameOver, Point);
	int key = inputKey();
	if(key == 'a' || key == 'A') movement = Left;
	else if(key == 'd' || key == 'D') movement = Right;
	else if(key == 's' || key == 'S') movement = Down;
	else if(key == 'w' || key == 'W') movement = Up;
	else if(key == 27) statusExit = Exit; 
	if (movement == Left)
	{
		if (direction.x != 1)
		{
			direction.x = -1;
			direction.y = 0;
		}
	}
	else if (movement == Right)
	{
		if (direction.x != -1)
		{
			direction.x = 1;
			direction.y = 0;
		}
	}
	else if (movement == Up)
	{
		if (direction.y != 1)
		{
			direction.y = -1;
			direction.x = 0;
		}
	}
	else if (movement == Down)
	{
		if (direction.y != -1)
		{
			direction.y = 1;
			direction.x = 0;
		}
	}
	if (statusExit == Exit)
	{
		checkGameOver = true;
		exit(0);
	}
}

void startModeGame()
{
	playSoundTheme();
	vector<SnakeThroughWall> snake;
	SnakeThroughWall direction, food;
	keyInput movement, statusExit;
	int point = 0;
	bool checkGameOver = false;
	initGame(snake, food, direction, checkGameOver, point);
	while(!checkGameOver)
	{
		DrawBoardOriginal(0, 0, NEW_WIDTH_CONSOLE, NEW_HEIGH_CONSOLE);
		Sleep(50);
		mainGameSnakeThroughWall(movement, statusExit, snake, direction, food, checkGameOver, point);
		drawSnake(snake);
	}
	if(checkGameOver == true) playSoundGameOver();
	GotoXY(65, 5);
	cout << "Press any key to exit";
}
//mode through wall

void main()
{
	resizeConsole(1000, 800);
	char myFile[100];
	/*splash screen 1*/
	Intro();
	_setmode(_fileno(stdout), _O_TEXT);
	system("cls");
	/*splash screen 1*/
	int temp;
	FixConsoleWindow();
	/*splashscreen 2*/
	TextColor(ColorCode_DarkCyan);
	for (int i = 0; i < 120; i++)
	{
		printf("=");
	}
	GotoXY(0, 29);
	TextColor(ColorCode_DarkCyan);
	for (int i = 0; i < 120; i++)
	{
		printf("=");
	}	
	int i;
	for (i = 18; i < 100; i++) 
	{
		TextColor(9);
		GotoXY(i, 5); 
		cout<<static_cast<char>(219); 
		TextColor(12);
		GotoXY(i, 20); 
		cout<< static_cast<char>(219);  
	}
	for (i = 5; i <= 20; i++)  
	{
		TextColor(10);
		GotoXY(18, i); 
		cout << static_cast<char>(219); 
		TextColor(14);
		GotoXY(100, i); 
		cout << static_cast<char>(219); 
	}
	ifstream File;
	File.open("splashscreen.txt", ios::in);
	char c;
	GotoXY(10, 8);
	while (!File.eof())
	{
		File.get(c);
		TextColor(1 + rand() % 15);
		cout << c;
	}
	File.close();
	TextColor(ColorCode_Green);
	GotoXY(47, 14);
	cout << "Press any key to continue ";
	/*splashscreen*/
	int s = toupper(_getch());
	ShowConsoleCursor(false);
	drawSplashStartSnake();
	StartGame();
	thread t1(ThreadFunc);
	HANDLE handle_t1 = t1.native_handle();
	while (1) 
	{
		temp = toupper(_getch());
		GotoXY(0, 23);
		if (STATE == 1) 
		{
			if (temp == 'P') 
			{
				PauseGame(handle_t1);
			}
			//control music
			else if(temp == 'M') 
			{
				PlaySound(NULL, NULL, 0);
			}
			else if(temp == 'N') 
			{
				playSoundTheme();
			}
			//control music
			//load and save game
			else if(temp == 'L')
			{
				PauseGame(handle_t1);
				clearInfo();
				ShowConsoleCursor(true);
				GotoXY(0, HEIGH_CONSOLE + 3);
				cout << "Input name of file to save game: ";
				cin.getline(myFile, 100);
				saveGame(myFile);
				GotoXY(0, HEIGH_CONSOLE + 3);
				printf("                                           ");
				ShowConsoleCursor(false);
				ResumeThread(handle_t1);
			}
			else if (temp == 'T') 
			{
			LoadGame:
				PauseGame(handle_t1);
				system("cls");
				ShowConsoleCursor(true);
				cout << "Input name of file for loading game: ";
				cin.getline(myFile, 100);
				while (!loadGame(myFile)) 
				{
					cout <<"Wrong file, try-again!!!" << endl; 
					cout << "Input again name of file for loading game: ";
					cin.getline(myFile, 100);
				}
				ShowConsoleCursor(false);
				//draw loading bar
				system("cls");
				system("color 2f");
				system("color 0e");
				char a = 177, b = 219;
				cout << "\n\n\n\t\t\t\tGame is loading...";
				cout << "\n\n\n";
				cout << "\t\t\t\t\t";
				for(int i = 0; i <= 25; i++) cout << a;
				Sleep(150);
				cout << "\r";
				cout << "\t\t\t\t\t";
				for(int i = 0; i <= 25; i++)
				{
					cout << b;
					Sleep(200);
				}
				system("cls");
				DrawBoardNotObstacle(0, 0, WIDTH_CONSOLE, HEIGH_CONSOLE);
				drawInfoGame();
				//draw loading bar
				ResumeThread(handle_t1);
			}
			//load and save game
			else if (temp == 27) 
			{
				ExitGame(handle_t1);
				PlaySound(NULL, NULL, 0);
				typeWritter();
				splashBye();
				exit(0);
			}
			else 
			{
				ResumeThread(handle_t1);
				if ((temp != CHAR_LOCK) && (temp == 'D' || temp == 'A' || temp == 'W' || temp == 'S')) 
				{
					MOVING = temp;
				}
			}
		}
		else 
		{
			if (temp == 89) 
			{
				DeleteObstacles();
				StartGame();
			}
			//print high score
			else if(temp == 'H')
			{
				system("cls");
				ifstream fin;
				fin.open("highscore_sorted.txt");
				string line;
				while(!fin.eof())
				{
					getline(fin, line);
					cout << line << endl;
				}
			}
			//print high score
			//about us
			else if(temp == 'C')
			{
				//copyright
				system("cls");
				copyRight();
				//copyright
				system("cls");
				struct motionWord
				{
					char name1[100], name2[100], name3[100], name4[100];
					int x, y; 
				};
				motionWord A;
				strcpy_s(A.name1, "\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t    Tran Huy Vu ");
				strcpy_s(A.name2, "\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t    Tran Dai Chi ");
				strcpy_s(A.name3, "\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t    Phan Nhat Minh ");
				strcpy_s(A.name4, "\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t    Dang Nguyen Quynh Nhu ");
				A.x = 50;
				A.y = 30;
				for (int i = 0; i < 29; i++)
				{
					system("cls");
					GotoXY(A.x, A.y);
					TextColor(11);
					printf("%s", A.name1);
					A.y--;
					Sleep(150);
				}
				A.x = 50;
				A.y = 30;
				for (int i = 0; i < 29; i++)
				{
					system("cls");
					GotoXY(A.x, A.y);
					TextColor(12);
					printf("%s", A.name2);
					A.y--;
					Sleep(150);
				}
				A.x = 50;
				A.y = 30;
				for (int i = 0; i < 29; i++)
				{
					system("cls");
					GotoXY(A.x, A.y);
					TextColor(13);
					printf("%s", A.name3);
					A.y--;
					Sleep(150);
				}
				A.x = 50;
				A.y = 30;
				for (int i = 0; i < 29; i++)
				{
					system("cls");
					GotoXY(A.x, A.y);
					TextColor(14);
					printf("%s", A.name4);
					A.y--;
					Sleep(150);
				}
				system("cls");
				GotoXY(0, 0); 
				exit(0);
			}
			//about us
			//load game
			else if (temp == 'T') 
			{
				goto LoadGame;
			}
			//load game
			//mode through wall
			else if(temp == 'Z')
			{
				system("cls");
				DeleteObstacles();
				startModeGame();
			}
			//mode through wall
			//mode obstacle
			else if(temp == 'X')
			{
				SetObstacles(70, 20);
				StartGameObstacle();
			}
			//mode obstacle
			else if(temp == 27)
			{
				ExitGame(handle_t1);
				typeWritter();
				splashBye();
				exit(0);
			}
		}
	}
}