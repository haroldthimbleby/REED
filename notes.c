#include "header.h"
#include "notes.h"
extern char *flagdefinitions[], *flagcolors[];
extern int flagsused[], flagsusedaftercascades[];
extern void LaTeXtranslate(FILE *opfd, char *version, char *note, str *innode); // convert <<id>> notation

/*
nodes, arrows can have lists of notes
a note is a string, a list of authors, a next note

operations on notes

get text 
sort by author
get next note 

author consensus --- short hand for all authors. Hmm, there'd only be one note then?

checks
- every node has at least one note
- if an arrow or a node has more than one note, there are authors on every one
- all notes (for a given node/arrow) have different *sets* of authors

 */

authorList *authors = NULL, *endofAuthorlist;

int versionCount = 0; 
str *listOfVersions = NULL;
	
void appendVersions(char *version)
{	versionCount++;
	if( listOfVersions == NULL ) listOfVersions = newstr("");
	if( strlen(listOfVersions->s) > 0 ) appendcstr(listOfVersions, "; ");
	listOfVersions = appendcstr(listOfVersions, version);
}

int needSwap(str *u, str *v)
{	if( u->rankx > v->rankx ) return 1;
	if( u->rankx == v->rankx && u->ranky > v->ranky ) return 1;
	if( u->rankx+u->ranky+v->rankx+v->ranky ) return 0;
	// if there is no node numbering, use their names
	//fprintf(stderr, "%s <> %s\n", u->s, v->s);
	if( strcmp(u->s, v->s) > 1 ) return 1;
	return 0;
}

char *trim(char *s)
{	while( *s == ' ' || *s == '\t' )
		s++;
	return s; // first non-blank of string
}

int needArrowSwap(arrow *u, arrow *v) 
{	char *a, *b;
	if( u == NULL || v == NULL ) return 0;
	a = u->arrowis != NULL? trim(u->arrowis->s): "xx";
	b = v->arrowis != NULL? trim(v->arrowis->s): "xx";
	//fprintf(stderr, "'%s' <> '%s' = %d\n", a, b, strcmp(a, b));
	return strcmp(a, b) > 0;
}

int newauthor(char *author) // add author to REED author list
{
	// ignore repeated authors
	authorList *p = authors;
	while( p != NULL )
	{	if( !strcmp(p->author, author) )
			return 0;
		p = p->next;
	}
	
	// then append authors in same order they are mentioned in the file
	p = (authorList*) malloc(sizeof(authorList));
	if( authors == NULL ) // first author
	{	p->author = author;
		p->next = NULL;
		authors = p;
		endofAuthorlist = p;
		return 1;
	}
	endofAuthorlist->next = p;
	p->author = author;
	p->next = NULL;
	endofAuthorlist = p;
	return 0;
}

void latexxrefs(FILE *opfd)
{	fprintf(opfd, "%% \\input this file before \\begin{document} to get cross references\n");
	fprintf(opfd, "%% \\ref{node-ID} gives the node version and reference\n");
	fprintf(opfd, "%% \\ref{node-ID-is} gives the node full name\n");
	// for all nodes id, generate a list: \newlabel{node-id}{{node reference}{}} and \newlabel{node-id-is}{{node name}{}}
	for( node *t = nodeList; t != NULL; t = t->next )
	{	fprintf(opfd, "\\newlabel{node-%s}{{", t->s->s);
		printrank(opfd, t->s, version);
		fprintf(opfd, "}{}}\n");
		fprintf(opfd, "\\newlabel{node-%s-is}{{%s}{}}\n", t->s->s, t->s->is == NULL? t->s->s: t->s->is->s);
	}
}

