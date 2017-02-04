dist_doc_DATA = faq.html faq.txt TODO.txt TODO.html

if ASCIIDOCTOR
faq.html: faq.txt ../www/style.css
	asciidoctor -n faq.txt

TODO.html: faq.txt ../www/style.css
	asciidoctor -n TODO.txt
endif
