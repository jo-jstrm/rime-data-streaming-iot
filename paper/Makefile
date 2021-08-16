name = main
submissionName = DimitriosGiouroukis

pdf:
	@TEXINPUTS=.//: pdflatex -synctex=1 -interaction=nonstopmode --shell-escape $(name)
all:
	@TEXINPUTS=.//: pdflatex -synctex=1 -interaction=nonstopmode --shell-escape $(name)
	pdflatex $(name)
	bibtex $(name)
	pdflatex $(name)
	pdflatex -synctex=1 -interaction=nonstopmode --shell-escape $(name)
	pdflatex -synctex=1 -interaction=nonstopmode --shell-escape $(name)
submission:
	cp $(name).pdf $(submissionName).pdf
bundle:
	zip $(submissionName).zip -@ < bundle.lst
view:
	evince $(name).pdf &
fig:
	rm figures/cache/*
	touch figures/cache/.init
	@TEXINPUTS=.//: pdflatex -synctex=1 -interaction=nonstopmode --shell-escape $(name)
clean:
	find . -type f \( -name  '*.thm' -or -name '*.aux' -or -name  '*.auxlock' -or -name '*.figlist' -or -name '*eps-converted-to.pdf' -or -name '*.bbl' -or -name '*.fdb_latexmk' -or -name '*.synctex.gz' -or -name '*.blg' -or -name '*.log' -or -name '*.out' -or -name '*.toc' -or -name '*.lot' -or -name '*.lof' -or -name '*.loa' -or -name '*.fls' -or -name '*~' \) -exec rm -f '{}' \;
	rm figures/cache/*
	touch figures/cache/.init
cleancache:
	rm -f figures/cache/*
	mkdir -p figures/cache
	touch figures/cache/.init
