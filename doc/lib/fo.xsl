<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
                xmlns:fo="http://www.w3.org/1999/XSL/Format"
                version='1.0'>

<!-- ###################################################################### -->

<xsl:import href="http://docbook.sourceforge.net/release/xsl/1.72.0/fo/docbook.xsl"/>

<!-- ###################################################################### -->

<xsl:param name="fop1.extensions">1</xsl:param>
<xsl:param name="admon.graphics">1</xsl:param>
<xsl:param name="admon.graphics.path">doc/lib/images/</xsl:param>
<xsl:param name="admon.graphics.extension">.png</xsl:param>

<!-- ###################################################################### -->

<xsl:param name="draft.mode">no</xsl:param>

<!-- ###################################################################### -->

<xsl:param name="body.font.family">sans-serif</xsl:param>
<xsl:param name="body.start.indent">0.5in</xsl:param>

<!-- ###################################################################### -->

<xsl:param name="shade.verbatim">1</xsl:param>
<xsl:attribute-set name="shade.verbatim.style">
    <xsl:attribute name="border">solid</xsl:attribute>
    <xsl:attribute name="border-color">#000000</xsl:attribute>
    <xsl:attribute name="background-color">#ffffcf</xsl:attribute>
    <xsl:attribute name="padding-top">0.5em</xsl:attribute>
    <xsl:attribute name="padding-bottom">0.5em</xsl:attribute>
    <xsl:attribute name="padding-left">0.5em</xsl:attribute>
    <xsl:attribute name="padding-right">0.5em</xsl:attribute>
</xsl:attribute-set>

<!-- ###################################################################### -->

<xsl:attribute-set name="xref.properties">
    <xsl:attribute name="color">#0000aa</xsl:attribute>
</xsl:attribute-set>

<!-- ###################################################################### -->

<!--
<xsl:attribute-set name="root.properties">
    <xsl:attribute name="text-align">left</xsl:attribute>
</xsl:attribute-set>
-->

<!-- ###################################################################### -->

</xsl:stylesheet>
