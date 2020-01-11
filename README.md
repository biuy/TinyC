# 编译原理大作业

> **姓名：毕瑜**
>
> **学号：PB1606**

[TOC]

## 1 TinyC工程说明

### 1.1 词法分析 LexicalAnalysis.c

词法分析主要由`nextToken()`函数识别，并返回记号流中的下一个符号到语法分析中。 

词法基本不变。

letter -> a..z | A..Z | _

digit -> 0..9

reloop -> < | = | > | ! | & | ‘|’

calop -> + | - | * | /

id -> letter\_(letter\_|digit)\*

number -> digit digit*

delim -> blank | tab | newline

ws -> | /* | */ | //

symbol -> ( | ) | , | ; | { | } 

赋值号=、非运算符!被归入关系符中分析，然后在细分关系符时再筛选出来。

关键字归入id中分析，然后再通过判断筛选出来。

>词法分析主要修改了DFA转换表，补充了逻辑运算实现。

#### 1.1.1 无法读入换行符

<font color=red>错误</font>	无法读入换行符，文件读入时遇到错误退出。

> 对于换行符，windows的换行是\r\n，unix的是\n，mac的是\r。

windows的换行是`\r\n`，因此也需要识别一下`\r`。

`nextToken()`函数中

```c
else if (c==' ' || c=='\t' || c=='\n')
```

<font color=gren>修改</font>

```c
else if (c==' ' || c=='\t' || c=='\n' || c=='\r')
```

#### 1.1.2 读入==、!=、&&、||

<font color=red>错误</font>	遇到 ==、!=、&&、|| 多字符符号时只读了第一个字符。

```c
main()
{ int i,j;
  
  i=1;j=1;
  if(i == 1 && j == 1){
  	i = 3;
  }
  show(i);
}
```

查看以上示例词法分析的错误输出

```c
LEX: 2,main
LEX: 21
LEX: 22
LEX: 23
LEX: 56
LEX: 2,i
LEX: 25
LEX: 2,j
LEX: 25
LEX: 2,k
LEX: 26
LEX: 2,i
LEX: 27	//应该是'=='而不是'='
LEX: 1,1
LEX: 26	//应该是'&&'而不是';'
LEX: 2,j
LEX: 27	//应该是'=='而不是'='
LEX: 1,1
LEX: 26
LEX: 58
LEX: 21
LEX: 2,i
LEX: 15
LEX: 27
LEX: 1,1
LEX: 51
```

可见遇到 ==、!=、&&、|| 多字符符号时只读了第一个字符。

`FoundRELOOP()`函数中

```c
static int FoundRELOOP()
{
	if (tokenStr[0]=='<' && tokenStr[1]!='=') return(SYN_LT);
	else if (tokenStr[0]=='<' && tokenStr[1]=='=') { prebuf=0; return(SYN_LE); }
	else if (tokenStr[0]=='>' && tokenStr[1]!='=') return(SYN_GT);
	else if (tokenStr[0]=='>' && tokenStr[1]=='=') { prebuf=0; return(SYN_GE); }
	else if (tokenStr[0]=='=' && tokenStr[1]!='=') return(SYN_SET);
	else if (tokenStr[0]=='=' && tokenStr[1]=='=') return(SYN_EQ);
	else if (tokenStr[0]=='!' && tokenStr[1]!='=') return(SYN_NOT);
	else if (tokenStr[0]=='!' && tokenStr[1]=='=') return(SYN_NE);
	else if (tokenStr[0]=='&' && tokenStr[1]=='&') return(SYN_AND);
	else if (tokenStr[0]=='|' && tokenStr[1]=='|') return(SYN_OR);
	else return(ERR);
}
```

<font color=gren>修改</font>	需要添加prebuf。

