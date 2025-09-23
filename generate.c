#include "header.h"
#include "notes.h"

char *flagstyle = "fillcolor=%s; style=filled; penwidth=2; shape=note; ";
char *defaultstyle = "fillcolor=\"gray90\"; style=filled; shape=ellipse; fontname=\"Helvetica\"; ";

rownodes *cols = NULL;
extern char *title, *date, *version, *abstract, *direction;
extern void summarizeMissingFlagDefinitions();
str *latexdefinitions, *htmldefinitions;

void xmlconverted(FILE *opfd, char *content)
{	for( char *s = content; *s; s++ )
	{	switch( *s )
		{	case '<':
				if( s[1] == '<' ) { myfprintf(opfd, "<link>"); s++; break; }
				myfprintf(opfd, "&lt;");
				break;
			case '>':
				if( s[1] == '>' ) { myfprintf(opfd, "</link>"); s++; break; }
				myfprintf(opfd, "&gt;");
				break;
			case '\'': myfprintf(opfd, "&apos;"); break;
			case '"': myfprintf(opfd, "&quot;"); break;
			case '&': myfprintf(opfd, "&amp;"); break;
			default: myfprintf(opfd, "%c", *s);
		}
	}
}

void xmlattribute(FILE *opfd, char *indent, char *tag, char *content)
{
    myfprintf(opfd, "%s<%s>\n    %s", indent, tag, indent);
    xmlconverted(opfd, content);
    myfprintf(opfd, "\n%s</%s>\n", indent, tag);
}

void xmlintattribute(FILE *opfd, char *indent, char *tag, int content)
{
    myfprintf(opfd, "%s<%s>\n    %s%d", indent, tag, indent, content);
     myfprintf(opfd, "\n%s</%s>\n", indent, tag);
}

void xmlCDATA(FILE *opfd, char *indent, char *content)
{
	myfprintf(opfd, "%s<%s><![CDATA[%s]]>\n", indent, content);
}

char skipblank(char **s)
{	while( **s && (isblank(**s) || **s == '\n') )
		(*s)++;
	return **s;
}

char copy(int quoted, char q, char **s)
{	while( **s && (quoted? (**s != q): (**s != '=' && **s != ',' && **s != ';')) )
		putchar(*(*s)++);
	if( quoted && **s == q ) (*s)++;
	else if( **s == '=' || **s == ',' || **s == ';' ) (*s)++;
	return **s;
}

void xmlstyle(char *style) // to convert Graphviz styles into XML xx
{	char *s = style;
	char c;
	int quoted;
	// label = "text" ,
	c = skipblank(&s);
	quoted = c == '"' || c == '\'';
	putchar('<');
	if( c == copy(quoted, c, &s) )
	{	quoted = c == '"' || c == '\'';
		copy(quoted, c, &s);
	;
	}
}

void xmlhighlight(FILE *opfd, char *indent, enum flagcolor flag, int cascade)
{	if( flag != noflag )
		fprintf(opfd, "%s<highlight color=\"%s\" cascade=\"%s\"/>\n", indent, flagcolor(flag), cascade? "true": "false");
}

