#include "header.h"
#include "notes.h"
extern char *flagdefinitions[], *flagcolors[];
extern int flagsused[], flagsusedaftercascades[];
extern void allVersions(FILE *opfd);
extern int needSwap(str *u, str *v);
extern int needArrowSwap(arrow *u, arrow *v);
extern int versionCount;
extern void HTMLprintfiledata(FILE *opfd);
extern void HTMLtranslate(FILE *opfd, str *context, char *note); // translate Latex and HTML to HTML, and include <<id>> notation

char *css = ".showCells { border:1px solid; border-collapse:collapse; }\n\
.showCells td  { border:1px solid; padding: 8px;  border-collapse:collapse; margin:0; }\n\
table {padding: 0; margin:0; }\n\
    .boxed {\n\
      border: 2.5px solid black; padding: 10px; margin: 10px;\n\
    }\n\
h1, h2, h3 { font-family: Arial, Helvetica, sans-serif;}\n\
h3 { font-size: 90%; }\n\
.shadedBox {\n\
      padding: 2mm; margin-top: 3mm; margin-bottom: 3mm;\
      background-color: PapayaWhip;\n\
}\n\
    .NOTshadedBox {\n\
          padding: 2mm; margin-top: 3mm; margin-bottom: 3mm;\
          background-color: white;\n\
    }\n";

void directHref(FILE *opfd, int html, str *here, str *linksto, char *close)
{
    version = linksto->nodeversion;
    if( html )
        fprintf(opfd, "<a href=\"#%s\">", linksto->s);
    else
        myfprintf(opfd, "\\hyperlink{%s}{", linksto->s);
    if( showIDsOption ) myfprintf(opfd, "(%t) ", linksto->s);
    //myfprintf(opfd, html? "%s%s%d.%d %s": "{{%t%s%d.%d} %t",
      myfprintf(opfd,"%s%s%d.%d %s",
              *version? version: "", *version? "-": "",
                linksto->rankx, linksto->ranky, linksto->is == NULL? linksto->s: linksto->is->s);
    if( html )
        fprintf(opfd, "</a>%s", close);
    else
        fprintf(opfd, "%s", "}");
}

void href(FILE *opfd, int html, str *here, char *id, char *close)
{	//fprintf(stderr, "Got '%s' at offset %d\n", &s[2], id);

    // id may be in form
    // from: id
    // to: id
    // here: id
    // and embedded in blanks
    int from = 0, to = 0, this = 0, change = 1;

    //fprintf(stderr, "'%s'->",id);
    while( change )
    {   change = 0;
        while( *id == ' ' || *id == '\n' || *id == '\t' )
            id++;
        if( !strncmp(id, "from:", 5) ) { change = from = 1; id += 5; }
        if( !strncmp(id, "to:", 3) ) { change = to = 1; id += 3; }
        if( !strncmp(id, "here:", 5) ) { change = this = 1; id += 5; }
    }

	int found = 0;
    node *t;
	for( t = nodeList; t != NULL; t = t->next )
		if( !strcmp(t->s->s, id) )
		{	version = t->s->nodeversion;
            if( html )
            {   char *sp = "";
                fprintf(opfd, "<a href=\"#%s\">", t->s->s);
                if( from || to || this )
                {   fprintf(opfd, "<span style=\"border-style:solid;border-width:thin;font-family:sans-serif;\">");
                    if( from ) { fprintf(opfd, "FROM"); sp = " "; }
                    if( to ) { fprintf(opfd, "%sTO", sp); sp = " "; }
                    if( this ) fprintf(opfd, "%sHERE", sp);
                    fprintf(opfd, "</span> ");
                }
            }
            else{
                char *sp = "";
                myfprintf(opfd, "\\hyperlink{%s}{", t->s->s);
                if( from || to || this )
                {   fprintf(opfd, "\\raise 1pt \\hbox{\\fbox{\\sf\\tiny ");
                    if( from ) { fprintf(opfd, "FROM"); sp = " "; }
                    if( to ) { fprintf(opfd, "%sTO", sp); sp = " "; }
                    if( this ) fprintf(opfd, "%sHERE", sp);
                    fprintf(opfd, "}} ");
                }
            }
            if( showIDsOption ) myfprintf(opfd, "(%t) ", t->s->s);
            //myfprintf(opfd, html? "%s%s%d.%d %s": "{{%t%s%d.%d} %t",
              myfprintf(opfd,"%s%s%d.%d %s",
                      *version? version: "", *version? "-": "",
                        t->s->rankx, t->s->ranky, t->s->is == NULL? t->s->s: t->s->is->s);
            if( html )
                fprintf(opfd, "</a>%s", close);
            else
                fprintf(opfd, "%s", "}");
            found = 1;
            if( to ) saveCheckRtrans(here, here, t->s);
            if( from ) saveCheckRtrans(here, t->s, here);
            if( this && t->s != here )
            {   nolineerror("** Failed [[[here: %s]]] is not in node %s%s%N%s\n", here->s,
                            here->is != NULL? " (": "",
                            here->is != NULL? here->is->s: "",
                            here->is != NULL? ")": "");
            }
            break;
		}
	//fprintf(stderr, "Got '%s' at offset %d - found = %d\n", &s[2], offset, found); fflush(stderr);
	if( !found )
		nolineerror("No matching node found for cross-reference to '%s'", id);
    else
    if( !to && !from && !this ) // no from:/to:/here: label
    {   if( t->s == here )
            fprintf(stderr, "[[[%s]]] in node %s would be better as [[[here: %s]]]\n", id, here->s, id);
        else
        {   if( checkOneRtrans(here, t->s) ) fprintf(stderr, "[[[%s]]] in node %s would be better as [[[to: %s]]]\n", id, here->s, id);
            if( checkOneRtrans(t->s, here) ) fprintf(stderr, "[[[%s]]] in node %s would be better as [[[from: %s]]]\n", id, here->s, id);
        }
    }
}

