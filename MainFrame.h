// ============================================================================
// MainFrame.h
// ============================================================================
// Fenêtre principale de l'application de traitement d'images et vidéos
// Gère le pipeline visuel, les miniatures, la webcam et les vidéos
// ============================================================================

#pragma once
#ifndef MAIN_FRAME_H
#define MAIN_FRAME_H

#include <wx/wx.h>
#include <wx/listbox.h>
#include <wx/timer.h>
#include <wx/gauge.h>
#include <wx/checklst.h>
#include <opencv2/opencv.hpp>
#include <vector>
#include <string>

#include "ImagePanel.h"
#include "ThumbnailPanel.h"

// ============================================================================
// CLASSE PRINCIPALE
// ============================================================================

class MainFrame : public wxFrame
{
public:
    MainFrame();
    ~MainFrame();

    // Appelé par le drop-target quand un traitement est glissé-déposé
    void ApplyDroppedProcessing(const std::string& procName);

private:
    // ========== COMPOSANTS UI ==========

    // Panneaux principaux
    wxPanel* leftPanel = nullptr;           // Panneau de contrôle gauche
    ImagePanel* imagePanel = nullptr;       // Panneau d'affichage central
    ThumbnailPanel* thumbsPanel = nullptr;  // Panneau des miniatures à droite

    // Listes de traitements
    wxListBox* availableList = nullptr;      // Liste des traitements disponibles
    wxCheckListBox* pipelineList = nullptr;  // Liste du pipeline actuel

    // Boutons de fichiers
    wxButton* btnLoadImage = nullptr;
    wxButton* btnLoadVideo = nullptr;
    wxButton* btnSaveVideo = nullptr;
    wxButton* btnSaveImage = nullptr;
    wxButton* btnCaptureToggle = nullptr;

    // Boutons de pipeline
    wxButton* btnRemoveFromPipeline = nullptr;
    wxButton* btnClearPipeline = nullptr;
    wxButton* btnSavePipeline = nullptr;
    wxButton* btnLoadPipeline = nullptr;

    // Contrôles de lecture vidéo
    wxButton* btnPlayPause = nullptr;
    wxButton* btnStop = nullptr;
    wxSlider* videoSlider = nullptr;
    wxStaticText* videoTimeLabel = nullptr;
    wxGauge* progressGauge = nullptr;

    // ========== DONNÉES ==========

    cv::Mat originalImage;                  // Image source originale
    cv::Mat finalImage;                     // Image après application du pipeline
    std::vector<std::string> pipeline;      // Liste ordonnée des traitements

    // ========== WEBCAM ==========

    cv::VideoCapture cap;                   // Capture webcam
    wxTimer* captureTimer = nullptr;        // Timer pour capture continue
    bool capturing = false;                 // État de capture

    // ========== VIDÉO ==========

    cv::VideoCapture videoCapture;          // Lecteur vidéo
    wxTimer* videoTimer = nullptr;          // Timer pour lecture vidéo
    bool isPlayingVideo = false;            // État de lecture
    bool isProcessingVideo = false;         // Traitement vidéo en cours
    double videoFPS = 30.0;                 // FPS de la vidéo
    int totalVideoFrames = 0;               // Nombre total de frames
    int currentVideoFrame = 0;              // Frame actuelle
    std::string currentVideoPath;           // Chemin de la vidéo

    // ========== MÉTHODES D'INITIALISATION ==========

    void CreateControls();                  // Crée tous les widgets
    void CreateLayout();                    // Organise les widgets
    void PopulateAvailableList();           // Remplit la liste des traitements
    void ApplyModernTheme();                // Applique le thème sombre

    // ========== TRAITEMENT DU PIPELINE ==========

    void ApplySingleProcessing(const std::string& name, cv::Mat& img);
    void RecalculatePipeline();             // Recalcule tout le pipeline
    void ClearThumbnails();                 // Efface toutes les miniatures
    void AddProcessedToChain(const cv::Mat& img);  // Ajoute une miniature

    // ========== GESTIONNAIRES D'ÉVÉNEMENTS - FICHIERS ==========

    void OnLoadImage(wxCommandEvent& evt);
    void OnLoadVideo(wxCommandEvent& evt);
    void OnSaveVideo(wxCommandEvent& evt);
    void OnSaveImage(wxCommandEvent& evt);

    // ========== GESTIONNAIRES D'ÉVÉNEMENTS - WEBCAM ==========

    void OnCaptureToggle(wxCommandEvent& evt);
    void OnTimerCapture(wxTimerEvent& evt);

    // ========== GESTIONNAIRES D'ÉVÉNEMENTS - PIPELINE ==========

    void OnRemoveFromPipeline(wxCommandEvent& evt);
    void OnClearPipeline(wxCommandEvent& evt);
    void OnSavePipeline(wxCommandEvent& evt);
    void OnLoadPipeline(wxCommandEvent& evt);

    // ========== GESTIONNAIRES D'ÉVÉNEMENTS - DRAG & DROP ==========

    void OnAvailableListMouseDown(wxMouseEvent& evt);
    void OnPipelineListMouseDown(wxMouseEvent& evt);

    // ========== GESTIONNAIRES D'ÉVÉNEMENTS - MINIATURES ==========

    void OnThumbnailClicked(size_t idx);

    // ========== GESTIONNAIRES D'ÉVÉNEMENTS - VIDÉO ==========

    void OnPlayPause(wxCommandEvent& evt);
    void OnStop(wxCommandEvent& evt);
    void OnVideoTimer(wxTimerEvent& evt);
    void OnVideoSlider(wxCommandEvent& evt);

    // ========== MÉTHODES UTILITAIRES VIDÉO ==========

    void UpdateVideoControls();             // Met à jour l'état des contrôles vidéo
    void UpdateVideoTimeLabel();            // Met à jour l'affichage du temps
    std::string FormatTime(int seconds);    // Formate le temps en MM:SS

    // ========== GESTIONNAIRES D'ÉVÉNEMENTS - À PROPOS ==========
    void OnAbout(wxCommandEvent& evt);

    wxDECLARE_EVENT_TABLE();
};

#endif // MAIN_FRAME_H