// ============================================================================
// ImageProcessingApp.h
// ============================================================================
// Classe principale de l'application wxWidgets
// Point d'entrée du programme
// ============================================================================

#pragma once
#ifndef IMAGE_PROCESSING_APP_H
#define IMAGE_PROCESSING_APP_H

#include <wx/wx.h>

class ImageProcessingApp : public wxApp
{
public:
    // Initialisation de l'application
    virtual bool OnInit() override;
};

#endif // IMAGE_PROCESSING_APP_H