/* ⚐	9872		WHITE FLAG
   ⚑	9873		BLACK FLAG */
void htmlflagcolor(FILE *opfd, int flag, int saycolor)
{	char *color = flagcolor(flag), *clearerColor = color;
    int iswhite = !strcmp(color, "white");
    if( !strcmp(color, "yellow") ) clearerColor = "gold";
    if( iswhite ) clearerColor = "black";
    if( saycolor )
        fprintf(opfd, "<span style='color:%s;'>%s</span>", iswhite? "black": clearerColor, color);
	fprintf(opfd, "<span style='color:%s;'>&#%s;</span>", clearerColor, iswhite? "9872": "9873");
}

void pullColorTitle(FILE *opfd, enum flagcolor pullStringEnum)
{   fprintf(opfd, "<table class=\"boxed\"><tr align=\"center\"><td><h1>Narrative restricted to ");
    htmlflagcolor(opfd, pullStringEnum, 1);
    fprintf(opfd, " highlighting");
    if( pullString == gray ) fprintf(opfd, " or not hightlighted");
    fprintf(opfd, "</h1></td></tr><tr><td>");
    HTMLtranslate(opfd, NULL, flagdefinitions[pullStringEnum]);
    fprintf(opfd, "</td></tr></table>\n");
}

void arrowTable(FILE *opfd, node *t, int allLinked)
{int anyarrows = 0;
    fprintf(opfd, "<table>\n<tr><td valign=\"top\">");
    arrow *a = arrowList, *b = arrowList;
    for( arrow *a = arrowList; a != NULL; a = a->next )
        if( a->v == t->s )
        {   if( !anyarrows ) fprintf(opfd, "<table>");
            anyarrows = 1;
            fprintf(opfd, "<tr><td>&larr;&nbsp;");
            if( a->u->flag != noflag )
            {   htmlflagcolor(opfd, a->u->flag, 0);
                myfprintf(opfd, " ");
            }
            else myfprintf(opfd, "&nbsp;&nbsp;&nbsp;");
            // maybe one day, don't put hrefs in if !pull and !allLinked (these nodes aren't in the diagram)
            directHref(opfd, 1, a->v, a->u, "</td></tr>");
        }
    if( anyarrows ) fprintf(opfd, "</table></td>\n");
    anyarrows = 0;
    for( arrow *b = arrowList; b != NULL; b = b->next )
        if( b->u == t->s )
        {    if( !anyarrows ) fprintf(opfd, "<td>&nbsp;&nbsp;&nbsp;</td><td valign=\"top\"><table>");
            anyarrows = 1;
            fprintf(opfd, "<tr><td>&rarr;&nbsp;");
            if( a->v->flag != noflag )
            {   htmlflagcolor(opfd, b->v->flag, 0);
                myfprintf(opfd, " ");
            }
            else myfprintf(opfd, "&nbsp;&nbsp;&nbsp;");
            directHref(opfd, 1, b->u, b->v, "</td></tr>");
        }
    if( anyarrows ) fprintf(opfd, "</table></td>\n");
    fprintf(opfd, "</tr>\n</table>\n");
    fprintf(opfd, "<h3>&uarr;&nbsp;<a href=\"#REEDabstract\">Back to top</a></h3>\n</div>\n");
}

