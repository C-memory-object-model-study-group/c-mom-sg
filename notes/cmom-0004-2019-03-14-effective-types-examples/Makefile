#TOP=cmom-0004-2019-03-14-effective-types-examples
TOP=slides-2019-07-wg21-effective-types

all: $(TOP).pdf

$(TOP).pdf: $(TOP).tex acmart.cls #csembib-fullnames.bib
	pdflatex $(TOP).tex
#	bibtex $(TOP)
#	pdflatex $(TOP).tex
#	pdflatex $(TOP).tex

quick: $(TOP).tex acmart.cls #csembib-fullnames.bib
	pdflatex $(TOP).tex


clean:
	rm -rf *~
	rm -rf *.aux  *.log  *.out  *.bbl *.blg
	rm -rf $(TOP).pdf
	rm -rf *.bak

#install: $(TOP).pdf
#	cp $(TOP).pdf ../built_doc

install: slides-2019-07-wg21-effective-types.pdf
	cp slides-2019-07-wg21-effective-types.pdf ../built_doc/slides-2019-07-wg21-effective-types-with-notes.pdf

install-P1796R0:  $(TOP).pdf
	cp $(TOP).pdf ../built_doc/P1796R0.pdf

################## build figures from .fig #########

%.pstex %.pstex_t : %.fig
	fig2dev -L pstex                $< $*.pstex
	fig2dev -L pstex_t -p $*.pstex  $< $*.pstex_t

%.pdf %.pdftex_t : %.fig
	fig2dev -L pdftex               $< $*.pdf
	fig2dev -L pdftex_t -p $*.pdf   $< $*.pdftex_t

####################################################
