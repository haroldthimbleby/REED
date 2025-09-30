#include "header.h"
#include "notes.h"

const int EndOfFile = -1;

extern rownodes *cols;

char *title = "", *date = "", *version = "", *abstract = "", *direction = "";

char *flagcolors[] = {"NONE", "black", "blue", "gray", "green", "red", "white", "yellow"};

str *lex1 = NULL, *lex2 = NULL, *lex3 = NULL;

char *flagcolor(enum flagcolor fc)
{	switch( fc )
	{	case noflag: return "none";
		case blue: return "blue";
		case white: return "white";
		case red: return "red";
		case black: return "black";
		case yellow: return "yellow";
        case green: return "green";
        case gray: return "gray";
        default: error("unknown flag color"); return "?flag colour error?";
	}
}

void defineArrowStyle(str *u, str *v, str *theStyle)
{	for( arrow *t = styledArrowList; t != NULL; t = t->next )
		if( t->u == u && t->v == v )
		{	error("arrow %s -> %s style being redefined", t->u->s, t->v->s);
		}

	arrow *a = (arrow*) malloc(sizeof(arrow));
	a->next = styledArrowList;
	a->arrowStyle = theStyle;
	//fprintf(stderr, "%s->%s styled as %s\n", u->s, v->s, theStyle->s); 
	a->u = u;
	a->v = v;
	styledArrowList = a;
}

int p, eof;
char *buffer;
int lineno, nextlineno, startline, nextstartline;

int rowsTOCstyled = 0;

// if rawOption is set, move p (in buffer[p) to just beyond matching startTag.tagString
// this then puts the input stream into READ mode
int ifRAWskipToStart(int p)
{   if( rawOption )
    {   if( verboseOption ) fprintf(stdout, "|--Skipping over raw text to start tag \"%s\"\n", startTag.tagString);
        // fprintf(stderr, "start=%s, |start|=%d\nbuffer=%s\nEND\n", startTag.tagString, startTag.tagLength, buffer);
        while( buffer[p] && strncmp(&buffer[p], startTag.tagString, startTag.tagLength) )
            p++;
        for( int n = startTag.tagLength; n > 0 && buffer[p]; n-- )
            p++;
        // fprintf(stderr, "after start tag, buffer=%s\nEND\n", &buffer[p]);
        // printf("We've got this left: %s\n", &buffer[p]);
        if( !buffer[p] )
            fprintf(stderr, "Warning: raw text mode never left as start tag \"%s\" not found\n", startTag.tagString);
    }
    return p;
}

int countStarTagWarnings = 0;

char getch()
{	if( eof || !buffer[p] )
	{	eof = 1;
		return EndOfFile;
	}
    // if startTag is recognised, warn
    // if endTag is recognised, skip to (just after) startTag
    // check for EOF during search
    if( countStarTagWarnings < 2 && startTag.tagLength > 0 && !strncmp(startTag.tagString, &buffer[p], startTag.tagLength) )
    {   countStarTagWarnings++;
        if( countStarTagWarnings == 1 )
            fprintf(stderr, "Warning: start tag \"%s\" found inside REED code\n", startTag.tagString);
        else
            fprintf(stderr, "Second warning: start tag \"%s\" found inside REED code\nNo further warnings will be given!\n", startTag.tagString);
    }
    if( endTag.tagLength > 0 && !strncmp(endTag.tagString, &buffer[p], endTag.tagLength) )
    {   // end of REED code, skip to startTag
        // printf(">end tag matched\n");
        if( !startTag.tagLength )
           error("There is an end tag, but the start tag has zero length!\n");
        //p += endTag.tagLength;
        while( 1 )
        {   //putchar(buffer[p]);
            if( !buffer[p] )
            {   error("Missing start tag \"%s\", and EOF encountered after end tag \"%s\"\n", startTag.tagString, endTag.tagString);
                eof = 1;
                return EndOfFile;
            }
            if( buffer[p] == '\n' )
            {   nextlineno++;
                nextstartline = p+1;
            }
            p++;
            if( !strncmp(startTag.tagString, &buffer[p], startTag.tagLength) )
                break;
        }
        p += startTag.tagLength;
        if( !buffer[p] )
        {   eof = 1;
            return EndOfFile;
        }
    }

    if( buffer[p] == '\n' )
	{	//fprintf(stderr, "\ngetch newline nextstartline:=%d\n", p);
		nextlineno++;
		nextstartline = p+1;
	}
	else 
	{	startline = nextstartline;
		lineno = nextlineno;
	}
	return buffer[p++];
}

char nextch()
{	if( eof ) 
		return EndOfFile;
	return buffer[p];
}

struct { lexval l; char *symbol; } lexes[] =
{
    { STAR, "*"},
    { LBRA, "(" },
    { RBRA, ")" },
    { SEMI, ";" },
    { LARROW, "<-" },
    { RARROW, "->" },
    { DOUBLEARROW, "<->" },
    { ID, "identifier or string" },
    { IS, "<is>" },
    { NOTE, "<note>" },
    { TITLE, "<title>" },
    { AUTHOR, "<author>" },
    { DATE, "<date>" },
    { ABSTRACT, "<abstract>" },
    { VERSION, "<version>" },
    { HIGHLIGHT, "<highlight>" },
    { GROUP, "<group>" },
    { NEW, "<new>" },
    { STYLE, "<style>" },
    { EndOfFile, "end of file" },
    { NUMBERING, "<numbering>"},
    { ROWS, "<rows>"},
    { DIRECTION, "<direction>"},
    { OVERRIDE, "<override>"},
    { TAGS, "<tags>"},
    { LATEXDEFINITIONS, "<latexdefinitions>"},
    { HTMLDEFINITIONS, "<htmldefinitions>"},
    { CHECK, "<check>"},
    { TRANSARROW, "=>"},
    { KEYWORDS, "<keywords>"}
};

