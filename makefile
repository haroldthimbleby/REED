SOURCES = cascade.c main.c notes.c parse.c generate.c makefiles.c properties.c printerror.c hash.c html.c translate.c keywords.c pull.c listnodes.c metadata.c

OBJECTS = $(SOURCES:.c=.o)

TARGET = reed

WEBSITE = $(PAPERdirectory)/REED-web-site

CFLAGS = -g -fsanitize=address -O1

CC = cc

PAPERdirectory = ../REED-paper

run: reed

%.o: %.c header.h notes.h
	$(CC) $(CFLAGS) -c $< $(CFLAGS)

$(TARGET) : $(OBJECTS)
	$(CC) $(CFLAGS) $^ $(LDFLAGS) -o $@

main.o: SyntaxCode.c header.h notes.h

SyntaxCode.c: SyntaxOutline.tex SyntaxCodeScript
	SyntaxCodeScript SyntaxOutline.tex > SyntaxCode.c
	git add SyntaxCode.c SyntaxOutline.tex
	git commit -m "Updated SyntaxOutline.tex and SyntaxCode.c (generated from SyntaxOutline.tex)"

papernobib: inputs betweenness1.jpg betweenness2.jpg REEDsummary.tex
	cd $(PAPERdirectory); pdflatex REED.tex
	echo Generated $(PAPERdirectory)/REED.pdf

REEDsummary.tex: reed
	reed -summarise > REEDsummary.tex
	cp REEDsummary.tex $(PAPERdirectory)/auxfiles/REEDsummary.tex

