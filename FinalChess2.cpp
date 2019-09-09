// FinalChess2.cpp : 定義主控台應用程式的進入點。
//
  
#include "stdafx.h"
#include <stdlib.h>
#include <stdio.h>
#include <Windows.h>
#include <time.h>
#include <conio.h>
#include <map>
#include <sstream>
#include <iostream>
#include <string>
#include <vector>
#include <list>
#include <numeric> 
#include <algorithm> 
#include <cstring>
#include <fstream>
 
using namespace std;


#define SIZE 255

#define DIR_SAME 0
#define DIR_UP 1
#define DIR_DOWN 2
#define DIR_LEFT 3
#define DIR_RIGHT 4
#define DIR_UNKNOWN 5

#define UP -8
#define DOWN 8
#define LEFT -1
#define RIGHT 1
#define TURN 4  //翻棋

#define COLOR_BLACK 0
#define COLOR_RED 1
#define COLOR_EMPTY 2
#define COLOR_UNKNOWN 64

#define NO_ACTION 0
#define ACTION 1

#define LEVEL 8   //搜尋深度
#define MAX_DEPTH 8   //搜尋深度

#define MAX_NODE_VALUE INT_MAX
#define MIN_NODE_VALUE INT_MIN
#define NULL_NODE_VALUE 99999999

#define GO_CHESS 15  //無法重複走棋的回合


// 各個索引分別代表： 各個索引分別代表： 各個索引分別代表： 0空格 , 1帥, 2仕, 3相, 
//4車, 5 傌, 6 炮, 7 兵,8將, 9 士, 10 象, 
//11車, 12, 馬, 13 包, 14卒,15 Unknown
#define PIECE_N 0
#define PIECE_n 0
#define PIECE_K 1
#define PIECE_A 2
#define PIECE_E 3
#define PIECE_R 4
#define PIECE_H 5
#define PIECE_C 6
#define PIECE_P 7
#define PIECE_k 8
#define PIECE_a 9
#define PIECE_e 10
#define PIECE_r 11
#define PIECE_h 12
#define PIECE_c 13
#define PIECE_p 14
#define PIECE_u 15


//////////////////////////////////////////////////////////////////
string strGameTreeRep="";
int G_treeNodeId=0;
//////////////////////////////////////////////////////////////////


//00000000 00111111 11112222 22222233
//01234567 89012345 67890123 45678901

//10000000 00000000 00000000 00000000 = mask_table[0]
//01000000 00000000 00000000 00000000 = mask_table[1]
//00100000 00000000 00000000 00000000 = mask_table[2]
//00010000 00000000 00000000 00000000 = mask_table[3]
//...
//00000000 00000000 00000000 00000001 = mask_table[31] 

int getColor(char);

unsigned int mask_table[32] = {
	0x80000000, 0x40000000, 0x20000000, 0x10000000, 
	0x08000000, 0x04000000, 0x02000000, 0x01000000,
	0x00800000, 0x00400000, 0x00200000, 0x00100000,
	0x00080000, 0x00040000, 0x00020000, 0x00010000,

	0x00008000, 0x00004000, 0x00002000, 0x00001000,
	0x00000800, 0x00000400, 0x00000200, 0x00000100,
	0x00000080, 0x00000040, 0x00000020, 0x00000010,
	0x00000008, 0x00000004, 0x00000002, 0x00000001,
};

//棋子ID
int table_piece_id[256]; //table_piece_id['A'~'z']


typedef unsigned int U32;  //8x4=32bits 0xFFFFFFFF
// 可以表示 0x00000000 ~ 0xFFFFFFFF
U32 b_piece[16];
U32 b_red, b_black, b_occupied;

//int convert_array_to_bitboard(char p_currentboard[32]) {
//
//	//clear
//	for (int i = 0; i < 16; i++)
//		b_piece[i] = 0;
//	b_red = 0;
//	b_black = 0;
//	b_occupied = 0;
//
//	//b_piece
//	for (int i = 0; i < 32; i++) {//i=0
//		int pid = table_piece_id[p_currentboard[i]]; //  'A' ==> 2 , pid=2
//
//		b_piece[pid] |= mask_table[i]; // b_piece[2]
//
//		if (getColor(p_currentboard[i]) == COLOR_RED) {
//			b_red |= mask_table[i];
//			b_occupied |= mask_table[i];
//		}
//		else if (getColor(p_currentboard[i]) == COLOR_BLACK) {
//		  b_black |= mask_table[i];
//		  b_occupied |= mask_table[i];
//	    }
//		else if (pid != PIECE_N) { //p_currentboard[i] != NULL
//			b_occupied |= mask_table[i];
//		}
//    }
//}
//
//void print_bitboard() {
//
//}

////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////
void SetColor(int f=7,int b=0)
{
    unsigned short ForeColor=f+16*b;
    HANDLE hCon = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(hCon,ForeColor);
}


////////////////////////////////////////////////////////////
int g_debug_cc = 100;

////////////////////////////////////////////////////////////
list<string> number;

//實際盤面
char startboard[32]={'K','k','p','E','p','H','H','R',
	                 'R','C','C','P','P','P','P','P',
					 'A','a','a','e','e','h','h','r',
                     'r','c','c','E','p','p','p','A'};  
char currentBoard[32];		//當前盤面
char tempBoard[32];			//暫存當前盤面
char Simulation[10][32];	//模擬盤面 [level][]

//int makeSelect( int, int, int);
int Computer(int);
void printBoard();


/////////////map////////////////////////

////////////////棋子分數/////////////////
typedef map<char, int> MyValue;
typedef map<char, int> MyScore;
typedef map<int, string> HistoryPlayer;
typedef map<int, string> HistoryComputer;
typedef list<string> HistoryPath1;
typedef list<string> HistoryPath2;
typedef list<string> HistoryPath3; 
typedef list<string> SimulationAction;

MyValue table1;   //棋子大小
MyScore table2;   //棋子分數
HistoryPath1 list1; //紀錄電腦路徑(push-pop use)
HistoryPath2 list2; //紀錄重複路徑
HistoryPath3 list3; //分數計算完後退回root路徑

string table_PieceName[128];
int table_PieceColor[128];

SimulationAction Action;
/////////////////////移動過程/////////////////////

map<int, int> MoveHistory;

//=================重複移動===========//
struct repeat{
	int level;
	int location;
	int path;
};

struct repeat REPEAT[GO_CHESS];

//=================重複移動===========//


class History {
int level;
int location;
int path;


public:

	History() {
	  level = 0;
	  path=0;
	}

	History(int level, int location, int path) {
	  level = level;
	  MoveHistory[location]=path;
	}
};

class ChessMove {
public: 
	int level, location, path,c_loc;
	char to_piece;
	//int cannon_dist; //


	ChessMove() {
		level=0;
		location=0; 
		path=0;
		to_piece = NULL;
		c_loc=99;
	}

	ChessMove(int p_level, int p_location, int p_path, char p_to_piece, int p_c_loc) {
		level = p_level;
		location = p_location;
		path = p_path;
		to_piece = p_to_piece;
		c_loc = p_c_loc;
	}


	void get(int &p_level, int &p_location, int &p_path, char &p_to_piece){
		p_level = level;
		p_location = location;
		p_path = path;
		p_to_piece = to_piece;
	}

	void copy(ChessMove cm){
		level = cm.level;
		location = cm.location;
		path = cm.path;
		to_piece = cm.to_piece;
		c_loc = cm.c_loc;
	}

	void copy(ChessMove *cm){
		level = cm->level;
		location = cm->location;
		path = cm->path;
		to_piece = cm->to_piece;
		c_loc = cm->c_loc;
	}
};

vector<History*> *transTable;


//K->帥 A->仕 E->相 H->傌 R->? C->炮 P->兵
//k->將 A->士 e->象 h->馬 r->車 c->包 p->卒
//===================socre====================
//int K=2000,A=1000,E=800,R=600,H=400,C=0,P=200;
char Score[7]={'C','P','H','R','E','A','K'};

//Global variables
//push pop control

int temp[5]={UP,DOWN,LEFT,RIGHT,TURN};  //上下左右
int decoding[4]; //[0]=level [1]=location [2]=path [3]C_loc
char disk_CP[32]; //紀錄用盤面
char disk_PL[32]; //紀錄用盤面
int Record[10][5];
int ROUND=0;
char chess; //紀錄被吃掉的棋子
//int G_score;  //紀錄盤面分數  [4]上一手的方向   [32]位置   [10]本次下棋的方向
int counter[32];  //計算分數用盤面

int lv_push=5,lv_pop=5,lv=0;
int ex=0; //跳出遞迴


int G_Mastership_CP, G_Mastership_PL;  //Game() 代表玩家與電腦棋子的主控權
int control=0;

//每個tree node 都有level， 不要用global 變數
//int G_Mastership; //Game()中輪到誰下 (不是Computer()的Game tree)
//int level=1; //深度

//int Move, Move_CP, Move_PL;  //玩家與電腦上次移動到的位置
int Path=0; //, Path_CP, Path_PL; //玩家與電腦上次移動路徑
int push=0,pop=0;
int color=0; //決定顏色
int Rotation;  //回合輪替
int site;  //generate_All_Moves
int nodes=0;
int Repeat=0; //計算反覆走步
int C_score=0;  //炮&包的分數
int Reward_Points=100;
int TIMES=0;
int SCORE=0;

