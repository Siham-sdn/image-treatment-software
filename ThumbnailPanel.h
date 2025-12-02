// ============================================================================
// ThumbnailPanel.h
// ============================================================================
// Panneau scrollable affichant les miniatures du pipeline
// Permet de cliquer sur une miniature pour l'afficher en grand
// ============================================================================

#pragma once
#ifndef THUMBNAIL_PANEL_H
#define THUMBNAIL_PANEL_H

#include <wx/wx.h>
#include <wx/scrolwin.h>
#include <opencv2/opencv.hpp>
#include <vector>
#include <functional>

class ThumbnailPanel : public wxScrolledWindow
{
public:
    ThumbnailPanel(wxWindow* parent, int thumbWidth = 200, int spacing = 10);
    ~ThumbnailPanel();

    // Ajoute une miniature au panneau
    void AddThumbnail(const cv::Mat& img);

    // Efface toutes les miniatures
    void Clear();

    // Définit le callback lors du clic sur une miniature
    void SetOnThumbClick(std::function<void(size_t)> cb);

    // Récupère l'image originale à partir de son index
    cv::Mat GetImageAt(size_t index) const;

private:
    int thumbWidth;                             // Largeur des miniatures
    int spacing;                                // Espacement entre miniatures
    wxBoxSizer* sizer;                          // Layout vertical
    std::vector<cv::Mat> images;                // Images originales
    std::vector<wxStaticBitmap*> widgets;       // Widgets d'affichage
    std::function<void(size_t)> onClick;        // Callback de clic

    // Convertit une Mat OpenCV en wxBitmap pour l'affichage
    wxBitmap MatToWxBitmap(const cv::Mat& src);
};

#endif // THUMBNAIL_PANEL_H
