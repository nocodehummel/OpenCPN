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

#include <wx/print.h>

#include "ocpn_frame.h"
#include "ocpn_print.h"

// Global print data, to remember settings during the session
extern wxPrintData* g_printData;

/**
 * OpenCPNPrint constructor.
 */
OpenCPNPrint::OpenCPNPrint(const wxChar* title) : wxPrintout(title) {
  m_enablePageNumbers = false;
  m_numberOfPages = 1;
}

/*!
 * Set print orientation in global print data
 * with first print in the session.
 */
void OpenCPNPrint::SetOrientation(wxPrintOrientation orientation) {
  if (g_printData == NULL) {
    g_printData = new wxPrintData;
    g_printData->SetOrientation(orientation);
  }
}

/*!
 * Print the document.
 */
void OpenCPNPrint::Print(wxWindow* parent, bool dialog) {
  wxPrintDialogData printDialogData(*g_printData);
  printDialogData.EnablePageNumbers(m_enablePageNumbers);

  wxPrinter printer(&printDialogData);
  if (!printer.Print(parent, this, dialog)) {
    if (wxPrinter::GetLastError() == wxPRINTER_ERROR) {
      OCPNMessageBox(
          NULL,
          _("There was a problem printing.\nPerhaps your current printer is "
            "not set correctly?"),
          _T("OpenCPN"), wxOK);
    }
  }
}

bool OpenCPNPrint::OnBeginDocument(int startPage, int endPage) {
  if (!wxPrintout::OnBeginDocument(startPage, endPage)) return false;

  return true;
}

void OpenCPNPrint::GetPageInfo(int* minPage, int* maxPage, int* selPageFrom,
                               int* selPageTo) {
  *minPage = 1;
  *maxPage = m_numberOfPages;
  *selPageFrom = 1;
  *selPageTo = m_numberOfPages;
}

bool OpenCPNPrint::HasPage(int pageNum) {
  return pageNum > 0 && pageNum <= m_numberOfPages;
}