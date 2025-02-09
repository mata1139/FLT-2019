/***************************************************************
*      scanner routine for Mini C language                    *
*                                   2003. 3. 10               *
***************************************************************/

#pragma once


#define NO_KEYWORD 16
#define ID_LENGTH 12

struct tokenType { //toeknType class
	int number;   //token의 고유 number
	union {
		char id[ID_LENGTH]; //identifier
		int num; //실제 value
		double d_num; //double 자료형 value
	} value;

	char* filename; //파일 이름
	int lineno; //라인 수
	int colno;  //column 위치
};


enum tsymbol {
	tnull = -1,
	tnot, tnotequ, tremainder, tremAssign, tident, tnumber, 
	/* 0          1            2         3            4          5     */
	tand, tlparen, trparen, tmul, tmulAssign, tplus,
	/* 6          7            8         9           10         11     */
	tinc, taddAssign, tcomma, tminus, tdec, tsubAssign,
	/* 12         13          14        15           16         17     */
	tdiv, tdivAssign, tsemicolon, tless, tlesse, tassign,
	/* 18         19          20        21           22         23     */
	tequal, tgreat, tgreate, tlbracket, trbracket, teof,
	/* 24         25          26        27           28         29     */
	//   ...........    word symbols ................................. //
	/* 30         31          32        33           34         35     */
	tconst,     telse,        tif,     tint,        treturn,   tvoid,

	/* 36         37          38        39           40         41     */  //추가 확장 부분 (40~ )
	twhile,     tlbrace,      tor,     trbrace,     tchar,    tdouble,      

	/* 42         43          44        45           46         47         48   */
	  tfor,       tdo,       tgoto,   tswitch,      tcase,    tbreak,    tdefault,

	//49          50          51        52
	 tcolon,    tchars,     tstring,   tdoubles
};


struct tokenType scanner();
void printToken(struct tokenType token);
