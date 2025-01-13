#ifndef GUI_CHART_PRINTOUT_H
#define GUI_CHART_PRINTOUT_H

#include <wx/bitmap.h>

#include "ocpn_print.h"

/*!
 * Chart printout class is a derived class of OpenCPNPrint.
 * This class is used to print the chart.
 */
class ChartPrintout : public OpenCPNPrint {
public:
  ChartPrintout();
  ~ChartPrintout();

  // Print the chart
  bool OnPrintPage(int page);
  void GenerateGLbmp(void);

private:
  wxBitmap m_GLbmp;

  void DrawPage(wxDC *dc);
};

#endif  // GUI_CHART_PRINTOUT_H