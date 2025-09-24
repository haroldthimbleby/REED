typedef struct authorlist { char *author; struct authorlist *next; } authorList;

extern int newauthor(char *author);

extern void defineArrowNote(str *u, str *v, str *theNote, str *theIs, arrow **keywordlist);

extern void defineNodeNote(arrow *nl, str *theNote, arrow **keywordlist);

extern void notes(FILE *opfd, char *title, char *version, authorList *authors, char *date, char *abstract);
extern void htmlnotes(FILE *opfd, char *title, char *version, authorList *authors, char *date, char *abstract);

extern authorList *authors;

extern void defineArrowStyle(str *u, str *v, str *theStyle);

extern char *flagdefinitions[], *flagcolors[];

extern int rowsTOCstyled;

extern str *basename(char *fullname);

extern int  numberingCount;
