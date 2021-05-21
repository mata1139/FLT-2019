/***************************************************************
*      scanner routine for Mini C language                    *
***************************************************************/

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

#include "Scanner.h"

extern FILE* sourceFile;                       // miniC source program


int superLetter(char ch);
int superLetterOrDigit(char ch);
int getNumber(char firstCharacter);
int hexValue(char ch);
void lexicalError(int n);

double getFractional(double n) {
	while (n > 1)
		n /= 10.0;

	return n;
}

int col = 1, line = 1;

char* tokenName[] = {
	"!",        "!=",      "%",       "%=",     "%ident",   "%number",
	/* 0          1           2         3          4          5        */
	"&&",       "(",       ")",       "*",      "*=",       "+",
	/* 6          7           8         9         10         11        */
	"++",       "+=",      ",",       "-",      "--",	    "-=",
	/* 12         13         14        15         16         17        */
	"/",        "/=",      ";",       "<",      "<=",       "=",
	/* 18         19         20        21         22         23        */
	"==",       ">",       ">=",      "[",      "]",        "eof",
	/* 24         25         26        27         28         29        */
	//   ...........    word symbols ................................. //
	/* 30         31         32        33         34         35        */
	"const",    "else",     "if",      "int",     "return",  "void",
	/* 36         37         38        39         40         41        */
	"while",    "{",        "||",      "}",      "char",   "double",
	/* 42         43         44        45         46         47      */
	 "for",      "%do",      "goto",   "switch",    "case",     "break",
	 /* 48         49         50        51         52*/
	 "default",    ":",     "%char", "%string", "%double"

};

char* keyword[NO_KEYWORD] = {
	"const",  "else",    "if",    "int",    "return",  "void",    "while",

	//추가 확장 부분
	"char",   "double",  "for",   "do",     "goto",    "switch",  "case",

	"break",  "default"
};


enum tsymbol tnum[NO_KEYWORD] = {
	tconst,    telse,     tif,     tint,     treturn,   tvoid,     twhile,
	//추가 확장 부분
	tchar,     tdouble,   tfor,    tdo,      tgoto,     tswitch,   tcase,

	tbreak,    tdefault
};

struct tokenType scanner()
{
	struct tokenType token;
	int i, index;
	char ch, id[ID_LENGTH];

	token.number = tnull;

