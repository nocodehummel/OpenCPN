/***************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  OpenCPN Track printout
 * Author:   Pavel Saviankou, Sean D'Epagnier
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

#ifndef __TRACKPRINTOUT_H__
#define __TRACKPRINTOUT_H__

#include <wx/print.h>
#include <wx/datetime.h>
#include <wx/cmdline.h>

#ifdef __WXMSW__
#include <wx/msw/private.h>
#endif

#include "model/ocpn_types.h"
#include "navutil.h"
#include "TrackPropDlg.h"
#include "printtable.h"
#include "ocpn_print.h"
#include "ocpn_print_dlg.h"
#include "ocpn_frame.h"

class TrackPrintout : public OpenCPNPrint {
public:
  TrackPrintout(std::vector<bool> _toPrintOut, Track* track,
                OCPNTrackListCtrl* lcPoints,
                const wxString& title = _T( "My Track printout" ));
  virtual bool OnPrintPage(int page);
  void DrawPage(wxDC* dc);
  virtual void OnPreparePrinting();
  virtual bool HasPage(int num) { return num > 0 && num <= numberOfPages; };

  virtual void GetPageInfo(int* minPage, int* maxPage, int* selPageFrom,
                           int* selPageTo);

protected:
  wxDC* myDC;
  PrintTable table;
  Track* myTrack;
  std::vector<bool> toPrintOut;  // list of fields of bool, if certain element
                                 // should be print out.
  static const int pN = 5;       // number of fields sofar
  int pageToPrint;
  int numberOfPages;
  int marginX;
  int marginY;
  int textOffsetX;
  int textOffsetY;
};

#define SYMBOL_TRACKPRINT_SELECTION_TITLE _("Print Route Selection")

/*!
 * Track print selection dialog implementation.
 */
class TrackPrintDialog : public PrintDialog {
public:
  // Constructor
  TrackPrintDialog(wxWindow* parent, Track* track, OCPNTrackListCtrl* lcPoints,
                   const wxString& caption = SYMBOL_TRACKPRINT_SELECTION_TITLE);

  // Should we show tooltips?
  static bool ShowToolTips();

  void OnOKClick(wxCommandEvent& event) override;

private:
  wxCheckBox* m_checkBoxPosition;
  wxCheckBox* m_checkBoxCourse;
  wxCheckBox* m_checkBoxDistance;
  wxCheckBox* m_checkBoxTime;
  wxCheckBox* m_checkBoxSpeed;

  Track* m_track;
  OCPNTrackListCtrl* m_lcPoints;
};

#endif