str *currentlexstr;

char *lexvalue(str *v)
{	if( v->l == ID ) return v->s;
	for( int i = 0; i < sizeof(lexes)/sizeof(lexes[0]); i++ )
		if( lexes[i].l == v->l )
			return lexes[i].symbol;
	return "No known lexical value!";
}

// make same IDs use same string
str *allIDs = NULL;

str *canonicalise(str *s)
{	str *p = allIDs;
	//fprintf(stderr," got %s - ", s->s);
	while( p != NULL )
		if( !strcmp(p->s, s->s) )
		{	//fprintf(stderr, " - old\n");
			return p;
		}
		else { //fprintf(stderr, " .. ");
			p = p->nextstr;
		}
	//fprintf(stderr, " - new\n");
	s->nextstr = allIDs;
	allIDs = s;
	return s;
}

void removetrailingblanks(str *t)
{	char *cp, *lastnonblank;
	lastnonblank = cp = t->s;
	while( *cp )
	{	if( !isblank(*cp) ) lastnonblank = cp;
		cp++;
	}
	lastnonblank[1] = '\0';
}

lexval readlex(str **lexstr)
{	*lexstr = newstr("");
	(*lexstr)->lineno = lineno;
	for(;;)
	{	char ch = getch();

        while( isblank(ch) || ch == '\n' ) // skip blanks
			ch = getch();

		if( isalnum(ch) )
		{	for(;;)
			{	appendch(*lexstr, ch);
				if( !isalnum(nextch()) && nextch() != '_' ) 
				{	*lexstr = canonicalise(*lexstr);
					if( !strcmp("is", (*lexstr)->s) ) return IS;
					if( !strcmp("note", (*lexstr)->s) ) return NOTE;
					if( !strcmp("title", (*lexstr)->s) ) return TITLE;
					if( !strcmp("author", (*lexstr)->s) ) return AUTHOR;
					if( !strcmp("date", (*lexstr)->s) ) return DATE;
					if( !strcmp("abstract", (*lexstr)->s) ) return ABSTRACT;
					if( !strcmp("version", (*lexstr)->s) ) return VERSION;
					if( !strcmp("highlight", (*lexstr)->s) ) return HIGHLIGHT;
					if( !strcmp("override", (*lexstr)->s) ) return OVERRIDE;
					if( !strcmp("group", (*lexstr)->s) ) return GROUP;
					if( !strcmp("new", (*lexstr)->s) ) return NEW;
					if( !strcmp("style", (*lexstr)->s) ) return STYLE;
					if( !strcmp("numbering", (*lexstr)->s) ) return NUMBERING;
					if( !strcmp("rows", (*lexstr)->s) ) return ROWS;
					if( !strcmp("ref", (*lexstr)->s) ) return REF;
                    if( !strcmp("direction", (*lexstr)->s) ) return DIRECTION;
                    if( !strcmp("tags", (*lexstr)->s) ) return TAGS;
                    if( !strcmp("latexdefinitions", (*lexstr)->s) ) return LATEXDEFINITIONS;
                    if( !strcmp("htmldefinitions", (*lexstr)->s) ) return HTMLDEFINITIONS;
                    if( !strcmp("check", (*lexstr)->s) ) return CHECK;
                    if( !strcmp("keywords", (*lexstr)->s) ) return KEYWORDS;
                    if( !strcmp("keyword", (*lexstr)->s) ) return KEYWORDS;
                    // if( !strcmp("flag", (*lexstr)->s) ) error("Use of obsolete 'flag' - use 'highlight' instead\n");
					return ID;
				}
				ch = getch();
			} 
		}
		switch( ch )
		{
			case '(': return LBRA;
			case ')': return RBRA;
			case '*': return STAR;
			case ';': return SEMI;
			case '<':  // can start <-, <->, and <<<
				if( nextch() == '<' ) // start of <<<? (herestring)
                {	char c;
                    getch();
					if( nextch() == '<' )
					{	
						getch(); // grab the last < of <<<
						while( isblank(nextch()) )
							getch();
						str *ending = newstr("\n");
						while( (c = nextch()) != '\n' && c != EndOfFile )
							ending = appendch(ending, getch());
						removetrailingblanks(ending);
						// fprintf(stderr, "got '%s' after <<<\n", ending->s);
						if( nextch() != '\n' ) fprintf(stderr, "<<<%10s... should be followed by newline\n", ending->s);
						getch(); // consume end of line
						if( !*ending->s )
							error("<<< not followed by text that can be used to end the herestring");
						int matched = 0;
						while( nextch() != EndOfFile )
						{
							if( nextch() == ending->s[matched] )
							{	matched++;
								// fprintf(stderr, "matched %c; now check against %c\n", nextch(), ending->s[matched]);
								if( !ending->s[matched] )
								{	getch();
									return ID;
								}
							}
							else
							{	while( matched > 0 )
									appendch(*lexstr, ending->s[--matched]);
							}
							if( !matched ) appendch(*lexstr, nextch());
							getch();
						}
						error("end of file after unmatched herestring started with <<< %s\nlikely caused by missing newline+'%s'", &ending->s[1], &ending->s[1]);
                    }
					else error("<< not formed into a proper <<<");
					return SEMI;
				}
				if( nextch() != '-' ) error("< not followed by - to make either <- or <-> arrows");
				else getch();
				if( nextch() == '>' )
				{
					getch();
					return DOUBLEARROW;
				}
				return LARROW;
            case '=':
                if( nextch() != '>' )
                    error("= not followed by > to make checking arrow =>");
                else getch();
                return TRANSARROW;
                break;
			case '-':
                if( nextch() != '>' ) error("- not followed by > to make -> arrow");
                else getch();
				return RARROW;
			case '#': // comment
                if( commentOption ) fprintf(stderr, "Line %4d #", lineno);
				while( nextch() != '\n' && nextch() != EndOfFile )
                {   char c = getch();
                    if( commentOption ) fprintf(stderr, "%c", c);
                }
                if( commentOption ) fprintf(stderr, "\n");
                continue;
			case '"': // string
				{	ch = getch();
					while( ch != '"' && ch != EndOfFile )
					{	if( ch == '\\' ) 
						{	char t;
							switch( t = getch() )
							{	case EndOfFile: break; 
								case '\\': appendch(*lexstr, '\\'); break;
								case 'n': appendch(*lexstr, '\n'); break;
								case 't': appendch(*lexstr, '\t'); break;
								case '"': appendch(*lexstr, '"'); break;
								case '\n': break; // ignore newlines after \
								default: 
									error("Unknown escape code in string: \"\\%c\" converted to \"?\"", t); 
									appendch(*lexstr, '?');
									break;
							} 
						}
						else appendch(*lexstr, ch);
						ch = getch();
					}
					if( ch == EndOfFile )
						error("End of file found inside a string (perhaps caused by a missing \")");
					//fprintf(stderr, "string = '%s'\n", (*lexstr)->s);
					return ID;
				}
			case EndOfFile:
				*lexstr = newstr("<eof>");
				return EndOfFile;
		}
		error("Unrecognized or out of place character '%c' (%d) ignored", ch, ch);
	}
}

