#include "header.h"
#include <regex.h>

extern void href(FILE *opfd, char *id, char *close);

struct { char *latex, *html; char *close; } 

// rules to convert Latex to HTML
HTMLmappings[] = { // anything that is a prefix of another MUST come later (eg, ' and '', or -- and ---)
	{ "``", "&ldquo;", 0 },
	{ "''", "&rdquo;", 0 },
	{ "`", "&lsquo;", 0 },
	{ "'", "&rsquo;", 0 },
	{ "---", "&mdash;", 0 },
	{ "--", "&ndash;", 0 },
	{ "<", "&lt;", 0 },
	{ ">", "&gt;", 0 },
	{ "\\&", "&amp;", 0 },
	{ "\\@", "", 0},
	{ "\n\n", "<p/>\n", 0 },
	{ "\\/", "", 0 },
	{ "\\ ", " ", 0 },
	{ "~", "&nbsp;", 0 },
	{ "\\warn{", "<span style=\"color:red\">", "</span>" },
	{ "\\emph{", "<em>", "</em>" },
	{ "\\textbf{", "<b>", "</b>" },
	{ "\\footnote{", "<blockquote>&mdash; ", "</blockquote>" },
    { "\\", "error", 0 }
},

// rules to convert HTML to Latex
LaTeXmappings[] = { // anything that is a prefix of another MUST come later (eg, ' and '', or -- and ---)
	{ "``", "&ldquo;", 0 },
	{ "''", "&rdquo;", 0 },
	{ "`", "&lsquo;", 0 },
	{ "'", "&rsquo;", 0 },
	{ "---", "&mdash;", 0 },
	{ "--", "&ndash;", 0 },
	{ "\\&", "&amp;", 0 },
	{ "\n\n", "<p/>\n", 0 },
	{ "~", "&nbsp;", 0 },
	{ "\\emph{", "<em>", "</em>" },
	{ "\\textbf{", "<b>", "</b>" }
//	{ "\\/", "", 0 }
};

void explainTranslationRules()
{	fprintf(stdout, "In the following * means a space, and \\n a newline (so you can see them)\n\n");
	fprintf(stdout, "Rules for converting LaTeX to HTML\n\n");
	for( int i = 0; i < sizeof(HTMLmappings)/sizeof(HTMLmappings[0]); i++ )
		if( HTMLmappings[i].close )
			myfprintf(stdout, "  %l...}\n      -> %l...%l\n", HTMLmappings[i].latex, HTMLmappings[i].html, HTMLmappings[i].close);
		else
			myfprintf(stdout, "  %l\n      -> %l\n", HTMLmappings[i].latex, HTMLmappings[i].html);

	fprintf(stdout, "\nRules for converting HTML to LaTeX\n\n");
	for( int i = 0; i < sizeof(LaTeXmappings)/sizeof(LaTeXmappings[0]); i++ )
		if( LaTeXmappings[i].close )
			myfprintf(stdout, "  %l...</%s\n      -> %l...}\n", LaTeXmappings[i].html, &LaTeXmappings[i].html[1], LaTeXmappings[i].latex);		
		else
			myfprintf(stdout, "  %l\n      -> %l\n", LaTeXmappings[i].html, LaTeXmappings[i].latex);
	fprintf(stdout, "\n");
}

struct stack { int length; char **contents; } closestack = { 0, (char**) 0 };

void push(struct stack *stack, char *item)
{	if( !stack->contents ) stack->contents = (char**) malloc(2000);
	stack->contents[stack->length++] = item;
}

char *pop(struct stack *stack)
{	if (stack->length <= 0) {
        	error("Fatal! Stack underflow - pop() in translate.c\n");
        	(void) exit(1);
		return (char*) NULL;
    	}
 	return stack->contents[--stack->length];
}

char *top(struct stack *stack)
{	return stack->contents[stack->length-1];
}

enum mode { latex, html, both } mode = both;

char *fixParagraphs(char *s)
// replace all \s*\n\s*\n with \n\n
{   str *parafixed = newstr(""); return s;
    int newlineCount = 0, blankCount = 0;
    for( ; *s; s++ )
    {   if( *s == '\n' ) { newlineCount++; continue; }
        if( *s == ' ' || *s == '\t' ) { blankCount++; continue; }
        if( newlineCount > 1 ) appendcstr(parafixed, "\n\n\n");
        else if( blankCount || newlineCount ) appendch(parafixed, ' ');
        newlineCount = blankCount = 0;
        appendch(parafixed, *s);
    }
    return parafixed->s;
}

char *includeFile(FILE *opfd, char *s)
{   // open and output the file from s[0] to either !*s or blanks
    FILE *fd;
    char *file, ch, savech;

    while( *s && (isblank(*s) || *s == '\n') ) // skip blanks
        s++;
    file = s;
    while( *s && !isblank(*s) && *s != '\n' && *s != '<' )
        s++;
    savech = *s; *s = (char) 0;
    // fprintf(stderr, "<input> \"%s\"\n", file);
    if( (fd = fopen(file, "r")) != NULL )
    {   struct stat stat_buf;
        int errno;
        if( (errno = fstat(fileno(fd), &stat_buf)) != 0 )
        {   nolineerror("** Cannot stat <input> file: %s\n%s\n", file, strerror(errno));
            exit(0);
        }
        while( (ch = fgetc(fd)) != EOF )
            fputc(ch, opfd);
    }
    else
        error("Cannot open <input> file: %s", file);
    *s = savech;
    return s;
}