int End(int end, char currentBoard[]) {

	int Red=0,Black=0,U=0;

	for(int i=0;i<32;i++) {
	 if(currentBoard[i]>'A' && currentBoard[i]<'Z')   Red++;
	 if(currentBoard[i]>'a' && currentBoard[i]<'z')   Black++;
	 if(currentBoard[i]==64) U++;
	}
	
	if(Red==0 && Black>0 && U==0)   return COLOR_BLACK;  //黑贏
	else if(Red>0 && Black==0 && U==0) return COLOR_RED;  //紅贏
	//snow
	//else if(Red!=0 || Black!=0 || U!=0) return 2;  //繼續對局
	else return COLOR_EMPTY;

	//s
	//return 1;
}

//1表示玩家所選之顏色(red)
int selectOrder() {
	char color;
	char order;

	//printf("You want to Before or After b/A?");
	//scanf("%c",&order);
	order='b';
	//預設值為後手
	if(order=='b' || order=='B')  //先手
	  return 1;
	else                          //後手
	  return 0;



	/*
	printf("Select color Red/Black (r/B):");
	scanf("%c",&color);

	if(color=='R' || color=='r') {
	   Mastership_PL=1;
       Mastership_CP=0;
	   printf("You choose Red!!\n");
	   system("pause");
	   return 1;
	}

	Mastership_PL=0;
    Mastership_CP=1;

	printf("You choose Black!!\n");
	*/
	return 0;
}

//const int x=8,y=4;
/*
char* two_one(char *Simulation, char b[x*y]) {

	for(int i = 5 ; i < x*y; i++)
    {
     b[i] = Simulation[i];
     }
    return b;
}
*/

//////計算盤面分數                    
////void Counter(int Mastership, int location, int path) {
////
////	char str1[7]={'K','A','E','R','H','C','P'}; //RED
////	char str2[7]={'k','a','e','r','h','c','p'}; //BLACK
////	int sc_t=0, sc_r=0, sc_b=0;  //分數
////
////
////	for(int i=0;i<32;i++) {
////		for(int j=0;j<7;j++) {
////		  //計算盤面紅棋分數
////		  if(currentBoard[i] == table2.find(str1[i])->first)   //first表示map裡面的key=棋子
////			 sc_r+=table2.find(str1[i])->second;   //second表示map裡面的value=棋子的分數
////		  //計算盤面黑棋分數
////		  if(currentBoard[i] == table2.find(str2[i])->first)
////			 sc_b+=table2.find(str2[i])->second;   
////		}// j
////	  }// i
////
////	//COLOR_RED代表大寫  COLOR_BLACK代表小寫
////	if(Mastership==COLOR_RED) {
////	  sc_t = sc_r - sc_b;  //盤面分數紅棋-黑棋  sc_t盤面總合分數  sc_r紅棋分數  sr_b黑棋分數
////	  //score[1][location][path]=sc_t;
////	}//if
////
////	if(Mastership==COLOR_BLACK) {
////		sc_t = sc_b - sc_r;  //盤面分數紅棋-黑棋  sc_t盤面總合分數  sc_r紅棋分數  sr_b黑棋分數
////	  //score[1][location][path]=sc_t;
////	}
////}

//eat chess
char eat(char Board1[], int loc, int path) {

	int temp1=0,temp2=0;

	for(int i=0;i<7;i++)
	 if(Board1[loc]==Score[i]) temp1=i;  

	switch(path) {
		case 1: for(int i=0;i<7;i++)
				  if(Board1[loc-8]==Score[i]+32) temp2=i;
			    break;
		case 2: for(int i=0;i<7;i++)
				  if(Board1[loc+8]==Score[i]+32) temp2=i;
			    break;
		case 3: for(int i=0;i<7;i++)
				  if(Board1[loc-1]==Score[i]+32) temp2=i;
				break;
		case 4: for(int i=0;i<7;i++)
				  if(Board1[loc+1]==Score[i]+32) temp2=i;
				break;
	}

	if(temp1==6 && temp2==1) return 0;
	else if(temp1>=temp2) return 1;
	else return 0;
	
}

//////AAAAAAA

/*char* getPieceName(char c) {

	table_PieceName[c].c_str();
}
*/

//snow
/*
//start the board
void startBoard(char startboard[]) {

	char temp,a,b;


	srand(time(NULL));

	for(int i=0;i<10000;i++) {
	
		 a=rand()%32;
		 b=rand()%32;

		 temp = startboard[a];
		 startboard[a] = startboard[b];
		 startboard[b] = temp;
	 }
}
void Tokenize(const string& str,
                      vector<string>& tokens,
                      const string& delimiters = " ")
{
    // Skip delimiters at beginning.
    string::size_type lastPos = str.find_first_not_of(delimiters, 0);
    // Find first "non-delimiter".
    string::size_type pos     = str.find_first_of(delimiters, lastPos);

    while (string::npos != pos || string::npos != lastPos)
    {
        // Found a token, add it to the vector.
        tokens.push_back(str.substr(lastPos, pos - lastPos));
        // Skip delimiters.  Note the "not_of"
        lastPos = str.find_first_not_of(delimiters, pos);
        // Find next "non-delimiter"
        pos = str.find_first_of(delimiters, lastPos);
    }
}
string getKey(char Board[32]) {
	string key = "";

	for(int i=0; i<32; i++) {
		key += Board[i] ;		
	    key += " " ;
	}
  
	return key;
}
int setKey(string key, char Board[32]) {

	vector<string> tokens;
	Tokenize(key, tokens);

	for (int i=0; i<tokens.size() && i <32; i++) {
		stringstream ss(tokens[i]);
		ss >> Board[i];
	}
	
	return 0;
}
*/

 
inline int getPos(int X, int Y) {
	return Y*8 + X; //
}

inline int getX(int pos) {  //0-7 => 0, 8-15 =>1, 16-23 => 2, 24-31 => 3 
	return pos % 8; 
}

inline int getY(int pos) {
	return pos / 8; 
}



//00 01 02 03 04 05 06 07
//08 09 10 11 12 13 14 15
//
inline int getUp(int pos) { 
	if(getY(pos)==0) {
		printf("ERR 1");
		return pos;
	}
	
	return pos - 8; //0-7 => 0, 8-15 =>1, 16-23 => 2, 24-31 => 3 
}

inline int getDown(int pos) {
	if(getY(pos)==3) {
		printf("ERR 2");
		return pos;
	}

	return pos + 8; //0-7 => 0, 8-15 =>1, 16-23 => 2, 24-31 => 3 
}

inline int getLeft(int pos) {
	if(getX(pos)==0) {
		printf("ERR 3");
		return pos;
	}

	return pos - 1; //0-7 => 0, 8-15 =>1, 16-23 => 2, 24-31 => 3 
}

inline int getRight(int pos) {
	if(getX(pos)==7) {
		printf("ERR 4");
		return pos;
	}

	return pos + 1; //0-7 => 0, 8-15 =>1, 16-23 => 2, 24-31 => 3 
}

//snow ?
//下一手
int makeMove(char currentBoard[32], ChessMove *amove) {

	//assume that amove->path = UP, and UP=-8
	//then, amove->location + amove->path = amove->location + UP

	//吃棋&空步
	if(amove->c_loc==99) { //吃隔壁

	  amove->to_piece = currentBoard[amove->location + temp[amove->path]];                //c_loc=99
	  currentBoard[amove->location + temp[amove->path]] = currentBoard[amove->location];  //location=24, c_loc=99, temp[amove->path]=2
	  currentBoard[amove->location] = NULL;                                               //24==>26: 26=24, 24=NULL

	  //吃棋分數高

	}
	else { //砲吃
		amove->to_piece = currentBoard[amove->c_loc];                //c_loc=被吃的砲
		currentBoard[amove->c_loc] = currentBoard[amove->location];  //location=24, c_loc=25
		currentBoard[amove->location] = NULL;                        //24==>25: 25=24, 24=NULL    

		C_score = table2[amove->to_piece];                           //被吃的棋子的分數
	}

	//printBoard();

	//currentBoard[amove->location] = decoding[1] + temp[decoding[decoding[2]]]] = disk_PL[decoding[1]]; //從此位置開始模擬
	return 0;
}

//上一手
int unMakeMove(ChessMove *amove, int Mastership) {


	//system("pause");
	if(amove->c_loc==99) {
	currentBoard[amove->location] = currentBoard[amove->location + temp[amove->path]]; 
	currentBoard[amove->location + temp[amove->path]] = amove->to_piece;
	}

	else {
		currentBoard[amove->location] = currentBoard[amove->c_loc];
		currentBoard[amove->c_loc] = amove->to_piece;
	}

	//printBoard();
	
	//cout << "POP : " <<currentBoard[amove->location + temp[amove->path]] << " Recovery : " << amove->to_piece << endl;
	//system("pause");

	//currentBoard[amove->location] = decoding[1] + temp[decoding[decoding[2]]]] = disk_PL[decoding[1]]; //從此位置開始模擬
	return 0;
}

