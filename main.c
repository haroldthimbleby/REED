#include "header.h"
#include "notes.h"

extern void allmetadata();

void *safealloc(size_t size)
{   void *p = (void *) malloc(size);
    if( p == NULL )
        fatalError("** run out of memory (safealloc %d in main.c)", size);
    return p;
}

void *safeCalloc(size_t count, size_t size)
{   void *p = (void *) calloc(count, size);
    if( p == NULL )
        fatalError("** run out of memory (safeCalloc %d in main.c)", size);
    return p;
}

void *saferealloc(char *p, size_t size)
{	p = (void *) realloc(p, size);
	if( p == NULL )
		fatalError("** run out of memory (realloc %d in main.c)", size);
	return p;
}

char *undefinedVersion = "node version not set!";

str *newstr(const char *s)
{	str *new = (str*) safealloc(sizeof(str));
    new->s = (char*) safealloc(strlen(s)+1);
    (void) strcpy(new->s, s);
    //fprintf(stderr, "ok\n"); fflush(stderr);
    new->t = None;
	new->l = ID;
	new->is = new->note = new->style = new->group = new->exampleGroupMember = NULL;
	new->pointsTo = new->pointedFrom = new->isgroup = new->isstyle = new->rowDefaultNodeStyle = new->cascade = 0;
	new->flag = new->originalflag = noflag;
	new->plain = 0;
	new->rankx = new->ranky = 0;
	new->color = 0;
	new->styleName = NULL;
	new->nodeversion = undefinedVersion;
	new->noderef = "";
	new->component = 0;
    new->cyclic = new->declaredCyclic = 0;
    new->keywordsOK = 1;
    new->wasString = 0;
    new->visible = 1;
    new->metadata = NULL;
    new->inDegree = new->outDegree = 0;
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
{	// ch[] = "x";
	//ch[0] = c;
    //fprintf(stderr, "appendch %s + %c = ", d->s, c);
	d->s = saferealloc(d->s, strlen(d->s)+2);
    char *cp = d->s;
    while( *cp ) cp++;
    cp[0] = c;
    cp[1] = (char) 0;
	//strlcat(d->s, ch, strlen(d->s)+2);
    //fprintf(stderr, "[%s]\n", d->s);
	return d;
}

void strpad(str **s, int d)
{   //printf(" pad %s to %d\n", (*s)->s, d);
	while( strlen((*s)->s) < d )
		*s = appendch(*s, ' ');
}

str *appendstr(str *d, str *e)
{   appendcstr(d, e->s);
	return d;
}

str *appendcstr(str *d, char *e) // appends in situ, so old *d is changed
{   char *oldds = d->s;
    char *s = d->s = saferealloc(oldds, strlen(d->s)+strlen(e)+2);
    //free(oldds);
	while( *s ) s++;
    while( (*s++ = *e++) );
	return d;
}

str *newappendcstr(str *d, char *e) // appends to a new string, so old *d not changed
{	return appendcstr(newstr(d->s), e);
}

int basenameOption = 0,
    flagOption = 0,
	flagTextOption = 0,
	verboseOption = 0, 
    showIDsOption = 0,
    optionsOption = 0,
    graphvizOption = 0,
    transposeOption = 0,
    commentOption = 0,
    mathematicaOption = 0,
    transitOption = 0,
    showSignatures = 0,
    xmlOption = 0,
    latexOption = 0,
    htmlOption = 0,
    showVersionsOption = 0,
    showRulesOption = 0,
    componentsOption = 0,
    generatePDFOption = 0,
    generateSVGOption = 0,
    syntaxOption = 0,
    openOption = 0,
    IDsOption = 0,
    rawOption = 0,
    JSONOption = 0,
    colorsOption = 0,
    colorsPlusOption = 0,
    matchedpullOption = 0,
    matchedpullPlusOption = 0,
    pullPlusOption = 0,
    goOption = 0,
    hoOption = 0,
    pullOption = 0,
    versionOption = 0,
    allColorsOption = 0,
    listidsOption = 0,
    listissOption = 0,
    listBothOption = 0,
    flagsOption = 0,
    summariseOption = 0,
    keywordsOption = 0;

char *outputbasename = ""; // no basename is zero length string

enum flagcolor pullString = noflag;

char *syntaxSummary =
    #include "SyntaxCode.c"
;

int handleTags = 0, handleInsert = 0, handleWatch = 0, separatorOption = 0, hasHadTagsOption = 0;
tag startTag = {"", 0}, endTag = {"", 0};

tag setTag(char *str)
{   tag tmp = {str, strlen(str)};
    return tmp;
}

// the final 0/1 is to specify generating a .gv file
// for instance, a .gv file is needed to generate PDF
structOption options[] =
// usage replaces:
// * with <space>-- (so *- becomes <space>---) or newline and an indent
// [...] with \texttt{...} or nothing
// # replaced with \# or #
{	{"-#", "", "show comments in the REED file, useful if comments are used as reminders", &commentOption, 0},
    {"-basename", "<path/file.ext>", "set pathname for generated files (replacing [.ext] with [.gv], [.html], [.pdf], etc)", &basenameOption, 0},
    {"-c", "", "show weakly connected components", &componentsOption, 0},
    {"-allcolors", "", "list all colors that are available for highlighting", &allColorsOption, 0},
    {"-colors", "", "list all colors used", &colorsOption, 0},
    {"-colors+", "", "lists all colors like [-colors] but also gives the meanings of all colors used", &colorsPlusOption, 0},
    {"-F", "", "highlight flags in graph drawing*- unfortunately due to a GraphViz bug, this breaks up any groups", &flagOption, 1},
	{"-f", "", "show textual descriptions of flag colors in REED drawing", &flagTextOption, 1},
    {"-flags", "", "show all flag definitions", &flagsOption, 0},
	{"-g", "", "generate a GraphViz [.gv] file*- default option if nothing else chosen*- See [https://graphviz.org] for details", &graphvizOption, 1},
    {"-go", "", "as [-g] but also open the [.gv] file", &goOption, 1},
	{"-h", "", "generate an interactive [.html] REED file", &htmlOption, 0},
    {"-ho", "", "as [-h] but also open the [.html] file", &hoOption, 0},
    {"-ids", "", "show full names and IDs in graph", &IDsOption, 0},
    {"-insert", "<text>", "insert this text to process before next file", &handleInsert, 0},
    {"-json", "", "generate a JSON [.js] file with all information from the GraphViz diagram", &JSONOption, 1},
    {"-keywords", "", "list all keywords used", &keywordsOption, 0},
    {"-l", "", "generate a Latex REED file*- also generates some useful Latex definition files", &latexOption, 0},
    {"-m", "", "generate a Mathematica [.nb] file representing the REED graph as a series of expressions", &mathematicaOption, 0},
    {"-n", "", "show node IDs in graph drawing", &showIDsOption, 1},
    {"-o", "", "open generated files automatically", &openOption, 1},
    {"-pdf", "", "generate a [.pdf] file representing the REED graph", &generatePDFOption, 1},
    {"-pick", "<color>", "restrict generated files to just this color", &matchedpullOption, 1},
    {"-pick", "<keyword>", "restrict generated files to notes with this keyword. Keyword can be abbreviated: use [...] so [xyz...] matches keywords or phrases starting [xyz]", &matchedpullOption, 1},
    {"-pick+", "<color>", "same as [-pick] but also explains this color meaning", &matchedpullPlusOption, 1},
    {"-raw", "", "start in raw mode (skipping text until a start tag)*- only use [-raw] with [-tags] flag", &rawOption, 0},
    {"-rules", "", "summarise HTML-Latex rules", &showRulesOption, 0},
    {"-s", "", "list nodes and their full names, sorted by node ID", &listidsOption, 0},
    {"-ss", "", "list nodes and their full names, sorted by full names", &listissOption, 0},
    {"-sss", "", "list nodes and their full names, sorted by both node ID and full names", &listBothOption, 0},
    {"-sep", "", "draw a separator line before processing any files (useful with [-watch])", &separatorOption, 0},
    {"-sig", "", "show REED file signatures", &showSignatures, 0},
    {"-summarise", "", "put Latex overview of REED syntax and command line flags to standard output", &summariseOption, 0},
    {"-svg", "", "generate a [.svg] file*- representing the REED graph", &generateSVGOption, 1},
    {"-syntax", "", "summarise REED syntax", &syntaxOption, 0},
	{"-t", "", "transpose node numbering*- swap row and column node numbers", &transposeOption, 0},
    {"-tags", "<start> <end>", "only process REED information written between these tags*- you can change tags between files*- and also set tags within a REED file by: [tags <start> <end>]", &handleTags, 0},
    {"-transit", "", "list transit nodes and nodes with no influence", &transitOption, 0},
    {"-v", "", "verbose mode", &verboseOption, 0},
    {"-version", "", "state the version number of the REED command", &versionOption, 0},
    {"-w", "", "what versions are used in these files?*- helpful to know if using the [v=] flag", &showVersionsOption, 0},
    {"-watch", "", "run REED when any used file changes (nice with [-o] flag)", &handleWatch, 0},
	{"-x", "", "generate an [.xml] file*- representing all REED data for import into other applications", &xmlOption, 0},
	{"--", "", "treat all further parameters as filenames*- if you want to have no restrictions on filenames as they otherwise cannot be flags", &optionsOption, 0}
};

void sayVersion(int inLatex, FILE *fd)
{   if( inLatex )
        fprintf(fd, "The following summary was generated automatically by using \\texttt{reed~-summarise}, running ");
    fprintf(fd, "%s version 2.3. Compiled %s, %s.\n", "REED", __TIME__, __DATE__);
}

void summariseFeatures()
{   FILE *fd;
    char *summaryFile = "REEDsummary.tex";

    fd = stdout; // fopen(summaryFile, "wx");
    summaryFile = "standard output";

    if( fd == NULL )
    {   fprintf(stderr, "%s can't be written or already exists\n", summaryFile);
        return;
    }

    fprintf(stderr, "Writing Latex to %s to summarise REED features\n", summaryFile);

    fprintf(fd, "\\section{REED command line flags summary}\\label{flagsSummary}\n");
    sayVersion(1, fd);

    fprintf(fd, "\\vskip 5mm\\begin{description}\\raggedright\n");
    for( int i = 0; i < sizeof options/sizeof(structOption); i++ )
    {   fprintf(fd, "\\item[\\tt ");
        for( char *s = options[i].option; *s; s++ )
            if( *s == '#') fprintf(fd, "\\#");
            else fprintf(fd, "%c", *s);
        if( *options[i].args )
            fprintf(fd, " %s", options[i].args);
        fprintf(fd, "]~\\\\ ");
        for( char *s = options[i].usage; *s; s++ )
            fprintf(fd, *s == '*'? " --":
                    *s == '['? "\\texttt{":
                    *s == ']'? "}": "%c", *s);
        fprintf(fd, ".\n\n");
    }
    fprintf(fd, "\\end{description}\n");

    fprintf(fd, "\\section{REED syntax summary}\\label{syntaxSummary}\n");
    sayVersion(1, fd);
    fprintf(fd, "\\vskip 5mm\n{\\tt\\noindent\\hbox{}");
    int roman = 0;
    for( char *s = syntaxSummary; *s; s++ )
        switch( *s )
        {
            case '#':
                if( !roman )
                {   roman = 1;
                    fprintf(fd, "{\\rm ");
                }
            case '{':
            case '}':
                fprintf(fd, "\\%c", *s); break;
            case ' ':
                fprintf(fd, "~"); break;
            case '|':
                fprintf(fd, "\\texttt{|}"); break;
            case '!':
                if( roman ) fprintf(fd, "}");
                roman = 1;
                fprintf(fd, "\\\\\n\\hbox to 3em{}{\\rm "); break;
            case '\\':
                fprintf(fd, "\\textbackslash\\@"); break;
            case '\n':
                if( roman ) fprintf(fd, "}");
                roman = 0;
                fprintf(fd, "\\\\\n\\hbox{}"); break;
            case '<':
                fprintf(fd, roman? "\\texttt{": "<"); break;
            case '>':
                fprintf(fd, roman? "}": ">"); break;
            default:
                fprintf(fd, "%c", *s); break;
        }
    if( roman ) fprintf(fd, "}");
    roman = 0;
    fprintf(fd, "}\n");
    // fclose(fd); // not for stdout :-)
}

int setSomeInterestingOption = 0;

int setOption(char *argvi)
{	if( !optionsOption )
		for( int o = 0; o < sizeof options/sizeof(structOption); o++ )
			if( !strcmp(options[o].option, argvi) )
            {   setSomeInterestingOption |= 1;
                graphvizOption |= options[o].needGraphViz; // for all options that require the .gv file
                return *options[o].optionFlag = 1;
            }
    if( !optionsOption && argvi[0] == '-' )
        nolineerror("Flag '%s' not recognised", argvi);
	return 0;
}

void usage(char *process)
{	fprintf(stderr, "%s command line parameters summary\n", process);
    fprintf(stderr, "       filenames... process files, each according to preceding flags\n");
    for( int o = 0; o < sizeof options/sizeof(structOption); o++ )
	{	fprintf(stderr, "       %s ", options[o].option);
        if( *options[o].args )
            fprintf(stderr, "%s ", options[o].args );
		for( char *s = options[o].usage; *s; s++ )
			if( *s == '*' )
				fprintf(stderr, "\n             ");
            else if( *s == '[' || *s == ']' )
                continue;
			else
				fputc(*s, stderr);
		fprintf(stderr, "\n");
    }
    fprintf(stderr, "       v=<value> include all versions up to <value>\n");
    exit(0);
}

extern int versionCount;
char *parseversion(char *name, char *value)
{	if( strcmp(name, "v") )
		nolineerror("** only v=<value> skip versions option implemented\n");
	else
    {   if( !strcmp(value, "") )
        {   nolineerror("v='' no version defined");
            exit(1);
        }
        fprintf(stderr, "Generate version '%s'\n", value);
        return value;
    }
    return NULL;
}

extern char *keywordtopull, *whichPull;

void test(char *a, char *b)
{   int r = versionstrcmp(a, b), s = versionstrcmp(b, a), rfix, sfix;
    rfix = r < -1? -1: r > 1? 1: r;
    sfix = s < -1? -1: s > 1? 1: s;
    fprintf(stderr, "%s <> %s = %d (strcmp=%d) reverse?%s => %s %s %s\n", a, b, r, strcmp(a,b),
            rfix == -sfix? "OK": "FAIL",
            r < 0? b: a,
            (!r)? "=": "<",
            r < 0? a: b
            );
}

int main(int argc, char *argv[])
{	int opened = 0;
    FILE *fp;
	char *bp, *openedfile, *processedFileName = NULL, *targetVersion = "";

/*
    test("hello", "hello");
    test("helloLonger", "hello");
    test("A", "B");
    test("a", "A");
    test("xyzBe", "xyzbe");
    test("fred12.5", "fred12.5");
    test("!@#&", "!@#");
    test("!@#&", "!@#&*");
    test("fred15x", "fred1x");
    test("v2.3", "v2.29");
    test("v2.3", "v2.31");
    test("v9001", "v873");
    test("v1234", "v90");
    exit(0);
*/

    // if any argument is -watch
    // pull out string of files for fswatch system call
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
                if( argv[ii][0] != '-' ) // skip other flags
                {   filecount++;
                    appendcstr(files, argv[ii]);
                    appendcstr(files, " ");
                }
                if( !strcmp(argv[ii], "-v") )
                    verboseOption = 1; // this is the only option we need pay attention to in -watch
            }
            if( !filecount )
                fatalError("-watch specified, but no files to watch, so nothing to do");
            //fprintf(stderr, "command = %s\n", command->s);
            if( verboseOption ) fprintf(stderr, "|--%s watching file%s: %s\n", argv[0], filecount? "": "s", files->s);
            appendstr(fswatch, files);
            appendcstr(fswatch, "| xargs -I {} ");
            appendstr(fswatch, command);
            if( verboseOption ) fprintf(stderr, "| -- %s\n", fswatch->s);
            system(fswatch->s);
            exit(0);
        }
    for( int i = 1; i < argc; i++ )
        if( setOption(argv[i]) )
        {   // fprintf(stderr, "i=%d arg=%d\n", i, argc);
            if( handleInsert )
            {   if( i+1 >= argc ) // can't use error() as there is no lineno yet
                    fatalError("-insert <text> must be followed by some text to insert");
                // fprintf(stderr, "-insert this text: %s\n", argv[i+1]);
                opened = 1;
                openedfile = "inserted-text";
                bp = argv[i+1];
                if( !parse(openedfile, bp) ) // return 0 means a fatal error
                {   processedFileName = bp;
                    break;
                }
                checkarrowlist(1012);
                processedFileName = openedfile;
                i++; // skip over the insert text
                handleInsert = 0;
            }
            if( handleTags ) // set on each use of -tags
            {   hasHadTagsOption = 1;
                if( i+2 >= argc ) // can't use error() as there is no lineno yet
                    fatalError("-tags must be followed by both a start tag and an end tag");
                //fprintf(stderr, "tag start=\"%s\"\n", argv[i+1]);
                if( startTag.tagLength > 0 )
                    // may fix this limitation soon
                    fatalError("Attempting to redefine tags, but can only have one set of start and end tags");
                startTag = setTag(argv[i+1]);
                endTag = setTag(argv[i+2]);
                //fprintf(stderr, "tag end=\"%s\"\n", endTag.tagString);
                if( !strcmp(startTag.tagString, endTag.tagString) )
                    fprintf(stderr, "Warning: it's usually a bad idea for start and end tags to be the same (they are currently both set to \"%s\")\n", endTag.tagString);
                i += 2;
                handleTags = 0;
            }
            if( basenameOption )
            {   if( i+1 >= argc )
                    fatalError("-basename must be followed by either pathname/file or file");
                if( *outputbasename )
                    nolineerror("Cannot have multiple basenames, '%s' and '%s'", outputbasename, argv[i+1]);
                outputbasename = argv[i+1];
                basenameOption = 0;
                i += 1;
            }
            if( matchedpullOption || matchedpullPlusOption )
            {   whichPull = matchedpullPlusOption? "pick+": "pick";
                pullOption |= matchedpullOption;
                pullPlusOption |= matchedpullPlusOption;
                matchedpullOption = matchedpullPlusOption = 0;
                if( i+1 >= argc || iscolor(argv[i+1]) == noflag ) // can't use error() as there is no lineno yet
                {   // -pick <not a color>, so see if it is a keyword
                    keywordtopull = argv[i+1];
                    i += 1;
                    continue;
                }
                if( pullString != noflag )
                   nolineerror("Can only -%s one color", whichPull);
                pullString = iscolor(argv[i+1]);
                if( verboseOption ) fprintf(stderr, "|-- -%s %s\n", whichPull, flagcolor(pullString));
                fprintf(stderr, "Warning: -%s for -l has not been implemented yet\n", whichPull);
                fprintf(stderr, "Warning: because of -pick, -h and -svg flags have been applied automatically (and will only generate picked material)\n");
                htmlOption = 1;
                generateSVGOption = 1;
                i += 1;
            }
            continue;
        }
        else if( !optionsOption && (bp = index(argv[i], '=')) != NULL )
        {   bp[0] = (char) 0;
            targetVersion = parseversion(newstr(argv[i])->s, &bp[1]);
        }
        else
        if( (fp = fopen(openedfile = argv[i], "r")) != NULL )
        {   struct stat stat_buf;
            int errno;
            fprintf(stderr, "%s:\n", openedfile);
            if( (errno = fstat(fileno(fp), &stat_buf)) != 0 )
                fatalError("** %s cannot stat \"%s\": %s", argv[0], openedfile, strerror(errno));
            bp = safealloc(1+stat_buf.st_size);
            if( fread(bp, 1, stat_buf.st_size, fp) != stat_buf.st_size )
            {   nolineerror("** %s cannot read from \"%s\" (maybe a permissions problem?)\n", argv[0], openedfile);
                continue;
            }
            bp[stat_buf.st_size] = (char) 0; // EOF is made (char) 0
            opened = 1;
            //if( verboseOption ) fprintf(stderr, ": File %s\n", processedFileName);
            hash(openedfile);

            if( rawOption && !hasHadTagsOption )
                nolineerror("Using -raw makes no sense unless -tags has been set, because in raw mode nothing will be processed until a begin tag is found");

            processedFileName = openedfile;
            checkarrowlist(10199);
            if( !parse(openedfile, bp) ) // return 0 means a fatal error
                break;
            checkarrowlist(10100);
            //fprintf(stderr, "file processed = %s\n", openedfile);
            //free(bp); // we need to keep bp around in case we later generate error messages quoting it
            fclose(fp);
        }
        else nolineerror("** %s cannot open file \"%s\": %s", argv[0], openedfile, strerror(errno));
    if( syntaxOption )
    {   for( char *summary = syntaxSummary; *summary; summary++ )
        {  if( *summary == '!' ) fprintf(stderr, "\n        -  ");
           else
               fprintf(stderr, "%c", *summary);
        }
        fprintf(stderr, "\n");
    }
    checkarrowlist(1010);
    if( showRulesOption )
        explainTranslationRules();
    if( allColorsOption )
        showAllColors();
    if( !opened )
        fprintf(stderr, "** %s did not process any files\n", argv[0]);
    checkarrowlist(101);
    findComponents(); // find components before generating HTML, Latex, etc
    checkarrowlist(102);
    findCycles();
    checkarrowlist(103);
    allmetadata();

    if( processedFileName != NULL && *processedFileName ) // make dot, latex, etc files after last processed file name
    {checkarrowlist(104);
        if( !setSomeInterestingOption ) graphvizOption = 1; // effectively set -g if nothing else

        if( keywordtopull )
        {   if( !isakeyword(keywordtopull) )
                nolineerror("-%s must be followed by highlight colors or keywords to pick", whichPull);
            else pullkeywords(keywordtopull);
        }

        generateFiles(targetVersion, *outputbasename? outputbasename: processedFileName); // processFileName is the last file named
        if( colorsOption || colorsPlusOption )
            listColorsUsed();
        if( keywordsOption )
            summarisekeywords(stderr);
    }

    if( versionOption ) sayVersion(0, stderr);

    listnodes(); // does stuff if -s -ss or -sss used

    if( summariseOption )
        summariseFeatures();

    if( flagsOption )
        usage(argv[0]);
    else if( !summariseOption && !syntaxOption && !showRulesOption && !opened && !allColorsOption && !versionOption && !listidsOption )
        fprintf(stderr, "Use -flags to list all flag options\n");

    return 0;
}
