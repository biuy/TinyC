#include "stdio.h"
#include "errno.h"

extern void SyntaxAnalysis();

FILE *sFile;

int main(int argc, char* argv[])
{
    /*errno_t err;
    err=fopen_s(&sFile, argv[1],"r");*/

    sFile=fopen("test1.txt","rt");
    /*if(!sFile)
    {
        printf("文件打开失败\n");
        return 0;
    }

    char ch;
    while((ch=fgetc(sFile))!=EOF)//如果读入的字符没有不是文件结束符
    {
        putchar(ch);//打印这个字符到显示屏上面
    }putchar('\n');//打印换行符*/
    
	SyntaxAnalysis();
	fclose(sFile);
//Free all memories

	return 0;
}
 