	do {
		while (isspace(ch = fgetc(sourceFile))) {	// state 1: skip blanks isspace->공백이면 0반환, fgetc 한글자씩 로드
			col++;
			if (ch == '\n')
			{
				col = 0;
				line++;
			}
		}
		col++;

		if (superLetter(ch)) { // identifier or keyword
			i = 0;
			token.colno = col - 1;
			do {
				if (i < ID_LENGTH) id[i++] = ch;
				ch = fgetc(sourceFile);
				col++;
			} while (superLetterOrDigit(ch));
			if (i >= ID_LENGTH) lexicalError(1);
			id[i] = '\0';
			ungetc(ch, sourceFile);  //  retract
			col--;
			// find the identifier in the keyword table
			for (index = 0; index < NO_KEYWORD; index++)
				if (!strcmp(id, keyword[index])) break;
			if (index < NO_KEYWORD)    // found, keyword exit
			{
				token.number = tnum[index];
				token.colno = col - strlen(tokenName[token.number]);
				token.lineno = line;
			}
			else {                     // not found, identifier exit
				token.number = tident;
				strcpy_s(token.value.id, id);
			}
		}  // end of identifier or keyword

		else if (isdigit(ch)) {  // number
			token.colno = col - 1;
			token.value.num = getNumber(ch);
			ch = fgetc(sourceFile);
			col++;
			if (ch == '.') { //double인 경우
				double tmp;
				token.number = tdoubles; //token number 설정
				ch = fgetc(sourceFile);
				if (isdigit(ch)) {

					ungetc(ch, sourceFile);
					col--;
					if (ch == '0') { //이전 문자가 0일 경우, ex) 2.04 등
						ch = fgetc(sourceFile);
						col++;
						token.value.d_num = token.value.num + 0.1 * getFractional(getNumber(ch)); //이번 숫자에 0.1를 곱한다.
						ch = fgetc(sourceFile);
						col++;
					}
					else { //이전 문자가 0이 아닌 경우,
						ch = fgetc(sourceFile);
						col++;
						token.value.d_num = token.value.num + getFractional(getNumber(ch));
						ch = fgetc(sourceFile);
						col++;
					}

					if (ch == 'e') { //부동소수점
						ch = fgetc(sourceFile);
						col++;
						if (isdigit(ch)) {
							index = getNumber(ch);
							for (i = 0; i < index; i++)
								token.value.d_num *= 10;
						}
						else if (ch == '+') {
							ch = fgetc(sourceFile);
							col++;
							index = getNumber(ch);
							for (i = 0; i < index; i++)
								token.value.d_num *= 10; //+뒤의 수만큼 10을 곱해줌
						}
						else if (ch == '-') {
							ch = fgetc(sourceFile);
							col++;
							index = getNumber(ch);
							for (i = 0; i < index; i++)
								token.value.d_num /= 10; //-뒤의 수만큼 10으로 나눠줌.
						}
						else
						{
							ungetc(ch, sourceFile);
							col--;
						}

					}
					else //부동소수점이 아닌 경우,
					{
						ungetc(ch, sourceFile);
						col--;
					}
				}
				else if (ch == ';') { //X. 경우
					token.value.d_num = token.value.num;
					ungetc(ch, sourceFile);
					col--;
				}
			}
			else // Integer형
			{
				token.number = tnumber;
				ungetc(ch, sourceFile);
				col--;
			}
		}
		else if (ch == '.') // .(dot)이 맨앞인 경우 ex) .24
		{
			token.colno = col - 1;
			ch = fgetc(sourceFile);
			col++;
			if (isdigit(ch)) {
				token.number = tdoubles;
				token.value.num = 0;
				token.value.d_num = getFractional(getNumber(ch));
			}
			else { //뒤의 문자가 숫자가 아니라면 에러 출력
				lexicalError(7);
			}
		}
		else
		{
			token.colno = col - 1;
			switch (ch) {  // special character

			case '/': //Comments
				ch = fgetc(sourceFile);
				col++;
				if (ch == '*') //text or Documented Comments
				{
					ch = fgetc(sourceFile);
					col++;
					if (ch == '\n') {
						col = 0;
						line++;
					}
					if (ch == '*') //Documented Comment 일 때,
					{
						printf("Documented Comments ------> ");
						do {
							while (ch != '*') //Comment 내용 출력
							{

								printf("%c", ch);
								ch = fgetc(sourceFile);
								if (ch == '\n') {
									col = 0;
									line++;
								}
								else
									col++;

							}
							ch = fgetc(sourceFile);
							if (ch == '\n') {
								col = 0;
								line++;
							}
							else
								col++;
						} while (ch != '/');
						printf("\n");

					}
					else // 일반 text Comment 일 때,
					{
						ungetc(ch, sourceFile);
						col--;
						do {
							while (ch != '*')
							{
								ch = fgetc(sourceFile);
								if (ch == '\n') {
									col = 0;
									line++;
								}
								else
									col++;
							}
							ch = fgetc(sourceFile);
							if (ch == '\n') {
								col = 0;
								line++;
							}
							else
								col++;
						} while (ch != '/');
					}

				}

				else if (ch == '/') //line or Single Documented Commenets
				{
					ch = fgetc(sourceFile);
					col++;
					if (ch == '/') //Single Documented Comments 일 때,
					{
						printf("Single Documented Comments ------> ");

						while (ch != '\n') //Comment 내용 출력
						{
							ch = fgetc(sourceFile);
							if (ch == '\n') {
								col = 0;
								line++;
							}
							else
								col++;
							printf("%c", ch);

						}

						printf("\n");
					}
					else // line comment
					{
						ungetc(ch, sourceFile);
						while (fgetc(sourceFile) != '\n')
						{
							col++;
						}
						line++;
						col = 0;
					}
				}
				else if (ch == '=')  token.number = tdivAssign;
				else {
					token.number = tdiv;
					ungetc(ch, sourceFile); // retract
					col--;
				}
				break;
			case '!':
				ch = fgetc(sourceFile);
				col++;
				if (ch == '=')  token.number = tnotequ;
				else {
					token.number = tnot;
					ungetc(ch, sourceFile); // retract
					col--;
				}
				break;
			case '%':
				ch = fgetc(sourceFile);
				col++;
				if (ch == '=') {
					token.number = tremAssign;
				}
				else {
					token.number = tremainder;
					ungetc(ch, sourceFile);
					col--;
				}
				break;
			case '&':
				ch = fgetc(sourceFile);
				col++;
				if (ch == '&')  token.number = tand;
				else {
					lexicalError(2);
					ungetc(ch, sourceFile);  // retract
					col--;
				}
				break;
			case '*':
				ch = fgetc(sourceFile);
				col++;
				if (ch == '=')  token.number = tmulAssign;
				else {
					token.number = tmul;
					ungetc(ch, sourceFile);  // retract
					col--;
				}
				break;
			case '+':
				ch = fgetc(sourceFile);
				col++;
				if (ch == '+')  token.number = tinc;
				else if (ch == '=') token.number = taddAssign;
				else {
					token.number = tplus;
					ungetc(ch, sourceFile);  // retract
					col--;
				}
				break;
			case '-':
				ch = fgetc(sourceFile);
				col++;
				if (ch == '-')  token.number = tdec;
				else if (ch == '=') token.number = tsubAssign;
				else {
					token.number = tminus;
					ungetc(ch, sourceFile);  // retract
					col--;
				}
				break;
			case '<':
				ch = fgetc(sourceFile);
				col++;
				if (ch == '=') token.number = tlesse;
				else {
					token.number = tless;
					ungetc(ch, sourceFile);  // retract
					col--;
				}
				break;
			case '=':
				ch = fgetc(sourceFile);
				col++;
				if (ch == '=')  token.number = tequal;
				else {
					token.number = tassign;
					ungetc(ch, sourceFile);  // retract
					col--;
				}
				break;
			case '>':
				ch = fgetc(sourceFile);
				col++;
				if (ch == '=') token.number = tgreate;
				else {
					token.number = tgreat;
					ungetc(ch, sourceFile);  // retract
					col--;
				}
				break;
			case '|':
				ch = fgetc(sourceFile);
				col++;
				if (ch == '|')  token.number = tor;
				else {
					lexicalError(3);
					ungetc(ch, sourceFile);  // retract
					col--;
				}
				break;

			case '\'': //Character
				ch = fgetc(sourceFile);
				col++;
				if (ch == '\\') //Escape 문자가 들어온 Case,
				{
					token.value.id[0] = ch; //id에 Escape 값 저장
					token.value.id[1] = fgetc(sourceFile);
					token.value.id[2] = '\0'; //char 배열이기 때문에, NULL값 지정				
				}
				else { //Escape 문자가 아닐 때,
					token.value.id[0] = ch; //id에 값 저장
					token.value.id[1] = '\0';
				}
				ch = fgetc(sourceFile);
				col++;
				if (ch == '\'') //끝이 ' 일 때 정상.
					token.number = tchars;
				else            // '이 아니면 에러 출력.
					lexicalError(5);
				break;

			case '\"': //String
				i = 0;
				while ((ch = fgetc(sourceFile)) != '\"' || //끝(")을 만날 때 까지,
					id[i - 1] == '\\')                  //또는 Escape 문자인 \" 가 들어왔을 때,
				{
					col++;
					if (i < ID_LENGTH)  id[i++] = ch;

					else {             //ID_LENGTH 보다 같거나 커질 때,
						lexicalError(1);//ID_LENGTH 초과 시 에러(1)
						break;
					}
					if (ch == ';') { // 끝(")전에 ;이 나올 경우

						ungetc(ch, sourceFile); //한 글자 이전으로 이동
						col--;
						lexicalError(6); // string error 출력
						break;
					}
				}
				if (i < ID_LENGTH) { //길이가 정상
					id[i] = '\0'; //NULL 설정

					if (ch == '\"') { //끝이 정상일 경우			
						strcpy_s(token.value.id, id); //value값 id에 저장
						token.number = tstring; //token의 number 설정
					}
				}
				else { //길이가 비정상			
					   // 그 다음 line 부터 읽어들이기 위해서,
					while ((ch = fgetc(sourceFile)) != '\n')
					{
						col++;
					}
				}
				break;


			case '(': token.number = tlparen;         break;
			case ')': token.number = trparen;         break;
			case ',': token.number = tcomma;          break;
			case ';': token.number = tsemicolon;      break;
			case '[': token.number = tlbracket;       break;
			case ']': token.number = trbracket;       break;
			case '{': token.number = tlbrace;         break;
			case '}': token.number = trbrace;         break;
			case EOF: token.number = teof;            break;
				//추가 확장 colon 연산자
			case ':': token.number = tcolon;          break;

			default: {
				printf("Current character : %c", ch);
				lexicalError(4);
				break;
			}

			} // switch end
		}
		token.lineno = line;
	} while (token.number == tnull);
	return token;
} // end of scanner

