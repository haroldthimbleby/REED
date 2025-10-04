#include "header.h"
#include <stdarg.h>

#define MAXERRORS 5 // before quitting

extern int p, eof;
extern char *buffer;
extern str *lex1, *lex2, *lex3;
extern int lineno, nextlineno, startline, nextstartline;

int errcount = 0;

void nolineerror(char *fmt, ...)
{ fprintf(stderr, "Error: ");
      va_list ap;
       int d;
       char c, *s;
       va_start(ap, fmt);
       while( *fmt )
	   {	if( *fmt == '%' )
       		{	fmt++;
	       		switch(*fmt)
	       		{
		               case '%':
	               			fprintf(stderr, "%%");
	               			break;
		               case 's':                      
		                       s = va_arg(ap, char *);
		                       fprintf(stderr, "%s", s);
		                       break;
		               case 'd':                       
		                       d = va_arg(ap, int);
		                       fprintf(stderr, "%d", d);
		                       break;
		               case 'c':                       
		                       /* Note: char is promoted to int. */
		                       c = va_arg(ap, int);
		                       fprintf(stderr, "%c", c);
		                       break;
                    case 'N':
                        for( s = va_arg(ap, char *); *s; s++ )
                             switch( *s )
                                {
                                    case '\n': fprintf(stderr, " "); break;
                                    case '\t': fprintf(stderr, " "); break;
                                    default:   fprintf(stderr, "%c", *s);
                                }
                        break;
                    default:
		                		fprintf(stderr, " !!!(1) unknown format %%%c !!!\n", *fmt);
		                		break;
		       	}
		       	fmt++;
       		}
       		else
       			fprintf(stderr, "%c", *fmt++);       
       }
       va_end(ap);
       fprintf(stderr, "\n");
	if( ++errcount > MAXERRORS )
	{	fprintf(stderr, "... too many errors!\n");
		exit(1);
	}
}

void error(char *fmt, ...)
{	fprintf(stderr, "Error on line%s %d", lex1->lineno == lineno? "": "s", lex1->lineno);
	if( lex1->lineno != lineno ) fprintf(stderr, "-%d", lineno);
	fprintf(stderr, ": ");
	fflush(stderr);
      va_list ap;
       int d;
       char c, *s;
       va_start(ap, fmt);
       while( *fmt )
	   {	if( *fmt == '%' )
       		{	fmt++;
	       		switch(*fmt)
	       		{
		               case '%':
	               			fprintf(stderr, "%%");
	               			break;
		               case 's':                      
		                       s = va_arg(ap, char *);
		                       fprintf(stderr, "%s", s);
		                       break;
		               case 'd':                       
		                       d = va_arg(ap, int);
		                       fprintf(stderr, "%d", d);
		                       break;
		               case 'c':                       
		                       /* Note: char is promoted to int. */
		                       c = va_arg(ap, int);
		                       fprintf(stderr, "%c", c);
		                       break;
		                default:
		                		fprintf(stderr, " !!!(2) unknown format %%%c !!!\n", *fmt);
		                		break;
		       	}
		       	fmt++;
       		}
       		else
       			fprintf(stderr, "%c", *fmt++);       
       }
       va_end(ap);
       fprintf(stderr, "\n");
	for( int i = startline; i < p && buffer[i] != '\n' && buffer[i]; i++ )
	{	fprintf(stderr, "%c", buffer[i]);
	}
	fprintf(stderr, "\n");
	{	for( int i = startline; i < p-1; i++ )
			fprintf(stderr, " ");
		fprintf(stderr, "^\n");
	}
	if( ++errcount > MAXERRORS )
	{	fprintf(stderr, "... too many errors!\n");
		exit(1);
	}
}