paper: inputs betweenness1.jpg betweenness2.jpg REEDsummary.tex
	cp $(PAPERdirectory)/figures/* $(PAPERdirectory)/
	cp $(PAPERdirectory)/auxfiles/* $(PAPERdirectory)/
	cd $(PAPERdirectory); pdflatex REED.tex
	cd $(PAPERdirectory); bibrun
	cd $(PAPERdirectory); pdflatex REED.tex; pdflatex REED.tex
	@echo Generated $(PAPERdirectory)/REED.pdf
	@echo PS use make papernobib to make paper without updating bibliography and reed summary
	cat grab.log

lib/pow-reed.nb: lib/pow-reed reed
	reed -m lib/pow-reed
	
betweenness1.jpg betweenness2.jpg: lib/pow-reed.nb plotBetweenness.nb
	@printf "\033[38;35m"
	#   if wolframscript fails, run betweenness.nb or plotBetweenness.nb manually
	#   to create files betweenness1.jpg betweenness2.jpg
	#   PS read comments in betweenness.nb to figure out what is going on
	@if wolframscript -file plotBetweenness.nb; then Success; else echo "***" Sorry, it looks like wolframscript failed, so run betweenness.nb or plotBetweenness.nb manually; fi
	@printf "\033[0m"
	cp betweenness2.jpg $(PAPERdirectory)/figures/fig8-betweenness2.jpg
	cp betweenness1.jpg $(PAPERdirectory)/figures/fig9-betweenness1.jpg

inputs: reed figures auxfiles both inlinefigures

inlinefigures:
	reed -g -insert norefs lib/LR lib/ABCD
	dot -Tpdf lib/ABCD.gv > $(PAPERdirectory)/figures/ABCD.pdf
	reed -g -insert norefs lib/LR lib/ABCD lib/ABCDis
	dot -Tpdf lib/ABCDis.gv > $(PAPERdirectory)/figures/ABCDis.pdf
	reed -g -insert norefs lib/TB lib/ABCD lib/ABCDis-styles 
	dot -Tpdf lib/ABCDis-styles.gv > $(PAPERdirectory)/figures/ABCDis-styles.pdf

figures:
	reed -g lib/pow-basic
	dot -Tpdf lib/pow-basic.gv > $(PAPERdirectory)/figures/fig1-pow-basic.pdf
	reed lib/cycles
	dot -Tpdf lib/cycles.gv  > $(PAPERdirectory)/figures/fig7-cycles.pdf
	reed -pick yellow -basename lib/pow-reed-yellow v=v2 -l -g lib/pow-reed
	dot -Tpdf lib/pow-reed-yellow.gv > $(PAPERdirectory)/figures/fig10-reedv2-yellow.pdf
	reed -g -insert norefs lib/toc-styles lib/pow-toc
	dot -Tpdf lib/pow-toc.gv > $(PAPERdirectory)/figures/fig11-pow-toc.pdf

both:
	reed v=v2 -l -g lib/pow-reed
	dot -Tpdf lib/pow-reed.gv > $(PAPERdirectory)/figures/fig5-reedv2.pdf
	cp lib/pow-reed-color-legend.tex $(PAPERdirectory)/auxfiles
	reed v=v3 -l -x -h -g lib/pow-reed 
	dot -Tpdf lib/pow-reed.gv > $(PAPERdirectory)/figures/fig6-reedv3.pdf
	cp lib/pow-reed-xrefs.aux $(PAPERdirectory)/auxfiles
	
auxfiles: lib/pow-reed.tex
	@cat /dev/null > grab.log
	grab lib/pow-reed.tex "/Node v2-1.1 Wrong XceedPros/" "/v2-3.1 Police summary/" > $(PAPERdirectory)/auxfiles/node-examplev2.tex
	grab lib/pow-reed.tex "/Arrow  E/" "/unreliable/" > $(PAPERdirectory)/auxfiles/arrow-examplev2.tex
	grab lib/pow-reed.tex "/Node v2-2.2 Unsupervised Abbott engineer/" "/manual edits to PrecisionWeb/" > $(PAPERdirectory)/auxfiles/narrative-examplev3.tex
	@echo "\\\\noindent\\hbox{" > $(PAPERdirectory)/auxfiles/flags-examplev3.tex
	grab lib/pow-reed.tex "/hbox{.colorflag/" "/end{tabular/" >> $(PAPERdirectory)/auxfiles/flags-examplev3.tex
	echo "}" >> $(PAPERdirectory)/auxfiles/flags-examplev3.tex
	cat grab.log

rsm: lib/darzi
	reed -g lib/darzi
	echo darzi: fix arrow to arc upwards
	sed 's/"b2"->"b4"/&:n/' lib/darzi.gv > tmp
	mv tmp lib/darzi.gv
	dot -Tpdf lib/darzi.gv > lib/darzi.pdf
	mv lib/darzi.pdf "/Users/harold/Desktop/Blindspots Darzi report"
	
tidy:
	rm -f *.o
	rm -f betweenness.jpg
	rm -f betweenness1.jpg
	rm -f betweenness2.jpg

website: paper narrative $(WEBSITE)/REED.pdf $(WEBSITE)/fig11-pow-toc.pdf $(WEBSITE)/fig5-reedv2.pdf $(WEBSITE)/fig6-reedv3.pdf $(WEBSITE)/pow-reed.xml $(WEBSITE)/pow-reed.html

$(WEBSITE)/REED.pdf: $(WEBSITE)/../REED.pdf
	cp $^ $@

$(WEBSITE)/fig11-pow-toc.pdf: $(WEBSITE)/../figures/fig11-pow-toc.pdf
	cp $^ $@

lib/pow-reed.xml: lib/pow-reed
	reed -x lib/pow-reed

$(WEBSITE)/pow-reed.xml: lib/pow-reed.xml
	cp $^ $@

lib/pow-reed.html: lib/pow-reed
	reed -h lib/pow-reed

$(WEBSITE)/pow-reed.html: lib/pow-reed.html
	cp $^ $@

$(WEBSITE)/fig5-reedv2.pdf: $(WEBSITE)/../figures/fig5-reedv2.pdf
	cp $^ $@

$(WEBSITE)/fig6-reedv3.pdf: $(WEBSITE)/../figures/fig6-reedv3.pdf
	cp $^ $@

lib/pow-reed.tex: lib/pow-reed
	reed -l lib/pow-reed

narrative: lib/pow-reed.tex
	cp lib/pow-reed.tex $(PAPERdirectory)
	cd $(PAPERdirectory); pdflatex pow-reed.tex; bibtex pow-reed
	cd $(PAPERdirectory); pdflatex pow-reed.tex; bibtex pow-reed
	cd $(PAPERdirectory); pdflatex pow-reed.tex; bibtex pow-reed
	echo Generated lib/pow-reed.pdf
	cp $(PAPERdirectory)/pow-reed.pdf $(WEBSITE)/narrative.pdf

