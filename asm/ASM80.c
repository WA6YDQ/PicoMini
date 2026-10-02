/* newASM80.c update asm80 */
/*
	ASM80.c - updated 8080 assembler based on 2008 asm80
	(C) 2026 Kurt Theis <theis.kurt@gmail.com>
	This is licensed under the MIT license (see enclosed)
	
	Main Rules:
	Labels start at position 0
	Everything else starts after position 0
	Labels have a max length of 40 chars with a max of 1000 labels.
	Opcodes can be lower or UPPER case.
	Directives: ORG, DB, DS, DW, EQU (upper or lower case)
	Numbers 0 (decimal) 10h (hex) and 0x0010 (hex)
	Comments start with ;
	Labels may end with : (removed in processing)
	mvi and lxi opcodes can take the format of: mvi a,10  or mvi a 10
	Equates example: TCB   EQU  1000h, TCBL  EQU  TCB+1
	DB examples: label  DB "This is a String",'$',CR,LF,0
	
	!!NOTE: Strings use double quotes "string" while single chars use single quotes 'a'.
	        This is different from most CP/M assemblers.
	!!NOTE: This is NOT a macro assembler.
	
	Version 1.0 3/3/2026 - Passes all tests.
	
	TODO 


*/

#define _GNU_SOURCE		/* allows strcasestr instruction in string.h */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdint.h>

#define MAXLINE 132		// max line length
#define MAXLEN 80		// for ihex routine

/* Globals */
int labelcount;
char labelname[1000][40];		// hold list of labels
uint16_t labeladdress[1000];	// hold address of label
	
	
	
char opcodes8080[256][10] = { \
"nop", "lxi b", "stax b", "inx b", "inr b", "dcr b", \
"mvi b", "rlc", "__8", "dad b", "ldax b", "dcx b", \
"inr c", "dcr c", "mvi c", "rrc", "_10", "lxi d", \
"stax d", "inx d", "inr d", "dcr d", "mvi d", "ral", \
"_18", "dad d", "ldax d", "dcx d", "inr e", "dcr e", \
"mvi e", "rar", "_20", "lxi h", "shld", "inx h", \
"inr h", "dcr h", "mvi h", "daa", "_28", "dad h", \
"lhld", "dcx h", "inr l", "dcr l", "mvi l", "cma", \
"_30", "lxi sp", "sta", "inx sp", "inr m", "dcr m", \
"mvi m", "stc", "_38", "dad sp", "lda", "dcx sp", \
"inr a", "dcr a", "mvi a", "cmc", "mov b,b", "mov b,c", \
"mov b,d", "mov b,e", "mov b,h", "mov b,l", "mov b,m", "mov b,a", \
"mov c,b", "mov c,c", "mov c,d", "mov c,e", "mov c,h", "mov c,l", \
"mov c,m", "mov c,a", "mov d,b", "mov d,c", "mov d,d", "mov d,e", \
"mov d,h", "mov d,l", "mov d,m", "mov d,a", "mov e,b", "mov e,c", \
"mov e,d", "mov e,e", "mov e,h", "mov e,l", "mov e,m", "mov e,a", \
"mov h,b", "mov h,c", "mov h,d", "mov h,e", "mov h,h", "mov h,l", \
"mov h,m", "mov h,a", "mov l,b", "mov l,c", "mov l,d", "mov l,e", \
"mov l,h", "mov l,l", "mov l,m", "mov l,a", "mov m,b", "mov m,c", \
"mov m,d", "mov m,e", "mov m,h", "mov m,l", "hlt", "mov m,a", \
"mov a,b", "mov a,c", "mov a,d", "mov a,e", "mov a,h", "mov a,l", \
"mov a,m", "mov a,a", "add b", "add c", "add d", "add e", \
"add h", "add l", "add m", "add a", "adc b", "adc c", \
"adc d", "adc e", "adc h", "adc l", "adc m", "adc a", \
"sub b", "sub c", "sub d", "sub e", "sub h", "sub l", \
"sub m", "sub a", "sbb b", "sbb c", "sbb d", "sbb e", \
"sbb h", "sbb l", "sbb m", "sbb a", "ana b", "ana c", \
"ana d", "ana e", "ana h", "ana l", "ana m", "ana a", \
"xra b", "xra c", "xra d", "xra e", "xra h", "xra l", \
"xra m", "xra a", "ora b", "ora c", "ora d", "ora e", \
"ora h", "ora l", "ora m", "ora a", "cmp b", "cmp c", \
"cmp d", "cmp e", "cmp h", "cmp l", "cmp m", "cmp a", \
"rnz", "pop b", "jnz", "jmp", "cnz", "push b", \
"adi", "rst 0", "rz", "ret", "jz", "_cb", \
"cz", "call", "aci", "rst 1", "rnc", "pop d", \
"jnc", "out", "cnc", "push d", "sui", "rst 2", \
"rc", "_d9", "jc", "in", "cc", "_dd", \
"sbi", "rst 3", "rpo", "pop h", "jpo", "xthl", \
"cpo", "push h", "ani", "rst 4", "rpe", "pchl", \
"jpe", "xchg", "cpe", "_ed", "xri", "rst 5", \
"rp", "pop psw", "jp", "di", "cp", "push psw", \
"ori", "rst 6", "rm", "sphl", "jm", "ei", \
"cm", "_fd", "cpi", "rst 7" };