void lexicalError(int n)
{
	printf(" *** Lexical Error : ");
	switch (n) {
	case 1: printf("an identifier length must be less than 12.\n");
		break;
	case 2: printf("next character must be &\n");
		break;
	case 3: printf("next character must be |\n");
		break;
	case 4: printf("invalid character\n");
		break;
	case 5: printf("next character must be \'\n"); //character 처리 시, 끝 문자 Error
		break;
	case 6: printf("next character must be \"\n"); //string 처리 시, 끝 문자 Error
		break;
	case 7: printf("next character must be Number\n"); //double 처리 시, 끝 문자 Error
	}
}

int superLetter(char ch)
{
	if (isalpha(ch) || ch == '_') return 1;
	else return 0;
}

int superLetterOrDigit(char ch)
{
	if (isalnum(ch) || ch == '_') return 1;
	else return 0;
}

int getNumber(char firstCharacter)
{
	int num = 0;
	int value;
	char ch;

	if (firstCharacter == '0') {
		ch = fgetc(sourceFile);
		if ((ch == 'X') || (ch == 'x')) {		// hexa decimal
			while ((value = hexValue(ch = fgetc(sourceFile))) != -1)
				num = 16 * num + value;
		}
		else if ((ch >= '0') && (ch <= '7'))	// octal
			do {
				num = 8 * num + (int)(ch - '0');
				ch = fgetc(sourceFile);
			} while ((ch >= '0') && (ch <= '7'));
		else num = 0;						// zero
	}
	else {									// decimal
		ch = firstCharacter;
		do {
			num = 10 * num + (int)(ch - '0');
			ch = fgetc(sourceFile);
		} while (isdigit(ch));
	}
	ungetc(ch, sourceFile);  /*  retract  */
	return num;
}



