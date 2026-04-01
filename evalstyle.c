//
//  evalstyle.c
//  
//
//  Created by Harold Thimbleby on 3/15/26.
//

#include "header.h"
#include "evalstyle.h"

void printm(char *where, int N, int huge, int **m)
{   fprintf(stderr, "%s\n    ", where);
    for( int i = 0; i < N; i++ )
        fprintf(stderr, "%4d ", i);
    fprintf(stderr, "\n");
    for( int i = 0; i < N; i++ )
    {   fprintf(stderr, "%3d ", i);
        for( int j = 0; j < N; j++ )
            fprintf(stderr, m[i][j] == huge? "   - ": "%4d ", m[i][j]);
        fprintf(stderr, "\n");
    }
    fprintf(stderr, "\n\n");
}

void evalStyles()
{   int **m = NULL, N, huge;

    // check the style definitions are not recursive
    if( stylelist != NULL )
    {   N = stylelist->count+1;
        huge = N+1;
        //fprintf(stderr, "There are %d style.new\n", N);
        m = (int**) safeCalloc(N, sizeof(int*));
        for( int i = 0; i < N; i++ )
        {    m[i] = (int*) safeCalloc(N, sizeof(int));
             for( int j = 0; j < N; j++ )
                m[i][j] = huge;
        }
        if( 0 )
            for( node *u = stylelist; u != NULL; u = u->next )
            {   fprintf(stderr, "%d >>> style.new %s is %s\n", u->count, u->strp->style->s, u->strp->s);
            }
        //printm("initialised", N, huge, m);
        for( node *u = stylelist; u != NULL; u = u->next )
            expand(u->count, &u->strp, 0, m);
        //printm("filled in", N, huge, m);

        // check if recursive?
        for( int k = 0; k < N; k++ )
            for( int i = 0; i < N; i++ )
                for( int j = 0; j < N; j++ )
                    if( m[i][j] > m[i][k] + m[k][j] )
                        m[i][j] = m[i][k] + m[k][j];
        //printm("after FW alg", N, huge, m);
        // if any m[i][i] is < huge then there is recursion
        int recursive = 0;
        for( int i = 0; i < N; i++ )
        {   if( m[i][i] < huge )
            {   for( node *u = stylelist; u != NULL; u = u->next )
                if( u->count == i )
                    fprintf(stderr, "Recursive style definition: style.new %s is %s\n", u->strp->style->s, u->strp->s);
                recursive = 1;
            }
        }
        if( recursive )
            fatalError("Cannot run with recursive styles");

        for( node *u = stylelist; u != NULL; u = u->next )
            expand(0, &u->strp, 1, m);
        for( node *t = nodeList; t != NULL; t = t->next )
            if( t->strp->style != NULL && !t->strp->style->isstyle )
            {   //fprintf(stderr, "![%s] is %s --> ", t->strp->s, t->strp->style->s);
                expand(0, &t->strp->style, 1, m);
                //fprintf(stderr, "%s\n", t->strp->style->s);
            }
        for( arrow *styleda = styledArrowList; styleda != NULL; styleda = styleda->next )
            //if( u->s->style->s == styleda->arrowStyle->s )
            //    styleda->arrowStyle = u->s;
            expand(0, &styleda->arrowStyle, 1, m);
     }

    // now expand styles with embedded definitions
    //fprintf(stderr, "All styles:\n");
    //for( node *u = stylelist; u != NULL; u = u->next )
    //    fprintf(stderr, " >>> %s is %s\n", u->s->style->s, u->s->s);

    // now everything collected, replace use of style nodes with the style values themselves
    // node is: typedef struct tmpnode { str *s; struct tmpnode *next; } node;
    for( node *t = nodeList; t != NULL; t = t->next )
    {    if( (t->strp->rankx || t->strp->ranky) && t->strp->noderef && *t->strp->noderef )
        {   fprintf(stderr, "Warning: node %s has references set by ref (%s) and numbering (", t->strp->s, t->strp->noderef);
            if( t->strp->rankx ) fprintf(stderr, "%d", t->strp->rankx);
            if( t->strp->ranky ) fprintf(stderr, ".%d", t->strp->ranky);
            fprintf(stderr, ")\n");
        }
        //if( t->strp->style != NULL && !t->strp->style->isstyle )
        //    expand(&t->strp->style, 1, m);
    }

    // expand() WITH ARROW STYLES...
    //    typedef struct tmparrow { str *u, *v; struct tmparrow *next; } arrow;
    //     typedef struct tmpnode { str *s; struct tmpnode *next; } node;
    //for( arrow *styleda = styledArrowList; styleda != NULL; styleda = styleda->next )
        //if( u->s->style->s == styleda->arrowStyle->s )
        //    styleda->arrowStyle = u->s;
      //  expand(&styleda->arrowStyle, 1, m);
}

