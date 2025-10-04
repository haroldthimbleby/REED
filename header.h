#include <stdio.h>
#include <strings.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/errno.h>
#include <ctype.h>

void *safealloc(size_t size);

extern int parse(char *skip, char *filename, char *bp);

extern void latexxrefs(FILE *opfd);

extern void hash(char *file);
extern void printfiledata(FILE *opfd);

extern void appendVersions(char *version);

typedef enum { None, String, Node, Arrow, Annotation } type;

enum flagcolor { noflag, black, blue, gray, green, red, white, yellow };  // in alpha order

enum flagcolor pullString;

typedef enum {
            DIRECTION, ROWS, STAR, NUMBERING, LBRA, RBRA, SEMI, LARROW, RARROW, TRANSARROW, CHECK,
			DOUBLEARROW, IS, NOTE, TITLE, VERSION, AUTHOR, DATE, ABSTRACT, HIGHLIGHT, KEYWORDS,
			GROUP, STYLE, NEW, OVERRIDE, REF, // these don't require a string
			ID,// assumes the string s is initialised
            TAGS, LATEXDEFINITIONS, HTMLDEFINITIONS
		 } lexval;

typedef struct tmpstr { 
	char *s; char *styleName; lexval l; type t; 
	struct tmpstr *is, *style, *note;
	int number, rankx, ranky, pointsTo, pointedFrom, rowDefaultNodeStyle, cascade;
	struct tmpstr *next, *nextstr, *group, *exampleGroupMember; 
	int isgroup, isstyle, plain;
	int color;
	int lineno;
	int component;
	enum flagcolor flag, originalflag;
	char *nodeversion, *noderef;
    struct keywordlist *keywords;
    int keywordsOK;
} str;

extern void LaTeXcolorkey(FILE *opfd, char *heading, char *vskip);

typedef struct rownode { str *node; int label; struct rownode *right, *down;} rownodes;

extern char *undefinedVersion;
extern str *latexdefinitions, *htmldefinitions;
extern str *newstr(char *s);

extern str *appendch(str *d, char c);

extern const int EndOfFile;

extern void error(char *fmt, ...);

extern str *copystr(str *s);
extern str *appendcstr(str *d, char *e); // appends in situ (changing d)
extern str *newappendcstr(str *d, char *e); // appends to a new string (not changing d)
extern void strpad(str **s, int d); // pad to length d with spaces
extern str *appendstr(str *d, str *e); // append a string to a string

typedef struct
{    char *option, *usage;
    int *optionFlag;
    int needGraphViz;
} structOption;
extern int verboseOption, graphvizOption, openOption, showIDsOption, optionsOption, transposeOption, flagOption, flagTextOption, xmlOption, generatePDFOption, showVersionsOption, componentsOption, JSONOption, colorsOption, generateSVGOption, pullPlusOption, colorsPlusOption, IDsOption,
	mathematicaOption, showSignatures, latexOption, htmlOption, commentOption, separatorOption, rawOption, pullOption, goOption, hoOption, keywordsOption;

extern void generateFiles(char *filename);

typedef struct tmparrow { str *u, *v, *arrowStyle, *arrowis, *arrownote; struct keywordlist *keywords; int force; int doublearrow; struct tmparrow *next;
		// flags for connected components analysis
		int expanded, component;
		enum flagcolor flag;
	} arrow;
	
extern void newarrow(arrow **putonthisarrowlist, str *u, str *v, int doublearrow, int forceadd);

typedef struct tmpnode { str *s; struct tmpnode *next; } node;

extern node *nodeList, *stylelist;

extern void myfprintf(FILE *opfd, char *fmt, ...);

extern void nolineerror(char *fmt, ...);

extern int printrank(FILE *opfd, str *n, char *version); // for tex file

extern arrow *arrowList, *styledArrowList, *noteArrowList;

extern char *flagcolor(enum flagcolor fc);

extern enum flagcolor iscolor(char *s);

extern void cascade();

extern int flagcascade[];

extern char *version;

extern void saveCheckRtrans(str *claims, str *u, str *v);
extern void checkAllRtrans();
extern void findComponents();
extern void explainTranslationRules();

extern int numberOfComponents;

typedef struct {
    char *tagString;
    int tagLength;
} tag;

extern tag startTag, endTag;
extern tag setTag(char *string);

extern void checkIS();
extern int errcount;
extern void checkNumbering();
extern void listColorsUsed();
extern void HTMLtranslate(FILE *opfd, str *context, char *note); // translate Latex and HTML to HTML, and include <<id>> notation
extern void makefiles(char *filename);
extern void stopiferror();
extern void addkeyword(str *keyword);
extern void summarisekeywords(FILE *opfd);
extern void HTMLkeywords(FILE *opfd, struct keywordlist *keywords);
extern void summariseLaTeXkeywords(FILE *opfd);
extern int isakeyword(char *keyword);
extern void pullkeywords(char *keyword);
extern int pullnode(str *n);
extern int ispullingkeywords();
extern void htmlsaypullingkeyword(FILE *opfd);
extern void href(FILE *opfd, int html, str *here, char *id, char *close);
extern int checkOneRtrans(str *u, str *v);

struct keywordlist {
    str *keyword;
    int xkey, ykey;
    struct keywordlist *next;
};

extern void sortkeywords(struct keywordlist **keywordlist);
extern struct keywordlist *allkeywords;
extern void linkkeyword(FILE *opfd, struct keywordlist *t, char *debug);
extern void noteaddkeywordtolist(str *keyword, struct keywordlist **keywords);

