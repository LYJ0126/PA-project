/***************************************************************************************
* Copyright (c) 2014-2022 Zihao Yu, Nanjing University
*
* NEMU is licensed under Mulan PSL v2.
* You can use this software according to the terms and conditions of the Mulan PSL v2.
* You may obtain a copy of Mulan PSL v2 at:
*          http://license.coscl.org.cn/MulanPSL2
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*
* See the Mulan PSL v2 for more details.
***************************************************************************************/

#include <isa.h>

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <regex.h>

enum {
  TK_NOTYPE = 256, TK_EQ,
  /* TODO: Add more token types */
	TK_NUMBER = 255, 
	TK_HEX = 254,
	TK_DEREF = 253,
};

static struct rule {
  const char *regex;
  int token_type;
} rules[] = {

  /* TODO: Add more rules.
   * Pay attention to the precedence level of different rules.
   */

  {" +", TK_NOTYPE},    // spaces
  {"\\+", '+'},         // plus
  {"==", TK_EQ},        // equal
	{"-",'-'},            // minus
	{"\\*",'*'},          // multiply
	{"/",'/'},					  // divide  
	{"0x[0-9,a-z,A-Z]",TK_HEX}, //hex number
	{"[0-9]+",TK_NUMBER},   // number
	{"\\(",'('},					// left bracket
	{"\\)",')'},					// right bracket
	{"\\*",TK_DEREF},			  // dereference
};

#define NR_REGEX ARRLEN(rules)

static regex_t re[NR_REGEX] = {};

/* Rules are used for many times.
 * Therefore we compile them only once before any usage.
 */
void init_regex() {
  int i;
  char error_msg[128];
  int ret;

  for (i = 0; i < NR_REGEX; i ++) {
    ret = regcomp(&re[i], rules[i].regex, REG_EXTENDED);
    if (ret != 0) {
      regerror(ret, &re[i], error_msg, 128);
      panic("regex compilation failed: %s\n%s", error_msg, rules[i].regex);
    }
  }
}

typedef struct token {
  int type;
  char str[32];
} Token;

static Token tokens[32] __attribute__((used)) = {};
static int nr_token __attribute__((used))  = 0;

static bool make_token(char *e) {
  int position = 0;
  int i;
  regmatch_t pmatch;

  nr_token = 0;

  while (e[position]!='\0') {
    /* Try all rules one by one. */
		if(nr_token>31){
			printf("输入表达式的token数超过了缓冲区长度\n");
			return false;
		}
    for (i = 0; i < NR_REGEX; i ++) {
      if (regexec(&re[i], e + position, 1, &pmatch, 0) == 0 && pmatch.rm_so == 0) {
        char *substr_start = e + position;
        int substr_len = pmatch.rm_eo;

        Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s",
            i, rules[i].regex, position, substr_len, substr_len, substr_start);

        position += substr_len;

        /* TODO: Now a new token is recognized with rules[i]. Add codes
         * to record the token in the array `tokens'. For certain types
         * of tokens, some extra actions should be performed.
         */

        switch (rules[i].token_type) {
					case TK_NOTYPE: break;
					case '+': tokens[nr_token].type = (int)'+';
										nr_token++;
										break;
					case '-': tokens[nr_token].type = (int)'-';
										nr_token++;
										break;
					case '*': tokens[nr_token].type = (int)'*';
										nr_token++;
										break;
					case '/': tokens[nr_token].type = (int)'/';
										nr_token++;
										break;
					case '(': tokens[nr_token].type = (int)'(';
										nr_token++;
										break;
					case ')': tokens[nr_token].type = (int)')';
										nr_token++;
										break;
					case TK_NUMBER: {
															if(substr_len>32){
																printf("输入的数长度超过了缓冲区长度\n");
																return false;
															}
															else{
																tokens[nr_token].type = TK_NUMBER;
																for(int i=substr_len-1;i>=0;i--){
																	tokens[nr_token].str[i]=e[position-(substr_len-i)];
																}
																if(substr_len<32){
																	tokens[nr_token].str[substr_len]='\0';
																}
																nr_token++;
															}
															break;
														}
          default: TODO();
        }

        break;
      }
    }

    if (i == NR_REGEX) {
      printf("no match at position %d\n%s\n%*.s^\n", position, e, position, "");
      return false;
    }
  }

  return true;
}

