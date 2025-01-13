#ifndef OCPN_PRINT_DLG_H
#define OCPN_PRINT_DLG_H

#include <wx/dialog.h>
#include <wx/button.h>
#include <wx/statbox.h>
#include <wx/stattext.h>
#include <wx/checkbox.h>
#include <wx/sizer.h>

/*!
 * Print dialog base.
 */
class PrintDialog : public wxDialog {
  DECLARE_EVENT_TABLE()

public:
  // Constructors
  PrintDialog(wxWindow *parent, const wxString &caption);

  // Destructor to prevent leakage.
  ~PrintDialog();

  // Map with string keys and Person object values
  wxButton *m_CancelButton;
  wxButton *m_OKButton;

  // Functions to create the dialog.
  wxFlexGridSizer *CreateGroup();
  wxStaticBoxSizer *AddGroup(wxFlexGridSizer *grid, const wxString &title);
  wxCheckBox *AddCheckBox(wxFlexGridSizer *grid, const wxString label,
                          const wxString &description, bool checked = true);
  void Finalize(wxStaticBoxSizer *sizer);

  // Event handlers
  void OnCancelClick(wxCommandEvent &event);
  virtual void OnOKClick(wxCommandEvent &event) = 0;
};

#endif