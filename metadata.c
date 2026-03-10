//
//  metadata.c
//  
//
//  Created by Harold Thimbleby on 3/7/26.
//

#include "header.h"

extern str *lex1, *lex2, *lex3;
extern void getlex();

/* typedef struct tmpmetadata {
 struct tmpmetadata *next;
 char *property, *value;
} metadataList;
 */

void newmetadata(arrow *nl, char *prop, char *val)
{   metadataList *new = (metadataList*) safealloc(sizeof(metadataList));
    new->property = prop;
    new->value = val;
    new->count = 0;
    // fprintf(stderr, "%s --- %s : %s\n", nl->u->s, prop, val);
    new->next = nl->u->metadata;
    nl->u->metadata = new;
}

void metadata(arrow *nl)
{   // handle id colon id
    while( lex1->l == ID && lex2->l == COLON )
    {
        if( lex3->l != ID )
        {   error("expected property value after ':'");
            getlex();
            getlex();
            return;
        }

        newmetadata(nl, lex1->s, lex3->s);

        getlex();
        getlex();
        getlex();
    }
}

void allmetadata() // check properties are not repeated
{
    int anymeta = 0;
    for( node *t = nodeList; t != NULL; t = t->next )
    {   if( t->s->metadata != NULL )
        {   if( !anymeta )
                fprintf(stderr, "To use metadata, generate XML or Mathematica files (see reed -flags)\n");
            anymeta = 1;
        }
        if( t->s->metadata != NULL )
        {   //fprintf(stderr, "   %s\n", t->s->s);
            for( metadataList *ml = t->s->metadata; ml != NULL; ml = ml->next )
            {   if( ml->count == 1 )
                {   nolineerror("node %s has repeated property %s", t->s->s, ml->property);
                }
                if( ml->next != NULL )
                    for( metadataList *ml2 = ml->next; ml2 != NULL; ml2 = ml2->next )
                        if( !strcmp(ml->property, ml2->property) )
                            ml2->count++;
            }
        }
    }
}
