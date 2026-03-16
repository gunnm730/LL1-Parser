#include "Syntactic_Analysis.h"

char Syntactic_Analysis::Start_Symbol;

map<char, vector<string>> Syntactic_Analysis::Language;

vector<char> Syntactic_Analysis::Non_Terminal_Symbol;

vector<char> Syntactic_Analysis::Terminal_Symbol;

map<char, set<char>> Syntactic_Analysis::First_Char;

map<string, set<char>> Syntactic_Analysis::First_String;

map<char, set<char>> Syntactic_Analysis::Follow;

vector<pair<char, string>> Syntactic_Analysis::Way;

bool Syntactic_Analysis::_isNonTerminalSymbol(char ch)
{
    if (find(Non_Terminal_Symbol.begin(), Non_Terminal_Symbol.end(), ch) != Non_Terminal_Symbol.end()) {
        return true;
    }
    return false;
}

bool Syntactic_Analysis::_isTerminalSymbol(char ch)
{
    if (find(Terminal_Symbol.begin(), Terminal_Symbol.end(), ch) != Terminal_Symbol.end()) {
        return true;
    }
    return false;
}

/*
* 利用拓扑排序完成字符依赖关系的排列
* 之所以须要依赖关系,可以用下面的例子来解释
* S::=Ab
* A::=Bx
* B::=Sb
* 这是一个左递归的例子,我们可以用判断环路的方式快速判断出有没有左递归
* 比如这里S->A->B->S,形成环路,判断完成
* 除此之外,拓扑排序还有可以提升代入效率(因为按照依赖顺序)
* 帮助输出文法(保证文法按照依赖关系排序)等优点
*/
vector<char> Syntactic_Analysis::_getOrder()
{
    //构建有向图
    map<char, vector<char>> dependencies;
    for (auto& entry : Language) {
        for (string s : entry.second) {
            //这里须要注意:我们对所谓的依赖关系,只判定字符串的首字母是不是非终结符,毕竟LL(1)是一个最左推导算法
            if (!s.empty() && _isNonTerminalSymbol(s[0])) {
                dependencies[entry.first].push_back(s[0]);
            }
        }
    }
    vector<char> TRAVERSAL_ORDER = topological_sort(dependencies);
    /*
    * 很显然,如果返回数组为空,也就是出现了环路,这时候我们将非终结符数组倒序作为代入顺序
    */
    if (TRAVERSAL_ORDER.empty())
    {
        TRAVERSAL_ORDER = vector<char>(Non_Terminal_Symbol.rbegin(), Non_Terminal_Symbol.rend());
    }
    else {
        for (char ch : Non_Terminal_Symbol) {
            if (find(TRAVERSAL_ORDER.begin(), TRAVERSAL_ORDER.end(), ch) == TRAVERSAL_ORDER.end()) {
                TRAVERSAL_ORDER.push_back(ch);
            }
        }
    }
    return TRAVERSAL_ORDER;
}

/*
* 读取文本内容,这部分将文本内容按照生成式分割为数个字符串,以字符串数组储存
*/
vector<string> Syntactic_Analysis::_getFileContent(const string& path)
{
    //文件路径
    string FILE_PATH;
    //返回数组
    vector<string> DATA;

    //Windows直接复制路径会有双引号,在shell里无法解析,所以这里预先处理一下
    if (path[0] == '"')FILE_PATH = path.substr(1, path.size() - 2);
    else FILE_PATH = path;

    //二进制方式读取文本内容
    ifstream FILE(FILE_PATH, ifstream::binary);
    
    //检测文件是否成功打开
    if (!FILE.is_open()) {
        cerr << "Failed to open the file!";
        return DATA;
    }

    //逐行读取,保存在DATA内
    string LINE;
    while (getline(FILE, LINE)) {
        istringstream ISS(LINE);
        string TOKEN;
        while (ISS >> TOKEN) {
            DATA.push_back(TOKEN);
        }
    }
    FILE.close();
    return DATA;
}

/*
* 构建状态转换机
* 对形如A::=abc这样的文法,我们将其分为三个部分"A""::=""abc"
* 这个函数和实验一词法分析思路是一样的,将每个字符串当作一个单词进行读取,不进行赘述
*/
bool Syntactic_Analysis::_isValidSentence(const string& sentence, char& LHS, vector<string>& RHS)
{
    enum State {
        START,
        IN_LHS,//左部
        IN_FIRST_COLON,//第一个冒号
        IN_SECOND_COLON,//第二个冒号
        IN_EQUALSSIGN,//等于号
        IN_RHS//右部
    };

    RHS.clear();

    State STATE = START;
    char CH;
    string CURRENT_RHS;
    int INDEX = 0;
    for (; INDEX < sentence.size();) {
        CH = sentence[INDEX];
        switch (STATE) {
        case START:
            if (isalnum(CH)) {
                STATE = IN_LHS;
                LHS = CH;
                INDEX++;
            }
            else {
                return false;
            }
            break;
        case IN_LHS:
            if (CH == ':') {
                STATE = IN_FIRST_COLON;
                INDEX++;
            }
            else {
                return false;
            }
            break;
        case IN_FIRST_COLON:
            if (CH == ':') {
                STATE = IN_SECOND_COLON;
                INDEX++;
            }
            else {
                return false;
            }
            break;
        case IN_SECOND_COLON:
            if (CH == '=') {
                STATE = IN_EQUALSSIGN;
                INDEX++;
            }
            else {
                return false;
            }
            break;
        case IN_EQUALSSIGN:
            if (isalnum(CH) || CH == '@') {
                STATE = IN_RHS;
                CURRENT_RHS += CH;
                INDEX++;
            }
            else {
                return false;
            }
            break;
        case IN_RHS:
            if (isalnum(CH) || CH == '@') {
                CURRENT_RHS += CH;
                INDEX++;
            }
            else if (CH == '|') {
                RHS.push_back(CURRENT_RHS);
                CURRENT_RHS.clear();
                INDEX++;
            }
            else {
                return false;
            }
            break;
        default:
            return false;
        }
    }
    switch (STATE) {
    case IN_RHS:
        RHS.push_back(CURRENT_RHS);
        break;
    default:
        return false;
    }
    return true;
}

/*
* 文法在处理左递归和回溯时免不了会产生新的非终结符
* 比如A::=Aa|a
* 消除左递归就是A::=aA'  A'=aA'|@
* 因为本程序采取的存储文法的方式为map<char,vector<string>>,左部是字符变量
* 所以本程序旨在找一个没出现过的大写字母(ascii尽量小)作为A'的位置
*/
char Syntactic_Analysis::_findLeastAlpha()
{
    int mask = 0;//掩码
    for (char CH : Non_Terminal_Symbol) {
        if (CH >= 'A' && CH <= 'Z') {
            mask = mask | (0x1 << (CH - 65));
        }
    }
    for (char CH : Terminal_Symbol) {
        if (CH >= 'A' && CH <= 'Z') {
            mask = mask | (0x1 << (CH - 65));
        }
    }
    for (int i = 0; i < 26; ++i) {
        if (!(mask & (1 << i))) { // 找到第一个 0 位
            return static_cast<char>('A' + i);
        }
    }
    return '0';
}