arrow *arrowList = (arrow*) NULL, *styledArrowList = (arrow*) NULL, *noteArrowList = (arrow*) NULL;

node *nodeList = (node*) NULL, *stylelist = (node*) NULL;

int rankx = 1;

void newnode(str **u)
{	if( (*u)->nodeversion == undefinedVersion ) 
		(*u)->nodeversion = version; // update version to be latest (in file order)
	for( node *t = nodeList; t != NULL; t = t->next )
		if( !strcmp(t->s->s, (*u)->s) )
    	{ //fprintf(stderr, "existing node %s -- ", u->s);
    	  //fprintf(stderr, "%s\n", t->s == u? "SAME": "DIFFERENT ADDRESSES!");
    	  *u = t->s;
    	  return;
    	}
	// fprintf(stderr, "new node %s at %ld\n", (*u)->s, (long) *u);
	node *new = malloc(sizeof(node));
	new->next = nodeList;
	new->s = *u;
	(*u)->nodeversion = version;
	(*u)->component = 0;
	nodeList = new;
}

void newarrow(arrow **putonthisarrowlist, str *u, str *v, int doublearrow, int forceadd) 
{	newnode(&u);
	newnode(&v);

	// do we already have the arrow?
	for( arrow *t = *putonthisarrowlist; t != NULL; t = t->next )
	{
		if( t->u == u && t->v == v )
		{	if( !t->force ) 
			{	t->force = forceadd;
				return;
			}
			break;
		}
	}
	arrow *new = (arrow*) malloc(sizeof(arrow));
	new->next = *putonthisarrowlist;
	new->force = forceadd;
	new->arrowis = NULL;
	new->u = u;
	new->v = v;
	new->doublearrow = doublearrow;
	*putonthisarrowlist = new;
}

void getlex()
{	lex1 = lex2;
	lex2 = lex3;
	lex3->l = readlex(&lex3);
}

void whilearrow(arrow **whicharrowlist, int forceadd)
{	while( lex2->l == LARROW || lex2->l == RARROW || lex2->l == DOUBLEARROW )
	{	if( lex1->l == HIGHLIGHT || lex3->l == HIGHLIGHT ) { error("highlight cannot be at end of an arrow"); getlex(); break; } 
		if( lex3->l != ID ) { error("Expected ID after arrow"); getlex(); break; }
		// myfprintf(stderr, "in whilearrow() read arrow: %s -> %s\n", lex1->s, lex3->s);
		if( lex2->l == LARROW ) newarrow(whicharrowlist, lex3, lex1, 0, forceadd);
		else if( lex2->l == RARROW ) newarrow(whicharrowlist, lex1, lex3, 0, forceadd);
		else // double arrow
		{	
			newarrow(whicharrowlist, lex1, lex3, 1, forceadd);
		}
		getlex();
		getlex();
	}
}

