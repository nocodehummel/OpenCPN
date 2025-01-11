/***************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  Implement ocpn_print.h
 * Author:   David Register
 *
 ***************************************************************************
 *   Copyright (C) 2010 by David S. Register                               *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,  USA.         *
 **************************************************************************/
#include "config.h"

#include <wx/dialog.h>
#include <wx/progdlg.h>
#include <wx/printdlg.h>
#include <wx/event.h>

#include "chcanv.h"
#include "glChartCanvas.h"
#include "ocpn_frame.h"
#include "ocpn_print.h"

#include <navutil.h>

extern bool g_bopengl;
extern MyFrame* gFrame;

class ChartCanvas;
ChartCanvas* GetFocusCanvas();

bool OpenCPNPrint::OnPrintPage(int page) {
  wxDC* dc = GetDC();
  if (dc) {
    if (page == 1) DrawPageOne(dc);

    return true;
  } else
    return false;
}

bool OpenCPNPrint::OnBeginDocument(int startPage, int endPage) {
  if (!wxPrintout::OnBeginDocument(startPage, endPage)) return false;

  return true;
}

void OpenCPNPrint::GetPageInfo(int* minPage, int* maxPage, int* selPageFrom,
                               int* selPageTo) {
  *minPage = 1;
  *maxPage = 1;
  *selPageFrom = 1;
  *selPageTo = 1;
}

bool OpenCPNPrint::HasPage(int pageNum) { return (pageNum == 1); }

void OpenCPNPrint::DrawPageOne(wxDC* dc) {
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

void OpenCPNPrint::GenerateGLbmp() {
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

// Print selection dialog definitions.
#define ID_PRINTSELECTION 9000
#define SYMBOL_PRINT_SELECTION_STYLE \
  wxCAPTION | wxRESIZE_BORDER | wxSYSTEM_MENU | wxCLOSE_BOX
#define SYMBOL_PRINT_SELECTION_SIZE wxSize(750, 450)
#define SYMBOL_PRINT_SELECTION_POSITION wxDefaultPosition
#define ID_PRINT_SELECTION_OK 9001
#define ID_PRINT_SELECTION_CANCEL 9002

/*!
 * RouteProp event table definition
 */
BEGIN_EVENT_TABLE(PrintSelectionDialog, wxDialog)
EVT_BUTTON(ID_PRINT_SELECTION_CANCEL, PrintSelectionDialog::OnCancelClick)
EVT_BUTTON(ID_PRINT_SELECTION_OK, PrintSelectionDialog::OnOKClick)
END_EVENT_TABLE()

/*!
 * Print selection dialog base class implementation.
 */
PrintSelectionDialog::PrintSelectionDialog(wxWindow* parent,
                                           const wxString& caption) {
  long wstyle = SYMBOL_PRINT_SELECTION_STYLE;
  SetExtraStyle(GetExtraStyle() | wxWS_EX_BLOCK_EVENTS);

#ifdef __WXOSX__
  style |= wxSTAY_ON_TOP;
#endif

  Create(parent, ID_PRINTSELECTION, caption, SYMBOL_PRINT_SELECTION_POSITION,
         SYMBOL_PRINT_SELECTION_SIZE, wstyle);
}

PrintSelectionDialog::~PrintSelectionDialog() {}

/*!
 * Create grid for print selection.
 */
wxFlexGridSizer* PrintSelectionDialog::CreateGroup() {
  return new wxFlexGridSizer(0, 2, 0, 0);
}

/*!
 * Add the group box to the dialog.
 */
wxStaticBoxSizer* PrintSelectionDialog::AddGroup(wxFlexGridSizer* grid,
                                                 const wxString& title) {
  wxStaticBox* group = new wxStaticBox(this, wxID_ANY, _(title));
  wxStaticBoxSizer* group_sizer = new wxStaticBoxSizer(group, wxVERTICAL);
  this->SetSizer(group_sizer);

  group_sizer->Add(grid, 5, wxEXPAND, 5);
  return group_sizer;
}

/*!
 * Add a checkbox with label and description to the group.
 */
wxCheckBox* PrintSelectionDialog::AddCheckBox(wxFlexGridSizer* grid,
                                              const wxString label,
                                              const wxString& description,
                                              bool checked) {
  wxCheckBox* checkbox = new wxCheckBox(
      this, wxID_ANY, _(label), wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT);
  checkbox->SetValue(checked);
  grid->Add(checkbox, 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);

  wxStaticText* desc = new wxStaticText(this, wxID_ANY, _(description),
                                        wxDefaultPosition, wxDefaultSize);
  grid->Add(desc, 1, wxALL | wxALIGN_CENTER_VERTICAL, 5);

  return checkbox;
}

/**
 * Finalize the dialog.
 */
void PrintSelectionDialog::Finalize(wxStaticBoxSizer* sizer) {
  wxBoxSizer* footer = new wxBoxSizer(wxHORIZONTAL);
  sizer->Add(footer, 0, wxALIGN_RIGHT | wxALL, 5);

  m_CancelButton = new wxButton(this, ID_PRINT_SELECTION_CANCEL, _("Cancel"),
                                wxDefaultPosition, wxDefaultSize, 0);
  footer->Add(m_CancelButton, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);

  m_OKButton = new wxButton(this, ID_PRINT_SELECTION_OK, _("OK"),
                            wxDefaultPosition, wxDefaultSize, 0);
  footer->Add(m_OKButton, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);
  m_OKButton->SetDefault();

  DimeControl(this);
  Centre();
}

/*!
 * Cancel event handler.
 */
void PrintSelectionDialog::OnCancelClick(wxCommandEvent& event) {
  Close();  // Hide();
  event.Skip();
}
