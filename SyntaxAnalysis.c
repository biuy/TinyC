#include "stdio.h"
#include "stdlib.h"
#include "constvar.h"

extern TERMINAL nextToken();
extern void renewLex();
static int match (int t);
static int strcompare(char *sstr, char *tstr);	//�Ƚ�������
static IDTABLE* InstallID();		//�ڷ��ű���Ϊcurtoken_str����һ����Ŀ
static IDTABLE* LookupID();			//�ڷ��ű��в���curtoken_str
static void FreeExit();
static int cast2int(EXPVAL exp);		//��exp��ֵת��Ϊint����
static char cast2char(EXPVAL exp);		//��exp��ֵת��Ϊchar����
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
static int run_status=1;	//0������ִ�У�1:��������ִ�У�2:������ǰ�ṹ�����ִ�� 3:continue������ǰ�ṹ

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

// ������SYN_ID���޷���ֵ
//����token����curtoken_num��curtoken_str��ֵ����ȡ�µ�token
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

//ID������ű�(����
//����ֵ������ڵ�ṹ��
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
			IDTHead=p;//�µ�IDHead
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

//q->p����ӡ�������val
static void FreeExit()
{
	IDTABLE *p,*q;
	//�ͷ�����ռ�
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
	exit(0); //�������г����˳�����
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

//����������
static int Prod_FUNC()
{
	IDTABLE *p;
	match(SYN_ID); //ʶ��main
	if (strcompare(curtoken_str,"main")==ERR) FreeExit();
	p=InstallID(); //װ��һ��ID
	p->type=ID_FUN;
	#if defined(AnaTypeSyn)
	printf("SYN: FUNC-->main() {S}\n");
	#endif
	match(SYN_PAREN_L); //ʶ��(
	match(SYN_PAREN_R); //ʶ��)
	match(SYN_BRACE_L); //ʶ��{
	Prod_S();
	match(SYN_BRACE_R); //ʶ��}
	return(0);
}

//������
static int Prod_S()
{
	EXPVAL exp;
	int bval;
	long file_index;
	if (lookahead.token==SYN_INT || lookahead.token==SYN_CHAR)
	{	//����ǹؼ���int/char
		#if defined(AnaTypeSyn)
		printf("SYN: S-->D S\n");
		#endif
		Prod_D(); //�����������
		Prod_S(); //������
	}
	else if (lookahead.token==SYN_ID) //�����һ��ID
	{
		#if defined(AnaTypeSyn)
		printf("SYN: S-->A S\n");
		#endif
		Prod_A();
		Prod_S();
	}
	else if (lookahead.token==SYN_SHOW) //show()����
	{
		#if defined(AnaTypeSyn)
		printf("SYN: S-->show(E); S\n");
		#endif
		match(SYN_SHOW);
		match(SYN_PAREN_L);
		exp=Prod_E(); //ʶ����ʽ
		match(SYN_PAREN_R);
		match(SYN_SEMIC);
		if (run_status==1) //�����ʽ�е�ֵshow����
			if (exp.type==ID_INT)
				printf("%d\n",exp.val.intval);
			else if (exp.type==ID_CHAR)
				printf("%c\n",exp.val.charval);
		Prod_S();
	}
	else if (lookahead.token==SYN_IF) //if���
	{
		int next_status = run_status;
		#if defined(AnaTypeSyn)
		printf("SYN: S-->if (B) {S} [else {S}] S\n");
		#endif
		match(SYN_IF);
		match(SYN_PAREN_L);
		bval=Prod_B();
		match(SYN_PAREN_R);
		if (run_status==1 && bval==0) run_status=2;//�жϽ��0����

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
		file_index=ftell(sFile)-1; //�޸�ָ��ֻ���˵�(
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
			//�޸�tokenΪ'while'��SYN
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

// �����������
static int Prod_D()
{
	int type;
	IDTABLE *p;
	EXPVAL exp;
	#if defined(AnaTypeSyn)
	printf("SYN: D-->T id [=E] L;\n");
	#endif
	type=Prod_T(); //����
	match(SYN_ID); //ʶ��һ��ID
	p=InstallID(); //װ��ID
	p->type=type;
	if (lookahead.token==SYN_SET) //'='
	{
		match(SYN_SET); //ʶ��'='
		exp=Prod_E(); //���ʽ
		if (run_status==1)
		{	
			if (type==ID_INT)
				p->val.intval=cast2int(exp);//���ر��ʽintֵ
			else if (type==ID_CHAR)
				p->val.charval=cast2char(exp);// ���ر��ʽcharֵ
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
	if (lookahead.token==SYN_COMMA) //�������int i,j
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
		printf("SYN: L--> \n"); //��
		#endif
	}
	return(0);
}

//����
//����ֵID_CHAR����ID_INT
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

//��ֵ���
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

//if,while�е��ж����
//����ֵ���жϽ��
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

//��||������ʽ
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

//���ʽTB
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


//'&&'����ʽ
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

//�߼����ʽ
//����ֵ
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

//���ʽE
//����ֵ��EXPVAL 
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

//���ʽTE
//����ֵ��EXPVAL 
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

//���ʽF
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
