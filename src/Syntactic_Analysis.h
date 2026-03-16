#pragma once
#include<vector>
#include<iostream>
#include<map>
#include<set>
#include<fstream>
#include<sstream>
#include<algorithm>
#include<queue>
#include<iomanip> 
using namespace std;

vector<char> topological_sort(map<char, vector<char>>& graph);

class Syntactic_Analysis {
private:
	static char Start_Symbol;

	//文法
	static map<char,vector<string>> Language;

	//非终结符
	static vector<char> Non_Terminal_Symbol;

	//终结符
	static vector<char> Terminal_Symbol;

	//FIRST集,终结符和非终结符
	static map<char, set<char>> First_Char;

	//FIRST集,字符串
	static map<string, set<char>> First_String;

	//FOLLOW集
	static map<char, set<char>> Follow;

	//推导顺序
	static vector<pair<char, string>> Way;

	//是否属于非终结符
	static bool _isNonTerminalSymbol(char ch);

	//是否属于终结符
	static bool _isTerminalSymbol(char ch);

	//拓扑排序得到依赖顺序
	static vector<char> _getOrder();

	//读取文件,将每个生成式都用 string 表示
	static vector<string> _getFileContent(const string& path);

	//判断一个字符串是否满足生成式格式
	static bool _isValidSentence(const string& sentence, char& LHS, vector<string>& RHS);

	//辅助函数,找到Non_Terminal_Symbol和Terminal_Symbol数组里最小的未出现大写字母
	static char _findLeastAlpha();

	//获取原始文法
	static void _getOriginalLanguage(const vector<string>& content);

	//判断一个生成式是否是左递归,如果是则返回右部字符串数组里面对应的索引,如果不是则返回 {-1}
	//如"A::=b|AcB|@|Ab",将会返回{1,3} ("AcB"和"Ab"在{"b","AcB","@","Ab"}里的索引）
	static vector<int> _isLeftRecursion(const char& LHS, const vector<string>& RHS);

	//清除左递归,返回的值是消除左递归后的局部文法
	static map<char, vector<string>> _removeLeftRecursion_Sentence(const char& LHS, const vector<string>& RHS, vector<int> indexs);

	//清除Language中的左递归
	static bool _removeLeftRecursion();
	
	//判断一个生成式是否是包含回溯,返回逻辑类似_isLeftRecursion
	static vector<int> _isBackTracking(const char& LHS, const vector<string>& RHS);

	//判断一个文法是否具有直接回溯
	static bool _hasDirectBacktracking();

	//消除回溯
	static  map<char, vector<string>> _eliminateBackTracking_Sentence(const char& LHS, const vector<string>& RHS, vector<int> indexs);

	//清除Language中的回溯
	static bool _eliminateBackTracking();

	//代入一个临时副本来检查是否具有间接回溯
	static map<char, vector<string>> _getSubstitutedGrammarForCheck();

	//消除冗余式
	static void _eliminateUselessSentence();

	//输出文法
	static void _printLanguage();

	/*
	* 第一部分函数接口
	*/
	static void _getLanguage(const string& path);

	//生成单符号的FIRST集
	static void _getFirst_Char();

	//辅助函数,根据字符串生成FIRST集
	static set<char> _getFirstOfString(const string& s);
	
	//生成每个右部式子的FIRST集
	static void _getFirst_String();

	//生成每个单字符的FOLLOW集
	static void _getFollow();

	//通过两个符号找到对应的生成式,如果没找到,返回{0,"Error"}
	static pair<char, string> _getSentenceFrom2Symbols(char NT, char T);

	//LL1分析表分析用户给出的句子是否为可生成式
	static bool _isAvailableSentence(const string& s);

	//每行的逻辑
	static int _printLineOfTable(int SN, string& Q1, string& Q2);

	static void _printFirstStringSets();

	static void _printFollowSets();

	static void _printLL1ParsingTable();

	static void _printDerivationSteps();
public:
	static void Analysis(const string& path, const string& s);
};