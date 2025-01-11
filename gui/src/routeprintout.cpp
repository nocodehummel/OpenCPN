/***************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  OpenCPN Route printout
 * Author:   Pavel Saviankou
 *
 ***************************************************************************
 *   Copyright (C) 2012 by David S. Register                               *
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
#include <sstream>
#include <iomanip>

#include <wx/wxprec.h>

#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif  // precompiled headers
#ifdef __WXMSW__
// #include "c:\\Program Files\\visual leak detector\\include\\vld.h"
#endif

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

#include "navutil.h"
#include "dychart.h"

#ifdef __WXMSW__
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <psapi.h>
#endif

#ifndef __WXMSW__
#include <signal.h>
#include <setjmp.h>
#endif

#include "tcmgr.h"
#include "routeprintout.h"
#include "printtable.h"
#include "model/navutil_base.h"
#include "model/wx28compat.h"
#include "model/track.h"
#include "model/route.h"
#include "gui_lib.h"

#define PRINT_WP_NAME 0
#define PRINT_WP_POSITION 1
#define PRINT_WP_COURSE 2
#define PRINT_WP_DISTANCE 3
#define PRINT_WP_DESCRIPTION 4
#define PRINT_WP_SPEED 5
#define PRINT_WP_ETA 6
#define PRINT_WP_TIDE 7

using namespace std;

// Global print data, to remember settings during the session
extern wxPrintData* g_printData;
// Global page setup data
extern wxPageSetupData* g_pageSetupData;
// Global Tide and Current Manager
extern TCMgr* ptcmgr;

RoutePrintout::RoutePrintout(std::vector<bool> _toPrintOut, Route* route,
                             const wxString& title)
    : OpenCPNPrint(title), myRoute(route), toPrintOut(_toPrintOut) {
  // Let's have at least some device units margin
  marginX = 100;
  marginY = 100;

  // Offset text from the edge of the cell (Needed on Linux)
  textOffsetX = 5;
  textOffsetY = 8;

  // setup table headers
  table.StartFillHeader();

  table << _("Leg");

  if (toPrintOut[PRINT_WP_NAME]) {
    table << _("Destination");
  }
  if (toPrintOut[PRINT_WP_POSITION]) {
    table << _("Position");
  }
  if (toPrintOut[PRINT_WP_COURSE]) {
    table << _("Course");
  }
  if (toPrintOut[PRINT_WP_DISTANCE]) {
    table << _("Distance");
  }
  if (toPrintOut[PRINT_WP_SPEED]) {
    table << _("Speed");
  }
  if (toPrintOut[PRINT_WP_ETA]) {
    std::wostringstream eta_str;
    eta_str << _("ETA").wc_str();
    eta_str << " (" << myRoute->m_TimeDisplayFormat.wc_str() << ")";
    table << eta_str.str();
  }
  if (toPrintOut[PRINT_WP_TIDE]) {
    std::wostringstream tide_str;
    tide_str << _("Tide").wc_str();
    tide_str << " (" << myRoute->m_TimeDisplayFormat.wc_str() << ")";
    table << tide_str.str();
  }
  if (toPrintOut[PRINT_WP_DESCRIPTION]) {
    table << _("Description");
  }

  // setup widths for columns
  table.StartFillWidths();
  table << 20;  // "Leg" column

  if (toPrintOut[PRINT_WP_NAME]) {
    table << 80;
  }
  if (toPrintOut[PRINT_WP_POSITION]) {
    table << 60;
  }
  if (toPrintOut[PRINT_WP_COURSE]) {
    table << 50;
  }
  if (toPrintOut[PRINT_WP_DISTANCE]) {
    table << 60;
  }
  if (toPrintOut[PRINT_WP_SPEED]) {
    table << 40;
  }
  if (toPrintOut[PRINT_WP_ETA]) {
    table << 80;
  }
  if (toPrintOut[PRINT_WP_TIDE]) {
    table << 120;
  }
  if (toPrintOut[PRINT_WP_DESCRIPTION]) {
    table << 150;
  }

  table.StartFillData();

  for (int n = 1; n <= myRoute->GetnPoints(); n++) {
    RoutePoint* point = myRoute->GetPoint(n);

    RoutePoint* pointm1 = NULL;
    if (n - 1 >= 0) pointm1 = myRoute->GetPoint(n - 1);

    if (NULL == point) continue;

    std::ostringstream leg;
    if (n > 1) {
      leg << n - 1;
    } else {
      leg << "---";
    }

    string cell(leg.str());
    table << cell;

    if (toPrintOut[PRINT_WP_NAME]) {
      string cell(point->GetName().mb_str());
      table << cell;
    }
    if (toPrintOut[PRINT_WP_POSITION]) {
      std::wostringstream point_position;
      point_position << toSDMM(1, point->m_lat, false).wc_str() << "\n"
                     << toSDMM(2, point->m_lon, false).wc_str();
      table << point_position.str();
    }
    if (toPrintOut[PRINT_WP_COURSE]) {
      std::wostringstream point_course;
      if (pointm1) {
        point_course << formatAngle(point->GetCourse()).wc_str();
      } else {
        point_course << "---";
      }
      table << point_course.str();
    }
    if (toPrintOut[PRINT_WP_DISTANCE]) {
      std::wostringstream point_distance;
      if (n > 1) {
        point_distance << std::fixed << std::setprecision(2)
                       << toUsrDistance(point->GetDistance())
                       << getUsrDistanceUnit().wc_str();
      } else {
        point_distance << "---";
      }
      table << point_distance.str();
    }
    if (toPrintOut[PRINT_WP_SPEED]) {
      std::wostringstream point_speed;
      if (n > 1) {
        point_speed << std::fixed << std::setprecision(1);
        if (point->GetPlannedSpeed() > 0.1) {
          point_speed << toUsrSpeed(point->GetPlannedSpeed());
        } else {
          point_speed << toUsrSpeed(myRoute->m_PlannedSpeed);
        }
        point_speed << getUsrSpeedUnit().wc_str();
      } else {
        point_speed << "---";
      }
      table << point_speed.str();
    }

    if (toPrintOut[PRINT_WP_ETA]) {
      std::wostringstream point_eta;
      if (n == 1) {
        point_eta << _("Start").wc_str() << ": ";
      }
      int daylight =
          getDaylightStatus(point->m_lat, point->m_lon, point->GetETA());
      point_eta << toUsrDateTime(point->GetETA(), myRoute->m_TimeDisplayFormat,
                                 point->m_lon)
                       .Format(DT_FORMAT_ISO);
      point_eta << "\n(" << GetDaylightString(daylight).wc_str() << ")";

      table << point_eta.str();
    }
    if (toPrintOut[PRINT_WP_TIDE]) {
      std::wostringstream point_tide;

      if (point->m_TideStation.Len() > 0) {
        TideEvent tide_event =
            ptcmgr->GetTideEvent(point->m_TideStation.wc_str(), point->GetETA(),
                                 point->m_lat, point->m_lon);
        point_tide << tide_event.GetEventStr(
                          myRoute->m_TimeDisplayFormat.wc_str(), DT_FORMAT_ISO)
                   << "\n"
                   << tide_event.GetLocalOffsetStr(
                          myRoute->m_TimeDisplayFormat.wc_str())
                   << " @" << tide_event.m_station_name;
      }
      table << point_tide.str();
    }
    if (toPrintOut[PRINT_WP_DESCRIPTION]) {
      table << point->GetDescription().wc_str();
    }
    table << "\n";
  }
}

void RoutePrintout::GetPageInfo(int* minPage, int* maxPage, int* selPageFrom,
                                int* selPageTo) {
  *minPage = 1;
  *maxPage = numberOfPages;
  *selPageFrom = 1;
  *selPageTo = numberOfPages;
}

void RoutePrintout::OnPreparePrinting() {
  pageToPrint = 1;
  wxDC* dc = GetDC();
  wxFont routePrintFont(10, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL,
                        wxFONTWEIGHT_NORMAL);
  dc->SetFont(routePrintFont);

  // Get the size of the DC in pixels
  dc->GetSize(&pageSizeX, &pageSizeY);

  // We don't know before hand what size the Print DC will be, in pixels.
  // Varies by host. So, if the dc size is greater than 1000 pixels, we scale
  // accordinly.

  int maxX = wxMin(pageSizeX, 1000);
  int maxY = wxMin(pageSizeY, 1000);

  // Calculate a suitable scaling factor
  double scaleX = (double)(pageSizeX / maxX);
  double scaleY = (double)(pageSizeY / maxY);

  // Use x or y scaling factor, whichever fits on the DC
  double actualScale = wxMin(scaleX, scaleY);
  pageSizeX = (pageSizeX / actualScale) - (2 * marginX);
  pageSizeY = (pageSizeY / actualScale) - (2 * marginY);

  // Set the scale and origin
  dc->SetUserScale(actualScale, actualScale);
  dc->SetDeviceOrigin((long)marginX, (long)marginY);

  table.AdjustCells(dc, marginX, marginY);
  numberOfPages = table.GetNumberPages();
}

bool RoutePrintout::OnPrintPage(int page) {
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

void RoutePrintout::DrawPage(wxDC* dc) {
  wxFont title_font(16, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL,
                    wxFONTWEIGHT_BOLD);
  wxFont subtitle_font(12, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL,
                       wxFONTWEIGHT_NORMAL);
  wxFont header_font(10, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL,
                     wxFONTWEIGHT_BOLD);
  wxFont normal_font(10, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL,
                     wxFONTWEIGHT_NORMAL);

  wxBrush brush(wxColour(255, 255, 255), wxBRUSHSTYLE_TRANSPARENT);
  dc->SetBrush(brush);

  int currentX = marginX;
  int currentY = marginY;

  std::wostringstream title;
  std::wostringstream subtitle;
  std::wostringstream distance;

  title << myRoute->m_RouteNameString.wc_str();
  distance << std::fixed << std::setprecision(1) << "("
           << toUsrDistance(myRoute->m_route_length)
           << getUsrDistanceUnit().wc_str() << ")";

  if (myRoute->m_RouteStartString.Trim().Len() > 0) {
    subtitle << _("From").wc_str() << " "
             << myRoute->m_RouteStartString.wc_str();
    if (myRoute->m_RouteEndString.Trim().Len() > 0) {
      subtitle << " " << _("To").wc_str() << " "
               << myRoute->m_RouteEndString.wc_str();
    }
    subtitle << " " << distance.str();
  } else if (myRoute->m_RouteEndString.Trim().Len() > 0) {
    subtitle << _("Destination").wc_str() << ": "
             << myRoute->m_RouteEndString.wc_str();
    subtitle << " " << distance.str();
  } else {
    title << " " << distance.str();
  }

  int title_width, title_height;
  dc->SetFont(title_font);
  dc->GetTextExtent(title.str(), &title_width, &title_height);
  dc->DrawText(title.str(), currentX, currentY);
  currentY += title_height;

  if (subtitle.str().length() > 0) {
    currentY += 2;  // add top margin
    int subtitle_width, subtitle_height;
    dc->SetFont(subtitle_font);
    dc->GetTextExtent(subtitle.str(), &subtitle_width, &subtitle_height);
    dc->DrawText(subtitle.str(), currentX, currentY);
    currentY += subtitle_height;
  }

  // Route description on page 1.
  if (pageToPrint == 1 && myRoute->m_RouteDescription.Trim().Len() > 0) {
    currentY += 10;  // add top margin
    dc->SetFont(normal_font);
    PrintCell cell_desc;
    cell_desc.Init(myRoute->m_RouteDescription, dc, pageSizeX, marginX);
    dc->DrawText(cell_desc.GetText(), currentX, currentY);
    currentY += cell_desc.GetHeight();
  }

  int header_textOffsetX = 2;
  int header_textOffsetY = 2;
  dc->SetFont(header_font);

  currentY += 20;  // add top margin
  vector<PrintCell>& header_content = table.GetHeader();
  for (size_t j = 0; j < header_content.size(); j++) {
    PrintCell& cell = header_content[j];
    dc->DrawRectangle(currentX, currentY, cell.GetWidth(), cell.GetHeight());
    dc->DrawText(cell.GetText(), currentX + header_textOffsetX,
                 currentY + header_textOffsetY);
    currentX += cell.GetWidth();
  }

  dc->SetFont(normal_font);

  vector<vector<PrintCell> >& cells = table.GetContent();
  currentY += table.GetHeaderHeight();
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

// ---------- RoutePrintSelection dialof implementation

/*!
 * RouteProp constructors
 */
