//
//  main.cpp
//  24point
//
//  Created by 绯浅yousa on 16/8/1.
//  Copyright © 2016年 绯浅yousa. All rights reserved.
//


/*
 * 下周搞这个初音，镜头+补贴图。。现在啥都没-  -
 * 现在先搞这个24点
 */

/* CORE QUESTION
 *
 * 1.如何找到答案？参考编程之美，使用集合论解决(这个思路有趣),8.6
 * 2.找到答案后如何输出运算结果？……,8.9
 *************************************************
 * KEEP ON MOVING
 *
 * 1.如果没有答案如何判断？OK:8.10
 * 1.1 删除冗余代码:8.10
 * 2.配套标准接口,似乎还有点问题。OK搞定。:8.10
 * 3.支持变长入参，比如4-10（这里因为是采用空间换时间的策略，
 * 所以个人并不建议支持太大，而且我还没做第二层，只得舍去比如
 * n>12的场景or15，这个得测试一下了。）8:10
 * 昨天完成到这里。
 * PS:n=52可以直接输出结果，别问我为啥。:8.11
 * 4.准备测试用例。。
 * 5.衔接多次调用,需要添加一个初始化函数StateInit();8.11
 * 6.List优化
 * 7.其他优化
 * 8.clean code
 */

#include <algorithm>
#include <string>
#include <sstream>
#include <list>
#include <cmath>
#include <climits>
#include <bitset>
#include <iostream>
#include <vector>
#include <map>

using namespace std;


/* 本次计算有多少张卡牌 */
int g_number_cnt = 0;
/* 本次计算状态数 */
int g_state_cnt = 0;
/* 最大支持计算的卡牌数。这里取18 */
const int MAX_NUMBER_COUNT  = 16;
/* 采用集合论的方法，故其实是2^n个状态 */
const int MAX_STATE_COUNT   = (1 << MAX_NUMBER_COUNT);
/* 计算24点的每个数的最大值。这里是13 */
const int MAX_NUMBER    = 13;
/* 目标值24 */
const int EXPECTION = 24;
/* 状态集合中的最大元素数值 */
//TODO,这里有一点问题，因为考虑到有些情况，它会超过maxvalue
const int MAX_VALUE = EXPECTION * MAX_NUMBER * MAX_NUMBER_COUNT;

struct elementNode {
    int value;//本Node的值
    int left, right;//该值由哪两个State的index计算而来
    int left_value, right_value;//这两个State中的value键值
    char opr;//运算符号
};

typedef map<int, elementNode> elementMap;

struct State {
    bitset<MAX_VALUE+10> exist;
    elementMap element_map;//这个应该可以优化成map……，value是键值
};

/*
 * global var
 */
int number[MAX_NUMBER_COUNT];
State state[MAX_STATE_COUNT];
string answer;//每次使用reset 一次

/*
 * declaration
 */
void ReadData();
void StateInit();
void StateMerge(int a, int b, int x);
void Solve();
bool PrintExpression(elementNode node);
bool Output();
bool Calculate24Point();
string num2Card(int num);
int card2Num(char Point);
std::string Calc24(const std::vector<unsigned char>& InputCards);

/*
 * 手动测试赋值用
 */
void ReadData()
{
    /*
     number[0] = 1;
     number[1] = 2;
     number[2] = 3;
     number[3] = 4;
     number[4] = 5;
     number[5] = 6;
     number[6] = 3;
     */
    number[0] = 1;
    number[1] = 1;
    number[2] = 1;
    number[3] = 1;
    number[4] = 1;
    number[5] = 1;
    number[6] = 2;
    
    //expection = 24;
}

/*
 * 初始化集合，即二进制表示1仅有一个的数组。
 */
void StateInit()
{
    elementNode node ;
    
    for (int i = 0; i < g_state_cnt; i++) {
        state[i].element_map.clear();
        state[i].exist.reset();
    }
    
    answer = "";
    
    for (int i = 0; i < g_number_cnt; i++) {
        node.value              = number[i];
        node.left = node.right = -1;
        //state[(1 << i)].nodelist.push_back(node);
        //element_map
        state[(1 << i)].element_map[node.value] = node;
        state[(1 << i)].exist[node.value] = true;
    }
}

