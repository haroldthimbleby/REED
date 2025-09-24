#include "header.h"

struct list {
    str *keyword;
    struct list *next;
}
*keywords = NULL;

void addkeyword(str *keyword)
{   struct list **p = &keywords;
    int cmp;

    //fprintf(stderr, "keyword: %s\n", keyword->s);

    while( *p != NULL && (cmp = strcasecmp((*p)->keyword->s, keyword->s)) <= 0 )
    {   if( !cmp ) return;
        p = &(*p)->next;
    }

    struct list *new = (struct list*) malloc(sizeof(struct list));
    new->keyword = keyword;
    new->next = *p;
    *p = new;
}

int keywordsneedSwap(char *a, char *b)
{
    return strcasecmp(a, b) > 1;
}

void sortkeywords(arrow **keywordlist)
{
    // sort keywordlist into alphabetic (no case) order using bubble sort
    int swapped = 0;
    do
    {
        swapped = 0;
        for( arrow **t = keywordlist; (*t) != NULL && (*t)->next != NULL; t = &(*t)->next )
            if( keywordsneedSwap((*t)->u->s, (*t)->next->u->s) )
            {    //fprintf(stderr, "  swap *t: %d.%d & (*t)->next: %d.%d\n", (*t)->s->rankx, (*t)->s->ranky, (*t)->next->s->rankx, (*t)->next->s->ranky);
                arrow *u = *t;
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
}

void summarisekeywords(FILE *opfd)
// either to an HTML file or to stderr
{   if( keywords != NULL )
    {   char *sep = "";
        if( opfd == stderr )
            fprintf(opfd, "Keywords:\n");
        else
        {    fprintf(opfd, "<blockquote><h2>");
            fprintf(opfd, "Keyphrase%s: <span style=\"font-weight:normal;\">\n", keywords->next == NULL? "": "s");
        }
        for( struct list *t = keywords; t != NULL; t = t->next )
        {   fprintf(opfd, "%s    %s", sep, t->keyword->s);
            sep = ";\n";
        }
        if( opfd == stderr )
            fprintf(opfd, "\n");
        else
            fprintf(opfd, ".</span></h2></blockquote>\n");
    }
    else if( opfd == stderr )
        fprintf(opfd, "No keywords defined.\n");
}

void summariseLaTeXkeywords(FILE *opfd)
// either to an HTML file or to stderr
{   if( keywords != NULL )
    {   char *sep = "";
        fprintf(opfd, "\\noindent\\textcolor{blue}{\\sf\\textbf{Keyphrase%s:}\n", keywords->next == NULL? "": "s");
        for( struct list *t = keywords; t != NULL; t = t->next )
        {   fprintf(opfd, "%s%s", sep, t->keyword->s);
            sep = ";\n";
        }
        fprintf(opfd, "}\n\\vskip .5ex\n");
    }
}

int isakeyword(char *keyword)
{   for( struct list *t = keywords; t != NULL; t = t->next )
        if( !strcmp(t->keyword->s, keyword) )
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
        for( arrow *tt = t->s->keywords; tt != NULL; tt = tt->next )
        {   //fprintf(stderr, "%s: compare %s with %s\n", t->s->s, keyword, tt->u->s);
            if( !strcmp(keyword, tt->u->s) )
            {   t->s->keywordsOK = 1;
                break;
            }
        }
    }
}
