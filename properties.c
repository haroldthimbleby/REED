#include "header.h"

void maketype(str *s, type t)
{
	if( s->t != None && s->t != t )
		error("Making %s (type %s) have type %s", s->s, s->t, t);
	s->t = t;
}