#include <stdio.h>
#include <strings.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/errno.h>
#include <ctype.h>

#include "mainStructs.h"

#define checkarrowlist(x) (checkMemory(__FILE__,__LINE__))
extern void checkMemory(char *file, int line);

#define startBlue fprintf(stderr, "\033[94;91m")
#define beginError fprintf(stderr, "\033[38;91m")
#define endError   fprintf(stderr, "\033[0m")

extern int nflagcolors;
extern void fatalError(char *fmt, ...);
extern int isIDchar(char ch);
extern void debug(int n);
extern void generateMetadata(FILE *opdf);
extern void showAllColors();
void *safealloc(size_t size);
void *safeCalloc(size_t count, size_t size);
extern int parse(char *filename, char *bp);
extern void listnodes();
extern int listidsOption;
extern int listissOption;
extern int listBothOption;

extern void latexxrefs(FILE *opfd);
extern void hash(char *file);
extern void printfiledata(FILE *opfd);
enum flagcolor pullString;
extern int norefs;
extern void LaTeXcolorkey(FILE *opfd, char *heading, char *vskip);
extern char *undefinedVersion;
extern str *latexdefinitions, *latexendoffile, *htmldefinitions, *introduction, *conclusion;
extern str *newstr(const char *s);
extern str *appendch(str *d, char c);
extern const int EndOfFile;
extern void error(char *fmt, ...);
extern void warning(char *fmt, ...);
extern str *copystr(str *s);
extern str *appendcstr(str *d, char *e); // appends in situ (changing d)
extern str *newappendcstr(str *d, char *e); // appends to a new string (not changing d)
extern void strpad(str **s, int d); // pad to length d with spaces
extern str *appendstr(str *d, str *e); // append a string to a string

extern int verboseOption, csvOption, graphvizOption, openOption, showIDsOption, optionsOption, transposeOption, flagOption, flagTextOption, xmlOption, generatePDFOption, showVersionsOption, componentsOption, JSONOption, colorsOption, generateSVGOption, pullPlusOption, colorsPlusOption, IDsOption, transitOption,
	mathematicaOption, showSignatures, latexOption, htmlOption, commentOption, separatorOption, rawOption, pullOption, goOption, hoOption, keywordsOption;

extern void generateFiles(char *targetVersion, char *filename);
extern int metadata(arrow *nl);
extern void newarrow(arrow **putonthisarrowlist, str **u, str **v, int forceadd);
extern node *nodeList, *stylelist;
extern void myfprintf(FILE *opfd, char *fmt, ...);
extern void nolineerror(char *fmt, ...);
extern int printrank(FILE *opfd, str *n, char *version); // for tex file

// nodes are made unique, but arrows aren't:
// they are pairs of nodes and can end up on one or more different lists depending...
extern arrow *arrowList, *styledArrowList, *noteArrowList, *metaArrowList;

extern char *flagcolor(enum flagcolor fc);
extern enum flagcolor iscolor(char *s);
extern void cascade();
extern int flagcascade[];
extern char *version;
extern char *defaultStyle;
extern char *cyclicStyle;
extern void saveCheckRtrans(str *claims, str *u, str *v);
extern void checkAllRtrans();
extern void findComponents();
extern void findCycles();
extern void explainTranslationRules();
extern int numberOfComponents;
extern tag startTag, endTag;
extern tag setTag(char *string);
extern int versionstrcmp(char *a, char *b);
extern char *lastVersion();
extern void appendVersions(char *version);
extern void allVersionsSeparator(FILE *opfd, char *separator);
extern int isVersion(char *v);
extern void checkIS();
extern int errcount;
extern void checkNumbering();
extern void listColorsUsed();
extern void HTMLtranslate(FILE *opfd, str *context, char *note); // translate Latex and HTML to HTML, and include <<id>> notation
extern void makefiles(char *targetVersion, char *filename);
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
extern void sortkeywords(struct keywordlist **keywordlist);
extern struct keywordlist *allkeywords;
extern void linkkeyword(FILE *opfd, struct keywordlist *t, char *debug);
extern void noteaddkeywordtolist(str *keyword, struct keywordlist **keywords);
extern int keywordcmp(char *keyword, char *pattern);
