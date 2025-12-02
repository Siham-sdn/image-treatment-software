// ============================================================================
// MainFrame.cpp
// ============================================================================
// Implémentation de la fenêtre principale
// ============================================================================

#include "MainFrame.h"
#include <wx/filedlg.h>
#include <wx/msgdlg.h>
#include <wx/dnd.h>
#include <wx/progdlg.h>
#include <wx/statline.h> 
#include <wx/datetime.h>
#include <algorithm>
#include <fstream>

// ============================================================================
// CLASSE DROP TARGET
// ============================================================================
// Gère le glisser-déposer des traitements vers la zone d'image
// ============================================================================

class TextDropToMainFrame : public wxTextDropTarget
{
public:
    explicit TextDropToMainFrame(MainFrame* owner) : m_owner(owner) {}

    // Appelé quand un traitement est déposé sur l'image
    bool OnDropText(wxCoord x, wxCoord y, const wxString& data) override
    {
        if (m_owner) {
            m_owner->ApplyDroppedProcessing(data.ToStdString());
        }
        return true;
    }

private:
    MainFrame* m_owner;
};

// ============================================================================
// TABLE D'ÉVÉNEMENTS
// ============================================================================

wxBEGIN_EVENT_TABLE(MainFrame, wxFrame)
wxEND_EVENT_TABLE()

// ============================================================================
// CONSTRUCTEUR / DESTRUCTEUR
// ============================================================================

MainFrame::MainFrame()
    : wxFrame(nullptr, wxID_ANY, "Application de Traitement d'Images et Vidéos",
        wxDefaultPosition, wxSize(1280, 720))
{
    // Initialisation des timers
    captureTimer = new wxTimer(this);
    videoTimer = new wxTimer(this);

    // Construction de l'interface
    CreateControls();
    CreateLayout();
    ApplyModernTheme();

    // Connexion des événements de timer
    Bind(wxEVT_TIMER, &MainFrame::OnTimerCapture, this, captureTimer->GetId());
    Bind(wxEVT_TIMER, &MainFrame::OnVideoTimer, this, videoTimer->GetId());

    // Configuration de la fenêtre
    Centre();
    SetMinSize(wxSize(1000, 600));
}

MainFrame::~MainFrame()
{
    // Arrêt des timers
    if (captureTimer && captureTimer->IsRunning()) captureTimer->Stop();
    if (videoTimer && videoTimer->IsRunning()) videoTimer->Stop();

    // Libération des ressources vidéo
    if (cap.isOpened()) cap.release();
    if (videoCapture.isOpened()) videoCapture.release();

    // Suppression des timers
    delete captureTimer;
    delete videoTimer;
}

// ============================================================================
// CRÉATION DE L'INTERFACE
// ============================================================================

