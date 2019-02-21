<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform" version="1.0">

<!-- Load documents into variables -->
<!-- <xsl:variable name="DoxyOutput"   select="document('DoxyOutput.xml')/*"/> -->
<xsl:variable name="TestOutput"   select="document('TestReport.xml')/*"/>

<xsl:template match="/">
 <!-- Write Header -->
 <html>
 <head>
  <title>Test Report</title>
   
   <script language="javascript" type="text/javascript">
      function expand(elId, txtId) {
        var el, disp;
        el = document.getElementById(elId);
        disp = el.style.display;
        disp = (disp == 'block') ? ('none') : ('block');
        el.style.display = disp;
        el = document.getElementById(txtId);
        el.innerHTML = (disp == 'none') ? ('More details') : ('Less details');
      }
    </script>

  <style type="text/css">
      table, th, td { border-collapse: collapse; padding: 0px; }

      .div_title{ text-align: center; font-size:24px; font-weight:bold; margin-top:20px; margin-bottom:20px; }
      .div_sum  { margin-top:30px; margin-bottom:100px; }
      .summary  { text-align: center; font-size:20px; font-weight:bold; margin-top:20px; margin-bottom:20px; }
      .caption  { border-bottom:solid 1px; font-size:18px; font-weight:bold; text-align: center; }
      
      .td_req   { vertical-align:top; padding-left:5px; padding-top:10px; width:100px; }
      .td_req_d { padding-top:10px; }
      .td_meth  { vertical-align:top; padding-left:5px; }
      .td_dbg   { padding-left:5px; padding-top:10px; }
      .td_line  { padding-bottom: 5px }
      .td_res   { text-align: center; vertical-align:top; border:solid 1px; }
      .td_tc    { font-size:16px; font-weight:bold; text-align: center; vertical-align:top;
                  border-bottom:solid 1px; border-right:solid 1px; }
      .td_fn    { padding-left:2px; width:100px; }
      .td_fnval {  }
      .td_more  { text-align:center; width:100px; }
      
      .an_det   { cursor:pointer; }
      .an_noexec{ color: Blue;  font-size:16; font-weight:bold; }
      .an_pass  { color: Green; font-size:16; font-weight:bold; }
      .an_warn  { color: Orange; font-size:16; font-weight:bold; }
      .an_fail  { color: Red;   font-size:16; font-weight:bold; }

      .tab_inf  { width:100%; border-bottom:solid 1px }
      .tab_fr   { width:80%; border-left:solid 1px; border-right:solid 1px; border-top:solid 1px; }
      .tab_det  { width:100%; font-family:Arial; font-size:12px; font-style:italic; display:none; }
  </style>
   
  </head>
  <body>
    <div>
        
      <!-- Print out title, date and time -->
      <h2 align="center">
        <xsl:value-of select="$TestOutput/test/title"/>
      </h2>
      <h3 align="center">
        <xsl:value-of select="$TestOutput/test/date"/> 
        <xsl:text> </xsl:text> 
        <xsl:value-of select="$TestOutput/test/time"/>
      </h3>
        
      <table class="tab_fr" align="center">

        <!-- Print out result header -->
        <tr>
          <td class="caption" style="width:10%">Test Case</td>
          <td class="caption">Details</td>
          <td class="caption" style="width:12%">Status</td>
        </tr>

        <xsl:for-each select="$TestOutput/test">
          <!-- Print out results for all test cases -->
          <xsl:apply-templates select="test_cases"/>
        </xsl:for-each> <!-- $TestOutput/test -->
      </table>
    </div>

  <!-- Write Summary -->
  <div class="div_sum">
    <div class="summary">Summary</div>
    <table align="center" style="border:solid 1px;">
      <tr class="caption" style="border:solid 1px;">
        <td style="padding:4px; border:solid 1px;">Tests:</td>
        <td style="padding:4px; border:solid 1px;">Total</td>
        <td style="padding:4px; border:solid 1px;">Executed</td>
        <td style="padding:4px; border:solid 1px;">Passed</td>
        <td style="padding:4px; border:solid 1px;">Failed</td>
        <td style="padding:4px; border:solid 1px;">Warnings</td>
      </tr>
      <tr style="border:solid 1px">
        <td style="padding:4px; border:solid 1px;">Count:</td>
        <td style="border:solid 1px">#<xsl:value-of select="sum(//summary/tcnt)"/></td>
        <td style="border:solid 1px">#<xsl:value-of select="sum(//summary/exec)"/></td>
        <td style="border:solid 1px">#<xsl:value-of select="sum(//summary/pass)"/></td>
        <td style="border:solid 1px">#<xsl:value-of select="sum(//summary/fail)"/></td>
        <td style="border:solid 1px">#<xsl:value-of select="sum(//summary/warn)"/></td>
      </tr>
      <tr>
        <td style="padding:4px; border:solid 1px;">Result:</td>
        <xsl:choose>
          <xsl:when test="sum(//summary/fail) = 0">
            <td colspan="5" align="center"><a class="an_pass">Test Passed</a></td>
          </xsl:when>
          <xsl:otherwise>
            <td colspan="5" align="center"><a class="an_fail">Test Failed</a></td>
          </xsl:otherwise>
        </xsl:choose>
      </tr>
    </table>
  </div>


  <!-- Close report -->
  </body>
 </html>

</xsl:template>

<xsl:template match="tc">
 <!-- Create test case info row -->
  <tr>
    <td class="td_tc"><xsl:value-of select="no"/></td>
    <!-- <td class="td_tc"><xsl:number count="$TestOutput/test"/></td> -->
    <td>

      <table class="tab_inf">
        <tr>
          <td>
            <table width="100%">
              <tr>
                <td class="td_fnval"><xsl:value-of select="func"/></td>
                <td class="td_more"><a class="an_det" id="A{no}" onclick="expand('DT{no}', 'A{no}')">More details</a></td>
              </tr>
            </table>
          </td>
        </tr>

        <tr>
          <td colspan="2">
            <table class="tab_det" id="DT{no}">
              <!-- Print all debug output lines -->
              <xsl:for-each select="dbgi/detail">
                <tr>
                  <td class="td_line">
                    <xsl:value-of select="module"/>
                    <xsl:text> (</xsl:text>
                    <xsl:value-of select="line"/>
                    <xsl:text>) </xsl:text> 
                    <xsl:value-of select="desc"/>
                  </td>
                </tr>
              </xsl:for-each>

            </table>
          </td>
        </tr>
      </table>

    </td>
    <td class="td_res">
      <xsl:choose>
        <xsl:when test="res = 'PASSED'">
          <a class="an_pass">Passed</a>
        </xsl:when>
        <xsl:when test="res = 'WARNING'">
          <a class="an_warn">Warning</a>
        </xsl:when>
        <xsl:when test="res = 'NOT EXECUTED'">
          <a class="an_noexec">Not executed</a>
        </xsl:when>
        <xsl:otherwise>
          <a class="an_fail">Failed</a>
        </xsl:otherwise>
      </xsl:choose>
    </td>
  </tr>
</xsl:template>

</xsl:stylesheet>