//空步
char canMove(int from, int to, int Mastership) {

	int color=-1;

	//空步不得超出棋盤範圍
	if(to<0 || to>31)
	  return NO_ACTION;

	//如果空步的棋子為未翻棋 || 空步棋位置無棋 || 空步目地位置為未翻棋 || 空步目地位置有棋 皆不可空步 
	if(currentBoard[from]==64 || currentBoard[from]==NULL || currentBoard[to]==64 || currentBoard[to]!=NULL)
	   return NO_ACTION;

	//如果非自己顏色的棋子則不能移動
	
	if(currentBoard[from]>'a' && currentBoard[from] <'z')  
	   color=COLOR_BLACK;
	//s
	else //if(currentBoard[from]>'A' && currentBoard[from] <'Z')
	   color=COLOR_RED;

	//s
	//if (color == COLOR_RED && Mastership == COLOR_BLACK || color == COLOR_BLACK && Mastership == COLOR_RED)
	if (color != Mastership)
	   return NO_ACTION;
	 
	

	

	//getY(from)>0   表示棋子往上移動的極限值
	//getY(from)-getY(to)==1  表示棋子只能移動一格
	//getX(from)-getX(to)==0  表示棋子起點與目的地都在同一個Y軸上
	//Board[from]!=NULL   表示移動的起點棋子存在
	//Board[to]==NULL    表示移動的目的地沒有棋子
	//移動方向-上
	if(	getY(from)>=0 
		&& 
		getY(from)-getY(to)==1 && 
		getX(from)-getX(to)==0 && 
		currentBoard[from]!=NULL && 
		currentBoard[to]==NULL) {

	  return DIR_UP;
	}

	//移動方向-下
	//else if(Board[getDown(from)]==0 && Board[from]!=NULL && getDown(from) < 32 && from-to==-8 && Board[to]==NULL) {
	else if (getY(from)<=3 && 
		getY(from)-getY(to)==-1 && 
		getX(from)-getX(to)==0 && 		
		currentBoard[from]!=NULL && 
		currentBoard[to]==NULL) {
	   
       return DIR_DOWN;
	}

	//移動方向-左
	//else if(to/8==(from-1)/8 && Board[from]!=NULL && from-1>0 && from-to==1 && Board[to]==NULL) {
	else if(getX(from)>=0 && 
		getX(from)-getX(to)==1 && 
		getY(from)-getY(to)==0 && 
		currentBoard[from]!=NULL && 
		currentBoard[to]==NULL) {
	   
	   return DIR_LEFT;
	}

	//移動方向-右
	//else if(to/8==(from+1)/8 && Board[from]!=NULL && from-1<32 && from-to==-1 && Board[to]==NULL) {
	else if(getX(from)<=7 && 
		getX(from)-getX(to)==-1 && 
		getY(from)-getY(to)==0 && 
		currentBoard[from]!=NULL && 
		currentBoard[to]==NULL) {
	  
	   return DIR_RIGHT;
	}

	return 0;
}


//copy Board
char copy(char Board1[], char currentBoard[]) {

	for(int i=0;i<32;i++)
	  Board1[i] = currentBoard[i];

	return 0;
}


//編碼
string getNumber(int from, int to, char Status) {
	  
	string number;

	//吃棋->e
	if(Status=='e') {
	   number='e';

	    if(from<10) {
	      number+="0";
	      number+= (from%10) + 48;
	   }

	   else{
	      number += (from/10) + 48;
	      number += (from%10) + 48;
	   }

	   if(to<10) {
		 number+="0";
	     number+= (to%10) + 48;
	   }

	   else{
		  number += (to/10) + 48;
	      number += (to%10) + 48;
	   }
	}

	//移動-> m
	if(Status=='m') {
	   number='m';

	    if(from<10) {
	      number+="0";
	      number+= (from%10) + 48;
	   }

	   else{
	      number += (from/10) + 48;
	      number += (from%10) + 48;
	   }

	   if(to<10) {
		 number+="0";
	     number+= (to%10) + 48;
	   }

	   else{
		  number += (to/10) + 48;
	      number += (to%10) + 48;
	   }
	}

	//翻棋->t
	if(Status=='t') {
	   number='t';

	   if(to==0 && from<10) {
	      number+="0";
	      number+= (from%10) + 48;
	   }

	   else {
	      number += (from/10) + 48;
	      number += (from%10) + 48;
	   }
	}

	  return number;
}
//Turned chess
int canSelect(int level,  int location) {

	if(Simulation[level][location]=='U') 
	   return ACTION;
	else 
	   return NO_ACTION;

}

string test(char cc){   //棋子中文
	
	return table_PieceName[cc];

	}

//print Board
void printBoard(){
	/*
	cout <<"┌───────────────────────────────┐"<<endl;
	cout <<"│         中英文對照  ─→  黑方(紅方) = 英文代號對照          │"<<endl;
	cout <<"│==============================================================│"<<endl;
	cout <<"│ 將(帥)=k(K)     士(仕)=a(A)     象(相)=e(E)     車(陣)=r(R)  │"<<endl;
	cout <<"│ 馬(傌)=h(H)     包(炮)=c(C)     卒(兵)=p(P)                  │"<<endl;
	cout <<"└───────────────────────────────┘"<<endl;

	cout <<endl;
	*/
	//印出顏色
	if(G_Mastership_PL==COLOR_RED && G_Mastership_CP==COLOR_BLACK)
	cout <<"           玩家為紅棋 (大)  ←─────→  電腦為黑棋 (小)       "<<endl;
	else
	//s if(Mastership_PL==0 && Mastership_CP==1)
    cout <<"           玩家為黑棋 (小)  ←─────→  電腦為紅棋 (大)       "<<endl;
	
	cout <<endl;	 

	   
	  cout <<"┌───────────────────────────────┐"<<endl;
	  if(ROUND<10)
      cout <<"│"<<"                        Game  ROUND    "<<ROUND<<"                      "<<"│"<<endl;
	  if(ROUND>9)
	  cout <<"│"<<"                        Game  ROUND    "<<ROUND<<"                     "<<"│"<<endl;
      cout <<"├───┬───┬───┬───┬───┬───┬───┬───┤"<<endl;
	  cout <<"│  "<<"0 0"<<" │  "<<"0 1"<<" │  "<<"0 2"<<" │  "<<"0 3"<<" │ "<<"0 4"<<"  │ "<<"0 5"<<"  │ "<<"0 6"<<"  │  "<<"0 7"<<" │"<<endl;
	  cout <<"├───┼───┼───┼───┼───┼───┼───┼───┤"<<endl;
	  cout <<"│  "<<"0 8"<<" │  "<<"0 9"<<" │  "<<"1 0"<<" │  "<<"1 1"<<" │ "<<"1 2"<<"  │ "<<"1 3"<<"  │ "<<"1 4"<<"  │  "<<"1 5"<<" │"<<endl;
	  cout <<"├───┼───┼───┼───┼───┼───┼───┼───┤"<<endl;
	  cout <<"│  "<<"1 6"<<" │  "<<"1 7"<<" │  "<<"1 8"<<" │  "<<"1 9"<<" │ "<<"2 0"<<"  │ "<<"2 1"<<"  │ "<<"2 2"<<"  │  "<<"2 3"<<" │ "<<endl;
	  cout <<"├───┼───┼───┼───┼───┼───┼───┼───┤"<<endl;
	  cout <<"│  "<<"2 4"<<" │  "<<"2 5"<<" │  "<<"2 6"<<" │  "<<"2 7"<<" │ "<<"2 8"<<"  │ "<<"2 9"<<"  │ "<<"3 0"<<"  │  "<<"3 1"<<" │ "<<endl;
	  cout <<"└───┴───┴───┴───┴───┴───┴───┴───┘"<<endl;

	  cout<<endl;

//aaaaaaa


	  cout <<"┌───────────────────────────────┐"<<endl;
	  if(ROUND<10)
      cout <<"│"<<"                        Game  ROUND    "<<ROUND<<"                      "<<"│"<<endl;
	  if(ROUND>9)
	  cout <<"│"<<"                        Game  ROUND    "<<ROUND<<"                     "<<"│"<<endl;
      cout <<"├───┬───┬───┬───┬───┬───┬───┬───┤"<<endl;
	  cout <<"│  ";
	  SetColor(table_PieceColor[currentBoard[0]],0);
	  cout <<table_PieceName[currentBoard[0]];
	  SetColor();
	  cout <<" │  ";
	  SetColor(table_PieceColor[currentBoard[1]],0);
	  cout <<table_PieceName[currentBoard[1]];
	  SetColor();
	  cout <<" │  ";
	  SetColor(table_PieceColor[currentBoard[2]],0);
	  cout <<table_PieceName[currentBoard[2]];
	  SetColor();
	  cout <<" │  ";
	  SetColor(table_PieceColor[currentBoard[3]],0);
	  cout <<table_PieceName[currentBoard[3]];
	  SetColor();
	  cout <<" │ ";
	  SetColor(table_PieceColor[currentBoard[4]],0);
	  cout <<table_PieceName[currentBoard[4]];
	  SetColor();
	  cout <<"  │ ";
	  SetColor(table_PieceColor[currentBoard[5]],0);
	  cout <<table_PieceName[currentBoard[5]];
	  SetColor();
	  cout <<"  │ ";
	  SetColor(table_PieceColor[currentBoard[6]],0);
	  cout <<table_PieceName[currentBoard[6]];
	  SetColor();
	  cout <<"  │  ";
	  SetColor(table_PieceColor[currentBoard[7]],0);
	  cout <<table_PieceName[currentBoard[7]];
	  SetColor();
	  cout <<" │"<<endl;

	  cout <<"├───┼───┼───┼───┼───┼───┼───┼───┤"<<endl;
	  cout <<"│  ";
	  SetColor(table_PieceColor[currentBoard[8]],0);
	  cout <<table_PieceName[currentBoard[8]]; 
	  SetColor();
	  cout <<" │  ";
	  SetColor(table_PieceColor[currentBoard[9]],0);
	  cout <<table_PieceName[currentBoard[9]];
	  SetColor();
	  cout <<" │  ";
	  SetColor(table_PieceColor[currentBoard[10]],0);
	  cout <<table_PieceName[currentBoard[10]];
	  SetColor();
	  cout <<" │  ";
	  SetColor(table_PieceColor[currentBoard[11]],0);
	  cout <<table_PieceName[currentBoard[11]];
	  SetColor();
	  cout <<" │ ";
	  SetColor(table_PieceColor[currentBoard[12]],0);
	  cout <<table_PieceName[currentBoard[12]];
	  SetColor();
	  cout <<"  │ ";
	  SetColor(table_PieceColor[currentBoard[13]],0);
	  cout <<table_PieceName[currentBoard[13]];
	  SetColor();
	  cout <<"  │ ";
	  SetColor(table_PieceColor[currentBoard[14]],0);
	  cout <<table_PieceName[currentBoard[14]];
	  SetColor();
	  cout <<"  │  ";
	  SetColor(table_PieceColor[currentBoard[15]],0);
	  cout <<table_PieceName[currentBoard[15]];
	  SetColor();
	  cout <<" │"<<endl;

	  cout <<"├───┼───┼───┼───┼───┼───┼───┼───┤"<<endl;
	  cout <<"│  ";
	  SetColor(table_PieceColor[currentBoard[16]],0);
	  cout <<table_PieceName[currentBoard[16]];
	  SetColor();
	  cout <<" │  ";
	  SetColor(table_PieceColor[currentBoard[17]],0);
	  cout <<table_PieceName[currentBoard[17]];
	  SetColor();
	  cout <<" │  ";
	  SetColor(table_PieceColor[currentBoard[18]],0);
	  cout <<table_PieceName[currentBoard[18]];
	  SetColor();
	  cout <<" │  ";
	  SetColor(table_PieceColor[currentBoard[19]],0);
	  cout <<table_PieceName[currentBoard[19]];
	  SetColor();
	  cout <<" │ ";
	  SetColor(table_PieceColor[currentBoard[20]],0);
	  cout <<table_PieceName[currentBoard[20]];
	  SetColor();
	  cout <<"  │ ";
	  SetColor(table_PieceColor[currentBoard[21]],0);
	  cout <<table_PieceName[currentBoard[21]];
	  SetColor();
	  cout <<"  │ ";
	  SetColor(table_PieceColor[currentBoard[22]],0);
	  cout <<table_PieceName[currentBoard[22]];
	  SetColor();
	  cout <<"  │  ";
	  SetColor(table_PieceColor[currentBoard[23]],0);
	  cout <<table_PieceName[currentBoard[23]];
	  SetColor();
	  cout <<" │ "<<endl;

	  cout <<"├───┼───┼───┼───┼───┼───┼───┼───┤"<<endl;
	  cout <<"│  ";
      SetColor(table_PieceColor[currentBoard[24]],0);
	  cout <<table_PieceName[currentBoard[24]];
	  SetColor();
	  cout <<" │  ";
	  SetColor(table_PieceColor[currentBoard[25]],0);
	  cout <<table_PieceName[currentBoard[25]];
	  SetColor();
	  cout <<" │  ";
	  SetColor(table_PieceColor[currentBoard[26]],0);
	  cout <<table_PieceName[currentBoard[26]];
	  SetColor();
	  cout <<" │  ";
	  SetColor(table_PieceColor[currentBoard[27]],0);
	  cout <<table_PieceName[currentBoard[27]];
	  SetColor();
	  cout <<" │ ";
	  SetColor(table_PieceColor[currentBoard[28]],0);
	  cout <<table_PieceName[currentBoard[28]];
	  SetColor();
	  cout <<"  │ ";
	  SetColor(table_PieceColor[currentBoard[29]],0);
	  cout <<table_PieceName[currentBoard[29]];
	  SetColor();
	  cout <<"  │ ";
	  SetColor(table_PieceColor[currentBoard[30]],0);
	  cout <<table_PieceName[currentBoard[30]];
	  SetColor();
	  cout <<"  │  ";
	  SetColor(table_PieceColor[currentBoard[31]],0);
	  cout <<table_PieceName[currentBoard[31]];
	  SetColor();
	  cout <<" │ "<<endl;

	  cout <<"└───┴───┴───┴───┴───┴───┴───┴───┘"<<endl;


}