/*
* 通过_isValidSentence函数得到初始文法
*/
void Syntactic_Analysis::_getOriginalLanguage(const vector<string>& content){
    char LHS;//左部
    vector<string> RHS;//右部
    bool isFirstProduction = true;

    for (const string& s : content) {
        //这里函数内的LHS和RHS是引用传参
        if (_isValidSentence(s, LHS, RHS)) {
            if (isFirstProduction) { // 如果是第一个有效的产生式
                Start_Symbol = LHS; // 将其左部设为起始符号
                isFirstProduction = false; // 关闭标志
            }
            Language.insert(make_pair(LHS, RHS));
            Non_Terminal_Symbol.push_back(LHS);
        }
    }

    for (auto& entry : Language) {
        for (string s : entry.second) {
            for (char ch : s) {
                if (!_isNonTerminalSymbol(ch) && !_isTerminalSymbol(ch)) {
                    Terminal_Symbol.push_back(ch);
                }
            }
        }
    }
}

/*
* 判断单句是否是直接左递归
* 如"A::=b|AcB|@|Ab",将会返回{1,3} ("AcB"和"Ab"在{"b","AcB","@","Ab"}里的索引）
* 这样_removeLeftRecursion_Sentence函数,也就是单句左递归处理函数可以根据得到的索引直接进行处理
*/
vector<int> Syntactic_Analysis::_isLeftRecursion(const char& LHS, const vector<string>& RHS)
{
    vector<int> INDEXS;
    for (int i = 0; i < RHS.size(); i++) {
        if (LHS == RHS[i][0]) {//如果右部字符串首字母与左部非终结符相同
            INDEXS.push_back(i);
        }
    }
    if (INDEXS.size() == 0)return { -1 };//如果不存在左递归则返回vector<int>{-1}
    return INDEXS;
}

/*
* 本函数用于处理已经包含左递归的句子
*/
map<char, vector<string>> Syntactic_Analysis::_removeLeftRecursion_Sentence(const char& LHS, const vector<string>& RHS, vector<int> indexs)
{
    /*
    * 用"A::=b|AcB|@|Ab"讲解本函数如何运作
    * 本函数对于"A::=b|AcB|@|Ab"传递的参数为：
    * LHS:"A"
    * RHS:{"b","AcB",@,"Ab"}
    * indexs:{1,3}
    */

    map<char, vector<string>> RESULT;

    /*
    * 满足左递归的右半部分
    * 对于"A::=b|AcB|@|Ab",其中左递归式为"A::=AcB"和"A::=Ab"
    * 那么本数组的值为{"cB","b"}
    * RIGHT_PART_OF_LEFT_RECURSION:{"cB","b"}
    */
    vector<string> RIGHT_PART_OF_LEFT_RECURSION;
    for (int i : indexs) {
        RIGHT_PART_OF_LEFT_RECURSION.push_back
        (RHS[i].substr
        (1, RHS[i].size() - 1));
    }

    /*
    * 右半部分中无左递归的部分
    * 对于"A::=b|AcB|@|Ab",其中非递归式为"A::=b"和"A::=@"
    * 本数组值为{"b","@"}
    * RIGHT_PART_WITHOUT_LEFT_RECURSION:{"b","@"}
    * 其实上下可以合并起来写,效率更快,这里为了可读性更明晰所以遍历两次
    */
    vector<string> RIGHT_PART_WITHOUT_LEFT_RECURSION;
    for (int i = 0; i < RHS.size(); i++) {
        if (find(indexs.begin(), indexs.end(), i) == indexs.end()) {
            RIGHT_PART_WITHOUT_LEFT_RECURSION.push_back(RHS[i]);
        }
        else;
    }

    /*
    * 用改写法消除左递归,第一步是新建一个字符
    * 这里从所有文法字符集里找一个没出现过的大写字母
    * 假设这个字母是G
    * NEW_NTS:'G'
    */
    char NEW_NTS = _findLeastAlpha();
    Non_Terminal_Symbol.push_back(NEW_NTS);

    /*
    * 第二步,构造左部为G的生成式
    * 右部的格式应该为RIGHT_PART_OF_LEFT_RECURSION+NEW_NTS
    * 比如RIGHT_PART_OF_LEFT_RECURSION:{"cB","b"}
    * 那么新的右部就应该为{"cBG","bG","@"}
    * RIGHT_PART_OF_NEW_NTS:{"cBG","bG","@"}
    */
    vector<string> RIGHT_PART_OF_NEW_NTS = { "@" };
    if (!_isTerminalSymbol('@'))Terminal_Symbol.push_back('@');
    for (string s : RIGHT_PART_OF_LEFT_RECURSION) {
        RIGHT_PART_OF_NEW_NTS.push_back(s + NEW_NTS);
    }

    /*
    * 第三步,构造原来的非终结符的新的右部
    * 右部的格式应该为RIGHT_PART_WITHOUT_LEFT_RECURSION+NEW_NTS
    * 比如RIGHT_PART_WITHOUT_LEFT_RECURSION:{"b","@"}
    * 那么原终结符新的右部就应该为{"bG","@G"}
    * 这里我们处理原字符串为@,即空串的情况
    * NEW_RIGHT_PART_OF_ORIGINAL_NTS：{"bG","G"}
    */
    vector<string> NEW_RIGHT_PART_OF_ORIGINAL_NTS;
    for (string s : RIGHT_PART_WITHOUT_LEFT_RECURSION) {
        if (s != "@") {
            NEW_RIGHT_PART_OF_ORIGINAL_NTS.push_back(s + NEW_NTS);
        }
        else {
            NEW_RIGHT_PART_OF_ORIGINAL_NTS.push_back(string(1, NEW_NTS));
        }
    }

    /*
    * 现在审视我们得到的左部字符和右部数组:
    *
    * LHS:'A'
    * NEW_RIGHT_PART_OF_ORIGINAL_NTS：{"bG","G","@"}
    *
    * NEW_NTS:'G'
    * RIGHT_PART_OF_NEW_NTS:{"cBG","bG"}
    *
    * 不难看出,两两组合就是改写后的式子:
    *
    * A::=bG|G
    * G::=cBG|bG|@
    *
    * 原式:
    * A::=b|AcB|@|Ab
    */

    RESULT.insert(make_pair(LHS, NEW_RIGHT_PART_OF_ORIGINAL_NTS));
    RESULT.insert(make_pair(NEW_NTS, RIGHT_PART_OF_NEW_NTS));

    return RESULT;
}