```c
//找符号
static int FoundRELOOP()
{
	if (tokenStr[0]=='<' && tokenStr[1]!='=') return(SYN_LT);
	else if (tokenStr[0]=='<' && tokenStr[1]=='=') { prebuf=0; return(SYN_LE); }
	else if (tokenStr[0]=='>' && tokenStr[1]!='=') return(SYN_GT);
	else if (tokenStr[0]=='>' && tokenStr[1]=='=') { prebuf=0; return(SYN_GE); }
	else if (tokenStr[0]=='=' && tokenStr[1]!='=') return(SYN_SET);
	else if (tokenStr[0]=='=' && tokenStr[1]=='=') { prebuf=0; return(SYN_EQ); }
	else if (tokenStr[0]=='!' && tokenStr[1]!='=') return(SYN_NOT);
	else if (tokenStr[0]=='!' && tokenStr[1]=='=') { prebuf=0; return(SYN_NE); }
	else if (tokenStr[0]=='&' && tokenStr[1]=='&') { prebuf=0; return(SYN_AND); }
	else if (tokenStr[0]=='|' && tokenStr[1]=='|') { prebuf=0; return(SYN_OR); }
	else return(ERR);
}
```

#### 1.1.3 1+1或i+1型数字加法

<font color=red>错误</font>	对于1+1或i+1型数字加法，报错退出了程序。

```c
main()
{ int i,j,k;
  
  i=1;j=1;
  show(1+1);
}
```

查看以上示例语法分析的错误输出

```
SYN: FUNC-->main() {S}
SYN: S-->D S
SYN: D-->T id [=E] L;
SYN: T-->int
SYN: L-->, id [=E] L
SYN: L-->, id [=E] L
SYN: L--> 
SYN: S-->A S
SYN: A-->id=E;
SYN: E-->TE E1
SYN: TE-->F TE1
SYN: F-->num
SYN: TE1--> 
SYN: E1--> 
SYN: S-->A S
SYN: A-->id=E;
SYN: E-->TE E1
SYN: TE-->F TE1
SYN: F-->num
SYN: TE1--> 
SYN: E1--> 
SYN: S-->show(E); S
SYN: E-->TE E1
SYN: TE-->F TE1
SYN: F-->num
SYN: TE1--> 
SYN: E1--> 
NAME:main, TYPE:1, 
NAME:i, TYPE:2, VALUE:1
NAME:j, TYPE:2, VALUE:1
NAME:k, TYPE:2, VALUE:0
```

单步调试，发现词法分析调用`LexTable`DFA转换表时出现问题

DFA转换表中各状态含义：

- 0: reloop
- 1: delim
- 2: 乘法
- 3: 除法
- 4: 加减
- 5: 数字
- 6: letter_
- 7: symbol

10*表示接收状态，但多读了一个有用符号，需要缓存：

- 101: 逻辑符号reloop
- 102: 除法
- 103: +和-
- 104: 数字
- 105: symbol

20*表示接收状态，没有多读有用符号：

- 201: 文件末尾
- 202: 注释
- 203: 换行
- 204: 乘法
- 205: 括号

```c
//词法分析DFA转换表
static int LexTable[6][8]=
   {{   1, 201, 204,   2,   3,   4,   5, 205},
	{ 101, 101, 101, 101, 101, 101, 101, 101},
	{ 102, 102, 202, 203, 102, 102, 102, 102},
	{ 103, 103, 103, 103, 103,   4, 103, 103},
	{ 104, 104, 104, 104, 104,   4, 104, 104},
	{ 105, 105, 105, 105, 105,   5,   5, 105}};
```

<font color=gren>修改</font>	 `LexTable[4][5] = 103;`

```c
//词法分析DFA转换表
static int LexTable[6][8]=
   {{   1, 201, 204,   2,   3,   4,   5, 205},
	{ 101, 101, 101, 101, 101, 101, 101, 101},
	{ 102, 102, 202, 203, 102, 102, 102, 102},
	{ 103, 103, 103, 103, 103, 103, 103, 103},
	{ 104, 104, 104, 104, 104,   4, 104, 104},
	{ 105, 105, 105, 105, 105,   5,   5, 105}};
```

运行示例得到结果

```
2
```

### 1.2 语法分析 SyntaxAnalysis.c

