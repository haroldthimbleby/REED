#include "header.h"

char *keywordtopull = NULL, *whichPull = NULL;

int auxpullnode(str *n)
{   if( n == NULL ) return 0;
    return n->flag == pullString || (n->flag == noflag && pullString == gray);
}

int pullnode(str *n) // if -pull used, return true if color or keywords match
{   if( !n->keywordsOK ) return 0;
    if( pullString == noflag ) return 1;
    if( auxpullnode(n) ) return 1;
    return 0;
    // if this node is on either end of an arrow ending in this node the pull it
    for( arrow *t = arrowList; t != NULL; t = t->next )
        if( auxpullnode(t->u) && auxpullnode(t->v) )
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