void HTMLtranslate(FILE *opfd, char *note) // translate Latex and HTML to HTML, and include [[[id]]] notation
{	mode = both;

    // first do simple multinewline->\n\n case (in for loop initialisation)
    for( char *s = fixParagraphs(note); *s; s++ )
    {	int translated = 0;
        if( !strncmp(s, "<both>", 6) ) { mode = both; s = s+5; continue; }
		else if( !strncmp(s, "<latex>", 7) ) { mode = latex; s = s+6; continue; }
        else if( !strncmp(s, "<html>", 6) ) { mode = html; s = s+5; continue; }
        else if( !strncmp(s, "<input>", 7) ) { s = includeFile(opfd, s+7); continue; }
        if( mode == latex ) continue; // ignore latex
        else if( mode == html ) { fputc(*s, opfd); continue; } // copy HTML
		else if( mode == both )
		{	if( *s == '}' && closestack.length )
			{	fprintf(opfd, "%s", pop(&closestack));
				continue;
			}
            for( int i = 0; i < sizeof(HTMLmappings)/sizeof(HTMLmappings[0]); i++ )
            {   if( !strncmp(s, HTMLmappings[i].latex, strlen(HTMLmappings[i].latex)) )
                {   if( !strcmp(HTMLmappings[i].html, "error") )
                    {   char *t = s+1;
                        while( *t && isalpha(*t) ) t++;
                        char save = *t;
                        *t = (char) 0;
                        fprintf(stderr, "Warning: Unrecognised Latex which has been copied directly to HTML: %s...\n", s);
                        *t = save;
                        break;
                    }
                    fprintf(opfd, "%s", HTMLmappings[i].html);
                    s += strlen(HTMLmappings[i].latex)-1;
					if( HTMLmappings[i].close )
						push(&closestack, HTMLmappings[i].close);
					//fprintf(stderr, "translated %s -> %s ending on '%c'\n", HTMLmappings[i].latex, HTMLmappings[i].html, *s);
                    translated = 1;
                    break;
				}
			}
            if( translated ) continue;
			if( *s == '{' ) push(&closestack, "}");
			else if( *s == '}' && closestack.length )
			{	fprintf(opfd, "%s", pop(&closestack));
				continue;
			}
		}
		if( s[0] == '[' && s[1] == '[' && s[2] == '[' ) // the [[[...]]] notation must not use stuff that translates to HTML entities (they are translated above)
		{	int offset = 3;
			while( !s[offset] || s[offset] != ']' || !s[offset+1] || s[offset+1] != ']' || !s[offset+2] || s[offset+2] != ']' ) offset++;
			if( s[offset] == ']' && s[offset+1] == ']' && s[offset+2] == ']' )
			{	s[offset] = (char) 0;
				href(opfd, &s[3], "</a>");
				s[offset] = ']';
				s = s+offset+2;
			}
			else
				nolineerror("incomplete [[[node]]] in notes for %s", note);
		}
		else
			fprintf(opfd, "%c", *s);
	}
}

void LaTeXtranslate(FILE *opfd, char *version, char *note, str *innode) // translate Latex and HTML to Latex and convert [[[id]]] notation
{	mode = both;
	for( char *s = note; *s; s++ )
	{	if( !strncmp(s, "<both>", 6) ) { mode = both; s = s+5; continue; }
		else if( !strncmp(s, "<latex>", 7) ) { mode = latex; s = s+6; continue; }
		else if( !strncmp(s, "<html>", 6) ) { mode = html; s = s+5; continue; }
        else if( !strncmp(s, "<input>", 7) ) { s = includeFile(opfd, s+7); continue; }
        if( mode == html ) continue; // ignore HTML
		else if( mode == both )
        {	int translated = 0;
            for( int i = 0; i < sizeof(LaTeXmappings)/sizeof(LaTeXmappings[0]); i++ ) // bug?
			{	if( !strncmp(s, LaTeXmappings[i].html, strlen(LaTeXmappings[i].html)) )
				{	fprintf(opfd, "%s", LaTeXmappings[i].latex);
					s += strlen(LaTeXmappings[i].html)-1;
                    translated = 1;
					break;
				}
			}
			if( translated ) continue;
		}
		if( s[0] == '[' && s[1] == '[' && s[2] == '[' )
		{	int offset = 3;
			while( !s[offset] || s[offset] != ']' || !s[offset+1] || s[offset+1] != ']' || !s[offset+2] || s[offset+2] != ']' ) offset++;
			if( s[offset] == ']' && s[offset+1] == ']' && s[offset+2] == ']' )
			{	s[offset] = (char) 0;
				int found = 0;
				for( node *t = nodeList; t != NULL; t = t->next )
					if( !strcmp(t->s->s, &s[3]) )
					{	version = t->s->nodeversion;
						myfprintf(opfd, "\\hyperlink{%s}{{%s%s%d.%d} (%t)}",
                            t->s->s,
							*version? version: "", *version? "--": "",
							t->s->rankx, t->s->ranky, t->s->is == NULL? t->s->s: t->s->is->s);
						if( showIDsOption ) myfprintf(opfd, " \\fbox{%t}\\,", t->s->s);
						found = 1;
						break;
					}
				if( !found )
					nolineerror("no matching node for [[[%s]]] found in %s", &s[2], innode? "notes": "abstract");
				s[offset] = ']';
				s = s+offset+2;
			}
			else
				nolineerror("incomplete <<node>> in notes for %s", innode->s);
		}
		else
			myfprintf(opfd, "%c", s[0]);
	}
	myfprintf(opfd, "\n");
}
