// ============================================================================
// ImageProcessingApp.cpp
// ============================================================================
// Implémentation de l'application principale
// ============================================================================

#include "ImageProcessingApp.h"
#include "MainFrame.h"

bool ImageProcessingApp::OnInit()
{
    // Création et affichage de la fenêtre principale
    MainFrame* frame = new MainFrame();
    frame->Show(true);

    return true;
}

// Macro wxWidgets pour créer le point d'entrée
wxIMPLEMENT_APP(ImageProcessingApp);