void LaTeXcolorkey(FILE *opfd, char *heading, char *vskip)
{	int flagLegends = 0;
	int hasHeading = *heading;
	char *vbar = hasHeading? "|": "";
	char *hbar = hasHeading? "\\hline": "";
		
		// if there was a cascade, all the flags used will be wrong, so fix them -- because we thought auxcascade() was the wrong place to do it :-)
		for( int flag = 0; flag <= numberOfColors; flag++ )
			flagsusedaftercascades[flag] = 0;
		for( node *t = nodeList; t != NULL; t = t->next )
			flagsusedaftercascades[t->s->flag]++;
		
		for( int i = 1; i <= numberOfColors; i++ ) // gets them in alphabetical order
		{	if( *flagdefinitions[i] )
			{	if( !flagLegends )
				{	myfprintf(opfd, "\\setbox0=\\hbox{\\colorflag{white}}%%\n\
\\dimen0 = \\ht0 \\advance \\dimen0 by 1ex \\ht0 = \\dimen0\n");
					myfprintf(opfd, "\\begin{tabular}{@{}%sclp{3.75in}%s}\n", vbar, vbar);
					if( hasHeading )
                    {   extern char *title;
                        myfprintf(opfd, "%s\n\\multicolumn{3}{@{}|l|}{\\bf %s", hbar, heading);
                        if( *title ) myfprintf(opfd, " for %s", title);
                        myfprintf(opfd, "} \\\\ \\hline\n");
                    }
				}
				flagLegends++;
				if( flagLegends == 1 )
					myfprintf(opfd, "\\vphantom{\\copy0}");
				myfprintf(opfd, "\\colorflag{%s}&", flagcolors[i]);
				myfprintf(opfd, "%s ", flagcolors[i]);
				if( !flagsusedaftercascades[i] ) myfprintf(opfd, "\\bf not used&");
				else myfprintf(opfd, "used %d time%s&", flagsusedaftercascades[i], flagsusedaftercascades[i] == 1? "": "s");
				myfprintf(opfd, "%s", flagdefinitions[i]); 
				if( hasHeading && flagsusedaftercascades[i] != flagsused[i] )
				{	myfprintf(opfd, "\\\\&&\\scriptsize (%s used explicitly ", flagcolors[i]);
					if( flagsused[i] == 1 )
						myfprintf(opfd, "once before cascading)", flagsused[i]);
					else
						myfprintf(opfd, "%d times before cascading)", flagsused[i]);
				}
				myfprintf(opfd, "\\\\ \n");
			}
		}
		if( flagLegends )
			myfprintf(opfd, "%s \\end{tabular}\n%s\n", hbar, vskip);
}

void LaTeXkeywords(FILE *opfd, struct keywordlist *keywords)
{   if( keywords != NULL )
    {   char *sep = "";
        fprintf(opfd, "\\textcolor{blue}{\\sf\\textbf{Keyphrase%s:} ", keywords->next == NULL? "": "s");
        for( struct keywordlist *t = keywords; t != NULL; t = t->next )
        {   myfprintf(opfd, "%s%s", sep, t->keyword->s);
            sep = "; ";
        }
        fprintf(opfd, ".}\n\\vskip 1ex\n");
    }
}