void HTMLkeywords(FILE *opfd, struct keywordlist *keywords)
{   if( keywords != NULL )
    {   char *sep = "";
        fprintf(opfd, "<h3>Keyphrase%s: ", keywords->next == NULL? "": "s");
        for( struct keywordlist *t = keywords; t != NULL; t = t->next )
        {   myfprintf(opfd, "%s", sep);
            linkkeyword(opfd, t, t->keyword->s);
            myfprintf(opfd, "%s</a>", t->keyword->s);
            sep = "; ";
        }
        fprintf(opfd, ".</h3>\n");
    }
}

void pullAcolor(FILE *opfd, enum flagcolor pullString)
{   int count = 0;
    {   char *s = flagcolor(pullString);
        myfprintf(opfd, "\n<center><b>Note: %c%s highlighting ", toupper(*s), &s[1]);
        if( !*flagdefinitions[(int)pullString] )
            myfprintf(opfd, "is not defined!");
         else
             myfprintf(opfd, "means &ldquo;%s&rdquo;", flagdefinitions[(int)pullString] );
        myfprintf(opfd, "</b></center>\n");
    }
    for( node *t = nodeList; t != NULL; t = t->next )
        if( (t->s->flag == pullString || (t->s->flag == noflag && pullString == gray)) && t->s->note != NULL )
        {   count++;
            fprintf(opfd, "\n<a name=\"%s\"><br/></a>\n<div class=\"shadedBox\"><h2>", t->s->s);
            myfprintf(opfd, t->s->isgroup? "Group ": "Node ");
            (void) printrank(opfd, t->s, version);
            myfprintf(opfd, " %T",
                t->s->is != NULL? t->s->is->s: t->s->s);
            if( t->s->group != NULL )
                myfprintf(opfd, " <span style=\"font-weight:lighter;\"(group: %t)</span> ", t->s->group->is == NULL? t->s->group->s: t->s->group->is->s);
            if( showIDsOption ) myfprintf(opfd, "%t ", t->s->s);
            if( t->s->flag == noflag ) fprintf(opfd, " (not highlighted)");
            myfprintf(opfd, "</h2>\n");
            HTMLkeywords(opfd, t->s->keywords);
            HTMLtranslate(opfd, t->s, t->s->note->s);
            arrowTable(opfd, t, 0);
            myfprintf(opfd, "</div>\n");
        }
    if( !count )
        nolineerror("No notes highlighted in %s, so nothing to pull", flagcolor(pullString));
    if( pullPlusOption )
    {   myfprintf(stderr, "%s: ", flagcolor(pullString));
        if( strlen(flagdefinitions[pullString]) == 0 )
            fprintf(stderr, "NOT DEFINED.  Say: highlight %s is \"...\" to define %s.", flagcolor(pullString), flagcolor(pullString));
        else
            HTMLtranslate(stderr, NULL, flagdefinitions[pullString]);
    }
}