void xml(FILE *opfd)
{	fprintf(opfd, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<reed>\n");

	if( *title ) xmlattribute(opfd, "", "title", title);
 	//if( *version ) myfprintf(opfd, "<version>%j</version>\n", version); // individual nodes have versions
	for( authorList *a = authors; a != NULL; a = a->next )
		xmlattribute(opfd, "", "author", a->author);
 	if( *date ) xmlattribute(opfd, "", "date", date);
 	if( *direction ) xmlattribute(opfd, "", "direction", direction);
	
	for( int i = 1; i < 8; i++ ) // gets them in alphabetical order
		{	if( *flagdefinitions[i] )
			{	myfprintf(opfd, "<highlightDefinition color=\"%s\">", flagcolors[i]);
				xmlconverted(opfd, flagdefinitions[i]); 
				myfprintf(opfd, "</highlightDefinition>\n");
			}
		}
	
	for( node *t = nodeList; t != NULL; t = t->next )
	{
		myfprintf(opfd, "<node id=\"%s\">\n", t->s->s);
			xmlattribute(opfd, "    ", "version", t->s->nodeversion);
			if(  t->s->is != NULL )
				xmlattribute(opfd, "    ", "label", t->s->is->s);
			xmlhighlight(opfd, "    ", t->s->flag,  t->s->cascade);
			if( t->s->note != NULL ) 
				xmlattribute(opfd, "    ", "note", t->s->note->s);
            xmlintattribute(opfd, "    ", "component", t->s->component);
			for( arrow *a = arrowList; a != NULL; a = a->next )
				if( a->u == t->s )
				{	myfprintf(opfd, "    <arrow to=\"%s\"", a->v->s);
					if( a->doublearrow ) myfprintf(opfd, " doubleArrow=\"true\"");
					myfprintf(opfd, ">\n");
					xmlattribute(opfd, "    ", "version", t->s->nodeversion);
					
					for( arrow *t = noteArrowList; t != NULL; t = t->next )
						if( t->u == a->u && t->v == a->v )
						{	if( t->arrowis != NULL && *t->arrowis->s )
                                xmlattribute(opfd, "        ", "label", t->arrowis->s);
							if( t->arrownote != NULL ) xmlattribute(opfd, "        ", "note", t->arrownote->s);
						}
					for( arrow *t = styledArrowList; t != NULL; t = t->next )
						if( t->u == a->u && t->v == a->v )
							if( t->arrowStyle != NULL )
							{
								xmlattribute(opfd, "    ", "style", t->arrowStyle->s);
								//xmlstyle(t->arrowStyle->s);
							}
					xmlhighlight(opfd, "        ", a->flag, 0);
					myfprintf(opfd, "    </arrow>\n");
				}
		myfprintf(opfd, "</node>\n\n");
	}
	fprintf(opfd, "</reed>\n");
}

int printrank(FILE *opfd, str *n, char *version) // for tex file
{	if( *n->nodeversion )
	{	//fprintf(stderr, "Version: %s\n", n->nodeversion);
		fprintf(opfd, "%s-", n->nodeversion);
	}
	if( n->noderef && *n->noderef )
	{	fprintf(opfd, "%s", n->noderef);
	}
	else
	{
		if( n->rankx )
		{	fprintf(opfd, "%d", n->rankx);
			if( n->ranky )
				fprintf(opfd, ".");
		}
		if( n->ranky )
			fprintf(opfd, "%d", n->ranky);
	}
	return *version || n->rankx || n->ranky;
}

void printNodeLabel(FILE *opfd, node *t, char *version)  // for dot file
{
	myfprintf(opfd, "URL=\"#%s\"; label=\"", t->s->s); 
	if( !t->s->plain )
	{	int print = printrank(opfd, t->s, version);
		if( showIDsOption ) 
		{ 	if( print ) myfprintf(opfd, " ");
			myfprintf(opfd, "(%j)", t->s->s);
		}
		if( print || showIDsOption )
			fprintf(opfd, "\\n");
	}
	else
	{	if( showIDsOption ) 
		{	if( !t->s->plain && *version ) fprintf(opfd, "-");
			myfprintf(opfd, "(%j)\\n", t->s->s);
		}
		else if( !t->s->plain && *version )
			myfprintf(opfd, "\n");
	}	
	myfprintf(opfd, "%S", t->s->is != NULL? t->s->is->s: t->s->s);
	if( flagTextOption )
		{	if( t->s->flag != noflag )
				myfprintf(opfd, "\n\nHighlighted %j", flagcolor(t->s->flag));
		}
	myfprintf(opfd, "\";");
}

void printColor(FILE *opfd, char *scheme, int index, int c)
{	// rdylgn3 - rdylgn11
	if( index <= 2 && c == 2 )
	{	index = 3;
		c = 3;
	}
	if( c > 0 )
		fprintf(opfd, "fillcolor=\"/%s%d/%d\";color=\"/%s%d/%d\";style=filled;fontcolor=%s;", 
			scheme, index < 3? 3: index > 11? 11: index, c,
			scheme, index < 3? 3: index > 11? 11: index, c,
			c == 1? "white": "black");
	// fprintf(stderr, "/%s%d/%d;\n", scheme, index < 3? 3: index > 11? 11: index, c);
}

int auxpullnode(str *n)
{   if( n == NULL ) return 0;
    return n->flag == pullString || (n->flag == noflag && pullString == gray);
}

int pullnode(str *n) // if -pull used, return true if color matches
{   if( pullString == noflag ) return 1;
    if( auxpullnode(n) ) return 1;
    return 0;
    // if this node is on either end of an arrow ending in this node the pull it
    for( arrow *t = arrowList; t != NULL; t = t->next )
        if( auxpullnode(t->u) && auxpullnode(t->v) )
            return 1;
    return 0;
}

void dot(FILE *opfd, char *title, char *version, char *date, char *direction)
{ 	fprintf(opfd, "digraph {\n  compound=true;\n  bgcolor=\"transparent\";\n  color=red;\n  labelloc=t;\n  fontname=\"Helvetica\";\n  fontsize=24;\n  ");
	myfprintf(opfd, "label=\"");
	if( *title ) myfprintf(opfd, "%j", title);
 	if( *version ) myfprintf(opfd, "\n%j", version);
 	if( *date )  myfprintf(opfd, "%s%j", *title || *version? ", ": "", date);
    if( pullString != noflag )
    {   char *s = flagcolor(pullString);
        myfprintf(opfd, ".  %c%s nodes only", toupper(*s), &s[1]);
    }
 	if( *title || *version || *date ) myfprintf(opfd, "\n "); // add a blank line after
 	myfprintf(opfd, "\";\n  rankdir=\"%s\";\n", direction);
 	
	int maxlength = 0;
	for( rownodes *row = cols; row != NULL; row = row->down )
	{	for( rownodes *col = row; col != NULL; col = col->right )
		{	if( col->label )
			{	//printf(" Pass 1: %s is the row label\n", col->node->is? col->node->is->s: col->node->s);
				int len = strlen(col->node->is? col->node->is->s: col->node->s);
				if( len > maxlength ) maxlength = len;
			}
		}
	}
	// pad out row names to maxlength+1
	maxlength = maxlength+1;
	int rowcounter = 1;
	for( rownodes *row = cols; row != NULL; row = row->down )
	{	myfprintf(opfd, "{rank=same; ");
		for( rownodes *col = row; col != NULL; col = col->right )
		{	if( col->label )
			{	// printf("  Pass 2: %s is the row label\n", col->node->is? col->node->is->s: col->node->s);
				strpad(col->node->is? &col->node->is: &col->node, maxlength);
				//newarrow(arrow **putonthisarrowlist, str *u, str *v, int doublearrow, int forceadd) 
				myfprintf(opfd, "\"%S\";", col->node->s);
				if( col->down != NULL )
				{	newarrow(&arrowList, col->node, col->down->node, 0, 1);
					defineArrowStyle(col->node, col->down->node, newstr("style=invis"));
				}
				if( col->node->style == NULL )
				{	col->node->plain = 1; // don't show version number etc
					col->node->style = newstr("shape=cds;fontname=\"Monaco\"");
				}
				else
					fprintf(stderr, "row label %s has own style which will not be overridden by standard style", col->node->is? col->node->is->s: col->node->s);
			} 
			else 
			{	if( rowsTOCstyled )
					col->node->rowDefaultNodeStyle = rowcounter;
				myfprintf(opfd, "\"%S\";", col->node->s);
			}			
		}
		rowcounter++;
		myfprintf(opfd, "}\n");
	}
	// the above generates this (where r1 etc are row labels) for each row arrow:
	// style (r1 r2 r3 r4 r5) is "shape=cds; fontname=\"Monaco\""
	// r1->r2->r3->r4->r5
	// style r1->r2->r3->r4->r5 is "style=invis"

	myfprintf(opfd, "\n");
		
	// do flag markers before any cluster --- to avoid a bug in Graphviz
	if( flagOption )
	{	int flagclustern = 1;
		for( node *t = nodeList; t != NULL; t = t->next )
			if( pullnode(t->s) && t->s->flag != noflag )
			{	enum flagcolor fc = t->s->flag;
				char *color = flagcolor(fc), *edgecolor = color, *fontcolor = "black";
				if( fc == white ) edgecolor = "black";
				if( fc == red || fc == blue || fc == black || fc == green ) fontcolor = "white";
				myfprintf(opfd, "subgraph \"clusterflag%d\" { \"%s\"; color=%s; bgcolor=%s; shape=circle; label=\"Flagged %s\"; labelloc=\"b\"; fontcolor=\"%s\"; fontsize=12; fontname=\"Helvetica\"; penwidth=2; };\n", flagclustern++, t->s->s, edgecolor, color, color, fontcolor);
			}
	}

	for( node *t = nodeList; t != NULL; t = t->next )
		if( pullnode(t->s) && t->s->isgroup &&
			!(flagOption && t->s->flag != noflag) // bug in graphviz means we can't have both
		)
		{	myfprintf(opfd, "subgraph \"cluster%S\" {\n   ", t->s->s);
			for( node *u = nodeList; u != NULL; u = u->next )
				if( pullnode(u->s) && u->s->group == t->s )
				{	t->s->exampleGroupMember = u->s;
					// myfprintf(opfd, //fprintf(opfd, "    %s [color=%s; style=filled];\n", u->s->s, u->s->flag? "pink": "white");
					fprintf(opfd, "\"%s\"; ", u->s->s);
				}
			if( t->s->is != NULL )
				myfprintf(opfd, "fontname=\"Helvetica-Bold\"; fontcolor=black; labelloc=b;  style=\"rounded\"; ", t->s->is->s);
			printNodeLabel(opfd, t, version);
			fprintf(opfd, "\n};\n\n");
		}
	//for( node *t = nodeList; t != NULL; t = t->next )
	//	printf("%s: group=%d, style=%d\n",t->s->s,t->s->isgroup,t->s->isstyle); 

	for( node *t = nodeList; t != NULL; t = t->next )
		if( pullnode(t->s) && !t->s->isgroup && !t->s->isstyle && t->s->l != HIGHLIGHT )
		{	myfprintf(opfd, "  \"%s%S\"", t->s->isgroup? "cluster": "", t->s->s);
			myfprintf(opfd, " [");
			if( !rowsTOCstyled && t->s->style == NULL ) // do not override an explicit style
				myfprintf(opfd, "%s", defaultstyle);
			if( rowsTOCstyled )
			{	printColor(opfd, "rdylgn", rowcounter, t->s->rowDefaultNodeStyle);
				myfprintf(opfd, "shape=box; fontname=Helvetica; ");
			}
			printNodeLabel(opfd, t, version);
			if( //(!rowsTOCstyled || t->s->plain) && 
				t->s->style != NULL )
			{	myfprintf(opfd, "%j;", t->s->style->s);
				//fprintf(stderr, "%s\n", t->s->style->s);
			}
			if( !rowsTOCstyled && t->s->flag != noflag ) // put last, flag style overrides other styles
			{	if( t->s->flag == white ) myfprintf(opfd, "color=black;");
				myfprintf(opfd, flagstyle, flagcolor(t->s->flag)); 
				if( t->s->flag == blue || t->s->flag == red || t->s->flag == black ) myfprintf(opfd, "fontcolor=white;");
			}
			fprintf(opfd, "];\n");
		}
	
	fprintf(opfd, "\n");
	for( arrow *t = arrowList; t != NULL; t = t->next )
	{	if( (t->u != NULL && pullnode(t->u)) && (t->v != NULL && pullnode(t->v)) && (t->u->isgroup || t->v->isgroup) )
		{	myfprintf(opfd, "  \"%S\"->\"%S\" [",
				t->u->exampleGroupMember == NULL? t->u->s: t->u->exampleGroupMember->s,
				t->v->exampleGroupMember == NULL? t->v->s: t->v->exampleGroupMember->s);
			if( t->v->isgroup ) myfprintf(opfd, "lhead=\"cluster%S\"", t->v->s);
			if( t->u->isgroup && t->v->isgroup ) myfprintf(opfd, "; ");
			if( t->doublearrow ) fprintf(opfd, " dir=both; ");

			for( arrow *a = styledArrowList; a != NULL; a = a->next )
			{	if( a->u == t->u && a->v == t->v )
				{	myfprintf(opfd,"%s; ", a->arrowStyle->s);
					//fprintf(stderr, "AA %s->%s, %s\n", t->u->s, t->v->s, flagcolors[a->flag]);
				}
			}
			if( t->flag != noflag )
			{	myfprintf(opfd, "color=%s;penwidth=4;", flagcolors[t->flag]);
				// arrow labels are not on top of the arrow, so we don't need to have a contrasting color
				//if( t->flag == blue || t->flag == red || t->flag == black ) myfprintf(opfd, "fontcolor=white;");	
			}
			//fprintf(stderr, "??? in dot(), arrow %s->%s is: %s\n", t->u->s, t->v->s, t->arrowis == NULL? "!!!!": t->arrowis->s);

			if( t->u->isgroup ) myfprintf(opfd, "ltail=\"cluster%S\"", t->u->s);

			if( t->arrowStyle != NULL ) // what am I doing
			{	fprintf(stderr, "  %s->%s ", 
				t->u->exampleGroupMember == NULL? t->u->s: t->u->exampleGroupMember->s,
				t->v->exampleGroupMember == NULL? t->v->s: t->v->exampleGroupMember->s);
				fprintf(stderr, " in group IS %s\n",  t->arrowis != NULL? t->arrowis->s: "");
			}
			fprintf(opfd, "]");
			fprintf(opfd, ";\n");
		}
		else
        if( pullnode(t->u) && pullnode(t->v) )
		{	myfprintf(opfd, "  \"%S\"->\"%S\"", t->u->s, t->v->s);
			char *openbra = " [", *closebra = "";
			for( arrow *a = styledArrowList; a != NULL; a = a->next )
			{	if( a->u == t->u && a->v == t->v )
				{	myfprintf(opfd,"%s%s; ", openbra, a->arrowStyle->s);
					openbra = "";
					closebra = "]";
				}
			}
			if( t->arrowStyle != NULL ) 
			{	printf("still to implement ***???\n");printf("arrowStyle = %s\n",t->arrowStyle->s);
				//printf("  %s->%s", t->u->s, t->v->s);
				printf(" free IS %s\n", t->arrowis != NULL? t->arrowis->s: "");
			}
			if( t->doublearrow ) 
			{	fprintf(opfd, "%sdir=both;", openbra);
				openbra = "";
				closebra = "]";
			}
			for( arrow *a = noteArrowList; a != NULL; a = a->next )
				if( t->u == a->u && t->v == a->v && a->arrowis != NULL )
				{	myfprintf(opfd, "%slabel=\"%t\";", openbra, a->arrowis->s);
					//fprintf(stderr, " - label %s\n", a->arrowis->s);
					//fprintf(stderr, "BB  %s->%s, %s\n", t->u->s, t->v->s, flagcolors[a->flag]);
					openbra = "";
					closebra = "]";
				}	
			if( t->flag != noflag )
			{	myfprintf(opfd, "%scolor=%s;penwidth=4;", openbra, flagcolors[t->flag]);	
				closebra = "]";
			}
			fprintf(opfd, "%s;\n", closebra);
		}
	}
	fprintf(opfd, "}\n");
}

void connectedComponents()
{
// int expanded, component;
	int current = 0;
	for( arrow *t = arrowList; t != NULL; t = t->next )
		if( !t->expanded && t->component == current )
		{
			t->expanded = 1;
			for( node *n = nodeList; n != NULL; n = n->next )
				if( t->u == n->s || t->v == n->s )
					n->s->color = current;
		}
}

void mathematica(FILE *opfd, char *title, char *version, authorList *authors, char *date, char *abstract)
{	// note use of %m in myfprintf instead of %s in fprintf - this makes %s mathematica-safe

	myfprintf(opfd, "Notebook[{\n");
	
	if( *title ) myfprintf(opfd, "Cell[BoxData[\"title=\\\"%m\\\";\"],\"Input\"],\n", title);

	myfprintf(opfd, "Cell[BoxData[\"authors=\\\"");
	while( authors != NULL )
	{	myfprintf(opfd, "%m", authors->author);
		if( authors->next != NULL ) 
			fprintf(opfd, ", ");
		authors = authors->next;
	}
	myfprintf(opfd, "\\\";\"],\"Input\"],\n");

    if( *version ) myfprintf(opfd, "Cell[BoxData[\"version=\\\"%m\\\";\"],\"Input\"],\n", version);
    if( *date ) myfprintf(opfd, "Cell[BoxData[\"date=\\\"%m\\\";\"],\"Input\"],\n", date);

	myfprintf(opfd, "Cell[BoxData[\"edges={");

	char *commarise = "", *ccommarise = "";
	for( arrow *t = arrowList; t != NULL; t = t->next )
	{	/* if( t->u->isgroup && t->v->isgroup)
			fprintf(stderr, "! ? ? ! ?\n");
		else if( t->u->isgroup )
		{	str *thegroup = t->u->group;
			for( node *u = nodeList; u != NULL; u = u->next )
			if( u->s != t->v && u->s->group == thegroup )
			{	myfprintf(stderr, "%s\n\"%m\" -> \"%m\" (* %t -> %t *)", commarise, u->s->s, t->v->s, u->s->is->s, t->v->is->s);
			}
		}
		else if( t->v->isgroup )
		{	str *thegroup = t->v->group;
			for( node *u = nodeList; u != NULL; u = u->next )
			if( u->s != t->u && u->s->group == thegroup )
			{	myfprintf(stderr, "%s\n\"%m\" -> \"%m\"", commarise, t->u->s, u->s->s);
			}
		}
		else 
		*/
		myfprintf(opfd, "%s\n   \\\"%m\\\" -> \\\"%m\\\"", commarise, t->u->s, t->v->s);
		if( t->doublearrow ) myfprintf(opfd, ", \\\"%m\\\" -> \\\"%m\\\"", t->v->s, t->u->s);
		commarise = ",";
	}	
	myfprintf(opfd, "\n};\"],\"Input\"],\nCell[BoxData[\"vertexNames={\n");

	commarise = "";
	for( node *t = nodeList; t != NULL; t = t->next )
		if( !t->s->isgroup && !t->s->isstyle && t->s->l == ID ) 
		{	myfprintf(opfd, "%s   \\\"%m\\\"->\\\"", commarise, t->s->s);
			if( showIDsOption ) myfprintf(opfd, "[%m] ", t->s->s);
			printrank(opfd, t->s, version);
			myfprintf(opfd, " %m\\\"", t->s->is != NULL? t->s->is->s: t->s->s); // was % ....
			commarise = ",\n";
		}

	myfprintf(opfd, "};\"],\"Input\"],\nCell[BoxData[\"keywords={\n");
    commarise = "";
    for( node *t = nodeList; t != NULL; t = t->next )
        if( !t->s->isgroup && !t->s->isstyle && t->s->l == ID )
        {   myfprintf(opfd, "%s   \\\"%m\\\"->{", commarise, t->s->s);
            char *sep = "";
            if( t->s->keywords != NULL )
            {
                for( arrow *tt = t->s->keywords; tt != NULL; tt = tt->next )
                {   myfprintf(opfd, "%s\\\"%s\\\"", sep, tt->u->s);
                    sep = ",\n";
                }
            }
            commarise = "},\n";
        }

	myfprintf(opfd, "}\n};\"],\"Input\"],Cell[BoxData[\"communities={");
	if(1 )
	{ccommarise = "";
	for( int c = 1; c <= numberOfComponents; c++ )
	{	fprintf(opfd, "%s{", ccommarise);
		ccommarise = ",\n  ";
		commarise = "";
		for( node *t = nodeList; t != NULL; t = t->next )
			if( t->s->component == c )
			{	myfprintf(opfd, "%s\\\"%m\\\"", commarise, t->s->s);
				commarise = ",";
			}
		fprintf(opfd, "}");
	}
	}
	myfprintf(opfd, "\n};\"],\"Input\"],\n");

    char *outercomma = "";
    myfprintf(opfd, "Cell[BoxData[\"groups={");
    for( node *t = nodeList; t != NULL; t = t->next )
        if( t->s->isgroup &&
            !(flagOption && t->s->flag != noflag) // bug in graphviz means we can't have both
        )
        {   char *comma = "";
            myfprintf(opfd, "%s{\\\"name\\\"->\\\"%S\\\", \\\"nodes\\\"->{", outercomma, t->s->s);
            outercomma = ",\n";
            for( node *u = nodeList; u != NULL; u = u->next )
                if( u->s->group == t->s )
                {    t->s->exampleGroupMember = u->s;
                    // myfprintf(opfd, //fprintf(opfd, "    %s [color=%s; style=filled];\n", u->s->s, u->s->flag? "pink": "white");
                    fprintf(opfd, "%s\\\"%s\\\"", comma, u->s->s);
                    comma = ", ";
                }
           myfprintf(opfd, "},\n\\\"label\\\"->\\\"");
            if( !t->s->plain )
            {   int print = printrank(opfd, t->s, version);
                if( showIDsOption )
                {     if( print ) myfprintf(opfd, " ");
                    myfprintf(opfd, "(%j)", t->s->s);
                }
                if( print || showIDsOption )
                    fprintf(opfd, "\\n");
            }
            else
            {    if( showIDsOption )
                {    if( !t->s->plain && *version ) fprintf(opfd, "-");
                    myfprintf(opfd, "(%j)\\n", t->s->s);
                }
                else if( !t->s->plain && *version )
                    myfprintf(opfd, "\n");
            }
            myfprintf(opfd, "%S", t->s->is != NULL? t->s->is->s: t->s->s);
            if( flagTextOption )
                {    if( t->s->flag != noflag )
                        myfprintf(opfd, "\n\nHighlighted %j", flagcolor(t->s->flag));
                }
            myfprintf(opfd, "\\\"}");
        }

    // myfprintf(opfd, "Cell[BoxData[\"title=\\\"%m\\\";\"],\"Input\"],\n", title);

     myfprintf(opfd, "};\"],\"Input\"],\nCell[BoxData[\"Print[Style[\\\"Defines\\\", Bold],\\\"\\\\ntitle=\\\", title, \\\"\\\\nauthors=\\\", authors, \\\"\\\\nversion=\\\", version, \\\"\\\\ndate=\\\", date, \\\"\\\\nand: edges, groups, communities, keywords, vertexNames\\\"]\"],\"Input\"]\n}\n]\n");
}
/*  ]

 */
void generateFiles(char *filename)
{
    // printf("Styles are:\n");
	// for( node *u = stylelist; u != NULL; u = u->next )
	//	printf("  %s = %s\n", u->s->style->s, u->s->s);

    checkIS();
    checkNumbering();

	// now everything collected, replace use of style nodes with the style values themselves
	for( node *t = nodeList; t != NULL; t = t->next )
    {	if( (t->s->rankx || t->s->ranky) && t->s->noderef && *t->s->noderef )
        {   fprintf(stderr, "Warning: node %s has references set by ref (%s) and numbering (", t->s->s, t->s->noderef);
            if( t->s->rankx ) fprintf(stderr, "%d", t->s->rankx);
            if( t->s->ranky ) fprintf(stderr, ".%d", t->s->ranky);
            fprintf(stderr, ")\n");
        }

        if( t->s->style != NULL && !t->s->style->isstyle )
        {	//printf("%s has style [%s] ", t->s->s, t->s->style->s);
            //printf(" .. and that istyle = %d\n", t->s->style->isstyle);
            for( node *u = stylelist; u != NULL; u = u->next )
            {	//printf("compare %s (node) ..with.. %s (style)\n", t->s->style->s, u->s->style->s);
                if(  t->s->style->s == u->s->style->s )
                {
                    //printf("  set %s in %s\n", t->s->style->s, t->s->s);
                    //printf("  to style %s\n**\n", u->s->s);
                    t->s->style = u->s;
                }
            }
        }
    }
	// DO THE SAME WITH ARROW STYLES...
	for( node *u = stylelist; u != NULL; u = u->next )
		for( arrow *styleda = styledArrowList; styleda != NULL; styleda = styleda->next )
			if(  u->s->style->s == styleda->arrowStyle->s )
				styleda->arrowStyle = u->s;
			
//	typedef struct tmparrow { str *u, *v; struct tmparrow *next; } arrow;
// 	typedef struct tmpnode { str *s; struct tmpnode *next; } node;

	if( numberingCount ) // not numbering a node is only an error if any nodes are numbered
		for( node *t = nodeList; t != NULL; t = t->next )
			if( t->s->l == ID )
			{	if( !t->s->plain && !t->s->ranky && !t->s->rankx && !t->s->noderef )  
					fprintf(stderr, "Warning: Numbering for %s '%s' not defined\n", t->s->isgroup? "group": "node", t->s->s);
				// fprintf(stderr, "Node %s @ %d.%d\n", t->s->s, t->s->rankx, t->s->ranky);
			}
			
	cascade();
    checkAllRtrans();
	summarizeMissingFlagDefinitions();

    if( 0 )
		for( node *t = nodeList; t != NULL; t = t->next )
			if( !t->s->pointsTo && !t->s->pointedFrom )
				fprintf(stderr, "Node %s is not on any arrow: %d %d\n", t->s->s, t->s->pointsTo, t->s->pointedFrom);
	
    stopiferror();

    if( verboseOption && graphvizOption )
        fprintf(stderr, "|--NB https://magjac.com/graphviz-visual-editor is a playground that can pinpoint Graphviz errors\n");

    // dot (graphviz) is very sensitive to node order, so we sort nodes before doing anything interesting
    int swapped = 0;
    do
    {    swapped = 0;
        for( node **t = &nodeList; (*t) != NULL && (*t)->next != NULL; t = &(*t)->next )
            if( strcmp((*t)->s->s, (*t)->next->s->s) < 0 )
            {    node *u = *t;
                *t = (*t)->next;
                u->next = (*t)->next;
                (*t)->next = u;
                swapped = 1;
            }
    } while( swapped );

    makefiles(filename);
}

extern int flagsusedaftercascades[];
void listColorsUsed() // list highlighting colors used to stdout
{   for( int flag = 0; flag < 8; flag++ )
        flagsusedaftercascades[flag] = 0;
    for( node *t = nodeList; t != NULL; t = t->next )
        flagsusedaftercascades[t->s->flag]++;
    char *spacing = "";
    for( int i = 0; i < 8; i++ ) // gets them in alphabetical order
        if( flagsusedaftercascades[i] )
        {   fprintf(stdout, "%s%s", spacing, !i? "gray": flagcolor(i));
            spacing = " ";
        }
    fprintf(stdout, "\n");
    if( colorsPlusOption )
    {   for( int i = 1; i < 8; i++ ) // gets them in alphabetical order
            if( strlen(flagdefinitions[i]) != 0 )
            {   fprintf(stdout, "\n%s: ", flagcolor(i));
                HTMLtranslate(stdout, flagdefinitions[i]);
                fprintf(stdout, "\n");
            }
    }
}