int opcode_bytes8080[256] = { \
1,3,1,1,1,1,2,1,1,1,1,1,1,1,2,1,1,3,1,1,1,1,2, \
1,1,1,1,1,1,1,2,1,1,3,3,1,1,1,2,1,1,1,3,1,1,1,2, \
1,1,3,3,1,1,1,2,1,1,1,3,1,1,1,2,1,1,1,1,1,1,1,1, \
1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, \
1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, \
1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, \
1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, \
1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, \
1,1,1,3,3,3,1,2,1,1,1,3,1,3,3,2,1,1,1,3,2,3,1,2, \
1,1,1,3,2,3,1,2,1,1,1,3,1,3,1,2,1,1,1,3,1,3,1,2, \
1,1,1,3,1,3,1,2,1,1,1,3,1,3,1,2,1 };



void usage(void) {
	printf("Usage: asm80 [source.asm]\n");
	printf("\
	An 8080 Assembler\n\n\
	Main Rules:\n\
	Labels start at position 0\n\
	Everything else starts after position 0\n\
	Labels have a max length of 40 chars with a max of 1000 labels.\n\
	Opcodes can be lower or UPPER case.\n\
	Directives: ORG, DB, DS, EQU (upper or lower case)\n\
	Numbers 0 (decimal) 10h (hex) and 0x0010 (hex)\n\
	Comments start with ;\n\n\
	After assembly, three files are created:\n\
	source.prn		An output listing \n\
	source.com		Assembled object code\n\
	source.hex		Intel hex file ready for upload\n\n\
	Any errors found during assembly will stop the \n\
	assembler process and show the error with the line number\n\
	and offending line from the source.asm file\n\
	\n");
	return;
}


/*
*	isReserved() -  check if passed value is a reserved
*			word or character. Typically an opcode
*			or some such.
*
*/
int isReserved(char *token){	/* return 1 if match to reserved work, 0 if not */
int n;
const char reserved[21][8] = {"db",".db","DB",".DB", \
    		   "ds",".ds","DS",".DS", \
	           "dw",".dw","DW",".DW", \
     		   "org",".org","ORG",".ORG", \
		   "equ",".equ","EQU",".EQU"};

	for (n=0; n<255; n++){
		if (strcmp(token,opcodes8080[n])==0)	// test all opcodes
			return 1;
	}

	for (n=0; n<20; n++) {
		if (strcmp(token,reserved[n])==0)	// other reserved words
		    	return 1;
	}

	return 0;	/* everything else is OK */
}

/*
*	isOpcode() - check if passed value is a valid
*	opcode in 8080 format. If not, return -1
*	else return the size of the opcode, 1, 2 or 3
*/

int isOpcode(char *token){	/* check if opcode - return byte size or -1 */
int n;
	for (n=0; n<256; n++)
		if (strcmp(token,opcodes8080[n])==0)
			return opcode_bytes8080[n];
	return -1;	/* token not an opcode */
}

int hex2dec(char h){            /* convert single hex digit to decimal */
int byte;
        if(isalpha(h)){     /* 1st digit */
                byte = toupper(h);
                byte = byte - 55;
        }
        if (isdigit(h)){
                byte = h - 48;
        }
        return(byte);
}

