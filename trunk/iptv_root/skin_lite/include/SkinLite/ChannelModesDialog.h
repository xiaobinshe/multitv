/////////////////////////////////////////////////////////////////////////////
// Name:        ChannelModesDialog.h
// Purpose:     
// Author:      MultiTV
// Modified by: 
// Created:     06/06/2008 15:06:07
// RCS-ID:      
// Copyright:   
// Licence:     
/////////////////////////////////////////////////////////////////////////////

// Generated by DialogBlocks (unregistered), 06/06/2008 15:06:07
#ifndef CHANNEL_MODES_DIALOG_H
#define CHANNEL_MODES_DIALOG_H


/*!
 * Includes
 */
#include "wx/frame.h"
#include "wx/combo.h"
#include "wx/spinctrl.h"

/*!
 * Forward declarations
 */
class wxComboCtrl;
class wxSpinCtrl;

/*!
 * Control identifiers
 */
#define ID_CHANNELMODES 10000
#define ID_PANEL1 10003
#define ID_TEXTCTRL1 10002
#define ID_CHECKBOX1 10001
#define ID_CHECKBOX2 10004
#define ID_CHECKBOX3 10005
#define ID_CHECKBOX4 10006
#define ID_CHECKBOX5 10007
#define ID_COMBOBOX1 10008
#define ID_CHECKBOX6 10009
#define ID_TEXTCTRL3 10010
#define ID_CHECKBOX7 10011
#define ID_CHECKBOX8 10012
#define ID_CHECKBOX9 10013
#define ID_SPINCTRL1 10014
#define ID_CHECKBOX10 10015
#define ID_SPINCTRL2 10016
#define ID_CHECKBOX11 10017
#define ID_SPINCTRL3 10018
#define ID_CHECKBOX12 10019
#define ID_CHECKBOX13 10020
#define ID_SPINCTRL4 10021
#define ID_CHECKBOX14 10022
#define ID_CHECKBOX15 10023
#define ID_BUTTON_OK 10024
#define ID_BUTTON_CANCEL 10025
#define SYMBOL_CHANNELMODESDIALOG_STYLE wxCAPTION|wxSYSTEM_MENU|wxCLOSE_BOX|wxFRAME_NO_TASKBAR
#define SYMBOL_CHANNELMODESDIALOG_TITLE _("Channel Modes")
#define SYMBOL_CHANNELMODESDIALOG_IDNAME ID_CHANNELMODES
#define SYMBOL_CHANNELMODESDIALOG_SIZE wxSize(450, 350)
#define SYMBOL_CHANNELMODESDIALOG_POSITION wxDefaultPosition

// Max transmission limit displayed (real limit - 1)
#define TRANSMISSION_LIMIT 6

// User limit
#define USER_LIMIT 99999

// Band share limit
#define BAND_SHARE_LIMIT 9

// One audio limit in miliseconds
#define ONE_AUDIO_LIMIT 99999
#define ONE_AUDIO_INITIAL_TIME 1000


/** @brief Class thats implements channel modes dialog.
*
*/
class ChannelModesDialog: public wxDialog
{    
    DECLARE_CLASS( ChannelModesDialog )
    DECLARE_EVENT_TABLE()
private:
	static const wxString BITRATES[];

	void EnableControls();
	void DisableControls(bool topicTextCtrlToo);

public:
    // Constructors
    ChannelModesDialog();
    ChannelModesDialog( wxWindow* parent, AppInterface* iface, wxWindowID id = SYMBOL_CHANNELMODESDIALOG_IDNAME, const wxString& caption = SYMBOL_CHANNELMODESDIALOG_TITLE, const wxPoint& pos = SYMBOL_CHANNELMODESDIALOG_POSITION, const wxSize& size = SYMBOL_CHANNELMODESDIALOG_SIZE, long style = SYMBOL_CHANNELMODESDIALOG_STYLE );

    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CHANNELMODESDIALOG_IDNAME, const wxString& caption = SYMBOL_CHANNELMODESDIALOG_TITLE, const wxPoint& pos = SYMBOL_CHANNELMODESDIALOG_POSITION, const wxSize& size = SYMBOL_CHANNELMODESDIALOG_SIZE, long style = SYMBOL_CHANNELMODESDIALOG_STYLE );

    // Destructor
    ~ChannelModesDialog();

    // Initialises member variables
    void Init();
    void InitControlsState();

    // Creates the controls and sizers
    void CreateControls();

    // Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    // Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );

    // Should we show tooltips?
    static bool ShowToolTips();

private:
	AppInterface* m_appInterface;

    wxTextCtrl* m_txtTopic;
    wxCheckBox* m_ckbOpChangeTopic;
    wxCheckBox* m_ckbAudioMute;
    wxCheckBox* m_ckbNoExternalMessages;
    wxCheckBox* m_ckbPrivate;
    wxCheckBox* m_ckbBitRate;
    wxComboBox* m_cmbBitRate;
    wxCheckBox* m_ckbPassword;
    wxTextCtrl* m_txtPassword;
    wxCheckBox* m_ckbMultipleTransmissions;
    wxCheckBox* m_ckbInviteOnly;
    wxCheckBox* m_ckbTransmissionLimit;
    wxSpinCtrl* m_spcTransmissionLimit;
    wxCheckBox* m_ckbUserLimit;
    wxSpinCtrl* m_spcUserLimit;
    wxCheckBox* m_ckbBandShareLimit;
    wxSpinCtrl* m_spcBandShareLimit;
    wxCheckBox* m_ckbSecret;
    wxCheckBox* m_ckbOneAudio;
    wxSpinCtrl* m_spcOneAudio;
    wxCheckBox* m_ckbModerated;
    wxCheckBox* m_ckbAutoChangeBitRate;
    wxButton* m_btnOk;
    wxButton* m_btnCancel;

	// button events
	void OnOkBtnClick(wxCommandEvent &event);
	void OnCancelBtnClick(wxCommandEvent &event);
	
	//CheckBox events
	void OnChkboxBitRateClick( wxCommandEvent& event );
	void OnChkboxTransmissionLimitClick( wxCommandEvent& event );
	void OnChkboxBandShareClick( wxCommandEvent& event );
	void OnChkboxOneAudioClick( wxCommandEvent& event );
	void OnChkboxPasswordClick( wxCommandEvent& event );
	void OnChkboxUserLimitClick( wxCommandEvent& event );

};

#endif
    // CHANNEL_MODES_DIALOG_H