#include "stdio.h"
#include "stdlib.h"
#include "constvar.h"

extern TERMINAL nextToken();
extern void renewLex();
static int match (int t);
static int strcompare(char *sstr, char *tstr);	//比较两个串
static IDTABLE* InstallID();		//在符号表中为curtoken_str建立一个条目
static IDTABLE* LookupID();			//在符号表中查找curtoken_str
static void FreeExit();
static int cast2int(EXPVAL exp);		//将exp的值转换为int类型
static char cast2char(EXPVAL exp);		//将exp的值转换为char类型
static int Prod_FUNC();
static int Prod_S();
static int Prod_D();
static int Prod_L(int type);
static int Prod_T();
static int Prod_A();
static int Prod_B();
static int Prod_B1(int bval);
static int Prod_TB();
static int Prod_TB1(int bval);
static int Prod_FB();
static EXPVAL Prod_E();
static EXPVAL Prod_E1(EXPVAL val);
static EXPVAL Prod_TE();
static EXPVAL Prod_TE1(EXPVAL val);
static EXPVAL Prod_F();

extern FILE *sFile;
static TERMINAL lookahead;
static int curtoken_num;
static char curtoken_str[MAXTOKENLEN];
static IDTABLE *IDTHead=NULL;
static int run_status=1;	//0；程序不执行；1:程序正常执行；2:跳过当前结构后继续执行 3:continue跳过当前结构

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
		else if (token.token==SYN_CH)
			printf("LEX: %d,%c\n",token.token,token.tokenVal.number);
		else
			printf("LEX: %d\n",token.token);
		token=nextToken();
	}
#else
//syntax analysis
	lookahead=nextToken();
	if (Prod_FUNC()==ERR)
		printf("PROGRAM HALT!\n");
	FreeExit();

#endif
}

// 参数：SYN_ID，无返回值
//处理token，给curtoken_num或curtoken_str赋值，并取新的token
static int match (int t)
{
	char *p,*q;
	if (lookahead.token == t)
	{	
		if (t==SYN_NUM || t==SYN_CH)
			curtoken_num=lookahead.tokenVal.number;
		else if (t==SYN_ID)
			for (p=lookahead.tokenVal.str,q=curtoken_str;(*q=*p)!='\0';p++,q++);
		lookahead = nextToken();
	}
	else
		FreeExit();
	return(0);
}

static int strcompare(char *sstr, char *tstr)
{
	while (*sstr==*tstr && *sstr!='\0') { sstr++; tstr++; }
	if (*sstr=='\0' && *tstr=='\0')	return(0);
	else return(ERR);
}

//ID放入符号表(链表）
//返回值：链表节点结构体
static IDTABLE* InstallID()
{
	IDTABLE *p,*q;
	char *a,*b;
	p=IDTHead; q=NULL;
	while (p!=NULL && strcompare(curtoken_str,p->name)==ERR)
	{
		q=p;//q->p
		p=p->next;
	}
	if (p!=NULL)
		return(NULL);
	else
	{
		p=(IDTABLE*)malloc(sizeof(IDTABLE));
		if (q==NULL)
			IDTHead=p;//新的IDHead
		else
			q->next=p;
		p->next=NULL;
		for (a=curtoken_str,b=p->name;(*b=*a)!='\0';a++,b++);
		return(p);
	}
}

static IDTABLE* LookupID()
{
	IDTABLE *p;
	p=IDTHead;
	while (p!=NULL && strcompare(curtoken_str,p->name)==ERR)
		p=p->next;
	if (p==NULL)
		return(NULL);
	else
		return(p);
}

//q->p，打印出链表的val
static void FreeExit()
{
	IDTABLE *p,*q;
	//释放链表空间
	p=IDTHead;
	while ((q=p)!=NULL)
	{	p=p->next;
		#if defined(AnaTypeSyn)
		printf("NAME:%s, TYPE:%d, ",q->name,q->type);
		if (q->type==ID_INT)
			printf("VALUE:%d\n",q->val.intval);
		else if (q->type==ID_CHAR)
			printf("VALUE:%c\n",q->val.charval);
		else
			printf("\n");
		#endif
		free(q);
	}
	exit(0); //正常运行程序并退出程序
}