//determine the direction of from/to pair
int ex_Direction(int from, int to, int &distance) {
    // - -  to    - -
	// - -   |    - -
	// - -  from  - -  
	distance = 0;
	//上下吃子
	if( getX(from) == getX(to) ) {
		if (getY(from) > getY(to)) { 
			distance = getY(from) - getY(to);
			return DIR_UP;
		} else if (getY(from) < getY(to)) { 
			distance = getY(to) - getY(from);
			return DIR_DOWN;
		} else { 
			return DIR_SAME; 
		} 
	} 

	//snow???
	//左右吃子(如果distance是負值則往右)
	if( getY(from) == getY(to) ) {
		if (getX(from) > getX(to)) { 
			distance = getX(from) - getX(to);
			return DIR_RIGHT;
		} else if (getX(from) < getX(to)) { 
			distance = getX(to) - getX(from);
			return DIR_LEFT;
		} else { 
			return DIR_SAME; 
		} 
	}

	return DIR_UNKNOWN;
}


int canTurn(int from, int to) {

     if(currentBoard[from] == 64) 
	   return 1;
	 return 0;
}

int getColor(char c) {
	if (c >= 'a' && c <= 'z')
		return COLOR_BLACK;
	else if (c >= 'A' && c <= 'Z')
		return COLOR_RED;
	else return COLOR_EMPTY;
}

//特殊吃子判斷  from 吃 to   輪到Mastership回合
inline int canEat(int from, int to, int Mastership) {


	//吃棋不能超出棋盤範圍
	/*if (to < 0 || to > 31)
		return NO_ACTION;*/


	//snow
	if (getColor(currentBoard[from]) != Mastership || getColor(currentBoard[to]) != !Mastership)
		return NO_ACTION;
	/*
	//吃棋的棋子不能為未翻開的棋||無棋||吃棋位置不能是未翻開的棋子||吃棋位置不能無棋
	if (currentBoard[from] == 64 || currentBoard[from] == NULL || currentBoard[to] == 64 || currentBoard[to] == NULL)	return NO_ACTION;

	
	
	//如果非自己顏色的棋子則不能吃
	if (Mastership == 1 && currentBoard[from] >= 'a' && currentBoard[from] <= 'z' &&
		currentBoard[to] >= 'A' && currentBoard[to] <= 'Z')
		return NO_ACTION;

	if(Mastership==0 && currentBoard[from]>='A' && currentBoard[from] <='Z' && 
		currentBoard[to]>='a' && currentBoard[to]<='z')
		return NO_ACTION;
	


	
	//同色不能互吃
	if(currentBoard[from]>='A' && currentBoard[from]<='Z' && currentBoard[to]>='A' && currentBoard[to]<='Z')
	   return NO_ACTION;

	if(currentBoard[from]>='a' && currentBoard[from]<='z' && currentBoard[to]>='a' && currentBoard[to]<='z')
	   return NO_ACTION;
	*/


	

	int distance=0;
	int dir = ex_Direction(from, to, distance);

	if (!(dir==DIR_UP || dir==DIR_DOWN || dir==DIR_LEFT || dir==DIR_RIGHT))
		return NO_ACTION;
	
	if(distance==1 || distance==-1) {
		//special case
		if (currentBoard[from] == 'C' || currentBoard[from] == 'c')
			return NO_ACTION;

		if (currentBoard[from] == 'P' && currentBoard[to] == 'c')
			return NO_ACTION;

		if (currentBoard[from] == 'p' && currentBoard[to] == 'C')
			return NO_ACTION;

		if (currentBoard[from] == 'K' && currentBoard[to] == 'p')
			return NO_ACTION;

		if (currentBoard[from] == 'k' && currentBoard[to] == 'P')
			return NO_ACTION;

		if (currentBoard[from] == 'P' && currentBoard[to] == 'k')
			return ACTION;

		if (currentBoard[from] == 'p' && currentBoard[to] == 'K')
			return ACTION;

		//normal case
		if(table1[currentBoard[from]] >= table1[currentBoard[to]])
			return ACTION;
		else
			return NO_ACTION;
		
	} else { //Cannon

		//only for 'C' / 'c'
		if (currentBoard[from] != 'C' && currentBoard[from] != 'c')
			return NO_ACTION;

		int mountain= 0;
		switch( ex_Direction(from, to, distance) ) {

		case DIR_UP:
			
			//snow BUG
			//for(int y=getY(from)-1; y <= getY(to)+1; y++) {
			for (int y = getY(to) + 1; y <= getY(from) - 1; y++) {
				if(currentBoard[getPos(getX(from), y)] != NULL)
					mountain ++;
			}

			if(mountain == 1)
				return ACTION;
			//snow ??
			break;

			
		case DIR_DOWN:

			for(int y=getY(from)+1; y <= getY(to)-1; y++) {
				if(currentBoard[getPos(getX(from), y)] != NULL)
					mountain ++;
			}

			if(mountain == 1)
				return ACTION;
			break;

			//有問題  getX =%   getY=/  getPos=Y*8 + X
		case DIR_LEFT:
			//          10%8=2              13%8=5
			//                  3-4
			for(int x=getX(from)+1; x <= getX(to)-1; x++) {  
				//                     2       1
				if(currentBoard[getPos(x, getY(from))] != NULL)
				    mountain ++;
			}
			  
			if(mountain == 1)
				return ACTION;
			break;
			
		case DIR_RIGHT:

			for(int x=getX(from)-1; x >= getX(to)+1; x--) {
				if(currentBoard[getPos(x, getY(from))] != NULL)
				    mountain ++;
			}

			if(mountain == 1)
				return ACTION;
			break;
			

		default:
			return NO_ACTION;
			
		}
	 }//swhich
	return 0;
}//canEat