/*
* 综合的通过代入进行间接和直接的左递归消除函数
*/
bool Syntactic_Analysis::_removeLeftRecursion()
{
    /*
    * 因为一般情况下,文法的终止状态非终结符是第一个
    * 我们使用拓扑排序确定关系遍历顺序
    * 如果产生循环依赖,则需要手动固定顺序
    * 我们将Non_Terminal_Symbol倒转得到的数组作为遍历的顺序
    * 比如Non_Terminal_Symbol:{'S','A','B'}
    * 那么遍历顺序就是B->A->S
    * 这么做还有一个原因是S是最终状态,是有向图里只进不出的终止点(或者说只出不进也对)
    * TRAVERSAL_ORDER:{'B','A','S'}
    */
    vector<char> TRAVERSAL_ORDER = _getOrder();

    /*
    * 这里新建一个临时map来遍历数组,因为遍历过程中会存在两种情况:
    * 1,例如:
    * A->Bac
    * B->a
    * 这样一个文法,将B代入第一条式子:
    * A->aac
    * 依然不存在左递归,那么这样的代入仅仅是为了往后推导用的,而原式并不需要做出改变
    * 2,过程中会对文法map作修改,创建临时数组便于写修改逻辑
    */
    map<char, vector<string>> TEMP_Language = Language;

    /*
    * 新建一个数组来存储已经被解决过的非终止符
    */
    vector<char> NON_TERMINALS_USED;
    /*
    * 新建一个数组来存储左递归判断时的索引
    */
    vector<int> INDEXS;
    /*
    * 接下来的内容我们将跟踪一个例子
    * A::=aB|Bb
    * B::=Ac|d
    */
    for (char CH : TRAVERSAL_ORDER) {
        /*
        * 接下来,if模块的内容是处理直接递归的,这部分同样出现在else模块内
        * 因为本函数的举例不包含直接左递归,所以如果要查看if模块逻辑可以直接看else内部相同的部分跟随例子的注释
        */
        /*
        * INDEXS用于存储判断左递归时的结果(如果是则返回右部字符串数组里面对应的索引,如果不是则返回 {-1})
        */
        INDEXS = _isLeftRecursion(CH, TEMP_Language[CH]);
        /*
        * 根据_removeLeftRecursion_Sentence可以得到去除左递归后的局部文法
        * 这样我们将原文法中对应的内容移去
        */
        if (INDEXS != vector<int>{-1}) {
            map<char, vector<string>> NEW_MAP = _removeLeftRecursion_Sentence(CH, TEMP_Language[CH], INDEXS);
            TEMP_Language.erase(CH);
            TEMP_Language.insert(NEW_MAP.begin(), NEW_MAP.end());
        }
        /*
        * 本例的处理从此开始
        */
        else {
            /*
            * 新产生式的右部,对于本例:
            * A::=aB|Bb
            * B::=Ac|d
            * 消除后的结果应该是:
            * A::=aBC|dbC
            * B::=Ac|d
            * C::=@|cbC
            * 那么就代表NEW_PRODUCTION的值将会是{"@","cbC"}
            */
            vector<string> NEW_PRODUCTIONS;
            /*
            * 迭代器数组,指向需要被删除的右部
            * 本例中,出现左递归的入口是A->Bb
            * 所以,该数组内存放的迭代器(本例仅1个),指向"Bb"
            */
            vector<vector<string>::iterator> TO_ERASE;
            /*
            * 用迭代器遍历左部为B/A,本部分旨在寻找左递归入口A->Bb
            * 所以,对于非终结符为B的部分,会在条件判断时不通过,所以以下逻辑默认不讨论B的部分
            */
            /*
            * 首先遍历A的右部几个字符串(B先遍历,但B不是入口,所以跳过)
            */
            for (auto it = TEMP_Language[CH].begin(); it != TEMP_Language[CH].end(); it++) {
                /*
                * 本例A::=aB|Bb
                * FIRST_CH为当前字符串的首字母
                * 意味着在两次遍历里,FIRST_CH的值分别为'a','B'
                */
                char FIRST_CH = (*it)[0];
                /*
                * 首先要判断当前字符串的首字母是不是非终结符
                * 如果不是那就可以直接跳过进入下一个字符串
                */
                if (_isNonTerminalSymbol(FIRST_CH)) {
                    /*
                    * 除了必须是非终结符以外,还必须保证该首字母已经被处理过(这也是为什么B的部分被跳过了,因为A当时没有被处理)
                    */
                    if (find(NON_TERMINALS_USED.begin(), NON_TERMINALS_USED.end(), FIRST_CH) 
                        != NON_TERMINALS_USED.end()) {
                        /*
                        * 取该字符串除了首字母以外的地方
                        * 在A->Bb中,RIGHT:"b"(注意是字符串!)
                        */
                        string RIGHT = (*it).substr(1);

                        /*
                        * 将这个取出的字符串RIGHT接在
                        * "该首字母对应的生成式右部的每个字符串后面"
                        * 也就是取当前首字母FIRST_CH:'B'
                        * 在文法中找到B的部分,也就是:
                        * B::=Ac|d
                        * 取"Ac"和"d"分别接上RIGHT然后压入NEW_PRODUCTIONS
                        * NEW_PRODUCTIONS:{"AcB","dB"}
                        */
                        for (const string& s : TEMP_Language[FIRST_CH]) {
                            if (s != "@")NEW_PRODUCTIONS.push_back(s + RIGHT);
                            else if (!RIGHT.empty())NEW_PRODUCTIONS.push_back(RIGHT);
                            else NEW_PRODUCTIONS.push_back(string("@"));
                        }

                        /*
                        * 当前迭代器加入数组,留待当前生成式解决完毕后统一删除
                        */
                        TO_ERASE.push_back(it);
                    }
                }
            }

            /*
            * 将临时map内对应的右部删除
            * 因为迭代器删除会影响后面的元素,所以需要逆序处理
            */
            
            for (auto rit = TO_ERASE.rbegin(); rit != TO_ERASE.rend(); ++rit) {
                TEMP_Language[CH].erase(*rit);
            }

            /*
            * 将新的生成式加入,也就是A->AcB和A->dB
            */
            TEMP_Language[CH].insert(TEMP_Language[CH].end(), NEW_PRODUCTIONS.begin(), NEW_PRODUCTIONS.end());


            /*
            * 此时的文法为:
            * A::=aB|AcB|dB
            * B::=Ac|d
            */
            INDEXS = _isLeftRecursion(CH, TEMP_Language[CH]);
            /*
            * 判断是不是形成左递归,也就是和外if模块(本函数第一个if)逻辑相同
            */
            if (INDEXS != vector<int>{-1}) {
                map<char, vector<string>> NEW_MAP = _removeLeftRecursion_Sentence(CH, TEMP_Language[CH], INDEXS);
                TEMP_Language.erase(CH);
                TEMP_Language.insert(NEW_MAP.begin(), NEW_MAP.end());
            }
        }
        NON_TERMINALS_USED.push_back(CH);
        
    }
    Language = TEMP_Language;
    return true;
}

/*
* 判断单句是否存在直接回溯
* 如"A::=acb|@|acd",将会返回{0,2}("acb"和"acd"在{"acb","@","acd"}里的索引)
* 这样_eliminateBackTracking_Sentence函数,可以直接根据索引处理直接回溯
*/
vector<int> Syntactic_Analysis::_isBackTracking(const char& LHS, const vector<string>& RHS)
{
    vector<char> FIRST_OF_RHS;
    vector<int> INDEXS;
    char ch;//用来表示右部的首位
    char CH = '@';//用来表示产生回溯的首位,比如A::=Bb|BAc,那么CH=='B'.初始值为'@'因为这个首位不可能是'@'
    for (string s : RHS) {
        ch = s[0];
        if (ch == LHS)continue;
        if (_isNonTerminalSymbol(ch))continue;
        auto it = find(FIRST_OF_RHS.begin(), FIRST_OF_RHS.end(), ch);
        //如果找到
        if (it != FIRST_OF_RHS.end()) {
            CH = ch;
            break;
        }
        else {
            FIRST_OF_RHS.push_back(ch);
        }
    }
    //如果存在这样一个首位回溯
    if (CH != '@') {
        for (int i = 0; i < RHS.size(); i++) {
            if (RHS[i][0] == CH) {
                INDEXS.push_back(i);
            }
        }
        return INDEXS;
    }
    
    return{ -1 };
}

/*
* 一个调用_isBackTracking的函数,可以归约为_isBackTracking
* 用处是返回文法是否存在直接左递归,其实意义不大......
*/
bool Syntactic_Analysis::_hasDirectBacktracking()
{
    for (auto const& [lhs, rhs] : Language)
    {
        if (_isBackTracking(lhs, rhs) != vector<int>{-1})
        {
            cout << "诊断：发现非终结符 '" << lhs << "' 存在直接回溯。" << endl;
            return true;
        }
    }
    return false;
}

