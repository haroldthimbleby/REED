#include "header.h"

extern char *flagdefinitions[], *flagcolors[];
extern int flagsused[], flagcascade[];

void auxcascade(enum flagcolor fly, str *t, int depth) // recursive cascading
{
	//fprintf(stderr, "Cascade %s flag on node %s\n", flagcolor(t->flag), t->s);
	for( arrow *a = arrowList; a != NULL; a = a->next )
	{	//fprintf(stderr, "%s -> %s\n", a->u->s, a->v->s);
		if( a->u == t )
		{	//if( a->flag == noflag )
			//fprintf(stderr, "got %s->%s := %s\n", a->u->s, a->v->s, flagcolors[fly]);
			a->flag = fly;			
			//for( int v = 0; v <= depth; v++ )
			//	fprintf(stderr, "|||");
			if( a->v->flag != fly )
			{	//fprintf(stderr, " matches %s -> %s\n", a->u->is->s, a->v->is->s);
				a->v->originalflag = a->v->flag;
				a->v->flag = fly;
				auxcascade(fly, a->v, depth+1);
			}
		}
		if( a->doublearrow && a->v == t )
		{	a->flag = fly;			
			if( a->u->flag != fly )
			{	//fprintf(stderr, " matches %s -> %s\n", a->u->is->s, a->v->is->s);
				a->u->originalflag = a->u->flag;
				a->u->flag = fly;
				auxcascade(fly, a->u, depth+1);
			}
		}
	}
}

void cascade()
{	// there are two things that may be cascaded
	// flag colors if flagcascade[] is set
	// or nodes if ->cascade is set
	
	for( int i = 1; i < 7; i++ )
		if( flagcascade[i] )
		{	fprintf(stderr, "Cascade all flags coloured %s\n", flagcolor(i));
            for( node *t = nodeList; t != NULL; t = t->next )
                if( t->s->flag == i )
                    auxcascade(t->s->flag, t->s, 0);
		}
	
	for( int flag = 0; flag < 7; flag++ )
		flagsused[flag] = 0;	
	for( node *t = nodeList; t != NULL; t = t->next )
			flagsused[t->s->flag]++;
	
	for( node *t = nodeList; t != NULL; t = t->next )
	{	if( t->s->cascade )
			auxcascade(t->s->flag, t->s, 0);
	}
}
