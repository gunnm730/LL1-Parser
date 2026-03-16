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

	// 文法
	static map<char,vector<string>> Language;

	// 非终结符
	static vector<char> Non_Terminal_Symbol;

	// 终结符
	static vector<char> Terminal_Symbol;

	// FIRST集,非终结符开头的终结符
	static map<char, set<char>> First_Char;

	// FIRST集,字符串
	static map<string, set<char>> First_String;

	// FOLLOW集
	static map<char, set<char>> Follow;

	// 推导顺序
	static vector<pair<char, string>> Way;

	// 是否是非终结符
	static bool _isNonTerminalSymbol(char ch);

	// 是否是终结符
	static bool _isTerminalSymbol(char ch);

	// 获取拓扑排序得到的处理顺序
	static vector<char> _getOrder();

	// 读取文件,将每个产生式用 string 表示
	static vector<string> _getFileContent(const string& path);

	// 判断一个字符串是否是有效的产生式格式
	static bool _isValidSentence(const string& sentence, char& LHS, vector<string>& RHS);

	// 辅助函数,找到Non_Terminal_Symbol和Terminal_Symbol中尚未使用的大写字母
	static char _findLeastAlpha();

	// 获取原始文法
	static void _getOriginalLanguage(const vector<string>& content);

	// 判断一个产生式是否存在左递归,如果是则返回右部字符串数组里对应的索引,如果不是则返回 {-1}
	// 如"A::=b|AcB|@|Ab",将会返回{1,3}
	static vector<int> _isLeftRecursion(const char& LHS, const vector<string>& RHS);

	// 消除左递归,返回值为消除左递归后的局部文法
	static map<char, vector<string>> _removeLeftRecursion_Sentence(const char& LHS, const vector<string>& RHS, vector<int> indexs);

	// 消除Language中的左递归
	static bool _removeLeftRecursion();
	
	// 判断一个产生式是否存在回溯,辅助逻辑同_isLeftRecursion
	static vector<int> _isBackTracking(const char& LHS, const vector<string>& RHS);

	// 判断一个文法是否存在直接回溯
	static bool _hasDirectBacktracking();

	// 消除回溯
	static  map<char, vector<string>> _eliminateBackTracking_Sentence(const char& LHS, const vector<string>& RHS, vector<int> indexs);

	// 消除Language中的回溯
	static bool _eliminateBackTracking();

	// 辅助函数:检查代入后的文法是否存在间接回溯
	static map<char, vector<string>> _getSubstitutedGrammarForCheck();

	// 消除无用产生式
	static void _eliminateUselessSentence();

	// 打印文法
	static void _printLanguage();

	/*
	 * 整合函数：文法预处理主入口
	 */
	static void _getLanguage(const string& path);

	// 生成得到的字符的FIRST集
	static void _getFirst_Char();

	// 计算字符串的FIRST集
	static set<char> _getFirstOfString(const string& s);
	
	// 计算每个右部候选式的FIRST集
	static void _getFirst_String();

	// 计算每个非终结符的FOLLOW集
	static void _getFollow();

	// 通过栈顶非终结符和向前看终结符找到对应的产生式,如果没有找到,返回{0,"Error"}
	static pair<char, string> _getSentenceFrom2Symbols(char NT, char T);

	// LL1分析器:检查输入串是否为合法的句子
	static bool _isAvailableSentence(const string& s);

	// 每行的分析逻辑
	static int _printLineOfTable(int SN, string& Q1, string& Q2);

	static void _printFirstStringSets();

	static void _printFollowSets();

	static void _printLL1ParsingTable();

	static void _printDerivationSteps();
public:
	static void Analysis(const string& path, const string& s);
};