RoutePrintSelection::RoutePrintSelection(wxWindow* parent, Route* route,
                                         const wxString& caption)
    : PrintSelectionDialog(parent, caption) {
  m_route = route;

  // Create selection check-boxes.
  wxFlexGridSizer* group = CreateGroup();
  m_checkBoxWPName = AddCheckBox(group, "Name", "Show Waypoint name.", true);
  m_checkBoxWPPosition =
      AddCheckBox(group, "Position", "Show Waypoint position.", true);
  m_checkBoxWPCourse = AddCheckBox(
      group, "Course", "Show course from each Waypoint to the next one.", true);
  m_checkBoxWPDistanceToNext =
      AddCheckBox(group, "Distance",
                  "Show Distance from each Waypoint to the next one.", true);
  m_checkBoxWPDescription =
      AddCheckBox(group, "Description", "Show Waypoint description.", true);
  m_checkBoxWPSpeed =
      AddCheckBox(group, "Speed", "Show planned speed to Waypoint.", true);
  m_checkBoxWPETA =
      AddCheckBox(group, "ETA", "Show Estimated Time of Arrival.", true);
  m_checkBoxWPTide = AddCheckBox(group, "Tide event",
                                 "Show next tide event at station.", true);

  // Add the group to the dialog.
  wxStaticBoxSizer* sizer = AddGroup(group, "Waypoint elements to print...");
  Finalize(sizer);
}

