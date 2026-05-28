## Reliability of Electronic Evidence Diagram (REED) tool plus resources

Almost all court evidence now originates in digital systems, or is processed through digital tools. The increasing use of digital evidence in courts has led to the widespread use of tools to facilitate identifying, managing, and disclosing evidence, with the ironic result that evidence is now much more complex than ever before.

Computer evidence used in courts is often problematic, not just because of complexity but because computers are not reliable and computers may be managed and operated poorly. Digital forensic standards may not be upheld. The Common Law presumption that computer evidence is reliable is obsolete and encourages miscarriages of justice.

This project, **REED**, has built a simple, versatile, visually-based approach to help better understand, communicate, and improve the quality of computer evidence. The approach provides a comprehensive, easy to use map of evidence. REEDs record and help critique and improve the quality of what is already thought to be known — they are especially useful for expert witnesses in pre-action discussions. REEDs can also help manage IT systems prior to and regardless of possible legal action, and hence help improve the reliability of computer evidence used in any investigations.

This Git repository has the C code of the ***prototype, explorative*** Unix/Darwin command line tool, `reed`. This prototype tool was designed to play with the REED idea, and it is very much a flexible prototype so we can reimplement it properly.[^1] 

`reed -syntax` gives a summary of the REED language syntax.

`reed -flags` summarises all the REED tool's command line flags.

A typical use is `reed -htmlo pow-reed` which compiles a REED file `pow-reed` into an HTML file (which will be `pow-reed.html`) and then (thanks to the `o` in the flag) automatically opens it in a browser so you can easily navigate around the REED hypertext/web document.

There are lots of flags providing powerful features; for example:

* `-watch` enables you to edit REED files and repeatedly run the REED tool whenever they change. Thus `reed -htmlo -watch *files*` will generate an HTML file every time the *files* are edited, open it, and your browser will immediately show the changes you are making.

* `reed -insert 'feedback "Here is an idea to improve the REED tool..."'` which runs the `-insert` parameter to run the `feedback` command to provide (in this case illustrative outline) feedback to send to the author.

See [web home page](https://www.harold.thimbleby.net/reeds/) for papers and other resources, including a refereed paper currently in press in the journal *Computer Law & Security Review*. This paper includes a substantial resource on a large NHS criminal trial, which is used as a case study for using REEDs effectively.

[^1] The next version is going to be written in Rust.