static int cast2int(EXPVAL exp)
{
	if (exp.type==ID_INT)
		return(exp.val.intval);
	else if (exp.type==ID_CHAR)
		return((int)(exp.val.charval));
}

static char cast2char(EXPVAL exp)
{
	if (exp.type==ID_INT)
		return((char)(exp.val.intval));
	else if (exp.type==ID_CHAR)
		return(exp.val.charval);
}

//函数主程序
static int Prod_FUNC()
{
	IDTABLE *p;
	match(SYN_ID); //识别main
	if (strcompare(curtoken_str,"main")==ERR) FreeExit();
	p=InstallID(); //装入一个ID
	p->type=ID_FUN;
	#if defined(AnaTypeSyn)
	printf("SYN: FUNC-->main() {S}\n");
	#endif
	match(SYN_PAREN_L); //识别(
	match(SYN_PAREN_R); //识别)
	match(SYN_BRACE_L); //识别{
	Prod_S();
	match(SYN_BRACE_R); //识别}
	return(0);
}

//函数体
static int Prod_S()
{
	EXPVAL exp;
	int bval;
	long file_index;
	if (lookahead.token==SYN_INT || lookahead.token==SYN_CHAR)
	{	//如果是关键字int/char
		#if defined(AnaTypeSyn)
		printf("SYN: S-->D S\n");
		#endif
		Prod_D(); //变量声明语句
		Prod_S(); //函数体
	}
	else if (lookahead.token==SYN_ID) //如果是一个ID
	{
		#if defined(AnaTypeSyn)
		printf("SYN: S-->A S\n");
		#endif
		Prod_A();
		Prod_S();
	}
	else if (lookahead.token==SYN_SHOW) //show()函数
	{
		#if defined(AnaTypeSyn)
		printf("SYN: S-->show(E); S\n");
		#endif
		match(SYN_SHOW);
		match(SYN_PAREN_L);
		exp=Prod_E(); //识别表达式
		match(SYN_PAREN_R);
		match(SYN_SEMIC);
		if (run_status==1) //将表达式中的值show出来
			if (exp.type==ID_INT)
				printf("%d\n",exp.val.intval);
			else if (exp.type==ID_CHAR)
				printf("%c\n",exp.val.charval);
		Prod_S();
	}
	else if (lookahead.token==SYN_IF) //if语句
	{
		int next_status = run_status;
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
		if (run_status != 0 && run_status !=3)
		run_status = next_status;
		Prod_S();
	}
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
	else
	{
		#if defined(AnaTypeSyn)
		printf("SYN: S--> \n");
		#endif
	} 
	return(0);
}

// 变量声明语句
static int Prod_D()
{
	int type;
	IDTABLE *p;
	EXPVAL exp;
	#if defined(AnaTypeSyn)
	printf("SYN: D-->T id [=E] L;\n");
	#endif
	type=Prod_T(); //类型
	match(SYN_ID); //识别一个ID
	p=InstallID(); //装入ID
	p->type=type;
	if (lookahead.token==SYN_SET) //'='
	{
		match(SYN_SET); //识别'='
		exp=Prod_E(); //表达式
		if (run_status==1)
		{	
			if (type==ID_INT)
				p->val.intval=cast2int(exp);//返回表达式int值
			else if (type==ID_CHAR)
				p->val.charval=cast2char(exp);// 返回表达式char值
		}
	}
	Prod_L(type);
	match(SYN_SEMIC);
	return(0);
}

