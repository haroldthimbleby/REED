#include "header.h"
#include "notes.h"
extern char *flagdefinitions[], *flagcolors[];
extern int flagsused[], flagsusedaftercascades[];

extern int needSwap(str *u, str *v);
extern int needArrowSwap(arrow *u, arrow *v);
extern int versionCount; 
extern str *listOfVersions;
extern void HTMLprintfiledata(FILE *opfd);
extern void HTMLtranslate(FILE *opfd, char *note); // translate Latex and HTML to HTML, and include <<id>> notation

char *css = ".showCells { border:1px solid; border-collapse:collapse; }\n\
.showCells td  { border:1px solid; padding: 8px;  border-collapse:collapse; margin:0; }\n\
table {padding: 0; margin:0; }\n\
    .boxed {\n\
      border: 2.5px solid black; padding: 10px; margin: 10px;\n\
    }\n\
h1, h2, h3 { font-family: Arial, Helvetica, sans-serif;}\n\
h3 { font-size: 90%; }\n";

void href(FILE *opfd, char *id, char *close)
{	//fprintf(stderr, "Got '%s' at offset %d\n", &s[2], id);
	int found = 0;
	for( node *t = nodeList; t != NULL; t = t->next )
		if( !strcmp(t->s->s, id) )
		{	version = t->s->nodeversion;
			fprintf(opfd, "<a href=\"#%s\">", t->s->s);
			if( showIDsOption ) myfprintf(opfd, "(%t) ", t->s->s);
			fprintf(opfd, "%s%s%d.%d %s %s",
				*version? version: "", *version? "-": "",
				t->s->rankx, t->s->ranky, t->s->is == NULL? t->s->s: t->s->is->s, close);
			found = 1;
			break;
		}
	//fprintf(stderr, "Got '%s' at offset %d - found = %d\n", &s[2], offset, found); fflush(stderr);
	if( !found )
		nolineerror("no matching node found for '%s'", id);
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

void pullColorTitle(FILE *opfd, enum flagcolor pullString)
{   fprintf(opfd, "<table class=\"boxed\"><tr align=\"center\"><td><h1>Narrative restricted to ");
    htmlflagcolor(opfd, pullString, 1);
    fprintf(opfd, " highlighting");
    if( pullString == gray ) fprintf(opfd, " or not hightlighted");
    fprintf(opfd, "</h1></td></tr><tr><td>");
    HTMLtranslate(opfd, flagdefinitions[pullString]);
    fprintf(opfd, "</td></tr></table>\n");
}

void pullAcolor(FILE *opfd, enum flagcolor pullString)
{   int count = 0;
    for( node *t = nodeList; t != NULL; t = t->next )
        if( (t->s->flag == pullString || (t->s->flag == noflag && pullString == gray)) && t->s->note != NULL )
        {   count++;
            fprintf(opfd, "\n<a name=\"%s\"><h2>", t->s->s);
            myfprintf(opfd, t->s->isgroup? "Group ": "Node ");
            (void) printrank(opfd, t->s, version);
            myfprintf(opfd, " %T",
                t->s->is != NULL? t->s->is->s: t->s->s);
            if( t->s->group != NULL )
                myfprintf(opfd, " &mdash; in group: %t ", t->s->group->is == NULL? t->s->group->s: t->s->group->is->s);
            if( showIDsOption ) myfprintf(opfd, "%t ", t->s->s);
            if( t->s->flag == noflag ) fprintf(opfd, " (not highlighted)");
            myfprintf(opfd, "</h2></a>\n");
            HTMLtranslate(opfd, t->s->note->s);
        }
    if( !count )
        nolineerror("No notes highlighted in %s, so nothing to pull", flagcolor(pullString));
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
				HTMLtranslate(opfd, flagdefinitions[i]); 
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

	fprintf(opfd, "</h2>\n<h3>%s</h3></center>\n", date);

	if( *abstract && pullString == noflag )
	{	fprintf(opfd, "<a name=\"REEDabstract\"/><blockquote>");
		HTMLtranslate(opfd, abstract);
		fprintf(opfd, "</blockquote>\n");
	}
		
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
		fprintf(opfd, "%sersion%s: %s", 
			versionCount > 1? "combined v": "v", versionCount == 1? "": "s", listOfVersions->s);
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
					href(opfd, t->s->s, "</td>");
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
			if( t->s->note != NULL && t->s->component == component ) 
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
				fprintf(opfd, "\n<a name=\"%s\"><h2>", t->s->s);
				if( t->s->flag != noflag ) htmlflagcolor(opfd, t->s->flag, 0);

				myfprintf(opfd, " Node ");
				(void) printrank(opfd, t->s, version);
				myfprintf(opfd, " %T",
					t->s->is != NULL? t->s->is->s: t->s->s);
				
				if( t->s->group != NULL )
					myfprintf(opfd, " &mdash; Group: %t ", t->s->group->is == NULL? t->s->group->s: t->s->group->is->s);
				if( showIDsOption ) myfprintf(opfd, "%t ", t->s->s);
				myfprintf(opfd, "</h2></a>\n");
				HTMLtranslate(opfd, t->s->note->s);
				
				int anyarrows = 0;
				fprintf(opfd, "<table>\n<tr><td valign=\"top\">");
				arrow *a = arrowList, *b = arrowList;
				for( arrow *a = arrowList; a != NULL; a = a->next )
					if( a->v == t->s )
					{	if( !anyarrows ) fprintf(opfd, "<table>");
						anyarrows = 1;
						fprintf(opfd, "<tr><td>&larr;&nbsp;");
						if( a->u->flag != noflag ) 
						{	htmlflagcolor(opfd, a->u->flag, 0);
							myfprintf(opfd, " ");
						}
						else myfprintf(opfd, "&nbsp;&nbsp;&nbsp;");
						href(opfd, a->u->s, "</a></td></tr>");
					}
				if( anyarrows ) fprintf(opfd, "</table></td>\n");
				anyarrows = 0;
				for( arrow *b = arrowList; b != NULL; b = b->next )
					if( b->u == t->s )
					{	if( !anyarrows ) fprintf(opfd, "<td>&nbsp;&nbsp;&nbsp;</td><td valign=\"top\"><table>");
						anyarrows = 1;
						fprintf(opfd, "<tr><td>&rarr;&nbsp;");
						if( a->v->flag != noflag ) 
						{	htmlflagcolor(opfd, b->v->flag, 0);
							myfprintf(opfd, " ");
						}
						else myfprintf(opfd, "&nbsp;&nbsp;&nbsp;");
						href(opfd, b->v->s, "</a></td></tr>");
					}
				if( anyarrows ) fprintf(opfd, "</table></td>\n");
				fprintf(opfd, "</tr>\n</table>\n");
                fprintf(opfd, "<h3>&uarr;&nbsp;<a href=\"#REEDabstract\">Back to top</a></h3>");
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
			myfprintf(opfd, "<h1 style='text-decoration: underline'>Arrow narrative evidence</h1>\n");
		arrownotes = 1;
		myfprintf(opfd,"<h2>Arrow ");
		if( t->arrowis != NULL )
			myfprintf(opfd, "%t<br/>\n", t->arrowis->s);
		fprintf(opfd, "</h2>\n<table><tr><td/><td>");
		href(opfd, t->u->s, "</td></tr>\n<tr><td>&rarr;</td><td>");
		//printrank(opfd, t->u, t->u->nodeversion); 
		//myfprintf(opfd, " %t ", t->u->is != NULL? t->u->is->s: t->u->s);
		//myfprintf(opfd, "</td></tr>\n<tr><td>&rarr;</td><td>");
		href(opfd, t->v->s, "</td></tr>\n</table><p/>");//printrank(opfd, t->v, t->v->nodeversion); 
		//myfprintf(opfd, " %t</td></tr>\n</table><p/>", t->v->is != NULL? t->v->is->s: t->v->s);
		HTMLtranslate(opfd, t->arrownote->s);
	}

	//if( !anyflags ) 
	//	myfprintf(opfd, "\n<h2>No highlighted nodes</h2>\n");
	
	if( nonotes ) 
	{	fprintf(opfd, "<h1>No notes provided</h1>\n");
		for( node *t = nodeList; t != NULL; t = t->next )
			if( t->s->note == NULL )
			{	myfprintf(opfd, "<h2>Node ");
				printrank(opfd, t->s, version);
				myfprintf(opfd, " %t", t->s->is != NULL? t->s->is->s: t->s->s);
				if( t->s->is != NULL )
					myfprintf(opfd, " [%t]", t->s->s);
				fprintf(opfd, "<h2/>");
			} 
	}
	
	fprintf(opfd, "</body>\n</html>\n");
}
