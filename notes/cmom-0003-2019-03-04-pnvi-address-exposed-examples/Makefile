TOP=cmom-0003-2019-03-04-pnvi-address-exposed-examples

all: $(TOP).pdf

$(TOP).pdf: $(TOP).tex acmart.cls csembib-fullnames.bib pointer_byte.pdf
	pdflatex $(TOP).tex
	bibtex $(TOP)
	pdflatex $(TOP).tex
	pdflatex $(TOP).tex

quick: $(TOP).tex acmart.cls csembib-fullnames.bib
	pdflatex $(TOP).tex


clean:
	rm -rf *~
	rm -rf *.aux  *.log  *.out  *.bbl *.blg
	rm -rf $(TOP).pdf
	rm -rf *.bak

install: $(TOP).pdf
	cp $(TOP).pdf ../built_doc
	cp $(TOP).pdf ../built_doc/n2363.pdf
<<<<<<< HEAD
=======

>>>>>>> b31df959847eb762524f776bbe52345100fca708



pointer_byte.pdf: pointer_byte.dot
	dot -Tpdf pointer_byte.dot > pointer_byte.pdf

################## build figures from .fig #########

%.pstex %.pstex_t : %.fig
	fig2dev -L pstex                $< $*.pstex
	fig2dev -L pstex_t -p $*.pstex  $< $*.pstex_t

%.pdf %.pdftex_t : %.fig
	fig2dev -L pdftex               $< $*.pdf
	fig2dev -L pdftex_t -p $*.pdf   $< $*.pdftex_t

####################################################