int Player(char startboard[], char CMD[], int Mastership) {  //翻吃棋走步 指令

	int eat1,eat2;

	eat1 = (CMD[0]-48)*10+(CMD[1]-48);   
	eat2 = (CMD[2]-48)*10+(CMD[3]-48); 

	//snow BUG
	//翻棋
	if(canTurn(eat1, eat2)) {
	   currentBoard[eat1] = startboard[eat1];
	   //startboard[(CMD[0]-48)*10+(CMD[1]-48)] = NULL;
	}

	//吃棋
	/* else if(canEat(eat1, eat2 ,Mastership)) {
		  currentBoard[(CMD[2]-48)*10+(CMD[3]-48)] = currentBoard[(CMD[0]-48)*10+(CMD[1]-48)];
		  currentBoard[(CMD[0]-48)*10+(CMD[1]-48)] = NULL;
		  startboard[(CMD[2]-48)*10+(CMD[3]-48)] = startboard[(CMD[0]-48)*10+(CMD[1]-48)];
		  startboard[(CMD[0]-48)*10+(CMD[1]-48)] = NULL;
	}*/
	 else if (canEat(eat1, eat2, Mastership)) {
		 currentBoard[eat2] = currentBoard[eat1];
		 currentBoard[eat1] = NULL;
		 startboard[eat2] = startboard[eat1];
		 startboard[eat1] = NULL;
	 }

	 
	//移動
	else if(canMove(eat1, eat2, Mastership)) {
		currentBoard[(CMD[2]-48)*10+(CMD[3]-48)] = currentBoard[(CMD[0]-48)*10+(CMD[1]-48)];
		currentBoard[(CMD[0]-48)*10+(CMD[1]-48)] = NULL;
		startboard[(CMD[2]-48)*10+(CMD[3]-48)] = startboard[(CMD[0]-48)*10+(CMD[1]-48)];
		startboard[(CMD[0]-48)*10+(CMD[1]-48)] = NULL;
	}

	//輸入錯誤
	else {
		printf("illegal move!!\n");
		system("pause");
		return 0;
	}
}



int eval_Score(int Mastership, char currentBoard[32]) {
 
 
	int sc_t=0, sc_r=0, sc_b=0;  //分數

	for(int i=0;i<32;i++) {
		  //計算盤面紅棋分數
		  if( getColor(currentBoard[i]) == COLOR_RED)   //first表示map裡面的key=棋子
			 sc_r+=table2.find( currentBoard[i] )->second;   //second表示map裡面的value=棋子的分數
		  else if( getColor(currentBoard[i]) == COLOR_BLACK)   //first表示map裡面的key=棋子
			 sc_b+=table2.find(currentBoard[i])->second;   
	} 


	////for(int i=0;i<32;i++) {
	////	for(int j=0;j<7;j++) {
	////	  //計算盤面紅棋分數
	////	  if(currentBoard[i] == table2.find(str1[j])->first)   //first表示map裡面的key=棋子
	////		 sc_r+=table2.find(str1[j])->second;   //second表示map裡面的value=棋子的分數
	////	  //計算盤面黑棋分數
	////	  //snow
	////	  else if (currentBoard[i] == table2.find(str2[j])->first)
	////	//if(currentBoard[i] == table2.find(str2[j])->first)
	////		 sc_b+=table2.find(str2[j])->second;   
	////	}// j
	////  }// i

	//COLOR_RED代表大寫  COLOR_BLACK代表小寫
	if(Mastership==COLOR_RED) {
	  return sc_r - sc_b + C_score;  //盤面分數紅棋-黑棋  sc_t盤面總合分數  sc_r紅棋分數  sr_b黑棋分數 C_score 炮&包的分數
	}//if

	if(Mastership==COLOR_BLACK) {
	  return sc_b - sc_r + C_score;  //盤面分數紅棋-黑棋  sc_t盤面總合分數  sc_r紅棋分數  sr_b黑棋分數 //-100, +100, +200 
	}

	return 0;
}

//snow ?
//解碼
void Decoding(string code, ChessMove *amove) {

  //切割字串

  char * cstr = new char [code.length()+1];
  strcpy_s (cstr, code.length()+1, code.c_str());
  string temp1[4];

  char * p = std::strtok (cstr,",");
  int i=0;
  while (i<4)
  {
	temp1[i]=p;
    p = strtok(NULL,",");
	i++;
  }
  delete[] cstr;

  //將字串轉為整數
  
    i=0;
	if(i==2)  //路徑不需除去開頭
		amove->level = temp1[i].back()-48;

	else if(temp1[i].front()==0)  //如果字串開頭為0則去除
		amove->level =temp1[i].back()-48;
	  
	else { //如果開頭非0則轉換
		amove->level = (temp1[i].front()-48)*10;
		amove->level = amove->level + temp1[i].back()-48;
	}
	   
	i=1;
	if(i==2)  //路徑不需除去開頭
	amove->location = temp1[i].back()-48;

	else if(temp1[i].front()==0)  //如果字串開頭為0則去除
		amove->location =temp1[i].back()-48;
	  
	else { //如果開頭非0則轉換
	amove->location  = (temp1[i].front()-48)*10;
	amove->location =amove->location+temp1[i].back()-48;
	}

	i=2;
	if(i==2)  //路徑不需除去開頭
	amove->path = temp1[i].back()-48;

	else if(temp1[i].front()==0)  //如果字串開頭為0則去除
		amove->path =temp1[i].back()-48;
	  
	else { //如果開頭非0則轉換
	amove->path  = (temp1[i].front()-48)*10;
	amove->path =amove->path+temp1[i].back()-48;
	}

	i=3;
	if(i==2)  //路徑不需除去開頭
	amove->c_loc = temp1[i].back()-48;

	else if(temp1[i].front()==0)  //如果字串開頭為0則去除
		amove->c_loc =temp1[i].back()-48;
	  
	else { //如果開頭非0則轉換
	amove->c_loc  = (temp1[i].front()-48)*10;
	amove->c_loc =amove->c_loc+temp1[i].back()-48;
	}


	//test print
	//cout<<temp1[0] << ", "<<temp1[1] << ", " << temp1[2] << endl;
	//system("pause");


}


//解碼
void Decoding_1(string code) {

  //切割字串

  char * cstr = new char [code.length()+1];
  std::strcpy (cstr, code.c_str());
  string temp1[3];

  char * p = std::strtok (cstr,",");
  int i=0;
  while (i<3)
  {
	temp1[i]=p;
    p = strtok(NULL,",");
	i++;
  }
  delete[] cstr;

  //將字串轉為整數
  for(int i=0;i<3;i++) {
	  if(i==2)  //路徑不需除去開頭
	    temp[i]=temp1[i].back()-48;

	  else if(temp1[i].front()==0)  //如果字串開頭為0則去除
		 temp[i]=temp1[i].back()-48;
	  
	  else { //如果開頭非0則轉換
		temp[i] = (temp1[i].front()-48)*10;
		temp[i]=temp[i]+temp1[i].back()-48;
	  }
	   
  }

    //for(int i=0;i<3;i++)
	//cout << temp[i] << endl;
   //system("pause");
}

//編碼
string Transcoding(int level, int location, int path, int c_loc) {

	string Combination="";

	Combination+=level/10+48;
	Combination+=level%10+48;
	Combination+=",";
	Combination+=location/10+48;
	Combination+=location%10+48;
	Combination+=",";
	Combination+=path+48;
	Combination+=",";
	Combination+=c_loc/10+48;
	Combination+=c_loc%10+48;

	//cout <<Combination <<endl;
	//system("pause");
	return Combination;
}

