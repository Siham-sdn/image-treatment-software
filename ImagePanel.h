// ============================================================================
// ImagePanel.h
// ============================================================================
// Panneau d'affichage d'image avec redimensionnement automatique
// et support du thème sombre
// ============================================================================

#pragma once
#ifndef IMAGE_PANEL_H
#define IMAGE_PANEL_H

#include <wx/wx.h>
#include <opencv2/opencv.hpp>

class ImagePanel : public wxPanel
{
public:
    ImagePanel(wxWindow* parent);

    // Définit l'image à afficher (redimensionnement automatique)
    void SetImage(const cv::Mat& img);

    // Efface l'image affichée
    void ClearImage();

private:
    cv::Mat currentImage;       // Image source en format OpenCV
    wxBitmap displayBitmap;     // Bitmap redimensionné pour l'affichage

    // Met à jour l'affichage avec redimensionnement
    void UpdateDisplay();

    // Gestionnaires d'événements
    void OnPaint(wxPaintEvent& evt);
    void OnSize(wxSizeEvent& evt);
};

#endif // IMAGE_PANEL_H