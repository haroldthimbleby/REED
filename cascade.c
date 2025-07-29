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

void markComponent(str *t, int component)
{	if( t->component == component ) // been here before
		return;
		
	t->component = component;
		
	if( t->group )
	{	// myfprintf(stderr, "** %t is in group %t\n", t->is->s, t->group->is->s);
		markComponent(t->group, component);
	}

	if( t->isgroup )
	{	// myfprintf(stderr, "Group %t is %t\n", t->s, t->is->s);
		for( node *u = nodeList; u != NULL; u = u->next )
			if( u->s->group == t )
			{	// myfprintf(stderr, "  %t is %t\n", u->s->s, u->s->is->s);
				markComponent(u->s, component);
			}
	}
	
	for( arrow *a = arrowList; a != NULL; a = a->next )
	{	if( a->u == t )
			markComponent(a->v, component);
		if( a->v == t )
			markComponent(a->u, component);
	}
}

int numberOfComponents = 0;

void findComponents() // weakly connected components
{	// initially all nodes have component set to 0; once we've finished components are numbered from 1
	int c = 1; 
	int repeat = 0; // if true (redundantly:-) keep on looking for more components
	
	// find every node in (notional) component 0
	do
    {	repeat = 0;
    	for( node *n = nodeList; n != NULL; n = n->next )
    		if( n->s->component == 0 )
    		{	repeat = 1;
    			// anything n points (or anything that points to n) to is in c, and so on recursively
    			markComponent(n->s, c);
				c++;
    		}
	} while( repeat );
	
	if( (numberOfComponents = c-1) > 1 )
	{	myfprintf(stderr, "Warning: There are %d components, so there may be missing arrows that should have linked %s components", numberOfComponents, numberOfComponents == 2? "the": "some"); 
                myfprintf(stderr, "\n         ('components' here means weakly connected components - groups of nodes connected by arrows, regardless of which way they point)\n");
		if( !componentsOption ) 
			myfprintf(stderr, " (use option -c to show component details)");
		myfprintf(stderr, "\n");
	}
	
	if( componentsOption )
	{   fprintf(stderr, "\n%d component%s\n\n", numberOfComponents, numberOfComponents > 1? "s": "");
    	for( int ac = 1; ac < c; ac++ )
    	{	fprintf(stderr, "Component %d:\n", ac);
    		for( node *n =  nodeList; n != NULL; n = n->next ) 
    			if( n->s->component == ac )
    				myfprintf(stderr, "   %s %s is %t\n", n->s->isgroup? "Group":"     ", n->s->s, n->s->is->s);
    		fprintf(stderr, "\n");
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
            for( node *t =  nodeList; t != NULL; t = t->next )
                if( t->s->flag == i )
                    auxcascade(t->s->flag, t->s, 0);
		}
	
	for( int flag = 0; flag < 7; flag++ )
		flagsused[flag] = 0;	
	for( node *t =  nodeList; t != NULL; t = t->next )
			flagsused[t->s->flag]++;
	
	for( node *t =  nodeList; t != NULL; t = t->next )
	{	if( t->s->cascade )
			auxcascade(t->s->flag, t->s, 0);
	}
}

struct transpair {
    str *u, *v;
    struct transpair *next;
} *transPairsList = NULL;

void saveCheckRtrans(str *u, str *v)
{   struct transpair *np = transPairsList;
    transPairsList = (struct transpair *) malloc(sizeof(struct transpair));
    transPairsList->next = np;
    transPairsList->u = u;
    transPairsList->v = v;
}

int checkOneRtrans(str *u, str *v)
{   for( arrow *a = arrowList; a != NULL; a = a->next )
    {   if( a->u == u )
        {   if( a->v == v )
            {   //fprintf(stderr, "yes! ->");
                return 1;
            }
            if( checkOneRtrans(a->v, v) )
                return 1;
        }
        if( a->doublearrow && a->v == u &&  a->v == u )
        {   //fprintf(stderr, "yes! <->");
            return 1;
        }
    }
    return 0;
}

void checkAllRtrans()
{   for( struct transpair *p = transPairsList; p != NULL; p = p->next )
    {
        if( checkOneRtrans(p->u, p->v) )
        {   if( verboseOption )
            fprintf(stderr, "|    Checked OK: %s ->* %s\n", p->u->s, p->v->s);
        }
        else
            fprintf(stderr, "** Failed: %s ->* %s\n", p->u->s, p->v->s);
    }
}