void MainFrame::CreateControls()
{
    // ---------- PANNEAU DE CONTRÔLE GAUCHE ----------
    leftPanel = new wxPanel(this);
    leftPanel->SetMinSize(wxSize(320, -1));

    // ---------- BOUTONS DE CHARGEMENT ----------
    btnLoadImage = new wxButton(leftPanel, wxID_ANY, "Charger Image");
    btnLoadVideo = new wxButton(leftPanel, wxID_ANY, "Charger Vidéo");
    btnCaptureToggle = new wxButton(leftPanel, wxID_ANY, "Démarrer Webcam");

    // ---------- BOUTONS DE PIPELINE ----------
    btnRemoveFromPipeline = new wxButton(leftPanel, wxID_ANY, "Retirer du Pipeline");
    btnClearPipeline = new wxButton(leftPanel, wxID_ANY, "Vider le Pipeline");
    btnSavePipeline = new wxButton(leftPanel, wxID_ANY, "Sauvegarder Pipeline");
    btnLoadPipeline = new wxButton(leftPanel, wxID_ANY, "Charger Pipeline");

    // ---------- BOUTONS DE SAUVEGARDE ----------
    btnSaveImage = new wxButton(leftPanel, wxID_ANY, "Sauvegarder Image");
    btnSaveVideo = new wxButton(leftPanel, wxID_ANY, "Sauvegarder Vidéo");

    // ---------- CONTRÔLES VIDÉO ----------
    btnPlayPause = new wxButton(leftPanel, wxID_ANY, "Play");
    btnStop = new wxButton(leftPanel, wxID_ANY, "Stop");
    btnPlayPause->Enable(false);
    btnStop->Enable(false);

    // Slider de position vidéo (hauteur fixe pour éviter les sauts)
    videoSlider = new wxSlider(leftPanel, wxID_ANY, 0, 0, 100,
        wxDefaultPosition, wxSize(-1, 30), wxSL_HORIZONTAL);
    videoSlider->Enable(false);

    // Label de temps et barre de progression
    videoTimeLabel = new wxStaticText(leftPanel, wxID_ANY, "00:00 / 00:00");
    progressGauge = new wxGauge(leftPanel, wxID_ANY, 100);
    progressGauge->Hide();

    // ---------- LISTES DE TRAITEMENTS ----------
    availableList = new wxListBox(leftPanel, wxID_ANY, wxDefaultPosition,
        wxDefaultSize, 0, nullptr, wxLB_SINGLE);
    pipelineList = new wxCheckListBox(leftPanel, wxID_ANY, wxDefaultPosition,
        wxDefaultSize, 0, nullptr);

    PopulateAvailableList();

    // ---------- PANNEAU CENTRAL (IMAGE) ----------
    imagePanel = new ImagePanel(this);
    imagePanel->SetDropTarget(new TextDropToMainFrame(this));

    // ---------- PANNEAU DE MINIATURES (DROITE) ----------
    thumbsPanel = new ThumbnailPanel(this, 200, 10);
    thumbsPanel->SetMinSize(wxSize(240, -1));
    thumbsPanel->SetOnThumbClick([this](size_t idx) { OnThumbnailClicked(idx); });

    // ---------- CONNEXION DES ÉVÉNEMENTS ----------

    // Événements fichiers
    btnLoadImage->Bind(wxEVT_BUTTON, &MainFrame::OnLoadImage, this);
    btnLoadVideo->Bind(wxEVT_BUTTON, &MainFrame::OnLoadVideo, this);
    btnSaveVideo->Bind(wxEVT_BUTTON, &MainFrame::OnSaveVideo, this);
    btnSaveImage->Bind(wxEVT_BUTTON, &MainFrame::OnSaveImage, this);

    // Événements webcam
    btnCaptureToggle->Bind(wxEVT_BUTTON, &MainFrame::OnCaptureToggle, this);

    // Événements pipeline
    btnRemoveFromPipeline->Bind(wxEVT_BUTTON, &MainFrame::OnRemoveFromPipeline, this);
    btnClearPipeline->Bind(wxEVT_BUTTON, &MainFrame::OnClearPipeline, this);
    btnSavePipeline->Bind(wxEVT_BUTTON, &MainFrame::OnSavePipeline, this);
    btnLoadPipeline->Bind(wxEVT_BUTTON, &MainFrame::OnLoadPipeline, this);

    // Événements vidéo
    btnPlayPause->Bind(wxEVT_BUTTON, &MainFrame::OnPlayPause, this);
    btnStop->Bind(wxEVT_BUTTON, &MainFrame::OnStop, this);
    videoSlider->Bind(wxEVT_SLIDER, &MainFrame::OnVideoSlider, this);

    // Événements drag & drop
    availableList->Bind(wxEVT_LEFT_DOWN, &MainFrame::OnAvailableListMouseDown, this);
    pipelineList->Bind(wxEVT_LEFT_DOWN, &MainFrame::OnPipelineListMouseDown, this);

    // Désélection automatique quand la souris quitte la liste disponible
    availableList->Bind(wxEVT_LEAVE_WINDOW, [this](wxMouseEvent&) {
        availableList->SetSelection(wxNOT_FOUND);
        });
}

