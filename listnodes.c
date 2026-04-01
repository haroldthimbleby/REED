//
//  listnodes.c
//  
//
//  Created by Harold Thimbleby on 3/3/26.
//

#include "header.h"

void listnodes()
{
    if( listBothOption )
        listidsOption = listissOption = 1;

    if( !listidsOption && !listissOption ) return;

    if( nodeList == NULL )
    {   fprintf(stderr, "No nodes have been specified, so nothing to list.\n");
        return;
    }

    if( listidsOption )
    {   fprintf(stderr, "Sorted by id:\n\n");
        int swapped = 0;
        do
        {   swapped = 0;
            for( node **t = &nodeList; (*t) != NULL && (*t)->next != NULL; t = &(*t)->next )
                if( strcasecmp((*t)->strp->s, (*t)->next->strp->s) > 0 )
                {   node *u = *t;
                    *t = (*t)->next;
                    u->next = (*t)->next;
                    (*t)->next = u;
                    swapped = 1;
                }
        } while( swapped );

        for( node *t = nodeList; t != NULL; t = t->next )
        {   fprintf(stderr, "%s", t->strp->s);
            if( t->strp->is != NULL )
            {   fprintf(stderr, " is\n  \"");
                for( char *s = t->strp->is->s; *s; s++ )
                    fprintf(stderr, "%c", *s == '\n'? ' ': *s);
                fprintf(stderr, "\"");
            }
            fprintf(stderr, "\n");
        }

        if( listissOption )
            fprintf(stderr, "\n\n");
    }

    if( listissOption )
    {   fprintf(stderr, "Sorted by full name (where defined):\n\n");
        int swapped = 0;
        do
        {   swapped = 0;
            for( node **t = &nodeList; (*t) != NULL && (*t)->next != NULL; t = &(*t)->next )
            {   char *u, *v;

                u = (*t)->strp->is == NULL? (*t)->strp->s: (*t)->strp->is->s;
                v = (*t)->next->strp->is == NULL? (*t)->next->strp->s: (*t)->next->strp->is->s;

                if( strcasecmp(u, v) > 0 )
                {   node *u = *t;
                    *t = (*t)->next;
                    u->next = (*t)->next;
                    (*t)->next = u;
                    swapped = 1;
                }
            }
        } while( swapped );

        for( node *t = nodeList; t != NULL; t = t->next )
        {    fprintf(stderr, "%s", t->strp->s);
            if( t->strp->is != NULL )
            {   fprintf(stderr, " is\n  \"");
                for( char *s = t->strp->is->s; *s; s++ )
                    fprintf(stderr, "%c", *s == '\n'? ' ': *s);
                fprintf(stderr, "\"");
            }
            fprintf(stderr, "\n");
        }
    }
}
