DOCBOOK.xsltproc = xsltproc
DOCBOOK.catalogs = $(DOCBOOK.catalog.xml) $(DOCBOOK.catalog.xsl)

DOCBOOK.lang      = en
DOCBOOK.lang/     = $(PROJECT/)doc/lang/$(DOCBOOK.lang)/
DOCBOOK.lang.args = --stringparam "l10n.gentext.default.language" "$(DOCBOOK.lang)"

DOCBOOK.path = \
	$(BUILD/)doc/$(DOCBOOK.lang)/ \
	\
	$(DOCBOOK.lang/) \
	\
	$(PROJECT/)doc/book \
	$(PROJECT/)doc/lib \
	$(PROJECT/)doc/feature \
	$(PROJECT/)doc/glossary \
	$(PROJECT/)doc/task \
	$(PROJECT/)doc/cvar \
	$(PROJECT/)doc/cmd \
	$(PROJECT/)doc/sample

DOCBOOK.catalog.xml = /opt/docbook-xml-4.5/catalog.xml
DOCBOOK.catalog.xsl = /opt/docbook-xsl-1.72.0/catalog.xml

DOCBOOK.fnCatalogs = $(call fnDelimit,:,$(DOCBOOK.catalogs) $($(DOCBOOK.inherit).catalogs))
DOCBOOK.fnPath     = $(call fnDelimit,:,$(DOCBOOK.path) $($(DOCBOOK.inherit).path))

###############################################################################

#DOCBOOK.fop/ = /opt/fop-0.20.5/
#DOCBOOK.fop  = $(DOCBOOK.fop/)fop.sh

#DOCBOOK.fop/ = /opt/fop-0.92beta/
#DOCBOOK.fop/ = /local/mike/work/fop/trunk/
DOCBOOK.fop/ = /opt/fop-0.93/
DOCBOOK.fop  = $(DOCBOOK.fop/)fop

###############################################################################

DOCBOOK.fnXML_HTML = $(call print.GENERIC,$(DOCBOOK.xsltproc),$(1),$(3),$(DOCBOOK.fnCatalogs),$(strip \
	SGML_CATALOG_FILES=$(DOCBOOK.fnCatalogs) \
	$(DOCBOOK.xsltproc) \
	--catalogs --nonet \
	--path $(DOCBOOK.fnPath) \
    $(DOCBOOK.lang.args) \
	--output $(1) $(2) $(3)))

DOCBOOK.fnXML_HTMLCHUNK = $(call print.GENERIC,$(DOCBOOK.xsltproc),$(1),$(3),$(DOCBOOK.fnCatalogs),$(strip \
	SGML_CATALOG_FILES=$(DOCBOOK.fnCatalogs) \
	$(DOCBOOK.xsltproc) \
	--catalogs --nonet \
	--path $(DOCBOOK.fnPath) \
    $(DOCBOOK.lang.args) \
	--output $(dir $(1)) $(2) $(3)))

###############################################################################

DOCBOOK.fnXML_FO = $(call print.GENERIC,$(DOCBOOK.xsltproc),$(1),$(3),$(DOCBOOK.fnCatalogs),$(strip \
	SGML_CATALOG_FILES=$(DOCBOOK.fnCatalogs) \
	$(DOCBOOK.xsltproc) \
	--catalogs --nonet \
	--path $(DOCBOOK.fnPath) \
    $(DOCBOOK.lang.args) \
	--output $(1) $(2) $(3)))

DOCBOOK.fnFO_PDF = $(call print.GENERIC,$(DOCBOOK.fop),$(1),$(2),,$(strip \
	FOP_OPTS="-Xms128m -Xmx512m -Djava.awt.headless=true" $(DOCBOOK.fop) -q -fo $(2) -pdf $(1)))
