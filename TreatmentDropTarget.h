// ============================================================================
// TreatmentDropTarget.h
// ============================================================================
// Classe pour gérer le glisser-déposer des traitements
// ============================================================================

#pragma once
#ifndef TREATMENT_DROP_TARGET_H
#define TREATMENT_DROP_TARGET_H

#include <wx/wx.h>
#include <wx/dnd.h>

// Forward declaration
class MainFrame;

class TreatmentDropTarget : public wxTextDropTarget
{
public:
    explicit TreatmentDropTarget(MainFrame* frame);

    // Appelé quand du texte est déposé sur la cible
    virtual bool OnDropText(wxCoord x, wxCoord y, const wxString& data) override;

private:
    MainFrame* mainFrame;
};

#endif // TREATMENT_DROP_TARGET_H