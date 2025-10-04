#include "header.h"

struct keywordlist *allkeywords = NULL;

void addkeyword(str *keyword)
{   noteaddkeywordtolist(keyword, &allkeywords);
}

void noteaddkeywordtolist(str *keyword, struct keywordlist **keywords)
{   struct keywordlist **p = keywords;
    int cmp;

    //fprintf(stderr, "keyword: %s\n", keyword->s);

    while( *p != NULL && (cmp = strcasecmp((*p)->keyword->s, keyword->s)) <= 0 )
    {   if( !cmp ) return;
        p = &(*p)->next;
    }

    struct keywordlist* new = safealloc(sizeof(struct keywordlist));
    new->keyword = keyword;
    new->next = *p;
    new->ykey = new->xkey = 0;
    *p = new;
}

int keywordsneedSwap(char *a, char *b)
{
    return strcasecmp(a, b) > 1;
}

void debugkeywords(char *d, struct keywordlist *keywordlist)
{   fprintf(stderr, "%s: ", d);
    for( struct keywordlist *t = keywordlist; t != NULL; t = t->next )
        fprintf(stderr, " '%s'", t->keyword->s);
    fprintf(stderr, "\n");
}

void sortkeywords(struct keywordlist **keywordlist)
{   // sort keywordlist into alphabetic (no case) order using bubble sort
    int swapped = 0;
    do
    {
        swapped = 0;
        for( struct keywordlist **t = keywordlist; (*t) != NULL && (*t)->next != NULL; t = &(*t)->next )
            if( keywordsneedSwap((*t)->keyword->s, (*t)->next->keyword->s) )
            {    //fprintf(stderr, "  swap *t: %d.%d & (*t)->next: %d.%d\n", (*t)->s->rankx, (*t)->s->ranky, (*t)->next->s->rankx, (*t)->next->s->ranky);
                struct keywordlist *u = *t;
                *t = (*t)->next;
                //fprintf(stderr, "  A now u: %d.%d & t: %d.%d\n", u->s->rankx, u->s->ranky, (*t)->s->rankx, (*t)->s->ranky);
                u->next = (*t)->next;
                //fprintf(stderr, "  B now u->next: %d.%d & t: %d.%d\n", u->next->s->rankx, u->next->s->ranky, (*t)->s->rankx, (*t)->s->ranky);
                (*t)->next = u;
                //fprintf(stderr, "  C now u: %d.%d & t: %d.%d\n", u->s->rankx, u->s->ranky, (*t)->s->rankx, (*t)->s->ranky);
                swapped = 1;
                //for( node *t = nodeList; t != NULL; t = t->next )
                //    fprintf(stderr, "%d.%d - ", t->s->rankx, t->s->ranky);
                //fprintf(stderr, "\n");
                //break;
            }
    } while( swapped );
    int counter = 0;
    if( *keywordlist == allkeywords ) // initialise href links for keywords
        for( struct keywordlist *t = allkeywords; t != NULL && t->next != NULL; t = t->next )
            t->xkey = ++counter;
}

void linkkeyword(FILE *opfd, struct keywordlist *t, char *debug)
{   // stupid coding! should have had a keyword symbol table and used it first... easy to fix but this is easier
    struct keywordlist *p;
    for( p = allkeywords; p != NULL; p = p-> next )
        if( !strcasecmp(p->keyword->s, t->keyword->s) )
            break;

    if( p == NULL ) { fprintf(stderr, "ooops! didn't find %s in keyword symbol table\n", t->keyword->s); exit(1); }

    // fprintf(stderr, "%s: <a href=\"#key-%d-%d\" name=\"key-%d-%d\">\n", debug, p->xkey, p->ykey+1, p->xkey, p->ykey);
    fprintf(opfd, "<a href=\"#key-%d-%d\" name=\"key-%d-%d\">", p->xkey, p->ykey+1, p->xkey, p->ykey);
    p->ykey++;
}

void summarisekeywords(FILE *opfd)
// either to an HTML file or to stderr
{   int toHTML = opfd != stderr;
    if( allkeywords != NULL )
    {   char *sep = "";
        if( !toHTML )
            fprintf(opfd, "Keywords:\n");
        else
        {    fprintf(opfd, "<blockquote><h2>");
             fprintf(opfd, "Keyphrase%s: <span style=\"font-weight:normal;\">\n", allkeywords->next == NULL? "": "s");
        }
        for( struct keywordlist *t = allkeywords; t != NULL; t = t->next )
        {   fprintf(opfd, "%s    ", sep);
            if( toHTML ) linkkeyword(opfd, t, t->keyword->s);
            fprintf(opfd, "%s", t->keyword->s);
            if( toHTML ) fprintf(opfd, "</a>");
            sep = ";\n";
        }
        if( !toHTML )
            fprintf(opfd, "\n");
        else
            fprintf(opfd, ".</span></h2></blockquote>\n");
    }
    else if( !toHTML )
        fprintf(opfd, "No keywords defined.\n");
}

void summariseLaTeXkeywords(FILE *opfd)
// either to an HTML file or to stderr
{   if( allkeywords != NULL )
    {   char *sep = "";
        fprintf(opfd, "\\noindent\\textcolor{blue}{\\sf\\textbf{Keyphrase%s:}\n", allkeywords->next == NULL? "": "s");
        for( struct keywordlist *t = allkeywords; t != NULL; t = t->next )
        {   fprintf(opfd, "%s%s", sep, t->keyword->s);
            sep = ";\n";
        }
        fprintf(opfd, ".}\n\\vskip .5ex\n");
    }
}

int suffix(char *s, char *pattern)
{    while( *s )
        {   if( !strcmp(s, pattern) )
            {  //fprintf(stderr, "%s==%s\n", s, pattern);
               return 0;
            }
            s++;
        }
    //fprintf(stderr, "%s<>%s\n", s, pattern);
    return 1;
}

int keywordcmp(char *keyword, char *pattern)
{   if( !strcmp(pattern, "...") || !strcmp(pattern, "") )
        fprintf(stderr, "Warning: keyword '%s' will match everything\n", pattern);
    if( suffix(keyword, "...") )
   {   int leng = strlen(pattern)-3;
       if( !strncmp(keyword, pattern, leng) )
            return 0;
       return 1;
   }
    return strcmp(keyword, pattern);
}

int isakeyword(char *keyword)
{   for( struct keywordlist *t = allkeywords; t != NULL; t = t->next )
        if( !keywordcmp(t->keyword->s, keyword) )
            return 1;
    return 0;
}

char *pullingthiskeyword = (char*) NULL;

int ispullingkeywords()
{   return pullingthiskeyword != NULL;
}

void htmlsaypullingkeyword(FILE *opfd)
{
    fprintf(opfd, "<blockquote><h2>Selecting only keyphrase: <span style=\"font-weight:normal;\">%s.</span></h2></blockquote>\n", pullingthiskeyword);
}

void pullkeywords(char *keyword)
{   pullingthiskeyword = keyword;
    //fprintf(stderr, "IMPLEMENTING -pull %s\n", keyword);
    for( node *t = nodeList; t != NULL; t = t->next )
    {   // if keyword is not in the list of the nodes keywords...
        t->s->keywordsOK = 0;
        for( struct keywordlist *tt = t->s->keywords; tt != NULL; tt = tt->next )
        {   //fprintf(stderr, "%s: compare %s with %s\n", t->s->s, keyword, tt->u->s);
            if( !keywordcmp(tt->keyword->s, keyword) )
            {   t->s->keywordsOK = 1;
                break;
            }
        }
    }
}