// generate the narrative .tex (latex) file
void notes(FILE *opfd, char *title, char *version, authorList *authors, char *date, char *abstract)
{ 	// note use of %t in myfprintf instead of %s in fprintf - this makes %s latex-safe

    myfprintf(opfd, "\\documentclass[10pt,a4wide]{article}\n"
              "\\usepackage{geometry}\n\\geometry{a4paper}\n"
              "\\usepackage{xcolor}\n"
              "\\usepackage{url}\n"
              "\\usepackage{graphicx}\n"
              "\\usepackage{hyperref}\n"
              "\\DeclareGraphicsRule{.tif}{png}{.png}{`convert #1 `dirname #1`/`basename #1 .tif`.png}"
            );

    myfprintf(opfd, "\n%s\n", latexdefinitions->s);

	myfprintf(opfd, "\\title{%t%t%t%t}\\author{", title,
			*title? "\\\\": "", *version? "Version ": "", version);

	while( authors != NULL )
	{	myfprintf(opfd, "%t", authors->author);
		if( authors->next != NULL ) 
			fprintf(opfd, authors->next->next == NULL? " \\& ": ", ");
		authors = authors->next;
	}

	fprintf(opfd, "}\n\\date{%s}\n\\begin{document} \\maketitle\n", date);

	fprintf(opfd, "\\def\\colorflag#1{\\fboxrule=0.3pt\n\\fboxsep=0pt\n\
\\hbox{\\vrule height 2.2ex width 1pt\n\
\\raise 1ex\\hbox{\\fbox{\\color{#1}\\hbox{\\vrule width .5ex height 1ex depth 0ex}}}%%\n\
\\hskip -.6pt\\raise .9ex\\hbox{\\fbox{\\color{#1}\\hbox{\\vrule width .5ex height 1ex depth 0ex}}}%%\n\
\\hskip -.6pt\\raise 1ex\\hbox{\\fbox{\\color{#1}\\hbox{\\vrule width .7ex height 1ex depth 0ex}}}}%%\n\
}");

    fprintf(opfd, "\\setlength{\\fboxsep}{1.5pt}\n"); // for from: to: here: annotations

	if( *abstract )
	{	myfprintf(opfd, "\n\\begin{abstract}\n");
        summariseLaTeXkeywords(opfd);
		LaTeXtranslate(opfd, version, abstract, NULL);
        myfprintf(opfd, "\n\\end{abstract}\n");
	}

    myfprintf(opfd, "\n%s\n", introduction->s);

	// sort notes into order using bubble sort
	int swapped = 0;
	do
	{	
		//for( node *t = nodeList; t != NULL; t = t->next )
		//	fprintf(stderr, "%d.%d (%s)   ", t->s->rankx, t->s->ranky, t->s->s);
		//fprintf(stderr, "\n");

		swapped = 0;
		for( node **t = &nodeList; (*t) != NULL && (*t)->next != NULL; t = &(*t)->next )
			if( needSwap((*t)->s, (*t)->next->s) ) 
			{	//fprintf(stderr, "  swap *t: %d.%d & (*t)->next: %d.%d\n", (*t)->s->rankx, (*t)->s->ranky, (*t)->next->s->rankx, (*t)->next->s->ranky);
				node *u = *t;
				*t = (*t)->next;
				//fprintf(stderr, "  A now u: %d.%d & t: %d.%d\n", u->s->rankx, u->s->ranky, (*t)->s->rankx, (*t)->s->ranky);
				u->next = (*t)->next;
				//fprintf(stderr, "  B now u->next: %d.%d & t: %d.%d\n", u->next->s->rankx, u->next->s->ranky, (*t)->s->rankx, (*t)->s->ranky);
				(*t)->next = u;
				//fprintf(stderr, "  C now u: %d.%d & t: %d.%d\n", u->s->rankx, u->s->ranky, (*t)->s->rankx, (*t)->s->ranky);
				swapped = 1;
				//for( node *t = nodeList; t != NULL; t = t->next )
				//	fprintf(stderr, "%d.%d - ", t->s->rankx, t->s->ranky);
				//fprintf(stderr, "\n");
				//break;
			}
	} while( swapped );
	
	//fprintf(stderr, "\\section*{Notes}\n");
	int nonotes = 0, anyflags = 0, anynodes = 0, anynotes = 0, anyarrows = 0, anyarrownotes = 0;
	for( node *t = nodeList; t != NULL; t = t->next )
	{	anynodes++;
		if( t->s->flag ) anyflags++;
		//fprintf(stderr, "%s - %d\n", t->s->s, (int) t->s->note);
		if( t->s->note == NULL ) // && !t->s->isstyle ) // styles don't have notes
		{ 	if( nonotes == 0 )
			{ 	fprintf(stderr, "Warning: No notes provided for these nodes:\n");
				nonotes = 1;
			}
			myfprintf(stderr, "         - %t", t->s->s);
			if( t->s->flag != noflag )
				fprintf(stderr, "         - Node with %s highlight has no notes", flagcolor(t->s->flag));
			myfprintf(stderr, "\n");
			nonotes = 1;
		} else anynotes++;
	}

	for( arrow *a = arrowList; a != NULL; a = a->next )
		anyarrows++;
	for( arrow *t = noteArrowList; t != NULL; t = t->next )
		anyarrownotes++;
	fprintf(opfd, "\\section*{Quick overview}\n{\\large\\sf\\noindent \\begin{tabular}{@{}|rlcrl|}\\hline \n");
	if( versionCount > 0 )
		fprintf(opfd, "\\multicolumn{5}{@{}|l|}{%sersion%s: %s}\\\\\\hline\n", 
			versionCount > 1? "Combined v": "V", versionCount == 1? "": "s", listOfVersions->s);
	else
		fprintf(opfd, "\\multicolumn{5}{@{}|l|}{Single version}\\\\\n");
	fprintf(opfd, " %d & node%s & --- & %d & highlighted\\\\&&& %d & with notes\\\\\n\n    %d & arrow%s & --- & %d & with notes\\\\\\hline\n\\end{tabular}\n", 
		anynodes, anynodes==1? "": "s", anyflags, anynotes, anyarrows, anyarrows==1? "": "s", anyarrownotes);
		
	printfiledata(opfd);
	fprintf(opfd, "}\n");

    if( numberOfComponents > 1 )
    {    myfprintf(opfd, "\n\n\\vskip 5mm\\noindent\nThere are %d weakly connected components (i.e., there are %d independent REED diagrams not connected to each other with any arrows, regardless of the directions of the arrows).\n\n", numberOfComponents, numberOfComponents);
        myfprintf(opfd, "\\begin{description}\n");
        for( int c = 1; c <= numberOfComponents; c++ )
            myfprintf(opfd, "\\item[$\\rightarrow$] \\hyperlink{component%d-narrative}{Narrative evidence for component %d}\n", c, c);
        myfprintf(opfd, "\\end{description}\n");
    }

	if( anyflags )
	{
		myfprintf(opfd, "\n\\section*{%d highlighted node%s}\n", anyflags, anyflags == 1? "": "s");
		
        LaTeXcolorkey(opfd, "Highlighting key", "\\vskip 4ex");

		myfprintf(opfd, "\\noindent\\begin{tabular}{@{}llll}\n");
		for( int i = 1; i < 8; i++ ) // gets flags in alphabetical order
		{	for( node *t = nodeList; t != NULL; t = t->next )
				if( t->s->flag != noflag && t->s->flag == i )
				{	myfprintf(opfd, "\\colorflag{%s}&", flagcolor(t->s->flag));
					if( t->s->cascade || flagcascade[i] )
						myfprintf(opfd, "Cascaded %s&", flagcascade[i]? "color": "node");
					else 
					if( t->s->originalflag != noflag && t->s->originalflag != t->s->flag )
						myfprintf(opfd, "\\scriptsize (%s before cascade)&", flagcolor(t->s->originalflag));
					else
						myfprintf(opfd, "&");
					printrank(opfd, t->s, version);
					myfprintf(opfd, "&\\hyperlink{%s}{%t}\\\\\n", t->s->s, t->s->is == NULL? t->s->s: t->s->is->s);
				}
		}
		myfprintf(opfd, "\n\\end{tabular}\n");
	} 
	
	if( 1 ) 
	{	int anynotes = 0;
        int component;
        // if there are lots of components, list nodes in order of component number

        for( component = 1; component <= numberOfComponents; component++ )
        {   anynotes = 0; // per component
            for( node *t = nodeList; t != NULL; t = t->next )
                if( t->s->note != NULL && t->s->component == component )
                {	if( !anynotes )
                {    myfprintf(opfd, "\\hypertarget{component%d-narrative}{\\section*{Node narrative evidence", component);
                    if( numberOfComponents > 1 )
                        myfprintf(opfd, " for component %d", component);
                    myfprintf(opfd, "}}\n");
                    myfprintf(opfd, "\\begin{description}\n");
                    for( int c = 1; c <= numberOfComponents; c++ )
                        if( c != component )
                            myfprintf(opfd, "\\item[$\\rightarrow$] \\hyperlink{component%d-narrative}{All narrative for component %d}\n", c, c);
                    myfprintf(opfd, "\\end{description}\n");
                }
                    anynotes = 1;
                    myfprintf(opfd, "\n\n\\hypertarget{%s}{\\subsection*{", t->s->s);
                    if( t->s->flag != noflag )
                        myfprintf(opfd, " \\colorflag{%s} ", flagcolor(t->s->flag));

                    myfprintf(opfd, "Node ");
                    printrank(opfd, t->s, version);
                    myfprintf(opfd, " %T",
                              t->s->is != NULL? t->s->is->s: t->s->s);

                    if( t->s->group != NULL )
                        myfprintf(opfd, "\\\\\\hskip 2em --- (Group: %t) ", t->s->group->is == NULL? t->s->group->s: t->s->group->is->s);
                    if( showIDsOption ) myfprintf(opfd, "\\fbox{%t} ", t->s->s);
                    //if( *version ) myfprintf(opfd, " (%t)", version);
                    myfprintf(opfd, "}}\n");
                    LaTeXkeywords(opfd, t->s->keywords);
                    LaTeXtranslate(opfd, version, t->s->note->s, t->s);

                    int anyarrows = 0;
                    for( arrow *a = arrowList; a != NULL; a = a->next )
                        if( a->u == t->s )
                        {	if( !anyarrows ) myfprintf(opfd, "\\vskip .5ex\\vbox{\\small ");
                            anyarrows = 1;
                            myfprintf(opfd, "\\hskip 2em\\\\$\\rightarrow$ \\hyperlink{%s}{", a->v->s);
                            if( a->v->flag != noflag ) myfprintf(opfd, "\\colorflag{%s}~", flagcolor(a->v->flag));
                            else myfprintf(opfd, "\\hphantom{\\colorflag{white}}~");
                            printrank(opfd, a->v, version);
                            myfprintf(opfd, " %t}", a->v->is != NULL? a->v->is->s: a->v->s);
                        }
                    if( anyarrows ) myfprintf(opfd, "}");
                }
        }
	}

	int arrownotes = 0;
	// sorts into order using bubble sort
	do
	{	
		//for( node *t = nodeList; t != NULL; t = t->next )
		//	fprintf(stderr, "%d.%d (%s)   ", t->s->rankx, t->s->ranky, t->s->s);
		//fprintf(stderr, "\n");	
		swapped = 0;
		for( arrow **t = &noteArrowList; (*t) != NULL && (*t)->next != NULL; t = &(*t)->next )
			if( needArrowSwap((*t), (*t)->next) )  
			{	//fprintf(stderr, "  swap *t: %d.%d & (*t)->next: %d.%d\n", (*t)->s->rankx, (*t)->s->ranky, (*t)->next->s->rankx, (*t)->next->s->ranky);
				arrow *u = *t;
				*t = (*t)->next;
				//fprintf(stderr, "  A now u: %d.%d & t: %d.%d\n", u->s->rankx, u->s->ranky, (*t)->s->rankx, (*t)->s->ranky);
				u->next = (*t)->next;
				//fprintf(stderr, "  B now u->next: %d.%d & t: %d.%d\n", u->next->s->rankx, u->next->s->ranky, (*t)->s->rankx, (*t)->s->ranky);
				(*t)->next = u;
				//fprintf(stderr, "  C now u: %d.%d & t: %d.%d\n", u->s->rankx, u->s->ranky, (*t)->s->rankx, (*t)->s->ranky);
				swapped = 1;
				//for( node *t = nodeList; t != NULL; t = t->next )
				//	fprintf(stderr, "%d.%d - ", t->s->rankx, t->s->ranky);
				//fprintf(stderr, "\n");
				//break;
			}
	} while( swapped );	

	for( arrow *t = noteArrowList; t != NULL; t = t->next )
	{ 	if( !arrownotes )
			myfprintf(opfd, "\\section*{Arrow narrative evidence}\n");
		arrownotes = 1;
		myfprintf(opfd,"\\subsection*{Arrow ");
		if( t->arrowis != NULL )
			myfprintf(opfd, "%t\\\\\n", t->arrowis->s);
		myfprintf(opfd, "\\setbox0=\\hbox{$\\rightarrow$~}\\hbox to \\wd0{}");
		printrank(opfd, t->u, t->u->nodeversion); 
		myfprintf(opfd, " \\hyperlink{%s}{%t}\\\\", t->u->s, t->u->is != NULL? t->u->is->s: t->u->s);
		myfprintf(opfd, "\\copy0{}");
		printrank(opfd, t->v, t->v->nodeversion); 
		myfprintf(opfd, " \\hyperlink{%s}{%t}", t->v->s, t->v->is != NULL? t->v->is->s: t->v->s);
		myfprintf(opfd, "}\n");
        LaTeXkeywords(opfd, t->keywords);
		LaTeXtranslate(opfd, t->u->nodeversion, t->arrownote->s, t->u);
	}

	//if( !anyflags ) 
	//	myfprintf(opfd, "\n\\section*{No highlighted nodes}\n");
	
	if( nonotes ) 
	{	fprintf(opfd, "\\section*{No notes provided}\n");
		for( node *t = nodeList; t != NULL; t = t->next )
			if( t->s->note == NULL )
			{	myfprintf(opfd, "\\textbf{Node ");
				printrank(opfd, t->s, version);
				myfprintf(opfd, " %t}", t->s->is != NULL? t->s->is->s: t->s->s);
				if( t->s->is != NULL )
					myfprintf(opfd, " [%t]", t->s->s);
				fprintf(opfd, "\\\\");
			} 
	}
    myfprintf(opfd, "\n%s\n", conclusion->s);
	fprintf(opfd, "\n\n\\end{document}\n");
}