/*
* 本函数用于处理已经包含直接回溯的句子
*/
map<char, vector<string>> Syntactic_Analysis::_eliminateBackTracking_Sentence(const char& LHS, const vector<string>& RHS, vector<int> indexs)
{
    /*
    * 用"A::=b|BcB|@|Bb"讲解本函数如何运作
    * 本函数对于"A::=b|BcB|@|Bb"传递的参数为：
    * LHS:"A"
    * RHS:{"b","BcB",@,"Bb"}
    * indexs:{1,3}
    */

    map<char, vector<string>> RESULT;

    /*
    * 满足回溯的右半部分
    * 对于"A::=b|BcB|@|Bb",满足的部分为"A::=BcB"和"A::=Bb"
    * 那么本数组的值为{"cB","b"}
    * RIGHT_PART_OF_BACKTRACKING:{"cB","b"}
    */
    vector<string> RIGHT_PART_OF_BACKTRACKING;
    for (int i : indexs) {
        string NEW_STRING_OF_BACKTRACKING = RHS[i].substr(1, RHS[i].size() - 1);
        if (NEW_STRING_OF_BACKTRACKING.empty())NEW_STRING_OF_BACKTRACKING = "@";
        if (find(RIGHT_PART_OF_BACKTRACKING.begin(), RIGHT_PART_OF_BACKTRACKING.end(), NEW_STRING_OF_BACKTRACKING) == RIGHT_PART_OF_BACKTRACKING.end())
        {
            RIGHT_PART_OF_BACKTRACKING.push_back(NEW_STRING_OF_BACKTRACKING);
        }
    }

    

    /*
    * 右半部分中无回溯的部分
    * 对于"A::=b|AcB|@|Ab",其中无回溯式为"A::=b"和"A::=@"
    * 本数组值为{"b","@"}
    * RIGHT_PART_WITHOUT_BACKTRACKING:{"b","@"}
    */
    vector<string> RIGHT_PART_WITHOUT_BACKTRACKING;
    for (int i = 0; i < RHS.size(); i++) {
        if (find(indexs.begin(), indexs.end(), i) == indexs.end()) {
            RIGHT_PART_WITHOUT_BACKTRACKING.push_back(RHS[i]);
        }
        else;
    }

    /*
    * 用改写法消除左递归,第一步是新建一个字符
    * 这里从所有文法字符集里找一个没出现过的大写字母
    * 假设这个字母是G
    * NEW_NTS:'G'
    */
    char NEW_NTS = _findLeastAlpha();
    Non_Terminal_Symbol.push_back(NEW_NTS);

    /*
    * 对于原式的左部A,右部的格式应该是:
    * 一部分为RIGHT_PART_WITHOUT_BACKTRACKING
    * 然后RHS[indexs[0]][0]+NEW_BTS
    * 其中RHS[indexs[0]][0]:B
    * NEW_BTS:G
    * A::=b|@|BG
    */
    RIGHT_PART_WITHOUT_BACKTRACKING.push_back(string(1, RHS[indexs[0]][0]) + NEW_NTS);
    RESULT.insert(make_pair(LHS, RIGHT_PART_WITHOUT_BACKTRACKING));

    /*
    * 很显然,RIGHT_PART_OF_BACKTRACKING就是G的右部
    * G::=cB|b
    */
    RESULT.insert(make_pair(NEW_NTS, RIGHT_PART_OF_BACKTRACKING));

    return RESULT;
}

/*
* 其实本函数逻辑应该是和左递归处理差不多的,但这里实际上出现了一点偏差,说明如下:
* _removeLeftRecursion函数同时完成了代入和消除左递归的工作,但是因为一些编写时的思路错漏,导致_eliminateBackTracking并没有同时完成两项任务
* 所以实际上本函数只用于处理直接回溯
* 为了弥补这一点,下一个函数_getSubstitutedGrammarForCheck就是做了代入的工作
* _getSubstitutedGrammarForCheck按照依赖顺序代入展开右部出现的非终结符,并返回这样做之后的文法
*/
bool Syntactic_Analysis::_eliminateBackTracking()
{
    /*
    * 同拓扑排序
    */
    bool RETURN = true;
    vector<char> TRAVERSAL_ORDER = _getOrder();

    // TEMP_Language 仍然是必要的，因为我们需要在修改的同时进行迭代
    map<char, vector<string>> TEMP_Language = Language;

    vector<int> INDEXS;

    for (char CH : TRAVERSAL_ORDER) { 
    label1:
        // 在循环中反复检查并消除回溯，直到当前非终结符稳定为止
        INDEXS = _isBackTracking(CH, TEMP_Language[CH]);

        if (INDEXS != vector<int>{-1}) {
            map<char, vector<string>> NEW_MAP = _eliminateBackTracking_Sentence(CH, TEMP_Language[CH], INDEXS);

            // 重要：同时更新 Language 和 TEMP_Language
            Language.erase(CH);
            Language.insert(NEW_MAP.begin(), NEW_MAP.end());
            TEMP_Language.erase(CH);
            TEMP_Language.insert(NEW_MAP.begin(), NEW_MAP.end());
            goto label1; // 继续检查当前非终结符是否还有其他回溯
        }
    }
    return RETURN;
}

/*
* 上文提到的代入函数,逻辑和左递归消除函数_removeLeftRecursion中的代入部分一致
* 其实就是把非终结符展开,没有什么复杂逻辑
*/
map<char, vector<string>> Syntactic_Analysis::_getSubstitutedGrammarForCheck()
{
    map<char, vector<string>> temp_grammar = Language;
    vector<char> non_terminals_used;

    vector<char> TRAVERSAL_ORDER = _getOrder();

    for (char CH : TRAVERSAL_ORDER)
    {
        vector<string> new_productions;
        vector<vector<string>::iterator> to_erase;

        // 在temp_grammar上执行代入逻辑
        for (auto it = temp_grammar[CH].begin(); it != temp_grammar[CH].end(); ++it)
        {
            if (it->empty()) continue; // 安全检查

            char first_ch = (*it)[0];
            if (_isNonTerminalSymbol(first_ch) && find(non_terminals_used.begin(), non_terminals_used.end(), first_ch) != non_terminals_used.end())
            {
                string right_part = it->substr(1);
                to_erase.push_back(it);
                for (const string& s : temp_grammar[first_ch])
                {
                    if (s != "@") new_productions.push_back(s + right_part);
                    else if (!right_part.empty()) new_productions.push_back(right_part);
                    else if (find(new_productions.begin(), new_productions.end(), "@") == new_productions.end())
                        new_productions.push_back("@");
                }
            }
        }

        // 更新这个临时文法
        for (auto rit = to_erase.rbegin(); rit != to_erase.rend(); ++rit) {
            temp_grammar[CH].erase(*rit);
        }
        temp_grammar[CH].insert(temp_grammar[CH].end(), new_productions.begin(), new_productions.end());

        non_terminals_used.push_back(CH);
    }
    return temp_grammar;
}