int hexValue(char ch)
{
	switch (ch) {
	case '0': case '1': case '2': case '3': case '4':
	case '5': case '6': case '7': case '8': case '9':
		return (ch - '0');
	case 'A': case 'B': case 'C': case 'D': case 'E': case 'F':
		return (ch - 'A' + 10);
	case 'a': case 'b': case 'c': case 'd': case 'e': case 'f':
		return (ch - 'a' + 10);
	default: return -1;
	}
}

void printToken(struct tokenType token)
{
	if (token.number == tident)                                                                          //파일이름과 line 수, column 위치를 출력 한다.
		printf("number: %d, value: %s, filename: %s, line: %d, col: %d\n", token.number, token.value.id, token.filename, token.lineno, token.colno);
	else if (token.number == tnumber)
		printf("number: %d, value: %d, filename: %s, line: %d, col: %d\n", token.number, token.value.num, token.filename, token.lineno, token.colno);
	else if (token.number == tchars || token.number == tstring) // character, string 출력
		printf("number: %d, value: %s, filename: %s, line: %d, col: %d\n", token.number, token.value.id, token.filename, token.lineno, token.colno);
	else if (token.number == tdoubles)                          // double 출력
		printf("number: %d, value: %.3lf, filename: %s, line: %d, col: %d\n", token.number, token.value.d_num, token.filename, token.lineno, token.colno);
	else
		printf("number: %d(%s), filename: %s, line: %d, col: %d\n", token.number, tokenName[token.number], token.filename, token.lineno, token.colno);

}