void MainFrame::CreateLayout()
{
    // ========== COLONNE DE GAUCHE ==========
    wxBoxSizer* leftSizer = new wxBoxSizer(wxVERTICAL);

    // ---------- LISTE DES TRAITEMENTS DISPONIBLES ----------
    wxStaticBoxSizer* availBox = new wxStaticBoxSizer(wxVERTICAL, leftPanel, "Traitements");
    wxStaticBox* availStaticBox = availBox->GetStaticBox();
    availStaticBox->SetForegroundColour(wxColour(230, 240, 255));
    availBox->Add(availableList, 1, wxEXPAND | wxALL, 4);
    leftSizer->Add(availBox, 1, wxEXPAND | wxLEFT | wxRIGHT | wxTOP, 4);

    // ---------- PIPELINE ACTUEL ----------
    wxStaticBoxSizer* pipeBox = new wxStaticBoxSizer(wxVERTICAL, leftPanel, "Pipeline");
    wxStaticBox* pipeStaticBox = pipeBox->GetStaticBox();
    pipeStaticBox->SetForegroundColour(wxColour(230, 240, 255));
    pipeBox->Add(pipelineList, 1, wxEXPAND | wxALL, 4);
    leftSizer->Add(pipeBox, 2, wxEXPAND | wxLEFT | wxRIGHT | wxTOP, 4);

    // ---------- BOUTONS DE GESTION DU PIPELINE ----------
    // Première ligne : Retirer / Vider
    wxBoxSizer* pipeButtonsSizer1 = new wxBoxSizer(wxHORIZONTAL);
    pipeButtonsSizer1->Add(btnRemoveFromPipeline, 1, wxEXPAND | wxRIGHT, 2);
    pipeButtonsSizer1->Add(btnClearPipeline, 1, wxEXPAND | wxLEFT, 2);
    leftSizer->Add(pipeButtonsSizer1, 0, wxEXPAND | wxLEFT | wxRIGHT, 4);

    // Deuxième ligne : Sauvegarder / Charger
    wxBoxSizer* pipeButtonsSizer2 = new wxBoxSizer(wxHORIZONTAL);
    pipeButtonsSizer2->Add(btnSavePipeline, 1, wxEXPAND | wxRIGHT, 2);
    pipeButtonsSizer2->Add(btnLoadPipeline, 1, wxEXPAND | wxLEFT, 2);
    leftSizer->Add(pipeButtonsSizer2, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 4);

    // Ligne de séparation
    leftSizer->Add(new wxStaticLine(leftPanel), 0, wxEXPAND | wxTOP | wxBOTTOM, 3);

    // ---------- BOUTONS DE CHARGEMENT ----------
    wxBoxSizer* loadButtonsSizer = new wxBoxSizer(wxHORIZONTAL);
    loadButtonsSizer->Add(btnLoadImage, 1, wxEXPAND | wxRIGHT, 2);
    loadButtonsSizer->Add(btnLoadVideo, 1, wxEXPAND | wxLEFT | wxRIGHT, 2);
    loadButtonsSizer->Add(btnCaptureToggle, 1, wxEXPAND | wxLEFT, 2);
    leftSizer->Add(loadButtonsSizer, 0, wxEXPAND | wxLEFT | wxRIGHT, 4);

    leftSizer->Add(new wxStaticLine(leftPanel), 0, wxEXPAND | wxTOP | wxBOTTOM, 3);

    // ---------- CONTRÔLES VIDÉO ----------
    wxStaticBoxSizer* videoBox = new wxStaticBoxSizer(wxVERTICAL, leftPanel, "Vidéo");
    wxStaticBox* videoStaticBox = videoBox->GetStaticBox();
    videoStaticBox->SetForegroundColour(wxColour(230, 240, 255));

    // Boutons Play/Stop
    wxBoxSizer* playControlsSizer = new wxBoxSizer(wxHORIZONTAL);
    playControlsSizer->Add(btnPlayPause, 1, wxEXPAND | wxRIGHT, 2);
    playControlsSizer->Add(btnStop, 1, wxEXPAND | wxLEFT, 2);
    videoBox->Add(playControlsSizer, 0, wxEXPAND | wxALL, 2);

    // Label de temps
    videoTimeLabel->SetFont(videoTimeLabel->GetFont().Scale(0.9));
    videoBox->Add(videoTimeLabel, 0, wxALIGN_CENTER | wxTOP | wxBOTTOM, 2);

    // Slider de position (hauteur minimale fixe pour éviter les sauts)
    videoSlider->SetMinSize(wxSize(-1, 30));
    videoBox->Add(videoSlider, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 4);

    // Barre de progression
    videoBox->Add(progressGauge, 0, wxEXPAND | wxALL, 2);

    leftSizer->Add(videoBox, 0, wxEXPAND | wxLEFT | wxRIGHT, 4);

    leftSizer->Add(new wxStaticLine(leftPanel), 0, wxEXPAND | wxTOP | wxBOTTOM, 3);

    // ---------- BOUTONS DE SAUVEGARDE ----------
    wxBoxSizer* saveButtonsSizer = new wxBoxSizer(wxHORIZONTAL);
    saveButtonsSizer->Add(btnSaveImage, 1, wxEXPAND | wxRIGHT, 2);
    saveButtonsSizer->Add(btnSaveVideo, 1, wxEXPAND | wxLEFT, 2);
    leftSizer->Add(saveButtonsSizer, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 4);

    leftPanel->SetSizer(leftSizer);

    // ========== LAYOUT PRINCIPAL ==========
    wxBoxSizer* mainSizer = new wxBoxSizer(wxHORIZONTAL);

    // Panneau gauche (contrôles)
    mainSizer->Add(leftPanel, 0, wxEXPAND | wxALL, 2);

    // Panneau central (image) - prend le plus d'espace
    mainSizer->Add(imagePanel, 3, wxEXPAND | wxALL, 2);

    // ---------- PANNEAU DROIT (MINIATURES) ----------
    wxBoxSizer* rightSizer = new wxBoxSizer(wxVERTICAL);

    // Titre des miniatures
    wxStaticText* thumbTitle = new wxStaticText(this, wxID_ANY, "Miniatures");
    thumbTitle->SetForegroundColour(wxColour(230, 240, 255));
    wxFont titleFont = thumbTitle->GetFont();
    titleFont.SetPointSize(11);
    titleFont.SetWeight(wxFONTWEIGHT_BOLD);
    thumbTitle->SetFont(titleFont);

    rightSizer->Add(thumbTitle, 0, wxALIGN_CENTER | wxALL, 8);
    rightSizer->Add(thumbsPanel, 1, wxEXPAND | wxALL, 2);

    mainSizer->Add(rightSizer, 0, wxEXPAND | wxALL, 2);

    SetSizer(mainSizer);
    Layout();
}

void MainFrame::ApplyModernTheme()
{
    // ---------- PALETTE DE COULEURS FUTURISTE ----------
    wxColour darkBg(5, 8, 20);            // Fond bleu nuit
    wxColour darkerBg(10, 14, 30);        // Fond panneaux
    wxColour accentColor(0, 255, 190);    // Accent néon cyan
    wxColour textColor(230, 240, 255);    // Texte clair
    wxColour buttonBg(20, 26, 50);        // Boutons style SF

    // Fenêtre principale
    SetBackgroundColour(darkBg);

    // Panneau gauche
    leftPanel->SetBackgroundColour(darkerBg);

    // Listes
    availableList->SetBackgroundColour(buttonBg);
    availableList->SetForegroundColour(textColor);
    pipelineList->SetBackgroundColour(buttonBg);
    pipelineList->SetForegroundColour(textColor);

    // Application du thème à tous les boutons
    std::vector<wxButton*> buttons = {
        btnLoadImage, btnLoadVideo, btnSaveVideo, btnCaptureToggle,
        btnRemoveFromPipeline, btnClearPipeline, btnSaveImage,
        btnPlayPause, btnStop, btnSavePipeline, btnLoadPipeline
    };

    for (auto* btn : buttons) {
        if (btn) {
            btn->SetBackgroundColour(buttonBg);
            btn->SetForegroundColour(textColor);
        }
    }

    // Labels et contrôles
    videoTimeLabel->SetForegroundColour(textColor);
    videoSlider->SetBackgroundColour(darkerBg);
    videoSlider->SetForegroundColour(accentColor);

    Refresh();
}

