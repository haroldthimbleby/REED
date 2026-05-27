#include "header.h"

extern char *flagdefinitions[], *flagcolors[];
extern int flagsusedexplicitly[], flagcascade[];

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
			if( u->strp->group == t )
			{	// myfprintf(stderr, "  %t is %t\n", u->s->s, u->s->is->s);
				markComponent(u->strp, component);
			}
	}
	
	for( arrow *a = arrowList; a != NULL; a = a->next )
	{	if( a->u == t )
			markComponent(a->v, component);
		if( a->v == t )
			markComponent(a->u, component);
	}
}

void mypadstring(FILE *opfd, const char *str, int max)
{   int pad = max-strlen(str);
    for( int i = 0; i < pad; i++ )
        fputc(' ', opfd);

    myfprintf(opfd, "%t", str);
}

char *nodename(str *s)
{   if( s->is && s->is != NULL ) return s->is->s;
        return s->s;
}

void findCycles() // also computes in and out degrees
{   // we don't rely on findComponents in case it's got bugs or interactions with findCycles somehow :-)
    // besides, each component may have more than one cycle
    // initially, all nodes have ->cyclePhase set to 0

    // how many nodes? and number each node
    int N = 0;
    int max = 0;

    for( node *t = nodeList; t != NULL; t = t->next )
    {   int tmp;
        t->strp->nodeNumber = N; // numbering from 0
        // fprintf(stderr, "node %s is %d\n", t->s->s, t->s->nodeNumber);
        N++;
        if( (tmp = strlen(t->strp->s)) > max )
            max = tmp;
    }

    // create a path length NxN matrix m
    int huge = N+1;
    int **m = (int**) safeCalloc(N+2, sizeof(int*));
    for( int i = 0; i < N; i++ )
    {   m[i] = (int*) safeCalloc(N+2, sizeof(int));
        for( int j = 0; j < N; j++ )
            m[i][j] = huge;
    }
    // fprintf(stderr, "allocated m\n");

    struct cycles { str *s; int cycle; }
    *cycles = (struct cycles*) safeCalloc(N+2, sizeof(struct cycles));

    {   int i = 0;
        for( node *t = nodeList; t != NULL; t = t->next )
        {   if( i < 0 || i > N+1 )
                fprintf(stderr, "out of range: 0 <= %d <= %d\n", i, N); fflush(stderr);
            cycles[i].s = t->strp;
            cycles[i].cycle = 0;
            i++;
        }
    }

    // for each edge, put path length of 1 in m
    for( arrow *a = arrowList; a != NULL; a = a->next )
    {   // arrow goes from a->u to a->v
        //fprintf(stderr, "arrow goes from %s to %s\n", cycles[a->u->nodeNumber].s->s, cycles[a->v->nodeNumber].s->s);
        m[a->u->nodeNumber][a->v->nodeNumber] = 1;
        a->u->outDegree++;
        a->v->inDegree++;
    }

    if( transitOption ) // summarise degrees
    {
        int transits = 0;
        for( node *t = nodeList; t != NULL; t = t->next )
        {   if( t->strp->inDegree == 1 && t->strp->outDegree == 1 )
            {   if( !transits )
                {   transits = 1;
                    fprintf(stderr, "Transit nodes (in degree = out degree = 1):\n");
                }
                fprintf(stderr, "    ");
                if( t->strp->is != NULL )
                    myfprintf(stderr, "%N (", t->strp->is->s);
                fprintf(stderr, "%s", t->strp->s);
                if( t->strp->is != NULL )
                    fprintf(stderr, ")");
                fprintf(stderr, "\n");
            }
        }
        int noInfluence = 0;
        for( node *t = nodeList; t != NULL; t = t->next )
        {   if( t->strp->outDegree == 0 )
            {   if( !noInfluence )
                {   noInfluence = 1;
                    fprintf(stderr, "Nodes with no influence (out degree = 0):\n");
                }
                fprintf(stderr, "    ");
                if( t->strp->is != NULL )
                    myfprintf(stderr, "%N (", t->strp->is->s);
                fprintf(stderr, "%s", t->strp->s);
                if( t->strp->is != NULL )
                    fprintf(stderr, ")");
                fprintf(stderr, "\n");
            }
        }
    }

    if( 0 )
    {   fprintf(stderr, "Edges:\n");
        for( int i = 0; i < N; i++ )
        {   mypadstring(stderr, cycles[i].s->s, max);
            fprintf(stderr, " ");
            for( int j = 0; j < N; j++ )
                fprintf(stderr, m[i][j] == huge? "  - ": "%3d ", m[i][j]);
            fprintf(stderr, "\n");
        }
    }

    // Floyd-Warshall to find shortest paths
    for( int k = 0; k < N; k++ )
        for( int i = 0; i < N; i++ )
            for( int j = 0; j < N; j++ )
                if( m[i][j] > m[i][k] + m[k][j] )
                    m[i][j] = m[i][k] + m[k][j];

    if( 0 )
    {   fprintf(stderr, "Paths:\n");
        for( int i = 0; i < N; i++ )
        {   mypadstring(stderr, cycles[i].s->s, max);
            fprintf(stderr, " ");
            for( int j = 0; j < N; j++ )
                fprintf(stderr, m[i][j] == huge? "  - ": "%3d ", m[i][j]);
            fprintf(stderr, "\n");
        }
    }

    // there is a cycle when you can get from a to b and from b to a
    // in particular if m[a][a] < huge, a is on a cycle
    // hence if m[a][b] < huge and m[b][a] < huge then a (and b if different) are both on a cycle

    int anyUndeclaredCycles = 0;
    for( int i = 0; i < N; i++ )
    {   if( m[i][i] < huge )
        {   if( !cycles[i].s->declaredCyclic )
                anyUndeclaredCycles = cycles[i].cycle = 1;
        }
        else if( cycles[i].s->declaredCyclic )
            myfprintf(stderr, "Warning: %t declared in a cycle BUT is not in a cycle\n", nodename(cycles[i].s));
    }

    if( anyUndeclaredCycles )
    {   fprintf(stderr, "There are %d nodes\n", N+1);
        beginError;
        fprintf(stderr, "Warning: The following nodes are on undeclared cycles:\n");
        for( int i = 0; i < N; i++ )
            if( cycles[i].cycle && !cycles[i].s->declaredCyclic )
            {   mypadstring(stderr, cycles[i].s->s, max+5);
                if( cycles[i].s->is != NULL ) myfprintf(stderr, " is \"%t\"", nodename(cycles[i].s));
                fprintf(stderr, "\n");
                cycles[i].s->cyclic = 1;
            }
        endError;
    }
}

