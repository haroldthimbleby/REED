#include "header.h"

void *safealloc(size_t n)
{	char *p = (char *) malloc(n);
	if( p == NULL )
	{	fprintf(stderr, "** run out of memory (malloc)!\n");
		exit(0);
	}
	return p;
}

void *saferealloc(char *p, size_t n)
{	p = (char *) realloc(p, n);
	if( p == NULL )
	{	fprintf(stderr, "** run out of memory (realloc)!\n");
		exit(0);
	}
	return p;
}

char *undefinedVersion = "node version not set!";

str *newstr(char *s)
{	int len = strlen(s);
	// we could treat "" specially .... if( !len ) return s; // return "" directly
	str *new = (str*) safealloc(sizeof(str));
	new->s = safealloc(len+1);
	new->t = None;
	new->l = ID;
	new->is = new->note = new->style = new->group = new->exampleGroupMember = NULL;
	new->pointsTo = new->pointedFrom = new->isgroup = new->isstyle = new->rowDefaultNodeStyle = new->cascade = 0;
	new->flag = new->originalflag =noflag;
	new->plain = 0;
	new->rankx = new->ranky = 0;
	new->color = 0;
	new->styleName = NULL;
	new->nodeversion = undefinedVersion;
	new->noderef = "";
	strcpy(new->s, s);
	return new;
} 

str *copystr(str *s)
{	str *new = newstr(s->s);
	new->t = s->t;
	new->l = s->l;
	new->next = s->next;
	return new;
}

str *appendch(str *d, char c)
{	char ch[] = "x";
	ch[0] = c;
	d->s = saferealloc(d->s, strlen(d->s)+2);
	strlcat(d->s, ch, strlen(d->s)+2);
	return d;
}

void strpad(str **s, int d)
{
	//printf(" pad %s to %d\n", (*s)->s, d);
	while( strlen((*s)->s) < d )
		*s = appendch(*s, ' ');
}

str *appendstr(str *d, str *e)
{	d->s = saferealloc(d->s, strlen(d->s)+strlen(e->s)+2);
	strlcat(d->s, e->s, strlen(d->s)+strlen(e->s)+2);
	return d;
}

str *appendcstr(str *d, char *e) // appends in situ
{	d->s = saferealloc(d->s, strlen(d->s)+strlen(e)+2);
	strlcat(d->s, e, strlen(d->s)+strlen(e)+2);
	return d;
}

str *newappendcstr(str *d, char *e) // appends to a new string
{	str *new = newstr("");
	int leng = strlen(d->s)+strlen(e)+2;
	new->s = (char*) safealloc(leng);
	strlcpy(new->s, d->s, leng);
	strlcat(new->s, e, leng);
	return new;
}

int flagOption = 0,
	flagTextOption = 0,
	verboseOption = 0, 
    showIDsOption = 0,
    optionsOption = 0,
    graphvizOption = 0,
    transposeOption = 0,
    mathematicaOption = 0,
    showSignatures = 0,
    jsonOption = 0,
    xmlOption = 0,
    latexOption = 0,
    htmlOption = 0,
    showVersionsOption = 0,
    showRulesOption = 0;

struct structOption
{	char *option, *usage;
	int *optionFlag;
} 
	options[] =
{	{"-F", "highlight flags in drawing (unfortunately due to a Graphviz bug, this breaks up any groups)", &flagOption},
	{"-f", "show textual descriptions of flag colours in drawing", &flagTextOption},
	{"-g", "open generated REED graphics file using GraphViz (on MacOS)", &graphvizOption},
	{"-h", "generate an HTML file", &htmlOption},
	{"-j", "generate a JSON file", &jsonOption},
	{"-l", "generate a Latex file", &latexOption},
	{"-m", "generate a Mathematica file", &mathematicaOption},
	{"-n", "show node IDs in drawing", &showIDsOption},
	{"-r", "show all HTML-Latex rules", &showRulesOption},
	{"-s", "show REED file signatures", &showSignatures},
	{"-t", "transpose numbering (swap row and column node numbering)", &transposeOption},
	{"-v", "verbose mode", &verboseOption},
    {"-w", "what versions are used in these files? (Needed for using with v= flag.)", &showVersionsOption},
	{"-x", "generate an XML file", &xmlOption},
	{"--", "treat all further parameters as filenames", &optionsOption},
};

int setOption(char *argvi)
{	if( !optionsOption )
		for( int o = 0; o < sizeof options/sizeof(struct structOption); o++ )
			if( !strcmp(options[o].option, argvi) ) return *options[o].optionFlag = 1;
	return 0;
}

void usage(char *process)
{	fprintf(stderr, "** did not process any files\nUsage: %s ", process);
	fprintf(stderr, "[v=value] ");
	for( int o = 0; o < sizeof options/sizeof(struct structOption); o++ )
		fprintf(stderr, "[%s] ", options[o].option);
	fprintf(stderr, "files...\n       v=value use all versions to <value> and skip later versions in file\n");
	for( int o = 0; o < sizeof options/sizeof(struct structOption); o++ )
		fprintf(stderr, "       %s %s\n", options[o].option, options[o].usage);
    fprintf(stderr, "Note:  %s will always generate a .gv (Graphviz) file if possible\n", process);
	exit(0);
}

char *skipversion(char *name, char *value)
{	if( strcmp(name, "v") ) 
		fprintf(stderr, "** only v=value skip versions option implemented\n");
	else
		return value;
	return NULL;
}

int main(int argc, char *argv[]) 
{	int opened = 0;
	FILE *fp;
	char *bp, *openedfile, *processedFileName, *skip = NULL;
	int successfulskip = 0;
	for( int i = 1; i < argc; i++ ) 
		if( setOption(argv[i]) ) continue;
		else if( !optionsOption && (bp = index(argv[i], '=')) != NULL ) 
		{	bp[0] = (char) 0;
			skip = skipversion(newstr(argv[i])->s, &bp[1]);
		}
		else if( (fp = fopen(openedfile = argv[i], "r")) != NULL )
		{	struct stat stat_buf;
			int errno;
			if( (errno = fstat(fileno(fp), &stat_buf)) != 0 )
			{	fprintf(stderr, "Cannot stat %s: %s\n", openedfile, strerror(errno));
				exit(0);
			}
			bp = safealloc(1+stat_buf.st_size);
			if( fread(bp, 1, stat_buf.st_size, fp) != stat_buf.st_size )
			{	fprintf(stderr, "** cannot read from \"%s\" (maybe a permissions problem?)\n", openedfile);
				continue;
			}
			bp[stat_buf.st_size] = (char) 0;
			opened = 1;
			//if( verboseOption ) fprintf(stderr, ": File %s\n", processedFileName);
			//fprintf(stderr, "Parse %s starting with successfulskip=%d\n", openedfile, successfulskip);
			hash(openedfile);
			if( !parse(skip, openedfile, bp) ) // return 0 means a fatal error or matched version number to skip
			{	processedFileName = openedfile;
				successfulskip = 1;
				break;
			}
			processedFileName = openedfile;
			//fprintf(stderr, "file processed = %s\n", openedfile);
			free(bp);
			fclose(fp);
		}
		else fprintf(stderr, "** cannot open file \"%s\"\n", openedfile);
	if( showRulesOption ) explainTranslationRules();
	if( !opened ) usage(argv[0]);
	if( skip && !successfulskip )
		nolineerror("Never matched version v=%s but used version '%s' instead", skip, version);
	if( *processedFileName ) // make dot, latex, etc files after last processed file name
		generateFiles(processedFileName); // processFileName is the last name
	return 0;
}
