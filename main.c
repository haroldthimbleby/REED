#include "header.h"
#include "notes.h"

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
	new->component = 0;
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
    commentOption = 0,
    mathematicaOption = 0,
    showSignatures = 0,
    xmlOption = 0,
    latexOption = 0,
    htmlOption = 0,
    showVersionsOption = 0,
    showRulesOption = 0,
    componentsOption = 0,
    generatePDFOption = 0,
    syntaxOption = 0;

char *syntaxSummary =
    #include "SyntaxCode.c"
;

int handleTags = 0, handleInsert = 0, handleWatch = 0, separatorOption = 0;
tag startTag = {"", 0}, endTag = {"", 0};

tag setTag(char *str)
{   tag tmp = {str, strlen(str)};
    return tmp;
}

struct structOption
{	char *option, *usage;
	int *optionFlag;
} 
	options[] =
{	{"-#", "show comments, if any", &commentOption},
    {"-c", "show weakly connected components", &componentsOption},
	{"-F", "highlight flags in drawing*- unfortunately due to a Graphviz bug, this breaks up any groups", &flagOption},
	{"-f", "show textual descriptions of flag colours in REED drawing", &flagTextOption},
	{"-g", "open generated REED graphics GraphViz file automatically *- using dot on MacOS", &graphvizOption},
	{"-h", "generate an interactive HTML REED document*- will refer to a PDF of the REED (generate using -p flag)", &htmlOption},
    {"-insert", "<text> insert this text to process before next file", &handleInsert},
	{"-l", "generate a Latex REED document*- also generates some useful definition files*- will refer to a PDF of the REED", &latexOption},
    {"-m", "generate a Mathematica notebook*- representing the REED graph as a series of expressions", &mathematicaOption},
    {"-p", "generate a PDF file*- representing the REED graph", &generatePDFOption},
    {"-n", "show node IDs in graph drawing", &showIDsOption},
	{"-r", "show all HTML <-> Latex rules", &showRulesOption},
	{"-s", "show REED file signatures", &showSignatures},
    {"-sep", "draw a separator line before any output (useful with -watch)", &separatorOption},
    {"-syntax", "summarise REED syntax", &syntaxOption},
	{"-t", "transpose node numbering*- swap row and column node numbering", &transposeOption},
    {"-tags", "<start> <end> only process REED information written between these tags*- you can change tags between files*- and also set tags within a REED file by: tags \"start\" \"end\"", &handleTags},
    {"-v", "verbose mode", &verboseOption},
    {"-w", "what versions are used in these files?*- helpful to know if using the v= flag", &showVersionsOption},
    {"-watch", "run reed when any file changes (nice with -g)", &handleWatch},
	{"-x", "generate an XML file*- representing all REED data for import into other applications", &xmlOption},
	{"--", "treat all further parameters as filenames*- if you want to have no restrictions on filenames (they otherwise cannot be flags)", &optionsOption},
};

int setOption(char *argvi)
{	if( !optionsOption )
		for( int o = 0; o < sizeof options/sizeof(struct structOption); o++ )
			if( !strcmp(options[o].option, argvi) ) return *options[o].optionFlag = 1;
    if( !optionsOption && argvi[0] == '-' )
        nolineerror("Flag %s not recognised", argvi);
	return 0;
}

void usage(char *process)
{	fprintf(stderr, "%s [v=value] ", process);
	for( int o = 0; o < sizeof options/sizeof(struct structOption); o++ )
    {	fprintf(stderr, "[%s", options[o].option);
        if( !strcmp(options[o].option, "-tags") )
            fprintf(stderr, " <start> <end>");
        if( !strcmp(options[o].option, "-insert") )
            fprintf(stderr, " <text>");
        fprintf(stderr, "] ");
    }
    fprintf(stderr, "files...\n");
	for( int o = 0; o < sizeof options/sizeof(struct structOption); o++ )
	{	fprintf(stderr, "       %s ", options[o].option);
		for( char *s = options[o].usage; *s; s++ )
			if( *s == '*' )
				fprintf(stderr, "\n             ");
			else
				fputc(*s, stderr);
		fprintf(stderr, "\n");
    }
    fprintf(stderr, "       v=value include all versions up to <value> and skip later versions in file\n");
    fprintf(stderr, "Note:  %s will always generate a .gv (dot) file if possible\n             (see https://graphviz.org)\n", process);
	exit(0);
}

char *skipversion(char *name, char *value)
{	if( strcmp(name, "v") ) 
		nolineerror("** only v=value skip versions option implemented\n");
	else
		return value;
	return NULL;
}