void HTMLcolorkey(FILE *opfd, char *heading, char *vskip)
{	int flagLegends = 0;
	int hasHeading = *heading;
	char *vbar = hasHeading? "|": "";
	char *hbar = hasHeading? "\\hline": "";
		
		// if there was a cascade, all the flagsused will be wrong, so fix them -- because we thought auxcascade() was the wrong place to do it :-)
		for( int flag = 0; flag < 8; flag++ )
			flagsusedaftercascades[flag] = 0;
		for( node *t = nodeList; t != NULL; t = t->next )
			flagsusedaftercascades[t->s->flag]++;
		
		for( int i = 1; i < 8; i++ ) // gets them in alphabetical order
		{	if( *flagdefinitions[i] )
			{	if( !flagLegends )
				{	myfprintf(opfd, "<table>\n");
					if( hasHeading ) myfprintf(opfd, "<tr><td colspan=3>%s</td></tr>\n", heading);
				}
				myfprintf(opfd, "<tr><td style=\"vertical-align:top\">");
				flagLegends++;
				htmlflagcolor(opfd, i, 0);
				fprintf(opfd, "</td><td style=\"vertical-align:top\">%s", flagcolors[i]);
				if( !flagsusedaftercascades[i] ) myfprintf(opfd, "&nbsp;not&nbsp;used</td><td>");
				else myfprintf(opfd, "&nbsp;used&nbsp;%d&nbsp;time%s</td><td>", flagsusedaftercascades[i], flagsusedaftercascades[i] == 1? "": "s");
				HTMLtranslate(opfd, NULL, flagdefinitions[i]);
				if( hasHeading && flagsusedaftercascades[i] != flagsused[i] )
				{	// %s&nbsp; , flagcolors[i] used&nbsp;explicitly 
					myfprintf(opfd, "<td/><td/><tr><td/><td>");
					if( flagsused[i] == 1 )
						myfprintf(opfd, "(once&nbsp;before&nbsp;cascading)", flagsused[i]);
					else
						myfprintf(opfd, "(%d&nbsp;times&nbsp;before&nbsp;cascading)", flagsused[i]);
					myfprintf(opfd, "</td></tr>\n");
				}
				myfprintf(opfd, "</td></tr>\n");
			}
		}
		if( flagLegends )
			myfprintf(opfd, "\n</table><p/>\n");
}

void wrapuplinkkeyword(FILE *opfd, struct keywordlist *t, char *debug)
{   // stupid coding! should have had a keyword symbol table and used it first... easy to fix but this is easier
    struct keywordlist *p;
    for( p = allkeywords; p != NULL; p = p-> next )
        if( !strcasecmp(p->keyword->s, t->keyword->s) )
            break;

    if( p == NULL ) { fprintf(stderr, "ooops! didn't find %s in keyword symbol table\n", t->keyword->s); exit(1); }

    // fprintf(stderr, "%s: <a href=\"#key-%d-%d\" name=\"key-%d-%d\">\n", debug, p->xkey, 1, p->xkey, p->ykey);
    fprintf(opfd, "<a href=\"#key-%d-%d\" name=\"key-%d-%d\">", p->xkey, 1, p->xkey, p->ykey);
    p->ykey++;
}

