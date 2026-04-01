#include "header.h"
#include "notes.h"
#include "evalstyle.h"

char *flagstyle = "fillcolor=%s; style=filled; penwidth=2; shape=note; ";

int darkcolor(int fc)
{   return fc == purple || fc == navy || fc == maroon || fc == red || fc == blue || fc == black || fc == green || fc == gray || fc == teal || fc == olive;
}

rownodes *cols = NULL;
extern char *title, *date, *version, *abstract, *direction, *defaultStyle;
extern void summarizeMissingFlagDefinitions();
str *latexdefinitions, *latexendoffile, *htmldefinitions, *introduction, *conclusion;

char *cyclicStyle = "peripheries=3;fontcolor=\"white\";color=\"firebrick1\",fillcolor=\"fireBrick1\"; style=\"filled,dashed\";";

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

void xmlstyle(char *style) // to convert Graphviz styles into XML
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
	
    for( int i = 1; i < nflagcolors; i++ ) // gets them in alphabetical order
		{	if( *flagdefinitions[i] )
			{	myfprintf(opfd, "<highlightDefinition color=\"%s\">", flagcolors[i]);
				xmlconverted(opfd, flagdefinitions[i]); 
				myfprintf(opfd, "</highlightDefinition>\n");
			}
		}
	
	for( node *t = nodeList; t != NULL; t = t->next )
	{
		myfprintf(opfd, "<node id=\"%s\">\n", t->strp->s);
        xmlattribute(opfd, "    ", "version", t->strp->nodeversion);
        if(  t->strp->is != NULL )
            xmlattribute(opfd, "    ", "label", t->strp->is->s);
        xmlhighlight(opfd, "    ", t->strp->flag,  t->strp->cascade);
        if( t->strp->note != NULL )
            xmlattribute(opfd, "    ", "note", t->strp->note->s);
        xmlintattribute(opfd, "    ", "component", t->strp->component);

        if( t->strp->metadata != NULL )
        {   fprintf(opfd, "    <metadata>\n");
            for( metadataList *ml = t->strp->metadata; ml != NULL; ml = ml->next )
                    fprintf(opfd, "        <property name=\"%s\" value=\"%s\"/>\n", ml->property, ml->value);
            fprintf(opfd, "    </metadata>\n");
        }

			for( arrow *a = arrowList; a != NULL; a = a->next )
				if( a->u == t->strp )
				{	myfprintf(opfd, "    <arrow to=\"%s\"", a->v->s);
					myfprintf(opfd, ">\n");
					xmlattribute(opfd, "    ", "version", t->strp->nodeversion);

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
{	char *dash = ""; // insert a dash if version AND (node ref OR ranks)
    if( *n->nodeversion )
	{	//fprintf(stderr, "Version: %s\n", n->nodeversion);
		fprintf(opfd, "%s", n->nodeversion);
        dash = "-";
	}
	if( n->noderef && *n->noderef )
	{	fprintf(opfd, "%s%s", dash, n->noderef);
	}
	else
	{
		if( n->rankx )
		{	fprintf(opfd, "%s%d", dash, n->rankx);
            dash = "";
			if( n->ranky )
				fprintf(opfd, ".");
		}
		if( n->ranky )
			fprintf(opfd, "%s%d", dash, n->ranky);
	}
	return *version || n->rankx || n->ranky;
}

void printNodeLabel(FILE *opfd, node *t, char *version)  // for dot file
{
	myfprintf(opfd, "URL=\"#%s\"; label=\"", t->strp->s);
	if( !t->strp->plain )
	{	int print = printrank(opfd, t->strp, version);
		if( showIDsOption )
		{ 	if( print ) myfprintf(opfd, " ");
			myfprintf(opfd, "(%j)", t->strp->s);
		}
		if( print || showIDsOption )
			fprintf(opfd, "\\n");
	}
	else
	{	if( showIDsOption ) 
		{	if( !t->strp->plain && *version ) fprintf(opfd, "-");
			myfprintf(opfd, "(%j)\\n", t->strp->s);
		}
		else if( !t->strp->plain && *version )
			myfprintf(opfd, "\n");
	}	
	myfprintf(opfd, "%S", t->strp->is != NULL? t->strp->is->s: t->strp->s);
	if( flagTextOption )
		{	if( t->strp->flag != noflag )
				myfprintf(opfd, "\n\nHighlighted %j", flagcolor(t->strp->flag));
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

extern void checkMemory(char *file, int line)
{   return;
    if( strcmp(file, "generate.c") ) return;

    // check arrows are OK
    fprintf(stderr, "--?? checkarrowlist %s %d --\n", file, line); fflush(stderr);
    for( arrow *t = arrowList; t != NULL; t = t->next )
    {   fprintf(stderr, "%%t=%p t->flag=%d\n", t, t->flag); fflush(stderr);
        fprintf(stderr, "%%flagcolors[%d]=%s\n", t->flag, flagcolors[t->flag]); fflush(stderr);
    }
    fprintf(stderr, "--DONE checkarrowlist %s %d --\n", file, line); fflush(stderr);
}

void dot(FILE *opfd, char *title, char *version, char *date, char *direction)
{ 	fprintf(opfd, "digraph {\n  compound=true;\n  bgcolor=\"transparent\";\n  color=red;\n  labelloc=t;\n  fontname=\"Helvetica\";\n  fontsize=24;\n  ");
    if( *defaultStyle )
        fprintf(opfd, "graph [%s];\nnode [%s];\nedge [%s];\n", defaultStyle, defaultStyle, defaultStyle);
    myfprintf(opfd, "label=\"");
	if( *title ) myfprintf(opfd, "%j", title);
 	if( *version ) myfprintf(opfd, "\n%j", version);
 	if( *date ) myfprintf(opfd, "%s%j", *title || *version? ", ": "", date);
    if( pullString != noflag )
    {   char *s = flagcolor(pullString);
        myfprintf(opfd, ".  %c%s nodes only\n", toupper(*s), &s[1]);
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
				myfprintf(opfd, "\"%S\";", col->node->s);
				if( col->down != NULL )
				{	newarrow(&arrowList, &col->node, &col->down->node, 1);
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
			if( pullnode(t->strp) && t->strp->flag != noflag )
			{	enum flagcolor fc = t->strp->flag;
				char *color = flagcolor(fc), *edgecolor = color, *fontcolor = "black";
				if( fc == white ) edgecolor = "black";
				if( darkcolor(fc) )
                    fontcolor = "white";
				myfprintf(opfd, "subgraph \"clusterflag%d\" { \"%s\"; color=%s; bgcolor=%s; shape=circle; label=\"Flagged %s\"; labelloc=\"b\"; fontcolor=\"%s\"; fontsize=12; fontname=\"Helvetica\"; penwidth=2; };\n", flagclustern++, t->strp->s, edgecolor, color, color, fontcolor);
			}
	}

	for( node *t = nodeList; t != NULL; t = t->next )
		if( pullnode(t->strp) && t->strp->isgroup &&
			!(flagOption && t->strp->flag != noflag) // bug in graphviz means we can't have both
		)
		{	myfprintf(opfd, "subgraph \"cluster%S\" {\n   ", t->strp->s);
			for( node *u = nodeList; u != NULL; u = u->next )
				if( pullnode(u->strp) && u->strp->group == t->strp )
				{	t->strp->exampleGroupMember = u->strp;
					// myfprintf(opfd, //fprintf(opfd, "    %s [color=%s; style=filled];\n", u->s->s, u->s->flag? "pink": "white");
					fprintf(opfd, "\"%s\"; ", u->strp->s);
				}
			if( t->strp->is != NULL )
				myfprintf(opfd, "fontname=\"Helvetica-Bold\"; fontcolor=black; labelloc=b;  style=\"rounded\"; ", t->strp->is->s);
			printNodeLabel(opfd, t, version);
			fprintf(opfd, "\n};\n\n");
		}

	//for( node *t = nodeList; t != NULL; t = t->next )
	//	printf("%s: group=%d, style=%d\n",t->s->s,t->s->isgroup,t->s->isstyle); 

	for( node *t = nodeList; t != NULL; t = t->next )
		if( pullnode(t->strp) && !t->strp->isgroup && !t->strp->isstyle && t->strp->l != HIGHLIGHT )
		{	myfprintf(opfd, "  \"%s%S\"", t->strp->isgroup? "cluster": "", t->strp->s);
			myfprintf(opfd, " [");
		//	if( !rowsTOCstyled && t->s->style == NULL ) // do not override an explicit style
			//	myfprintf(opfd, "%s", defaultstyle);
			if( rowsTOCstyled )
			{	printColor(opfd, "rdylgn", rowcounter, t->strp->rowDefaultNodeStyle);
				myfprintf(opfd, "shape=box; fontname=Helvetica; ");
			}
			printNodeLabel(opfd, t, version);
			if( //(!rowsTOCstyled || t->s->plain) && 
				t->strp->style != NULL )
			{	myfprintf(opfd, "%j;", t->strp->style->s);
				//fprintf(stderr, "%s\n", t->s->style->s);
			}
			if( !rowsTOCstyled && t->strp->flag != noflag ) // put last, flag style overrides other styles
			{	if( t->strp->flag == white ) myfprintf(opfd, "color=black;");
				myfprintf(opfd, flagstyle, flagcolor(t->strp->flag));
				if( darkcolor(t->strp->flag))
                    myfprintf(opfd, "fontcolor=\"white\";");
			}
            if( t->strp->cyclic )
                myfprintf(opfd, cyclicStyle);
            if( !t->strp->visible )
                myfprintf(opfd, "style=invis;");
            fprintf(opfd, "];\n");
		}
	
	fprintf(opfd, "\n");
	for( arrow *t = arrowList; t != NULL; t = t->next )
	{	//if (!t || !t->u || !t->v) continue;  // early guard
        //fprintf(stderr, "%%t=%p t->flag=%d\n", t, t->flag); fflush(stderr);
        //fprintf(stderr, "%%flagcolors[%d]=\n", t->flag); fflush(stderr);
        //fprintf(stderr, "%s\n", flagcolors[t->flag]); fflush(stderr);
        if (!t || !t->u || !t->v || !(t->u) || !(t->v)) {
            fprintf(stderr, "**** Invalid node pointer in arrowList\n");
            continue;
        }
        if( (t->u != NULL && pullnode(t->u)) && (t->v != NULL && pullnode(t->v)) && (t->u->isgroup || t->v->isgroup) )
		{	myfprintf(opfd, "  \"%S\"->\"%S\" [",
				t->u->exampleGroupMember == NULL? t->u->s: t->u->exampleGroupMember->s,
				t->v->exampleGroupMember == NULL? t->v->s: t->v->exampleGroupMember->s);
			if( t->v->isgroup ) myfprintf(opfd, "lhead=\"cluster%S\"", t->v->s);
			if( t->u->isgroup && t->v->isgroup ) myfprintf(opfd, "; ");

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

            if( !t->u->visible || !t->v->visible )
                myfprintf(opfd, "color=invis;");

            fprintf(opfd, "];\n");
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
			{	printf("*** still to implement arrowstylesw ***???\n");
                //printf("arrowStyle = %s\n",t->arrowStyle->s);
				//printf("  %s->%s", t->u->s, t->v->s);
				//printf(" free IS %s\n", t->arrowis != NULL? t->arrowis->s: "");
			}
			for( arrow *a = noteArrowList; a != NULL; a = a->next )
				if( t->u == a->u && t->v == a->v && a->arrowis != NULL )
				{	myfprintf(opfd, "%slabel=\"%t\";", openbra, a->arrowis->s);
					//fprintf(stderr, " - label %s\n", a->arrowis->s);
					//fprintf(stderr, "BB  %s->%s, %s\n", t->u->s, t->v->s, flagcolors[a->flag]);
					openbra = "";
					closebra = "]";
				}
           // fprintf(stderr, "arrow %p u=%p v=%p\n", t, t->u, t->v); fflush(stderr);
           // fprintf(stderr, "arrow visibles %p u=%d v=%d\n", t, t->u->visible, t->v->visible); fflush(stderr);
            if( t->flag != noflag )
            {   //fprintf(stderr, "openbra=%s t=%p t->flag=%d\n", openbra, t, t->flag); fflush(stderr);
                //fprintf(stderr, "flagcolors[%d]=%s\n", t->flag, flagcolors[t->flag]); fflush(stderr);
                //fprintf(stderr, ".... %scolor=%s;penwidth=4;", openbra, flagcolors[t->flag]); fflush(stderr);
                fprintf(opfd, "%scolor=%s;penwidth=4;", openbra, flagcolors[t->flag]);
                closebra = "]";
                openbra = "";
            }
            if( !t->u->visible || !t->v->visible )
            {   myfprintf(opfd, "%sstyle=invis;", openbra);
                closebra = "]";
            }
			fprintf(opfd, "%s;\n", closebra);
		}
	}
	fprintf(opfd, "}\n");
}

void connectedComponents()
{   int current = 0;
	for( arrow *t = arrowList; t != NULL; t = t->next )
		if( !t->expanded && t->component == current )
        {   t->expanded = 1;
			for( node *n = nodeList; n != NULL; n = n->next )
				if( t->u == n->strp || t->v == n->strp )
					n->strp->color = current;
		}
}

void mathematica(FILE *opfd, char *title, char *version, authorList *authors, char *date, char *abstract)
{	// note use of %m in myfprintf instead of %s in fprintf - this makes %s mathematica-safe

    //myfprintf(opfd, "Notebook[{Cell[BoxData[");

    myfprintf(opfd, "title=\"%m\";\n", title);

    myfprintf(opfd, "authors=\"");
    while( authors != NULL )
    {	myfprintf(opfd, "%m", authors->author);
        if( authors->next != NULL )
            fprintf(opfd, ", ");
        authors = authors->next;
    }
    myfprintf(opfd, "\";\n");

    myfprintf(opfd, "version=\"%m\";\n", version);
    myfprintf(opfd, "date=\"%m\";\n", date);

    myfprintf(opfd, "edges={");
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
        myfprintf(opfd, "%s\n   \"%m\" -> \"%m\"", commarise, t->u->s, t->v->s);
        commarise = ",";
    }
    myfprintf(opfd, "\n};\n");

    myfprintf(opfd, "vertexNames={\n");
    commarise = "";
    for( node *t = nodeList; t != NULL; t = t->next )
        if( !t->strp->isgroup && !t->strp->isstyle && t->strp->l == ID )
        {	myfprintf(opfd, "%s   \"%m\"->\"", commarise, t->strp->s);
            if( showIDsOption ) myfprintf(opfd, "[%m] ", t->strp->s);
            printrank(opfd, t->strp, version);
            myfprintf(opfd, " %m\"", t->strp->is != NULL? t->strp->is->s: t->strp->s); // was % ....
            commarise = ",\n";
        }
    myfprintf(opfd, "};\n");

    myfprintf(opfd, "keywords={\n");
    commarise = "";
    for( node *t = nodeList; t != NULL; t = t->next )
        if( !t->strp->isgroup && !t->strp->isstyle && t->strp->l == ID )
        {   myfprintf(opfd, "%s   \"%m\"->{", commarise, t->strp->s);
            char *sep = "";
            if( t->strp->keywords != NULL )
            {
                for( struct keywordlist *tt = t->strp->keywords; tt != NULL; tt = tt->next )
                {   myfprintf(opfd, "%s\"%s\"", sep, tt->keyword->s);
                    sep = ", ";
                }
            }
            commarise = "},\n";
        }

    myfprintf(opfd, "}};\n");

    myfprintf(opfd, "notes={\n");
    commarise = "";
    for( node *t = nodeList; t != NULL; t = t->next )
        if( !t->strp->isgroup && !t->strp->isstyle && t->strp->l == ID )
        {   myfprintf(opfd, "%s   \"%m\"->\"%m\"", commarise, t->strp->s, t->strp->note != NULL? t->strp->note->s: "");
            commarise = ",\n";
        }
    myfprintf(opfd, "\n};\n");

    myfprintf(opfd, "highlights={\n");
    commarise = "";
    for( node *t = nodeList; t != NULL; t = t->next )
    {   myfprintf(opfd, "%s\"%s\"->\"%s\"", commarise, t->strp->s,  flagcolor(t->strp->flag));
        commarise = ",\n";
    }
    myfprintf(opfd,"};\n");

    myfprintf(opfd, "communities={");
    if(1 )
    {ccommarise = "";
        for( int c = 1; c <= numberOfComponents; c++ )
        {	fprintf(opfd, "%s{", ccommarise);
            ccommarise = ",\n  ";
            commarise = "";
            for( node *t = nodeList; t != NULL; t = t->next )
                if( t->strp->component == c )
                {	myfprintf(opfd, "%s\"%m\"", commarise, t->strp->s);
                    commarise = ",";
                }
            fprintf(opfd, "}");
        }
    }
    myfprintf(opfd, "\n};\n");

    char *outercomma = "";
    myfprintf(opfd, "groups={");
    for( node *t = nodeList; t != NULL; t = t->next )
        if( t->strp->isgroup &&
           !(flagOption && t->strp->flag != noflag) // bug in graphviz means we can't have both
           )
        {   char *comma = "";
            myfprintf(opfd, "%s{\"name\"->\"%S\", \"nodes\"->{", outercomma, t->strp->s);
            outercomma = ",\n";
            for( node *u = nodeList; u != NULL; u = u->next )
                if( u->strp->group == t->strp )
                {    t->strp->exampleGroupMember = u->strp;
                    // myfprintf(opfd, //fprintf(opfd, "    %s [color=%s; style=filled];\n", u->s->s, u->s->flag? "pink": "white");
                    fprintf(opfd, "%s\"%s\"", comma, u->strp->s);
                    comma = ", ";
                }
            myfprintf(opfd, "},\n\"label\"->\"");
            if( !t->strp->plain )
            {   int print = printrank(opfd, t->strp, version);
                if( showIDsOption )
                {     if( print ) myfprintf(opfd, " ");
                    myfprintf(opfd, "(%j)", t->strp->s);
                }
                if( print || showIDsOption )
                    fprintf(opfd, "\\n");
            }
            else
            {    if( showIDsOption )
            {   if( !t->strp->plain && *version ) fprintf(opfd, "-");
                myfprintf(opfd, "(%j)\\n", t->strp->s);
            }
            else if( !t->strp->plain && *version )
                myfprintf(opfd, "\n");
            }
            myfprintf(opfd, "%S", t->strp->is != NULL? t->strp->is->s: t->strp->s);
            if( flagTextOption )
            {    if( t->strp->flag != noflag )
                myfprintf(opfd, "\n\nHighlighted %j", flagcolor(t->strp->flag));
            }
            myfprintf(opfd, "\"}");
        }
    myfprintf(opfd, "};\n");

    myfprintf(opfd, "metadata={");
    for( node *t = nodeList; t != NULL; t = t->next )
    {   ccommarise = "";
        if( t->strp->metadata != NULL )
        {   commarise = "";
            myfprintf(opfd, "%s{\"%j\", {", ccommarise, t->strp->s);
            for( metadataList *ml = t->strp->metadata; ml != NULL; ml = ml->next )
            {   myfprintf(opfd, "%s\"%j\"->\"%j\"", commarise, ml->property, ml->value);
                commarise = ", ";
            }
            myfprintf(opfd, "}}\n", t->strp->s);
            ccommarise = "\n,";
        }
    }
    myfprintf(opfd, "};\n");

    myfprintf(opfd, "Print[\"      title=\", title, \n\"\n      authors=\", authors, \n\"\n      version=\", version, \n\"\n      date=\", date, \n\"\n      and: communities, edges, groups, highlights, keywords, metadata, notes, vertexNames\"];\n");

    // to close off Notebook[{Cell[BoxData["...",
    //myfprintf(opfd, ", \"Input\"]]}];\n");
}



int versionstrcmp(char *a, char *b)
// like strcmp
// but runs of digits are integers which are compared
// uppercase before lower case for each letter
{
    int cmp = 0;
    while( *a && *b )
    {   if( isdigit(*a) && isdigit(*b) )
        {   int an = 0, bn = 0;
            while( *a && isdigit(*a) )
                an = 10*an+*a++-'0';
            while( *b && isdigit(*b) )
                bn = 10*bn+*b++-'0';
            //fprintf(stderr, "..  %d <> %d\n", an, bn);
            if( an == bn ) continue;
            return bn-an;
        }
        if( *a == *b ) { a++; b++; continue; }
        if( isalpha(*a) && isalpha(*b) )
        {   if( tolower(*a) == tolower(*b) )
            {   if( isupper(*a) && !isupper(*b) ) return 1;
                if( !isupper(*a) && isupper(*b) ) return -1;
                return 0;
            }
            return tolower(*b)-tolower(*a);
        }
        return *b-*a;
    }
    if( *a ) return -1;
    if( *b ) return 1;
    return 0;
}

extern int versionCount;

void checkversions(char *targetVersion, char *debug)
{   //fprintf(stderr, "checkversions(%s) @ %s\n", targetVersion, debug);
    for( node *t = nodeList; t != NULL; t = t->next )
    {   if( t->strp->nodeversion != NULL )
        {   // fprintf(stderr, "%s v=%s versionstrcmp=%d : ", t->s->s, t->s->nodeversion, versionstrcmp(targetVersion, t->s->nodeversion) );
            if( !strcmp(t->strp->nodeversion, "") )
                t->strp->visible = 1;
            else
            if( !strcmp(t->strp->nodeversion, undefinedVersion) || versionstrcmp(targetVersion, t->strp->nodeversion) > 0 )
            {   //fprintf(stderr, "%s v=%s versionstrcmp=%d : ", t->s->s, t->s->nodeversion, versionstrcmp(targetVersion, t->s->nodeversion) );
                //fprintf(stderr, "  - so make %s invisible\n", t->s->s);
                t->strp->visible = 0;
            }
            else
            {   // fprintf(stderr, "%s v=%s versionstrcmp=%d : ", t->s->s, t->s->nodeversion,versionstrcmp(targetVersion, t->s->nodeversion) );
                // fprintf(stderr, "  - so make %s visible\n", t->s->s);
                t->strp->visible = 1;
            }
        }
        else
        {
            fprintf(stderr, "?error? %s nodeversion=NULL\n", t->strp->s);
            t->strp->visible = 0;
        }
    }
}
void fixVersions(char *targetVersion)
{   if( !strcmp(targetVersion, "") ) return;

    if( !isVersion(targetVersion) )
    {   if( versionCount <= 0 )
            warning("No versions are defined");
        else
        {   fprintf(stderr, "Available versions are:\n   ");
            allVersionsSeparator(stderr, "\n   ");
        }
        nolineerror("v='%s' is not a defined version", targetVersion);
        exit(1);
    }
    checkversions(targetVersion, "A");
}

void generateFiles(char *targetVersion, char *filename)
{
    // printf("Styles are:\n");
    // for( node *u = stylelist; u != NULL; u = u->next )
    //	printf("  style %s is '%s'\n", u->s->style->s, u->s->s);

    if( !*targetVersion || !strcmp(targetVersion, "") )
    {   targetVersion = lastVersion();
        if( strcmp(targetVersion, "") )
            fprintf(stderr, "Version defaulted to v='%s'\n", targetVersion);
    }

    checkIS();
    checkNumbering();
    evalStyles();

	if( numberingCount ) // not numbering a node is only an error if any nodes are numbered
		for( node *t = nodeList; t != NULL; t = t->next )
			if( t->strp->l == ID )
			{	if( !t->strp->plain && !t->strp->ranky && !t->strp->rankx && !t->strp->noderef )
					fprintf(stderr, "Warning: Numbering for %s '%s' not defined\n", t->strp->isgroup? "group": "node", t->strp->s);
				// fprintf(stderr, "Node %s @ %d.%d\n", t->s->s, t->s->rankx, t->s->ranky);
			}

    cascade();
    sortkeywords(&allkeywords);

    if( 0 )
		for( node *t = nodeList; t != NULL; t = t->next )
			if( !t->strp->pointsTo && !t->strp->pointedFrom )
				fprintf(stderr, "Node %s is not on any arrow: %d %d\n", t->strp->s, t->strp->pointsTo, t->strp->pointedFrom);

    stopiferror();

    if( verboseOption && graphvizOption )
        fprintf(stderr, "|--NB https://magjac.com/graphviz-visual-editor is a playground that can pinpoint Graphviz errors\n");

    // dot (graphviz) is very sensitive to node order, so we sort nodes before doing anything interesting
    int swapped = 0;
    do
    {    swapped = 0;
         for( node **t = &nodeList; (*t) != NULL && (*t)->next != NULL; t = &(*t)->next )
            if( strcmp((*t)->strp->s, (*t)->next->strp->s) < 0 )
            {    node *u = *t;
                *t = (*t)->next;
                u->next = (*t)->next;
                (*t)->next = u;
                swapped = 1;
            }
    } while( swapped );
    summarizeMissingFlagDefinitions();

    // work out which files are visible in this version...
    fixVersions(targetVersion);

    makefiles(targetVersion, filename);
}

extern int flagsusedaftercascades[];
void listColorsUsed() // list highlighting colors used to stdout
{   for( int flag = 0; flag < nflagcolors; flag++ )
        flagsusedaftercascades[flag] = 0;
    for( node *t = nodeList; t != NULL; t = t->next )
        flagsusedaftercascades[t->strp->flag]++;
    char *spacing = "";
    for( int i = 0; i < nflagcolors; i++ ) // gets them in alphabetical order
        if( flagsusedaftercascades[i] )
        {   fprintf(stdout, "%s%s", spacing, !i? "gray": flagcolor(i));
            spacing = " ";
        }
    fprintf(stdout, "\n");
    if( colorsPlusOption )
    {   for( int i = 1; i < nflagcolors; i++ ) // gets them in alphabetical order
            if( strlen(flagdefinitions[i]) != 0 )
            {   fprintf(stdout, "\n%s: ", flagcolor(i));
                HTMLtranslate(stdout, NULL, flagdefinitions[i]);
                fprintf(stdout, "\n");
            }
    }
}
