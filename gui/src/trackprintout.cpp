/***************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  OpenCPN Route printout
 * Author:   Pavel Saviankou, Sean D'Epagnier
 *
 ***************************************************************************
 *   Copyright (C) 2017 by David S. Register                               *
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

#include <iostream>
using namespace std;

#include <wx/wxprec.h>

#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif  // precompiled headers

#include <wx/print.h>
#include <wx/printdlg.h>
#include <wx/artprov.h>
#include <wx/stdpaths.h>
#include <wx/intl.h>
#include <wx/listctrl.h>
#include <wx/aui/aui.h>
#include <wx/dialog.h>
#include <wx/progdlg.h>
#include <wx/brush.h>
#include <wx/colour.h>

#include <wx/dialog.h>
#include "dychart.h"
#include "ocpn_frame.h"

#ifdef __WXMSW__
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <psapi.h>
#endif

#include "trackprintout.h"
#include "printtable.h"
#include "model/track.h"
#include "gui_lib.h"

enum { PRINT_POSITION, PRINT_DISTANCE, PRINT_BEARING, PRINT_TIME, PRINT_SPEED };

// Global print data, to remember settings during the session
extern wxPrintData* g_printData;
// Global page setup data
extern wxPageSetupData* g_pageSetupData;

TrackPrintout::TrackPrintout(std::vector<bool> _toPrintOut, Track* track,
                             OCPNTrackListCtrl* lcPoints, const wxString& title)
    : OpenCPNPrint(title), myTrack(track), toPrintOut(_toPrintOut) {
  // Let's have at least some device units margin
  marginX = 100;
  marginY = 100;

  // Offset text from the edge of the cell (Needed on Linux)
  textOffsetX = 5;
  textOffsetY = 8;

  table.StartFillHeader();
  // setup widths for columns

  table << (const char*)wxString(_("Leg")).mb_str();

  if (toPrintOut[PRINT_POSITION]) {
    table << (const char*)wxString(_("Position")).mb_str();
  }
  if (toPrintOut[PRINT_BEARING]) {
    table << (const char*)wxString(_("Course")).mb_str();
  }
  if (toPrintOut[PRINT_DISTANCE]) {
    table << (const char*)wxString(_("Distance")).mb_str();
  }
  if (toPrintOut[PRINT_TIME]) {
    table << (const char*)wxString(_("Time")).mb_str();
  }
  if (toPrintOut[PRINT_SPEED]) {
    table << (const char*)wxString(_("Speed")).mb_str();
  }

  table.StartFillWidths();

  table << 20;  // "Leg" column
  // setup widths for columns
  if (toPrintOut[PRINT_POSITION]) table << 80;
  if (toPrintOut[PRINT_BEARING]) table << 40;
  if (toPrintOut[PRINT_DISTANCE]) table << 40;
  if (toPrintOut[PRINT_TIME]) table << 60;
  if (toPrintOut[PRINT_SPEED]) table << 40;

  table.StartFillData();
  for (int n = 0; n <= myTrack->GetnPoints(); n++) {
    table << lcPoints->OnGetItemText(n, 0);  // leg

    if (toPrintOut[PRINT_POSITION]) {
      // lat + lon
      wxString pos = lcPoints->OnGetItemText(n, 3) + _T(" ") +
                     lcPoints->OnGetItemText(n, 4);
      table << pos;
    }
    if (toPrintOut[PRINT_BEARING])
      table << lcPoints->OnGetItemText(n, 2);  // bearing
    if (toPrintOut[PRINT_DISTANCE])
      table << lcPoints->OnGetItemText(n, 1);  // distance
    if (toPrintOut[PRINT_TIME]) table << lcPoints->OnGetItemText(n, 5);  // time
    if (toPrintOut[PRINT_SPEED])
      table << lcPoints->OnGetItemText(n, 6);  // speed
    table << "\n";
  }
}

void TrackPrintout::GetPageInfo(int* minPage, int* maxPage, int* selPageFrom,
                                int* selPageTo) {
  *minPage = 1;
  *maxPage = numberOfPages;
  *selPageFrom = 1;
  *selPageTo = numberOfPages;
}

void TrackPrintout::OnPreparePrinting() {
  pageToPrint = 1;
  wxDC* dc = GetDC();
  wxFont trackPrintFont(10, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL,
                        wxFONTWEIGHT_NORMAL);
  dc->SetFont(trackPrintFont);

  // Get the size of the DC in pixels
  int w, h;
  dc->GetSize(&w, &h);

  // We don't know before hand what size the Print DC will be, in pixels. Varies
  // by host. So, if the dc size is greater than 1000 pixels, we scale
  // accordinly.

  int maxX = wxMin(w, 1000);
  int maxY = wxMin(h, 1000);

  // Calculate a suitable scaling factor
  double scaleX = (double)(w / maxX);
  double scaleY = (double)(h / maxY);

  // Use x or y scaling factor, whichever fits on the DC
  double actualScale = wxMin(scaleX, scaleY);

  // Set the scale and origin
  dc->SetUserScale(actualScale, actualScale);
  dc->SetDeviceOrigin((long)marginX, (long)marginY);

  table.AdjustCells(dc, marginX, marginY);
  numberOfPages = table.GetNumberPages();
}

bool TrackPrintout::OnPrintPage(int page) {
  wxDC* dc = GetDC();
  if (dc) {
    if (page <= numberOfPages) {
      pageToPrint = page;
      DrawPage(dc);
      return true;
    } else
      return false;
  } else
    return false;
}

void TrackPrintout::DrawPage(wxDC* dc) {
  wxFont trackPrintFont_bold(10, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL,
                             wxFONTWEIGHT_BOLD);
  dc->SetFont(trackPrintFont_bold);
  wxBrush brush(wxColour(255, 255, 255), wxBRUSHSTYLE_TRANSPARENT);
  dc->SetBrush(brush);

  int header_textOffsetX = 2;
  int header_textOffsetY = 2;

  dc->DrawText(myTrack->GetName(), 150, 20);

  int currentX = marginX;
  int currentY = marginY;
  vector<PrintCell>& header_content = table.GetHeader();
  for (size_t j = 0; j < header_content.size(); j++) {
    PrintCell& cell = header_content[j];
    dc->DrawRectangle(currentX, currentY, cell.GetWidth(), cell.GetHeight());
    dc->DrawText(cell.GetText(), currentX + header_textOffsetX,
                 currentY + header_textOffsetY);
    currentX += cell.GetWidth();
  }

  wxFont trackPrintFont_normal(10, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL,
                               wxFONTWEIGHT_NORMAL);
  dc->SetFont(trackPrintFont_normal);

  vector<vector<PrintCell> >& cells = table.GetContent();
  currentY = marginY + table.GetHeaderHeight();
  int currentHeight = 0;
  for (size_t i = 0; i < cells.size(); i++) {
    vector<PrintCell>& content_row = cells[i];
    currentX = marginX;
    for (size_t j = 0; j < content_row.size(); j++) {
      PrintCell& cell = content_row[j];
      if (cell.GetPage() == pageToPrint) {
        wxRect r(currentX, currentY, cell.GetWidth(), cell.GetHeight());
        dc->DrawRectangle(r);
        r.Offset(textOffsetX, textOffsetY);
        dc->DrawLabel(cell.GetText(), r);
        currentX += cell.GetWidth();
        currentHeight = cell.GetHeight();
      }
    }
    currentY += currentHeight;
  }
}

/*!
 * Track print selection dialog implementation.
 */
