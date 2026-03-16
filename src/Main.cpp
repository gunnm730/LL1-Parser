#include"Syntactic_Analysis.h"

int main() {
	string path;
	cout << "请输入文本路径:" ;
	cin >> path;
	string s;
	cout << "请输入待分析句子:";
	cin >> s;
	Syntactic_Analysis::Analysis(path, s);
	return 0;
}