void myfprintf(FILE *opfd, char *fmt, ...)
{	 va_list ap;
       int d;
       char c, *s;
       va_start(ap, fmt);
       while( *fmt )
	   {	if( *fmt == '%' )
       		{	fmt++;
	       		switch(*fmt)
	       		{
		               case '%':
	               			fprintf(opfd, "%%");
	               			break;
		               case 's':                      
		                       s = va_arg(ap, char *);
		                       fprintf(opfd, "%s", s);
		                       break;
		               case 'S':	// used in void dot(..) - preserves \l
		               		for( s = va_arg(ap, char *); *s; s++ )
		                       		switch( *s )
		                       		{
			                       		case '\n': fprintf(opfd, "\\n"); break;
			                       		case '\t': fprintf(opfd, "\\t"); break;
			                       		case '\\': 
			                       			//fprintf(stderr, "%c%c\n", *s, s[1]);
			                       			if( s[1] == 'l' || s[1] == 'r' )
			                       			{
			                       				fprintf(opfd, "\\%c", *++s);
			                       			}
			                       			else
			                       				fprintf(opfd, "\\\\"); break;
			                       		default:   fprintf(opfd, "%c", *s);
		                       		}
		                       break;
		               case 'l':	// for displaying translation rules: makes newlines visible - used in explainTranslationRules()
		               		for( s = va_arg(ap, char *); *s; s++ )
		                       		switch( *s )
		                       		{
			                       		case '\n': fprintf(opfd, "\\n"); break;
										case ' ':  fprintf(opfd, "*"); break;
			                       		case '\t': fprintf(opfd, "\\t"); break;
			                       		case '\\': 
			                       			//fprintf(stderr, "%c%c\n", *s, s[1]);
			                       			if( s[1] == 'l' || s[1] == 'r' )
			                       			{
			                       				fprintf(opfd, "\\%c", *++s);
			                       			}
			                       			else
			                       				fprintf(opfd, "\\"); break;
			                       		default:   fprintf(opfd, "%c", *s);
		                       		}
                           break;
		               case 'j':    // same as %s but works with JSON                
                           for( s = va_arg(ap, char *); *s; s++ )
                                switch( *s )
                                {
                                    case '\n': fprintf(opfd, "\\n"); break;
                                    case '\t': fprintf(opfd, "\\t"); break;
                                    case '\\': fprintf(opfd, "\\\\"); break;
                                    default:   fprintf(opfd, "%c", *s);
                                }
                           break;
                    case 't':    // same as %s but works with tex  - \n becomes space
                        for( s = va_arg(ap, char *); *s; s++ )
                             switch( *s )
                                {
                                    case '\n': fprintf(opfd, " "); break;
                                    case '\t': fprintf(opfd, " "); break;
                                    //case '\\': fprintf(opfd, "\\\\"); break;
                                    case '%':  fprintf(opfd, "\\%%"); break;
                                    default:   fprintf(opfd, "%c", *s);
                                }
                        break;
                    case 'N':    // same as %s but newline becomes space
                        for( s = va_arg(ap, char *); *s; s++ )
                             switch( *s )
                                {
                                    case '\n': fprintf(opfd, " "); break;
                                    case '\t': fprintf(opfd, " "); break;
                                    default:   fprintf(opfd, "%c", *s);
                                }
                        break;
                 case 'i': // indent, same as %s but prints \n as \n      with spaces so it can be more easily seen
                    {   int charsonline = 0;
                        const char *newlinestr = "\n>>    ";
                        for( s = va_arg(ap, char *); *s; s++ )
                            switch( *s )
                            {
                                case '\n': fprintf(opfd, "%s", newlinestr); break;
                                case '\t': fprintf(opfd, " "); break;
                                default:   if( charsonline++ > 80 && (*s == ' ' || *s == '\t') )
                                {   fprintf(opfd, "%s", newlinestr);
                                    charsonline = 0;
                                }
                                    fprintf(opfd, "%c", *s);
                            }
                    }
                        break;

                    case 'T':    // same as %t but also converts characters like  ” “
		                       for( s = va_arg(ap, char *); *s; s++ )
		                       		switch( *s )
		                       		{	case '\n': fprintf(opfd, " "); break;
			                       		case '\t': fprintf(opfd, " "); break;
			                       		//case '\\': fprintf(opfd, "\\\\"); break;
			                       		case '%':  fprintf(opfd, "\\%%"); break;
			                       		default:   fprintf(opfd, "%c", *s);
		                       		}
		                       break;
                    case 'm':    // same as %t but for mathematica input
		                       for( s = va_arg(ap, char *); *s; s++ )
		                       		switch( *s )
		                       		{	case '\n': fprintf(opfd, "\\n"); break;
			                       		case '\t': fprintf(opfd, " "); break;
			                       		case '\"': fprintf(opfd, "\\\""); break;
			                       		case '\\':  fprintf(opfd, "\\\\"); break;
			                       		default:   fprintf(opfd, "%c", *s);
		                       		}
		                       break;
		               case 'd':                       
		                       d = va_arg(ap, int);
		                       fprintf(opfd, "%d", d);
		                       break;
		               case 'c':                       
		                       /* Note: char is promoted to int. */
		                       c = va_arg(ap, int);
		                       fprintf(opfd, "%c", c);
		                       break;
		                default:
		                		fprintf(stderr, " !!!(3) unknown format %%%c !!!\n", *fmt);
		                		break;
		       	}
		       	fmt++;
       		}
       		else
       			fprintf(opfd, "%c", *fmt++);       
       }
       va_end(ap);
}
