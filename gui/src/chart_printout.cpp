
#include "chart_printout.h"
#include "chcanv.h"
#include "glChartCanvas.h"
#include "ocpn_frame.h"

extern bool g_bopengl;
extern MyFrame* gFrame;

class ChartCanvas;
ChartCanvas* GetFocusCanvas();

/**
 * Chart printout implementation.
 */
ChartPrintout::ChartPrintout() : OpenCPNPrint(_("Chart print")) {}

// Destructor to prevent memory leak.
ChartPrintout::~ChartPrintout() {}

/**
 * Print the chart on one page.
 */
bool ChartPrintout::OnPrintPage(int page) {
  wxDC* dc = GetDC();
  if (dc && page == 1) {
    DrawPage(dc);
    return true;
  } else
    return false;
}

void ChartPrintout::DrawPage(wxDC* dc) {
  // Get the Size of the Chart Canvas
  int sx, sy;
  gFrame->GetFocusCanvas()->GetClientSize(&sx, &sy);  // of the canvas

  float maxX = sx;
  float maxY = sy;

  // Let's have at least some device units margin
  float marginX = 50;
  float marginY = 50;

  // Add the margin to the graphic size
  maxX += (2 * marginX);
  maxY += (2 * marginY);

  // Get the size of the DC in pixels
  int w, h;
  dc->GetSize(&w, &h);

  // Calculate a suitable scaling factor
  float scaleX = (float)(w / maxX);
  float scaleY = (float)(h / maxY);

  // Use x or y scaling factor, whichever fits on the DC
  float actualScale = wxMin(scaleX, scaleY);

  // Calculate the position on the DC for centring the graphic
  float posX = (float)((w - (maxX * actualScale)) / 2.0);
  float posY = (float)((h - (maxY * actualScale)) / 2.0);

  posX = wxMax(posX, marginX);
  posY = wxMax(posY, marginY);

  // Set the scale and origin
  dc->SetUserScale(actualScale, actualScale);
  dc->SetDeviceOrigin((long)posX, (long)posY);

  //  Get the latest bitmap as rendered by the ChartCanvas

  if (g_bopengl) {
#ifdef ocpnUSE_GL
    if (m_GLbmp.IsOk()) {
      wxMemoryDC mdc;
      mdc.SelectObject(m_GLbmp);
      dc->Blit(0, 0, m_GLbmp.GetWidth(), m_GLbmp.GetHeight(), &mdc, 0, 0);
      mdc.SelectObject(wxNullBitmap);
    }
#endif
  } else {
    //  And Blit/scale it onto the Printer DC
    wxMemoryDC mdc;
    mdc.SelectObject(*(gFrame->GetFocusCanvas()->pscratch_bm));

    dc->Blit(0, 0, gFrame->GetFocusCanvas()->pscratch_bm->GetWidth(),
             gFrame->GetFocusCanvas()->pscratch_bm->GetHeight(), &mdc, 0, 0);

    mdc.SelectObject(wxNullBitmap);
  }
}

void ChartPrintout::GenerateGLbmp(void) {
  if (g_bopengl) {
#ifdef ocpnUSE_GL
    int gsx = gFrame->GetFocusCanvas()->GetglCanvas()->GetSize().x;
    int gsy = gFrame->GetFocusCanvas()->GetglCanvas()->GetSize().y;

    unsigned char* buffer = (unsigned char*)malloc(gsx * gsy * 4);
    glReadPixels(0, 0, gsx, gsy, GL_RGBA, GL_UNSIGNED_BYTE, buffer);

    unsigned char* e = (unsigned char*)malloc(gsx * gsy * 3);

    if (buffer && e) {
      for (int p = 0; p < gsx * gsy; p++) {
        e[3 * p + 0] = buffer[4 * p + 0];
        e[3 * p + 1] = buffer[4 * p + 1];
        e[3 * p + 2] = buffer[4 * p + 2];
      }
    }
    free(buffer);

    wxImage image(gsx, gsy);
    image.SetData(e);
    wxImage mir_imag = image.Mirror(false);
    m_GLbmp = wxBitmap(mir_imag);
#endif
  }
}