int numberOfComponents = 0;

void findComponents() // weakly connected components
{	// dot (graphviz) is very sensitive to node order, so we sort nodes before doing anything interesting
    // this also sorts the following two diagnostic lists into alphabetic order
    int swapped = 0;
    do
    {   swapped = 0;
        checkarrowlist(1);
        for( node **t = &nodeList; (*t) != NULL && (*t)->next != NULL; t = &(*t)->next )
            if( (*t)->strp->is != NULL && (*t)->next->strp->is != NULL &&
               strcmp((*t)->strp->is->s, (*t)->next->strp->is->s) > 0 )
            {   node *u = *t;
                *t = (*t)->next;
                u->next = (*t)->next;
                (*t)->next = u;
                swapped = 1;
            }
    } while( swapped );

    int max = 0, tmp;
        for( node *n = nodeList; n != NULL; n = n->next )
            if( (tmp = strlen(n->strp->s)) > max )
                max = tmp;

    if( IDsOption )
    {   for( node *n = nodeList; n != NULL; n = n->next )
        {   mypadstring(stderr, n->strp->s, max);
            if( n->strp->is != NULL ) myfprintf(stderr, " is \"%t\"", n->strp->is->s);
            if( n->strp->nodeversion != NULL && !strcmp(n->strp->nodeversion, undefinedVersion) ) myfprintf(stderr, " version: \"%t\"", n->strp->is->nodeversion);

            fprintf(stderr, "\n");
        }
    }

    // initially all nodes have component set to 0; once we've finished components are numbered from 1
	int c = 1;
	int repeat = 0; // if true (redundantly:-) keep on looking for more components
	
	// find every node in (notional) component 0
	do
    {	repeat = 0;
    	for( node *n = nodeList; n != NULL; n = n->next )
    		if( n->strp->component == 0 )
    		{	repeat = 1;
    			// anything n points (or anything that points to n) to is in c, and so on recursively
    			markComponent(n->strp, c);
				c++;
    		}
	} while( repeat );
	
	if( (numberOfComponents = c-1) > 1 )
	{	myfprintf(stderr, "Warning: There are %d components, so there may be missing arrows that should have linked %s components", numberOfComponents, numberOfComponents == 2? "the": "some"); 
                myfprintf(stderr, "\n         - 'components' here means weakly connected components\n         - i.e., groups of nodes connected by arrows, regardless of which way they point\n");
		if( !componentsOption )
			myfprintf(stderr, "         Use option -c to show more details\n");
	}

 	if( componentsOption )
	{   fprintf(stderr, "\n%d component%s\n\n", numberOfComponents, numberOfComponents > 1? "s": "");
    	for( int ac = 1; ac < c; ac++ )
    	{	fprintf(stderr, "Component %d:\n", ac);
    		for( node *n = nodeList; n != NULL; n = n->next ) 
    			if( n->strp->component == ac )
                {	myfprintf(stderr, "    %t ", n->strp->isgroup? "Group":"     ");
                    mypadstring(stderr, n->strp->s, max);
                    if( n->strp->is != NULL ) myfprintf(stderr, " is \"%t\"", n->strp->is->s);
                    fprintf(stderr, "\n");
                }
    		fprintf(stderr, "\n");
    	}
	}
}

extern int flagsusedaftercascades[];