bool flag=true;//flag用于在表达式求值中间判断表达式是否合法，以及最后expr函数中判断表达式是否合法
bool check_parentheses(int p, int q)
{
	bool flag2=true;//flag2是返回值,表示表达式是否被一对匹配的括号包围,并且表达式括号是否合法;若都满足,则为true,否则为false
	if(tokens[p].type!='('||tokens[q].type!=')'){
		flag2=false;
	}
	int parmatch=0;//用于检测该段表达式的括号是否匹配(合法)
	for(int i=p;i<=q;i++){
		if(tokens[i].type=='(') ++parmatch;
		else if(tokens[i].type==')') --parmatch;
		if(parmatch<0){
			flag=false;//括号检测发现不合法
			flag2=false;
			return false;
		}
		if(parmatch==0&&i<q) flag2=false;//最左边的括号并没有与最右边的括号匹配
	}
	if(parmatch!=0){//最后括号不能匹配,不合法
		flag=false;
		flag2=false;
	}
	return flag2;
}
long long eval(int p, int q)
{
	if(flag==false) return 0;
	if(p>q){//Bad expression
		flag=false;
		return 0;
	}
	else if(p==q){
		 /* Single token.
     * For now this token should be a number.
     * Return the value of the number.
     */
		 if(tokens[p].str[0]<48||tokens[p].str[0]>57){//不是数
			 flag=false;
			 return 0;
		 }
		 int t=0;
		 long long num=0;
		 while(t<32&&tokens[p].str[t]!='\0'){
			 num=num*10+(tokens[p].str[t]-'0');
			 t++;
		 }
		 return num;
	}
	else if(check_parentheses(p, q) == true){
		/* The expression is surrounded by a matched pair of parentheses.
     * If that is the case, just throw away the parentheses.
     */
		 return eval(p+1,q-1);
	}
	else{
		if(flag==false) return 0;//在上一步括号检测的时候发现不合法
		int parmatch=0;//记录括号匹配情况
		char op='@';
		int level = 2;//当前最低优先级
		int oppos=p;//最低优先级的位置
		for(int i=p;i<=q;i++){
			if(tokens[i].type=='(') ++parmatch;
			else if(tokens[i].type==')') --parmatch;
			if(parmatch==0){
				if(tokens[i].type==(int)'+'||tokens[i].type==(int)'-'){
					level=1;
					op=(char)tokens[i].type;
					oppos=i;
				}
				else if(tokens[i].type==(int)'*'||tokens[i].type==(int)'/'){
					if(level>=2){
						level=2;
						op=(char)tokens[i].type;
						oppos=i;
					}
				}
			}
		}
		if(op=='@'){//没有符合条件的运算符
			flag=false;
			return 0;
		}
		long long val1 = eval(p,oppos-1);
		long long val2 = eval(oppos+1,q);
		switch(op){
			case '+': return val1+val2;
								break;
			case '-': return val1-val2;
								break;
			case '*': return val1*val2;
								break;
			case '/': return val1/val2;
								break;
			default: assert(0);
		}
	}
	return 0;
}

word_t expr(char *e, bool *success) {
	flag = true;// 记得初始化
  if (!make_token(e)) {
    *success = false;
		printf("success:%d\n",*success);
    return 0;
  }

  /* TODO: Insert codes to evaluate the expression. */
	long long vvalue = eval(0,nr_token-1);
	printf("flag:%d\n",flag);
	if(flag){
		*success = true;
		printf("success:%d\n",*success);
		return (uint32_t)vvalue;
	}
	else {
		*success = false;
		printf("success:%d\n",*success);
		return 0;
	}
  return 0;
}
