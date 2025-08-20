SOURCES = cascade.c main.c notes.c parse.c generate.c properties.c printerror.c hash.c html.c translate.c

OBJECTS = $(SOURCES:.c=.o)

TARGET = reed

WEBSITE = ../REED-paper/REED-paper-web-site

run: reed

%.o: %.c header.h notes.h
	$(CC) -c $< $(CFLAGS) 

$(TARGET) : $(OBJECTS)
	$(CC) $(CFLAGS) $^ $(LDFLAGS) -o $@

main.o: SyntaxCode.c header.h notes.h

SyntaxCode.c: SyntaxOutline.tex SyntaxCodeScript
	SyntaxCodeScript > SyntaxCode.c
	git add SyntaxCode.c SyntaxOutline.tex
	git commit -m "Updated SyntaxOutline.tex"

paper: inputs website

inputs: reed
	reed v=v2 -l lib/pow-reed
	dot -Tpdf lib/pow-reed.gv > ../REED-paper/figures/reedv2.pdf
	mv lib/pow-reed-color-legend.tex ../REED-paper/figures
	grab lib/pow-reed.tex "/Node v2-1.1 Wrong XceedPros/" "/summary/" > ../REED-paper/figures/node-examplev2.tex
	grab lib/pow-reed.tex "/Arrow  E/" "/unreliable/" > ../REED-paper/figures/arrow-examplev2.tex
	reed v=v3 -l -x -h lib/pow-reed 
	mv lib/pow-reed-xrefs.aux ../REED-paper/figures
	dot -Tpdf lib/pow-reed.gv > ../REED-paper/figures/reedv3.pdf
	grab lib/pow-reed.tex "/Node v3-3.0 Unsupervised Abbott engineer/" "/Abbott PrecisionWeb database/" > ../REED-paper/figures/narrative-examplev3.tex
	echo "\\\\noindent\\hbox{" > ../REED-paper/figures/flags-examplev3.tex
	grab lib/pow-reed.tex "/hbox{.colorflag/" "/end{tabular/" >> ../REED-paper/figures/flags-examplev3.tex
	echo "}" >> ../REED-paper/figures/flags-examplev3.tex
	reed lib/toc-styles lib/pow-toc
	dot -Tpdf lib/pow-toc.gv > ../REED-paper/figures/pow-toc.pdf
	reed lib/pow-basic
	dot -Tpdf lib/pow-basic.gv > ../REED-paper/figures/pow-basic.pdf
	reed lib/LR lib/ABCD
	dot -Tpdf lib/ABCD.gv > ../REED-paper/figures/ABCD.pdf
	reed lib/LR lib/ABCD lib/ABCDis
	dot -Tpdf lib/ABCDis.gv > ../REED-paper/figures/ABCDis.pdf
	reed lib/TB lib/ABCD lib/ABCDis-styles 
	dot -Tpdf lib/ABCDis-styles.gv > ../REED-paper/figures/ABCDis-styles.pdf
	
rsm: lib/darzi
	reed lib/darzi
	echo darzi: fix arrow to arc upwards
	sed 's/"b2"->"b4"/&:n/' lib/darzi.gv > tmp
	mv tmp darzi.gv
	dot -Tpdf lib/darzi.gv > lib/darzi.pdf
	mv lib/darzi.pdf "/Users/harold/Desktop/Blindspots Darzi report"
	
tidy:
	rm -f *.o
	rm -f narrative.dvi narrative.log narrative.pdf

website: paper narrative $(WEBSITE)/REED.pdf $(WEBSITE)/pow-toc.pdf $(WEBSITE)/reedv2.pdf $(WEBSITE)/reedv3.pdf $(WEBSITE)/pow-reed.xml $(WEBSITE)/pow-reed.html

$(WEBSITE)/REED.pdf: $(WEBSITE)/../REED.pdf
	cp $^ $@

$(WEBSITE)/pow-toc.pdf: $(WEBSITE)/../figures/pow-toc.pdf
	cp $^ $@

$(WEBSITE)/pow-reed.xml: lib/pow-reed.xml
	cp $^ $@

$(WEBSITE)/pow-reed.html: lib/pow-reed.html
	cp $^ $@

$(WEBSITE)/reedv2.pdf: $(WEBSITE)/../figures/reedv2.pdf
	cp $^ $@

$(WEBSITE)/reedv3.pdf: $(WEBSITE)/../figures/reedv3.pdf
	cp $^ $@

narrative: narrative.tex
	pdflatex narrative.tex
	pdflatex narrative.tex
	cp narrative.pdf $(WEBSITE)