/*
* 去除冗余生成式,为什么会有冗余生成式呢？
* 举个例子
* S::=Aab|Bab
* A::=Sbb
* B::=Sa|@
* 这个文法直接处理而不去除冗余式会得到
* C -> @ | bbabC | aabC
* A -> Sbb
* B -> Sa | @
* S -> abC
* 因为输出也是按照拓扑排序得到的,我们发现C已经完成了整个语法生成的任务
* A,B,S都属于无效语句,正确文法就是:
* C -> @ | bbabC | aabC
*/
void Syntactic_Analysis::_eliminateUselessSentence()
{
    /*
    * 本算法主要有两个条件:
    * 1,找到所有必要的非终结符
    * 2,找到所有可达的非终结符
    */

    /*
    * Part 1:找到所有必要的非终结符
    */
    /*
    * 本部分的实现思路是这样的:
    * 先创建一个数组USEFUL_NTS,初始化为全部的终结符
    * 首先从文法中找到全部"只产生终结符的产生式"
    * 比如:A::=ab
    * 因为可以直接产生仅终结符的式子,所以A是"必要的"
    * 也就是说,A现在和终结符等价,A也加入数组RIGHTPART
    * 如此反复
    */
    vector<char> USEFUL_NTS = Terminal_Symbol;
    /*
    * USEFUL_NTS数组的复制,用于每次操作后检测RIGHTPART是否发生变化
    * 如果USEFUL_NTS不再变化,说明整理完毕
    * USEFUL_NTS中的非终结符可以保留,没出现的非终结符予以删除
    */
    vector<char> LAST_USEFUL_NTS;
    do {
        LAST_USEFUL_NTS = USEFUL_NTS;
        for (auto& [LHS, RHS] : Language) {
            for (string s : RHS) {
                bool isUSEFUL_NTS = true;
                for (char ch : s) {
                    /*
                    * 如果一个产生式右部满足出现一个只包含"必要的"字符的字符串
                    * 那么这个产生式左部也加入
                    */
                    if (find(USEFUL_NTS.begin(), USEFUL_NTS.end(), ch) == USEFUL_NTS.end()) {
                        isUSEFUL_NTS = false;
                        break;
                    }
                }
                if (isUSEFUL_NTS) {
                    /*
                    * 避免重复加入
                    */
                    if (find(USEFUL_NTS.begin(), USEFUL_NTS.end(), LHS) == USEFUL_NTS.end()) {
                        USEFUL_NTS.push_back(LHS);
                        break;
                    }
                }
            }
        }
    } while (LAST_USEFUL_NTS != USEFUL_NTS);
    /*
    * 废物利用,避免额外开辟空间
    * 现在LAST_USEFUL_NTS用来存储新的Non_Terminal_Symbol
    */
    LAST_USEFUL_NTS.clear();
    for (char ch : Non_Terminal_Symbol) {
        if (find(USEFUL_NTS.begin(), USEFUL_NTS.end(), ch) == USEFUL_NTS.end()) {
            Language.erase(ch);
        }
        else {
            LAST_USEFUL_NTS.push_back(ch);
        }
    }
    /*
    * 赋值Non_Terminal_Symbol
    */
    Non_Terminal_Symbol = LAST_USEFUL_NTS;

    /*
    * Part 2:找到所有可达的非终结符
    */
    /*
    * 本部分的实现思路是这样的:
    * 首先我们设置一个数组AVAILABLE_NTS,初始只包含S一个字符
    * 然后我们遍历AVAILABLE_NTS,将右部发现的新的非终结符加入AVAILABLE_NTS
    * 如果AVAILABLE_NTS不再更新,那么遍历结束
    */

    vector<char> AVAILABLE_NTS = { Start_Symbol };
    vector<char> LAST_AVAILABLE_NTS;
    do {
        LAST_AVAILABLE_NTS = AVAILABLE_NTS;
        for (char ch : AVAILABLE_NTS) {
            for (string s : Language[ch]) {
                for (char ch_1 : s) {
                    if (_isNonTerminalSymbol(ch_1)
                        && find(AVAILABLE_NTS.begin(), AVAILABLE_NTS.end(), ch_1) == AVAILABLE_NTS.end())
                        AVAILABLE_NTS.push_back(ch_1);
                }
            }
        }
    } while (LAST_AVAILABLE_NTS != AVAILABLE_NTS);
    for (char ch : Non_Terminal_Symbol) {
        if (find(AVAILABLE_NTS.begin(), AVAILABLE_NTS.end(), ch) == AVAILABLE_NTS.end()) {
            Language.erase(ch);
        }
    }
    Non_Terminal_Symbol = AVAILABLE_NTS;
}


void Syntactic_Analysis::_printLanguage()
{
    std::cout << "--- Grammar ---" << std::endl;

    bool RETURN = true;

    // Iterate over each key-value pair in the map (each non-terminal and its rules)
    for (char ch : Non_Terminal_Symbol) {
        // Print the non-terminal character (the key) and the production arrow
        std::cout << ch << " -> ";

        // Iterate through the vector of production rules (the value)
        for (size_t i = 0; i < Language[ch].size(); ++i) {
            std::cout << Language[ch][i];
            // If it's not the last rule, print a separator
            if (i < Language[ch].size() - 1) {
                std::cout << " | ";
            }
        }
        // Move to the next line for the next non-terminal
        std::cout << std::endl;
    }
    std::cout << "---------------" << std::endl;
}

void Syntactic_Analysis::_getLanguage(const string& path)
{

    vector<string> CONTENT = _getFileContent(path);
    _getOriginalLanguage(CONTENT);
    _eliminateUselessSentence();
    cout << "原 始 文 法 为:" << endl;
    _printLanguage();
    cout << "原始文法加载完毕，开始进行LL(1)特性诊断..." << endl;

    /*
    * 利用拓扑排序检测是否具有循环依赖,也就是左递归
    */
    map<char, vector<char>> DEPENDENCIES;
    for (auto const& [LHS, RHS] : Language) {
        for (string s : RHS) {
            if (!s.empty() && _isNonTerminalSymbol(s[0])) {
                DEPENDENCIES[LHS].push_back(s[0]);
            }
        }
    }
    //是否有左递归
    bool has_lr = topological_sort(DEPENDENCIES).empty();
    //是否有直接回溯
    bool has_direct_bt = _hasDirectBacktracking();

    // --- 决策阶段 ---
    if (has_lr)
    {
        // 路径一：有左递归，必须“大改”
        cout << "诊断结果：存在左递归。启动完整转换流程..." << endl;
        _removeLeftRecursion();
        _eliminateBackTracking();
        _eliminateUselessSentence();
        cout << "文法转换完成。注意：已生成新文法，无法按原文法推导。" << endl;
    }
    else if (has_direct_bt)
    {
        // 路径二：无左递归，但有直接回溯，需要“小改”
        cout << "诊断结果：无左递归，但存在直接回溯。启动回溯消除..." << endl;
        _eliminateBackTracking();    // 只需调用这个
        _eliminateUselessSentence();
        cout << "回溯消除完成。注意：已生成新文法，无法按原文法推导。" << endl;
    }
    _printLanguage();
    // 路径三：无左递归，无直接回溯。需要进行“深度检查”以排除间接回溯。
    cout << "初步检查通过。正在进行间接回溯的深度检查..." << endl;

    label:
    map<char, vector<string>> substituted_grammar = _getSubstitutedGrammarForCheck();
    bool has_indirect_bt = false;
    for (auto const& [lhs, rhs] : substituted_grammar) {
        if (_isBackTracking(lhs, rhs) != vector<int>{-1}) {
            has_indirect_bt = true;
            break;
        }
    }

    if (has_indirect_bt) {
        // 最终诊断：失败
        cerr << "诊断失败：文法无左递归和直接回溯，但存在“间接回溯”冲突。启动回溯消除..." << endl;
        Language = substituted_grammar;
        _eliminateBackTracking();
        _eliminateUselessSentence();
        goto label;
    }
    else {
        // 最终诊断：成功
        cout << "深度检查通过！该文法是合法的LL(1)文法，无需任何修改。" << endl;
    }
    cout << "所有文法转换和优化已完成。" << endl;
    _printLanguage(); // 打印最终转换后的文法

    // --- 在所有转换后，计算最终文法的 FIRST 和 FOLLOW 集 ---
    cout << "正在计算最终文法的 FIRST 和 FOLLOW 集..." << endl;
    _getFirst_Char();    // 填充 First_Char
    _getFirst_String();  // 填充 First_String
    _getFollow();        // 填充 Follow
    cout << "FIRST 和 FOLLOW 集计算完毕。" << endl;
}