RoutePrintSelection::~RoutePrintSelection() {}

/*
 * Should we show tooltips?
 */

bool RoutePrintSelection::ShowToolTips() { return TRUE; }

void RoutePrintSelection::OnOKClick(wxCommandEvent& event) {
  std::vector<bool> toPrintOut;
  toPrintOut.push_back(m_checkBoxWPName->GetValue());
  toPrintOut.push_back(m_checkBoxWPPosition->GetValue());
  toPrintOut.push_back(m_checkBoxWPCourse->GetValue());
  toPrintOut.push_back(m_checkBoxWPDistanceToNext->GetValue());
  toPrintOut.push_back(m_checkBoxWPDescription->GetValue());
  toPrintOut.push_back(m_checkBoxWPSpeed->GetValue());
  toPrintOut.push_back(m_checkBoxWPETA->GetValue());
  toPrintOut.push_back(m_checkBoxWPTide->GetValue());

  if (NULL == g_printData) {
    g_printData = new wxPrintData;

    // Elements to print
    int sum = std::count(toPrintOut.begin(), toPrintOut.end(), true);
    if (sum > 5) {
      g_printData->SetOrientation(wxLANDSCAPE);
    } else {
      g_printData->SetOrientation(wxPORTRAIT);
    }

    g_pageSetupData = new wxPageSetupDialogData;
  }

  RoutePrintout* routeprintout =
      new RoutePrintout(toPrintOut, m_route, _("Route Print"));

  wxPrintDialogData printDialogData(*g_printData);
  printDialogData.EnablePageNumbers(true);

  wxPrinter printer(&printDialogData);
  if (!printer.Print(this, routeprintout, true)) {
    if (wxPrinter::GetLastError() == wxPRINTER_ERROR) {
      OCPNMessageBox(NULL,
                     _("There was a problem printing.\nPerhaps your current "
                       "printer is not set correctly?"),
                     _T( "OpenCPN" ), wxOK);
    }
  }

  Close();
  event.Skip();
}
