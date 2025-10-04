//
//  makefiles.c
//  
//
//  Created by Harold Thimbleby on 9/19/25.
//

#include "header.h"

#include "notes.h"

extern char *title, *date, *version, *abstract, *direction;
extern void dot(FILE *opfd, char *title, char *version, char *date, char *direction);
extern void mathematica(FILE *opfd, char *title, char *version, authorList *authors, char *date, char *abstract);
extern void xml(FILE *opfd);

void stopiferror()
{   if( errcount > 0 )
    {   nolineerror("Stopped due to error%s", errcount>1? "s": "");
        exit(1);
    }
}

void generated(char *filename, char *suffix, char *reason)
{   if( verboseOption ) fprintf(stderr, "| ** ");
    fprintf(stderr, "Generated %s%s   - %s\n", filename, suffix, reason);
    stopiferror();
    if( openOption )
    {   str *cmd = newstr("open ");
        appendcstr(cmd, filename);
        if( *suffix )
            appendcstr(cmd, suffix);
        if( verboseOption ) fprintf(stderr, "|--");
        if( 1 || verboseOption ) fprintf(stderr, "System:  %s\n", cmd->s);
        system(cmd->s);
    }
}

void makefiles(char *filename)
{       FILE *fd = NULL;
    str *base = basename(filename);

    verboseOption = 1;

    if( hoOption )
        htmlOption = 1;

    if( htmlOption )
    {    generateSVGOption = 1;
        fprintf(stderr, "-h implies -svg for demos\n");
    }

    if( JSONOption ||
        generatePDFOption ||
        generateSVGOption ||
        goOption
       )
            graphvizOption = 1;

    if( graphvizOption ) // also true if a .gv file is needed for PDF, JSON, etc
    {    // try: $ dot -Tps graph1.gv -o graph1.ps
        fd = fopen(filename = newappendcstr(base, ".gv")->s, "w");
        if( fd == NULL )
        {   error("Can't open %s (graphviz file) for writing", filename);
            exit(1);
        }
        dot(fd, title, version, date, direction);
        fclose(fd);
        generated(filename, "", "dot file of the REED graph");
        if( goOption )
        {   str *cmd = newstr("open ");
            appendcstr(cmd, filename);
            if( verboseOption ) fprintf(stderr, "|--");
            if( 1 || verboseOption ) fprintf(stderr, "System:  %s\n", cmd->s);
            system(cmd->s);
        }
     }

    if( JSONOption )
    {   str *cmd = newstr("dot -Tjson ");
        appendstr(cmd, base);
        appendcstr(cmd, ".gv > ");
        appendstr(cmd, base);
        appendcstr(cmd, ".js");
        if( verboseOption ) fprintf(stderr, "|--");
        if( verboseOption ) fprintf(stderr, "System:  %s\n", cmd->s);
        system(cmd->s);
        generated(base->s, ".js", "JSON file of the REED graph");
    }

    if( generatePDFOption )
    {   str *cmd = newstr("dot -Tpdf ");
        appendstr(cmd, base);
        appendcstr(cmd, ".gv > ");
        appendstr(cmd, base);
        appendcstr(cmd, ".pdf");
        if( verboseOption ) fprintf(stderr, "|--");
        if( verboseOption ) fprintf(stderr, "System:  %s\n", cmd->s);
        system(cmd->s);
        generated(base->s, ".pdf", "PDF file of the REED graph");
    }

    if( generateSVGOption )
    {   str *cmd = newstr("dot -Tsvg ");
        appendstr(cmd, base);
        appendcstr(cmd, ".gv > ");
        appendstr(cmd, base);
        appendcstr(cmd, ".svg");
        if( verboseOption ) fprintf(stderr, "|--");
        if( verboseOption ) fprintf(stderr, "System:  %s\n", cmd->s);
        system(cmd->s);
        generated(base->s, ".svg", "SVG file of the REED graph");
    }

    if( latexOption )
    {   char *mainLatexFile;
        fd = fopen(mainLatexFile = filename = newappendcstr(base, ".tex")->s, "w");
        if( fd == NULL ) error("Can't open %s (tex/latex file) for writing", filename);
        else
        {
            if( !*title || authors == NULL || !*date || !*version  )
            {    myfprintf(stderr, "Warning: Missing details needed for the Latex file (%s)\n", filename);
                if( authors == NULL ) fprintf(stderr, "         - No author(s) provided\n");
                if( !*version ) fprintf(stderr, "         - No version provided\n");
                if( !*title ) fprintf(stderr, "         - No title provided\n");
                if( !*date ) fprintf(stderr, "         - No date provided\n");
            }
            notes(fd, title, version, authors, date, abstract);
            fclose(fd);
            generated(filename, "", "Latex summary of REED file");
        }
        fd = fopen(filename = newappendcstr(base, "-color-legend.tex")->s, "w");
        if( fd == NULL ) error("Can't open %s (tex/latex highlighting legend file) for writing", filename);
        else
        {
            LaTeXcolorkey(fd, "", "");
            fclose(fd);
            generated(filename, "", "Latex file explaining colour highlighting");
        }

        fd = fopen(filename = newappendcstr(base, "-xrefs.aux")->s, "w");
        if( fd == NULL ) error("Can't open %s (tex/latex cross reference file) for writing", filename);
        else
        {
            latexxrefs(fd);
            fclose(fd);
            generated(filename, "", "Latex .aux file defining cross-references: short node name to node reference, and short name-is to full node name");
        }
    }

    if( htmlOption )
    {   fd = fopen(filename = newappendcstr(base, ".html")->s, "w");
        if( fd == NULL ) error("Can't open %s (HTML file) for writing", filename);
        else
        {
            if( !*title || authors == NULL || !*date || !*version  )
            {    myfprintf(stderr, "Warning: Missing details needed for the HTML file (%s)\n", filename);
                if( authors == NULL ) fprintf(stderr, "         - No author(s) provided\n");
                if( !*version ) fprintf(stderr, "         - No version provided\n");
                if( !*title ) fprintf(stderr, "         - No title provided\n");
                if( !*date ) fprintf(stderr, "         - No date provided\n");
            }
            htmlnotes(fd, title, version, authors, date, abstract);
            fclose(fd);
            generated(filename, "", "interactive HTML of REED file");
            if( hoOption )
            {   str *cmd = newstr("open ");
                appendcstr(cmd, filename);
                if( verboseOption ) fprintf(stderr, "|--");
                if( 1 || verboseOption ) fprintf(stderr, "System:  %s\n", cmd->s);
                system(cmd->s);
            }
        }
    }

    if( xmlOption )
    {    fd = fopen(filename = newappendcstr(base, ".xml")->s, "w");
        if( fd == NULL ) error("Can't open %s (XML file) for writing", filename);
        else
        {    xml(fd);
            fclose(fd);
            generated(filename, "", "Full XML definition of the REED");
        }
    }

    if( mathematicaOption )
    {    fd = fopen(filename = newappendcstr(base, ".nb")->s, "w");
        if( fd == NULL ) error("Can't open %s (mathematica file) for writing", filename);
        else
        {    mathematica(fd, title, version, authors, date, abstract);
            fclose(fd);
            generated(filename, "", "Mathematica definition of the REED graph");
        }
    }
}