最终确定的文法：

```
FUNC-->main() {S}
S-->D S
S-->A S
S-->show(E); S
S-->if (B) {S} [else {S}] S
S-->while(B) {S} S
S-->break;
S-->continue;
S-->ε
D-->T id [=E] L;
L-->, id [=E] L | ε
T-->int | char
A-->id=E;
B-->id=TB B1
B1-->‘||’ TB B1 | ε
TB-->FB TB1
TB1-->&& FB TB1 | ε
FB-->E>[=] E | E <[=] E | E== E | E!=E | E | !B | TRUE | FALSE
E-->TE E1
E1-->+TE E1 | -TE E1 | ε
TE-->F TE1
TE1-->*F TE1 | /F TE1 | ε
F-->id | num | char | (B)
```

>文法主要修改了`F-->id | num | char | (B)`
>
>添加了`S-->break;` ` S-->continue;`

文法中各符号的含义：

- `Prod_FUNC()` 函数(主程序)
- `Prod_S()` 函数体
- `Prod_D()` 变量声明语句
- `Prod_T()`  类型
- `Prod_E()`  算术表达式E
- `Prod_L()` id和id_list 
- `Prod_A()` 赋值语句
- `Prod_B()` 判断语句，返回0或1
- `Prod_TB()` 逻辑表达式TB
- `Prod_B1()` ‘||’或表达式
- `Prod_FB()` 逻辑运算
- `Prod_TB1()` '&&'与表达式
- `Prod_F()` 参与算术的变量
- `Prod_TE()`乘除运算
- `Prod_E1` 加减运算

#### 1.2.1 if 功能实现

<font color=red>错误</font>	判断B为假后仍然执行了S。

```c
main()
{ int i,j,k;
  
  i=1;j=1;
  if(i == 2){
  	i = 3;
  }
  show(i);
}
```

查看以上示例语法分析的错误输出

```
SYN: FUNC-->main() {S}
SYN: S-->D S
SYN: D-->T id [=E] L;
SYN: T-->int
SYN: L-->, id [=E] L
SYN: L-->, id [=E] L
SYN: L--> 
SYN: S-->A S
SYN: A-->id=E;
SYN: E-->TE E1
SYN: TE-->F TE1
SYN: F-->num
SYN: TE1--> 
SYN: E1--> 
SYN: S-->A S
SYN: A-->id=E;
SYN: E-->TE E1
SYN: TE-->F TE1
SYN: F-->num
SYN: TE1--> 
SYN: E1--> 
SYN: S-->if (B) {S} [else {S}] S
SYN: B-->id=TB B1
SYN: TB-->FB TB1
SYN: E-->TE E1
SYN: TE-->F TE1
SYN: F-->id
SYN: TE1--> 
SYN: E1--> 
SYN: FB-->E==E
SYN: E-->TE E1
SYN: TE-->F TE1
SYN: F-->num
SYN: TE1--> 
SYN: E1--> 
SYN: TB1--> 
SYN: B1--> 
SYN: S-->A S
SYN: A-->id=E;
SYN: E-->TE E1
SYN: TE-->F TE1
SYN: F-->num
SYN: TE1--> 
SYN: E1--> 
SYN: S--> 
SYN: S-->show(E); S
SYN: E-->TE E1
SYN: TE-->F TE1
SYN: F-->id
SYN: TE1--> 
SYN: E1--> 
SYN: S--> 
NAME:main, TYPE:1, 
NAME:i, TYPE:2, VALUE:1
NAME:j, TYPE:2, VALUE:1
NAME:k, TYPE:2, VALUE:0
```

以上B的条件判断为假而执行了S，而显然if中的条件不满足，不应执行i = 3。

`Prod_S()`函数中

