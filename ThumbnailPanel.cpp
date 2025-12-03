// ============================================================================
// ThumbnailPanel.cpp
// ============================================================================
// Implémentation du panneau de miniatures
// ============================================================================

#include "ThumbnailPanel.h"
#include <wx/sizer.h>
#include <wx/statbmp.h>

ThumbnailPanel::ThumbnailPanel(wxWindow* parent, int thumbWidth, int spacing)
    : wxScrolledWindow(parent, wxID_ANY), thumbWidth(thumbWidth), spacing(spacing)
{
    // Thème futuriste
    SetBackgroundColour(wxColour(10, 14, 30));

    // Layout vertical pour empiler les miniatures
    sizer = new wxBoxSizer(wxVERTICAL);
    SetSizer(sizer);

    // Configuration du scroll (5 pixels par unité)
    SetScrollRate(5, 5);
}

ThumbnailPanel::~ThumbnailPanel()
{
    // Nettoyage (widgets détruits automatiquement par wxWidgets)
    images.clear();
    widgets.clear();
}

wxBitmap ThumbnailPanel::MatToWxBitmap(const cv::Mat& src)
{
    if (src.empty()) return wxBitmap();

    // ---------- CONVERSION VERS RGB ----------
    cv::Mat rgb;
    if (src.channels() == 1) {
        cv::cvtColor(src, rgb, cv::COLOR_GRAY2RGB);
    }
    else if (src.channels() == 4) {
        cv::cvtColor(src, rgb, cv::COLOR_BGRA2RGB);
    }
    else {
        cv::cvtColor(src, rgb, cv::COLOR_BGR2RGB);
    }

    // ---------- REDIMENSIONNEMENT PROPORTIONNEL ----------
    int w = rgb.cols;
    int h = rgb.rows;
    double scale = (double)thumbWidth / (double)w;
    int newW = std::max(1, (int)round(w * scale));
    int newH = std::max(1, (int)round(h * scale));

    cv::Mat resized;
    cv::resize(rgb, resized, cv::Size(newW, newH), 0, 0, cv::INTER_AREA);

    // ---------- CRÉATION DU WXBITMAP ----------
    wxImage img(resized.cols, resized.rows);
    unsigned char* data = img.GetData();
    size_t bytes = (size_t)resized.cols * resized.rows * 3;
    memcpy(data, resized.data, bytes);

    return wxBitmap(img);
}

void ThumbnailPanel::AddThumbnail(const cv::Mat& img)
{
    if (img.empty()) return;

    // Sauvegarde de l'image originale
    images.push_back(img.clone());

    // Création du bitmap miniature
    wxBitmap bmp = MatToWxBitmap(img);

    // ---------- CRÉATION DU CONTENEUR AVEC BORDURE ----------
    wxPanel* container = new wxPanel(this, wxID_ANY);
    container->SetBackgroundColour(wxColour(15, 20, 40));

    wxBoxSizer* containerSizer = new wxBoxSizer(wxVERTICAL);

    // Widget d'affichage
    wxStaticBitmap* sb = new wxStaticBitmap(container, wxID_ANY, bmp);
    containerSizer->Add(sb, 1, wxALL | wxEXPAND, 4);

    container->SetSizer(containerSizer);

    // ---------- GESTION DU CLIC ----------
    auto clickHandler = [this, sb](wxMouseEvent& ev) {
        // Recherche de l'index de la miniature cliquée
        for (size_t i = 0; i < widgets.size(); ++i) {
            if (widgets[i] == sb) {
                if (onClick) onClick(i);
                break;
            }
        }
        ev.Skip();
        };

    sb->Bind(wxEVT_LEFT_DOWN, clickHandler);
    container->Bind(wxEVT_LEFT_DOWN, clickHandler);

    // ---------- AJOUT AU LAYOUT ----------
    widgets.push_back(sb);
    sizer->Add(container, 0, wxALL | wxEXPAND, spacing);

    // Mise à jour optimisée avec Freeze/Thaw
    Freeze();
    Layout();
    FitInside();  // Ajuste la zone de scroll
    Thaw();

    // Scroll automatique vers le bas
    int x, y;
    GetViewStart(&x, &y);
    int xu, yu;
    GetScrollPixelsPerUnit(&xu, &yu);
    if (yu > 0) {
        Scroll(x, sizer->GetSize().GetHeight() / yu);
    }
}

void ThumbnailPanel::Clear()
{
    // Efface toutes les images
    images.clear();

    // Optimisation : gel pendant le nettoyage
    Freeze();

    // Destruction de tous les widgets
    for (auto w : widgets) {
        if (w) {
            wxWindow* parent = w->GetParent();
            if (parent) {
                parent->Destroy();
            }
        }
    }
    widgets.clear();

    // Nettoyage du sizer
    sizer->Clear(true);

    // Mise à jour
    Layout();
    FitInside();
    Scroll(0, 0);

    Thaw();
}

void ThumbnailPanel::SetOnThumbClick(std::function<void(size_t)> cb)
{
    onClick = cb;
}

cv::Mat ThumbnailPanel::GetImageAt(size_t index) const
{
    if (index >= images.size()) return cv::Mat();
    return images[index].clone();
}