int generate_All_Moves(int level, HistoryPath1& list1) {

	   int turn=0;

//????????????????????????????????????????????????????????????
	   if(level%2==0)      
		   Rotation = G_Mastership_PL;   //next level is player-level

	   if(level%2==1)      
		   Rotation = G_Mastership_CP;   //next level is computer-level

		for(int site=0;site<32;site++)  {//位置
		  for(int path=0;path<4;path++) {//4個方向

			  //電腦包&炮特殊吃棋
			  if(currentBoard[site]=='C' || currentBoard[site]=='c') {

				  //x軸確定是否有可吃的棋
				  

				  //左
				  if(path==2) {
					for(int i=2;i<=getX(site);i++)
				      if(canEat(site, site-i, Rotation)) {
						list1.push_back(Transcoding(level, site, path, site-i));  //先確定炮&包是否可以往這個方向吃
						i=8; //跳出迴圈
					}
				  }

				  //右
				  if(path==3) {
					 for(int i=2;i<=7-getX(site);i++)
						if(canEat(site, site+i, Rotation)) {
						  list1.push_back(Transcoding(level, site, path, site+i));  //先確定炮&包是否可以往這個方向吃
						  i=8; //跳出迴圈
					    }
				  }

				  //y軸確定是否有可吃的棋

				  //上  18->2
				  if(path==0) {
					for(int i=2;i<=getY(site);i++)
				      if(canEat(site, site-(i*8), Rotation)) {
						list1.push_back(Transcoding(level, site, path, site-(i*8)));  //先確定炮&包是否可以往這個方向吃
						i=8; //跳出迴圈
					}
				  }

				  //下 2->18
				  if(path==1) {
					for(int i=2;i<=3-getY(site);i++)
				      if(canEat(site, site+(i*8), Rotation)) {
						list1.push_back(Transcoding(level, site, path, site+(i*8)));  //先確定炮&包是否可以往這個方向吃
						i=8; //跳出迴圈
					}
				  }

			  } //if C & c
			  

			//eat & move -> from, to, Mastership
			//         from               to                                  from             to
			if(canEat(site, site+temp[path], Rotation) || canMove(site, site+temp[path], Rotation ) && path!=4) {//如果能夠吃棋或空步
			  turn=1;
			  chess=currentBoard[site+temp[path]]; //記錄被吃掉的棋子

			  //list1 電腦走過的路徑  list2 玩家走過的路徑
			  //i 深度 j位置 k方向 push list1
			  /*if(level==1) {  //當level=1時*/
			    //std::cout << currentBoard[site] << "(" << site << ")" << " eat " << currentBoard[site+temp[path]] << "(" << site+temp[path] << ")" << endl;  //檢查用

			    //將多餘&重複的路徑拿掉
		         //HistoryPath1::reverse_iterator ir;      
				 /*
                 for  (ir  = list1.rbegin(); ir != list1.rend();ir ++ )  {  
				   //if(Transcoding(level, site, path)==*ir) {
			       //  continue;
		         //  }

				   list1.push_back(Transcoding(level, site, path));  //儲存電腦模擬過的所有路徑至list1
				   list3.push_back(Transcoding(level, site, path));  //儲存電腦模擬過的所有路徑至list3(全部路徑)
				 }
		         */

			  


			   list1.push_back(Transcoding(level, site, path, 99));  //儲存電腦模擬過的所有路徑至list1

			  //當重複2次以後
			  
			     HistoryPath2::reverse_iterator ir;      

				 Repeat=0;
				 if(list2.size()%2==0 && list2.size()!=0) {
                   for  (ir  = list2.rbegin(); ir != list2.rend();ir ++ )  {  

					 string compare1= *ir ;
					 string compare2=Transcoding(level, site, path, 99);
				  
					 if(compare1==compare2 && Repeat==0) {
					   Repeat=1;
					   list1.pop_back();  //刪除路徑
					 }

				   }//for
				}//if

				 //清除list2
				 if(list2.size()>=10) 
				   list2.clear();
				 
				 //list3.push_back(Transcoding(level, site, path));  //儲存電腦模擬過的所有路徑至list3(全部路徑)
			    
			 
			   // currentBoard[site+temp[path]]=currentBoard[site];  //位置取代
		       // currentBoard[site]=NULL;  //位置清空
			    
				//std::system("pause");  //檢查用

			    //printBoard();
			  
			  //i 深度 j位置 k方向 pop list1
			  //將模擬盤面退回至上一手
			  //cout << "list1 = " << list1.size() << endl;  //印出list1路徑數量
			  //cout << "list1 = " << list1.back() << endl;  //印出list1 最後一個路徑
			  //std::cout << currentBoard[site+temp[path]] << "(" << site+temp[path] << ")" << " pop" << currentBoard[site] << "(" << site << ")" << " Recovery " << chess << endl;
			  //system("pause");
			  //currentBoard[site]=currentBoard[site+temp[path]];//回到上一手
			  //currentBoard[site+temp[path]]=chess;//被吃掉棋子回復
			  //std::system("pause");  //檢查用
			  //printBoard();
			  
			  }//caneat & canmove

			//如果location上是包or炮
			//else if()

			//如果不能吃棋和空步就翻棋
			/*
			if(turn==0 && canTurn(site, 0) && path==4) {
				list1.push_back(Transcoding(level, site, 5));  //儲存電腦模擬過的所有路徑至list1
				list3.push_back(Transcoding(level, site, 5));  //儲存電腦模擬過的所有路徑至list3(全部路徑)
			}
			  */



			/*}//if*/

		  }//path 路徑
		}//location 位置
		/*
		 //if(nodes>300) {
		 HistoryPath1::reverse_iterator ir;      

        for  (ir  = list1.rbegin(); ir != list1.rend();ir ++ )  {  
		cout   << * ir  <<  "  " <<  endl;  
		}

		system("pause");
		// }
	*/
		return 1;
}


//level=1 location=0;
//
//return: bestMove's value, if no move can be found, return NULL_NODE_VALUE
//
//output: p_move=best-move

string int2str(int &i) {  //int 轉 string
  string s;
  stringstream ss(s);
  ss << i;
  return ss.str();
}


inline bool isMinLevel(int level) {
  return (level % 2 != 0) ; // //=================== Player , min-level
}

inline bool isMaxLevel(int level) {
  return (level % 2 == 0) ; // //=================== Computer, max-level
}