/*
 * 合并状态集合
 *
 * 做的特殊处理：
 * 1.丢弃除不尽
 * 2.丢弃过大，不超过24*13
 * 3.-和/考虑顺序，+和*不需考虑顺序
 * 4.乘法考虑越界，越界就丢弃。
 */

void StateMerge(int a, int b, int x)
{
    elementNode node;
    elementMap::iterator i, j;
    
    for (i = state[a].element_map.begin(); i != state[a].element_map.end(); i++){
        for (j = state[b].element_map.begin(); j != state[b].element_map.end(); j++){
            
            /*
             ************************************************
             * +
             ************************************************
             */
            
            //node.value = (*i).value + (*j).value;
            node.value = i->second.value + j->second.value;
            
            if ( (node.value <= MAX_VALUE) && (!state[x].exist[node.value]) ) {
                node.left  = a;
                node.right = b;
                node.left_value  = i->second.value;
                node.right_value = j->second.value;
                node.opr   = '+';
                
                //state[x].nodelist.push_back(node);
                state[x].element_map[node.value] = node;
                state[x].exist[node.value] = true;
            }
            
            /*
             ************************************************
             * *
             ************************************************
             */
            
            //double tmp = double((*i).value) * double((*j).value);
            double tmp_max = double(i->second.value) * double(j->second.value);
            
            if (tmp_max < INT_MAX) {
                node.value = i->second.value * j->second.value;
                
                if ( (node.value <= MAX_VALUE) && (!state[x].exist[node.value]) ){
                    node.left  = a;
                    node.right = b;
                    node.left_value  = i->second.value;
                    node.right_value = j->second.value;
                    node.opr   = '*';
                    
                    //state[x].nodelist.push_back(node);
                    state[x].element_map[node.value] = node;
                    state[x].exist[node.value] = true;
                }
            }
            
            /*
             ************************************************
             * -
             ************************************************
             */
            
            if (i->second.value >= j->second.value) {
                node.value = i->second.value - j->second.value;
                node.left  = a;
                node.right = b;
                node.left_value  = i->second.value;
                node.right_value = j->second.value;
                node.opr   = '-';
            } else {
                node.value = j->second.value - i->second.value;
                node.left  = b;
                node.right = a;
                node.left_value  = j->second.value;
                node.right_value = i->second.value;
                node.opr   = '-';
            }
            
            if ( (node.value <= MAX_VALUE) && (!state[x].exist[node.value]) ) {
                //state[x].nodelist.push_back(node);
                state[x].element_map[node.value] = node;
                state[x].exist[node.value] = true;
            }
            
            /*
             ************************************************
             * /
             ************************************************
             */
            
            
            if ( (j->second.value != 0) && (i->second.value >= j->second.value) && (i->second.value % j->second.value == 0) )
            {
                node.value = i->second.value / j->second.value;
                node.left  = a;
                node.right = b;
                node.left_value  = i->second.value;
                node.right_value = j->second.value;
                node.opr   = '/';
            } else if ( (i->second.value != 0) && (j->second.value >= i->second.value) && (j->second.value % i->second.value == 0) )
            {
                node.value = j->second.value / i->second.value;
                node.left  = b;
                node.right = a;
                node.left_value  = j->second.value;
                node.right_value = i->second.value;
                node.opr   = '/';
            }
            
            if ( (node.value <= MAX_VALUE) && (!state[x].exist[node.value]) ){
                //state[x].nodelist.push_back(node);
                state[x].element_map[node.value] = node;
                state[x].exist[node.value] = true;
            }
            
        }
    }
}

/*
 * 循环
 */
void Solve()
{
    StateInit();
    
    /* 循环：
     * 这里有一次重复，通过if i < j解决，8.11
     * for x ← 1 to 2^n-1 do
     *   begin
     *     for i ← 1to x-1 do
     *     begin
     *       if x∧i=i then
     *       begin
     *         j ← x – i;
     *         if i < j
     *           then F[x] ← F[x] + comp(F[i],F[j]);
     *       end;
     *     end;
     *   end;
     */
    for (int x = 2; x < g_state_cnt; x++) {
        for (int i = 1; i < x; i++) {
            if ( (x & i) == i ) {
                int j = x - i;
                if (i <= j) {
                    StateMerge(i, j, x);
                }
            }
        }
    }
}