extern int flagscascaded[];
void texcasecadedcolors(FILE *opfd, int it)
{   char *sep = "";
    int lastone = -1;
    for( int i = nflagcolors-1; i >= 0; i-- )
        if( flagscascaded[i] )
        {   lastone = i;
            break;
        }
    for( int i = 1; i < nflagcolors; i++ ) // say color it first
        if( i == it && flagscascaded[i] )
        {   if( i == lastone && *sep )
                sep = " \\& ";
            fprintf(opfd, "%s%s", sep,
                    i == it? "it": flagcolor(i));
            sep = ", ";
            break;
        }
    for( int i = 1; i < nflagcolors; i++ )
        if( i != it && flagscascaded[i] )
        {   if( i == lastone && *sep )
                sep = " \\& ";
            fprintf(opfd, "%s%s", sep,
                    i == it? "it": flagcolor(i));
            sep = ", ";
        }
}

void cascade()
{	// there are two things that may be cascaded
    // flag colors if flagcascade[] is set
    // or nodes if ->cascade is set

    //fprintf(stderr, "number of flag colors = %d + NONE\n", nflagcolors-1);

    for( int flag = 0; flag < nflagcolors; flag++ )
        flagsusedexplicitly[flag] = 0;
    for( node *t = nodeList; t != NULL; t = t->next )
    {   flagsusedexplicitly[t->strp->flag]++;
       /* myfprintf(stderr, "%s %v\n    %s %d\n",
                  t->strp->s,
                  t->strp->is != NULL? t->strp->is->s: "null",
                  flagcolor(t->strp->flag),
                  flagsusedexplicitly[t->strp->flag]);
        */
    }

    for( int i = 1; i < nflagcolors; i++ )
        if( flagcascade[i] )
        {	fprintf(stderr, "Cascade all flags coloured %s\n", flagcolor(i));
            flagscascaded[i] = 1;
            for( node *t =  nodeList; t != NULL; t = t->next )
                if( t->strp->flag == i )
                    auxcascade(t->strp->flag, t->strp, 0);
        }

    for( node *t =  nodeList; t != NULL; t = t->next )
    {	if( t->strp->cascade )
        {	flagscascaded[t->strp->flag] = 1;
            auxcascade(t->strp->flag, t->strp, 0);
        }
    }

    for( int flag = 0; flag < nflagcolors; flag++ )
        flagsusedaftercascades[flag] = 0;
    for( node *t = nodeList; t != NULL; t = t->next )
        flagsusedaftercascades[t->strp->flag]++;
}

struct transpair {
    str *u, *v, *context;
    struct transpair *next;
} *transPairsList = NULL;

void saveCheckRtrans(str *context, str *u, str *v) // check there is a path s->v
{   struct transpair *np = transPairsList;
    transPairsList = (struct transpair *) safealloc(sizeof(struct transpair));
    transPairsList->next = np;
    transPairsList->u = u;
    transPairsList->v = v;
    transPairsList->context = context;
    //fprintf(stderr, "save check %s => %s in %s\n", u->s, v->s, context == NULL? "?": context->s);
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
    }
    return 0;
}

void checkAllRtrans()
{   for( struct transpair *p = transPairsList; p != NULL; p = p->next )
{   if( checkOneRtrans(p->u, p->v) )
        {   if( verboseOption )
            {   myfprintf(stderr, "| ** Passed check in %N:\n     %N (%s) => %N (%N)\n", p->context->is? p->context->is->s: p->context->s, p->u->is? p->u->is->s: p->u->s, p->u->s, p->v->is? p->v->is->s: p->v->s, p->v->s);
            }
        }
        else
        {    fprintf(stderr, "** Failed ");
             if( p->context != NULL ) myfprintf(stderr, "node \"%N\"'s ", p->context->s);
             myfprintf(stderr, "check %N (%N) => %N (%N)\n", p->u->is? p->u->is->s: p->u->s, p->u->s, p->v->is? p->v->is->s: p->v->s, p->v->s);
        }
    fprintf(stderr, "$\n"); }
}

void checkISaux(char *id, str *t)
{
    if( t != NULL )
    {   //fprintf(stderr, "got %s\n", t->s);
        // is there any node with this as its name?
        for( node *u = nodeList; u != NULL; u = u->next )
        {   //fprintf(stderr," %s <> %s\n", t->s, u->s->s);
            if( !u->reportedNameError && !strcmp(u->strp->s, t->s) )
                nolineerror("To avoid confusion, you must not have\n   \"%s\" is \"%s\"\nand a node actually called\n   \"%s\"!\n", id, u->strp->s, u->strp->s);
            u->reportedNameError = 1;
        }
    }
}

void checkIS() // warn if any name occurs as a node if there is any node is name or arrow is name
{
    for( arrow *a = arrowList; a != NULL; a = a->next )
    {   //checkISaux(a->s, a->arrowis);
        checkISaux(a->u->s, a->u->is);
        checkISaux(a->v->s, a->v->is);
    }
    for( arrow *t = noteArrowList; t != NULL; t = t->next )
        checkISaux(t->u->s, t->arrowis);
    for( node *u = nodeList; u != NULL; u = u->next )
    {   //fprintf(stderr, "Node %s\n", u->s->s);
        checkISaux(u->strp->s, u->strp->is);
    }
}
