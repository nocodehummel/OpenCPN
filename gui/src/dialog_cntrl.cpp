/***************************************************************************
 *   Copyright (C) 2022 by David S. Register                               *
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
 ***************************************************************************
 */

#include "dialog_cntrl.h"
#include "model/gui.h"

// Text field constructor
TextField::TextField(wxWindow* parent, wxWindowID id, const wxString& value,
                     const wxPoint& pos, const wxSize& size, long style)
    : wxTextCtrl(new wxPanel(parent), id, value, pos, size, style) {
  m_errorText = nullptr;

  // Create panel as
  wxPanel* panel = dynamic_cast<wxPanel*>(GetParent());
  wxBoxSizer* nameSizer = new wxBoxSizer(wxVERTICAL);
  nameSizer->Add(this, 0, wxEXPAND);
  panel->SetSizer(nameSizer);
}

// Get the position index within a sizer.
int TextField::GetSizerIndex(wxSizer* sizer) {
  if (sizer) {
    for (size_t i = 0; i < sizer->GetItemCount(); ++i) {
      wxSizerItem* item = sizer->GetItem(i);
      if (item->GetWindow() == this) {
        return static_cast<int>(i);
      }
    }
  }
  return wxNOT_FOUND;
}

/**
 * Error handler to show an error with the text field.
 * When the sizer position is found the error is shown
 * under the text field, otherwise with a popup message.
 */
void TextField::onError(const wxString& msg = wxEmptyString) {
  if (m_errorText != nullptr) {
    if (msg == wxEmptyString) {
      wxSizer* sizer = GetParent()->GetSizer();
      sizer->Detach(m_errorText);
      m_errorText = nullptr;
      PropagateResize(this);
    } else {
      m_errorText->SetLabel(msg);
      Refresh();
    }
  } else {
    if (msg == wxEmptyString) {
      SetBackgroundColour(*wxWHITE);
      Refresh();

    } else {
      wxSizer* sizer = GetParent()->GetSizer();
      int index = GetSizerIndex(sizer);  // field position in sizer

      if (sizer && index >= 0) {
        m_errorText =
            new wxStaticText(GetParent(), wxID_ANY, msg, wxDefaultPosition);
        m_errorText->SetForegroundColour(*wxRED);

        sizer->Insert(index + 1, m_errorText, 0, wxALL | wxEXPAND, 4);
        PropagateResize(this);
      } else {
        wxMessageDialog popup(this, msg, "Error", wxOK | wxICON_ERROR);
        popup.ShowModal();
        SetBackgroundColour(*wxRED);
        Refresh();
      }
    }
  }
}

// Text changed event handler validates the text.
void TextField::OnTextChanged(wxCommandEvent& event) {
  TextField* textCtrl = dynamic_cast<TextField*>(event.GetEventObject());
  if (textCtrl) {
    textCtrl->Validate();
  }
}

/**
 * Text field validation with error handling.
 */
bool TextValidator::Validate(wxWindow* parent) {
  TextField* text_field = dynamic_cast<TextField*>(GetWindow());

  if (text_field) {
    wxString err = IsValid(text_field->GetValue());
    text_field->onError(err);
    return err.IsEmpty();
  } else
    return true;
}
