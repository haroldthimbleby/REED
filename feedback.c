#include "header.h"

#include "notes.h"

authorList *feedbacks = NULL, *emails = NULL;

extern char *date;

int newfeedback(char *feedback)
{   authorList *p = (authorList*) safealloc(sizeof(authorList));
    //fprintf(stderr, "got feedback: %s\n", feedback);
    p->author = feedback;
    p->next = feedbacks;
    feedbacks = p;
    return 0;
}

int newemail(char *email)
{   authorList *p = (authorList*) safealloc(sizeof(authorList));
    //fprintf(stderr, "got feedback: %s\n", feedback);
    p->author = email;
    p->next = emails;
    emails = p;
    return 0;
}

char *fixchars = ":/?#[]@!$&'()*+,;=%\n\t ";

char *urlencode(char* originalText)
{   int specials = 0;
    for (int i = 0; i < strlen(originalText); i++)
    {
        if( index(fixchars, originalText[i]) != NULL )
            specials++;
    }

    char *encodedText = (char *) malloc(sizeof(char)*strlen(originalText)+1+2*specials);

    const char *hex = "0123456789abcdef";

    int pos = 0;
    for (int i = 0; i < strlen(originalText); i++)
    {
        if( index(fixchars, originalText[i]) == NULL )
            encodedText[pos++] = originalText[i];
        else {
                encodedText[pos++] = '%';
                encodedText[pos++] = hex[originalText[i] >> 4];
                encodedText[pos++] = hex[originalText[i] & 15];
            }
    }
    encodedText[pos] = '\0';
    //fprintf(stderr, "%s\n", encodedText);
    return encodedText;
}

extern char *reedVersion;

void dofeedback()
{   authorList *p = feedbacks;
    if( p == NULL )
    {   fprintf(stderr, "You are not providing any feedback\n");
        return;
    }
    if( authors == NULL )
        fprintf(stderr, "You are providing feedback without giving any authors\n");
    if( !strcmp(date, "") )
        fprintf(stderr, "You are providing feedback without giving any date\n");
    fprintf(stderr, "\033[1;94mFeedback:\033[0m\033[94m\n");
    for( p = feedbacks; p != NULL; p = p->next )
        fprintf(stderr, "%s ", p->author);
    fprintf(stderr, "\n");

    str *cmd = newstr("open \"https://www.harold.thimbleby.net/reeds/feedback.php?author=");
    for( p = authors; p != NULL; p = p->next )
    {   appendcstr(cmd, urlencode(p->author));
        if( p->next != NULL )
            appendcstr(cmd, urlencode(", "));
    }
    appendcstr(cmd, "&feedback=");
    for( p = feedbacks; p != NULL; p = p->next )
    {   appendcstr(cmd, urlencode(p->author));
        if( p->next != NULL )
            appendcstr(cmd, urlencode(" "));
    }
    appendcstr(cmd, "&email=");
    for( p = emails; p != NULL; p = p->next )
    {   appendcstr(cmd, urlencode(p->author));
        if( p->next != NULL )
            appendcstr(cmd, urlencode(", "));
    }
    appendcstr(cmd, "&date=`date`&version=");
    appendcstr(cmd, urlencode(reedVersion));
    appendcstr(cmd, "&compiled=");
    appendcstr(cmd, urlencode(__DATE__));
    appendcstr(cmd, "\"");
    if( verboseOption ) fprintf(stderr, "|-- System:  %s\n", cmd->s);
    system(cmd->s);
    fprintf(stderr, "\033[0m");
}