```c
else if (lookahead.token==SYN_IF)
{
		#if defined(AnaTypeSyn)
		printf("SYN: S-->if (B) {S} [else {S}] S");
		#endif
		match(SYN_IF);
		match(SYN_PAREN_L);
		bval=Prod_B();
		match(SYN_PAREN_R);
		if (run_status==1 && bval==0) run_status=2;
  
		match(SYN_BRACE_L);
		Prod_S();
		match(SYN_BRACE_R);
		if (lookahead.token==SYN_ELSE)
		{
			match(SYN_ELSE);
			if (run_status==1) run_status=2;
			else if (run_status==2) run_status=1;
			match(SYN_BRACE_L);
			Prod_S();
			match(SYN_BRACE_R);
			if (run_status==2) run_status=1;
		}
		Prod_S();
	}
```

<font color=gren>修改</font>	修改`Prod_S()`中SYN_IF项，用局部变量存储if语句前run_status的状态

```c
else if (lookahead.token==SYN_IF) //if语句
	{
		int next_status = run_status;//存储if语句前run_status的状态
		
  	#if defined(AnaTypeSyn)
		printf("SYN: S-->if (B) {S} [else {S}] S\n");
		#endif
		match(SYN_IF);
		match(SYN_PAREN_L);
		bval=Prod_B();
		match(SYN_PAREN_R);
		if (run_status==1 && bval==0) run_status=2;//判断结果0跳过

		match(SYN_BRACE_L);
		Prod_S();
		match(SYN_BRACE_R);
		if (lookahead.token==SYN_ELSE)
		{
			match(SYN_ELSE);
			if (run_status==1) run_status=2;
			else if (run_status==2) run_status=1;
			match(SYN_BRACE_L);
			Prod_S();
			match(SYN_BRACE_R);
			if (run_status==2) run_status=1;
		}
  
		if (run_status != 0 && run_status !=3) //支持continue、break的修改
		run_status = next_status;//恢复if语句前run_status的状态
		Prod_S();
	}
```

运行示例得到结果

```
1
```

#### 1.2.2 && 运算实现

<font color=red>错误</font>	&&运算的逻辑错误。

```c
main()
{ int i,j,k;
  
  i=1;j=1;
  if(i == 2 && j == 1){
  	i = 3;
  }
  show(i);
}
```

运行示例得到结果

```
3
```

显然`i!=2`不应执行`i=3`。

`Prod_TB1()`函数中，bval2还没有赋值，就执行了判断bval2的值是否为1的命令：

```c
bval1=(run_status==1 && (bval1==1 && bval2==1)) ? 1 : 0;
```

```c
static int Prod_TB1(int bval1)
{
	int bval2;
	if (lookahead.token==SYN_AND)
	{
		#if defined(AnaTypeSyn)
		printf("SYN: TB1-->&& TB1\n");
		#endif
		match(SYN_AND);
		bval1=Prod_FB();//与上面TB1-->&& TB1描述不一致，bval1的值被覆盖丢失
		bval1=(run_status==1 && (bval1==1 && bval2==1)) ? 1 : 0;//bval2还没有赋值就判断bval2==1
		bval2=Prod_TB1(bval1);
		return(bval2);
	}
	else
	{
		#if defined(AnaTypeSyn)
		printf("SYN: TB1--> \n");
		#endif
		return(bval1);
	}
}
```

<font color=gren>修改</font>	`bval2=Prod_FB();`

```c
//'&&'与表达式
static int Prod_TB1(int bval1)
{
	int bval2;
	if (lookahead.token==SYN_AND)
	{
		#if defined(AnaTypeSyn)
		printf("SYN: TB1-->&& FB TB1\n");//修改为正确表达式
		#endif
		match(SYN_AND);
		bval2=Prod_FB();	//改为赋值给bval2
		bval1=(run_status==1 && (bval1==1 && bval2==1)) ? 1 : 0;
		bval2=Prod_TB1(bval1);
		return(bval2);
	}
	else
	{
		#if defined(AnaTypeSyn)
		printf("SYN: TB1--> \n");
		#endif
		return(bval1);
	}
}
```

#### 1.2.3 while 功能实现

<font color=red>错误</font>	不能循环。

