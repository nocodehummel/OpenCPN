/***************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  OpenCPN Route printout
 * Author:   Pavel Saviankou
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

#include <iostream>

#ifndef __ROUTEPRINTOUT_H__
#define __ROUTEPRINTOUT_H__

#include <wx/print.h>
#include <wx/datetime.h>
#include <wx/cmdline.h>

#ifdef __WXMSW__
#include <wx/msw/private.h>
#endif

#include "model/ocpn_types.h"
#include "navutil.h"
#include "printtable.h"
#include "ocpn_print.h"
#include "ocpn_print_dlg.h"
#include "ocpn_frame.h"

class RoutePrintout : public OpenCPNPrint {
public:
  RoutePrintout(std::vector<bool> _toPrintOut, Route* route,
                const wxString& title = _T( "My Route printout" ));
  virtual bool OnPrintPage(int page);
  void DrawPage(wxDC* dc);
  virtual void OnPreparePrinting();

  virtual bool HasPage(int num) { return num > 0 && num <= numberOfPages; };

  virtual void GetPageInfo(int* minPage, int* maxPage, int* selPageFrom,
                           int* selPageTo);

protected:
  wxDC* myDC;
  PrintTable table;
  Route* myRoute;
  std::vector<bool> toPrintOut;  // list of fields of bool, if certain element
                                 // should be print out.
  static const int pN = 5;       // number of fields sofar
  int pageToPrint;
  int numberOfPages;
  int pageSizeX;
  int pageSizeY;
  int marginX;
  int marginY;
  int textOffsetX;
  int textOffsetY;
};

#define SYMBOL_ROUTEPRINT_SELECTION_TITLE _("Print Route Selection")

/**
 * Route print selection dialog implementation.
 */
class RoutePrintDialog : public PrintDialog {
public:
  // Constructor
  RoutePrintDialog(wxWindow* parent, Route* route,
                   const wxString& caption = SYMBOL_ROUTEPRINT_SELECTION_TITLE);

  // Destructor
  ~RoutePrintDialog();

  // Should we show tooltips?
  static bool ShowToolTips();

  void OnOKClick(wxCommandEvent& event) override;

private:
  wxCheckBox* m_checkBoxWPName;
  wxCheckBox* m_checkBoxWPPosition;
  wxCheckBox* m_checkBoxWPCourse;
  wxCheckBox* m_checkBoxWPDistanceToNext;
  wxCheckBox* m_checkBoxWPDescription;
  wxCheckBox* m_checkBoxWPSpeed;
  wxCheckBox* m_checkBoxWPETA;
  wxCheckBox* m_checkBoxWPTide;

  Route* m_route;
};

#endif