// generate the narrative HTML file
void htmlnotes(FILE *opfd, char *title, char *version, authorList *authors, char *date, char *abstract)
{ 	myfprintf(opfd, "<!DOCTYPE html>\n<html>\n<head>\n<title>%t%t%t%t</title>\n", title, 
			*title? "<br/>": "", *version? "Version ": "", version);
	fprintf(opfd, "<style>%s</style></head>\n", css);
    fprintf(opfd, "<body>\n%s\n", htmldefinitions->s);
	myfprintf(opfd, "<center><h1>%t</h1>", title);
    if( pullString != noflag )
        pullColorTitle(opfd, pullString);
    if( *version )
        myfprintf(opfd, "<h1>%t</h1>", version);
    myfprintf(opfd, "<h2>");
	while( authors != NULL )
	{	myfprintf(opfd, "%t", authors->author);
		if( authors->next != NULL ) 
			fprintf(opfd, authors->next->next == NULL? " \\& ": ", ");
		authors = authors->next;
	}

	fprintf(opfd, "</h2>\n<h3>%s</h3>", date);
    if( ispullingkeywords() )
    {
        htmlsaypullingkeyword(opfd);
    }
    else
        summarisekeywords(opfd);
    fprintf(opfd, "</center>\n");

	if( *abstract ) // && pullString == noflag )
	{	fprintf(opfd, "<a name=\"REEDabstract\"/><blockquote><div class=\"shadedBox\">");
		HTMLtranslate(opfd, NULL, abstract);
		fprintf(opfd, "</div></blockquote>\n");
	}

    fprintf(opfd, "\n%s\n", introduction->s);

    // sort notes into order using bubble sort
	int swapped = 0;
	do
	{	
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
			{	fprintf(stderr, "         - Node with %s highlight has no notes", flagcolor(t->s->flag));
			}
			myfprintf(stderr, "\n");
			nonotes = 1;
		} else anynotes++;
	}

    if( pullString != noflag ) // -pull used, so restrict to one color and exit
    {   pullAcolor(opfd, pullString);
        fprintf(opfd, "</body>\n</html>\n");
        return;
    }

	for( arrow *a = arrowList; a != NULL; a = a->next )
		anyarrows++;
	for( arrow *t = noteArrowList; t != NULL; t = t->next )
		anyarrownotes++;
	
	fprintf(opfd, "<h2>Quick overview &mdash; ");
	if( versionCount > 0 )
    {	fprintf(opfd, "%sersion%s: ",
                versionCount > 1? "combined v": "v", versionCount == 1? "": "s");
        allVersions(opfd);
    }
    else
		fprintf(opfd, "Single version");
	fprintf(opfd, "</h2>\n<p/>\n");

    HTMLprintfiledata(opfd);
	fprintf(opfd, "<p/>");
	
	fprintf(opfd, "<table>\n<tr><td style=\"text-align:right\">%d</td><td>node%s</td><td>&mdash;</td><td style=\"text-align:right\">%d</td><td>with notes</td>", anynodes, anynodes==1? "": "s", anynotes);
	fprintf(opfd, "<td>(%d highlighted)</td></tr></tr>\n", anyflags);
	fprintf(opfd, "<tr><td style=\"text-align:right\">%d</td><td>arrow%s</td><td>&mdash;</td><td style=\"text-align:right\">%d</td><td>with notes</td></tr>\n", 
		 anyarrows, anyarrows==1? "": "s", anyarrownotes);
	fprintf(opfd, "</table>\n");
	
	if( numberOfComponents > 1 )
	{	myfprintf(opfd, "<p>There are %d weakly connected components (i.e., there are %d independent REED diagrams not connected to each other with any arrows, regardless of the directions of the arrows).<p/>\n", numberOfComponents, numberOfComponents);
		myfprintf(opfd, "<ul style=\"list-style-type:none;\">");
		for( int c = 1; c <= numberOfComponents; c++ )
			myfprintf(opfd, "<li>&rarr; <a href=\"#component%d-narrative\">Narrative evidence for component %d</a></li>\n", c, c);
		myfprintf(opfd, "</ul>\n");
	}
		
	if( anyflags )
	{
		myfprintf(opfd, "\n<h2>%d highlighted node%s</h2>\n", anyflags, anyflags == 1? "": "s");
		
		HTMLcolorkey(opfd, "", "");
			
		myfprintf(opfd, "<table frame=\"box\">\n");
		for( int i = 1; i < 8; i++ ) // gets flags in alphabetical order
		{	for( node *t = nodeList; t != NULL; t = t->next )
				if( t->s->flag != noflag && t->s->flag == i )
				{	fprintf(opfd, "<tr><td>");
					htmlflagcolor(opfd, t->s->flag, 0);
					fprintf(opfd, "</td><td>");
                    directHref(opfd, 1, t->s, t->s, "</td>");
					//printrank(opfd, t->s, version);
					//myfprintf(opfd, "<td>%t</td><td>", t->s->is == NULL? t->s->s: t->s->is->s);
					fprintf(opfd, "<td>");
					if( t->s->cascade || flagcascade[i] )
						fprintf(opfd, "&mdash; Cascaded %s", flagcascade[i]? "color": "node");
					else 
					if( t->s->originalflag != noflag && t->s->originalflag != t->s->flag )
					{	fprintf(opfd, "&mdash; was ");
						htmlflagcolor(opfd, t->s->originalflag, 0);
						fprintf(opfd, " before cascade");
					}
					
					fprintf(opfd, "</td></tr>");
				}
		}
		fprintf(opfd, "\n</table>\n");
	} 
	
	if( 1 ) 
	{	int anynotes = 0;
		int component;
	
		// if there are lots of components, list nodes in order of component number
		for( component = 1; component <= numberOfComponents; component++ )
		{	anynotes = 0; // per component...
		for( node *t = nodeList; t != NULL; t = t->next )
			if( t->s->note != NULL && t->s->component == component && pullnode(t->s) )
            {	if( pullString != noflag && pullString != t->s->flag ) continue; //
                if( !anynotes )
				{	myfprintf(opfd, "<h1 style='text-decoration: underline'><a name=\"component%d-narrative\">Node narrative evidence", component);
					if( numberOfComponents > 1 )
						myfprintf(opfd, " for component %d", component);
					myfprintf(opfd, "</a></h1>");
					myfprintf(opfd, "<ul style=\"list-style-type:none;\">");
					for( int c = 1; c <= numberOfComponents; c++ )
						if( c != component )
							myfprintf(opfd, "<li>&rarr; <a href=\"#component%d-narrative\">All narrative for component %d</a></li>\n", c, c);
					myfprintf(opfd, "</ul>\n");
				}
				anynotes = 1;
				fprintf(opfd, "\n<a name=\"%s\"><br/></a>\n<div class=\"shadedBox\"><h2>", t->s->s);
				if( t->s->flag != noflag ) htmlflagcolor(opfd, t->s->flag, 0);

				myfprintf(opfd, " Node ");
				(void) printrank(opfd, t->s, version);
				myfprintf(opfd, " %T",
					t->s->is != NULL? t->s->is->s: t->s->s);
				
				if( t->s->group != NULL )
					myfprintf(opfd, " <span style=\"font-weight:lighter;\">(group: %t)</span> ", t->s->group->is == NULL? t->s->group->s: t->s->group->is->s);
				if( showIDsOption ) myfprintf(opfd, "%t ", t->s->s);
				myfprintf(opfd, "</h2>\n");

                HTMLkeywords(opfd, t->s->keywords);
				HTMLtranslate(opfd, t->s, t->s->note->s);

                arrowTable(opfd, t, 1);
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
    {   //fprintf(opfd, "[ %s -> %s ]\n", t->u->is->s, t->v->is->s);
        //if( pullnode(t->u) ) myfprintf(stderr, "%t ->\n", t->u->s);
        //if( pullnode(t->v) ) myfprintf(stderr, "<- %t\n", t->v->s);

        if( !pullnode(t->u) && !pullnode(t->v) ) continue;
        if( !arrownotes )
			myfprintf(opfd, "<h1 style='text-decoration: underline'>Arrow narrative evidence</h1>\n");
		arrownotes = 1;
		myfprintf(opfd, "<div class=\"shadedBox\"><h2>Arrow ");
		if( t->arrowis != NULL )
			myfprintf(opfd, "%t<br/>\n", t->arrowis->s);
		fprintf(opfd, "</h2>\n<table><tr><td/><td>");
        directHref(opfd, 1, t->u, t->u, "</td></tr>\n<tr><td>&rarr;</td><td>");
		//printrank(opfd, t->u, t->u->nodeversion);
		//myfprintf(opfd, " %t ", t->u->is != NULL? t->u->is->s: t->u->s);
		//myfprintf(opfd, "</td></tr>\n<tr><td>&rarr;</td><td>");
        directHref(opfd, 1, t->v, t->v, "</td></tr>\n</table><p/>");//printrank(opfd, t->v, t->v->nodeversion);
		//myfprintf(opfd, " %t</td></tr>\n</table><p/>", t->v->is != NULL? t->v->is->s: t->v->s);
        HTMLkeywords(opfd, t->keywords);
		HTMLtranslate(opfd, t->arrownote, t->arrownote->s);
        fprintf(opfd, "<h3>&uarr;&nbsp;<a href=\"#REEDabstract\">Back to top</a></h3>\n");
        fprintf(opfd, "</div>\n");
	}

	//if( !anyflags ) 
	//	myfprintf(opfd, "\n<h2>No highlighted nodes</h2>\n");
	
	if( nonotes ) 
		fprintf(opfd, "<h2>(no notes provided)</h2>\n");

    // wrap up keyword list to loop back to start...
    if( allkeywords != NULL )
    {   char *sep = "";
        int plural = allkeywords->next != NULL;
        fprintf(opfd, "<hr><blockquote><h2>");
             fprintf(opfd, "Jump back to first note with %s keyphrase:<br><span style=\"font-weight:normal;\">\n", plural? "any": "this");
        for( struct keywordlist *t = allkeywords; t != NULL; t = t->next )
        {   fprintf(opfd, "%s    ", sep);
            wrapuplinkkeyword(opfd, t, t->keyword->s);
            fprintf(opfd, "%s", t->keyword->s);
            fprintf(opfd, "</a>");
            sep = ";\n";
        }
        fprintf(opfd, ".</span></h2></blockquote>\n");
    }

    fprintf(opfd, "\n%s\n", conclusion->s);
	fprintf(opfd, "</body>\n</html>\n");
}