void MainFrame::PopulateAvailableList()
{
    availableList->Clear();
    availableList->AppendString("Flou Gaussien");
    availableList->AppendString("Détection de Contours (Canny)");
    availableList->AppendString("Niveaux de Gris");
    availableList->AppendString("Rotation 90°");
    availableList->AppendString("Miroir Horizontal");
    availableList->AppendString("Seuillage");
    availableList->AppendString("Négatif");
}

// ============================================================================
// GESTION DES FICHIERS
// ============================================================================

void MainFrame::OnLoadImage(wxCommandEvent& /*evt*/)
{
    // Arrêt de la vidéo si en cours
    if (isPlayingVideo) {
        videoTimer->Stop();
        isPlayingVideo = false;
        btnPlayPause->SetLabel("Play");
    }

    // Dialogue de sélection de fichier
    wxFileDialog dlg(this, "Choisir une Image", "", "",
        "Images (*.jpg;*.jpeg;*.png;*.bmp)|*.jpg;*.jpeg;*.png;*.bmp",
        wxFD_OPEN | wxFD_FILE_MUST_EXIST);

    if (dlg.ShowModal() == wxID_CANCEL) return;

    // Chargement de l'image
    originalImage = cv::imread(dlg.GetPath().ToStdString());
    if (originalImage.empty()) {
        wxMessageBox("Impossible de charger l'image!", "Erreur", wxICON_ERROR);
        return;
    }

    // Mise à jour de l'interface
    UpdateVideoControls();
    RecalculatePipeline();
}

void MainFrame::OnLoadVideo(wxCommandEvent& /*evt*/)
{
    // Dialogue de sélection de vidéo
    wxFileDialog dlg(this, "Choisir une Vidéo", "", "",
        "Vidéos (*.mp4;*.avi;*.mov;*.mkv;*.wmv)|*.mp4;*.avi;*.mov;*.mkv;*.wmv",
        wxFD_OPEN | wxFD_FILE_MUST_EXIST);

    if (dlg.ShowModal() == wxID_CANCEL) return;

    currentVideoPath = dlg.GetPath().ToStdString();

    // Fermeture de la vidéo précédente
    if (videoCapture.isOpened()) {
        videoCapture.release();
    }

    // Ouverture de la nouvelle vidéo
    videoCapture.open(currentVideoPath);
    if (!videoCapture.isOpened()) {
        wxMessageBox("Impossible de charger la vidéo!", "Erreur", wxICON_ERROR);
        return;
    }

    // Récupération des propriétés vidéo
    videoFPS = videoCapture.get(cv::CAP_PROP_FPS);
    if (videoFPS <= 0) videoFPS = 30.0;  // FPS par défaut

    totalVideoFrames = (int)videoCapture.get(cv::CAP_PROP_FRAME_COUNT);
    currentVideoFrame = 0;

    // Lecture de la première frame
    cv::Mat frame;
    videoCapture >> frame;
    if (!frame.empty()) {
        originalImage = frame.clone();
        RecalculatePipeline();
    }

    // Activation des contrôles vidéo
    btnPlayPause->Enable(true);
    btnStop->Enable(true);
    videoSlider->Enable(true);
    videoSlider->SetRange(0, std::max(0, totalVideoFrames - 1));
    videoSlider->SetValue(0);

    UpdateVideoTimeLabel();
    Layout();  // Force la mise à jour du layout

    wxMessageBox(wxString::Format("Vidéo chargée:\n%d frames à %.1f FPS",
        totalVideoFrames, videoFPS), "Info", wxICON_INFORMATION);
}

void MainFrame::OnSaveImage(wxCommandEvent& /*evt*/)
{
    if (finalImage.empty()) {
        wxMessageBox("Aucune image à sauvegarder!", "Erreur", wxICON_ERROR);
        return;
    }

    // Dialogue de sauvegarde
    wxFileDialog dlg(this, "Sauvegarder l'Image", "", "",
        "JPEG (*.jpg)|*.jpg|PNG (*.png)|*.png|BMP (*.bmp)|*.bmp",
        wxFD_SAVE | wxFD_OVERWRITE_PROMPT);

    if (dlg.ShowModal() == wxID_CANCEL) return;

    // Sauvegarde de l'image
    std::string path = dlg.GetPath().ToStdString();
    if (!cv::imwrite(path, finalImage)) {
        wxMessageBox("Erreur lors de la sauvegarde!", "Erreur", wxICON_ERROR);
    }
    else {
        wxMessageBox("Image sauvegardée avec succès!", "Succès", wxICON_INFORMATION);
    }
}