char *replace(str **target, char *from, int idlen, char *with)
{   //fprintf(stderr, "replace(%s, from=", (*target)->s);
    //for( char *q = (*target)->s; q < from; q++ )
    //    fprintf(stderr, " ");
    int max = idlen;
    char *q;
    //for( q = from; max-- > 0; q++ ) fprintf(stderr, "%c", *q);
    // fprintf(stderr, ", idlen=%d, with=%s)\n", idlen, with);
    //fprintf(stderr, " ... %s\n", q);

    char *newstring = (char*) safealloc(strlen((*target)->s)-idlen+strlen(with)+1);
    char *cp = newstring;
    for( char *cpp = (*target)->s; *cpp && cpp < from; cpp++ ) // copy prefix
        *cp++ = *cpp;
    char *news = &newstring[from-(*target)->s+strlen(with)];
    while( *with ) // copy replacement
        *cp++ = *with++;
    with = &from[idlen];
    do { // copy suffix
        *cp++ = *with;
    } while( *with++ );
    // for( node *u = stylelist; u != NULL; u = u->next )
    //     fprintf(stderr, "A>>> %s is %s\n",u->s->style->s,u->s->s);
    //fprintf(stderr, "old=%s\n",(*target)->s);
    free((*target)->s);
    (*target)->s = newstring;
    //fprintf(stderr, "newstring=%s\n",newstring);
    //  for( node *u = stylelist; u != NULL; u = u->next )
    //      fprintf(stderr, "B>>> %s is %s\n",u->s->style->s,u->s->s);

    //fprintf(stderr, "result=%s\n", newstring);
    return news; // in the calling for loop put s in right place in replacement to continue
}

void expand(int styleNumber, str **target, int doreplace, int **adjacencyMatrix)
{   // find ids in (*target)->s
    // lookup each id in stylelist
    // for( node *u = stylelist; u != NULL; u = u->next ) ...
    // if id == u->s->style->s, expand id to include spaces each side, then replace id with u->s->s

    for( char *s = (*target)->s; *s; s++ )
    {   //fprintf(stderr, ":: %s\n", s); fflush(stderr);
        if( isIDchar(*s) )
        {   int idlen = 1;
            char *from = s;
            while( isIDchar(*++s) ) idlen++;

            //fprintf(stderr, "got ");
            //for( char *f = from; f < s; f++ ) fprintf(stderr, "%c", *f);
            //fprintf(stderr, "\n");

            for( node *u = stylelist; u != NULL; u = u->next )
                if( !strncmp(from, u->strp->style->s, idlen) )
                {   // now matched, can we expand the text to replace?
                    if( 1 ) // expand to include spaces each side
                    {   // NB ' ' is not (char)0 so these loops don't drop off the end
                        while( from > (*target)->s && from[-1] == ' ' ) // prefix spaces
                        {   from--;
                            idlen++;
                        }
                        while( from[idlen] == ' ') // postfix spaces
                            idlen++;
                    }
                    char save = *s;
                    *s = (char) 0;
                    //fprintf(stderr, "replace %s with %s in %s\n", u->s->style->s, u->s->s, from);
                    *s = save;
                    if( doreplace )
                        s = replace(target, from, idlen, u->strp->s)-1;
                    else if( styleNumber >= 0 )
                    {   //fprintf(stderr, "%d expand style %d %s is %s\n", doreplace, styleNumber, (*target)->style->s, (*target)->s);
                        //fprintf(stderr, "  %d = with %s \n", u->count, u->strp->style->s);
                        adjacencyMatrix[styleNumber][u->count] = 1;
                        //fprintf(stderr, "s is at [%s]\n", s);
                        //fflush(stderr);
                    }

                    if( strlen(s) > 500 ) // this won't detect head recursion sadly
                    {   nolineerror("Very long style, so it looks like styles are recursive!");
                        for( node *u = stylelist; u != NULL; u = u->next )
                            fprintf(stderr, "  >>> %s is \"%s\"\n", u->strp->style->s, u->strp->s);
                        exit(1);
                    }
                    break;
                }
            s--;
            // fprintf(stderr, "Y\n"); fflush(stderr);
        }
    }
    //fprintf(stderr, "replaced: [%s]\n\n", (*target)->s);
}

/* old code...
 void styleReplace(str **target, char *styleName, char *replace)
{   // scan target for occurrences of styleName and replace with replace
    if( !strlen(replace) ) return; // nothing to do
    int replacements = 0;
    int len = strlen(styleName);
    if( !len ) return; // nothing to do (and would fail if it tried!)

    //fprintf(stderr, "before for s=%p\n", (*target)->s); fflush(stderr);
    for( char *s = (*target)->s; *s; s++ )
    {   if( !strncmp(s, styleName, len) )
        {
            replacements++;
            *s = (char) 0;

            // now target has 3 substrings:
            //  (*target)->s is a null terminated string before the text to replace
            //  &target[s] (which is now nulled) .. &target[s+len-1] is substring to replace
            //  then &s[len+1] is a null terminated string after the repacement
            // so...

            char *pre = (*target)->s; //fprintf(stderr, "pre: |%s|\n", pre); fflush(stderr);
            char *post = &s[len]; //fprintf(stderr, "post: |%s|\n", post); fflush(stderr);

            // gobble spaces after post
            while( *post == ' ' ) post = &s[++len];

            // gobble spaces at end of pre
            while( s > (*target)->s && s[-1] == ' ' ) s--;
            *s = (char) 0;

            *target = strlen(pre)? appendcstr(newstr(pre), replace): newstr(replace);

            int offset = strlen((*target)->s); //fprintf(stderr, "offset: %d\n", offset); fflush(stderr);
            // fprintf(stderr, "offset=%d\n", offset); fflush(stderr);
            if( strlen(post) )
                *target = appendcstr(*target, post);

            // now fix s to right place, since we've relocated the string s pointed to
            s = &(*target)->s[offset-1]; // because it's about to be incremented at end of for loop
            //fprintf(stderr, "full string: [%s]\n", (*target)->s); fflush(stderr);
            //fprintf(stderr, "rest of string: [%s]\n", s); fflush(stderr);
        }
    }
    if( 0 && replacements )
        fprintf(stderr, "%d replacements [of substrings], finally: [%s]\n", replacements, (*target)->s); fflush(stderr);
}
*/