```c
main()
{ int i,j,k;
  
  i=1;j=1;
  while(i<10){
  	i = i+1;
  }
  show(i);
}
```

`Prod_S()`函数中

```c
else if (lookahead.token==SYN_WHILE)
	{
		#if defined(AnaTypeSyn)
		printf("SYN: S-->while(B) {S} S\n");
		#endif
		match(SYN_WHILE);
		file_index=ftell(sFile)-6;
		match(SYN_PAREN_L);
		bval=Prod_B();
		match(SYN_PAREN_R);
		if (run_status==1 && bval==0) {
			run_status=2;
		}
		match(SYN_BRACE_L);
		Prod_S();
		match(SYN_BRACE_R);//match后，nextToken为示例中的show
		if (run_status==1)
		{	
			fseek(sFile,file_index,SEEK_SET);
			renewLex();
		}
		else if (run_status==2)
			run_status=1;
		Prod_S();
	}
```

在`match()`函数中读 `} `的 nextToken时，取了示例中的 `show` ，已经出了循环，开始等待循环体外的`show` 。

而此时文件指针已经回退到`while`，但nextToken是`show`无法match `while`。

<font color=gren>修改</font>	将这个token直接赋值成`while`，并且循环时，指针只回退到`while`后一个字符。

```c
else if (lookahead.token==SYN_WHILE)
	{
		#if defined(AnaTypeSyn)
		printf("SYN: S-->while(B) {S} S\n");
		#endif
		match(SYN_WHILE);
		file_index=ftell(sFile)-1; //修改指针只回退到(
		match(SYN_PAREN_L);
		bval=Prod_B();
		printf("bval:%d\n", bval);
		match(SYN_PAREN_R);
		if (run_status==1 && bval==0) {
			run_status=2;
		}
		match(SYN_BRACE_L);
		Prod_S();
		match(SYN_BRACE_R);
		
  	//continue
		if (run_status==3 && bval==0) {
			run_status=2;
		}		
		if (run_status==3) run_status=1;
  
		if (run_status==1)
		{	
			//修改token为'while'的SYN
			lookahead.token = 60;
			fseek(sFile,file_index,SEEK_SET);
			renewLex();
		}
		else if (run_status==2 || run_status==0) //break
			run_status=1;
		Prod_S();
	}
```

## 2 char型数据

**实验要求**	完善char型数据的处理，包括char型数据的变量定义、变量赋值、参与算术表达式运算等。

> 定义char型数据的存储方式：
>
> 1. SYN：`#define	SYN_CH	3`
> 2. 字符存储的位置（int整数类型存储）：`token.tokenVal.number='c';`
> 3. Id类型：`EXPVAL.type=IDT_CHAR;`

### 2.1 constvar.h

<font color=gren>修改</font>	新加入SYN_CH，定义字符类型。

SYN定义如下

```c
#define SYN_NUM		1		// int整数
#define SYN_ID		2		// id
#define SYN_CH		3		// char字符	
```

### 2.2 LexicalAnalysis.c

为了实现char型数据的变量定义，需识别 `char ch = 'c';` 或者 `char ch = "c";` 

<font color=gren>修改</font>	`nextToken()`函数，在关键字的选项中识别单引号`'`和双引号`"`。

```
TERMINAL nextToken()
{
	...
	else if ((c>='a' && c<='z')||(c>='A' && c<='Z')||(c=='_') || c=='\"' || c=='\'')
			state=LexTable[state][LEX_LETTER_];
	...
}
```

<font color=gren>修改</font>	`nextToken()`函数，对SYN_CN特别处理，把`'c'`字符串中的第二个字符(`c`)放入`token.tokenVal.number`中。

在处理关键字的switch-case中，对SYN_CN特别处理，把'c'字符串中的第二个字符('c')放入token.tokenVal.number中。

```c
//关键字
case 105: tokenStr[tokenLen]='\0';
		  token.token=FoundKeyword();
		  //对SYN_CN特别处理，把'c'字符串中的第二个字符('c')放入token.tokenVal.number中
		  if (token.token == SYN_CH)
		  	token.tokenVal.number=tokenStr[1];
		  else token.tokenVal.str=tokenStr;
		  break;
```