//id_list
static int Prod_L(int type)
{
	IDTABLE *p;
	EXPVAL exp;
	if (lookahead.token==SYN_COMMA) //多个变量int i,j
	{
		#if defined(AnaTypeSyn)
		printf("SYN: L-->, id [=E] L\n");
		#endif
		match(SYN_COMMA);
		match(SYN_ID);
		p=InstallID();
		p->type=type;
		if (lookahead.token==SYN_SET) //'='
		{
			match(SYN_SET);
			exp=Prod_E();
			if (run_status==1)
			{	if (type==ID_INT)
					p->val.intval=cast2int(exp);
				else if (type==ID_CHAR)
					p->val.charval=cast2char(exp);
			}
		}
		Prod_L(type);
	}
	else
	{
		#if defined(AnaTypeSyn)
		printf("SYN: L--> \n"); //空
		#endif
	}
	return(0);
}

//类型
//返回值ID_CHAR或者ID_INT
static int Prod_T()
{
	if (lookahead.token==SYN_INT)
	{
		#if defined(AnaTypeSyn)
		printf("SYN: T-->int\n");
		#endif
		match(SYN_INT);
		return(ID_INT);
	}
	else if (lookahead.token==SYN_CHAR)
	{
		#if defined(AnaTypeSyn)
		printf("SYN: T-->char\n");
		#endif
		match(SYN_CHAR);
		return(ID_CHAR);
	}
	else
		FreeExit();
	return(0);
}

//赋值语句
static int Prod_A()
{
	IDTABLE *p;
	EXPVAL exp;
	#if defined(AnaTypeSyn)
	printf("SYN: A-->id=E;\n");
	#endif
	match(SYN_ID);
	p=LookupID();
	match(SYN_SET);
	exp=Prod_E();
	match(SYN_SEMIC);
	if (run_status==1)
	{	if (p->type==ID_INT)
			p->val.intval=cast2int(exp);
		else if (p->type==ID_CHAR)
			p->val.charval=cast2char(exp);
	}
	return(0);
}

//if,while中的判断语句
//返回值：判断结果
static int Prod_B()
{
	int bval1,bval2;
	#if defined(AnaTypeSyn)
	printf("SYN: B-->id=TB B1\n");
	#endif
	bval1=Prod_TB();
	bval2=Prod_B1(bval1);
	return(bval2);
}

//‘||’或表达式
static int Prod_B1(int bval1)
{
	int bval2;
	if (lookahead.token==SYN_OR)
	{
		#if defined(AnaTypeSyn)
		printf("SYN: B1-->|| TB B1\n");
		#endif
		match(SYN_OR);
		bval2=Prod_TB();
		bval1=(run_status==1 && (bval1==1 || bval2==1)) ? 1 : 0;
		bval2=Prod_B1(bval1);
		return(bval2);
	}
	else
	{
		#if defined(AnaTypeSyn)
		printf("SYN: B1--> \n");
		#endif
		return(bval1);
	}
}

//表达式TB
//bval1:
static int Prod_TB()
{
	int bval1,bval2;
	#if defined(AnaTypeSyn)
	printf("SYN: TB-->FB TB1\n");
	#endif
	bval1=Prod_FB();
	bval2=Prod_TB1(bval1);
	return(bval2);
}