int numberingRow(int flat, int row) //kk
{	int col = flat? flat: 1;

	while( lex1->l == ID || lex1->l == STAR )
	{	if( lex1->l == ID )
		{	newnode(&lex1);
			if( lex1->rankx || lex1->ranky )
				error("numbering node %s in more than one position", lex1->s);
			if( transposeOption )
			{	lex1->rankx = col;
				lex1->ranky = row;
			}
			else
			{	lex1->ranky = col;
				lex1->rankx = row;
			}
		}
		col++;	
		//fprintf(stderr, "%d.%d ", lex1->rankx, lex1->ranky);
		getlex();
	}
	//fprintf(stderr, "\n");
	if( lex1->l != RBRA )
		error("a row should end with a closing bracket");
	return flat? col: 0;
}

int overrideCounter = 0;

int dontOverride() 
{	return overrideCounter != 1;
}

int  numberingCount = 0;
void numbering()
{	int row = 1, flat = 0;
	if( dontOverride() )
	{	if( numberingCount > 0 )
			error("There should be only one node numbering (unless using 'override')");
	}
	else
	{	// reset numbering --- yes because we need to see if individual nodes are renumbered
		numberingCount = 0;
		for( node *t = nodeList; t != NULL; t = t->next )
			t->s->rankx = t->s->ranky = 0;
	}
	numberingCount++;
	if( lex1->l == ID && !strcmp("flat", lex1->s) )
	{	flat = 1;
		getlex();
	}
	if( lex1->l != LBRA )
		error("bracketed lists of nodes expected after 'numbering'");
	getlex();
	if( !flat && (lex1->l == ID || lex1->l == STAR) ) // a single row
	{
		numberingRow(flat, 0); // ranky will be zero, so later we will convert x.0 into x only
		return;
	}

	// multiple rows
	while( lex1->l == LBRA )
	{	getlex();
		flat = numberingRow(flat, flat? 0: row++);
		getlex();
	}
	if( lex1->l != RBRA )
		error("closing bracket missing from numbering");
}

void checkNumbering()
{   // have we set reference numbers for all nodes?
    int notset = 0;
    for( node *t = nodeList; t != NULL; t = t->next )
    {   if( !t->s->rankx || !t->s->ranky )
        {   if( t->s->noderef && *t->s->noderef ) continue;
            if( !notset )
                fprintf(stderr, "Warning: Some nodes have not had their reference or x.y numbering set:\n   ");
            fprintf(stderr, "%s ", t->s->s);
            notset = 1;
        }
    }
    if( notset )
        fprintf(stderr, "\n   (use numbering or ref if you want to define references)\n");
}

// typedef struct rownode { str *node; int label; struct rownode *right, *down;} rownodes;
// rownodes *cols = NULL;

int rowsCount = 0;

void rows()
{	if( rowsCount++ > 0 )
		error("There should be only one rows layout");
	if( lex1->l == ID && !strcmp("TOC", lex1->s) )
	{	rowsTOCstyled = 1;
		getlex();
	}
	if( lex1->l != LBRA )
		error("Bracketed lists of nodes expected after 'rows'");
	getlex();
	rownodes **endofrow = NULL, **endofcols = &cols;
	while( lex1->l == LBRA || lex1->l == ID )
	{	int firstonrow = 1;
		if( lex1->l == ID ) // row starts with an ID label
		{	rownodes *n = (rownodes*) malloc(sizeof(rownodes));
			firstonrow = 0;
			*endofcols = n;
			n->down = NULL;
			n->right = NULL;
			n->label = 1;
			newnode(&lex1);
			n->node = lex1;
			endofcols = &n->down;
			endofrow = &n->right;
			getlex();
			if( lex1->l != LBRA )
				error("row name should be followed by a (...) row");
		}
		getlex(); 
		while( lex1->l == ID || lex1->l == STAR )
		{	rownodes *n = (rownodes*) malloc(sizeof(rownodes));
			n->label = 0;
			if( firstonrow )
			{	firstonrow = 0;
				*endofcols = n;
				n->down = NULL;
				n->right = NULL;
				endofcols = &n->down;
				endofrow = &n->right;

			} else
			{
				(*endofrow) = n;
				n->down = n->right = NULL;
				endofrow = &n->right;
			}
			if( lex1->l == ID )
			{	newnode(&lex1);
				n->node = lex1;
			}
			getlex();
		}
		//fprintf(stderr, "\n");
		if( lex1->l != RBRA )
			error("a row should end with a closing bracket");
		getlex();
	}
	if( lex1->l != RBRA )
		error("closing bracket missing from rows");
	
	//printf("ROWS - \n");
	//for( rownodes *row = cols; row != NULL; row = row->down )
	//{	for( rownodes *col = row; col != NULL; col = col->right )
	//		printf(" %s ", col->node->s);
	//	printf("\n");
	//}
	//printf(" - ROWS\n");
}