int getAddr(char *addr){	/* get address of passed variable - numbers can be 1234, 1234h, 0x1234 */
int n;
char value[10];

	if (strlen(addr)==0)	/* discard bad token (last token will be null in pass 2) */
		return -1;


	/* for 0xnn type of hex numbers */
	if (addr[1] == 'x') {
		n=strtol(addr,NULL,0);
		return n;
	}

	/* for 10h, 1234h type of hex numbers */
	if ((addr[strlen(addr)-1] == 'h') || (addr[strlen(addr)-1] == 'H')){ /* hex number */
		if (strlen(addr)==2) {	/* ex 8h */
			n = hex2dec(addr[0]);
			if (n < 0 || n > 255) return -1;
			return n;
		}
		if (strlen(addr)==3){	/* ex 45H */
			n = hex2dec(addr[1]);
			n += hex2dec(addr[0]) * 16;
			if (n < 0 || n > 255)	/* error checking */
				return -1;
			return n;
		}
		if (strlen(addr)==4){	/* ex 100H */
			n = hex2dec(addr[2]);
			n += hex2dec(addr[1]) * 16;
			n += hex2dec(addr[0]) * 256;
			if (n < 0 || n > 4095)	/* error checking */
				return -1;
			return n;
		}
		if (strlen(addr)==5){	/* ex 2345H */
			n = hex2dec(addr[3]);
			n += hex2dec(addr[2]) * 16;
			n += hex2dec(addr[1]) * 256;
			n += hex2dec(addr[0]) * 4096;
			if (n < 0 || n > 65535)	/* error checking */
				return -1;
			return n;
		}
		return -1;	/* wrong size */
	}
	if ((strlen(addr)>1) && (atoi(addr)==0))	/* bad number */
		return -1;

	if ((strlen(addr)==1) && (isdigit(addr[0])==0))	/* not a number */
		return -1;

	/* address is decimal */
	n = atoi(addr);
	if ((n < 0) || (n > 65535))		/* error checking */
		return -1;
	return n;
}

/*
*	isLabel() - check is passed token is a valid label
*	Return -1 if token is not in the pre-defined list
*	of labels, or if it is return the address or number
*	of the label references
*/

int isLabel(char *token){	/* check if token is a label - return -1 or address */
int n;
	for (n=0; n<=labelcount; n++){
		if (strcmp(labelname[n],token)==0)
			return labeladdress[n];
	}
	return -1;	/* token not a label */
}


/*
*	getOpCode() - find value of opcode from supplied
*	token. Should never see an error since program 
*	calls isOpcode() before calling this. Return
*	single byte value of op code.
*/

int getOpCode(char *token){	/* find opcode, return value */
int n;
	for (n=0; n<256; n++)
		if (strcmp(token,opcodes8080[n])==0)
			return n;
	return -1;	/* token not an opcode */
}




/************/
/*** MAIN ***/
/************/