int Computer(int parent_treeNodeId, string* moveString, int level, ChessMove *p_move, int alpha, int beta) {
	
	/////////////////////////////////////////////////////////////
	++G_treeNodeId;  //id of this node  
	int myTreeNodeId = G_treeNodeId;

	string a,b,c;
	a=int2str(alpha);
	b=int2str(beta);
	
	
	if(parent_treeNodeId > 0) { //非根節點
		/*
		strGameTreeRep = strGameTreeRep + to_string(myTreeNodeId);
		strGameTreeRep = strGameTreeRep + "[ label=\"";
		strGameTreeRep = strGameTreeRep + *moveString;
		strGameTreeRep = strGameTreeRep + " ; alpha= ";
		strGameTreeRep = strGameTreeRep + a;
		strGameTreeRep = strGameTreeRep + " ; beta= ";
		strGameTreeRep = strGameTreeRep + b;
		strGameTreeRep = strGameTreeRep + " ; score= ";
		strGameTreeRep = strGameTreeRep + c;
		strGameTreeRep = strGameTreeRep + "\"];";
		*/
		strGameTreeRep = strGameTreeRep + to_string(parent_treeNodeId);
		strGameTreeRep = strGameTreeRep + "--";
		strGameTreeRep = strGameTreeRep + to_string(myTreeNodeId);
		strGameTreeRep = strGameTreeRep + ";\n";
		
	}
	
	/////////////////////////////////////////////////////////////

	////int Mastership = G_Mastership_PL;
 ////   if(level%2==1)
	////  Mastership = G_Mastership_CP; //computer

	int Mastership;
    if(isMaxLevel(level))
	  Mastership = G_Mastership_CP; //computer
	else 
	  Mastership = G_Mastership_PL;

	HistoryPath1 list1; //紀錄電腦路徑(push-pop use) (list of string)
	int pass=0;
	int node=0;


	//622

	//做alpha-beta cut
	if(beta<=alpha) { 

		int sscore;

	    if(isMaxLevel(level)) //Computer, up level=Player 
			sscore = MAX_NODE_VALUE;
	    else 
			sscore = MIN_NODE_VALUE; 

		c=int2str(sscore);

			//622
			if(parent_treeNodeId > 0) {
	     	strGameTreeRep = strGameTreeRep + to_string(myTreeNodeId);

			if(isMaxLevel(level))
				strGameTreeRep = strGameTreeRep + "[ label=\"";
			else
				strGameTreeRep = strGameTreeRep + "[ shape=box label=\"";

		    strGameTreeRep = strGameTreeRep + *moveString;
		    strGameTreeRep = strGameTreeRep + "\\n alpha= ";
	    	strGameTreeRep = strGameTreeRep + a;
    		strGameTreeRep = strGameTreeRep + "\\n beta= ";
	    	strGameTreeRep = strGameTreeRep + b;
			strGameTreeRep = strGameTreeRep + "\\n score= ";
	    	strGameTreeRep = strGameTreeRep + c;
			strGameTreeRep = strGameTreeRep + "\\n cut";
			strGameTreeRep = strGameTreeRep + "\"";
            strGameTreeRep = strGameTreeRep + "color=red";
			strGameTreeRep = strGameTreeRep + "];\n";
	        }
			//622

			return sscore;
	 }

	//到達樹底層，取評估函數值
	//terminal condition
	if(level >= MAX_DEPTH-1){
	    
    //622
		int sscore = eval_Score(Mastership, currentBoard);
		c=int2str(sscore);
		
		//622
		if(parent_treeNodeId > 0) {

		strGameTreeRep = strGameTreeRep + to_string(myTreeNodeId);
			
		if(isMaxLevel(level))
			strGameTreeRep = strGameTreeRep + "[ label=\"";
		else
			strGameTreeRep = strGameTreeRep + "[ shape=box label=\"";

		strGameTreeRep = strGameTreeRep + *moveString;
		strGameTreeRep = strGameTreeRep + "\\n alpha= ";
		strGameTreeRep = strGameTreeRep + a;
		strGameTreeRep = strGameTreeRep + "\\n beta= ";
		strGameTreeRep = strGameTreeRep + b;
		strGameTreeRep = strGameTreeRep + "\\n score= ";
		strGameTreeRep = strGameTreeRep + c;
		strGameTreeRep = strGameTreeRep + "\"];\n";
		}
		
		//622
		return sscore; 
	}
	   
	//if(  ) {
    //***************************
/* 
	cout<<"-----------------------------------------------------------"<<endl;
	printBoard();
    cout<<"alpha = "<<alpha<<"  ;   beta  =  "<<beta<<endl;
	cout << "level = " << level <<endl;
	cout<<"score = "<<  eval_Score(Mastership, currentBoard) <<endl;
	system("pause");
*/
	//***************************
	//}




	//-----------翻棋----------------
	//1.盤面沒翻棋則翻棋

		//==========================All Moves=============================
		generate_All_Moves(level, list1); //stored in list1
/*最多 150種 ChessMove
我們要計算這150種ChessMove的分數，得出MAX SCORED
*/
		int moveFlag = 0;
		int bestValue = 0;
		ChessMove *amove = new ChessMove();
/* 0 1 2 3 4 5 6 7
   8 9101112131415
      ...
	            31 
*/
/*class ChessMove {
	int level樹的level
	int location 起始位置0-31, path上下左右0,1,2,3, c_loc 炮??;
	char to_piece; //被吃子??
	int cannon_dist; //炮??
*/
/*
Encoding, Transcoding
level + "," + location + "," + path + "," c_loc + "," to_piece + "," + cannon_dist
*/
/*
Decoding
10,20,30,40,50,60,70
level + "," + location + "," + path + "," c_loc + "," to_piece + "," + cannon_dist
*/

		//if(level % 2 != 0) { //Player, min-level
		if(isMinLevel(level)) {
			bestValue = MAX_NODE_VALUE;  
		}
		else { //Computer, max-level
			bestValue = MIN_NODE_VALUE;
		}

		//==========================Tree searching ad branching=============================
		//對所有ChessMove
		for(int times=list1.size()-1; times>=0; times--) { //節點數

			string s1 = list1.back();

			//解碼後存入amove
			Decoding(s1, amove);  //解碼list1的字串轉成int
			list1.pop_back(); //清除list1尾端數據
			nodes++;

			//下一手
			C_score=0; //包&炮動態分數
			makeMove(currentBoard, amove); //走amove


			//計算走amove之分數
			//tempMove /////////////////////////////////// 可以刪除
			ChessMove *tempMove = new ChessMove();			
			int value = Computer(myTreeNodeId, &s1, level+1, tempMove, alpha, beta);

			//取min或是max
			//p_move儲存最後一個位置

//			if(level % 2 != 0) { //=================== Player , min-level
			if(isMinLevel(level)) {
				if(value <= bestValue) {//取min
					moveFlag = 1;
					bestValue = value;
					p_move->copy(amove); 
				}

                //Set beta
                beta = min(value, beta);
			}
			else {               //=================== Computer, max-level
				if(value >= bestValue) {//取max
					moveFlag = 1;
					bestValue = value;
					p_move->copy(amove); 
				}

				//Set alpha
				alpha = max(value, alpha);
				
			}



			//&&
			//印出所有步數
			//cout << "move=" << s1 << ",value=" << bestValue << "\n";
			

			//上一手
			unMakeMove(amove, Mastership);

		}


		if(moveFlag == 0){ //no move is available
			//bestValue = NULL_NODE_VALUE;
			bestValue = eval_Score(Mastership, currentBoard);
		}

		//沒有alpha-beta cut, 不是樹葉
		//622
		c = int2str(bestValue);

		strGameTreeRep = strGameTreeRep + to_string(myTreeNodeId);

		if(isMaxLevel(level))
			strGameTreeRep = strGameTreeRep + "[ label=\"";
		else
			strGameTreeRep = strGameTreeRep + "[ shape=box label=\"";

		strGameTreeRep = strGameTreeRep + to_string(myTreeNodeId) + "_";
		strGameTreeRep = strGameTreeRep + *moveString;
		strGameTreeRep = strGameTreeRep + "\\n alpha= ";
		strGameTreeRep = strGameTreeRep + a;
		strGameTreeRep = strGameTreeRep + "\\n beta= ";
		strGameTreeRep = strGameTreeRep + b;
		strGameTreeRep = strGameTreeRep + "\\n score= ";
		strGameTreeRep = strGameTreeRep + c;
		strGameTreeRep = strGameTreeRep + "\"];\n";
		 
		//622

		return bestValue;
}


int run=0;  //控制
int Tie=0, check_turn; //控制回合

char Game(int end, char startboard[], int order) {

	int chess_in=0, chess_out=0;  //儲存下棋前後盤面是否有差異
	//printBoard();
	
	//=====================平手機制=======================//
	//記算盤面是否有變化(前)
	for(int i=0; i<32 ;i++) {
		if(currentBoard[i]>64 && currentBoard[i]<123) {
		  chess_in++;
		}
	}
	//=====================平手機制=======================//


	int error=1;
	//-----------------玩家------------------------
	/////////////////////////////////////////////////////////////////////////aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa
	//////if(order==1 || order==2) {
	//////	
	//////	//玩家下棋寫死  
	//////	for(int i=run; i<32; i++) {  //有翻棋就直接翻棋不走棋
	//////	  if(canTurn(i, 0) && currentBoard[i]!=NULL ) {
	//////		  currentBoard[i] = startboard[i];
	//////	      printBoard();
	//////	      printf("Player turn %c[%d] \n",startboard[i],i);
	//////		  system("pause");
	//////		  run++;
	//////	      break; //跳離迴圈
	//////	  }//if
	//////	}//for
	//////}
	 
	if(order==1 || order==2) {
		 for(int i=0;i<10000;i++) {

		   char CMD[5];
	       for(int i=0;i<5;i++)
	       CMD[i]=NULL;

		    if(error==1) {

				  printBoard();

 	          printf("Command:");
	          scanf("%s",&CMD);
			  //received = server_recv(local_buffer, connectfd, isWebSocket);
	          error = Player(startboard, CMD, G_Mastership_PL);

			  if(error==0) 
			    error=1;
			  else 
			    i=10001;
			}
		 }

		  ROUND++;
		 //1=紅色(大寫) 0=黑色(小寫)
		 if(color==0) {
			 for(int i=0;i<32;i++) {
			   if(currentBoard[i]>64 && currentBoard[i]<91) {
			     G_Mastership_PL=COLOR_RED;
				 G_Mastership_CP=COLOR_BLACK;
			   }//if

			   if(currentBoard[i]>96 && currentBoard[i]<123) {
				 G_Mastership_PL=COLOR_BLACK;
			     G_Mastership_CP=COLOR_RED;
			   }//if
			 }//for
			 color=1;
		 }//if color
	}//if order
	 
	 printBoard();
	 system("pause");


    //-----------------電腦------------------------f
	   
	  ChessMove* move = new ChessMove();
	  int score;
	  int check=0;

	  //////////////////////////////////////////////////
	  strGameTreeRep = "";
	  G_treeNodeId = 0;
	  //////////////////////////////////////////////////


	  //複製當前盤面
	  for(int i=0;i<32;i++)
		tempBoard[i] = currentBoard[i];
		
	  string s_move="";
	  score = Computer(0, &s_move, 1, move, INT_MIN, INT_MAX); //1=min-level, player-level
	  SCORE = score;
	  //printf("%d",score);
	  //system("pause");
	  //makeMove(currentBoard, move);

	  //////////////////////////////////////////////////
	  cout << "===========================================\n";
	  cout << strGameTreeRep << "\n";

		fstream file;      //宣告fstream物件

        file.open("D:\\Reader.txt", ios::out | ios::trunc);

        //開啟檔案為輸出狀態，若檔案已存在則清除檔案內容重新寫入

        file.write(strGameTreeRep.c_str(), strlen(strGameTreeRep.c_str()));   //將str寫入檔案

        file.close();       //關閉檔案

	  //////////////////////////////////////////////////

	  int Turn_Chess=0, Turn_location=0;
	  
	  //cout << "nodes=" << nodes <<endl;
	  //system("pause");




	  //=========================避免重複吃棋==========================//

	  //避免重複棋
	  
	    if(TIMES==GO_CHESS) TIMES=0;  //當重複走步存滿後重新儲存

		 if(check==1) {
		   REPEAT[TIMES].level    = move->level;
		   REPEAT[TIMES].location = move->location;
		   REPEAT[TIMES].path     = move->path;
		   TIMES++;
		 }
		 
	  //如果走步再重複走棋的陣列中，則換其他方法走棋
	  for(int j=0; j<GO_CHESS; j++) {
	    if( REPEAT[j].location == move->location && REPEAT[j].path == move->path && REPEAT[TIMES].level!=0 && REPEAT[TIMES].location!=0 && REPEAT[TIMES].path!=0)
			check=0;
	  }

	  //=========================避免重複吃棋==========================//

	  //確定是電腦的棋子才能下
	  if(G_Mastership_CP==1 &&  tempBoard[move->location]>='A' && tempBoard[move->location]<='Z')
	     check=1;
	  if(G_Mastership_CP==0 &&  tempBoard[move->location]>='a' && tempBoard[move->location]<='z')
	     check=1;

	  //如果在最大深度時且確定是電腦的棋子就下棋
	  if(move->level>0 && check==1 && move->c_loc==99) {
		//printBoard();
		cout << "Computer chess " <<  tempBoard[move->location]  << "("<< move->location  <<")->" <<  tempBoard[move->location+temp[move->path]] << "("<< move->location+temp[move->path] << ")" << endl;
	    //system("pause");
		list2.push_back(Transcoding(move->level, move->location, move->path, move->c_loc));
	    tempBoard[move->location+temp[move->path]] = tempBoard[move->location]; 
	    tempBoard[move->location]=NULL;
	    startboard[move->location+temp[move->path]] = startboard[move->location];
	    startboard[move->location]=NULL;
	   
	     Turn_Chess=1;

		  //恢復當前盤面
	  for(int i=0;i<32;i++)
		currentBoard[i] = tempBoard[i] ;

		 //printBoard();
		
	  }//if 

	  //如果在最大深度時且確定是電腦的棋子就下棋(包&炮)
	  if(move->level>0 && check==1 && move->c_loc!=99) {
		//printBoard();
		cout << "Computer chess " <<  tempBoard[move->location]  << "("<< move->location  <<")->" <<  tempBoard[move->c_loc] << "("<< move->c_loc << ")" << endl;
	    //system("pause");
		list2.push_back(Transcoding(move->level, move->location, move->path, move->c_loc));
	    tempBoard[move->c_loc] = tempBoard[move->location]; 
	    tempBoard[move->location]=NULL;
	    startboard[move->c_loc] = startboard[move->location];
	    startboard[move->location]=NULL;
	  
	     Turn_Chess=1;

	  //恢復當前盤面
	  for(int i=0;i<32;i++)
		currentBoard[i] = tempBoard[i] ;

		 //printBoard();
		
	  }//if 



	   //如果沒吃棋或空步則翻棋
	  if(Turn_Chess==0) {

		while(Turn_Chess!=-2) {
		  srand(time(NULL));
		 Turn_location=rand()%32;
		  
		  if(tempBoard[Turn_location]!=64) {
			  continue;
		  }

		  else {
			  Turn_Chess=-2;
			  tempBoard[Turn_location] = startboard[Turn_location];
		      startboard[Turn_location]=NULL;
		  }
		}//while

		 //恢復當前盤面
	  for(int i=0;i<32;i++)
		currentBoard[i] = tempBoard[i] ;

		printBoard();
		cout << "Computer turn " << tempBoard[Turn_location] << "(" << Turn_location << ")" << endl;
		//system("pause");
	  }

	  

	

	    printf("get score = %d\n",score);
	    system("pause");



	    //1=紅色(大寫) 0=黑色(小寫)
		 if(color==0) {
		   for(int i=0;i<32;i++) {
		     if(currentBoard[i]>64 && currentBoard[i]<91) {
			   G_Mastership_PL=COLOR_BLACK;
			   G_Mastership_CP=COLOR_RED;
			 }//if
			 if(currentBoard[i]>96 && currentBoard[i]<123) {
			   G_Mastership_PL=COLOR_RED;
			   G_Mastership_CP=COLOR_BLACK;
			 }//if
		   }//for
		   color=1;
		 }//if color
	  ROUND++;
	order=1;

	//=====================平手機制=======================//
	for(int i=0; i<32 ;i++) {
		if(currentBoard[i]>64 && currentBoard[i]<123) {
		  chess_out++;
		}
	}

	//平手機制(如果出手超過40回合且沒吃棋且沒棋可翻則平手)
	check_turn=0;
	for(int i=0; i<32; i++) {  
		if( currentBoard[i]==64) {
			check_turn=1;
		}
	  }

	if(chess_in==chess_out && check_turn==0) {
		  Tie++;
	  }

	if(Tie==40 && chess_in==chess_out && check_turn==0) {  //如果經過40回合，棋子數量仍然沒改變，則視為平手
	  //printf("Tie!!\n");
	  //system("pause");
	  return 'T';
	  
	}
	//=====================平手機制=======================//


	//--------------------------------------------
	end = End(end,currentBoard);
	//Game(end, startboard, order);

	if(end==2) return Game(end, startboard, order);
	else {
		if(end==1) return 'R';
		if(end==0) return 'B';
		//system("pause");
	}
}