//'&&'与表达式
static int Prod_TB1(int bval1)
{
	int bval2;
	if (lookahead.token==SYN_AND)
	{
		#if defined(AnaTypeSyn)
		printf("SYN: TB1-->&& FB TB1\n");
		#endif
		match(SYN_AND);
		bval2=Prod_FB();
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

//逻辑表达式
//返回值
static int Prod_FB()
{
	int bval;
	EXPVAL val1,val2;
	int ival1,ival2;
	if (lookahead.token==SYN_NOT)
	{
		#if defined(AnaTypeSyn)
		printf("SYN: FB-->!B\n");
		#endif
		match(SYN_NOT);
		bval=Prod_B();
		return(run_status==1 ? 1-bval : 0);
	}
	else if (lookahead.token==SYN_TRUE)
	{
		#if defined(AnaTypeSyn)
		printf("SYN: FB-->TRUE\n");
		#endif
		match(SYN_TRUE);
		return(run_status==1 ? 1 : 0);
	}
	else if (lookahead.token==SYN_FALSE)
	{
		#if defined(AnaTypeSyn)
		printf("SYN: FB-->FALSE\n");
		#endif
		match(SYN_FALSE);
		return(run_status==1 ? 0 : 0);
	}
	else if (lookahead.token==SYN_ID || lookahead.token==SYN_NUM || lookahead.token==SYN_PAREN_L)
	{
		val1=Prod_E();
		if (run_status==1) ival1=cast2int(val1);
		if (lookahead.token==SYN_LT)
		{	
			#if defined(AnaTypeSyn)
			printf("SYN: FB-->E<E\n");
			#endif
			match(SYN_LT);
			val2=Prod_E();
			if (run_status==1)
			{	ival2=cast2int(val2);
				return(ival1<ival2 ? 1 : 0);
			}
			else
				return(0);
		}
		else if (lookahead.token==SYN_LE)
		{
			#if defined(AnaTypeSyn)
			printf("SYN: FB-->E<=E\n");
			#endif
			match(SYN_LE);
			val2=Prod_E();
			if (run_status==1)
			{	ival2=cast2int(val2);
				return(ival1<=ival2 ? 1 : 0);
			}
			else
				return(0);
		}
		else if (lookahead.token==SYN_GT)
		{
			#if defined(AnaTypeSyn)
			printf("SYN: FB-->E>E\n");
			#endif
			match(SYN_GT);
			val2=Prod_E();
			if (run_status==1)
			{	ival2=cast2int(val2);
				return(ival1>ival2 ? 1 : 0);
			}
			else
				return(0);
		}
		else if (lookahead.token==SYN_GE)
		{
			#if defined(AnaTypeSyn)
			printf("SYN: FB-->E>=E\n");
			#endif
			match(SYN_GE);
			val2=Prod_E();
			if (run_status==1)
			{	
				ival2=cast2int(val2);
				return(ival1>=ival2 ? 1 : 0);
			}
			else
				return(0);
		}
		else if (lookahead.token==SYN_EQ)
		{
			#if defined(AnaTypeSyn)
			printf("SYN: FB-->E==E\n");
			#endif
			match(SYN_EQ);
			val2=Prod_E();
			if (run_status==1)
			{	ival2=cast2int(val2);
				return(ival1==ival2 ? 1 : 0);
			}
			else
				return(0);
		}
		else if (lookahead.token==SYN_NE)
		{
			#if defined(AnaTypeSyn)
			printf("SYN: FB-->E!=E\n");
			#endif
			match(SYN_NE);
			val2=Prod_E();
			if (run_status==1)
			{	ival2=cast2int(val2);
				return(ival1!=ival2 ? 1 : 0);
			}
			else
				return(0);
		}
		else
		{
			if (run_status==1)
				return(ival1!=0 ? 1 : 0);
			else
				return(0);
		}

	}
	else
	{	FreeExit();
		return(0);
	}
}

//表达式E
//返回值：EXPVAL 
static EXPVAL Prod_E()
{
	EXPVAL val1,val2;
	#if defined(AnaTypeSyn)
	printf("SYN: E-->TE E1\n");
	#endif
	val1=Prod_TE();
	val2=Prod_E1(val1);
	return(val2);
}

static EXPVAL Prod_E1(EXPVAL val1)
{
	EXPVAL val2,val;
	int i1,i2;
	char c1,c2;
	if (lookahead.token==SYN_ADD)
	{
		#if defined(AnaTypeSyn)
		printf("SYN: E1-->+TE E1\n");
		#endif
		match(SYN_ADD);
		val2=Prod_TE();
		if (run_status==1)
			if (val1.type==ID_INT || val2.type==ID_INT)
			{
				val.type=ID_INT;
				i1=cast2int(val1);
				i2=cast2int(val2);
				val.val.intval=i1+i2;
			}
			else
			{
				val.type=ID_CHAR;
				c1=cast2char(val1);
				c2=cast2char(val2);
				val.val.charval=c1+c2;
			}
		val=Prod_E1(val);
	}
	else if (lookahead.token==SYN_SUB)
	{
		#if defined(AnaTypeSyn)
		printf("SYN: E1-->-TE E1\n");
		#endif
		match(SYN_SUB);
		val2=Prod_TE();
		if (run_status==1)
			if (val1.type==ID_INT || val2.type==ID_INT)
			{
				val.type=ID_INT;
				i1=cast2int(val1);
				i2=cast2int(val2);
				val.val.intval=i1-i2;
			}
			else
			{
				val.type=ID_CHAR;
				c1=cast2char(val1);
				c2=cast2char(val2);
				val.val.charval=c1-c2;
			}
		val=Prod_E1(val);
	}
	else
	{
		#if defined(AnaTypeSyn)
		printf("SYN: E1--> \n");
		#endif
		val=val1;
	}
	return(val);
}

//表达式TE
//返回值：EXPVAL 
static EXPVAL Prod_TE()
{
	EXPVAL val1,val2;
	#if defined(AnaTypeSyn)
	printf("SYN: TE-->F TE1\n");
	#endif
	val1=Prod_F();
	val2=Prod_TE1(val1);
	return(val2);
}

static EXPVAL Prod_TE1(EXPVAL val1)
{
	EXPVAL val2,val;
	int i1,i2;
	char c1,c2;
	if (lookahead.token==SYN_MUL)
	{
		#if defined(AnaTypeSyn)
		printf("SYN: TE1-->*F TE1\n");
		#endif
		match(SYN_MUL);
		val2=Prod_F();
		if (run_status==1)
			if (val1.type==ID_INT || val2.type==ID_INT)
			{
				val.type=ID_INT;
				i1=cast2int(val1);
				i2=cast2int(val2);
				val.val.intval=i1*i2;
			}
			else
			{
				val.type=ID_CHAR;
				c1=cast2char(val1);
				c2=cast2char(val2);
				val.val.charval=c1*c2;
			}
		val=Prod_TE1(val);
	}
	else if (lookahead.token==SYN_DIV)
	{
		#if defined(AnaTypeSyn)
		printf("SYN: TE1-->/F TE1\n");
		#endif
		match(SYN_DIV);
		val2=Prod_F();
		if (run_status==1)
			if (val1.type==ID_INT || val2.type==ID_INT)
			{
				val.type=ID_INT;
				i1=cast2int(val1);
				i2=cast2int(val2);
				val.val.intval=i1/i2;
			}
			else
			{
				val.type=ID_CHAR;
				c1=cast2char(val1);
				c2=cast2char(val2);
				val.val.charval=c1/c2;
			}
		val=Prod_TE1(val);
	}
	else
	{
		#if defined(AnaTypeSyn)
		printf("SYN: TE1--> \n");
		#endif
		val=val1;
	}
	return(val);
}

//表达式F
static EXPVAL Prod_F()
{
	EXPVAL val;
	static IDTABLE *p;
	if (lookahead.token==SYN_NUM)
	{
		#if defined(AnaTypeSyn)
		printf("SYN: F-->num\n");
		#endif
		match(SYN_NUM);
		val.type=ID_INT;
		val.val.intval=curtoken_num;
	}
	else if (lookahead.token==SYN_CH)
	{
		#if defined(AnaTypeSyn)
		printf("SYN: F-->char\n");
		#endif
		match(SYN_CH);
		val.type=ID_CHAR;
		val.val.charval=curtoken_num;
	}
	else if (lookahead.token==SYN_ID)
	{
		#if defined(AnaTypeSyn)
		printf("SYN: F-->id\n");
		#endif
		match(SYN_ID);
		p=LookupID();
		val.type=p->type;
		val.val=p->val;
	}
	else if (lookahead.token==SYN_PAREN_L)
	{
		#if defined(AnaTypeSyn)
		printf("SYN: F-->(B)\n");
		#endif
		match(SYN_PAREN_L);
		val.type=ID_INT;
		val.val.intval=Prod_B();
		match(SYN_PAREN_R);
	}
	else{
		FreeExit();
	}

	return(val);
}