// collapse white space containing newlines to just the newlines
str *collapseblanks(str *s)
{   char *p = s->s, *skip = s->s;
    while(1)
    {   int blanks = 0, newlines = 0;
        while( *skip == ' ' || *skip == '\t' || *skip == '\n' )
        {   if( *skip == '\n' ) newlines++; else blanks++;
            skip++;
        }
        if( newlines )
            while( newlines-- )
                *p++ = '\n';
        else if( blanks ) *p++ = ' ';
        if( !(*p++ = *skip++) ) break;
    }
    return s;
}

void defineArrowNote(str *u, str *v, str *theNote, str *theIs, struct keywordlist **keywordlist)
{	//fprintf(stderr, "defineArrowNote: label=%s a->arrownote->s = %s\n", theIs == NULL? "NULL": theIs->s, theNote->s);

	for( arrow *t = noteArrowList; t != NULL; t = t->next )
		if( t->u == u && t->v == v )
			error("arrow %s -> %s note being redefined", t->u->s, t->v->s);

	arrow *a = (arrow*) malloc(sizeof(arrow));
	a->next = noteArrowList;
	a->arrownote = collapseblanks(theNote);
    a->keywords = *keywordlist;
    *keywordlist = NULL;
	a->arrowis = theIs;
	a->u = u;
	a->v = v;
	noteArrowList = a;
}

void defineNodeNote(arrow *nl, str *theNote, struct keywordlist **keywordlist)
{	if( nl->u->note != NULL ) error("Defining another note for %s", nl->u->s);
    nl->u->keywords = *keywordlist;
    *keywordlist = NULL;
	nl->u->note = collapseblanks(theNote);
}