TrackPrintSelection::TrackPrintSelection(wxWindow* parent, Track* _track,
                                         OCPNTrackListCtrl* lcPoints,
                                         const wxString& caption)
    : PrintSelectionDialog(parent, caption) {
  m_track = _track;
  m_lcPoints = lcPoints;

  // Create selection check-boxes.
  wxFlexGridSizer* group = CreateGroup();
  m_checkBoxPosition =
      AddCheckBox(group, "Position", "Show Waypoint position.", true);
  m_checkBoxCourse = AddCheckBox(
      group, "Course", "Show course from each Waypoint to the next one.", true);
  m_checkBoxDistance =
      AddCheckBox(group, "Distance",
                  "Show Distance from each Waypoint to the next one.", true);
  m_checkBoxTime = AddCheckBox(group, "Time", "Show Time", true);
  m_checkBoxTime = AddCheckBox(group, "Speed", "Show Speed", true);

  // Add the group to the dialog.
  wxStaticBoxSizer* sizer = AddGroup(group, "Elements to print...");
  Finalize(sizer);
}

/*
 * Should we show tooltips?
 */

bool TrackPrintSelection::ShowToolTips() { return TRUE; }

void TrackPrintSelection::OnOKClick(wxCommandEvent& event) {
  std::vector<bool> toPrintOut;
  toPrintOut.push_back(m_checkBoxPosition->GetValue());
  toPrintOut.push_back(m_checkBoxCourse->GetValue());
  toPrintOut.push_back(m_checkBoxDistance->GetValue());
  toPrintOut.push_back(m_checkBoxTime->GetValue());
  toPrintOut.push_back(m_checkBoxSpeed->GetValue());

  if (NULL == g_printData) {
    g_printData = new wxPrintData;
    g_printData->SetOrientation(wxPORTRAIT);
    g_pageSetupData = new wxPageSetupDialogData;
  }

  TrackPrintout* trackprintout =
      new TrackPrintout(toPrintOut, m_track, m_lcPoints, _("Track Print"));

  wxPrintDialogData printDialogData(*g_printData);
  printDialogData.EnablePageNumbers(true);

  wxPrinter printer(&printDialogData);
  if (!printer.Print(this, trackprintout, true)) {
    if (wxPrinter::GetLastError() == wxPRINTER_ERROR) {
      OCPNMessageBox(NULL,
                     _("There was a problem printing.\nPerhaps your current "
                       "printer is not set correctly?"),
                     _T( "OpenCPN" ), wxOK);
    }
  }

  Close();  // Hide();
  event.Skip();
}
