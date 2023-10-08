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
#include <memory/paddr.h> 
#include <cpu/cpu.h>
/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <regex.h>

extern word_t paddr_read(paddr_t addr, int len);
enum {
  TK_NOTYPE = 256, TK_EQ,
  /* TODO: Add more token types */
	TK_DEREF = 30,
	TK_LE = 29,
	TK_AND = 28,
	TK_OR = 27,
	TK_NE = 26,
	TK_EQUAL = 25,
	TK_NEGNUM = 24,
	TK_HEX = 23,
	TK_REG = 22,
	TK_NUMBER = 21,
};

static struct rule {
  const char *regex;
  int token_type;
} rules[] = {

  /* TODO: Add more rules.
   * Pay attention to the precedence level of different rules.
   */
//注意顺序(优先级)
  {" +", TK_NOTYPE},    // spaces
  {"\\+", '+'},         // plus
	{"-",'-'},            // minus,后面注意和负数区分
	{"\\*",'*'},          // multiply,后面注意和解引用区分
	{"/",'/'},					  // divide  
	{"\\(",'('},					// left bracket
	{"\\)",')'},					// right bracket
	{"\\=\\=",TK_EQUAL},	// expression equal
	{"\\!\\=",TK_NE},			// not equal
	{"\\<\\=",TK_LE},			// less than or equal
	{"\\&\\&",TK_AND},		// and
	{"\\|\\|",TK_OR},			// or
	{"\\$[a-z,A-Z]*[0-9]*",TK_REG},// register
//	{"==", TK_EQ},        // equal
	{"0[x,X][0-9,a-z,A-Z]+",TK_HEX}, //hex number
  {"[0-9]+",TK_NUMBER},   // number
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
		if(nr_token>30){
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
					case TK_NE: tokens[nr_token].type = TK_NE;
											nr_token++;
											break;
					case TK_EQUAL: tokens[nr_token].type = TK_EQUAL;
												 nr_token++;
												 break;
					case TK_LE: tokens[nr_token].type = TK_LE;
											nr_token++;
											break;
					case TK_AND: tokens[nr_token].type = TK_AND;
											 nr_token++;
											 break;
					case TK_OR: tokens[nr_token].type = TK_OR;
											nr_token++;
											break;
					case TK_REG:{
											 if(substr_len-1>31){
												 printf("输入的寄存器名字串长度超过了缓冲区长度\n");
												 return false;
											 }
											 else {
															tokens[nr_token].type = TK_REG;
															for(int i=substr_len-1;i>0;--i){//i到1即可,不需要把第一个'$'记录进去
																tokens[nr_token].str[i-1]=e[position-(substr_len-i)];
															}
															nr_token++;
											 }
											 break;
											} 
					case TK_HEX: {	
													if(substr_len-2>31){
														printf("输出的数长度超过了缓冲区长度\n");
														return false;
													}
													else {
														tokens[nr_token].type = TK_HEX;
														for(int i=substr_len-1;i>=2;--i){//i到2即可,不需要把前两个"0x"记录进去
															tokens[nr_token].str[i-2] = e[position-(substr_len-i)];
														}
														nr_token++;
													}
													break;
											 }
					case TK_NUMBER: {
															if(substr_len>31){
																printf("输入的数长度超过了缓冲区长度\n");
																return false;
															}
															else{
																tokens[nr_token].type = TK_NUMBER;
																for(int i=substr_len-1;i>=0;--i){
																	tokens[nr_token].str[i]=e[position-(substr_len-i)];
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
	//对负数,解引用的特殊处理
	//判断是否是解引用
	for(int i=0;i<nr_token;i++){
		if(tokens[i].type==(int)'*'&&(i==0||(tokens[i-1].type!=TK_NUMBER&&tokens[i-1].type!=TK_HEX&&tokens[i-1].type!=(int)')'))){
			tokens[i].type = TK_DEREF;
		}
	}
	//判断是否是负数
	for(int i=0;i<nr_token;i++){
		if(tokens[i].type==(int)'-'&&(i==0||(tokens[i-1].type!=TK_NUMBER&&tokens[i-1].type!=TK_HEX&&tokens[i-1].type!=(int)')'))){
			tokens[i].type = TK_NEGNUM;
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
		 //先判断是否是访问寄存器
		 if(tokens[p].type==TK_REG){
			 bool* regexist = (bool*)malloc(sizeof(bool));
			 uint32_t regnum = isa_reg_str2val(tokens[p].str,regexist);
			 if(*regexist==false){
				 flag=false;
				 free(regexist);
				 return 0;
			 }
			 else{
				 free(regexist);
				 return regnum;
			 }
		 }
		 //下面分十进制数和十六进制数讨论
		 printf("flag:%d\n",flag);
		 printf("%s\n",tokens[p].str);
		 //十六进制:
		 if(tokens[p].type==TK_HEX){
			 uint32_t hexnum=0;
			 sscanf(tokens[p].str,"%x",&hexnum);
			 return hexnum;
		 }
		 //十进制
		 if(tokens[p].str[0]<48||tokens[p].str[0]>57){//不是数或为空
        flag=false;
        return 0;
		 }
		 int t=0;
		 uint32_t num=0;
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
		int op=(int)'@';
		int level = 6;//当前最低优先级,每次取最低优先级的运算符作为根
		int oppos=p;//最低优先级的位置
		for(int i=p;i<=q;i++){
			if(tokens[i].type=='(') ++parmatch;
			else if(tokens[i].type==')') --parmatch;
			if(parmatch==0){
				if(tokens[i].type==TK_OR){
					level = 0;
					op = tokens[i].type;
					oppos = i;
				}
				else if(tokens[i].type==TK_AND){
					if(level>=1){
						level=1;
						op = tokens[i].type;
						oppos = i;
					}
				}
				else if(tokens[i].type==TK_EQUAL||tokens[i].type==TK_NE||tokens[i].type==TK_LE){
					if(level>=2){
						level = 2;
						op = tokens[i].type;
						oppos = i;
					}
				}
				else if(tokens[i].type==(int)'+'||tokens[i].type==(int)'-'){
					if(level>=3){
						level = 3;
						op = tokens[i].type;
						oppos = i;
					}
				}
				else if(tokens[i].type==(int)'*'||tokens[i].type==(int)'/'){
					if(level>=4){
						level = 4;
						op = tokens[i].type;
						oppos = i;
					}
				}
			}
		}
		if(op==(int)'@'){//没有符合条件的算术以及逻辑运算符
			//判断是否是一元运算符(解引用以及取负)
			if(tokens[p].type==TK_NEGNUM){//取负
				if(level>=5){
					level = 5;
					op = tokens[p].type;
					oppos = p;
					return -eval(oppos+1,q);
				}
			}
			else if(tokens[p].type==TK_DEREF){//解引用
				if(level>=5){
					level = 5;
					op = tokens[p].type;
					oppos = p;
					return paddr_read((uint32_t)eval(oppos+1,q),4);
				}
			}
			else{
				flag = false;
				return 0;
			}
		}
		long long val1 = eval(p,oppos-1);
		long long val2 = eval(oppos+1,q);
		switch(op){
			case (int)'+': return val1+val2;
										 break;
			case (int)'-': return val1-val2;
										 break;
			case (int)'*': return val1*val2;
										 break;
			case (int)'/': if(val2==0){
												printf("除数为0,错误\n");
												flag = false;
												return 0;
										 }
										 return val1/val2;
										 break;
			case TK_EQUAL: return val1==val2;
										 break;
			case TK_NE: return val1!=val2;
									break;
			case TK_LE: return val1<=val2;
									break;
			case TK_AND: return val1&&val2;
									 break;
			case TK_OR: return val1||val2;
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
    return 0;
  }

  /* TODO: Insert codes to evaluate the expression. */
	long long vvalue = eval(0,nr_token-1);
	if(flag){
		*success = true;
		return (uint32_t)vvalue;
	}
	else {
		*success = false;
		return 0;
	}
  return 0;
}
