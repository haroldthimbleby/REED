//
//  metadata.c
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

arrow *makearrowformeta(str *u, str *v)
{
    for( arrow *ap = metaArrowList; ap != NULL; ap = ap->next )
        if( !strcmp(ap->u->s, u->s) && (ap->v == NULL || !strcmp(ap->v->s, v->s)) )
            return ap;
    arrow *n = (arrow*) safealloc(sizeof(arrow));
    n->next = metaArrowList;
    n->u = u;
    n->v = v;
    n->arrowis = NULL;
    n->metadata = NULL;
    metaArrowList = n;
    return n;
}

void newmetadata(arrow *nl, str *prop, str *val)
{   for( arrow *a = nl; a != NULL; a = a->next ) // nodes and arrow skimmed by note
    {  if(0) fprintf(stderr, "* metadata %s%s%s property %s : %s\n",
                a->u->s,
                a->v == NULL? " ": " -> ",
                a->v == NULL? "": a->v->s,
                prop->s, val->s);
        metadataList *new;
        arrow *address = makearrowformeta(a->u, a->v);
        new = (metadataList*) safealloc(sizeof(metadataList));
        new->property = prop->s;
        new->value = val->s;
        new->count = 0;
        new->next = address->metadata;
        address->metadata = new;
    }
}

int metadata(arrow *nl)
{   // handle id colon id
    int atleastone = 0;
    while( lex1->l == ID && lex2->l == COLON )
    {
        if( lex3->l != ID )
        {   error("expected property value after ':'");
            getlex();
            getlex();
            return atleastone;
        }
        atleastone = 1;
        newmetadata(nl, lex1, lex3);

        getlex();
        getlex();
        getlex();
    }
    return atleastone;
}

void allmetadata() // check properties are not repeated
{   int anymeta = 0;
    for( arrow *t = metaArrowList; t != NULL; t = t->next )
    {   if( t->metadata != NULL )
        {   if( !anymeta )
                fprintf(stderr, "To use metadata, generate CSV, XML or Mathematica files (see reed -flags)\n");
            anymeta = 1;
        }
        if( t->metadata != NULL )
        {   //fprintf(stderr, "   %s\n", t->s->s);
            for( metadataList *ml = t->metadata; ml != NULL; ml = ml->next )
            {   if( ml->count == 1 )
                {   nolineerror("Node %s has repeated property %s", t->u->s, ml->property);
                }
                if( ml->next != NULL )
                    for( metadataList *ml2 = ml->next; ml2 != NULL; ml2 = ml2->next )
                        if( !strcmp(ml->property, ml2->property) )
                            ml2->count++;
            }
        }
    }
}

void generateMetadata(FILE *opfd)
{   int anymeta = 0;
    for( arrow *t = metaArrowList; t != NULL; t = t->next )
    {   // fprintf(stderr, "|||> %s %s\n", t->u->s, t->v == NULL || t->v->s == NULL? "node": "arrow");
        if( t->metadata != NULL )
        {   for( metadataList *ml = t->metadata; ml != NULL; ml = ml->next )
            {   if( !anymeta )
                    fprintf(opfd, "Type, ID1, ID2, \"Full name\", Property, Value\n");
                anymeta = 1;
                int isnode = t->v == NULL;
                myfprintf(opfd, "%s, %v, %v, %v, ",
                          isnode? "Node": "Arrow",
                          t->u->s,
                          isnode? "": t->v->s,
                          isnode?
                          (t->u->is != NULL && t->u->is->s != NULL? t->u->is->s: ""):
                          isnode? "": (t->arrowis != NULL && t->arrowis->s != NULL? t->arrowis->s: "")
                        );
                myfprintf(opfd, "%v, %v\n", ml->property, ml->value);
            }
        }
    }
    if( !anymeta )
        nolineerror("No properties for -properties to generate");
}