void MainFrame::OnSaveVideo(wxCommandEvent& /*evt*/)
{
    // Vérifications préalables
    if (!videoCapture.isOpened()) {
        wxMessageBox("Aucune vidéo chargée!", "Erreur", wxICON_ERROR);
        return;
    }

    if (pipeline.empty()) {
        wxMessageBox("Le pipeline est vide!\nAjoutez au moins un traitement.",
            "Attention", wxICON_WARNING);
        return;
    }

    // Dialogue de sauvegarde
    wxFileDialog dlg(this, "Sauvegarder la Vidéo Traitée", "", "output.mp4",
        "MP4 (*.mp4)|*.mp4|AVI (*.avi)|*.avi",
        wxFD_SAVE | wxFD_OVERWRITE_PROMPT);

    if (dlg.ShowModal() == wxID_CANCEL) return;

    std::string outputPath = dlg.GetPath().ToStdString();

    // Arrêt de la lecture
    if (isPlayingVideo) {
        videoTimer->Stop();
        isPlayingVideo = false;
    }

    // Réouverture de la vidéo pour traitement
    videoCapture.release();
    videoCapture.open(currentVideoPath);
    if (!videoCapture.isOpened()) {
        wxMessageBox("Erreur de réouverture de la vidéo!", "Erreur", wxICON_ERROR);
        return;
    }

    // Obtention des dimensions après traitement
    cv::Mat firstFrame;
    videoCapture >> firstFrame;
    if (firstFrame.empty()) {
        wxMessageBox("Erreur de lecture de la première frame!", "Erreur", wxICON_ERROR);
        return;
    }

    // Application du pipeline sur la première frame pour obtenir les dimensions
    cv::Mat processedFirst = firstFrame.clone();
    for (const auto& step : pipeline) {
        ApplySingleProcessing(step, processedFirst);
    }

    int frameWidth = processedFirst.cols;
    int frameHeight = processedFirst.rows;

    // Création du VideoWriter
    int fourcc = cv::VideoWriter::fourcc('m', 'p', '4', 'v');
    cv::VideoWriter writer(outputPath, fourcc, videoFPS,
        cv::Size(frameWidth, frameHeight), true);

    if (!writer.isOpened()) {
        wxMessageBox("Impossible de créer le fichier vidéo!", "Erreur", wxICON_ERROR);
        videoCapture.release();
        videoCapture.open(currentVideoPath);
        return;
    }

    // Affichage de la barre de progression
    progressGauge->Show();
    progressGauge->SetRange(totalVideoFrames);
    progressGauge->SetValue(0);
    Layout();

    // Dialogue de progression avec temps estimé
    wxProgressDialog progress("Traitement Vidéo",
        "Traitement des frames en cours...",
        totalVideoFrames, this,
        wxPD_APP_MODAL | wxPD_AUTO_HIDE | wxPD_CAN_ABORT |
        wxPD_ELAPSED_TIME | wxPD_ESTIMATED_TIME | wxPD_REMAINING_TIME);

    // Retour au début de la vidéo
    videoCapture.set(cv::CAP_PROP_POS_FRAMES, 0);

    int frameCount = 0;
    isProcessingVideo = true;

    // Traitement de chaque frame
    while (true) {
        cv::Mat frame;
        videoCapture >> frame;

        if (frame.empty()) break;  // Fin de la vidéo

        // Application du pipeline complet
        cv::Mat processed = frame.clone();
        for (const auto& step : pipeline) {
            ApplySingleProcessing(step, processed);
        }

        // Écriture de la frame traitée
        writer.write(processed);
        frameCount++;

        // Mise à jour de la progression
        progressGauge->SetValue(frameCount);

        if (!progress.Update(frameCount,
            wxString::Format("Frame %d/%d", frameCount, totalVideoFrames))) {
            break; // Annulation utilisateur
        }

        // Rafraîchissement de l'interface toutes les 10 frames
        if (frameCount % 10 == 0) {
            wxTheApp->Yield(true);
        }
    }

    // Nettoyage
    isProcessingVideo = false;
    writer.release();
    progressGauge->Hide();
    Layout();

    // Réouverture de la vidéo pour lecture
    videoCapture.release();
    videoCapture.open(currentVideoPath);
    videoCapture.set(cv::CAP_PROP_POS_FRAMES, 0);
    currentVideoFrame = 0;
    videoSlider->SetValue(0);

    // Message de succès
    if (frameCount == totalVideoFrames) {
        wxMessageBox(wxString::Format("Vidéo sauvegardée avec succès!\n%d frames traitées",
            frameCount), "Succès", wxICON_INFORMATION);
    }
    else {
        wxMessageBox(wxString::Format("Traitement interrompu:\n%d/%d frames traitées",
            frameCount, totalVideoFrames), "Info", wxICON_INFORMATION);
    }
}

// ============================================================================
// GESTION WEBCAM
// ============================================================================

void MainFrame::OnCaptureToggle(wxCommandEvent& /*evt*/)
{
    if (!capturing) {
        // Démarrage de la capture
        cap.open(0);  // Ouvre la première webcam disponible
        if (!cap.isOpened()) {
            wxMessageBox("Impossible d'ouvrir la webcam!", "Erreur", wxICON_ERROR);
            return;
        }

        captureTimer->Start(33); // ~30 FPS
        capturing = true;
        btnCaptureToggle->SetLabel("Arrêter Webcam");

        UpdateVideoControls();
    }
    else {
        // Arrêt de la capture
        captureTimer->Stop();
        if (cap.isOpened()) cap.release();
        capturing = false;
        btnCaptureToggle->SetLabel("Démarrer Webcam");
    }
}