/*
    * 本部分用于找到一个右部的FIRST集
    * 例:
    * A::=Bab|aAB
    * B::=bA|cb|@
    *
    * 这部分大概可以分为以下几种情况:
    * 1,如果s为空或者@,那么FIRST[s]={'@'};
    *
    * 2,如果s[0]为终结符,那么FIRST[s]={s[0]};
    * 例:A::=aAB,FIRST["aAB"]={'a'}
    *
    * 3,如果s[0]为非终结符T,那么FIRST[s]={T的右部字符串的FIRST集并集}
    * 例:A::=Bab,那么
    *
    * FIRST["Bab"]
    * =FIRST["bA"] U FIRST["cb"] U FIRST["@"]
    * ={'b','c','@'}
    *
    * 但这不是最终答案！
    * 如果这个返回值里面包含非终结符'@',那么就需要继续先去掉'@',再加上去掉第一个字符后字符串的结果,也就是说
    *
    * FIRST["Bab"]
    * =FIRST["bA"] U FIRST["cb"]  U FIRST["ab"](Bab去掉第一个字符得到ab)
    * ={'b','c','a'}
    */
void Syntactic_Analysis::_getFirst_Char()
{
    map<char, set<char>> first;
    /*
    * 先将所有终结符的first加入
    */
    for (char ch : Terminal_Symbol) {
        first[ch] = set<char>{ ch };
    }
    /*
    * 再将所有右部的以终结符开头的字符串加入
    */
    for (auto const& [lhs, rhs] : Language) {
        for (string s : rhs) {
            if (_isTerminalSymbol(s[0])) {
                first[lhs].insert(s[0]);
            }
        }
    }
    //标记first数组是否变化
    bool changed = true;
    while (changed) {
        changed = false;
        for (auto const& [lhs, rhs] : Language) {
            for (string s : rhs) {
                for (char ch : s) {
                    //遍历到每个右部字符
                    for (char symbol : first[ch]) {
                        if (symbol != '@') {
                            if (first[lhs].insert(symbol).second) {
                                changed = true;
                            }
                        }
                    }
                    //如果该字符的first集不包含'@'
                    if (!first[ch].count('@')) {
                        break;
                    }
                    if (ch == s.back()) {
                        if (first[lhs].insert('@').second) {
                            changed = true;
                        }
                    }
                }
            }
        }
    }
    First_Char = first;
}

set<char> Syntactic_Analysis::_getFirstOfString(const string& s)
{
    if (s.empty() || s == "@")return{ '@' };
    set<char> result;
    if (_isTerminalSymbol(s[0])) {
        return { s[0] };
    }
    else {
        result = First_Char[s[0]];
    }

    if (result.count('@') && s.size() > 1) {
        result.erase('@');
        result.merge(_getFirstOfString(s.substr(1)));
    }
    return result;
}

void Syntactic_Analysis::_getFirst_String()
{
    map<string, set<char>> first;
    for (auto const& [lhs, rhs] : Language) {
        for (string s : rhs) {
            first[s] = _getFirstOfString(s);
        }
    }
    First_String = first;
}

