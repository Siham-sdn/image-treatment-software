// ============================================================================
// ImagePanel.cpp
// ============================================================================
// Implémentation du panneau d'affichage d'image
// ============================================================================

#include "ImagePanel.h"

ImagePanel::ImagePanel(wxWindow* parent) : wxPanel(parent)
{
    // Thème futuriste
    SetBackgroundColour(wxColour(5, 8, 20));

    // Connexion des événements
    Bind(wxEVT_PAINT, &ImagePanel::OnPaint, this);
    Bind(wxEVT_SIZE, &ImagePanel::OnSize, this);
}

void ImagePanel::SetImage(const cv::Mat& img)
{
    if (img.empty()) return;

    // Clone pour éviter les problèmes de mémoire
    currentImage = img.clone();
    UpdateDisplay();
}

void ImagePanel::ClearImage()
{
    currentImage.release();
    displayBitmap = wxBitmap();
    Refresh();
}

void ImagePanel::UpdateDisplay()
{
    if (currentImage.empty()) return;

    // ---------- CONVERSION OPENCV → WXIMAGE ----------
    cv::Mat rgb;
    if (currentImage.channels() == 1) {
        // Image en niveaux de gris
        cv::cvtColor(currentImage, rgb, cv::COLOR_GRAY2RGB);
    }
    else if (currentImage.channels() == 4) {
        // Image avec canal alpha
        cv::cvtColor(currentImage, rgb, cv::COLOR_BGRA2RGB);
    }
    else {
        // Image couleur standard (BGR → RGB)
        cv::cvtColor(currentImage, rgb, cv::COLOR_BGR2RGB);
    }

    // ---------- REDIMENSIONNEMENT PROPORTIONNEL ----------
    wxSize panelSize = GetSize();
    if (panelSize.GetWidth() > 0 && panelSize.GetHeight() > 0)
    {
        // Calcul du facteur d'échelle pour conserver le ratio
        double scaleX = (double)panelSize.GetWidth() / rgb.cols;
        double scaleY = (double)panelSize.GetHeight() / rgb.rows;
        double scale = std::min(scaleX, scaleY) * 0.95;  // 95% pour marges

        // Application du redimensionnement
        cv::Mat resized;
        cv::resize(rgb, resized, cv::Size(), scale, scale, cv::INTER_LINEAR);

        // Création du wxBitmap
        wxImage wxImg(resized.cols, resized.rows, resized.data, true);
        displayBitmap = wxBitmap(wxImg);
    }

    // Rafraîchissement de l'affichage
    Refresh();
    Update();
}

void ImagePanel::OnPaint(wxPaintEvent& evt)
{
    wxPaintDC dc(this);

    if (displayBitmap.IsOk())
    {
        // Centrage de l'image dans le panneau
        wxSize panelSize = GetSize();
        int x = (panelSize.GetWidth() - displayBitmap.GetWidth()) / 2;
        int y = (panelSize.GetHeight() - displayBitmap.GetHeight()) / 2;
        dc.DrawBitmap(displayBitmap, x, y, false);
    }
    else
    {
        // Message si aucune image n'est chargée
        dc.SetTextForeground(wxColour(150, 150, 150));
        wxFont font = dc.GetFont();
        font.SetPointSize(12);
        dc.SetFont(font);

        wxString text = "Aucune image chargée\n\nChargez une image/vidéo et glissez un traitement ici";
        wxSize textSize = dc.GetTextExtent(text);
        wxSize panelSize = GetSize();

        dc.DrawText(text,
            (panelSize.GetWidth() - textSize.GetWidth()) / 2,
            (panelSize.GetHeight() - textSize.GetHeight()) / 2);
    }
}

void ImagePanel::OnSize(wxSizeEvent& evt)
{
    // Redimensionner l'image quand la fenêtre change de taille
    if (!currentImage.empty()) {
        UpdateDisplay();
    }
    evt.Skip();
}