void MainFrame::OnTimerCapture(wxTimerEvent& /*evt*/)
{
    if (!cap.isOpened()) return;

    // Capture d'une frame
    cv::Mat frame;
    cap >> frame;
    if (frame.empty()) return;

    // Application du pipeline sur la frame capturée
    originalImage = frame.clone();
    RecalculatePipeline();
}

// ============================================================================
// CONTRÔLES VIDÉO
// ============================================================================

void MainFrame::OnPlayPause(wxCommandEvent& /*evt*/)
{
    if (!videoCapture.isOpened()) return;

    if (!isPlayingVideo) {
        // Démarrage de la lecture
        int interval = std::max(1, (int)(1000.0 / videoFPS));
        videoTimer->Start(interval);
        isPlayingVideo = true;
        btnPlayPause->SetLabel("Pause");
    }
    else {
        // Pause
        videoTimer->Stop();
        isPlayingVideo = false;
        btnPlayPause->SetLabel("Play");
    }
}

void MainFrame::OnStop(wxCommandEvent& /*evt*/)
{
    if (!videoCapture.isOpened()) return;

    // Arrêt de la lecture
    videoTimer->Stop();
    isPlayingVideo = false;
    btnPlayPause->SetLabel("Play");

    // Retour au début
    videoCapture.set(cv::CAP_PROP_POS_FRAMES, 0);
    currentVideoFrame = 0;
    videoSlider->SetValue(0);

    // Affichage de la première frame
    cv::Mat frame;
    videoCapture >> frame;
    if (!frame.empty()) {
        originalImage = frame.clone();
        RecalculatePipeline();
    }

    UpdateVideoTimeLabel();
}

void MainFrame::OnVideoTimer(wxTimerEvent& /*evt*/)
{
    if (!videoCapture.isOpened()) return;

    // Lecture de la frame suivante
    cv::Mat frame;
    videoCapture >> frame;

    if (frame.empty()) {
        // Fin de la vidéo - retour au début
        videoTimer->Stop();
        isPlayingVideo = false;
        btnPlayPause->SetLabel("Play");

        videoCapture.set(cv::CAP_PROP_POS_FRAMES, 0);
        currentVideoFrame = 0;
        videoSlider->SetValue(0);
        return;
    }

    // Mise à jour de l'image
    currentVideoFrame++;
    originalImage = frame.clone();

    // Application du pipeline
    RecalculatePipeline();

    // Mise à jour du slider
    videoSlider->SetValue(currentVideoFrame);
    UpdateVideoTimeLabel();
}

void MainFrame::OnVideoSlider(wxCommandEvent& /*evt*/)
{
    if (!videoCapture.isOpened()) return;

    // Déplacement vers la frame sélectionnée
    int targetFrame = videoSlider->GetValue();
    videoCapture.set(cv::CAP_PROP_POS_FRAMES, targetFrame);
    currentVideoFrame = targetFrame;

    // Lecture et affichage de la frame
    cv::Mat frame;
    videoCapture >> frame;
    if (!frame.empty()) {
        originalImage = frame.clone();
        RecalculatePipeline();
    }

    UpdateVideoTimeLabel();
}

void MainFrame::UpdateVideoControls()
{
    bool hasVideo = videoCapture.isOpened();

    // Activation/désactivation des contrôles selon l'état
    btnPlayPause->Enable(hasVideo && !capturing);
    btnStop->Enable(hasVideo && !capturing);
    videoSlider->Enable(hasVideo && !capturing);

    // Force la mise à jour du layout
    Layout();
}

void MainFrame::UpdateVideoTimeLabel()
{
    if (!videoCapture.isOpened() || totalVideoFrames == 0) {
        videoTimeLabel->SetLabel("00:00 / 00:00");
        return;
    }

    // Calcul des temps
    int currentSeconds = (int)(currentVideoFrame / videoFPS);
    int totalSeconds = (int)(totalVideoFrames / videoFPS);

    videoTimeLabel->SetLabel(FormatTime(currentSeconds) + " / " + FormatTime(totalSeconds));
}

std::string MainFrame::FormatTime(int seconds)
{
    int minutes = seconds / 60;
    int secs = seconds % 60;
    char buffer[16];
    snprintf(buffer, sizeof(buffer), "%02d:%02d", minutes, secs);
    return std::string(buffer);
}

// ============================================================================
// GESTION DU PIPELINE
// ============================================================================

