#include "ocpn_print_dlg.h"
#include <navutil.h>

// Print selection dialog definitions.
#define ID_PRINTDIALOG 9000
#define SYMBOL_PRINTDIALOG_STYLE \
  wxCAPTION | wxRESIZE_BORDER | wxSYSTEM_MENU | wxCLOSE_BOX
#define SYMBOL_PRINTDIALOG_SIZE wxSize(750, 450)
#define SYMBOL_PRINTDIALOG_POSITION wxDefaultPosition
#define ID_PRINTDIALOG_OK 9001
#define ID_PRINTDIALOG_CANCEL 9002

/*!
 * Print dialog event table definition.
 */
BEGIN_EVENT_TABLE(PrintDialog, wxDialog)
EVT_BUTTON(ID_PRINTDIALOG_CANCEL, PrintDialog::OnCancelClick)
EVT_BUTTON(ID_PRINTDIALOG_OK, PrintDialog::OnOKClick)
END_EVENT_TABLE()

/*!
 * Print dialog implementation.
 */
PrintDialog::PrintDialog(wxWindow* parent, const wxString& caption) {
  long wstyle = SYMBOL_PRINTDIALOG_STYLE;
  SetExtraStyle(GetExtraStyle() | wxWS_EX_BLOCK_EVENTS);

#ifdef __WXOSX__
  style |= wxSTAY_ON_TOP;
#endif

  Create(parent, ID_PRINTDIALOG, caption, SYMBOL_PRINTDIALOG_POSITION,
         SYMBOL_PRINTDIALOG_SIZE, wstyle);
}

// Destructor to prevent leakage.
PrintDialog::~PrintDialog() {}

/*!
 * Create grid for print selection.
 */
wxFlexGridSizer* PrintDialog::CreateGroup() {
  return new wxFlexGridSizer(0, 2, 0, 0);
}

/*!
 * Add the group box to the dialog.
 */
wxStaticBoxSizer* PrintDialog::AddGroup(wxFlexGridSizer* grid,
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
wxCheckBox* PrintDialog::AddCheckBox(wxFlexGridSizer* grid,
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
void PrintDialog::Finalize(wxStaticBoxSizer* sizer) {
  wxBoxSizer* footer = new wxBoxSizer(wxHORIZONTAL);
  sizer->Add(footer, 0, wxALIGN_RIGHT | wxALL, 5);

  m_CancelButton = new wxButton(this, ID_PRINTDIALOG_CANCEL, _("Cancel"),
                                wxDefaultPosition, wxDefaultSize, 0);
  footer->Add(m_CancelButton, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);

  m_OKButton = new wxButton(this, ID_PRINTDIALOG_OK, _("OK"), wxDefaultPosition,
                            wxDefaultSize, 0);
  footer->Add(m_OKButton, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);
  m_OKButton->SetDefault();

  DimeControl(this);
  Centre();
}

/*!
 * Cancel event handler.
 */
void PrintDialog::OnCancelClick(wxCommandEvent& event) {
  Close();  // Hide();
  event.Skip();
}