<font color=gren>修改</font>	在`FoundKeyword()`中选择SYN_CH

```c
// 找关键字
static int FoundKeyword()
{
	if (tokenStr[0]=='\"' && tokenStr[0]=='\"') return(SYN_CH);
	if (tokenStr[0]=='\'' && tokenStr[0]=='\'') return(SYN_CH);
  
	if (strcompare(tokenStr,"TRUE")) return(SYN_TRUE);
	if (strcompare(tokenStr,"FALSE")) return(SYN_FALSE);
	if (strcompare(tokenStr,"int")) return(SYN_INT);
	if (strcompare(tokenStr,"char")) return(SYN_CHAR);
	if (strcompare(tokenStr,"if")) return(SYN_IF);
	if (strcompare(tokenStr,"else")) return(SYN_ELSE);
	if (strcompare(tokenStr,"while")) return(SYN_WHILE);
	if (strcompare(tokenStr,"show")) return(SYN_SHOW);
	return(SYN_ID);
}
```

### 2.3 SyntaxAnalysis.c

<font color=gren>修改</font>	在词法分析输出时，支持打印输出字符型数据。

```c
void SyntaxAnalysis()
{
#if defined(AnaTypeLex)
//testing lexical analysis
	TERMINAL token;
	token=nextToken();
	while (token.token!=ERR)
	{	if (token.token==SYN_NUM)
			printf("LEX: %d,%d\n",token.token,token.tokenVal.number);
		else if (token.token==SYN_ID)
			printf("LEX: %d,%s\n",token.token,token.tokenVal.str);
		else if (token.token==SYN_CH) //打印字符型数据
			printf("LEX: %d,%c\n",token.token,token.tokenVal.number);
		else
			printf("LEX: %d\n",token.token);
		token=nextToken();
	}
#else
...
}
```

<font color=gren>修改</font>	在`match()`函数中处理对字符型SYN_CH的token，使`curtoken_num`为字符变量的值。

```c
// 参数：SYN_ID，无返回值
//处理token，给curtoken_num或curtoken_str赋值，并取新的token
static int match (int t)
{
	char *p,*q;
	if (lookahead.token == t)
	{	
		if (t==SYN_NUM || t==SYN_CH) //SYN_CH
			curtoken_num=lookahead.tokenVal.number;
		else if (t==SYN_ID)
			for (p=lookahead.tokenVal.str,q=curtoken_str;(*q=*p)!='\0';p++,q++);
		lookahead = nextToken();
	}
	else
		FreeExit();
	return(0);
}
```

<font color=gren>修改</font>	添加文法`F-->char` 。

在函数`Prod_F()`中添加token为SYN_CH的情况。

```c
//表达式F
static EXPVAL Prod_F()
{
  ...                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                 
	else if (lookahead.token==SYN_CH)
	{
		#if defined( )
		printf("SYN: F-->char\n");
		#endif
		match(SYN_CH);
		val.type=ID_CHAR;
		val.val.charval=curtoken_num;
	}
	...
	return(val);
}
```

### 2.4 测试

示例一：char型数据的定义，赋值，show()函数传参。

```c
main()
{ int i,j;
  char c;
  
  i=1;j=1;
  c='d';
  show(c);
}
```

运行示例得到结果

```
d
```

词法分析输出

```c
LEX: 2,main
LEX: 21
LEX: 22
LEX: 23
LEX: 56
LEX: 2,i
LEX: 25
LEX: 2,j
LEX: 26
LEX: 57
LEX: 2,c
LEX: 26
LEX: 2,i
LEX: 27
LEX: 1,1
LEX: 26
LEX: 2,j
LEX: 27
LEX: 1,1
LEX: 26
LEX: 2,c
LEX: 27
LEX: 3,d	//字符'd'
LEX: 26
LEX: 61
LEX: 21
LEX: 2,c
LEX: 22
LEX: 26
LEX: 24
```