void MainFrame::OnRemoveFromPipeline(wxCommandEvent& /*evt*/)
{
    // Collecte des éléments cochés
    std::vector<int> checkedIndices;

    for (unsigned int i = 0; i < pipelineList->GetCount(); ++i) {
        if (pipelineList->IsChecked(i)) {
            checkedIndices.push_back(i);
        }
    }

    if (checkedIndices.empty()) {
        wxMessageBox("Veuillez cocher au moins un traitement à supprimer.",
            "Info", wxICON_INFORMATION);
        return;
    }

    // Vérification : "Original" ne peut pas être supprimé
    if (std::find(checkedIndices.begin(), checkedIndices.end(), 0) != checkedIndices.end()) {
        wxMessageBox("L'étape 'Original' ne peut pas être supprimée.",
            "Interdit", wxICON_WARNING);
        return;
    }

    // Suppression en ordre inverse pour maintenir les indices valides
    std::sort(checkedIndices.rbegin(), checkedIndices.rend());

    for (int idx : checkedIndices) {
        // Index dans le pipeline (décalé de 1 car "Original" est en position 0)
        int pipelineIndex = idx - 1;
        if (pipelineIndex >= 0 && pipelineIndex < (int)pipeline.size()) {
            pipeline.erase(pipeline.begin() + pipelineIndex);
        }
    }

    RecalculatePipeline();

    wxMessageBox(wxString::Format("%d traitement(s) supprimé(s).",
        (int)checkedIndices.size()), "Succès", wxICON_INFORMATION);
}

void MainFrame::OnClearPipeline(wxCommandEvent& /*evt*/)
{
    if (pipeline.empty()) {
        wxMessageBox("Le pipeline est déjà vide.", "Info", wxICON_INFORMATION);
        return;
    }

    // Confirmation
    int response = wxMessageBox("Êtes-vous sûr de vouloir vider tout le pipeline?",
        "Confirmation", wxYES_NO | wxICON_QUESTION);

    if (response == wxYES) {
        pipeline.clear();
        RecalculatePipeline();
    }
}

void MainFrame::OnSavePipeline(wxCommandEvent& /*evt*/)
{
    if (pipeline.empty()) {
        wxMessageBox("Le pipeline est vide. Rien à sauvegarder.",
            "Info", wxICON_INFORMATION);
        return;
    }

    // Dialogue de sauvegarde
    wxFileDialog dlg(this, "Sauvegarder le Pipeline", "", "pipeline.txt",
        "Fichiers texte (*.txt)|*.txt",
        wxFD_SAVE | wxFD_OVERWRITE_PROMPT);

    if (dlg.ShowModal() == wxID_CANCEL) return;

    std::string path = dlg.GetPath().ToStdString();
    std::ofstream file(path);

    if (!file.is_open()) {
        wxMessageBox("Impossible de créer le fichier!",
            "Erreur", wxICON_ERROR);
        return;
    }

    // Écriture de l'en-tête
    file << "# Pipeline de Traitements d'Images\n";
    file << "# Créé le " << wxDateTime::Now().FormatISOCombined() << "\n";
    file << "# Nombre de traitements: " << pipeline.size() << "\n\n";

    // Écriture de chaque traitement
    for (const auto& treatment : pipeline) {
        file << treatment << "\n";
    }

    file.close();

    wxMessageBox(wxString::Format("Pipeline sauvegardé avec succès!\n%d traitement(s) enregistré(s).",
        (int)pipeline.size()), "Succès", wxICON_INFORMATION);
}

void MainFrame::OnLoadPipeline(wxCommandEvent& /*evt*/)
{
    // Dialogue d'ouverture
    wxFileDialog dlg(this, "Charger un Pipeline", "", "",
        "Fichiers texte (*.txt)|*.txt",
        wxFD_OPEN | wxFD_FILE_MUST_EXIST);

    if (dlg.ShowModal() == wxID_CANCEL) return;

    std::string path = dlg.GetPath().ToStdString();
    std::ifstream file(path);

    if (!file.is_open()) {
        wxMessageBox("Impossible d'ouvrir le fichier!",
            "Erreur", wxICON_ERROR);
        return;
    }

    // Lecture des traitements
    std::vector<std::string> loadedPipeline;
    std::string line;

    while (std::getline(file, line)) {
        // Ignorer les lignes vides et les commentaires
        if (line.empty() || line[0] == '#') continue;

        // Vérification que le traitement existe
        bool found = false;
        for (unsigned int i = 0; i < availableList->GetCount(); ++i) {
            if (availableList->GetString(i).ToStdString() == line) {
                found = true;
                break;
            }
        }

        if (found) {
            loadedPipeline.push_back(line);
        }
        else {
            wxLogWarning("Traitement inconnu ignoré: %s", line);
        }
    }

    file.close();

    if (loadedPipeline.empty()) {
        wxMessageBox("Aucun traitement valide trouvé dans le fichier!",
            "Attention", wxICON_WARNING);
        return;
    }

    // Confirmation de remplacement si pipeline non vide
    if (!pipeline.empty()) {
        int response = wxMessageBox(
            wxString::Format("Le pipeline actuel contient %d traitement(s).\n"
                "Voulez-vous le remplacer par le pipeline chargé (%d traitement(s))?",
                (int)pipeline.size(), (int)loadedPipeline.size()),
            "Confirmation", wxYES_NO | wxICON_QUESTION);

        if (response != wxYES) return;
    }

    // Vérification qu'une image est chargée
    if (originalImage.empty()) {
        wxMessageBox("Veuillez d'abord charger une image ou une vidéo!",
            "Attention", wxICON_WARNING);
        return;
    }

    // Remplacement du pipeline
    pipeline = loadedPipeline;
    RecalculatePipeline();

    wxMessageBox(wxString::Format("Pipeline chargé avec succès!\n%d traitement(s) appliqué(s).",
        (int)pipeline.size()), "Succès", wxICON_INFORMATION);
}

