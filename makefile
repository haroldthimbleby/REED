SOURCES = cascade.c main.c notes.c parse.c generate.c makefiles.c properties.c printerror.c hash.c html.c translate.c keywords.c pull.c listnodes.c metadata.c

OBJECTS = $(SOURCES:.c=.o)

TARGET = reed

WEBSITE = ../REED-paper/REED-web-site

CC = cc

run: reed

%.o: %.c header.h notes.h
	$(CC) -c -g -fsanitize=address -O1 $< $(CFLAGS)

$(TARGET) : $(OBJECTS)
	$(CC) $(CFLAGS) -g -fsanitize=address -O1 $^ $(LDFLAGS) -o $@

main.o: SyntaxCode.c header.h notes.h

SyntaxCode.c: SyntaxOutline.tex SyntaxCodeScript
	SyntaxCodeScript SyntaxOutline.tex > SyntaxCode.c
	git add SyntaxCode.c SyntaxOutline.tex
	git commit -m "Updated SyntaxOutline.tex and SyntaxCode.c (generated from SyntaxOutline.tex)"

papernobib: inputs betweenness1.jpg betweenness2.jpg REEDsummary.tex
	cd ../REED-paper; pdflatex REED.tex
	echo Generated ../REED-paper/REED.pdf

REEDsummary.tex: reed
	reed -summarise > REEDsummary.tex

paper: inputs betweenness1.jpg betweenness2.jpg REEDsummary.tex
	cd ../REED-paper; pdflatex REED.tex
	cd ../REED-paper; bibrun
	cd ../REED-paper; pdflatex REED.tex; pdflatex REED.tex
	echo Generated ../REED-paper/REED.pdf
	echo PS use make papernobib to make paper without updating bibliography and reed summary

lib/pow-reed.nb: lib/pow-reed reed
	reed -m lib/pow-reed
	
betweenness1.jpg betweenness2.jpg: lib/pow-reed.nb plotBetweenness.nb
	@printf "\033[38;35m"
	wolframscript -file plotBetweenness.nb
	@printf "\033[0m"
	cp betweenness2.jpg ../REED-paper/figures/fig8-betweenness2.jpg
	cp betweenness1.jpg ../REED-paper/figures/fig9-betweenness1.jpg

inputs: reed figures auxfiles both inlinefigures

inlinefigures:
	reed -g -insert norefs lib/LR lib/ABCD
	dot -Tpdf lib/ABCD.gv > ../REED-paper/figures/ABCD.pdf
	reed -g -insert norefs lib/LR lib/ABCD lib/ABCDis
	dot -Tpdf lib/ABCDis.gv > ../REED-paper/figures/ABCDis.pdf
	reed -g -insert norefs lib/TB lib/ABCD lib/ABCDis-styles 
	dot -Tpdf lib/ABCDis-styles.gv > ../REED-paper/figures/ABCDis-styles.pdf

figures:
	reed -g lib/pow-basic
	dot -Tpdf lib/pow-basic.gv > ../REED-paper/figures/fig1-pow-basic.pdf	
	reed lib/cycles
	dot -Tpdf lib/cycles.gv  > ../REED-paper/figures/fig7-cycles.pdf
	reed -pick yellow -basename lib/pow-reed-yellow v=v2 -l -g lib/pow-reed
	dot -Tpdf lib/pow-reed-yellow.gv > ../REED-paper/figures/fig10-reedv2-yellow.pdf
	reed -g -insert norefs lib/toc-styles lib/pow-toc
	dot -Tpdf lib/pow-toc.gv > ../REED-paper/figures/fig11-pow-toc.pdf
		
both:
	reed v=v2 -l -g lib/pow-reed
	dot -Tpdf lib/pow-reed.gv > ../REED-paper/figures/fig5-reedv2.pdf
	cp lib/pow-reed-color-legend.tex ../REED-paper/auxfiles
	reed v=v3 -l -x -h -g lib/pow-reed 
	dot -Tpdf lib/pow-reed.gv > ../REED-paper/figures/fig6-reedv3.pdf
	cp lib/pow-reed-xrefs.aux ../REED-paper/auxfiles
	
auxfiles: lib/pow-reed.tex
	grab lib/pow-reed.tex "/Node v2-1.1 Wrong XceedPros/" "/v2-3.1 Police summary/" > ../REED-paper/auxfiles/node-examplev2.tex
	grab lib/pow-reed.tex "/Arrow  E/" "/unreliable/" > ../REED-paper/auxfiles/arrow-examplev2.tex
	grab lib/pow-reed.tex "/Node v3-2.2 Unsupervised Abbott engineer/" "/manual edits to PrecisionWeb/" > ../REED-paper/auxfiles/narrative-examplev3.tex
	@echo "\\\\noindent\\hbox{" > ../REED-paper/auxfiles/flags-examplev3.tex
	grab lib/pow-reed.tex "/hbox{.colorflag/" "/end{tabular/" >> ../REED-paper/auxfiles/flags-examplev3.tex
	echo "}" >> ../REED-paper/auxfiles/flags-examplev3.tex
			
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
	cp lib/pow-reed.tex ../REED-paper
	cd ../REED-paper; pdflatex pow-reed.tex; bibtex pow-reed
	cd ../REED-paper; pdflatex pow-reed.tex; bibtex pow-reed
	cd ../REED-paper; pdflatex pow-reed.tex; bibtex pow-reed
	echo Generated lib/pow-reed.pdf
	cp ../REED-paper/pow-reed.pdf $(WEBSITE)/narrative.pdf