int main(int argc, char *argv[])
{	int opened = 0;
	FILE *fp;
	char *bp, *openedfile, *processedFileName = NULL, *skip = NULL;
	int successfulskip = 0;

    // if any argument is -watch
    // pull out string of files for fswatch
    // create new argument string minus all -watch parameters
    // then system out and exit
    for( int i = 1; i < argc; i++ )
        if( !strcmp(argv[i], "-watch") )
        {   str *command = newstr("reed"), *files = newstr(""), *fswatch = newstr("fswatch ");
            int filecount = 0;
            for( int ii = 1; ii < argc; ii++ )
            {   if( strcmp(argv[ii], "-watch") )
                {   appendcstr(command, " ");
                    appendcstr(command, argv[ii]);
                }
                if( argv[ii][0] != '-' )
                {   filecount++;
                    appendcstr(files, argv[ii]);
                    appendcstr(files, " ");
                }
                if( !strcmp(argv[ii], "-v") )
                    verboseOption = 1; // this is the only option we pay attention in -watch
            }
            if( !filecount )
            {   nolineerror("-watch specified, but no files to watch, so nothing to do\n");
                exit(1);
            }
            //fprintf(stderr, "command = %s\n", command->s);
            if( verboseOption ) fprintf(stderr, "|--%s watching file%s: %s\n", argv[0], filecount? "": "s", files->s);
            appendstr(fswatch, files);
            appendcstr(fswatch, "| xargs -I {} ");
            appendstr(fswatch, command);
            if( verboseOption ) fprintf(stderr, "|--System:  %s\n", fswatch->s);
            system(fswatch->s);
            exit(0);
        }
    for( int i = 1; i < argc; i++ )
        if( setOption(argv[i]) )
        {   // fprintf(stderr, "i=%d arg=%d\n", i, argc);
            if( handleInsert )
            {   if( i+1 >= argc ) // can't use error() as there is no lineno yet
                {   nolineerror("-insert <text> must be followed by some text to insert\n");
                    exit(1);
                }
                fprintf(stderr, "-insert this text: %s\n", argv[i+1]);
                opened = 1;
                openedfile = "inserted-text";
                bp = argv[i+1];
                if( !parse(skip, openedfile, bp) ) // return 0 means a fatal error or matched version number to skip
                {   processedFileName = bp;
                    successfulskip = 1;
                    break;
                }
                processedFileName = openedfile;
                i++; // skip over the insert text
                handleInsert = 0;
            }
            if( handleTags )
            {   if( i+2 >= argc ) // can't use error() as there is no lineno yet
                {   nolineerror("-tags must be followed by both a start tag and an end tag\n");
                    exit(1);
                }
                fprintf(stderr, "tag start=\"%s\"\n", argv[i+1]);
                if( startTag.tagLength > 0 )
                {   // may fix this limitation soon
                    nolineerror("Attempting to redefine tags, but can only have one set of start and end tags\n");
                    exit(1);
                }
                startTag = setTag(argv[i+1]);
                endTag = setTag(argv[i+2]);
                fprintf(stderr, "tag end=\"%s\"\n", endTag.tagString);
                i += 2;
                handleTags = 0;
            }
            continue;
        }
        else if( !optionsOption && (bp = index(argv[i], '=')) != NULL )
        {   bp[0] = (char) 0;
            skip = skipversion(newstr(argv[i])->s, &bp[1]);
        }
        else
        if( (fp = fopen(openedfile = argv[i], "r")) != NULL )
        {   struct stat stat_buf;
            int errno;
            if( (errno = fstat(fileno(fp), &stat_buf)) != 0 )
            {   nolineerror("** Cannot stat %s: %s\n", openedfile, strerror(errno));
                exit(0);
            }
            bp = safealloc(1+stat_buf.st_size);
            if( fread(bp, 1, stat_buf.st_size, fp) != stat_buf.st_size )
            {   nolineerror("** Cannot read from \"%s\" (maybe a permissions problem?)\n", openedfile);
                continue;
            }
            bp[stat_buf.st_size] = (char) 0;
            opened = 1;
            //if( verboseOption ) fprintf(stderr, ": File %s\n", processedFileName);
            //fprintf(stderr, "Parse %s starting with successfulskip=%d\n", openedfile, successfulskip);
            hash(openedfile);
            if( !parse(skip, openedfile, bp) ) // return 0 means a fatal error or matched version number to skip
            {   processedFileName = openedfile;
                successfulskip = 1;
                break;
            }
            processedFileName = openedfile;
            //fprintf(stderr, "file processed = %s\n", openedfile);
            free(bp);
            fclose(fp);
        }
    if( syntaxOption )
        fprintf(stderr, "%s\n", syntaxSummary);
    if( showRulesOption )
        explainTranslationRules();
    if( !opened )
        fprintf(stderr, "** did not process any files\n");
   if( !syntaxOption && !showRulesOption && !opened )
        usage(argv[0]);
     if( skip && !successfulskip )
        nolineerror("Never matched version v=%s but used version '%s' instead", skip, version);
    findComponents(); // find components before generating HTML, Latex, etc
    if( processedFileName != NULL && *processedFileName ) // make dot, latex, etc files after last processed file name
        generateFiles(processedFileName); // processFileName is the last name
    return 0;
}
