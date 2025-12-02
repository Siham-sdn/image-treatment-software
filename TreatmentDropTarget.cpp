// ============================================================================
// TreatmentDropTarget.cpp
// ============================================================================
// Implémentation du glisser-déposer
// ============================================================================

#include "TreatmentDropTarget.h"
#include "MainFrame.h"

TreatmentDropTarget::TreatmentDropTarget(MainFrame* frame)
    : wxTextDropTarget(), mainFrame(frame)
{
}

bool TreatmentDropTarget::OnDropText(wxCoord x, wxCoord y, const wxString& data)
{
    // Application du traitement déposé
    if (mainFrame) {
        mainFrame->ApplyDroppedProcessing(data.ToStdString());
        return true;
    }
    return false;
}