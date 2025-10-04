#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>
#include "header.h"

#define MD5LENGTH 32

struct filedata {
	char *file;
	char *hash;
	struct filedata *next;
} *filelist = NULL;

void whoops(char *fmt, char *file)
{	fprintf(stderr, fmt, file);
	exit(1);
}

void hash(char *file)
{	char *str = (char *) safealloc(MD5LENGTH+1);
	int pid;
	if( str == NULL )
		whoops("** Sorry. Run out of memory in hash for %s\n", file);
	int pipeEnds[2];
	if( pipe(pipeEnds) == -1 ) // pipeEnds[0] - read, pipeEnds[1] - write
		whoops("** Sorry. pipe() failed during hash for %s\n", file);
    pid = fork();
    if( pid < 0 )
      	whoops("Fork failed!", "");
  	else if( !pid ) // child
    { 	dup2(pipeEnds[1], 1);
       	execlp("md5", "md5", "-q", file, NULL);
		whoops("** Sorry. execlp md5 for %s failed\n", file);        
    }
    wait(NULL);
    close(pipeEnds[1]);
    if( MD5LENGTH != read(pipeEnds[0], str, MD5LENGTH) )
      	whoops("** Sorry. Hash for %s failed\n", file);
    close(pipeEnds[0]);
    str[MD5LENGTH] = (char) 0;
    
    struct filedata *p = (struct filedata *) safealloc(sizeof(struct filedata));
    p->file = file;
    p->hash = str;
    p->next = filelist;
    filelist = p;
    
    if( showSignatures ) 
    	printf("Signature for '%s' is %s\n", file, str);
}

void printfiledata(FILE *opfd)
{	if( filelist == NULL ) return;
	char *plural = filelist == NULL || filelist->next == NULL? "": "s";
	fprintf(opfd, "\\vskip 4mm\n\\noindent\n\\begin{tabular}{|ll|}\\hline \nREED file%s&Digital signature%s\\\\ \\hline \n", plural, plural);
	for( struct filedata *p = filelist; p != NULL; p = p->next )
		fprintf(opfd, "%s & %s \\\\\n", p->file, p->hash);
	fprintf(opfd, "\\hline\n\\end{tabular}\n\n");
	fprintf(opfd, "\\vskip 2mm\\noindent\n\\emph{\\raggedright\\small If a REED file has been changed since this report was made, then its digital signature will now be different. You can run the REED tool with flag `--s' on any file to confirm its signature}.\n\n");
}

void HTMLprintfiledata(FILE *opfd)
{	if( filelist == NULL ) return;
	char *plural = filelist == NULL || filelist->next == NULL? "": "s";
	fprintf(opfd, "<table class=\"showCells\" frame=\"box\"><tr><td class=\"showCells\"><b>REED file%s</b></td><td class=\"showCells\"><b>Digital signature%s</b></td></tr>\n", plural, plural);
	for( struct filedata *p = filelist; p != NULL; p = p->next )
		fprintf(opfd, "<tr><td class=\"showCells\">%s</td><td class=\"showCells\">%s</td></tr>\n", p->file, p->hash);
	fprintf(opfd, "</table>\n");
	fprintf(opfd, "<p><em>If a REED file has been changed since this report was made, then its digital signature will now be different. You can run the REED tool with flag &lsquo;-s&rsquo; on any file to confirm its signature.</em></p>\n");
}

#ifdef test
int main()
{	
      printf("%s", hash("hash.c"));
}
#endif