bool PrintExpression(elementNode node)
{
    bool ret;
    
    if (node.left == -1) {
        //cout << node.value;
        answer = answer + num2Card(node.value);
    } else {
        elementMap::iterator iter;
        
        //cout << "(";
        answer = answer + "(";
        
        iter = state[node.left].element_map.find(node.left_value);
        if (iter == state[node.left].element_map.end()) {
            return false;
        }
        
        ret = PrintExpression(iter->second);
        if (ret == false) {
            return ret;
        }
        
        //cout << node.opr;
        answer = answer + node.opr;
        
        iter = state[node.right].element_map.find(node.right_value);
        if (iter == state[node.right].element_map.end()) {
            return false;
        }
        
        ret = PrintExpression(iter->second);
        if (ret == false) {
            return ret;
        }
        
        //cout << ")";
        answer = answer + ")";
    }
    
    return true;
}

/*
 * 看全集中是否有24这个结果
 */

bool Output()
{
    elementMap::iterator iter;
    bool ret = false;
    
    elementMap& node_map = state[g_state_cnt-1].element_map;
    
    iter = node_map.find(EXPECTION);
    if (iter == node_map.end()) {
        //cout<<"we can't fine the solution"<<endl;
        return false;
    }
    
    ret = PrintExpression(iter->second);
    if (ret == false) {
        return ret;
    }
    //cout<<endl;
    
    return true;
}

/*
 * 计算24点
 */
bool Calculate24Point()
{
    bool ret = false;
    
    //ReadData();

    Solve();

    ret = Output();
    
    return ret;
}

/*
 * num转换成卡牌值
 */
string num2Card(int num)
{
    /* 1:A
     * 2-9:
     * 10:"10"
     * 11:"J"
     * 12:"Q"
     * 13:"K"
     */
    stringstream ssCard;
    
    switch (num) {
        case 1:
            return "A";
        case 2:
        case 3:
        case 4:
        case 5:
        case 6:
        case 7:
        case 8:
        case 9:
            ssCard<<num;
            return ssCard.str();
        case 10:
            return "T";
        case 11:
            return "J";
        case 12:
            return "Q";
        case 13:
            return "K";
            
        default:
            break;
    }
    return "0";
}

/*
 * 卡牌值转换成数字
 */
int card2Num(char Point)
{
    /* A:1
     * 2-9:2-9
     * T:10
     * J:11
     * Q:12
     * K:13
     */
    
    switch (Point) {
        case 'A':
            return 1;
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
            return Point-'0';
        case 'T':
            return 10;
        case 'J':
            return 11;
        case 'Q':
            return 12;
        case 'K':
            return 13;
        default:
            return 0;
    }
    
    return 0;
}

/*
 * API接口，你懂的
 */

#define ANSWER_54 "A-A+2-2+3-3+4-4+5-5+6-6+7-7+8-8+9-9+T-T+J-J+K-K+A-A+2-2+3-3+4-4+5-5+6-6+7-7+8-8+9-9+T-T+J-J+K-K+Q+Q+Q-Q"

std::string Calc24(const std::vector<unsigned char>& InputCards)
{
    bool ret;
    size_t size = InputCards.size();
    
    if (size == 52) {
        return ANSWER_54;
    }
    
    if (size > MAX_NUMBER_COUNT) {
        return "NA";
    }
    
    for (int i = 0; i < size; i ++) {
        number[i] = card2Num(InputCards[i]);
    }
    
    g_number_cnt = (int)size;
    g_state_cnt = 1<<g_number_cnt;
    
    ret = Calculate24Point();
    if (ret == false) {
        return "NA";
    }
    
    return answer;//这里用一个全局变量来存储结果吧。
}

/*
 * 测试器……
 */