/*
* 计算所有非终结符的FOLLOW集
* FOLLOW(A)指的是在文法的所有句型中,可能紧跟在非终结符A后面的终结符的集合。
* FOLLOW集的计算遵循以下三条规则,直到所有集合不再变化为止:
*
* 1. 对于文法的起始符号S,将结束符'#'添加到FOLLOW(S)中。
*
* 2. 对于产生式 A -> αBβ:
* 将FIRST(β)中除了'@'之外的所有符号都加入到FOLLOW(B)中。
*
* 3. 对于产生式 A -> αB, 或者 A -> αBβ 且 FIRST(β)包含'@':
* 将FOLLOW(A)中的所有符号都加入到FOLLOW(B)中。
*
* 本函数通过循环迭代,不断应用规则2和3,直到所有FOLLOW集都收敛(不再变化)为止。
*/
void Syntactic_Analysis::_getFollow()
{
    map<char, set<char>> follow;
    // 初始化每个非终结符的FOLLOW集为空
    for (char ch : Non_Terminal_Symbol) {
        follow[ch] = {};
    }
    // 规则1: 将结束符'#'放入起始符号的FOLLOW集中
    follow[Start_Symbol] = { '#' };

    // 标记FOLLOW集是否发生变化,用于循环的终止判断
    bool changed = true;
    while (changed) {
        changed = false;
        // 遍历文法中的每一个产生式
        for (auto const& [lhs, rhs] : Language) {
            for (string s : rhs) {
                // 遍历产生式右部的每一个符号
                for (int i = 0; i < s.size(); i++) {
                    char current_symbol = s[i];

                    // 只处理非终结符
                    if (_isNonTerminalSymbol(current_symbol)) {

                        // 考虑 current_symbol (即B) 后面的部分 beta (即 s[i+1...])
                        string beta = s.substr(i + 1);
                        set<char> first_of_beta = _getFirstOfString(beta);

                        // 规则2: 将FIRST(beta)中所有非'@'符号加入FOLLOW(current_symbol)
                        for (char symbol : first_of_beta) {
                            if (symbol != '@') {
                                // insert().second 返回true说明插入了新元素,即集合发生了变化
                                if (follow[current_symbol].insert(symbol).second) {
                                    changed = true;
                                }
                            }
                        }

                        // 规则3: 如果FIRST(beta)包含'@'(或beta为空,此时_getFirstOfString返回{'@'})
                        // 则将FOLLOW(lhs)(即FOLLOW(A))加入FOLLOW(current_symbol)(即FOLLOW(B))
                        if (first_of_beta.count('@')) {
                            for (char symbol : follow[lhs]) {
                                if (follow[current_symbol].insert(symbol).second) {
                                    changed = true;
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    Follow = follow;
}

/*
* 根据栈顶非终结符(NT)和向前看终结符(T),获取应该使用的产生式
* 这个函数本质上是在模拟查询LL(1)分析表 M[NT, T] 的过程。
*
* 查询逻辑依据LL(1)分析表的构造规则:
* 1. 如果产生式为 A -> α, 且终结符 a 在 FIRST(α) 中, 那么 M[A, a] = (A -> α)。
* 2. 如果产生式为 A -> @, 那么对于FOLLOW(A)中的每一个终结符 b, M[A, b] = (A -> @)。
*
* 如果找不到匹配的产生式,说明这是一个语法错误,返回一个特殊的错误标记('0', "Error")。
*/
pair<char, string> Syntactic_Analysis::_getSentenceFrom2Symbols(char NT, char T)
{
    for (string s : Language[NT]) {
        if (s == "@")continue;
        if (First_String[s].count(T)) {
            return make_pair(NT, s);
        }
    }

    /*
    * 检查是否有@串
    */
    bool has_epsilon_production = false;
    for (const string& s : Language[NT]) {
        if (s == "@") {
            has_epsilon_production = true;
            break;
        }
    }

    if (has_epsilon_production) {
        // 检查 T 是否在 NT 的FOLLOW集中
        if (Follow.count(NT) && Follow[NT].count(T)) {
            return make_pair(NT, "@");
        }
    }

    // 如果所有规则都不匹配，则说明是语法错误
    return make_pair('0', "Error");
}

/*
* 拓扑排序 (Kahn's Algorithm 实现)
*
* 拓扑排序用于得到有向无环图(DAG)中节点的一个线性序列,该序列满足:
* 对于图中每一条有向边 (u, v), u 在序列中都出现在 v 的前面。
*
* 在我们的场景中,非终结符是节点,产生式 A -> B... 表示一条从 A 到 B 的依赖边。
* 我们用它来确定非终结符的代入顺序,以处理间接左递归等问题。
*
* 算法思想:
* 1. 计算所有节点的入度。
* 2. 将所有入度为0的节点放入一个队列。
* 3. 当队列不为空时,取出一个节点u,将其加入排序结果列表。
* 4. 遍历u的所有邻居v,将v的入度减1。如果v的入度变为0,则将v入队。
* 5. 重复步骤3和4,直到队列为空。
*
* 如果排序结果列表中的节点数不等于图中的节点总数,说明图中存在环路。
*/
vector<char> topological_sort(map<char, vector<char>>& graph)
{
    //入度
    map<char, int> in_degree;
    for (auto& [u, neighbors] : graph) {
        for (char v : neighbors) {
            in_degree[v]++;
        }
    }

    queue<char> q;
    for (auto& [u, _] : graph) {
        if (in_degree[u] == 0) q.push(u);
    }

    vector<char> order;
    while (!q.empty()) {
        char u = q.front(); q.pop();
        order.push_back(u);
        for (char v : graph[u]) {
            if (--in_degree[v] == 0) {
                q.push(v);
            }
        }
    }

    // 产生了环
    if (order.size() != graph.size()) {
        return {};
    }
    return vector<char>(order.rbegin(), order.rend());
}

/*
* 对给定的输入串s进行LL(1)语法分析
* 本函数是LL(1)总控分析程序,负责初始化分析栈、输入串,并驱动整个分析过程。
*
* - 分析栈(Q1): 初始状态为 #S (S为文法起始符号)
* - 余留输入串(Q2): 初始状态为 s# (s为待分析的输入串)
*
* 它通过一个循环,不断调用_printLineOfTable来执行单步分析,
* 直到分析成功(栈和输入串都只剩#)或失败(遇到错误)为止。
*/
bool Syntactic_Analysis::_isAvailableSentence(const string& s)
{
    // 初始化分析栈 Q1, 栈底为'#', 然后压入起始符号
    string Q1 = string(1, '#') + Start_Symbol;
    // 初始化输入串 Q2, 末尾添加结束符'#'
    string Q2 = s + '#';

    // 使用 iomanip 库来格式化输出, 创建一个美观的分析过程表
    cout << left << setw(8) << "步骤"
        << left << setw(30) << "分析栈"
        << left << setw(30) << "余留输入串"
        << left << "所用产生式" << endl;

    cout << string(80, '-') << endl; // 打印表头下的分隔线

    int i = 1;
    // 无限循环, 依赖内部逻辑(成功或失败)来跳出
    while (true) {
        // 调用单步分析函数, 并获取状态
        int status = _printLineOfTable(i++, Q1, Q2);
        // 如果status不为0, 说明分析过程已经结束
        if (status != 0) {
            // status为1代表成功, -1代表失败
            return status == 1;
        }
    }
}

int Syntactic_Analysis::_printLineOfTable(int SN, string& Q1, string& Q2)
{
    // 定义列宽以便对齐
    const int step_col = 8;
    const int stack_col = 30;
    const int input_col = 30;

    // 打印行号、分析栈和输入串
    cout << left << setw(step_col) << SN
        << left << setw(stack_col) << Q1
        << left << setw(input_col) << Q2;

    if (Q1 == "#" && Q2 == "#") {
        cout << "成功" << endl;
        return 1; // 成功
    }

    char X = Q1.back();
    char a = Q2[0];

    if (_isTerminalSymbol(X) && X == a) {
        cout << "匹配终结符: " << X << endl;
        Q1.pop_back();
        Q2 = Q2.substr(1);
        return 0; // 继续
    }

    if (_isNonTerminalSymbol(X)) {
        std::pair<char, std::string> production = _getSentenceFrom2Symbols(X, a);

        if (production.first == '0') {
            cout << "错误: 分析表 M[" << X << "," << a << "] 为空" << endl;
            return -1; // 错误
        }
        else {
            std::string rhs = production.second;
            cout << X << " -> " << (rhs == "@" ? "ε" : rhs) << endl;
            Way.push_back(production);

            Q1.pop_back();
            if (rhs != "@") {
                for (int k = rhs.length() - 1; k >= 0; --k) {
                    Q1.push_back(rhs[k]);
                }
            }
            return 0; // 继续
        }
    }

    // 其他所有未处理情况均为错误
    if (X == '#') {
        cout << "错误: 栈已空，但输入串未结束" << endl;
    }
    else {
        cout << "错误: 栈顶终结符 '" << X << "' 与输入符号 '" << a << "' 不匹配" << endl;
    }
    return -1; // 错误
}

void Syntactic_Analysis::_printFirstStringSets()
{
    std::cout << "\n--- FIRST 集 ---" << std::endl;
    // 为了输出顺序稳定，可以基于 _getOrder() 或者直接用 Non_Terminal_Symbol
    // 这里我们直接遍历 Language，map 会按键（LHS）排序
    for (const auto& entry : Language) {
        char lhs = entry.first;
        const std::vector<std::string>& rhs_vector = entry.second;
        if (rhs_vector.empty() && lhs == '0') continue; // 跳过可能的错误标记

        for (const std::string& s : rhs_vector) {
            std::cout << "FIRST(" << lhs << " -> " << (s.empty() ? "@" : s) << ") = { ";
            if (First_String.count(s)) {
                const std::set<char>& fs = First_String.at(s);
                bool first_token = true;
                for (char token : fs) {
                    if (!first_token) {
                        std::cout << ", ";
                    }
                    std::cout << token;
                    first_token = false;
                }
            }
            else {
                // 这种情况理论上不应该发生，如果 _getFirst_String() 被正确调用了
                std::cout << "(not computed for this string: \"" << s << "\")";
            }
            std::cout << " }" << std::endl;
        }
    }
    std::cout << "-----------------------------------" << std::endl;
}

void Syntactic_Analysis::_printFollowSets()
{
    std::cout << "\n--- FOLLOW 集 ---" << std::endl;
    // Non_Terminal_Symbol 应该是经过所有优化后最终的非终结符列表
    std::vector<char> nts_to_print = Non_Terminal_Symbol; // 创建副本以排序，不影响原数据
    std::sort(nts_to_print.begin(), nts_to_print.end()); // 按字母顺序打印

    for (char nt : nts_to_print) {
        std::cout << "FOLLOW(" << nt << ") = { ";
        if (Follow.count(nt)) {
            const std::set<char>& fs = Follow.at(nt);
            bool first_token = true;
            for (char token : fs) {
                if (!first_token) {
                    std::cout << ", ";
                }
                std::cout << token;
                first_token = false;
            }
        }
        std::cout << " }" << std::endl;
    }
    std::cout << "-------------------------------------" << std::endl;
}

void Syntactic_Analysis::_printLL1ParsingTable()
{
    std::cout << "\n--- LL(1) 分析表 ---" << std::endl;

    // 1. 收集表头（终结符列）
    std::set<char> table_terminals_set;
    for (char t : Terminal_Symbol) {
        if (t != '@') {
            table_terminals_set.insert(t);
        }
    }
    table_terminals_set.insert('#');
    std::vector<char> table_columns(table_terminals_set.begin(), table_terminals_set.end());
    std::sort(table_columns.begin(), table_columns.end());

    // 2. 打印表头
    const int col_width = 12;
    std::cout << std::left << std::setw(col_width) << "NT / T";
    for (char t_col : table_columns) {
        std::cout << "| " << std::left << std::setw(col_width - 2) << t_col;
    }
    std::cout << "|" << std::endl;
    // 打印分隔线
    for (int i = 0; i < (table_columns.size() + 1); ++i) {
        std::cout << std::string(col_width, '-') << (i == table_columns.size() ? "" : "+");
    }
    std::cout << std::endl;

    // 3. 构造并打印表内容，同时检查冲突
    std::vector<char> nts_to_print = Non_Terminal_Symbol;
    std::sort(nts_to_print.begin(), nts_to_print.end());

    bool conflict_found = false; // 冲突标志

    for (char nt_row : nts_to_print) {
        std::cout << std::left << std::setw(col_width) << nt_row;

        for (char t_col : table_columns) {

            // --- 新增的核心逻辑：为每个单元格寻找所有适用产生式 ---
            std::vector<std::string> applicable_productions;

            for (const std::string& rhs : Language[nt_row]) {
                // 情况一：对于 A -> α (α 不为 @)
                if (rhs != "@") {
                    std::set<char> first_of_rhs = _getFirstOfString(rhs);
                    if (first_of_rhs.count(t_col)) {
                        applicable_productions.push_back(rhs);
                    }
                }
                // 情况二：对于 A -> @
                else {
                    if (Follow.count(nt_row) && Follow.at(nt_row).count(t_col)) {
                        applicable_productions.push_back("@");
                    }
                }
            }
            // --- 逻辑结束 ---

            // 检查并处理冲突
            if (applicable_productions.size() > 1) {
                std::cout << "| " << std::left << std::setw(col_width - 2) << "冲突!";
                // 在循环结束后统一报告冲突细节
                conflict_found = true;
            }
            else if (applicable_productions.size() == 1) {
                std::string cell_content = std::string(1, nt_row) + "->" + applicable_productions[0];
                std::cout << "| " << std::left << std::setw(col_width - 2) << cell_content;
            }
            else {
                // size is 0, a normal error cell
                std::cout << "| " << std::left << std::setw(col_width - 2) << "";
            }
        }
        std::cout << "|" << std::endl;
    }
    std::cout << "------------------------------------" << std::endl;

    // 如果在填充过程中发现了任何冲突，现在中断并详细报告
    if (conflict_found) {
        std::cerr << "\n错误: 发现LL(1)分析表构造冲突！无法生成有效的LL(1)分析表。" << std::endl;
        std::cerr << "冲突详情如下:" << std::endl;

        for (char nt_row : nts_to_print) {
            for (char t_col : table_columns) {
                std::vector<std::string> prods;
                // 重新计算以找出冲突的具体内容
                for (const std::string& rhs : Language[nt_row]) {
                    if (rhs != "@") {
                        if (_getFirstOfString(rhs).count(t_col)) prods.push_back(rhs);
                    }
                    else {
                        if (Follow.count(nt_row) && Follow.at(nt_row).count(t_col)) prods.push_back("@");
                    }
                }
                if (prods.size() > 1) {
                    std::cerr << "  - 在 M[" << nt_row << ", " << t_col << "] 处, 以下产生式存在冲突:" << std::endl;
                    for (const auto& p : prods) {
                        std::cerr << "      " << nt_row << " -> " << p << std::endl;
                    }
                }
            }
        }
        return; // 中断函数
    }
}

void Syntactic_Analysis::_printDerivationSteps()
{
    if (Way.empty()) {
        std::cout << "\n--- 推 导 过 程 ---" << std::endl;
        std::cout << "无 推 导 记 录" << std::endl;
        std::cout << "--------------------------" << std::endl;
        return;
    }

    std::cout << "\n--- 推 导 过 程 ---" << std::endl;

    // 1. 获取起始符号
    if (Non_Terminal_Symbol.empty()) {
        std::cerr << "Error: Non_Terminal_Symbol list is empty, cannot determine start symbol for derivation." << std::endl;
        return;
    }
    char start_symbol = Start_Symbol;

    std::string current_sentential_form = "";
    current_sentential_form += start_symbol;

    // 2. 打印初始状态（起始符号）
    std::cout << current_sentential_form;

    // 3. 遍历 Way 中记录的每一步推导
    for (const auto& production_step : Way) {
        char lhs_to_replace = production_step.first;
        std::string rhs_to_insert = production_step.second;

        // 处理 epsilon 产生式
        if (rhs_to_insert == "@") {
            rhs_to_insert = ""; // 替换为空字符串
        }

        // 在当前句型中找到最左边的 lhs_to_replace
        // 对于LL(1)的最左推导，被替换的总是当前句型中最左边的非终结符
        // 而 Way 中记录的 lhs_to_replace 就是那个被选中的最左非终结符

        size_t pos = std::string::npos;
        // 查找第一个（最左边的）可以被替换的非终结符
        for (size_t i = 0; i < current_sentential_form.length(); ++i) {
            if (_isNonTerminalSymbol(current_sentential_form[i])) {
                if (current_sentential_form[i] == lhs_to_replace) {
                    pos = i;
                    break;
                }
            }
        }

        if (pos != std::string::npos) {
            // 执行替换
            current_sentential_form.replace(pos, 1, rhs_to_insert); // 替换1个字符 (LHS)
            std::cout << " -> " << current_sentential_form;
        }
        else {
            // 如果 Way 中的LHS在当前句型中找不到，说明 Way 的记录或推导逻辑可能有问题
            // 对于一个正确的LL(1)推导，这种情况不应该发生
            std::cout << " -> Error: Could not find non-terminal '" << lhs_to_replace
                << "' to replace in current form '" << current_sentential_form << "'" << std::endl;
            std::cerr << "Error in derivation logic: Mismatch between recorded productions and sentential form." << std::endl;
            return; // 出现逻辑错误，终止打印
        }
    }
    std::cout << std::endl; // 结束换行
    std::cout << "--------------------------" << std::endl;
}

void Syntactic_Analysis::Analysis(const string& path, const string& s)
{
    // _getLanguage 应该负责完成所有文法加载、转换、
    // 以及 First_Char, First_String, Follow 的计算
    _getLanguage(path); // 这个函数内部会输出原始文法和转换过程信息
    // 输出最终的非终结符和终结符列表（可选，但有助于调试）
    std::cout << "\n最终非终结符列表: ";
    for (char ch : Non_Terminal_Symbol) {
        std::cout << ch << '\t';
    }
    std::cout << std::endl;

    std::cout << "最终终结符列表: ";
    for (char ch : Terminal_Symbol) {
        std::cout << ch << '\t';
    }
    std::cout << std::endl;

    // 在 _getLanguage 完成所有工作后，我们再输出 First, Follow 和 LL(1) 表
    // 这些应该是针对最终转换后的文法

    _printFirstStringSets(); // 输出每个产生式右部的FIRST集
    _printFollowSets();    // 输出每个非终结符的FOLLOW集
    _printLL1ParsingTable(); // 输出LL(1)分析表
    _isAvailableSentence(s);
    _printDerivationSteps();
}