int main(int argc, char **argv) {

	int COMMENT;	// 1 if line is comment only
	int linepos;	// current position in line
	uint16_t address;		// numerical address 
	uint16_t STARTADDRESS;	// lowesr address found
	
	int LINENUMBER;
	int res;
	
	char teststring[40];			// get and test a word
	uint8_t MEM[65536];		/* set up storage for 8 bit values for output */
	memset(MEM,0,65536);
	
	int PASS;
	
	if (argc == 1) {
		usage();
		return 0;
	}

	FILE *infile;
	FILE *comfile;
	FILE *prnfile;
	FILE *hexfile;

	/* source file .asm */
	infile = fopen(argv[1],"r");
	if (infile == NULL) {
		printf("err1: Error opening %s\n",argv[1]);
		return 1;
	}
	
	/* print file .prn */
	char prnfilename[40] = {'\0'};
	for (int n=0; n<strlen(argv[1]); n++) {
		prnfilename[n] = argv[1][n];
		if (prnfilename[n] == '.') break;
	}
	strcat(prnfilename,"prn");
	prnfile = fopen(prnfilename,"w");
	if (prnfile == NULL) {
		printf("err2: Error creating file %s\n",prnfilename);
		return 1;
	}
	
	/* hex file .hex */
	char hexfilename[40] = {'\0'};
	for (int n=0; n<strlen(argv[1]); n++) {
		hexfilename[n] = argv[1][n];
		if (hexfilename[n] == '.') break;
	}
	strcat(hexfilename,"hex");
	
	
	char line[MAXLINE+1];
	labelcount=0;		// running count of labels
	for (int n=0; n<1000; n++) memset(labelname[n],0,40);
	for (int n=0; n<1000; n++) labeladdress[n] = -1;
	
	LINENUMBER = 0;
	address = 0;
	STARTADDRESS = 65535;
	PASS = 1;
	
	/******************/
	/*** Start PASS ***/
	/******************/
	startpass:
	printf("PASS %d\n",PASS);
	fprintf(prnfile,"*** Starting PASS %d\n",PASS);
	
	/* read infile line by line. Determine address, assign labels to variable names */
	
	while (1) {
		memset(line,0,MAXLINE);
		fgets(line,MAXLINE,infile);
		if (feof(infile)) break;
		COMMENT = 0;
		LINENUMBER++;
		
		/* check for initial \0 or \n */
		if (line[0] == '\n') continue;
		
		/* check for initial comment */
		for (int linepos=0; linepos<MAXLINE; linepos++) {
			if (line[linepos] == '\0') break;
			if (line[linepos] == '\n') break;
			if (isalnum(line[linepos])) break;
			if (isspace(line[linepos])) continue;
			if (line[linepos] == ';') {
				COMMENT=1;
				break;
			}
		}
		if (COMMENT) continue;		// line starts with a comment - ignore it
		
		
		
		/* strip \n from line */
		char *p = strchr(line,'\n');
		if (p) *p = '\0';
		
		/* show line */
		//printf("%d: [%s]\n",LINENUMBER,line);
		
		/* Split line into tokens */
		
		char lab[40] = {'\0'};
		char op1[40] = {'\0'}, op2[40] = {'\0'}, op3[40] = {'\0'}, op4[40] = {'\0'}, op5[40] = {'\0'};
		sscanf(line,"%s %s %s %s %s",op1,op2,op3,op4,op5);
		/* remove , after lxi and mvi */
		/* no label */
		if (isspace(line[0])) { 
			if (!strcmp(op1,"lxi") || !strcmp(op1,"LXI") || !strcmp(op1,"mvi") || !strcmp(op1,"MVI")) {
				char *p = strchr(line,','); if (p) *p = ' ';
				sscanf(line,"%s %s %s %s %s",op1,op2,op3,op4,op5);
			}
		}
		/* has label */
		if (!isspace(line[0])) { 
			if (!strcmp(op2,"lxi") || !strcmp(op2,"LXI") || !strcmp(op2,"mvi") || !strcmp(op2,"MVI")) {
				char *p = strchr(line,','); if (p) *p = ' ';
				sscanf(line,"%s %s %s %s %s",op1,op2,op3,op4,op5);
			}
		}

		
		
		/* test if op1 is a label: should start in pos 0 */
		if (isalpha(line[0])) {		// label
			// strip off trailing :
			char *p = strchr(op1,':');
			if (p) *p = '\0';
			// not a label?
			res = isReserved(op1);
			if (res == 1) {
				printf("err6: Reserved word [%s] in position 0 \nLine %d: %s\n",op1,LINENUMBER,line);
				return 1;
			}
			/* test if it's an already assigned label */
			if (PASS == 1) {
				res = isLabel(op1);
				if (res != -1) {
					printf("err7: Duplicate label [%s]\nLine %d: %s\n",op1,LINENUMBER,line);
					return 1;
				}
				/* a label, not assigned. Assign it */
				strcpy(labelname[labelcount],op1);
				labeladdress[labelcount] = address;
				labelcount++;
				strcpy(lab,op1);	// for later
				
			}
			
			
			memset(op1,0,40); memset(op2,0,40); memset(op3,0,40); memset(op4,0,40); memset(op5,0,40);
			sscanf(line,"%s %s %s %s %s %s",lab,op1,op2,op3,op4,op5);
			
			//printf("redo::lab:%s op1:%s  op2:%s  op3:%s  op4:%s  op5:%s\n",lab,op1,op2,op3,op4,op5);
			if (PASS == 2)
				fprintf(prnfile,"\n%04X %s\n",address,lab);
			if (strlen(lab) != 0 && strlen(op1) == 0)	// label only - get next line 
				continue;
		}
			
		/* test for random spaces, tabs, etc in position 0 */
		if (!isalpha(op1[0])) continue;
			 
			 
		/* test for END directive - immediately stop current pass */
		if (!strcmp(op1,"end") || !strcmp(op1,"END")) {
			if (PASS == 2) {
				printf("'%s' directive found at address %04X\n",op1,address);
				fprintf(prnfile,"%s at address %04X\n",op1,address);
			}
			break;
		}
			 
		/* test for IF/ENDIF */
			
			
			
			
		/* test for org/ORG - must start after position 0! */
		if (!strcmp(op1,"org") || !strcmp(op1,"ORG")) {
			if (line[0] == 'o' || line[0] == 'O') {		// org cannot start in pos 0
				printf("err3: Reserved word [%s] in position 0 \nLine %d: %s\n",op1,LINENUMBER,line);
				return 1;
			}
			res = isLabel(op2);
			if (res == -1) 
				res = getAddr(op2);
			if (res == -1) {
				printf("err4: Bad address [%s]\nLine %d: %s\n",op2,LINENUMBER,line);
				return 1;
			}
			address = res;
			if (res < STARTADDRESS) STARTADDRESS = res;
			if (PASS == 2) fprintf(prnfile,"%s 0x%04X\n",op1,address);
			continue;
			
			#ifdef DEAD
			/* test for label */
			res = isLabel(op2);
			if (res != -1) {
				address = res;
				continue;
			}
			res = getAddr(op2);
			if (res == -1) {
				printf("err5: Bad address [%s]\nLine %d: %s\n",op2,LINENUMBER,line);
				return 1;
			}
			address = res;
			//printf("New address assigned in Line %d: %s\n",LINENUMBER,line);
			continue;
			#endif
			
		}
			 
			 
		/* test for equ/EQU after label */
		if (PASS == 1) {
			if (!strcmp(op1,"equ") || !strcmp(op1,"EQU")) {
				if (strlen(lab) == 0) {
					printf("err8: EQU statement without label\nLine %d: %s\n",LINENUMBER,line);
					return 1;
				}
				char *p = strchr(op2,'+');	// label equ (value/label)+10
				if (p) {
					char lab1[40]={'\0'},lab2[40]={'\0'};
					*p = ' ';	// remove +
					sscanf(op2,"%s %s",lab1,lab2);
					labelcount--;
					res = isLabel(lab1);
					if (res == -1) res = getAddr(lab1);
					int res2 = isLabel(lab2);
					if (res2 == -1) res2 = getAddr(lab2);
					res += res2;
					labeladdress[labelcount] = res;
					labelcount++;
					continue;
				}
				// re-assign label address to value after equ
				labelcount--;
				res = isLabel(op2);
				if (res == -1) 
					res = getAddr(op2);		// label equ (value/label)
				if (res == -1) {
					if (!strcmp(op2,"$"))	// $ denotes current address
					res = address;
				}	 
				if (res == -1) {
					printf("err9: Bad value after equ [%s]\nLine %d: %s\n",op2,LINENUMBER,line);
					return 1;
				}
				labeladdress[labelcount] = res;
				labelcount++;
				continue;
			}
		}
		if (PASS == 2) {
			if (!strcmp(op1,"equ") || !strcmp(op1,"EQU")) continue;
		}
			
			
			
			 
		/* test for db/DB in op1 position */
		if (!strcmp(op1,"db") || !strcmp(op1,"DB")) {
			/* test for blank in next position */
			if (strlen(op2)==0) {
				printf("err10: Missing value after DB \nLine %d: %s\n",LINENUMBER,line);
				return 1;
			}
			if (PASS == 2) fprintf(prnfile,"%04X %s\n",address,line);
			char *hay = line;
			char *pos = strcasestr(hay,"db");
			pos += 2;		// point to 1st space after db
			int n=0;
			char val[40];
			int res = 0;
dbloop:
			memset(val,0,40);
			n=0;
			if (isspace(*pos)) while (isspace(*pos)) pos++;
			if (*pos == '\0') continue;
			if (*pos == ';') continue;
			if (*pos == ',') {
				pos++;
				goto dbloop;
			}
			if (*pos == '\'') {		// single quote ie '$'
				if (PASS == 1) {
					address++;
					pos += 3;
					goto dbloop;
				}
				if (PASS == 2) {
					pos++;
					MEM[address++] = *pos;
					pos += 2;
					goto dbloop;
				}
			}
			if (*pos == '"') {		// "string"
				pos++;	// skip "
				while (*pos != '"') {
					if (*pos == '\0') {
						printf("err10a: Unterminated string after DB\nLine %d: %s\n",LINENUMBER,line);
						return 1;
					}
					if (PASS == 1) {
						address++; pos++;
						continue;
					}
					if (PASS == 2) {
						MEM[address++] = *pos;
						pos++;
						continue;
					}
				}
				/* end of " */
				pos++;
				goto dbloop;
			}
				
			if (isalnum(*pos)) {	// 12h,10,0x01
				while (isalnum(*pos)) {
					val[n++] = *pos;		// get alnum chars (label, value)
					pos++;
				}
			}
			res = isLabel(val);
			if (res != -1) {
				if (PASS == 1) {
				address++;
				goto dbloop;
				}
				if (PASS == 2) {
					MEM[address++] = res;
					goto dbloop;
				}
			}
			res = getAddr(val);
			if (res == -1) {
				printf("err10b: Bad value %s after DB\nLine %d: %s\n",val,LINENUMBER,line);
				return 1;
			}
			if (PASS == 1) {
				address++;
				goto dbloop;
			}
			if (PASS == 2) {
				MEM[address++] = res;
				goto dbloop;
			}
		}	/* end of db test */	  
			
			
			
		/* test for DW in op1 position */
		if (!strcmp(op1,"dw") || !strcmp(op1,"DW")) {
			if (strlen(op2)==0) {
				printf("err14a: Missing value after DW\nLine %d: %s\n",LINENUMBER,line);
				return 1;
			}
			res = isLabel(op2);
			if (res == -1) 
				res = getAddr(op2);
			if (res == -1) {
				printf("err14b: Bad value after DW\nLine %d: %s\n",LINENUMBER,line);
				return 1;
			}
			if (PASS == 1) {
				address += 2;
				continue;
			}
			/* not quite sure about this - DW should be 2 bytes only... */
			if (PASS == 2) {
				MEM[address] = (res & 0x00ff);
				MEM[address+1] = (res & 0xff00) >> 8;
				fprintf(prnfile,"%04X %02X %02X %s\n",address,MEM[address],MEM[address+1],line);
				address += 2;
				continue;
			}
		}
			
		
		
		
		/* test for ds/DS in op1 position */
		if (!strcmp(op1,"ds") || !strcmp(op1,"DS")) {
			/* test for blank in next position */
			if (strlen(op2)==0) {
				printf("err14: Missing value after DS \nLine %d: %s\n",LINENUMBER,line);
				return 1;
			}
			/* test for a number(s) after ds */
			res = getAddr(op2); // printf("after DS: res 0x%04X\n",res);
			if (res == -1) {
				printf("err15: Bad address after DS [%s]\nLine %d: %s\n",op2,LINENUMBER,line);
				return 1;
			}
			if (PASS == 2) fprintf(prnfile,"%04X %s DS %02X\n",address,lab,res);
			if (PASS == 1) {
				address += res;
				continue;
			}
			if (PASS == 2) {
				address += res;
				continue;
			}

			printf("err16: Unknown operand %s after DS \nLine %d: %s\n",op2,LINENUMBER,line);
			return 1;
		}
		
		
		
					
		
		/* 
			Test for opcode - if so, increment address by opcode size (1-3)
			
			Convert opcodes to lower case - some assemblers only use UPPER CASE
			and retyping everything is wasteful.
			
		*/
		
		char opcode[8] = {'\0'};	// 1st opcode (mov)
		char opcode2[40] = {'\0'};	// part (m,a)
		char opcodelong[140] = {'\0'};
		// convert opcode to lower case
		for (int n=0; n<strlen(op1); n++) opcode[n] = tolower(op1[n]);
		for (int n=0; n<strlen(op2); n++) opcode2[n] = tolower(op2[n]);
		/* ignore comments */
		if (op2[0] == ';') op2[0] = '\0';
		if (op3[0] == ';') op3[0] = '\0';
		

		/* test for single byte opcode (nop, DI, ret, etc) */
		if (strlen(op2)==0 && isOpcode(opcode)==1) {
			if (PASS == 1) {
				address += 1;
				continue;
			}
			if (PASS == 2) {
				fprintf(prnfile,"%04X %02X      %s\n",address,getOpCode(opcode),opcode);
				MEM[address++] = getOpCode(opcode);
				continue;
			}
		}
		
		/* test if op1 needs to be combined with op2 */
		strcpy(opcodelong,opcode); strcat(opcodelong," "); strcat(opcodelong,opcode2);
	
		res = isOpcode(opcodelong);
		if (res == 1) {		// mov a,b
			if (PASS == 1) {
				address += res;
				continue;
			}
			if (PASS == 2) {
				fprintf(prnfile,"%04X %02X      %s\n",address,getOpCode(opcodelong),opcodelong);
				MEM[address++] = getOpCode(opcodelong);
				continue;
			}
		}
		if (res == 2) {		// mvi a 10
			if (PASS == 1) {
				address += res;
				continue;
			}
			if (PASS == 2) {
				if (strlen(op3)==0) {
					printf("err17: Missing label/address in 2 byte opcode%s \nLine %d: %s\n",opcodelong,LINENUMBER,line);
					return 1;
				}
				fprintf(prnfile,"%04X %02X ",address,getOpCode(opcodelong));
				MEM[address++] = getOpCode(opcodelong);
				res = isLabel(op3);
				/* test single quotes char next */
				if (res == -1) {	// single quoted char ie. cpi 'x'
					char *p = strchr(line,'\'');
					if (p) 
						if (*(p+2) == '\'') res = *(p+1);
				}
				if (res == -1) 
					res = getAddr(op3);
				if (res == -1) {
					printf("err18: Bad address in 2 byte opcode %s\nLine %d: %s\n",opcodelong,LINENUMBER,line);
					return 1;
				}
				fprintf(prnfile,"%02X   %s %s\n",res,opcodelong,op3);
				MEM[address++] = res;
				continue;
			}
			printf("err19: Exception in Line %d: %s\n",LINENUMBER,line);
			return 1;
		}
		if (res == 3) {			// jmp 0100, call 0100, lxi sp stack
			if (PASS == 1) {
				address += res;
				continue;
			}
			if (PASS == 2) {
				if (strlen(op3)==0) {
					printf("err20: Missing label/address in 2 byte opcode%s \nLine %d: %s\n",opcodelong,LINENUMBER,line);
					return 1;
				}
				fprintf(prnfile,"%04X %02X ",address,getOpCode(opcodelong));
				MEM[address++] = getOpCode(opcodelong);
				res = isLabel(op3);
				if (res == -1) res = getAddr(op3);
				if (res == -1) {
					printf("err21: Bad address in 2 byte opcode %s\nLine %d: %s\n",opcodelong,LINENUMBER,line);
					return 1;
				}
				fprintf(prnfile,"%04X %s %s\n",res,opcodelong,op3);
				MEM[address++] = (res & 0x00ff);			// LSB
				MEM[address++] = (res & 0xff00) >> 8;		// MSB
				continue;
			}
			printf("err22: Exception in Line %d: %s\n",LINENUMBER,line);
			return 1;
		}
		
		/* test for 2 or 3 byte but with single opcode (out x, in x, jmp nnnn, call nnmm) */	
		res = isOpcode(opcode);
		if (res == 2) {				// out 10
			if (PASS == 1) {
				address += res;
				continue;
			}
			if (PASS == 2) {	// this is duplicated from above
				fprintf(prnfile,"%04X %02X ",address,getOpCode(opcode));
				MEM[address++] = getOpCode(opcode);
				res = isLabel(op2);		// label
				
				if (res == -1) {	// single quoted char ie. cpi 'x'
					char *p = strchr(line,'\'');
					if (p) 
						if (*(p+2) == '\'') res = *(p+1);
				}	
				if (res == -1)
					res = getAddr(op2);	// address			
				if (res == -1) {
					printf("err23: Bad label/address in 2 byte opcode %s\nLine %d: %s\n",opcode,LINENUMBER,line);
					return 1;
				}
				fprintf(prnfile,"%02X   %s %s\n",res,opcode,op2);
				MEM[address++] = res;
				continue;
			}
		}
		if (res == 3) {			// jmp 0100, call 0001
			if (PASS == 1) {
				address += res;
				continue;
			}
			if (PASS == 2) {
				fprintf(prnfile,"%04X %02X ",address,getOpCode(opcode));
				MEM[address++] = getOpCode(opcode);
				if (strlen(op2) == 0) {
					printf("err24a: Missing address in 3 byte opcode %s\nLine %d: %s\n",opcode,LINENUMBER,line);
					return 1;
				}
				res = isLabel(op2);
				if (res == -1) res = getAddr(op2);
				if (res == -1) {
					printf("err24: Bad address in 3 byte opcode %s\nLine %d: %s\n",opcode,LINENUMBER,line);
					return 1;
				}
				fprintf(prnfile,"%04X %s %s\n",res,opcode,op2);
				MEM[address++] = (res & 0x00ff);			// LSB
				MEM[address++] = (res & 0xff00) >> 8;		// MSB
				continue;
			}	
			printf("err25: Exception in Line %d: %s\n",LINENUMBER,line);
			return 1;	
		}
		
		
		/* something unknown - show error and stop */
		printf("err26: Unknown token [%s] \nLine %d: %s\n",op1,LINENUMBER,line);
		res = isOpcode(op1);
		printf("opcode size is %d\n",res);
		return 1;
		
		/* nothing else to do */
	}
	
	/***************/
	/* end of pass */
	/***************/
	
	
	/* send a list of labels and their addresses to the print file */
	if (PASS == 2) {
	fprintf(prnfile,"\nLabels:\n");
	for (int n=0; n<labelcount; n++)
		fprintf(prnfile,"%16s   0x%04X   %d(d)\n",labelname[n],labeladdress[n],labeladdress[n]);
	
	fprintf(prnfile,"\nLast address %d(d)    0x%04X\n",address-1,address-1);
	fprintf(prnfile,"\n");
	}
	
	PASS += 1;
	rewind(infile);
	LINENUMBER = 0;
	if (PASS == 2) goto startpass;
	
	
	/* show memory */
	#ifdef DEAD
	uint16_t memstart = 0;
	uint8_t col = 0;
	while (1)  {
		printf("%04X  ",memstart);
		for (col = 0; col < 16; col++) printf("%02X ",MEM[memstart+col]);
		printf("  ");
		for (col = 0; col < 16; col++) {
			if (isprint(MEM[memstart+col])) 
				printf("%c",MEM[memstart+col]);
			else
				printf(".");
		}	
		printf("\n");
		memstart += col;
		if (memstart < address) continue;
		break;
	}
	#endif
	
	/* What is lowest address? */
	printf("Lowest Address Found is %d\n",STARTADDRESS);
	
	/* write memory to .com file */
	// .com file is basically a memory dump starting at the
	// lowest address found
	char comfilename[40] = {'\0'};
	for (int n=0; n<strlen(argv[1]); n++) {
		comfilename[n] = argv[1][n];
		if (comfilename[n] == '.') break;
	}
	strcat(comfilename,"com");
	comfile = fopen(comfilename,"w");
	if (comfile == NULL) {
		printf("err27: Error creating file %s\n",comfilename);
		return 1;
	}
	// save the data from the lowest address used
	for (int n=STARTADDRESS; n<address; n++)
		fprintf(comfile,"%c",MEM[n]);
	fclose(comfile);		
	
	printf("Assembly complete. \n");
	
	/* create Intel hex file from memory */
	int checksum = 0;
	int filelength = address;
	int runaddress = STARTADDRESS;		// 0;
	int adr = STARTADDRESS;  // 0;
	int cc = 0, cnt = 0;
	uint8_t addrhi, addrlo, byteval;
	hexfile = fopen(hexfilename,"w");
	if (hexfile == NULL) {
		printf("err28: Error creating file %s\n",hexfilename);
		return 1;
	}
	/* start conversion from binary to hex */
	while (filelength - adr > 0) {
		fprintf(hexfile,":");
		if (filelength - adr >= 16)
			cc = 16;
		else
			cc = filelength - adr;
		/* start checksum for this line */
		checksum = cc;
		byteval = cc;
		/* write byte count to file */
		fprintf(hexfile,"%2.2X",byteval);
		/* convert address to 2 bytes */
		addrhi = (adr & 0xff00) >> 8;
		addrlo = adr & 0x00ff;
		/* save address to hex file */
		fprintf(hexfile,"%2.2X",addrhi);
		fprintf(hexfile,"%2.2X",addrlo);
		/* add to running checksum */
		checksum += addrhi;
		checksum += addrlo;
		/* this is a normal record, output type */
		fprintf(hexfile,"%2.2X",0);
		/* get output and data types */
		for (int n=0; n != cc; n++) {
			byteval = MEM[runaddress++];
			fprintf(hexfile,"%2.2X",byteval);
			checksum += byteval;
			adr++;
		}
		byteval = ~checksum + 1;		// two's compliment
		fprintf(hexfile,"%2.2X",byteval);
		fprintf(hexfile,"\n");
	}
	/* mem bytes all read, create last record of :00000001FF */
	fprintf(hexfile,":00000001FF\n");		// bytecount addrhi addrlo endchar checksum
	/* done */
	fclose(hexfile);
	printf("Hex file Written\n");
	
	
	
	/* close up */
	fclose(infile);
	fclose(prnfile);
	return 0;
}
			
		
		
			