char card_arr[52] = {'A', '2', '3', '4', '5', '6', '7', '8', '9', 'T', 'J', 'Q', 'K',
                 'A', '2', '3', '4', '5', '6', '7', '8', '9', 'T', 'J', 'Q', 'K',
                 'A', '2', '3', '4', '5', '6', '7', '8', '9', 'T', 'J', 'Q', 'K',
                 'A', '2', '3', '4', '5', '6', '7', '8', '9', 'T', 'J', 'Q', 'K'};

#define MAX 52

void test()
{
    vector<unsigned char> TestInputCards(4);
    
    for (int i = 0; i < MAX - 3; i++) {
        for (int j = i + 1; j < MAX - 2; j++) {
            for (int m = j + 1; m < MAX - 1; m ++) {
                for (int n = m + 1; n < MAX ; n++) {
                    TestInputCards[0] = card_arr[i];
                    TestInputCards[1] = card_arr[j];
                    TestInputCards[2] = card_arr[m];
                    TestInputCards[3] = card_arr[n];
                    cout<<TestInputCards[0]<<" "<<TestInputCards[1]<<" "<<TestInputCards[2]<<" "<<TestInputCards[3]<<endl;
                    cout<<Calc24(TestInputCards)<<endl;
                }
            }
        }
    }
}
/*
 * 接下来是补充测试用例，但是写测试用例是需要按照一个原则来写的。首先是代码覆盖，
 * 然后考虑边界场景，再需要抵抗异常输入。还要考虑24点的经典6种公式是否都可以表达。
 */
int main() 
{
    test();
    
#if 0
    /*
    vector<unsigned char> InputCards(6);
    
    InputCards[0] = 'A';
    InputCards[1] = '2';
    InputCards[2] = '4';
    InputCards[3] = '4';
    InputCards[4] = '5';
    InputCards[5] = '6';
    //InputCards[6] = 'T';
    
    cout<<Calc24(InputCards)<<endl;
    
    vector<unsigned char> InputCards2(52);
    
    cout<<Calc24(InputCards2)<<endl;
    */
    
    
    vector<unsigned char> InputCards3(4);
    
    InputCards3[0] = 'A';
    InputCards3[1] = '2';
    InputCards3[2] = 'A';
    InputCards3[3] = '7';
    
    cout<<Calc24(InputCards3)<<endl;
    
    vector<unsigned char> InputCards4(9);
    
    InputCards4[0] = 'A';
    InputCards4[1] = 'A';
    InputCards4[2] = 'A';
    InputCards4[3] = 'A';
    InputCards4[4] = 'A';
    InputCards4[5] = 'A';
    InputCards4[6] = 'A';
    InputCards4[7] = 'A';
    InputCards4[8] = 'A';
    
    cout<<Calc24(InputCards4)<<endl;
    
    vector<unsigned char> InputCards5(8);
    
    InputCards5[0] = 'A';
    InputCards5[1] = 'A';
    InputCards5[2] = 'A';
    InputCards5[3] = 'A';
    InputCards5[4] = 'A';
    InputCards5[5] = 'A';
    InputCards5[6] = 'A';
    InputCards5[7] = 'A';
    
    cout<<Calc24(InputCards5)<<endl;
    
    vector<unsigned char> InputCards6(21);
    
    cout<<Calc24(InputCards6)<<endl;
    
    /*
    vector<unsigned char> InputCards7(16);
    
    InputCards7[0] = 'A';
    InputCards7[1] = 'A';
    InputCards7[2] = 'A';
    InputCards7[3] = 'A';
    InputCards7[4] = 'A';
    InputCards7[5] = 'A';
    InputCards7[6] = 'A';
    InputCards7[7] = 'A';
    InputCards7[8] = 'A';
    InputCards7[9] = 'A';
    InputCards7[10] = 'A';
    InputCards7[11] = 'A';
    InputCards7[12] = 'A';
    InputCards7[13] = 'A';
    
    InputCards7[14] = 'A';
    InputCards7[15] = 'A';
    
    cout<<Calc24(InputCards7)<<endl;
    */
#endif
    
    return 0; 
}