// ============================================================================
// GESTIONNAIRES DRAG & DROP
// ============================================================================

void MainFrame::OnAvailableListMouseDown(wxMouseEvent& evt)
{
    // Détection de l'élément cliqué
    int sel = availableList->HitTest(evt.GetPosition());
    if (sel != wxNOT_FOUND) {
        availableList->SetSelection(sel);
        wxString txt = availableList->GetString(sel);

        // Démarrage du drag & drop
        wxTextDataObject data(txt);
        wxDropSource drag(data, availableList);
        drag.DoDragDrop(wxDrag_CopyOnly);
    }
    evt.Skip();
}

void MainFrame::OnPipelineListMouseDown(wxMouseEvent& evt)
{
    // Détection de l'élément cliqué
    int sel = pipelineList->HitTest(evt.GetPosition());

    if (sel != wxNOT_FOUND) {
        pipelineList->SetSelection(sel);

        // "Original" ne peut pas être glissé
        if (sel == 0) {
            evt.Skip();
            return;
        }

        // Démarrage du drag & drop pour les autres éléments
        wxString txt = pipelineList->GetString(sel);
        wxTextDataObject data(txt);
        wxDropSource drag(data, pipelineList);
        drag.DoDragDrop(wxDrag_CopyOnly);
    }

    evt.Skip();
}

// ============================================================================
// TRAITEMENT DU PIPELINE
// ============================================================================

void MainFrame::RecalculatePipeline()
{
    // Mise à jour de la liste du pipeline
    pipelineList->Clear();

    // Optimisation : gel de l'interface pendant la mise à jour
    Freeze();
    ClearThumbnails();

    if (originalImage.empty()) {
        finalImage.release();
        imagePanel->ClearImage();
        Thaw();
        return;
    }

    // Ajout de l'image originale
    pipelineList->AppendString("Original");
    AddProcessedToChain(originalImage);

    // Application séquentielle de tous les traitements
    cv::Mat current = originalImage.clone();

    for (size_t i = 0; i < pipeline.size(); ++i) {
        const std::string& step = pipeline[i];
        ApplySingleProcessing(step, current);

        pipelineList->AppendString(step);
        AddProcessedToChain(current);
    }

    // Affichage du résultat final
    finalImage = current.clone();
    imagePanel->SetImage(finalImage);

    Thaw();

    // Rafraîchissement de l'image principale
    imagePanel->Refresh();
}

// ============================================================================
// TRAITEMENTS D'IMAGE
// ============================================================================

void MainFrame::ApplySingleProcessing(const std::string& name, cv::Mat& img)
{
    if (img.empty()) return;

    try {
        // Application du traitement selon le nom
        if (name == "Flou Gaussien") {
            cv::GaussianBlur(img, img, cv::Size(15, 15), 0);
        }
        else if (name == "Détection de Contours (Canny)") {
            cv::Mat gray, edges;
            cv::cvtColor(img, gray, cv::COLOR_BGR2GRAY);
            cv::Canny(gray, edges, 50, 150);
            cv::cvtColor(edges, img, cv::COLOR_GRAY2BGR);
        }
        else if (name == "Niveaux de Gris") {
            cv::Mat gray;
            cv::cvtColor(img, gray, cv::COLOR_BGR2GRAY);
            cv::cvtColor(gray, img, cv::COLOR_GRAY2BGR);
        }
        else if (name == "Rotation 90°") {
            cv::rotate(img, img, cv::ROTATE_90_CLOCKWISE);
        }
        else if (name == "Miroir Horizontal") {
            cv::flip(img, img, 1);
        }
        else if (name == "Seuillage") {
            cv::Mat gray, thr;
            cv::cvtColor(img, gray, cv::COLOR_BGR2GRAY);
            cv::threshold(gray, thr, 128, 255, cv::THRESH_BINARY);
            cv::cvtColor(thr, img, cv::COLOR_GRAY2BGR);
        }
        else if (name == "Négatif") {
            cv::bitwise_not(img, img);
        }
    }
    catch (const cv::Exception& e) {
        wxLogError("Erreur de traitement: %s", e.what());
    }
}

// ============================================================================
// GESTION DES MINIATURES
// ============================================================================

void MainFrame::ClearThumbnails()
{
    if (thumbsPanel) {
        thumbsPanel->Clear();
    }
}

void MainFrame::AddProcessedToChain(const cv::Mat& img)
{
    if (thumbsPanel && !img.empty()) {
        thumbsPanel->AddThumbnail(img);
    }
}

void MainFrame::OnThumbnailClicked(size_t idx)
{
    if (!thumbsPanel) return;

    // Récupération et affichage de l'image cliquée
    cv::Mat img = thumbsPanel->GetImageAt(idx);
    if (!img.empty()) {
        imagePanel->SetImage(img);
    }
}

void MainFrame::ApplyDroppedProcessing(const std::string& procName)
{
    // Vérification qu'une image est chargée
    if (originalImage.empty()) {
        wxMessageBox("Veuillez d'abord charger une image ou une vidéo!",
            "Attention", wxICON_WARNING);
        return;
    }

    // Ajout du traitement au pipeline
    pipeline.push_back(procName);
    RecalculatePipeline();
}