语法分析输出

```
SYN: FUNC-->main() {S}
SYN: S-->D S 
SYN: D-->T id [=E] L;
SYN: T-->int
SYN: L-->, id [=E] L
SYN: L--> 
SYN: S-->D S
SYN: D-->T id [=E] L;
SYN: T-->char
SYN: L--> 
SYN: S-->A S
SYN: A-->id=E;
SYN: E-->TE E1
SYN: TE-->F TE1
SYN: F-->num
SYN: TE1--> 
SYN: E1--> 
SYN: S-->A S
SYN: A-->id=E;
SYN: E-->TE E1
SYN: TE-->F TE1
SYN: F-->num
SYN: TE1--> 
SYN: E1--> 
SYN: S-->A S
SYN: A-->id=E;
SYN: E-->TE E1
SYN: TE-->F TE1
SYN: F-->char						//修改的文法
SYN: TE1--> 
SYN: E1--> 
SYN: S-->show(E); S
SYN: E-->TE E1
SYN: TE-->F TE1
SYN: F-->id
SYN: TE1--> 
SYN: E1--> 
d												//输出结果
SYN: S--> 
NAME:main, TYPE:1, 
NAME:i, TYPE:2, VALUE:1
NAME:j, TYPE:2, VALUE:1
NAME:c, TYPE:3, VALUE:d	//char型数据
```

示例二：char型数据支持算术表达式运算。

```c
main()
{ int i,j;
  char c;
  
  i=1;j=1;
  c='d';
  show(c-'a'+'b');
}
```

运行示例得到结果

```
e
```

## 3 逻辑表达式

**实验要求**	在表达式计算中，允许逻辑表达式参与算术表达式计算。例如 a+(b>1)。

### 3.1 修改文法 ` F-->id | num | char | (B)`

原本的文法为`F—>(E)`。

我们分析文法可见，逻辑运算的优先级比算术运算低，因此想要参与算术运算，首先逻辑表达式必须位于括号内。

返回判断语句的逻辑值的文法为

```
B-->id=TB B1
B1-->‘||’ TB B1 | ε
TB-->FB TB1
TB1-->&& FB TB1 | ε
FB-->E>[=] E | E <[=] E | E== E | E!=E | E | !B | TRUE | FALSE
```

可见在`FB-->E>[=] E | E <[=] E | E== E | E!=E | E | !B | TRUE | FALSE`中已经有单独的一句`FB-->E`。其中`E`表示算术表达式返回的值。因此我们可考虑直接用`B`替换`E`。修改文法为`F—>(B)`。

<font color=gren>修改</font>	在SyntaxAnalysis.c中修改文法 `F—->(B)`

```c
else if (lookahead.token==SYN_PAREN_L)
{
		#if defined(AnaTypeSyn)
		printf("SYN: F-->(B)\n");
		#endif
		match(SYN_PAREN_L);
		val.type=ID_INT;	//使返回的逻辑表达式的值为int类型
		val.val.intval=Prod_B();	//修改E为B
		match(SYN_PAREN_R);
}
```

### 3.2 测试

示例一：FALSE为0。

```c
main()
{ int i,j,k;
  char c;
  
  i=1;j=1;
  k = 1+(i!=j);
  show(k);
}
```

运行示例得到结果

```
1
```

示例二：TRUE为1。

```c
main()
{ int i,j,k;
  char c;
  
  i=1;j=1;
  k = 1+(i==j);
  show(k);
}
```

运行示例得到结果

```
2
```

## 4 Continue & Break

### 4.1 添加文法`S-->break;` 

<font color=gren>修改</font>	在`Prod_S()`函数中添加

```c
else if (lookahead.token==SYN_BREAK)
{
		#if defined(AnaTypeSyn)
		printf("SYN: S-->break;\n");
		#endif
		
		match(SYN_BREAK);
		match(SYN_SEMIC);
		if (run_status==1)
		{		
			run_status = 0;
		}
		Prod_S();
}
```