arrow *parsenodelist(int debug, int makenodes, int makearrows)
{	//fprintf(stderr, "parsenodelist()\n");
	if( lex1->l == ID ) // we have a single node or arrow
	{	arrow *t = NULL;
		if( lex2->l == LARROW || lex2->l == RARROW || lex2->l == DOUBLEARROW )
		{	// single row of arrows
			//myfprintf(stderr, "in parsenodelist() read arrow: %s -> %s\n", lex1->s, lex3->s);
			whilearrow(&t, makearrows);
			getlex();
			//printf("lex1 should be is ");
			//printf("> %s :: %s :: %s\n", lexvalue(lex1), lexvalue(lex2), lexvalue(lex3));
		}
		else
		{	// single node
            if( debug )
                fprintf(stderr, "single node... %s\n", lex1->s);
			if( makenodes && lex1->l != HIGHLIGHT )
                newnode(&lex1);
			t = (arrow*) malloc(sizeof(arrow));
			t->u = lex1;
			t->v = NULL;
			t->next = NULL;
			getlex();
			//fprintf(stderr, "single node %s at %ld\n", t->s->s, (long) t->s);
		}
		return t;
	}
	if( lex1->l == LBRA ) // we are expecting a list of nodes and/or arrows
	{	getlex();
		//fprintf(stderr, "list of nodes...\n");
		arrow *t = NULL;
		while( lex1->l == ID || lex1->l == HIGHLIGHT )
		{	if( lex2->l == LARROW || lex2->l == RARROW || lex2->l == DOUBLEARROW )
			{	// single row of arrows in the list
				// myfprintf(stderr, "in parsenodelist() read arrow in list: %s -> %s\n", lex1->s, lex3->s);
				whilearrow(&t, makearrows);
				getlex();
				//printf("lex1 should be is ");
				//printf("> %s :: %s :: %s\n", lexvalue(lex1), lexvalue(lex2), lexvalue(lex3));
			}
			else
			{	// single node in the list
				arrow *u = (arrow*) malloc(sizeof(arrow));
				u->next = t;
				if( makenodes ) newnode(&lex1);
				u->u = lex1;
				u->v = NULL;
				//fprintf(stderr, "got %s\n", u->s->s);
				t = u;
				getlex();
			}
		}
		if( lex1->l != RBRA ) error("node list should end with ')'");
		else getlex();
		return t;
	}
	return NULL;
}

str *basename(char *fullname)
{
	str *bn = newstr(fullname);
//
	char *s = fullname;
//	while( *s )
//	{	if( *s == '/' )
//			bn = newstr("");
//		else
//			bn = appendch(bn, *s);
//		s++;
//	}

	// if there is a last dot (and suffix), strip it off
	char *lastdot = (char*) NULL;
	s = bn->s;
	while( *s )
	{	if( *s == '.' )
			lastdot = s;
		s++;
	}
	if( lastdot )
		*lastdot = (char) 0;
		
	if( !*bn->s )
		error("No basename for the file to process?");

	return bn;
}

enum flagcolor iscolor(char *s)
{	if( !strcmp(s, "blue") ) return blue; 
	if( !strcmp(s, "white") ) return white; 
	if( !strcmp(s, "red") ) return red; 
	if( !strcmp(s, "black") ) return black; 
	if( !strcmp(s, "yellow") ) return yellow; 
	if( !strcmp(s, "green") ) return green;
    if( !strcmp(s, "gray") ) return gray;
	return noflag;
}

char *flagdefinitions[] = {"","","","","","","",""};
int flagsused[] = {0,0,0,0,0,0,0,0};
int flagsusedaftercascades[] = {0,0,0,0,0,0,0,0};
int flagcascade[] = {0,0,0,0,0,0,0,0};

void defineflag(char *color, char *meaning, int cascade)
{	if( iscolor(color) == noflag )
	{	error("Attempt to define a highlight colour that isn't a known colour!");
		return;
	}
	if( *flagdefinitions[(int) iscolor(color)] && strcmp(flagdefinitions[(int) iscolor(color)], meaning) )
	{	error("Trying to redefine %s highlight meaning with inconsistent definitions:\n- %s\n- %s", color,
			flagdefinitions[(int) iscolor(color)], meaning);
		return;
	} else
	if( *flagdefinitions[(int) iscolor(color)] )
	{	fprintf(stderr, "Warning: Trying to redefine meaning for %s highlight again, but definition hasn't changed from\n- %s\n", color, meaning);
		// return; ignore warning
	}
	else 
		if( verboseOption ) myfprintf(stderr, "|    %s highlight defined to mean:\n|      %i\n", color, meaning);

	// fprintf(stderr, "Define %s color #%d highlight\n", color, (int) iscolor(color));
	flagdefinitions[(int) iscolor(color)] = meaning;
	flagcascade[(int) iscolor(color)] |= cascade;
}

void summarizeMissingFlagDefinitions()
{
	for( int i = 1; i <= 6; i++ )
	{	if( iscolor(flagcolors[i]) != i )
			error("!!! Internal error !!! flacolors[%d]=%s but iscolor(%s)=%d\n", i, flagcolors[i], flagcolors[i], iscolor(flagcolors[i]));
	
		//fprintf(stderr, "%d highlight is colored %s. Used=%d. Defined=%s.\n", i, flagcolors[i], flagsused[i], flagdefinitions[i]);
		if( flagsused[i] && !*flagdefinitions[i] )
			error("%s highlight meaning not defined. Say: highlight %s is \"...\"", flagcolors[i], flagcolors[i]);
	}
}

int checkOverride(char *e)
{	if( overrideCounter >= 1 )
	{	error("You cannot override %s (unless you use the override command)", e);
		return 1;
	}
	return 0;
}

int skipnexttime = 0; // must be preserved between files

