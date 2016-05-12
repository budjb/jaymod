<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform" version='1.0'>

    <xsl:param name="annotate.toc">0</xsl:param>
    <xsl:param name="table.borders.with.css">1</xsl:param>
    <xsl:param name="tablecolumns.extension">1</xsl:param>

    <xsl:param name="toc.max.depth">2</xsl:param>

<!-- ###################################################################### -->

<!-- ###################################################################### -->

<xsl:template name="inline.superscriptseq">
  <xsl:param name="content">
    <xsl:call-template name="anchor"/>
    <xsl:call-template name="simple.xlink">
      <xsl:with-param name="content">
        <xsl:apply-templates/>
      </xsl:with-param>
    </xsl:call-template>
  </xsl:param>
  <sup>
    <xsl:choose>
      <xsl:when test="string-length(@role) != 0">
        <xsl:attribute name="class">
          <xsl:value-of select="@role"/>
        </xsl:attribute>
      </xsl:when>
    </xsl:choose>

    <xsl:call-template name="generate.html.title"/>
    <xsl:call-template name="dir"/>
    <xsl:copy-of select="$content"/>
    <xsl:call-template name="apply-annotations"/>
  </sup>
</xsl:template>

<!-- ###################################################################### -->

</xsl:stylesheet>