<font color=gren>修改</font>	我们将`run_state`设为0，以此来判断循环体中接下来的部分是不可执行的。

返回到程序`while`对应的分支中后，还需要修改：在退出while循环后，如果`run_state`为0，执行`runstate = 1`，使程序可以继续往后执行。

```c
else if (lookahead.token==SYN_WHILE)
{
		#if defined(AnaTypeSyn)
		printf("SYN: S-->while(B) {S} S\n");
		#endif
		match(SYN_WHILE);
		file_index=ftell(sFile)-1; //修改指针只回退到(
		match(SYN_PAREN_L);
		bval=Prod_B();
		match(SYN_PAREN_R);
		if (run_status==1 && bval==0) {
			run_status=2;
		}
		match(SYN_BRACE_L);
		Prod_S();
		match(SYN_BRACE_R);
		
  		//continue
		if (run_status==3 && bval==0) {
			run_status=2;
		}		
		if (run_status==3) run_status=1;
  
		if (run_status==1)
		{	
			//修改token为'while'的SYN
			lookahead.token = 60;
			fseek(sFile,file_index,SEEK_SET);
			renewLex();
		}
		else if (run_status==2 || run_status==0) //break
			run_status=1;
		Prod_S();
}
```

### 4.2 添加文法`S-->continue;` 

<font color=gren>修改</font>	在`Prod_S()`函数中添加

```c
else if (lookahead.token==SYN_CONTINUE)
{
		#if defined(AnaTypeSyn)
		printf("SYN: S-->continue;\n");
		#endif
		
		match(SYN_CONTINUE);
		match(SYN_SEMIC);
		if (run_status==1)
		{	
			run_status = 3;
		}
		Prod_S();
}
```

<font color=gren>修改</font>	我们将`run_state`设为3，以此来判断循环体中接下来的部分是不可执行的。

返回到程序`while`对应的分支中后，还需要修改：当`run_status==3`而且循环体判断为0，需要退出循环体时，执行`run_status=2`；当`run_status==3`而且继续下一次循环时，执行`run_status=1`。

```c
else if (lookahead.token==SYN_WHILE)
{
		#if defined(AnaTypeSyn)
		printf("SYN: S-->while(B) {S} S\n");
		#endif
		match(SYN_WHILE);
		file_index=ftell(sFile)-1; //修改指针只回退到(
		match(SYN_PAREN_L);
		bval=Prod_B();
		match(SYN_PAREN_R);
		if (run_status==1 && bval==0) {
			run_status=2;
		}
		match(SYN_BRACE_L);
		Prod_S();
		match(SYN_BRACE_R);
		
  		//continue
		if (run_status==3 && bval==0) {
			run_status=2;
		}		
		if (run_status==3) run_status=1;
  
		if (run_status==1)
		{	
			//修改token为'while'的SYN
			lookahead.token = 60;
			fseek(sFile,file_index,SEEK_SET);
			renewLex();
		}
		else if (run_status==2 || run_status==0) //break
			run_status=1;
		Prod_S();
}
```

### 4.3 测试

示例一：Continue

设`i==3`时不执行后面的`j=j+1`语句

```c
main()
{ int i,j,k;
  char c;
 
  i=1;j=1;

  while(i<5){
    i=i+1;
    if (i==3) {
      continue;
    }
    j=j+1;
  }
  show(i);
  show(j);
}
```

运行示例得到结果

```
5
4
```

示例二： Break

设`i==3`时，直接退出循环，此时j的值应为2。

```c
main()
{ int i,j,k;
  char c;
 
  i=1;j=1;

  while(i<5){
    i=i+1;
    if (i==3) {
      break;
    }
    j=j+1;
  }
  show(i);
  show(j);
}
```

运行示例得到结果

```
3
2
```

## 5 实验总结

源代码中添加了大量注释，提高了阅读源码的能力。

本次实验中使用了lldb调试，对lldb调试各种命令，及设置断点的技巧与艺术有了一些领略。