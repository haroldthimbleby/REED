//  mainStructs.h
//  Created by Harold Thimbleby on 3/24/26.

#ifndef mainStructs_h
#define mainStructs_h

typedef enum {
    DIRECTION, ROWS, STAR, NUMBERING, LBRA, RBRA,
    SEMI, LARROW, RARROW, TRANSARROW, CHECK, NOREFS,
    IS, NOTE, TITLE, VERSION, AUTHOR, DATE,
    ABSTRACT, HIGHLIGHT, KEYWORDS, INTRODUCTION, CONCLUSION,
    GROUP, STYLE,  REF, // these don't require a string
    ID,// assumes the string s is initialised
    COLON, INVISIBLE, VISIBLE, DEFAULTSTYLE, CYCLICSTYLE, INFLUENCES,
    TAGS, LATEXDEFINITIONS, LATEXENDOFFILE, HTMLDEFINITIONS,
    CYCLE, NEWSTYLE, PROPERTY
    // removed: OVERRIDE,
} lexval;

typedef enum { None, String, Node, Arrow, Annotation } type;

enum flagcolor { noflag, aqua, black, blue, fuchsia, gray, green,
                 lime, maroon, navy, olive, orange, purple, red,
                 silver, teal, white, yellow };  // in alpha order

typedef struct tmpmetadata {
    struct tmpmetadata *next;
    char *property, *value;
    int count; // for reporting property repetitions
} metadataList;

typedef struct tmpstr {
    char *s; char *styleName;
    lexval l;
    type t;
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
    int nodeNumber; // for cycle algorithms, etc
    int cyclic; // is node on a cycle?
    int declaredCyclic; // if REED has said cyclic <node>?
    int keywordsOK;
    int wasString;
    int visible;
    int inDegree, outDegree;
    metadataList *metadata;
} str;

typedef struct tmpnode { int count; str *strp; struct tmpnode *next; } node;

typedef struct rownode { str *node; int label; struct rownode *right, *down; } rownodes;

typedef struct
{    char *option, *args, *usage;
    int *optionFlag;
    int needGraphViz;
} structOption;


typedef struct tmparrow { str *u, *v, *arrowStyle, *arrowis, *arrownote;
    struct keywordlist *keywords;
    metadataList *metadata;
    int force;
    struct tmparrow *next;
        // flags for connected components analysis
        int expanded, component;
        enum flagcolor flag;
        int visible;
    } arrow;


typedef struct {
    char *tagString;
    int tagLength;
} tag;

struct keywordlist {
    str *keyword;
    int xkey, ykey;
    struct keywordlist *next;
};

#endif /* mainStructs_h */