int parse(char *skip, char *filename, char *bp)
{	if( separatorOption ) fprintf(stderr, "***************************************\n");
    if( verboseOption ) fprintf(stdout, "|--Reading file %s\n", filename);
	str *base = basename(filename);
	//fprintf(stderr, "Basename is %s\n\n", base->s);
	//fprintf(stderr, "Example pdf %s\n", newappendcstr(base, ".pdf")->s);
	//fprintf(stderr, "Example tex %s\n", newappendcstr(base, ".tex")->s);

	 p = 0;
	 eof = 0;
	 buffer = bp;
	 nextlineno = lineno = 1;
     p = ifRAWskipToStart(p);
	 nextstartline = startline = p;
	 lex1 = newstr("");
	 lex2 = newstr("");
	 lex3 = newstr("");
     latexdefinitions = newstr("");
     htmldefinitions = newstr("");

	 for( int i = 0; i < 3; i++ )
        getlex();

	if( 0 ) // check lexing
	{	while( lex1->l != EndOfFile )
	 	{	// printf(":: %s :: %s :: %s\n", lexvalue(lex1), lexvalue(lex2), lexvalue(lex3));
			getlex();
		}
		exit(0);
	}

	int makenewstyle = 0;
	
	struct keywordlist *keywordlist = NULL;
    arrow *nl = NULL;

	while( lex1->l != EndOfFile )
    {	if( 0 ) printf(":: %s :: %s :: %s\n", lexvalue(lex1), lexvalue(lex2), lexvalue(lex3));
        if( overrideCounter > 0 ) overrideCounter--;
        switch( lex1->l )
        {	case OVERRIDE:
                if( checkOverride("override") ) break;
                overrideCounter = 2; // so overrideCounter should be =1 next time the switch is called
                break;

            case TITLE: case AUTHOR: case DATE: case VERSION: case ABSTRACT: case DIRECTION:
                if( lex2->l != ID )
                    fprintf(stderr, "%s should be followed by a string but is followed by reserved word %s, which will be treated as a string\n", lex1->s, lexvalue(lex2));
                if( lex1->l == TITLE )
                {	if( dontOverride() && *title ) error("Multiple titles");
                    title = lex2->s;
                    if( verboseOption ) fprintf(stderr, "|    Title '%s'\n", title);
                }
                if( lex1->l == AUTHOR )
                {	if( !newauthor(lex2->s) )
                    error("Repeated author name in document author listings");
                }
                if( lex1->l == DATE )
                {	if( dontOverride() && *date ) error("Multiple dates");
                    date = lex2->s;
                }
                if( lex1->l == VERSION )
                {	//fprintf(stderr, "A skipnexttime=%d, skip=%s, version=%s\n", skipnexttime, skip, version);
                    if( dontOverride() && *version ) { error("Multiple versions need 'override' to be allowed"); break; }
                    if( verboseOption ) fprintf(stderr, "|    File %s defines version '%s'\n", filename, lex2->s);
                    if( showVersionsOption || verboseOption )
                    {   if( verboseOption ) fprintf(stderr, "| ** ");
                        fprintf(stderr, "Defines version '%s'\n", lex2->s);
                    }
                    if( skip != NULL ) // trace version setting
                    {	if( skipnexttime )
                    {	if( verboseOption ) fprintf(stderr, "   Skipping version '%s' and after\n", lex2->s);
                        return 0;
                    }
                        if( !strcmp(lex2->s, skip) ) // then return 0 next time
                        {	skipnexttime = 1;
                            if( verboseOption ) fprintf(stderr, "   Using version '%s'\n", lex2->s);
                        }
                        if( verboseOption ) fprintf(stderr, "   Using version '%s'\n", lex2->s);
                    }
                    appendVersions(version = lex2->s);
                    //fprintf(stderr, "B skipnexttime=%d, skip=%s, version=%s\n", skipnexttime, skip, version);
                }
                if( lex1->l == ABSTRACT )
                {	if( dontOverride() && *abstract ) error("Multiple abstracts");
                    abstract = lex2->s;
                }
                if( lex1->l == DIRECTION )
                {	if( dontOverride() && *direction )
                {	fprintf(stderr, strcmp(direction, lex2->s)?
                            "Warning: Too many drawing directions, %s, %s etc. (%s will be used)":
                            "Warning: Repeated drawing direction, %s", direction, lex2->s, lex2->s);
                    fprintf(stderr, "\n");
                }
                    char *valid[] = {"BT", "TB", "LR", "RL", (char*) 0};
                    int ok = 0;
                    direction = lex2->s;
                    for( int t = 0; valid[t]; t++ )
                        if( !strcasecmp(direction, valid[t]) )
                            ok = 1;
                    if( !ok )
                        error("invalid direction '%s'; possible options are BT, TB, LR, RL (in upper or lower case)", direction);
                }
                getlex();
                break;

            case LATEXDEFINITIONS:
                if( lex2->l != ID )
                    error("latexdefinitions should be followed by a string of Latex");
                else {
                    appendcstr(latexdefinitions, "\n");
                    appendstr(latexdefinitions, lex2);
                    getlex();
                }
                break;

            case HTMLDEFINITIONS:
                if( lex2->l != ID )
                    error("htmldefinitions should be followed by a string of HTML");
                else {
                    appendcstr(htmldefinitions, "\n");
                    appendstr(htmldefinitions, lex2);
                    getlex();
                }
                break;

            case TAGS: // expect two strings
                if( checkOverride("tags") ) break;
                getlex();
                //fprintf(stderr, "tag followed by two strings, %s %s\n", lex1->s, lex2->s);
                if( lex1->l != ID && lex2->l != ID )
                    error("Tag must be followed by two strings, not %s %s", lex1->s, lex2->s);
                startTag = setTag(lex1->s);
                endTag = setTag(lex2->s);
                if( verboseOption )
                    fprintf(stderr, "|--Tags set to:\n|--   %s\n|--   %s\n", startTag.tagString, endTag.tagString);
                getlex();
                break;

            case REF:
                if( checkOverride("ref") ) break;
                getlex();
                str *thenodetoref = lex1;
                if( thenodetoref->l != ID ) { error("Expected a node after 'ref'"); getlex(); break; }
                if( lex2->l != IS ) { error("Expected node then 'is' after 'ref %s'", thenodetoref->s); getlex(); break; }
                if( lex3->l != ID ) { error("Expected a reference after 'ref %s is'", thenodetoref->s); getlex(); break; }
                //fprintf(stderr, "!! setting ref for node %s to be '%s'\n", thenodetoref->s, lex3->s);
                thenodetoref->noderef = lex3->s;
                getlex();
                getlex();
                break;

            case HIGHLIGHT:  // highlight node [cascade] [is <color>]
                // highlight <color> [cascade] is <description> .... oops this means node names can't be colors :-(
                if( checkOverride("highlight") ) break;
                getlex();
                str *thenodetoflag = lex1;
                int cascadeflag = 0;
                if( !strcmp(lex2->s, "cascade") )
                {	cascadeflag = 1;
                    getlex();
                }
                if( iscolor(thenodetoflag->s) != noflag )
                {
                    if( lex2->l != IS ) error("Expected 'is' after highlight <color> ...");
                    else if( lex3->l != ID ) error("Expected string after highlight <color> is ...");
                    defineflag(thenodetoflag->s, lex3->s, cascadeflag);
                    getlex();
                    getlex();
                    break;
                }

                if( thenodetoflag->l != ID ) { error("Expected node after 'highlight'"); getlex(); break; }
                newnode(&thenodetoflag);
                enum flagcolor previousFlag = thenodetoflag->flag;
                thenodetoflag->flag = red; // default flag color if none is specified
                enum flagcolor fc;
                thenodetoflag->cascade = cascadeflag;
                if( lex2->l == IS ) // highlight <node> is form ; now read the color
                {	if( lex3->l != ID ) error("Expected a color for highlight <node> is <color>");
                    if( (fc = iscolor(lex3->s)) != noflag )
                    {	thenodetoflag->flag = fc;
                        flagsused[thenodetoflag->flag]++;
                    }
                    else
                        error("Expected blue, white, red, black, yellow, or green highlight colour, but got '%s' instead", lex3->s);
                    getlex();
                    getlex();
                }
                else
                    flagsused[thenodetoflag->flag]++;
                if( previousFlag != noflag )
                {	if( fc != previousFlag )
                    error("Highlighting %s to %s when it was previously highlighted %s",
                          thenodetoflag->s, flagcolor(fc), flagcolor(previousFlag));
                else
                    error("Highlighting %s to %s again",
                          thenodetoflag->s, flagcolor(fc));
                }
                break;

            case GROUP:
                if( checkOverride("group") ) break;
                getlex();
                //fprintf(stderr, "got a group ...\n");
                nl = parsenodelist(0, 1, 0);
                if( nl == NULL ) error("Expected a group list after 'group'");
                if( lex1->l != IS ) error("Expected 'is' after group");
                getlex();
                if( lex1->l != ID ) error("Expected a group name");
                newnode(&lex1);
                lex1->isgroup = 1;
                while( nl != NULL )
                {	//fprintf(stderr, "fixing group for %s at %ld to be %s \n", nl->s->s, (long) nl->s, lex1->s);
                    if( nl->v != NULL )
                        error("putting an arrow u->v in a group should be done by putting both nodes u and v separately in the group");
                    if( nl->u->group != NULL )
                    {	if( nl->u->group == lex1 )
                        error("%s is being put in group %s again", nl->u->s, nl->u->group->s);
                    else
                        error("%s in group %s is being changed to group %s", nl->u->s, nl->u->group->s, lex1->s);
                    }
                    nl->u->group = lex1;
                    nl = nl->next;
                }
                break;

            case KEYWORDS:
                getlex();
                keywordlist = NULL;
                while( lex1->l == ID || lex1->l == KEYWORDS )
                {   if( lex1->l == ID )
                    {   noteaddkeywordtolist(lex1, &keywordlist); // add to this node's keywords
                        addkeyword(lex1); // add to global keyword list
                    }
                    getlex();
                }
                if( lex1->l != NOTE )
                    error("keywords must be followed by a note");
                if( keywordlist == NULL ) error("Expected at least one keyword or keyword list after 'keywords'");
                sortkeywords(&keywordlist);

            case NOTE:  // EITHER note [author string [;]] idlist string
                // OR     note idlist [author string [;]] is string string
                if( checkOverride("note") ) break;
                getlex();
                nl = parsenodelist(0, 1, 1);
                if( nl == NULL ) error("Expected a node or arrow after 'note'");
                int isnode = nl->v == NULL;
                char *sort = isnode? "node": "arrow";
                if( nl->next != NULL ) error("Expected at least one node or arrow after 'note'");

                //printf(" * got note for %s", nl->u->s);
                //if( !isnode ) printf("->%s", nl->v->s);
                //printf(" (%s) --- lex1 = %s; lex2 = %s\n", sort, lexvalue(lex1), lexvalue(lex2));

                if( lex1->l == IS ) // expect: title note
                {    getlex();
                    if( lex1->l != ID ) { error("Expected %s name after 'is'", sort); getlex(); break; }
                    if( nl->u->is ) error("Multiple 'is' names for %s %s --- previously %s", sort, nl->u->s, nl->u->is->s);
                    //else if( isnode ) fprintf(stderr, "... %s is %s\n", nl->u->s, lex1->s);
                    //else fprintf(stderr, "... %s->%s is %s\n", nl->u->s, nl->v->s, lex1->s);
                    if( lex2->l != ID ) error("'note ... is ...' should be followed a note");
                    if( isnode )
                    {    nl->u->is = lex1;
                        if( nl->u->note != NULL ) error("Defining another note for %s", nl->u->s);
                        //nl->u->note = lex2;
                        defineNodeNote(nl, lex2, &keywordlist);
                    }
                    else
                    {    //printf("assigning %s\n", lex2->s);
                        defineArrowNote(nl->u, nl->v, lex2, lex1, &keywordlist);
                    }
                    getlex();
                    if( lex1->l == SEMI )
                        getlex();
                    break;
                }

                char *noteauthor = (char*) NULL;
                if( lex1->l == AUTHOR )
                {    if( lex2->l != ID )
                    error("%s must be followed by a string", lex1->s);
                    if( newauthor(lex2->s) ) // try author in main document author list
                        error("Author of note, '%s', is not mentioned as a document author", lex2->s);
                    noteauthor = lex2->s;
                    getlex();
                    getlex();
                    if( lex1->l == SEMI )
                        getlex();
                }

                if( lex1->l != ID ) error("Expected note text but got %s", lex1->s);
                else if( isnode )
                   defineNodeNote(nl, lex1, &keywordlist);
                else // void defineArrowNote(str *u, str *v, str *theNote, str *theIs)
                    defineArrowNote(nl->u, nl->v, lex1, NULL, &keywordlist);
                break;

			case NEW:
				getlex();
				makenewstyle = 1;
				// fall through
			case STYLE:
				if( checkOverride("style") ) break;
				getlex();
				//fprintf(stderr, "got a style ...\n");
				nl = parsenodelist(0, !makenewstyle, 0);
				if( nl == NULL ) error("Expected a list after 'style'");
				if( lex1->l != IS ) error("Expected 'is' after style");
				getlex();
				if( lex1->l != ID ) error("Expected a style name or a string");
				lex1->isstyle = makenewstyle;
				while( nl != NULL )
				{	// fprintf(stderr, "new style for %s is %s \n", nl->s->s, lex1->s);
					if( nl->v != NULL ) // an arrow
					{
						if( makenewstyle )
							error("arrows cannot be made into new styles");
						else
						{	// bug: new->doublearrow is ignored for style setting !!
							// is it already defined?
							defineArrowStyle(nl->u, nl->v, lex1);
						}
					}
					else // a node
					{	if( nl->u->style != NULL ) 
						{	if( nl->u->style == lex1 )
								myfprintf(stderr, "Warning: %s is being put in style %s again", nl->u->s, nl->u->style->s);
							else
								error("%s in style %s changed to style %s\n", nl->u->s, nl->u->style->s, lex1->s);
						}
						nl->u->style = lex1;
						nl->u->styleName = lex1->s;
						if( makenewstyle )
						{	if( nl->v != NULL ) error("arrows cannot be style names");
							node *new = malloc(sizeof(node));
							new->next = stylelist;
							new->s = lex1;
							lex1->style = nl->u;
							stylelist = new;
						}
					}	
					nl = nl->next;
				}
				makenewstyle = 0;
				break;

            case CHECK: // check id => id
                // fprintf(stderr, "got check...\n");
                getlex();
                if( lex1->l != ID || lex2->l != TRANSARROW || lex3->l != ID ) error("check should be followed by node => node");
                else
                    saveCheckRtrans(NULL, lex1, lex3);
                getlex();
                getlex();
                break;

			case ID:
				if( checkOverride("nodes or arrows") ) break;
				switch( lex2->l )
				{ 	case IS: // id1 is id3
						newnode(&lex1);
                        if( lex3->l != ID )
                        {   error("Expected string or identifier afer 'is'");
                            getlex();
                            break;
                        }
						if( lex1->is ) error("Mutliple 'is' names for same node %s", lex1->s);
						lex1->is = lex3;
						getlex();
						getlex();
						break;
					case LARROW: case RARROW: case DOUBLEARROW:
						whilearrow(&arrowList, 1);
						break;
                    default: // an isolated node
						fprintf(stderr, "Warning: isolated node with no arrow or 'is': %s\n", lex1->s);
						newnode(&lex1);
						break;
				}
				break;

			case NUMBERING:
				getlex();
				numbering();
				break;

			case ROWS:
				getlex();
				rows();
				break;

			case SEMI:
               // getlex();
				// just ignore it (it should only occur between statements)
				break;

			default: error("Unexpected %s [e1]", lexvalue(lex1)); 
				break;
		}
		getlex();
	}
	return !skipnexttime;
}