int _tmain(int argc, _TCHAR* argv[])
{
	//棋子ID
	table_piece_id[NULL] = 0;
	table_piece_id[' '] = 0;
	table_piece_id['n'] = 0;
	table_piece_id['N'] = 0;

	table_piece_id['K'] = 1;
	table_piece_id['A'] = 2;
	table_piece_id['E'] = 3;
	table_piece_id['R'] = 4;
	table_piece_id['H'] = 5;
	table_piece_id['C'] = 6;
	table_piece_id['P'] = 7;
	table_piece_id['k'] = 8;
	table_piece_id['a'] = 9;
	table_piece_id['e'] = 10;
	table_piece_id['r'] = 11;
	table_piece_id['h'] = 12;
	table_piece_id['c'] = 13;
	table_piece_id['p'] = 14;
	table_piece_id['u'] = 15;
	table_piece_id['U'] = 15;
	table_piece_id['@'] = 15;
 
	 

	table1['C'] = 0;    table1['c'] = 0;
	table1['P'] = 1;    table1['p'] = 1;
	table1['H'] = 2;    table1['h'] = 2;
	table1['R'] = 3;    table1['r'] = 3;
	table1['E'] = 4;    table1['e'] = 4;
	table1['A'] = 5;    table1['a'] = 5;
	table1['K'] = 6;    table1['k'] = 6;


//aAAAAAAAAAAAAAAAAAAAAAAA

table_PieceName['@'] = " @ ";    table_PieceName[0]   = "   ";
table_PieceName['P'] = " 兵";    table_PieceName['p'] = " 卒";
table_PieceName['C'] = " 炮";    table_PieceName['c'] = " 包";
table_PieceName['H'] = " 傌";    table_PieceName['h'] = " 馬";
table_PieceName['R'] = " 陣";    table_PieceName['r'] = " 車";
table_PieceName['E'] = " 相";    table_PieceName['e'] = " 象";
table_PieceName['A'] = " 仕";    table_PieceName['a'] = " 士";
table_PieceName['K'] = " 帥";    table_PieceName['k'] = " 將";

table_PieceColor['@'] = 7;
table_PieceColor['P'] = 12;    table_PieceColor['p'] = 15;
table_PieceColor['C'] = 12;    table_PieceColor['c'] = 15;
table_PieceColor['H'] = 12;    table_PieceColor['h'] = 15;
table_PieceColor['R'] = 12;    table_PieceColor['r'] = 15;
table_PieceColor['E'] = 12;    table_PieceColor['e'] = 15;
table_PieceColor['A'] = 12;    table_PieceColor['a'] = 15;
table_PieceColor['K'] = 12;    table_PieceColor['k'] = 15;

//棋子大小
table1['C'] = 0;    table1['c'] = 0;
table1['P'] = 1;    table1['p'] = 1;
table1['H'] = 2;    table1['h'] = 2;
table1['R'] = 3;    table1['r'] = 3;
table1['E'] = 4;    table1['e'] = 4;
table1['A'] = 5;    table1['a'] = 5;
table1['K'] = 6;    table1['k'] = 6;

//棋子分數
table2['C'] = 1;      table2['c'] = 1;
table2['P'] = 100;    table2['p'] = 100;
table2['H'] = 250;    table2['h'] = 250;
table2['R'] = 550;    table2['r'] = 550;
table2['E'] = 1150;   table2['e'] = 1150;
table2['A'] = 2350;   table2['a'] = 2350;
table2['K'] = 4750;   table2['k'] = 4750;
table2['@'] = 100;



	int end=0;
	char result;

//實際盤面
char startboard[32]={'K','k','p','e','p','H','h','R',
	                 'R','c','a','P','p','P','P','P',
					 'A','c','r','e','E','h','H','a',
                     'r','C','C','E','p','P','p','A'};  



	//currentBoard & Board1 Initialization
	for(int i=0;i<32;i++) {
		currentBoard[i]=64;
	}

	

	//startBoard(startboard);   //開局盤面
	currentBoard[5]=NULL;
	currentBoard[12]=NULL;
	currentBoard[13]=NULL;
	//currentBoard[17]=NULL;

	

	result = Game(end,startboard, selectOrder());

	int aaa[5];
	if(SCORE<0) {
	   aaa[0] = SCORE/-10000;
	   aaa[1] = (SCORE%10000)/-1000;
	   aaa[2] = ((SCORE%10000)%1000)/-100;
	   aaa[3] = (((SCORE%10000)%1000)%100)/-10;
	   aaa[4] = (((SCORE%10000)%1000)%100)%-10;
	}

	else {
	  aaa[0] = SCORE/10000;
	  aaa[1] = (SCORE%10000)/1000;
	  aaa[2] = ((SCORE%10000)%1000)/100;
	  aaa[3] = (((SCORE%10000)%1000)%100)/10;
	  aaa[4] = (((SCORE%10000)%1000)%100)%10;
	}
	


	printf("結果為%c, 實際分數為%d, 分數為%d,%d,%d,%d,%d",result,SCORE,aaa[0],aaa[1],aaa[2],aaa[3],aaa[4]);
	system("pause");

	//system("pause");
	return 0;
}
