﻿// -----------------------------------------------------------------------------------------
//     VCEEnc by rigaya
// -----------------------------------------------------------------------------------------
// The MIT License
//
// Copyright (c) 2014-2017 rigaya
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// IABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//
// ------------------------------------------------------------------------------------------

//以下warning C4100を黙らせる
//C4100 : 引数は関数の本体部で 1 度も参照されません。
#pragma warning( push )
#pragma warning( disable: 4100 )

#include "auo_version.h"
#include "auo_frm.h"
#include "auo_faw2aac.h"
#include "frmConfig.h"
#include "frmSaveNewStg.h"
#include "frmOtherSettings.h"
#include "frmBitrateCalculator.h"
#include "vce_param.h"
#include "vce_cmd.h"

using namespace VCEEnc;

/// -------------------------------------------------
///     設定画面の表示
/// -------------------------------------------------
[STAThreadAttribute]
void ShowfrmConfig(CONF_GUIEX *conf, const SYSTEM_DATA *sys_dat) {
    if (!sys_dat->exstg->s_local.disable_visual_styles)
        System::Windows::Forms::Application::EnableVisualStyles();
    System::IO::Directory::SetCurrentDirectory(String(sys_dat->aviutl_dir).ToString());
    frmConfig frmConf(conf, sys_dat);
    frmConf.ShowDialog();
}

/// -------------------------------------------------
///     frmSaveNewStg 関数
/// -------------------------------------------------
System::Boolean frmSaveNewStg::checkStgFileName(String^ stgName) {
    String^ fileName;
    if (stgName->Length == 0)
        return false;

    if (!ValidiateFileName(stgName)) {
        MessageBox::Show(L"ファイル名に使用できない文字が含まれています。\n保存できません。", L"エラー", MessageBoxButtons::OK, MessageBoxIcon::Error);
        return false;
    }
    if (String::Compare(Path::GetExtension(stgName), L".stg", true))
        stgName += L".stg";
    if (File::Exists(fileName = Path::Combine(fsnCXFolderBrowser->GetSelectedFolder(), stgName)))
        if (MessageBox::Show(stgName + L" はすでに存在します。上書きしますか?", L"上書き確認", MessageBoxButtons::YesNo, MessageBoxIcon::Question)
            != System::Windows::Forms::DialogResult::Yes)
            return false;
    StgFileName = fileName;
    return true;
}

System::Void frmSaveNewStg::setStgDir(String^ _stgDir) {
    StgDir = _stgDir;
    fsnCXFolderBrowser->SetRootDirAndReload(StgDir);
}


/// -------------------------------------------------
///     frmBitrateCalculator 関数
/// -------------------------------------------------
System::Void frmBitrateCalculator::Init(int VideoBitrate, int AudioBitrate, bool BTVBEnable, bool BTABEnable, int ab_max, const AuoTheme themeTo, const DarkenWindowStgReader *dwStg) {
    guiEx_settings exStg(true);
    exStg.load_fbc();
    enable_events = false;
    dwStgReader = dwStg;
    CheckTheme(themeTo);
    fbcTXSize->Text = exStg.s_fbc.initial_size.ToString("F2");
    fbcChangeTimeSetMode(exStg.s_fbc.calc_time_from_frame != 0);
    fbcRBCalcRate->Checked = exStg.s_fbc.calc_bitrate != 0;
    fbcRBCalcSize->Checked = !fbcRBCalcRate->Checked;
    fbcTXMovieFrameRate->Text = Convert::ToString(exStg.s_fbc.last_fps);
    fbcNUMovieFrames->Value = exStg.s_fbc.last_frame_num;
    fbcNULengthHour->Value = Convert::ToDecimal((int)exStg.s_fbc.last_time_in_sec / 3600);
    fbcNULengthMin->Value = Convert::ToDecimal((int)(exStg.s_fbc.last_time_in_sec % 3600) / 60);
    fbcNULengthSec->Value =  Convert::ToDecimal((int)exStg.s_fbc.last_time_in_sec % 60);
    SetBTVBEnabled(BTVBEnable);
    SetBTABEnabled(BTABEnable, ab_max);
    SetNUVideoBitrate(VideoBitrate);
    SetNUAudioBitrate(AudioBitrate);
    enable_events = true;
}
System::Void frmBitrateCalculator::CheckTheme(const AuoTheme themeTo) {
    //変更の必要がなければ終了
    if (themeTo == themeMode) return;

    //一度ウィンドウの再描画を完全に抑止する
    SendMessage(reinterpret_cast<HWND>(this->Handle.ToPointer()), WM_SETREDRAW, 0, 0);
    SetAllColor(this, themeTo, this->GetType(), dwStgReader);
    SetAllMouseMove(this, themeTo);
    //一度ウィンドウの再描画を再開し、強制的に再描画させる
    SendMessage(reinterpret_cast<HWND>(this->Handle.ToPointer()), WM_SETREDRAW, 1, 0);
    this->Refresh();
    themeMode = themeTo;
}
System::Void frmBitrateCalculator::frmBitrateCalculator_FormClosing(System::Object^  sender, System::Windows::Forms::FormClosingEventArgs^  e) {
    guiEx_settings exStg(true);
    exStg.load_fbc();
    exStg.s_fbc.calc_bitrate = fbcRBCalcRate->Checked;
    exStg.s_fbc.calc_time_from_frame = fbcPNMovieFrames->Visible;
    exStg.s_fbc.last_fps = Convert::ToDouble(fbcTXMovieFrameRate->Text);
    exStg.s_fbc.last_frame_num = Convert::ToInt32(fbcNUMovieFrames->Value);
    exStg.s_fbc.last_time_in_sec = Convert::ToInt32(fbcNULengthHour->Value) * 3600
                                 + Convert::ToInt32(fbcNULengthMin->Value) * 60
                                 + Convert::ToInt32(fbcNULengthSec->Value);
    if (fbcRBCalcRate->Checked)
        exStg.s_fbc.initial_size = Convert::ToDouble(fbcTXSize->Text);
    exStg.save_fbc();
    frmConfig^ fcg = dynamic_cast<frmConfig^>(this->Owner);
    if (fcg != nullptr)
        fcg->InformfbcClosed();
}
System::Void frmBitrateCalculator::fbcRBCalcRate_CheckedChanged(System::Object^  sender, System::EventArgs^  e) {
    if (fbcRBCalcRate->Checked && Convert::ToDouble(fbcTXSize->Text) <= 0.0) {
        guiEx_settings exStg(true);
        exStg.load_fbc();
        fbcTXSize->Text = exStg.s_fbc.initial_size.ToString("F2");
    }
}
System::Void frmBitrateCalculator::fbcBTVBApply_Click(System::Object^  sender, System::EventArgs^  e) {
    frmConfig^ fcg = dynamic_cast<frmConfig^>(this->Owner);
    if (fcg != nullptr)
        fcg->SetVideoBitrate((int)fbcNUBitrateVideo->Value);
}
System::Void frmBitrateCalculator::fbcBTABApply_Click(System::Object^  sender, System::EventArgs^  e) {
    frmConfig^ fcg = dynamic_cast<frmConfig^>(this->Owner);
    if (fcg != nullptr)
        fcg->SetAudioBitrate((int)fbcNUBitrateAudio->Value);
}
System::Void frmBitrateCalculator::fbcMouseEnter_SetColor(System::Object^  sender, System::EventArgs^  e) {
    fcgMouseEnterLeave_SetColor(sender, themeMode, DarkenWindowState::Hot, dwStgReader);
}
System::Void frmBitrateCalculator::fbcMouseLeave_SetColor(System::Object^  sender, System::EventArgs^  e) {
    fcgMouseEnterLeave_SetColor(sender, themeMode, DarkenWindowState::Normal, dwStgReader);
}
System::Void frmBitrateCalculator::SetAllMouseMove(Control ^top, const AuoTheme themeTo) {
    if (themeTo == themeMode) return;
    System::Type^ type = top->GetType();
    if (type == CheckBox::typeid) {
        top->MouseEnter += gcnew System::EventHandler(this, &frmBitrateCalculator::fbcMouseEnter_SetColor);
        top->MouseLeave += gcnew System::EventHandler(this, &frmBitrateCalculator::fbcMouseLeave_SetColor);
    }
    for (int i = 0; i < top->Controls->Count; i++) {
        SetAllMouseMove(top->Controls[i], themeTo);
    }
}


/// -------------------------------------------------
///     frmConfig 関数  (frmBitrateCalculator関連)
/// -------------------------------------------------
System::Void frmConfig::CloseBitrateCalc() {
    frmBitrateCalculator::Instance::get()->Close();
}
System::Void frmConfig::fcgTSBBitrateCalc_CheckedChanged(System::Object^  sender, System::EventArgs^  e) {
    if (fcgTSBBitrateCalc->Checked) {
        bool videoBitrateMode = true;

        frmBitrateCalculator::Instance::get()->Init(
            (int)fcgNUBitrate->Value,
            (fcgNUAudioBitrate->Visible) ? (int)fcgNUAudioBitrate->Value : 0,
            videoBitrateMode,
            fcgNUAudioBitrate->Visible,
            (int)fcgNUAudioBitrate->Maximum,
            themeMode,
            dwStgReader
            );
        frmBitrateCalculator::Instance::get()->Owner = this;
        frmBitrateCalculator::Instance::get()->Show();
    } else {
        frmBitrateCalculator::Instance::get()->Close();
    }
}
System::Void frmConfig::SetfbcBTABEnable(bool enable, int max) {
    frmBitrateCalculator::Instance::get()->SetBTABEnabled(fcgNUAudioBitrate->Visible, max);
}
System::Void frmConfig::SetfbcBTVBEnable(bool enable) {
    frmBitrateCalculator::Instance::get()->SetBTVBEnabled(enable);
}

System::Void frmConfig::SetVideoBitrate(int bitrate) {
    SetNUValue(fcgNUBitrate, bitrate);
}

System::Void frmConfig::SetAudioBitrate(int bitrate) {
    SetNUValue(fcgNUAudioBitrate, bitrate);
}
System::Void frmConfig::InformfbcClosed() {
    fcgTSBBitrateCalc->Checked = false;
}


/// -------------------------------------------------
///     frmConfig 関数
/// -------------------------------------------------
/////////////   LocalStg関連  //////////////////////
System::Void frmConfig::LoadLocalStg() {
    guiEx_settings *_ex_stg = sys_dat->exstg;
    _ex_stg->load_encode_stg();
    LocalStg.CustomTmpDir    = String(_ex_stg->s_local.custom_tmp_dir).ToString();
    LocalStg.CustomAudTmpDir = String(_ex_stg->s_local.custom_audio_tmp_dir).ToString();
    LocalStg.CustomMP4TmpDir = String(_ex_stg->s_local.custom_mp4box_tmp_dir).ToString();
    LocalStg.LastAppDir      = String(_ex_stg->s_local.app_dir).ToString();
    LocalStg.LastBatDir      = String(_ex_stg->s_local.bat_dir).ToString();
    LocalStg.vidEncName      = String(_ex_stg->s_vid.filename).ToString();
    LocalStg.vidEncPath      = String(_ex_stg->s_vid.fullpath).ToString();
    LocalStg.MP4MuxerExeName = String(_ex_stg->s_mux[MUXER_MP4].filename).ToString();
    LocalStg.MP4MuxerPath    = String(_ex_stg->s_mux[MUXER_MP4].fullpath).ToString();
    LocalStg.MKVMuxerExeName = String(_ex_stg->s_mux[MUXER_MKV].filename).ToString();
    LocalStg.MKVMuxerPath    = String(_ex_stg->s_mux[MUXER_MKV].fullpath).ToString();
    LocalStg.TC2MP4ExeName   = String(_ex_stg->s_mux[MUXER_TC2MP4].filename).ToString();
    LocalStg.TC2MP4Path      = String(_ex_stg->s_mux[MUXER_TC2MP4].fullpath).ToString();
    LocalStg.MPGMuxerExeName = String(_ex_stg->s_mux[MUXER_MPG].filename).ToString();
    LocalStg.MPGMuxerPath    = String(_ex_stg->s_mux[MUXER_MPG].fullpath).ToString();
    LocalStg.MP4RawExeName   = String(_ex_stg->s_mux[MUXER_MP4_RAW].filename).ToString();
    LocalStg.MP4RawPath      = String(_ex_stg->s_mux[MUXER_MP4_RAW].fullpath).ToString();

    LocalStg.audEncName->Clear();
    LocalStg.audEncExeName->Clear();
    LocalStg.audEncPath->Clear();
    for (int i = 0; i < _ex_stg->s_aud_ext_count; i++) {
        LocalStg.audEncName->Add(String(_ex_stg->s_aud_ext[i].dispname).ToString());
        LocalStg.audEncExeName->Add(String(_ex_stg->s_aud_ext[i].filename).ToString());
        LocalStg.audEncPath->Add(String(_ex_stg->s_aud_ext[i].fullpath).ToString());
    }
    if (_ex_stg->s_local.large_cmdbox)
        fcgTXCmd_DoubleClick(nullptr, nullptr); //初期状態は縮小なので、拡大
}

System::Boolean frmConfig::CheckLocalStg() {
    bool error = false;
    String^ err = "";
    //映像エンコーダのチェック
    if (LocalStg.vidEncPath->Length > 0
        && !File::Exists(LocalStg.vidEncPath)) {
        if (!error) err += L"\n\n";
        error = true;
        err += L"指定された 動画エンコーダ は存在しません。\n [ " + LocalStg.vidEncPath + L" ]\n";
    }
    //音声エンコーダのチェック (実行ファイル名がない場合はチェックしない)
    if (fcgCBAudioUseExt->Checked
        && LocalStg.audEncExeName[fcgCXAudioEncoder->SelectedIndex]->Length) {
        String^ AudioEncoderPath = LocalStg.audEncPath[fcgCXAudioEncoder->SelectedIndex];
        if (AudioEncoderPath->Length > 0
            && !File::Exists(AudioEncoderPath)
            && (fcgCXAudioEncoder->SelectedIndex != sys_dat->exstg->get_faw_index(!fcgCBAudioUseExt->Checked)
                || !check_if_faw2aac_exists()) ) {
            //音声実行ファイルがない かつ
            //選択された音声がfawでない または fawであってもfaw2aacがない
            if (!error) err += L"\n\n";
            error = true;
            err += L"指定された 音声エンコーダ は存在しません。\n [ " + AudioEncoderPath + L" ]\n";
        }
    }
    //FAWのチェック
    if (fcgCBFAWCheck->Checked) {
        if (sys_dat->exstg->get_faw_index(!fcgCBAudioUseExt->Checked) == FAW_INDEX_ERROR) {
            if (!error) err += L"\n\n";
            error = true;
            err += L"FAWCheckが選択されましたが、VCEEnc.ini から\n"
                + L"FAW の設定を読み込めませんでした。\n"
                + L"VCEEnc.ini を確認してください。\n";
        } else if (fcgCBAudioUseExt->Checked
                   && !File::Exists(LocalStg.audEncPath[sys_dat->exstg->get_faw_index(!fcgCBAudioUseExt->Checked)])
                   && !check_if_faw2aac_exists()) {
            //fawの実行ファイルが存在しない かつ faw2aacも存在しない
            if (!error) err += L"\n\n";
            error = true;
            err += L"FAWCheckが選択されましたが、FAW(fawcl)へのパスが正しく指定されていません。\n"
                +  L"一度設定画面でFAW(fawcl)へのパスを指定してください。\n";
        }
    }
    if (error)
        MessageBox::Show(this, err, L"エラー", MessageBoxButtons::OK, MessageBoxIcon::Error);
    return error;
}

System::Void frmConfig::SaveLocalStg() {
    guiEx_settings *_ex_stg = sys_dat->exstg;
    _ex_stg->load_encode_stg();
    _ex_stg->s_local.large_cmdbox = fcgTXCmd->Multiline;
    GetCHARfromString(_ex_stg->s_local.custom_tmp_dir,        sizeof(_ex_stg->s_local.custom_tmp_dir),        LocalStg.CustomTmpDir);
    GetCHARfromString(_ex_stg->s_local.custom_mp4box_tmp_dir, sizeof(_ex_stg->s_local.custom_mp4box_tmp_dir), LocalStg.CustomMP4TmpDir);
    GetCHARfromString(_ex_stg->s_local.custom_audio_tmp_dir,  sizeof(_ex_stg->s_local.custom_audio_tmp_dir),  LocalStg.CustomAudTmpDir);
    GetCHARfromString(_ex_stg->s_local.app_dir,               sizeof(_ex_stg->s_local.app_dir),               LocalStg.LastAppDir);
    GetCHARfromString(_ex_stg->s_local.bat_dir,               sizeof(_ex_stg->s_local.bat_dir),               LocalStg.LastBatDir);
    GetCHARfromString(_ex_stg->s_vid.fullpath,                sizeof(_ex_stg->s_vid.fullpath),                LocalStg.vidEncPath);
    GetCHARfromString(_ex_stg->s_mux[MUXER_MP4].fullpath,     sizeof(_ex_stg->s_mux[MUXER_MP4].fullpath),     LocalStg.MP4MuxerPath);
    GetCHARfromString(_ex_stg->s_mux[MUXER_MKV].fullpath,     sizeof(_ex_stg->s_mux[MUXER_MKV].fullpath),     LocalStg.MKVMuxerPath);
    GetCHARfromString(_ex_stg->s_mux[MUXER_TC2MP4].fullpath,  sizeof(_ex_stg->s_mux[MUXER_TC2MP4].fullpath),  LocalStg.TC2MP4Path);
    GetCHARfromString(_ex_stg->s_mux[MUXER_MPG].fullpath,     sizeof(_ex_stg->s_mux[MUXER_MPG].fullpath),     LocalStg.MPGMuxerPath);
    GetCHARfromString(_ex_stg->s_mux[MUXER_MP4_RAW].fullpath, sizeof(_ex_stg->s_mux[MUXER_MP4_RAW].fullpath), LocalStg.MP4RawPath);
    for (int i = 0; i < _ex_stg->s_aud_ext_count; i++)
        GetCHARfromString(_ex_stg->s_aud_ext[i].fullpath, sizeof(_ex_stg->s_aud_ext[i].fullpath), LocalStg.audEncPath[i]);
    _ex_stg->save_local();
}

System::Void frmConfig::SetLocalStg() {
    fcgTXVideoEncoderPath->Text   = LocalStg.vidEncPath;
    fcgTXMP4MuxerPath->Text       = LocalStg.MP4MuxerPath;
    fcgTXMKVMuxerPath->Text       = LocalStg.MKVMuxerPath;
    fcgTXTC2MP4Path->Text         = LocalStg.TC2MP4Path;
    fcgTXMPGMuxerPath->Text       = LocalStg.MPGMuxerPath;
    fcgTXMP4RawPath->Text         = LocalStg.MP4RawPath;
    fcgTXCustomAudioTempDir->Text = LocalStg.CustomAudTmpDir;
    fcgTXCustomTempDir->Text      = LocalStg.CustomTmpDir;
    fcgTXMP4BoxTempDir->Text      = LocalStg.CustomMP4TmpDir;
    fcgLBMP4MuxerPath->Text       = LocalStg.MP4MuxerExeName + L" の指定";
    fcgLBMKVMuxerPath->Text       = LocalStg.MKVMuxerExeName + L" の指定";
    fcgLBTC2MP4Path->Text         = LocalStg.TC2MP4ExeName   + L" の指定";
    fcgLBMPGMuxerPath->Text       = LocalStg.MPGMuxerExeName + L" の指定";
    fcgLBMP4RawPath->Text         = LocalStg.MP4RawExeName + L" の指定";

    fcgTXMP4MuxerPath->SelectionStart     = fcgTXMP4MuxerPath->Text->Length;
    fcgTXTC2MP4Path->SelectionStart       = fcgTXTC2MP4Path->Text->Length;
    fcgTXMKVMuxerPath->SelectionStart     = fcgTXMKVMuxerPath->Text->Length;
    fcgTXMPGMuxerPath->SelectionStart     = fcgTXMPGMuxerPath->Text->Length;
    fcgTXMP4RawPath->SelectionStart       = fcgTXMP4RawPath->Text->Length;
}

//////////////       その他イベント処理   ////////////////////////
System::Void frmConfig::ActivateToolTip(bool Enable) {
    fcgTTEx->Active = Enable;
}

System::Void frmConfig::fcgTSBOtherSettings_Click(System::Object^  sender, System::EventArgs^  e) {
    frmOtherSettings::Instance::get()->stgDir = String(sys_dat->exstg->s_local.stg_dir).ToString();
    frmOtherSettings::Instance::get()->SetTheme(themeMode, dwStgReader);
    frmOtherSettings::Instance::get()->ShowDialog();
    char buf[MAX_PATH_LEN];
    GetCHARfromString(buf, sizeof(buf), frmOtherSettings::Instance::get()->stgDir);
    if (_stricmp(buf, sys_dat->exstg->s_local.stg_dir)) {
        //変更があったら保存する
        strcpy_s(sys_dat->exstg->s_local.stg_dir, sizeof(sys_dat->exstg->s_local.stg_dir), buf);
        sys_dat->exstg->save_local();
        InitStgFileList();
    }
    //再読み込み
    guiEx_settings stg;
    stg.load_encode_stg();
    log_reload_settings();
    sys_dat->exstg->s_local.default_audenc_use_in = stg.s_local.default_audenc_use_in;
    sys_dat->exstg->s_local.default_audio_encoder_ext = stg.s_local.default_audio_encoder_ext;
    sys_dat->exstg->s_local.default_audio_encoder_in = stg.s_local.default_audio_encoder_in;
    sys_dat->exstg->s_local.get_relative_path = stg.s_local.get_relative_path;
    SetStgEscKey(stg.s_local.enable_stg_esc_key != 0);
    ActivateToolTip(stg.s_local.disable_tooltip_help == FALSE);
    if (str_has_char(stg.s_local.conf_font.name))
        SetFontFamilyToForm(this, gcnew FontFamily(String(stg.s_local.conf_font.name).ToString()), this->Font->FontFamily);
}
System::Boolean frmConfig::EnableSettingsNoteChange(bool Enable) {
    if (fcgTSTSettingsNotes->Visible == Enable &&
        fcgTSLSettingsNotes->Visible == !Enable)
        return true;
    if (CountStringBytes(fcgTSTSettingsNotes->Text) > fcgTSTSettingsNotes->MaxLength - 1) {
        MessageBox::Show(this, L"入力された文字数が多すぎます。減らしてください。", L"エラー", MessageBoxButtons::OK, MessageBoxIcon::Error);
        fcgTSTSettingsNotes->Focus();
        fcgTSTSettingsNotes->SelectionStart = fcgTSTSettingsNotes->Text->Length;
        return false;
    }
    fcgTSTSettingsNotes->Visible = Enable;
    fcgTSLSettingsNotes->Visible = !Enable;
    if (Enable) {
        fcgTSTSettingsNotes->Text = fcgTSLSettingsNotes->Text;
        fcgTSTSettingsNotes->Focus();
        bool isDefaultNote = String::Compare(fcgTSTSettingsNotes->Text, String(DefaultStgNotes).ToString()) == 0;
        fcgTSTSettingsNotes->Select((isDefaultNote) ? 0 : fcgTSTSettingsNotes->Text->Length, fcgTSTSettingsNotes->Text->Length);
    } else {
        SetfcgTSLSettingsNotes(fcgTSTSettingsNotes->Text);
        CheckOtherChanges(nullptr, nullptr);
    }
    return true;
}
System::Void frmConfig::fcgTSLSettingsNotes_DoubleClick(System::Object^  sender, System::EventArgs^  e) {
    EnableSettingsNoteChange(true);
}
System::Void frmConfig::fcgTSTSettingsNotes_Leave(System::Object^  sender, System::EventArgs^  e) {
    EnableSettingsNoteChange(false);
}
System::Void frmConfig::fcgTSTSettingsNotes_KeyDown(System::Object^  sender, System::Windows::Forms::KeyEventArgs^  e) {
    if (e->KeyCode == Keys::Return)
        EnableSettingsNoteChange(false);
}
System::Void frmConfig::fcgTSTSettingsNotes_TextChanged(System::Object^  sender, System::EventArgs^  e) {
    SetfcgTSLSettingsNotes(fcgTSTSettingsNotes->Text);
    CheckOtherChanges(nullptr, nullptr);
}

/////////////    音声設定関連の関数    ///////////////
System::Void frmConfig::fcgCBAudio2pass_CheckedChanged(System::Object^  sender, System::EventArgs^  e) {
    if (fcgCBAudio2pass->Checked) {
        fcgCBAudioUsePipe->Checked = false;
        fcgCBAudioUsePipe->Enabled = false;
    } else if (CurrentPipeEnabled) {
        fcgCBAudioUsePipe->Checked = true;
        fcgCBAudioUsePipe->Enabled = true;
    }
}

System::Void frmConfig::fcgCXAudioEncoder_SelectedIndexChanged(System::Object ^sender, System::EventArgs ^e) {
    setAudioExtDisplay();
}

System::Void frmConfig::fcgCXAudioEncMode_SelectedIndexChanged(System::Object ^sender, System::EventArgs ^e) {
    AudioExtEncodeModeChanged();
}

System::Int32 frmConfig::GetCurrentAudioDefaultBitrate() {
    return sys_dat->exstg->s_aud_ext[fcgCXAudioEncoder->SelectedIndex].mode[fcgCXAudioEncMode->SelectedIndex].bitrate_default;
}

System::Void frmConfig::setAudioExtDisplay() {
    AUDIO_SETTINGS *astg = &sys_dat->exstg->s_aud_ext[fcgCXAudioEncoder->SelectedIndex];
    //～の指定
    if (str_has_char(astg->filename)) {
        fcgLBAudioEncoderPath->Text = String(astg->filename).ToString() + L" の指定";
        fcgTXAudioEncoderPath->Enabled = true;
        fcgTXAudioEncoderPath->Text = LocalStg.audEncPath[fcgCXAudioEncoder->SelectedIndex];
        fcgBTAudioEncoderPath->Enabled = true;
    } else {
        //filename空文字列(wav出力時)
        fcgLBAudioEncoderPath->Text = L"";
        fcgTXAudioEncoderPath->Enabled = false;
        fcgTXAudioEncoderPath->Text = L"";
        fcgBTAudioEncoderPath->Enabled = false;
    }
    fcgTXAudioEncoderPath->SelectionStart = fcgTXAudioEncoderPath->Text->Length;
    fcgCXAudioEncMode->BeginUpdate();
    fcgCXAudioEncMode->Items->Clear();
    for (int i = 0; i < astg->mode_count; i++) {
        fcgCXAudioEncMode->Items->Add(String(astg->mode[i].name).ToString());
    }
    fcgCXAudioEncMode->EndUpdate();
    bool pipe_enabled = (astg->pipe_input && (!(fcgCBAudio2pass->Checked && astg->mode[fcgCXAudioEncMode->SelectedIndex].enc_2pass != 0)));
    CurrentPipeEnabled = pipe_enabled;
    fcgCBAudioUsePipe->Enabled = pipe_enabled;
    fcgCBAudioUsePipe->Checked = pipe_enabled;
    if (fcgCXAudioEncMode->Items->Count > 0)
        fcgCXAudioEncMode->SelectedIndex = 0;
}

System::Void frmConfig::AudioExtEncodeModeChanged() {
    int index = fcgCXAudioEncMode->SelectedIndex;
    AUDIO_SETTINGS *astg = &sys_dat->exstg->s_aud_ext[fcgCXAudioEncoder->SelectedIndex];
    if (astg->mode[index].bitrate) {
        fcgCXAudioEncMode->Width = fcgCXAudioEncModeSmallWidth;
        fcgLBAudioBitrate->Visible = true;
        fcgNUAudioBitrate->Visible = true;
        fcgNUAudioBitrate->Minimum = astg->mode[index].bitrate_min;
        fcgNUAudioBitrate->Maximum = astg->mode[index].bitrate_max;
        fcgNUAudioBitrate->Increment = astg->mode[index].bitrate_step;
        SetNUValue(fcgNUAudioBitrate, (conf->aud.ext.bitrate != 0) ? conf->aud.ext.bitrate : astg->mode[index].bitrate_default);
    } else {
        fcgCXAudioEncMode->Width = fcgCXAudioEncModeLargeWidth;
        fcgLBAudioBitrate->Visible = false;
        fcgNUAudioBitrate->Visible = false;
        fcgNUAudioBitrate->Minimum = 0;
        fcgNUAudioBitrate->Maximum = 65536;
    }
    fcgCBAudio2pass->Enabled = astg->mode[index].enc_2pass != 0;
    if (!fcgCBAudio2pass->Enabled) fcgCBAudio2pass->Checked = false;
    SetfbcBTABEnable(fcgNUAudioBitrate->Visible, (int)fcgNUAudioBitrate->Maximum);

    bool delay_cut_available = astg->mode[index].delay > 0;
    fcgLBAudioDelayCut->Visible = delay_cut_available;
    fcgCXAudioDelayCut->Visible = delay_cut_available;
    if (delay_cut_available) {
        const bool delay_cut_edts_available = str_has_char(astg->cmd_raw) && str_has_char(sys_dat->exstg->s_mux[MUXER_MP4_RAW].delay_cmd);
        const int current_idx = fcgCXAudioDelayCut->SelectedIndex;
        const int items_to_set = _countof(AUDIO_DELAY_CUT_MODE) - 1 - ((delay_cut_edts_available) ? 0 : 1);
        fcgCXAudioDelayCut->BeginUpdate();
        fcgCXAudioDelayCut->Items->Clear();
        for (int i = 0; i < items_to_set; i++)
            fcgCXAudioDelayCut->Items->Add(String(AUDIO_DELAY_CUT_MODE[i]).ToString());
        fcgCXAudioDelayCut->EndUpdate();
        fcgCXAudioDelayCut->SelectedIndex = (current_idx >= items_to_set) ? 0 : current_idx;
    } else {
        fcgCXAudioDelayCut->SelectedIndex = 0;
    }
}

System::Void frmConfig::fcgCBAudioUseExt_CheckedChanged(System::Object ^sender, System::EventArgs ^e) {
    fcgPNAudioExt->Visible = fcgCBAudioUseExt->Checked;
    fcgPNAudioInternal->Visible = !fcgCBAudioUseExt->Checked;

    //一度ウィンドウの再描画を完全に抑止する
    SendMessage(reinterpret_cast<HWND>(this->Handle.ToPointer()), WM_SETREDRAW, 0, 0);
    //なぜか知らんが、Visibleプロパティをfalseにするだけでは非表示にできない
    //しょうがないので参照の削除と挿入を行う
    fcgtabControlMux->TabPages->Clear();
    if (fcgCBAudioUseExt->Checked) {
        fcgtabControlMux->TabPages->Insert(0, fcgtabPageMP4);
        fcgtabControlMux->TabPages->Insert(1, fcgtabPageMKV);
        fcgtabControlMux->TabPages->Insert(2, fcgtabPageBat);
        fcgtabControlMux->TabPages->Insert(3, fcgtabPageMux);
    } else {
        fcgtabControlMux->TabPages->Insert(0, fcgtabPageInternal);
        fcgtabControlMux->TabPages->Insert(1, fcgtabPageBat);
        fcgtabControlMux->TabPages->Insert(2, fcgtabPageMux);
    }
    //一度ウィンドウの再描画を再開し、強制的に再描画させる
    SendMessage(reinterpret_cast<HWND>(this->Handle.ToPointer()), WM_SETREDRAW, 1, 0);
    this->Refresh();
}

System::Void frmConfig::fcgCXAudioEncoderInternal_SelectedIndexChanged(System::Object ^sender, System::EventArgs ^e) {
    setAudioIntDisplay();
}
System::Void frmConfig::fcgCXAudioEncModeInternal_SelectedIndexChanged(System::Object ^sender, System::EventArgs ^e) {
    AudioIntEncodeModeChanged();
}

bool frmConfig::AudioIntEncoderEnabled(const AUDIO_SETTINGS *astg, bool isAuoLinkMode) {
    if (isAuoLinkMode && astg->auolink_only < 0) {
        return false;
    } else if (!isAuoLinkMode && astg->auolink_only > 0) {
        return false;
    }
    return true;
}

System::Void frmConfig::setAudioIntDisplay() {
    AUDIO_SETTINGS *astg = &sys_dat->exstg->s_aud_int[fcgCXAudioEncoderInternal->SelectedIndex];
    if (!AudioIntEncoderEnabled(astg, false)) {
        fcgCXAudioEncoderInternal->SelectedIndex = DEFAULT_AUDIO_ENCODER_IN;
        astg = &sys_dat->exstg->s_aud_int[fcgCXAudioEncoderInternal->SelectedIndex];
    }
    fcgCXAudioEncModeInternal->BeginUpdate();
    fcgCXAudioEncModeInternal->Items->Clear();
    if (AudioIntEncoderEnabled(astg, false)) {
        for (int i = 0; i < astg->mode_count; i++) {
            fcgCXAudioEncModeInternal->Items->Add(String(astg->mode[i].name).ToString());
        }
    } else {
        fcgCXAudioEncModeInternal->Items->Add(String(L"-----").ToString());
    }
    fcgCXAudioEncModeInternal->EndUpdate();
    if (fcgCXAudioEncModeInternal->Items->Count > 0)
        fcgCXAudioEncModeInternal->SelectedIndex = 0;
}
System::Void frmConfig::AudioIntEncodeModeChanged() {
    const int imode = fcgCXAudioEncModeInternal->SelectedIndex;
    if (imode >= 0 && fcgCXAudioEncoderInternal->SelectedIndex >= 0) {
        AUDIO_SETTINGS *astg = &sys_dat->exstg->s_aud_int[fcgCXAudioEncoderInternal->SelectedIndex];
        if (astg->mode[imode].bitrate) {
            fcgCXAudioEncModeInternal->Width = fcgCXAudioEncModeSmallWidth;
            fcgLBAudioBitrateInternal->Visible = true;
            fcgNUAudioBitrateInternal->Visible = true;
            fcgNUAudioBitrateInternal->Minimum = astg->mode[imode].bitrate_min;
            fcgNUAudioBitrateInternal->Maximum = astg->mode[imode].bitrate_max;
            fcgNUAudioBitrateInternal->Increment = astg->mode[imode].bitrate_step;
            SetNUValue(fcgNUAudioBitrateInternal, (conf->aud.in.bitrate > 0) ? conf->aud.in.bitrate : astg->mode[imode].bitrate_default);
        } else {
            fcgCXAudioEncModeInternal->Width = fcgCXAudioEncModeLargeWidth;
            fcgLBAudioBitrateInternal->Visible = false;
            fcgNUAudioBitrateInternal->Visible = false;
            fcgNUAudioBitrateInternal->Minimum = 0;
            fcgNUAudioBitrateInternal->Maximum = 65536;
        }
    }
    SetfbcBTABEnable(fcgNUAudioBitrateInternal->Visible, (int)fcgNUAudioBitrateInternal->Maximum);
}

///////////////   設定ファイル関連   //////////////////////
System::Void frmConfig::CheckTSItemsEnabled(CONF_GUIEX *current_conf) {
    bool selected = (CheckedStgMenuItem != nullptr);
    fcgTSBSave->Enabled = (selected && memcmp(cnf_stgSelected, current_conf, sizeof(CONF_GUIEX)));
    fcgTSBDelete->Enabled = selected;
}

System::Void frmConfig::UncheckAllDropDownItem(ToolStripItem^ mItem) {
    ToolStripDropDownItem^ DropDownItem = dynamic_cast<ToolStripDropDownItem^>(mItem);
    if (DropDownItem == nullptr)
        return;
    for (int i = 0; i < DropDownItem->DropDownItems->Count; i++) {
        UncheckAllDropDownItem(DropDownItem->DropDownItems[i]);
        ToolStripMenuItem^ item = dynamic_cast<ToolStripMenuItem^>(DropDownItem->DropDownItems[i]);
        if (item != nullptr)
            item->Checked = false;
    }
}

System::Void frmConfig::CheckTSSettingsDropDownItem(ToolStripMenuItem^ mItem) {
    UncheckAllDropDownItem(fcgTSSettings);
    CheckedStgMenuItem = mItem;
    fcgTSSettings->Text = (mItem == nullptr) ? L"プロファイル" : mItem->Text;
    if (mItem != nullptr)
        mItem->Checked = true;
    fcgTSBSave->Enabled = false;
    fcgTSBDelete->Enabled = (mItem != nullptr);
}

ToolStripMenuItem^ frmConfig::fcgTSSettingsSearchItem(String^ stgPath, ToolStripItem^ mItem) {
    if (stgPath == nullptr)
        return nullptr;
    ToolStripDropDownItem^ DropDownItem = dynamic_cast<ToolStripDropDownItem^>(mItem);
    if (DropDownItem == nullptr)
        return nullptr;
    for (int i = 0; i < DropDownItem->DropDownItems->Count; i++) {
        ToolStripMenuItem^ item = fcgTSSettingsSearchItem(stgPath, DropDownItem->DropDownItems[i]);
        if (item != nullptr)
            return item;
        item = dynamic_cast<ToolStripMenuItem^>(DropDownItem->DropDownItems[i]);
        if (item      != nullptr &&
            item->Tag != nullptr &&
            0 == String::Compare(item->Tag->ToString(), stgPath, true))
            return item;
    }
    return nullptr;
}

ToolStripMenuItem^ frmConfig::fcgTSSettingsSearchItem(String^ stgPath) {
    return fcgTSSettingsSearchItem((stgPath != nullptr && stgPath->Length > 0) ? Path::GetFullPath(stgPath) : nullptr, fcgTSSettings);
}

System::Void frmConfig::SaveToStgFile(String^ stgName) {
    size_t nameLen = CountStringBytes(stgName) + 1;
    char *stg_name = (char *)malloc(nameLen);
    GetCHARfromString(stg_name, nameLen, stgName);
    init_CONF_GUIEX(cnf_stgSelected, FALSE);
    FrmToConf(cnf_stgSelected);
    String^ stgDir = Path::GetDirectoryName(stgName);
    if (!Directory::Exists(stgDir))
        Directory::CreateDirectory(stgDir);
    int result = guiEx_config::save_guiEx_conf(cnf_stgSelected, stg_name);
    free(stg_name);
    switch (result) {
        case CONF_ERROR_FILE_OPEN:
            MessageBox::Show(L"設定ファイルオープンに失敗しました。", L"エラー", MessageBoxButtons::OK, MessageBoxIcon::Error);
            return;
        case CONF_ERROR_INVALID_FILENAME:
            MessageBox::Show(L"ファイル名に使用できない文字が含まれています。\n保存できません。", L"エラー", MessageBoxButtons::OK, MessageBoxIcon::Error);
            return;
        default:
            break;
    }
    init_CONF_GUIEX(cnf_stgSelected, FALSE);
    FrmToConf(cnf_stgSelected);
}

System::Void frmConfig::fcgTSBSave_Click(System::Object^  sender, System::EventArgs^  e) {
    if (CheckedStgMenuItem != nullptr)
        SaveToStgFile(CheckedStgMenuItem->Tag->ToString());
    CheckTSSettingsDropDownItem(CheckedStgMenuItem);
}

System::Void frmConfig::fcgTSBSaveNew_Click(System::Object^  sender, System::EventArgs^  e) {
    frmSaveNewStg::Instance::get()->setStgDir(String(sys_dat->exstg->s_local.stg_dir).ToString());
    frmSaveNewStg::Instance::get()->SetTheme(themeMode, dwStgReader);
    if (CheckedStgMenuItem != nullptr)
        frmSaveNewStg::Instance::get()->setFilename(CheckedStgMenuItem->Text);
    frmSaveNewStg::Instance::get()->ShowDialog();
    String^ stgName = frmSaveNewStg::Instance::get()->StgFileName;
    if (stgName != nullptr && stgName->Length)
        SaveToStgFile(stgName);
    RebuildStgFileDropDown(nullptr);
    CheckTSSettingsDropDownItem(fcgTSSettingsSearchItem(stgName));
}

System::Void frmConfig::DeleteStgFile(ToolStripMenuItem^ mItem) {
    if (System::Windows::Forms::DialogResult::OK ==
        MessageBox::Show(L"設定ファイル " + mItem->Text + L" を削除してよろしいですか?",
        L"エラー", MessageBoxButtons::OKCancel, MessageBoxIcon::Exclamation))
    {
        File::Delete(mItem->Tag->ToString());
        RebuildStgFileDropDown(nullptr);
        CheckTSSettingsDropDownItem(nullptr);
        SetfcgTSLSettingsNotes(L"");
    }
}

System::Void frmConfig::fcgTSBDelete_Click(System::Object^  sender, System::EventArgs^  e) {
    DeleteStgFile(CheckedStgMenuItem);
}

System::Void frmConfig::fcgTSSettings_DropDownItemClicked(System::Object^  sender, System::Windows::Forms::ToolStripItemClickedEventArgs^  e) {
    ToolStripMenuItem^ ClickedMenuItem = dynamic_cast<ToolStripMenuItem^>(e->ClickedItem);
    if (ClickedMenuItem == nullptr)
        return;
    if (ClickedMenuItem->Tag == nullptr || ClickedMenuItem->Tag->ToString()->Length == 0)
        return;
    CONF_GUIEX load_stg;
    char stg_path[MAX_PATH_LEN];
    GetCHARfromString(stg_path, sizeof(stg_path), ClickedMenuItem->Tag->ToString());
    if (guiEx_config::load_guiEx_conf(&load_stg, stg_path) == CONF_ERROR_FILE_OPEN) {
        if (MessageBox::Show(L"設定ファイルオープンに失敗しました。\n"
                           + L"このファイルを削除しますか?",
                           L"エラー", MessageBoxButtons::YesNo, MessageBoxIcon::Error)
                           == System::Windows::Forms::DialogResult::Yes)
            DeleteStgFile(ClickedMenuItem);
        return;
    }
    ConfToFrm(&load_stg);
    CheckTSSettingsDropDownItem(ClickedMenuItem);
    memcpy(cnf_stgSelected, &load_stg, sizeof(CONF_GUIEX));
}

System::Void frmConfig::RebuildStgFileDropDown(ToolStripDropDownItem^ TS, String^ dir) {
    array<String^>^ subDirs = Directory::GetDirectories(dir);
    for (int i = 0; i < subDirs->Length; i++) {
        ToolStripMenuItem^ DDItem = gcnew ToolStripMenuItem(L"[ " + subDirs[i]->Substring(dir->Length+1) + L" ]");
        DDItem->DropDownItemClicked += gcnew System::Windows::Forms::ToolStripItemClickedEventHandler(this, &frmConfig::fcgTSSettings_DropDownItemClicked);
        DDItem->ForeColor = Color::Blue;
        DDItem->Tag = nullptr;
        RebuildStgFileDropDown(DDItem, subDirs[i]);
        TS->DropDownItems->Add(DDItem);
    }
    array<String^>^ stgList = Directory::GetFiles(dir, L"*.stg");
    for (int i = 0; i < stgList->Length; i++) {
        ToolStripMenuItem^ mItem = gcnew ToolStripMenuItem(Path::GetFileNameWithoutExtension(stgList[i]));
        mItem->Tag = stgList[i];
        TS->DropDownItems->Add(mItem);
    }
}

System::Void frmConfig::RebuildStgFileDropDown(String^ stgDir) {
    fcgTSSettings->DropDownItems->Clear();
    if (stgDir != nullptr)
        CurrentStgDir = stgDir;
    if (!Directory::Exists(CurrentStgDir))
        Directory::CreateDirectory(CurrentStgDir);
    RebuildStgFileDropDown(fcgTSSettings, Path::GetFullPath(CurrentStgDir));
}

//////////////   初期化関連     ////////////////
System::Void frmConfig::InitData(CONF_GUIEX *set_config, const SYSTEM_DATA *system_data) {
    if (set_config->size_all != CONF_INITIALIZED) {
        //初期化されていなければ初期化する
        init_CONF_GUIEX(set_config, FALSE);
    }
    conf = set_config;
    sys_dat = system_data;
}

System::Void frmConfig::InitComboBox() {
    //コンボボックスに値を設定する
    setComboBox(fcgCXCodec,         list_codec);
    setComboBox(fcgCXEncMode,       list_vce_rc_method_auo);
    setComboBox(fcgCXQualityPreset, list_vce_quality_preset_h264);
    setComboBox(fcgCXCodecLevel,    list_avc_level);
    setComboBox(fcgCXCodecProfile,  list_avc_profile);
    setComboBox(fcgCXHEVCLevel,     list_hevc_level);
    setComboBox(fcgCXHEVCProfile,   list_hevc_profile);
    setComboBox(fcgCXInterlaced,    list_interlaced);
    setComboBox(fcgCXBitdepth,      list_hevc_bitdepth);
    setComboBox(fcgCXAspectRatio,   list_aspect_ratio);
    setComboBox(fcgCXMotionEst,     list_mv_presicion);
    setComboBox(fcgCXColorMatrix,   list_colormatrix, "auto");
    setComboBox(fcgCXColorPrim,     list_colorprim, "auto");
    setComboBox(fcgCXTransfer,      list_transfer, "auto");
    setComboBox(fcgCXVideoFormat,   list_videoformat, "auto");
    setComboBox(fcgCXVppDenoiseMethod, list_vpp_denoise);
    setComboBox(fcgCXVppDetailEnhance, list_vpp_detail_enahance);

    setComboBox(fcgCXAudioTempDir,  list_audtempdir);
    setComboBox(fcgCXMP4BoxTempDir, list_mp4boxtempdir);
    setComboBox(fcgCXTempDir,       list_tempdir);

    setComboBox(fcgCXVppResizeAlg,   list_vpp_resize);
    setComboBox(fcgCXVppDeinterlace, list_vpp_deinterlacer);
    setComboBox(fcgCXVppDenoiseConv3DMatrix, list_vpp_convolution3d_matrix);
    setComboBox(fcgCXVppAfsAnalyze,     list_vpp_afs_analyze);
    setComboBox(fcgCXVppNnediNsize,     list_vpp_nnedi_nsize);
    setComboBox(fcgCXVppNnediNns,       list_vpp_nnedi_nns);
    setComboBox(fcgCXVppNnediQual,      list_vpp_nnedi_quality);
    setComboBox(fcgCXVppNnediPrec,      list_vpp_fp_prec);
    setComboBox(fcgCXVppNnediPrescreen, list_vpp_nnedi_pre_screen_gui);
    setComboBox(fcgCXVppNnediErrorType, list_vpp_nnedi_error_type);
    setComboBox(fcgCXVppYadifMode,      list_vpp_yadif_mode_gui);
    setComboBox(fcgCXVppDebandSample, list_vpp_deband);

    setComboBox(fcgCXAudioEncTiming, audio_enc_timing_desc);
    setComboBox(fcgCXAudioDelayCut,  AUDIO_DELAY_CUT_MODE);

    setMuxerCmdExNames(fcgCXMP4CmdEx, MUXER_MP4);
    setMuxerCmdExNames(fcgCXMKVCmdEx, MUXER_MKV);
    setMuxerCmdExNames(fcgCXInternalCmdEx, MUXER_INTERNAL);
#ifdef HIDE_MPEG2
    fcgCXMPGCmdEx->Items->Clear();
    fcgCXMPGCmdEx->Items->Add("");
#else
    setMuxerCmdExNames(fcgCXMPGCmdEx, MUXER_MPG);
#endif

    setAudioEncoderNames();

    setPriorityList(fcgCXMuxPriority);
    setPriorityList(fcgCXAudioPriority);
}

System::Void frmConfig::SetTXMaxLen(TextBox^ TX, int max_len) {
    TX->MaxLength = max_len;
    TX->Validating += gcnew System::ComponentModel::CancelEventHandler(this, &frmConfig::TX_LimitbyBytes);
}

System::Void frmConfig::SetTXMaxLenAll() {
    //MaxLengthに最大文字数をセットし、それをもとにバイト数計算を行うイベントをセットする。
    SetTXMaxLen(fcgTXVideoEncoderPath,   sizeof(sys_dat->exstg->s_vid.fullpath) - 1);
    SetTXMaxLen(fcgTXCmdEx,              sizeof(CONF_VCE::cmdex) - 1);
    SetTXMaxLen(fcgTXAudioEncoderPath,   sizeof(sys_dat->exstg->s_aud_ext[0].fullpath) - 1);
    SetTXMaxLen(fcgTXMP4MuxerPath,       sizeof(sys_dat->exstg->s_mux[MUXER_MP4].fullpath) - 1);
    SetTXMaxLen(fcgTXMKVMuxerPath,       sizeof(sys_dat->exstg->s_mux[MUXER_MKV].fullpath) - 1);
    SetTXMaxLen(fcgTXTC2MP4Path,         sizeof(sys_dat->exstg->s_mux[MUXER_TC2MP4].fullpath) - 1);
    SetTXMaxLen(fcgTXMPGMuxerPath,       sizeof(sys_dat->exstg->s_mux[MUXER_MPG].fullpath) - 1);
    SetTXMaxLen(fcgTXMP4RawPath,         sizeof(sys_dat->exstg->s_mux[MUXER_MP4_RAW].fullpath) - 1);
    SetTXMaxLen(fcgTXCustomTempDir,      sizeof(sys_dat->exstg->s_local.custom_tmp_dir) - 1);
    SetTXMaxLen(fcgTXCustomAudioTempDir, sizeof(sys_dat->exstg->s_local.custom_audio_tmp_dir) - 1);
    SetTXMaxLen(fcgTXMP4BoxTempDir,      sizeof(sys_dat->exstg->s_local.custom_mp4box_tmp_dir) - 1);
    SetTXMaxLen(fcgTXBatBeforeAudioPath, sizeof(conf->oth.batfile.before_audio) - 1);
    SetTXMaxLen(fcgTXBatAfterAudioPath,  sizeof(conf->oth.batfile.after_audio) - 1);
    SetTXMaxLen(fcgTXBatBeforePath,      sizeof(conf->oth.batfile.before_process) - 1);
    SetTXMaxLen(fcgTXBatAfterPath,       sizeof(conf->oth.batfile.after_process) - 1);
    fcgTSTSettingsNotes->MaxLength     = sizeof(conf->oth.notes) - 1;
}

System::Void frmConfig::InitStgFileList() {
    RebuildStgFileDropDown(String(sys_dat->exstg->s_local.stg_dir).ToString());
    stgChanged = false;
    CheckTSSettingsDropDownItem(nullptr);
}

System::Void frmConfig::fcgChangeEnabled(System::Object^  sender, System::EventArgs^  e) {
    int vce_rc_method = list_vce_h264_rc_method[fcgCXEncMode->SelectedIndex].value;
    bool cqp_mode = (vce_rc_method == 0);
    bool cbr_vbr_mode = (vce_rc_method == 1 || vce_rc_method == 2);

    this->SuspendLayout();

    fcgPNQP->Visible = cqp_mode;
    fcgNUQPI->Enabled = cqp_mode;
    fcgNUQPP->Enabled = cqp_mode;
    fcgNUQPB->Enabled = cqp_mode;
    fcgLBQPI->Enabled = cqp_mode;
    fcgLBQPP->Enabled = cqp_mode;
    fcgLBQPB->Enabled = cqp_mode;

    fcgPNBitrate->Visible = !cqp_mode;
    fcgNUBitrate->Enabled = !cqp_mode;
    fcgLBBitrate->Enabled = !cqp_mode;
    fcgNUMaxkbps->Enabled = cbr_vbr_mode;
    fcgLBMaxkbps->Enabled = cbr_vbr_mode;
    fcgLBMaxBitrate2->Enabled = cbr_vbr_mode;
    fcgNUVBVBufSize->Visible = cbr_vbr_mode;
    fcgNUVBVBufSize->Enabled = cbr_vbr_mode;
    fcgLBVBVBufSize->Visible = cbr_vbr_mode;
    fcgLBVBVBufSizeKbps->Visible = cbr_vbr_mode;

    fcggroupBoxResize->Enabled = fcgCBVppResize->Checked;
    fcgPNVppDenoiseKnn->Visible = (fcgCXVppDenoiseMethod->SelectedIndex == get_cx_index(list_vpp_denoise, _T("knn")));
    fcgPNVppDenoisePmd->Visible = (fcgCXVppDenoiseMethod->SelectedIndex == get_cx_index(list_vpp_denoise, _T("pmd")));
    fcgPNVppDenoiseSmooth->Visible = (fcgCXVppDenoiseMethod->SelectedIndex == get_cx_index(list_vpp_denoise, _T("smooth")));
    fcgPNVppDenoiseConv3D->Visible = (fcgCXVppDenoiseMethod->SelectedIndex == get_cx_index(list_vpp_denoise, _T("convolution3d")));
    fcgPNVppUnsharp->Visible = (fcgCXVppDetailEnhance->SelectedIndex == get_cx_index(list_vpp_detail_enahance, _T("unsharp")));
    fcgPNVppEdgelevel->Visible = (fcgCXVppDetailEnhance->SelectedIndex == get_cx_index(list_vpp_detail_enahance, _T("edgelevel")));
    fcgPNVppWarpsharp->Visible = (fcgCXVppDetailEnhance->SelectedIndex == get_cx_index(list_vpp_detail_enahance, _T("warpsharp")));
    fcgPNVppAfs->Visible = (fcgCXVppDeinterlace->SelectedIndex == get_cx_index(list_vpp_deinterlacer, L"自動フィールドシフト"));
    fcgPNVppNnedi->Visible = (fcgCXVppDeinterlace->SelectedIndex == get_cx_index(list_vpp_deinterlacer, L"nnedi"));
    fcgPNVppYadif->Visible = false; // (fcgCXVppDeinterlace->SelectedIndex == get_cx_index(list_vpp_deinterlacer, L"yadif"));
    fcggroupBoxVppDeband->Enabled = fcgCBVppDebandEnable->Checked;

    this->ResumeLayout();
    this->PerformLayout();
}
System::Void frmConfig::fcgCXCodec_SelectedIndexChanged(System::Object^  sender, System::EventArgs^  e) {
    const int nCodecId = list_codec[fcgCXCodec->SelectedIndex].value;

    this->SuspendLayout();

    fcgPNHEVCLevelProfile->Visible = nCodecId == RGY_CODEC_HEVC;
    bool bBframes = nCodecId != RGY_CODEC_HEVC;
    fcgPNBframes->Visible = bBframes;
    fcgLBQPB->Visible = bBframes;
    fcgNUQPB->Visible = bBframes;

    const int last_bitdepth = fcgCXBitdepth->SelectedIndex;
    fcgCXBitdepth->Enabled = true;
    fcgCXBitdepth->Items->Clear();
    if (nCodecId == RGY_CODEC_HEVC) {
        setComboBox(fcgCXBitdepth, list_hevc_bitdepth);
        SetCXIndex(fcgCXBitdepth, last_bitdepth);
    } else if (nCodecId == RGY_CODEC_H264) {
        setComboBox(fcgCXBitdepth, list_hevc_bitdepth, 1);
        SetCXIndex(fcgCXBitdepth, 0);
        fcgCXBitdepth->Enabled = false;
    }

    this->ResumeLayout();
    this->PerformLayout();
}

System::Void frmConfig::fcgChangeMuxerVisible(System::Object^  sender, System::EventArgs^  e) {
    //tc2mp4のチェック
    const bool enable_tc2mp4_muxer = (0 != str_has_char(sys_dat->exstg->s_mux[MUXER_TC2MP4].base_cmd));
    fcgTXTC2MP4Path->Visible = enable_tc2mp4_muxer;
    fcgLBTC2MP4Path->Visible = enable_tc2mp4_muxer;
    fcgBTTC2MP4Path->Visible = enable_tc2mp4_muxer;
    //mp4 rawのチェック
    const bool enable_mp4raw_muxer = (0 != str_has_char(sys_dat->exstg->s_mux[MUXER_MP4_RAW].base_cmd));
    fcgTXMP4RawPath->Visible = enable_mp4raw_muxer;
    fcgLBMP4RawPath->Visible = enable_mp4raw_muxer;
    fcgBTMP4RawPath->Visible = enable_mp4raw_muxer;
    //一時フォルダのチェック
    const bool enable_mp4_tmp = (0 != str_has_char(sys_dat->exstg->s_mux[MUXER_MP4].tmp_cmd));
    fcgCXMP4BoxTempDir->Visible = enable_mp4_tmp;
    fcgLBMP4BoxTempDir->Visible = enable_mp4_tmp;
    fcgTXMP4BoxTempDir->Visible = enable_mp4_tmp;
    fcgBTMP4BoxTempDir->Visible = enable_mp4_tmp;
    //Apple Chapterのチェック
    bool enable_mp4_apple_cmdex = false;
    for (int i = 0; i < sys_dat->exstg->s_mux[MUXER_MP4].ex_count; i++)
        enable_mp4_apple_cmdex |= (0 != str_has_char(sys_dat->exstg->s_mux[MUXER_MP4].ex_cmd[i].cmd_apple));
    fcgCBMP4MuxApple->Visible = enable_mp4_apple_cmdex;

    //位置の調整
    static const int HEIGHT = 31;
    fcgLBTC2MP4Path->Location = Point(fcgLBTC2MP4Path->Location.X, fcgLBMP4MuxerPath->Location.Y + HEIGHT * enable_tc2mp4_muxer);
    fcgTXTC2MP4Path->Location = Point(fcgTXTC2MP4Path->Location.X, fcgTXMP4MuxerPath->Location.Y + HEIGHT * enable_tc2mp4_muxer);
    fcgBTTC2MP4Path->Location = Point(fcgBTTC2MP4Path->Location.X, fcgBTMP4MuxerPath->Location.Y + HEIGHT * enable_tc2mp4_muxer);
    fcgLBMP4RawPath->Location = Point(fcgLBMP4RawPath->Location.X, fcgLBTC2MP4Path->Location.Y   + HEIGHT * enable_mp4raw_muxer);
    fcgTXMP4RawPath->Location = Point(fcgTXMP4RawPath->Location.X, fcgTXTC2MP4Path->Location.Y   + HEIGHT * enable_mp4raw_muxer);
    fcgBTMP4RawPath->Location = Point(fcgBTMP4RawPath->Location.X, fcgBTTC2MP4Path->Location.Y   + HEIGHT * enable_mp4raw_muxer);
}

System::Void frmConfig::SetStgEscKey(bool Enable) {
    if (this->KeyPreview == Enable)
        return;
    this->KeyPreview = Enable;
    if (Enable)
        this->KeyDown += gcnew System::Windows::Forms::KeyEventHandler(this, &frmConfig::frmConfig_KeyDown);
    else
        this->KeyDown -= gcnew System::Windows::Forms::KeyEventHandler(this, &frmConfig::frmConfig_KeyDown);
}

System::Void frmConfig::AdjustLocation() {
    //デスクトップ領域(タスクバー等除く)
    System::Drawing::Rectangle screen = System::Windows::Forms::Screen::GetWorkingArea(this);
    //現在のデスクトップ領域の座標
    Point CurrentDesktopLocation = this->DesktopLocation::get();
    //チェック開始
    bool ChangeLocation = false;
    if (CurrentDesktopLocation.X + this->Size.Width > screen.Width) {
        ChangeLocation = true;
        CurrentDesktopLocation.X = clamp(screen.X - this->Size.Width, 4, CurrentDesktopLocation.X);
    }
    if (CurrentDesktopLocation.Y + this->Size.Height > screen.Height) {
        ChangeLocation = true;
        CurrentDesktopLocation.Y = clamp(screen.Y - this->Size.Height, 4, CurrentDesktopLocation.Y);
    }
    if (ChangeLocation) {
        this->StartPosition = FormStartPosition::Manual;
        this->DesktopLocation::set(CurrentDesktopLocation);
    }
}

System::Void frmConfig::InitForm() {
    //UIテーマ切り替え
    CheckTheme();
    fcgpictureBoxVCEEnabled->Visible = false;
    GetVidEncInfoAsync();
    //ローカル設定のロード
    LoadLocalStg();
    //ローカル設定の反映
    SetLocalStg();
    //設定ファイル集の初期化
    InitStgFileList();
    //コンボボックスの値を設定
    InitComboBox();
    //タイトル表示
    this->Text = String(AUO_FULL_NAME).ToString();
    //バージョン情報,コンパイル日時
    fcgLBVersion->Text     = String(AUO_VERSION_NAME).ToString();
    fcgLBVersionDate->Text = L"build " + String(__DATE__).ToString() + L" " + String(__TIME__).ToString();
    //ツールチップ
    SetHelpToolTips();
    //パラメータセット
    ConfToFrm(conf);
    //イベントセット
    SetTXMaxLenAll(); //テキストボックスの最大文字数
    SetAllCheckChangedEvents(this); //変更の確認,ついでにNUのEnterEvent
    //フォームの変更可不可を更新
    fcgChangeMuxerVisible(nullptr, nullptr);
    fcgChangeEnabled(nullptr, nullptr);
    fcgCBAudioUseExt_CheckedChanged(nullptr, nullptr);
    fcgTXVideoEncoderPath_Leave(nullptr, nullptr);
    fcgTXAudioEncoderPath_Leave(nullptr, nullptr);
    fcgTXMP4MuxerPath_Leave(nullptr, nullptr);
    fcgTXTC2MP4Path_Leave(nullptr, nullptr);
    fcgTXMP4RawPath_Leave(nullptr, nullptr);
    fcgTXMKVMuxerPath_Leave(nullptr, nullptr);
    fcgTXMPGMuxerPath_Leave(nullptr, nullptr);
    EnableSettingsNoteChange(false);
#ifdef HIDE_MPEG2
    tabPageMpgMux = fcgtabControlMux->TabPages[2];
    fcgtabControlMux->TabPages->RemoveAt(2);
#endif
    //表示位置の調整
    AdjustLocation();
    //キー設定
    SetStgEscKey(sys_dat->exstg->s_local.enable_stg_esc_key != 0);
    //フォントの設定
    if (str_has_char(sys_dat->exstg->s_local.conf_font.name))
        SetFontFamilyToForm(this, gcnew FontFamily(String(sys_dat->exstg->s_local.conf_font.name).ToString()), this->Font->FontFamily);
}

/////////////         データ <-> GUI     /////////////
System::Void frmConfig::ConfToFrm(CONF_GUIEX *cnf) {
    this->SuspendLayout();

    VCEParam vce;
    parse_cmd(&vce, cnf->vce.cmd);

    SetCXIndex(fcgCXCodec,             get_cx_index(list_codec, vce.codec));
    SetCXIndex(fcgCXEncMode,           get_cx_index(get_rc_method(vce.codec), vce.rateControl));
    SetCXIndex(fcgCXQualityPreset,     get_cx_index(get_quality_preset(vce.codec), vce.qualityPreset));
    SetNUValue(fcgNUBitrate,           vce.nBitrate);
    SetNUValue(fcgNUMaxkbps,           vce.nMaxBitrate);
    SetNUValue(fcgNUVBVBufSize,        vce.nVBVBufferSize);
    SetNUValue(fcgNUQPI,               vce.nQPI);
    SetNUValue(fcgNUQPP,               vce.nQPP);
    SetNUValue(fcgNUQPB,               vce.nQPB);
    SetNUValue(fcgNUGopLength,         vce.nGOPLen);
    SetNUValue(fcgNUBframes,           vce.nBframes);
    fcgCBBPyramid->Checked           = vce.bBPyramid != 0;
    SetCXIndex(fcgCXCodecLevel,        get_cx_index(list_avc_level, vce.codecParam[RGY_CODEC_H264].nLevel));
    SetCXIndex(fcgCXCodecProfile,      get_cx_index(list_avc_profile, vce.codecParam[RGY_CODEC_H264].nProfile));
    SetCXIndex(fcgCXHEVCLevel,         get_cx_index(list_hevc_level, vce.codecParam[RGY_CODEC_HEVC].nLevel));
    SetCXIndex(fcgCXHEVCProfile,       get_cx_index(list_hevc_profile, vce.codecParam[RGY_CODEC_HEVC].nProfile));
    SetCXIndex(fcgCXInterlaced,        get_cx_index(list_interlaced, vce.input.picstruct));
    if (vce.par[0] * vce.par[1] <= 0)
        vce.par[0] = vce.par[1] = 0;
    SetCXIndex(fcgCXBitdepth,          get_cx_index(list_hevc_bitdepth, vce.outputDepth));
    SetCXIndex(fcgCXAspectRatio, (vce.par[0] < 0));
    SetNUValue(fcgNUAspectRatioX, abs(vce.par[0]));
    SetNUValue(fcgNUAspectRatioY, abs(vce.par[1]));

    SetNUValue(fcgNUQPMax,               vce.nQPMax);
    SetNUValue(fcgNUQPMin,               vce.nQPMin);
    SetNUValue(fcgNUBDeltaQP,            vce.nDeltaQPBFrame);
    SetNUValue(fcgNUBRefDeltaQP,         vce.nDeltaQPBFrameRef);

    SetNUValue(fcgNUSlices,             vce.nSlices);
    SetNUValue(fcgNURefFrames,          vce.nRefFrames);

    fcgCBDeblock->Checked             = vce.bDeblockFilter;
    fcgCBSkipFrame->Checked           = vce.bEnableSkipFrame;
    fcgCBTimerPeriodTuning->Checked   = vce.bTimerPeriodTuning;
    fcgCBVBAQ->Checked                = vce.bVBAQ;
    fcgCBFullrange->Checked           = vce.common.out_vui.colorrange == RGY_COLORRANGE_FULL;
    SetCXIndex(fcgCXColorMatrix,        get_cx_index(list_colormatrix, vce.common.out_vui.matrix));
    SetCXIndex(fcgCXTransfer,           get_cx_index(list_transfer, vce.common.out_vui.transfer));
    SetCXIndex(fcgCXColorPrim,          get_cx_index(list_colorprim, vce.common.out_vui.colorprim));
    SetCXIndex(fcgCXVideoFormat,        get_cx_index(list_videoformat, vce.common.out_vui.format));
    fcgCBPreAnalysis->Checked         = vce.pa.enable;

    SetCXIndex(fcgCXMotionEst,          get_cx_index(list_mv_presicion, vce.nMotionEst));

    SetCXIndex(fcgCXVppResizeAlg, get_cx_index(list_vpp_resize, vce.vpp.resize_algo));


        int denoise_idx = 0;
        if (vce.vpp.knn.enable) {
            denoise_idx = get_cx_index(list_vpp_denoise, _T("knn"));
        } else if (vce.vpp.pmd.enable) {
            denoise_idx = get_cx_index(list_vpp_denoise, _T("pmd"));
        } else if (vce.vpp.smooth.enable) {
            denoise_idx = get_cx_index(list_vpp_denoise, _T("smooth"));
        } else if (vce.vpp.convolution3d.enable) {
            denoise_idx = get_cx_index(list_vpp_denoise, _T("convolution3d"));
        }
        SetCXIndex(fcgCXVppDenoiseMethod, denoise_idx);

        int detail_enahance_idx = 0;
        if (vce.vpp.unsharp.enable) {
            detail_enahance_idx = get_cx_index(list_vpp_detail_enahance, _T("unsharp"));
        } else if (vce.vpp.edgelevel.enable) {
            detail_enahance_idx = get_cx_index(list_vpp_detail_enahance, _T("edgelevel"));
        } else if (vce.vpp.warpsharp.enable) {
            detail_enahance_idx = get_cx_index(list_vpp_detail_enahance, _T("warpsharp"));
        }
        SetCXIndex(fcgCXVppDetailEnhance, detail_enahance_idx);

        int deinterlacer_idx = 0;
        if (vce.vpp.afs.enable) {
            deinterlacer_idx = get_cx_index(list_vpp_deinterlacer, L"自動フィールドシフト");
        } else if (vce.vpp.nnedi.enable) {
            deinterlacer_idx = get_cx_index(list_vpp_deinterlacer, L"nnedi");
        }// else if (vce.vpp.yadif.enable) {
         //   deinterlacer_idx = get_cx_index(list_vpp_deinterlacer, L"yadif");
        //}
        SetCXIndex(fcgCXVppDeinterlace,          deinterlacer_idx);

        SetNUValue(fcgNUVppDenoiseKnnRadius,     vce.vpp.knn.radius);
        SetNUValue(fcgNUVppDenoiseKnnStrength,   vce.vpp.knn.strength);
        SetNUValue(fcgNUVppDenoiseKnnThreshold,  vce.vpp.knn.lerp_threshold);
        SetNUValue(fcgNUVppDenoisePmdApplyCount, vce.vpp.pmd.applyCount);
        SetNUValue(fcgNUVppDenoisePmdStrength,   vce.vpp.pmd.strength);
        SetNUValue(fcgNUVppDenoisePmdThreshold,  vce.vpp.pmd.threshold);
        SetNUValue(fcgNUVppDenoiseSmoothQuality, vce.vpp.smooth.quality);
        SetNUValue(fcgNUVppDenoiseSmoothQP,      vce.vpp.smooth.qp);
        SetCXIndex(fcgCXVppDenoiseConv3DMatrix, get_cx_index(list_vpp_convolution3d_matrix, (int)vce.vpp.convolution3d.matrix));
        SetNUValue(fcgNUVppDenoiseConv3DThreshYSpatial, vce.vpp.convolution3d.threshYspatial);
        SetNUValue(fcgNUVppDenoiseConv3DThreshCSpatial, vce.vpp.convolution3d.threshCspatial);
        SetNUValue(fcgNUVppDenoiseConv3DThreshYTemporal, vce.vpp.convolution3d.threshYtemporal);
        SetNUValue(fcgNUVppDenoiseConv3DThreshCTemporal, vce.vpp.convolution3d.threshCtemporal);
        fcgCBVppDebandEnable->Checked          = vce.vpp.deband.enable;
        SetNUValue(fcgNUVppDebandRange,          vce.vpp.deband.range);
        SetNUValue(fcgNUVppDebandThreY,          vce.vpp.deband.threY);
        SetNUValue(fcgNUVppDebandThreCb,         vce.vpp.deband.threCb);
        SetNUValue(fcgNUVppDebandThreCr,         vce.vpp.deband.threCr);
        SetNUValue(fcgNUVppDebandDitherY,        vce.vpp.deband.ditherY);
        SetNUValue(fcgNUVppDebandDitherC,        vce.vpp.deband.ditherC);
        SetCXIndex(fcgCXVppDebandSample,         vce.vpp.deband.sample);
        fcgCBVppDebandBlurFirst->Checked       = vce.vpp.deband.blurFirst;
        fcgCBVppDebandRandEachFrame->Checked   = vce.vpp.deband.randEachFrame;
        SetNUValue(fcgNUVppUnsharpRadius,        vce.vpp.unsharp.radius);
        SetNUValue(fcgNUVppUnsharpWeight,        vce.vpp.unsharp.weight);
        SetNUValue(fcgNUVppUnsharpThreshold,     vce.vpp.unsharp.threshold);
        SetNUValue(fcgNUVppEdgelevelStrength,    vce.vpp.edgelevel.strength);
        SetNUValue(fcgNUVppEdgelevelThreshold,   vce.vpp.edgelevel.threshold);
        SetNUValue(fcgNUVppEdgelevelBlack,       vce.vpp.edgelevel.black);
        SetNUValue(fcgNUVppEdgelevelWhite,       vce.vpp.edgelevel.white);
        SetNUValue(fcgNUVppWarpsharpBlur,        vce.vpp.warpsharp.blur);
        SetNUValue(fcgNUVppWarpsharpThreshold,   vce.vpp.warpsharp.threshold);
        SetNUValue(fcgNUVppWarpsharpType,        vce.vpp.warpsharp.type);
        SetNUValue(fcgNUVppWarpsharpDepth,       vce.vpp.warpsharp.depth);

        SetNUValue(fcgNUVppAfsUp,                vce.vpp.afs.clip.top);
        SetNUValue(fcgNUVppAfsBottom,            vce.vpp.afs.clip.bottom);
        SetNUValue(fcgNUVppAfsLeft,              vce.vpp.afs.clip.left);
        SetNUValue(fcgNUVppAfsRight,             vce.vpp.afs.clip.right);
        SetNUValue(fcgNUVppAfsMethodSwitch,      vce.vpp.afs.method_switch);
        SetNUValue(fcgNUVppAfsCoeffShift,        vce.vpp.afs.coeff_shift);
        SetNUValue(fcgNUVppAfsThreShift,         vce.vpp.afs.thre_shift);
        SetNUValue(fcgNUVppAfsThreDeint,         vce.vpp.afs.thre_deint);
        SetNUValue(fcgNUVppAfsThreYMotion,       vce.vpp.afs.thre_Ymotion);
        SetNUValue(fcgNUVppAfsThreCMotion,       vce.vpp.afs.thre_Cmotion);
        SetCXIndex(fcgCXVppAfsAnalyze,           vce.vpp.afs.analyze);
        fcgCBVppAfsShift->Checked              = vce.vpp.afs.shift != 0;
        fcgCBVppAfsDrop->Checked               = vce.vpp.afs.drop != 0;
        fcgCBVppAfsSmooth->Checked             = vce.vpp.afs.smooth != 0;
        fcgCBVppAfs24fps->Checked              = vce.vpp.afs.force24 != 0;
        fcgCBVppAfsTune->Checked               = vce.vpp.afs.tune != 0;
        SetCXIndex(fcgCXVppNnediNsize,           get_cx_index(list_vpp_nnedi_nsize, vce.vpp.nnedi.nsize));
        SetCXIndex(fcgCXVppNnediNns,             get_cx_index(list_vpp_nnedi_nns, vce.vpp.nnedi.nns));
        SetCXIndex(fcgCXVppNnediPrec,            get_cx_index(list_vpp_fp_prec, vce.vpp.nnedi.precision));
        SetCXIndex(fcgCXVppNnediPrescreen,       get_cx_index(list_vpp_nnedi_pre_screen_gui, vce.vpp.nnedi.pre_screen));
        SetCXIndex(fcgCXVppNnediQual,            get_cx_index(list_vpp_nnedi_quality, vce.vpp.nnedi.quality));
        SetCXIndex(fcgCXVppNnediErrorType,       get_cx_index(list_vpp_nnedi_error_type, vce.vpp.nnedi.errortype));
        //SetCXIndex(fcgCXVppYadifMode,            get_cx_index(list_vpp_yadif_mode_gui, vce.vpp.yadif.mode));

        fcgCBSSIM->Checked                     = vce.ssim;
        fcgCBPSNR->Checked                     = vce.psnr;

        //SetCXIndex(fcgCXX264Priority,        cnf->vid.priority);
        SetCXIndex(fcgCXTempDir,             cnf->oth.temp_dir);
        fcgCBAFS->Checked                  = cnf->vid.afs != 0;
        fcgCBAuoTcfileout->Checked         = cnf->vid.auo_tcfile_out != 0;
        fcgCBVppResize->Checked            = cnf->vid.resize_enable != 0;
        SetNUValue(fcgNUResizeW,             cnf->vid.resize_width);
        SetNUValue(fcgNUResizeH,             cnf->vid.resize_height);

        //音声
        fcgCBAudioUseExt->Checked          = cnf->aud.use_internal == 0;
        //外部音声エンコーダ
        fcgCBAudioOnly->Checked            = cnf->oth.out_audio_only != 0;
        fcgCBFAWCheck->Checked             = cnf->aud.ext.faw_check != 0;
        SetCXIndex(fcgCXAudioEncoder,        cnf->aud.ext.encoder);
        fcgCBAudio2pass->Checked           = cnf->aud.ext.use_2pass != 0;
        fcgCBAudioUsePipe->Checked = (CurrentPipeEnabled && !cnf->aud.ext.use_wav);
        SetCXIndex(fcgCXAudioDelayCut,       cnf->aud.ext.delay_cut);
        SetCXIndex(fcgCXAudioEncMode,        cnf->aud.ext.enc_mode);
        SetNUValue(fcgNUAudioBitrate,       (cnf->aud.ext.bitrate != 0) ? cnf->aud.ext.bitrate : GetCurrentAudioDefaultBitrate());
        SetCXIndex(fcgCXAudioPriority,       cnf->aud.ext.priority);
        SetCXIndex(fcgCXAudioTempDir,        cnf->aud.ext.aud_temp_dir);
        SetCXIndex(fcgCXAudioEncTiming,      cnf->aud.ext.audio_encode_timing);
        fcgCBRunBatBeforeAudio->Checked    =(cnf->oth.run_bat & RUN_BAT_BEFORE_AUDIO) != 0;
        fcgCBRunBatAfterAudio->Checked     =(cnf->oth.run_bat & RUN_BAT_AFTER_AUDIO) != 0;
        fcgTXBatBeforeAudioPath->Text      = String(cnf->oth.batfile.before_audio).ToString();
        fcgTXBatAfterAudioPath->Text       = String(cnf->oth.batfile.after_audio).ToString();
        //内蔵音声エンコーダ
        SetCXIndex(fcgCXAudioEncoderInternal, cnf->aud.in.encoder);
        SetCXIndex(fcgCXAudioEncModeInternal, cnf->aud.in.enc_mode);
        SetNUValue(fcgNUAudioBitrateInternal, (cnf->aud.in.bitrate != 0) ? cnf->aud.in.bitrate : GetCurrentAudioDefaultBitrate());

        //mux
        fcgCBMP4MuxerExt->Checked          = cnf->mux.disable_mp4ext == 0;
        fcgCBMP4MuxApple->Checked          = cnf->mux.apple_mode != 0;
        SetCXIndex(fcgCXMP4CmdEx,            cnf->mux.mp4_mode);
        SetCXIndex(fcgCXMP4BoxTempDir,       cnf->mux.mp4_temp_dir);
        fcgCBMKVMuxerExt->Checked          = cnf->mux.disable_mkvext == 0;
        SetCXIndex(fcgCXMKVCmdEx,            cnf->mux.mkv_mode);
        fcgCBMPGMuxerExt->Checked          = cnf->mux.disable_mpgext == 0;
        SetCXIndex(fcgCXMPGCmdEx,            cnf->mux.mpg_mode);
        fcgCBMuxMinimize->Checked          = cnf->mux.minimized != 0;
        SetCXIndex(fcgCXInternalCmdEx,       cnf->mux.internal_mode);
        SetCXIndex(fcgCXMuxPriority,         cnf->mux.priority);

        fcgCBRunBatBefore->Checked         =(cnf->oth.run_bat & RUN_BAT_BEFORE_PROCESS) != 0;
        fcgCBRunBatAfter->Checked          =(cnf->oth.run_bat & RUN_BAT_AFTER_PROCESS) != 0;
        fcgCBWaitForBatBefore->Checked     =(cnf->oth.dont_wait_bat_fin & RUN_BAT_BEFORE_PROCESS) == 0;
        fcgCBWaitForBatAfter->Checked      =(cnf->oth.dont_wait_bat_fin & RUN_BAT_AFTER_PROCESS) == 0;
        fcgTXBatBeforePath->Text           = String(cnf->oth.batfile.before_process).ToString();
        fcgTXBatAfterPath->Text            = String(cnf->oth.batfile.after_process).ToString();

        fcgTXCmdEx->Text = String(cnf->vce.cmdex).ToString();

        SetfcgTSLSettingsNotes(cnf->oth.notes);

    this->ResumeLayout();
    this->PerformLayout();
}

System::String^ frmConfig::FrmToConf(CONF_GUIEX *cnf) {
    //これもひたすら書くだけ。めんどい
    VCEParam vce;
    vce.codec                                   = (RGY_CODEC)list_codec[fcgCXCodec->SelectedIndex].value;
    conf->vce.codec = vce.codec;
    vce.rateControl                             = get_rc_method(vce.codec)[fcgCXEncMode->SelectedIndex].value;
    vce.qualityPreset                           = get_quality_preset(vce.codec)[fcgCXQualityPreset->SelectedIndex].value;
    vce.codecParam[RGY_CODEC_H264].nProfile     = list_avc_profile[fcgCXCodecProfile->SelectedIndex].value;
    vce.codecParam[RGY_CODEC_H264].nLevel       = list_avc_level[fcgCXCodecLevel->SelectedIndex].value;
    vce.codecParam[RGY_CODEC_HEVC].nProfile     = list_hevc_profile[fcgCXHEVCProfile->SelectedIndex].value;
    vce.codecParam[RGY_CODEC_HEVC].nLevel       = list_hevc_level[fcgCXHEVCLevel->SelectedIndex].value;
    vce.outputDepth                             = list_hevc_bitdepth[fcgCXBitdepth->SelectedIndex].value;
    vce.nBitrate                                = (int)fcgNUBitrate->Value;
    vce.nMaxBitrate                             = (int)fcgNUMaxkbps->Value;
    vce.nVBVBufferSize                          = (int)fcgNUVBVBufSize->Value;
    vce.nGOPLen                                 = (int)fcgNUGopLength->Value;
    vce.nQPI                                    = (int)fcgNUQPI->Value;
    vce.nQPP                                    = (int)fcgNUQPP->Value;
    vce.nQPB                                    = (int)fcgNUQPB->Value;
    vce.nQPMax                                  = (int)fcgNUQPMax->Value;
    vce.nQPMin                                  = (int)fcgNUQPMin->Value;

    vce.nBframes                                = (int)fcgNUBframes->Value;
    vce.bBPyramid                               = fcgCBBPyramid->Checked;
    vce.nDeltaQPBFrame                          = (int)fcgNUBDeltaQP->Value;
    vce.nDeltaQPBFrameRef                       = (int)fcgNUBRefDeltaQP->Value;

    vce.input.picstruct                         = (RGY_PICSTRUCT)list_interlaced[fcgCXInterlaced->SelectedIndex].value;
    vce.nSlices                                 = (int)fcgNUSlices->Value;
    vce.nRefFrames                              = (int)fcgNURefFrames->Value;

    vce.bDeblockFilter                          = fcgCBDeblock->Checked;
    vce.bEnableSkipFrame                        = fcgCBSkipFrame->Checked;
    vce.bVBAQ                                   = fcgCBVBAQ->Checked;
    vce.common.out_vui.colorrange               = fcgCBFullrange->Checked ? RGY_COLORRANGE_FULL : RGY_COLORRANGE_UNSPECIFIED;
    vce.common.out_vui.matrix                   = (CspMatrix)list_colormatrix[fcgCXColorMatrix->SelectedIndex].value;
    vce.common.out_vui.transfer                 = (CspTransfer)list_transfer[fcgCXTransfer->SelectedIndex].value;
    vce.common.out_vui.colorprim                = (CspColorprim)list_colorprim[fcgCXColorPrim->SelectedIndex].value;
    vce.common.out_vui.format                   = list_videoformat[fcgCXVideoFormat->SelectedIndex].value;
    vce.common.out_vui.descriptpresent          = 1;
    vce.pa.enable                               = fcgCBPreAnalysis->Checked;

    vce.nMotionEst                              = list_mv_presicion[fcgCXMotionEst->SelectedIndex].value;

    vce.bTimerPeriodTuning                      = fcgCBTimerPeriodTuning->Checked;

    vce.vpp.resize_algo                 = (RGY_VPP_RESIZE_ALGO)list_vpp_resize[fcgCXVppResizeAlg->SelectedIndex].value;

    vce.vpp.knn.enable = fcgCXVppDenoiseMethod->SelectedIndex == get_cx_index(list_vpp_denoise, _T("knn"));
    vce.vpp.knn.radius = (int)fcgNUVppDenoiseKnnRadius->Value;
    vce.vpp.knn.strength = (float)fcgNUVppDenoiseKnnStrength->Value;
    vce.vpp.knn.lerp_threshold = (float)fcgNUVppDenoiseKnnThreshold->Value;

    vce.vpp.pmd.enable = fcgCXVppDenoiseMethod->SelectedIndex == get_cx_index(list_vpp_denoise, _T("pmd"));
    vce.vpp.pmd.applyCount = (int)fcgNUVppDenoisePmdApplyCount->Value;
    vce.vpp.pmd.strength = (float)fcgNUVppDenoisePmdStrength->Value;
    vce.vpp.pmd.threshold = (float)fcgNUVppDenoisePmdThreshold->Value;

    vce.vpp.smooth.enable = fcgCXVppDenoiseMethod->SelectedIndex == get_cx_index(list_vpp_denoise, _T("smooth"));
    vce.vpp.smooth.quality = (int)fcgNUVppDenoiseSmoothQuality->Value;
    vce.vpp.smooth.qp = (int)fcgNUVppDenoiseSmoothQP->Value;

    vce.vpp.convolution3d.enable = fcgCXVppDenoiseMethod->SelectedIndex == get_cx_index(list_vpp_denoise, _T("convolution3d"));
    vce.vpp.convolution3d.matrix = (VppConvolution3dMatrix)list_vpp_convolution3d_matrix[fcgCXVppDenoiseConv3DMatrix->SelectedIndex].value;;
    vce.vpp.convolution3d.threshYspatial = (int)fcgNUVppDenoiseConv3DThreshYSpatial->Value;
    vce.vpp.convolution3d.threshCspatial = (int)fcgNUVppDenoiseConv3DThreshCSpatial->Value;
    vce.vpp.convolution3d.threshYtemporal = (int)fcgNUVppDenoiseConv3DThreshYTemporal->Value;
    vce.vpp.convolution3d.threshCtemporal = (int)fcgNUVppDenoiseConv3DThreshCTemporal->Value;

    vce.vpp.unsharp.enable = fcgCXVppDetailEnhance->SelectedIndex == get_cx_index(list_vpp_detail_enahance, _T("unsharp"));
    vce.vpp.unsharp.radius = (int)fcgNUVppUnsharpRadius->Value;
    vce.vpp.unsharp.weight = (float)fcgNUVppUnsharpWeight->Value;
    vce.vpp.unsharp.threshold = (float)fcgNUVppUnsharpThreshold->Value;

    vce.vpp.edgelevel.enable = fcgCXVppDetailEnhance->SelectedIndex == get_cx_index(list_vpp_detail_enahance, _T("edgelevel"));
    vce.vpp.edgelevel.strength = (float)fcgNUVppEdgelevelStrength->Value;
    vce.vpp.edgelevel.threshold = (float)fcgNUVppEdgelevelThreshold->Value;
    vce.vpp.edgelevel.black = (float)fcgNUVppEdgelevelBlack->Value;
    vce.vpp.edgelevel.white = (float)fcgNUVppEdgelevelWhite->Value;

    vce.vpp.warpsharp.enable = fcgCXVppDetailEnhance->SelectedIndex == get_cx_index(list_vpp_detail_enahance, _T("warpsharp"));
    vce.vpp.warpsharp.blur = (int)fcgNUVppWarpsharpBlur->Value;
    vce.vpp.warpsharp.threshold = (float)fcgNUVppWarpsharpThreshold->Value;
    vce.vpp.warpsharp.type = (int)fcgNUVppWarpsharpType->Value;
    vce.vpp.warpsharp.depth = (float)fcgNUVppWarpsharpDepth->Value;

    vce.vpp.deband.enable = fcgCBVppDebandEnable->Checked;
    vce.vpp.deband.range = (int)fcgNUVppDebandRange->Value;
    vce.vpp.deband.threY = (int)fcgNUVppDebandThreY->Value;
    vce.vpp.deband.threCb = (int)fcgNUVppDebandThreCb->Value;
    vce.vpp.deband.threCr = (int)fcgNUVppDebandThreCr->Value;
    vce.vpp.deband.ditherY = (int)fcgNUVppDebandDitherY->Value;
    vce.vpp.deband.ditherC = (int)fcgNUVppDebandDitherC->Value;
    vce.vpp.deband.sample = fcgCXVppDebandSample->SelectedIndex;
    vce.vpp.deband.blurFirst = fcgCBVppDebandBlurFirst->Checked;
    vce.vpp.deband.randEachFrame = fcgCBVppDebandRandEachFrame->Checked;

    vce.vpp.afs.enable             = (fcgCXVppDeinterlace->SelectedIndex == get_cx_index(list_vpp_deinterlacer, L"自動フィールドシフト"));
    vce.vpp.afs.timecode           = false;
    vce.vpp.afs.clip.top           = (int)fcgNUVppAfsUp->Value;
    vce.vpp.afs.clip.bottom        = (int)fcgNUVppAfsBottom->Value;
    vce.vpp.afs.clip.left          = (int)fcgNUVppAfsLeft->Value;
    vce.vpp.afs.clip.right         = (int)fcgNUVppAfsRight->Value;
    vce.vpp.afs.method_switch      = (int)fcgNUVppAfsMethodSwitch->Value;
    vce.vpp.afs.coeff_shift        = (int)fcgNUVppAfsCoeffShift->Value;
    vce.vpp.afs.thre_shift         = (int)fcgNUVppAfsThreShift->Value;
    vce.vpp.afs.thre_deint         = (int)fcgNUVppAfsThreDeint->Value;
    vce.vpp.afs.thre_Ymotion       = (int)fcgNUVppAfsThreYMotion->Value;
    vce.vpp.afs.thre_Cmotion       = (int)fcgNUVppAfsThreCMotion->Value;
    vce.vpp.afs.analyze            = fcgCXVppAfsAnalyze->SelectedIndex;
    vce.vpp.afs.shift              = fcgCBVppAfsShift->Checked;
    vce.vpp.afs.drop               = fcgCBVppAfsDrop->Checked;
    vce.vpp.afs.smooth             = fcgCBVppAfsSmooth->Checked;
    vce.vpp.afs.force24            = fcgCBVppAfs24fps->Checked;
    vce.vpp.afs.tune               = fcgCBVppAfsTune->Checked;

    vce.vpp.nnedi.enable           = (fcgCXVppDeinterlace->SelectedIndex == get_cx_index(list_vpp_deinterlacer, L"nnedi"));
    vce.vpp.nnedi.nsize            = (VppNnediNSize)list_vpp_nnedi_nsize[fcgCXVppNnediNsize->SelectedIndex].value;
    vce.vpp.nnedi.nns              = list_vpp_nnedi_nns[fcgCXVppNnediNns->SelectedIndex].value;
    vce.vpp.nnedi.quality          = (VppNnediQuality)list_vpp_nnedi_quality[fcgCXVppNnediQual->SelectedIndex].value;
    vce.vpp.nnedi.precision        = (VppFpPrecision)list_vpp_fp_prec[fcgCXVppNnediPrec->SelectedIndex].value;
    vce.vpp.nnedi.pre_screen       = (VppNnediPreScreen)list_vpp_nnedi_pre_screen_gui[fcgCXVppNnediPrescreen->SelectedIndex].value;
    vce.vpp.nnedi.errortype        = (VppNnediErrorType)list_vpp_nnedi_error_type[fcgCXVppNnediErrorType->SelectedIndex].value;

    //vce.vpp.yadif.enable = (fcgCXVppDeinterlace->SelectedIndex == get_cx_index(list_vpp_deinterlacer, L"yadif"));
    //vce.vpp.yadif.mode = (VppYadifMode)list_vpp_yadif_mode_gui[fcgCXVppYadifMode->SelectedIndex].value;

    vce.ssim                       = fcgCBSSIM->Checked;
    vce.psnr                       = fcgCBPSNR->Checked;

    cnf->vid.afs                    = fcgCBAFS->Checked;
    cnf->vid.auo_tcfile_out         = fcgCBAuoTcfileout->Checked;
    vce.par[0]                      = (int)fcgNUAspectRatioX->Value;
    vce.par[1]                      = (int)fcgNUAspectRatioY->Value;
    if (fcgCXAspectRatio->SelectedIndex == 1) {
        vce.par[0] *= -1;
        vce.par[1] *= -1;
    }

    //拡張部
    cnf->oth.temp_dir               = fcgCXTempDir->SelectedIndex;
    cnf->vid.afs                    = fcgCBAFS->Checked;
    cnf->vid.auo_tcfile_out         = fcgCBAuoTcfileout->Checked;
    cnf->vid.resize_enable          = fcgCBVppResize->Checked;
    cnf->vid.resize_width           = (int)fcgNUResizeW->Value;
    cnf->vid.resize_height          = (int)fcgNUResizeH->Value;

    //音声部
    cnf->aud.use_internal               = !fcgCBAudioUseExt->Checked;
    cnf->aud.ext.encoder                = fcgCXAudioEncoder->SelectedIndex;
    cnf->oth.out_audio_only         = fcgCBAudioOnly->Checked;
    cnf->aud.ext.faw_check              = fcgCBFAWCheck->Checked;
    cnf->aud.ext.enc_mode               = fcgCXAudioEncMode->SelectedIndex;
    cnf->aud.ext.bitrate                = (int)fcgNUAudioBitrate->Value;
    cnf->aud.ext.use_2pass              = fcgCBAudio2pass->Checked;
    cnf->aud.ext.use_wav                = !fcgCBAudioUsePipe->Checked;
    cnf->aud.ext.delay_cut              = fcgCXAudioDelayCut->SelectedIndex;
    cnf->aud.ext.priority               = fcgCXAudioPriority->SelectedIndex;
    cnf->aud.ext.audio_encode_timing    = fcgCXAudioEncTiming->SelectedIndex;
    cnf->aud.ext.aud_temp_dir           = fcgCXAudioTempDir->SelectedIndex;
    cnf->aud.in.encoder                 = fcgCXAudioEncoderInternal->SelectedIndex;
    cnf->aud.in.faw_check               = fcgCBFAWCheck->Checked;
    cnf->aud.in.enc_mode                = fcgCXAudioEncModeInternal->SelectedIndex;
    cnf->aud.in.bitrate                 = (int)fcgNUAudioBitrateInternal->Value;

    //mux部
    cnf->mux.use_internal           = !fcgCBAudioUseExt->Checked;
    cnf->mux.disable_mp4ext         = !fcgCBMP4MuxerExt->Checked;
    cnf->mux.apple_mode             = fcgCBMP4MuxApple->Checked;
    cnf->mux.mp4_mode               = fcgCXMP4CmdEx->SelectedIndex;
    cnf->mux.mp4_temp_dir           = fcgCXMP4BoxTempDir->SelectedIndex;
    cnf->mux.disable_mkvext         = !fcgCBMKVMuxerExt->Checked;
    cnf->mux.mkv_mode               = fcgCXMKVCmdEx->SelectedIndex;
    cnf->mux.disable_mpgext         = !fcgCBMPGMuxerExt->Checked;
    cnf->mux.mpg_mode               = fcgCXMPGCmdEx->SelectedIndex;
    cnf->mux.minimized              = fcgCBMuxMinimize->Checked;
    cnf->mux.priority               = fcgCXMuxPriority->SelectedIndex;
    cnf->mux.internal_mode          = fcgCXInternalCmdEx->SelectedIndex;

    cnf->oth.run_bat                = RUN_BAT_NONE;
    cnf->oth.run_bat               |= (fcgCBRunBatBeforeAudio->Checked) ? RUN_BAT_BEFORE_AUDIO   : NULL;
    cnf->oth.run_bat               |= (fcgCBRunBatAfterAudio->Checked)  ? RUN_BAT_AFTER_AUDIO    : NULL;
    cnf->oth.run_bat               |= (fcgCBRunBatBefore->Checked)      ? RUN_BAT_BEFORE_PROCESS : NULL;
    cnf->oth.run_bat               |= (fcgCBRunBatAfter->Checked)       ? RUN_BAT_AFTER_PROCESS  : NULL;
    cnf->oth.dont_wait_bat_fin      = RUN_BAT_NONE;
    cnf->oth.dont_wait_bat_fin     |= (!fcgCBWaitForBatBefore->Checked) ? RUN_BAT_BEFORE_PROCESS : NULL;
    cnf->oth.dont_wait_bat_fin     |= (!fcgCBWaitForBatAfter->Checked)  ? RUN_BAT_AFTER_PROCESS  : NULL;
    GetCHARfromString(cnf->oth.batfile.before_process, fcgTXBatBeforePath->Text);
    GetCHARfromString(cnf->oth.batfile.after_process,  fcgTXBatAfterPath->Text);
    GetCHARfromString(cnf->oth.batfile.before_audio,   fcgTXBatBeforeAudioPath->Text);
    GetCHARfromString(cnf->oth.batfile.after_audio,    fcgTXBatAfterAudioPath->Text);

    GetfcgTSLSettingsNotes(cnf->oth.notes, sizeof(cnf->oth.notes));

    GetCHARfromString(cnf->vce.cmdex, sizeof(cnf->vce.cmdex), fcgTXCmdEx->Text);
    strcpy_s(cnf->vce.cmd, gen_cmd(&vce, true).c_str());

    return String(gen_cmd(&vce, false).c_str()).ToString();
}

System::Void frmConfig::GetfcgTSLSettingsNotes(char *notes, int nSize) {
    ZeroMemory(notes, nSize);
    if (fcgTSLSettingsNotes->ForeColor == Color::FromArgb(StgNotesColor[0][0], StgNotesColor[0][1], StgNotesColor[0][2]))
        GetCHARfromString(notes, nSize, fcgTSLSettingsNotes->Text);
}

System::Void frmConfig::SetfcgTSLSettingsNotes(const char *notes) {
    if (str_has_char(notes)) {
        fcgTSLSettingsNotes->ForeColor = Color::FromArgb(StgNotesColor[0][0], StgNotesColor[0][1], StgNotesColor[0][2]);
        fcgTSLSettingsNotes->Text = String(notes).ToString();
    } else {
        fcgTSLSettingsNotes->ForeColor = Color::FromArgb(StgNotesColor[1][0], StgNotesColor[1][1], StgNotesColor[1][2]);
        fcgTSLSettingsNotes->Text = String(DefaultStgNotes).ToString();
    }
}

System::Void frmConfig::SetfcgTSLSettingsNotes(String^ notes) {
    if (notes->Length && String::Compare(notes, String(DefaultStgNotes).ToString()) != 0) {
        fcgTSLSettingsNotes->ForeColor = Color::FromArgb(StgNotesColor[0][0], StgNotesColor[0][1], StgNotesColor[0][2]);
        fcgTSLSettingsNotes->Text = notes;
    } else {
        fcgTSLSettingsNotes->ForeColor = Color::FromArgb(StgNotesColor[1][0], StgNotesColor[1][1], StgNotesColor[1][2]);
        fcgTSLSettingsNotes->Text = String(DefaultStgNotes).ToString();
    }
}

System::Void frmConfig::SetChangedEvent(Control^ control, System::EventHandler^ _event) {
    System::Type^ ControlType = control->GetType();
    if (ControlType == NumericUpDown::typeid)
        ((NumericUpDown^)control)->ValueChanged += _event;
    else if (ControlType == ComboBox::typeid)
        ((ComboBox^)control)->SelectedIndexChanged += _event;
    else if (ControlType == CheckBox::typeid)
        ((CheckBox^)control)->CheckedChanged += _event;
    else if (ControlType == TextBox::typeid)
        ((TextBox^)control)->TextChanged += _event;
}

System::Void frmConfig::SetToolStripEvents(ToolStrip^ TS, System::Windows::Forms::MouseEventHandler^ _event) {
    for (int i = 0; i < TS->Items->Count; i++) {
        ToolStripButton^ TSB = dynamic_cast<ToolStripButton^>(TS->Items[i]);
        if (TSB != nullptr) TSB->MouseDown += _event;
    }
}

System::Void frmConfig::TabControl_DarkDrawItem(System::Object^ sender, DrawItemEventArgs^ e) {
    //対象のTabControlを取得
    TabControl^ tab = dynamic_cast<TabControl^>(sender);
    //タブページのテキストを取得
    System::String^ txt = tab->TabPages[e->Index]->Text;

    //タブのテキストと背景を描画するためのブラシを決定する
    SolidBrush^ foreBrush = gcnew System::Drawing::SolidBrush(ColorfromInt(DEFAULT_UI_COLOR_TEXT_DARK));
    SolidBrush^ backBrush = gcnew System::Drawing::SolidBrush(ColorfromInt(DEFAULT_UI_COLOR_BASE_DARK));

    //StringFormatを作成
    StringFormat^ sf = gcnew System::Drawing::StringFormat();
    //中央に表示する
    sf->Alignment = StringAlignment::Center;
    sf->LineAlignment = StringAlignment::Center;

    //背景の描画
    e->Graphics->FillRectangle(backBrush, e->Bounds);
    //Textの描画
    e->Graphics->DrawString(txt, e->Font, foreBrush, e->Bounds, sf);
}

System::Void frmConfig::fcgMouseEnter_SetColor(System::Object^  sender, System::EventArgs^  e) {
    fcgMouseEnterLeave_SetColor(sender, themeMode, DarkenWindowState::Hot, dwStgReader);
}
System::Void frmConfig::fcgMouseLeave_SetColor(System::Object^  sender, System::EventArgs^  e) {
    fcgMouseEnterLeave_SetColor(sender, themeMode, DarkenWindowState::Normal, dwStgReader);
}

System::Void frmConfig::SetAllMouseMove(Control ^top, const AuoTheme themeTo) {
    if (themeTo == themeMode) return;
    System::Type^ type = top->GetType();
    if (type == CheckBox::typeid /* || isToolStripItem(type)*/) {
        top->MouseEnter += gcnew System::EventHandler(this, &frmConfig::fcgMouseEnter_SetColor);
        top->MouseLeave += gcnew System::EventHandler(this, &frmConfig::fcgMouseLeave_SetColor);
    } else if (type == ToolStrip::typeid) {
        ToolStrip^ TS = dynamic_cast<ToolStrip^>(top);
        for (int i = 0; i < TS->Items->Count; i++) {
            auto item = TS->Items[i];
            item->MouseEnter += gcnew System::EventHandler(this, &frmConfig::fcgMouseEnter_SetColor);
            item->MouseLeave += gcnew System::EventHandler(this, &frmConfig::fcgMouseLeave_SetColor);
        }
    }
    for (int i = 0; i < top->Controls->Count; i++) {
        SetAllMouseMove(top->Controls[i], themeTo);
    }
}

System::Void frmConfig::CheckTheme() {
    //DarkenWindowが使用されていれば設定をロードする
    if (dwStgReader != nullptr) delete dwStgReader;
    const auto [themeTo, dwStg] = check_current_theme(sys_dat->aviutl_dir);
    dwStgReader = dwStg;

    //変更の必要がなければ終了
    if (themeTo == themeMode) return;

    //一度ウィンドウの再描画を完全に抑止する
    SendMessage(reinterpret_cast<HWND>(this->Handle.ToPointer()), WM_SETREDRAW, 0, 0);
#if 0
    //tabcontrolのborderを隠す
    SwitchComboBoxBorder(fcgtabControlVideo, fcgPNHideTabControlVideo, themeMode, themeTo, dwStgReader);
    SwitchComboBoxBorder(fcgtabControlAudio, fcgPNHideTabControlAudio, themeMode, themeTo, dwStgReader);
    SwitchComboBoxBorder(fcgtabControlMux,   fcgPNHideTabControlMux,   themeMode, themeTo, dwStgReader);
#endif
    //上部のtoolstripborderを隠すためのパネル
    fcgPNHideToolStripBorder->Visible = themeTo == AuoTheme::DarkenWindowDark;
#if 0
    //TabControlをオーナードローする
    fcgtabControlVideo->DrawMode = TabDrawMode::OwnerDrawFixed;
    fcgtabControlVideo->DrawItem += gcnew DrawItemEventHandler(this, &frmConfig::TabControl_DarkDrawItem);

    fcgtabControlAudio->DrawMode = TabDrawMode::OwnerDrawFixed;
    fcgtabControlAudio->DrawItem += gcnew DrawItemEventHandler(this, &frmConfig::TabControl_DarkDrawItem);

    fcgtabControlMux->DrawMode = TabDrawMode::OwnerDrawFixed;
    fcgtabControlMux->DrawItem += gcnew DrawItemEventHandler(this, &frmConfig::TabControl_DarkDrawItem);
#endif
    if (themeTo != themeMode) {
        SetAllColor(this, themeTo, this->GetType(), dwStgReader);
        SetAllMouseMove(this, themeTo);
    }
    //一度ウィンドウの再描画を再開し、強制的に再描画させる
    SendMessage(reinterpret_cast<HWND>(this->Handle.ToPointer()), WM_SETREDRAW, 1, 0);
    this->Refresh();
    themeMode = themeTo;
}

System::Void frmConfig::SetAllCheckChangedEvents(Control ^top) {
    //再帰を使用してすべてのコントロールのtagを調べ、イベントをセットする
    for (int i = 0; i < top->Controls->Count; i++) {
        System::Type^ type = top->Controls[i]->GetType();
        if (type == NumericUpDown::typeid)
            top->Controls[i]->Enter += gcnew System::EventHandler(this, &frmConfig::NUSelectAll);

        if (type == Label::typeid || type == Button::typeid)
            ;
        else if (type == ToolStrip::typeid)
            SetToolStripEvents((ToolStrip^)(top->Controls[i]), gcnew System::Windows::Forms::MouseEventHandler(this, &frmConfig::fcgTSItem_MouseDown));
        else if (top->Controls[i]->Tag == nullptr)
            SetAllCheckChangedEvents(top->Controls[i]);
        else if (String::Equals(top->Controls[i]->Tag->ToString(), L"reCmd"))
            SetChangedEvent(top->Controls[i], gcnew System::EventHandler(this, &frmConfig::fcgRebuildCmd));
        else if (String::Equals(top->Controls[i]->Tag->ToString(), L"chValue"))
            SetChangedEvent(top->Controls[i], gcnew System::EventHandler(this, &frmConfig::CheckOtherChanges));
        else
            SetAllCheckChangedEvents(top->Controls[i]);
    }
}

System::Void frmConfig::SetHelpToolTips() {

    //拡張
    fcgTTEx->SetToolTip(fcgCXTempDir,      L""
        + L"一時ファイル群\n"
        + L"・音声一時ファイル(wav / エンコード後音声)\n"
        + L"・動画一時ファイル\n"
        + L"・タイムコードファイル\n"
        + L"・qpファイル\n"
        + L"・mux後ファイル\n"
        + L"の作成場所を指定します。"
        );
    fcgTTEx->SetToolTip(fcgCBAFS,                L""
        + L"自動フィールドシフト(afs)を使用してVFR化を行います。\n"
        + L"エンコード時にタイムコードを作成し、mux時に埋め込んで\n"
        + L"フレームレートを変更します。\n"
        + L"\n"
        + L"あとからフレームレートを変更するため、\n"
        + L"ビットレート設定が正確に反映されなくなる点に注意してください。"
        );
    fcgTTEx->SetToolTip(fcgCBAuoTcfileout, L""
        + L"タイムコードを出力します。このタイムコードは\n"
        + L"自動フィールドシフト(afs)を反映したものになります。"
        );

    //音声
    fcgTTEx->SetToolTip(fcgCXAudioEncoder, L""
        + L"使用する音声エンコーダを指定します。\n"
        + L"これらの設定はVCEEnc.iniに記述されています。"
        );
    fcgTTEx->SetToolTip(fcgCBAudioOnly,    L""
        + L"動画の出力を行わず、音声エンコードのみ行います。\n"
        + L"音声エンコードに失敗した場合などに使用してください。"
        );
    fcgTTEx->SetToolTip(fcgCBFAWCheck,     L""
        + L"音声エンコード時に音声がFakeAACWav(FAW)かどうかの判定を行い、\n"
        + L"FAWだと判定された場合、設定を無視して、\n"
        + L"自動的にFAWを使用するよう切り替えます。\n"
        + L"\n"
        + L"一度音声エンコーダからFAW(fawcl)を選択し、\n"
        + L"実行ファイルの場所を指定しておく必要があります。"
        );
    fcgTTEx->SetToolTip(fcgBTAudioEncoderPath, L""
        + L"音声エンコーダの場所を指定します。\n"
        + L"\n"
        + L"この設定はx264guiEx.confに保存され、\n"
        + L"バッチ処理ごとの変更はできません。"
        );
    fcgTTEx->SetToolTip(fcgCXAudioEncMode, L""
        + L"音声エンコーダのエンコードモードを切り替えます。\n"
        + L"これらの設定はVCEEnc.iniに記述されています。"
        );
    fcgTTEx->SetToolTip(fcgCBAudio2pass,   L""
        + L"音声エンコードを2passで行います。\n"
        + L"2pass時はパイプ処理は行えません。"
        );
    fcgTTEx->SetToolTip(fcgCBAudioUsePipe, L""
        + L"パイプを通して、音声データをエンコーダに渡します。\n"
        + L"パイプと2passは同時に指定できません。"
        );
    fcgTTEx->SetToolTip(fcgNUAudioBitrate, L""
        + L"音声ビットレートを指定します。"
        );
    fcgTTEx->SetToolTip(fcgCXAudioPriority, L""
        + L"音声エンコーダのCPU優先度を設定します。\n"
        + L"AviutlSync で Aviutlの優先度と同じになります。"
        );
    fcgTTEx->SetToolTip(fcgCXAudioEncTiming, L""
        + L"音声を処理するタイミングを設定します。\n"
        + L" 後　 … 映像→音声の順で処理します。\n"
        + L" 前　 … 音声→映像の順で処理します。\n"
        + L" 同時 … 映像と音声を同時に処理します。"
        );
    fcgTTEx->SetToolTip(fcgCXAudioTempDir, L""
        + L"音声一時ファイル(エンコード後のファイル)\n"
        + L"の出力先を変更します。"
        );
    fcgTTEx->SetToolTip(fcgBTCustomAudioTempDir, L""
        + L"音声一時ファイルの場所を「カスタム」にした時に\n"
        + L"使用される音声一時ファイルの場所を指定します。\n"
        + L"\n"
        + L"この設定はx264guiEx.confに保存され、\n"
        + L"バッチ処理ごとの変更はできません。"
        );

    //muxer
    fcgTTEx->SetToolTip(fcgCBMP4MuxerExt, L""
        + L"指定したmuxerでmuxを行います。\n"
        + L"チェックを外すとmuxを行いません。"
        );
    fcgTTEx->SetToolTip(fcgCXMP4CmdEx,    L""
        + L"muxerに渡す追加オプションを選択します。\n"
        + L"これらの設定はVCEEnc.iniに記述されています。"
        );
    fcgTTEx->SetToolTip(fcgBTMP4MuxerPath, L""
        + L"mp4用muxerの場所を指定します。\n"
        + L"\n"
        + L"この設定はVCEEnc.confに保存され、\n"
        + L"バッチ処理ごとの変更はできません。"
        );
    fcgTTEx->SetToolTip(fcgBTMP4RawPath, L""
        + L"raw用mp4muxerの場所を指定します。\n"
        + L"\n"
        + L"この設定はVCEEnc.confに保存され、\n"
        + L"バッチ処理ごとの変更はできません。"
        );
    fcgTTEx->SetToolTip(fcgCXMP4BoxTempDir, L""
        + L"mp4box用の一時フォルダの場所を指定します。"
        );
    fcgTTEx->SetToolTip(fcgBTMP4BoxTempDir, L""
        + L"mp4box用一時フォルダの場所を「カスタム」に設定した際に\n"
        + L"使用される一時フォルダの場所です。\n"
        + L"\n"
        + L"この設定はVCEEnc.confに保存され、\n"
        + L"バッチ処理ごとの変更はできません。"
        );
    fcgTTEx->SetToolTip(fcgCBMKVMuxerExt, L""
        + L"指定したmuxerでmuxを行います。\n"
        + L"チェックを外すとmuxを行いません。"
        );
    fcgTTEx->SetToolTip(fcgCXMKVCmdEx,    L""
        + L"muxerに渡す追加オプションを選択します。\n"
        + L"これらの設定はVCEEnc.iniに記述されています。"
        );
    fcgTTEx->SetToolTip(fcgBTMKVMuxerPath, L""
        + L"mkv用muxerの場所を指定します。\n"
        + L"\n"
        + L"この設定はVCEEnc.confに保存され、\n"
        + L"バッチ処理ごとの変更はできません。"
        );
    fcgTTEx->SetToolTip(fcgCBMPGMuxerExt, L""
        + L"指定したmuxerでmuxを行います。\n"
        + L"チェックを外すとmuxを行いません。"
        );
    fcgTTEx->SetToolTip(fcgCXMPGCmdEx,    L""
        + L"muxerに渡す追加オプションを選択します。\n"
        + L"これらの設定はVCEEnc.iniに記述されています。"
        );
    fcgTTEx->SetToolTip(fcgBTMPGMuxerPath, L""
        + L"mpg用muxerの場所を指定します。\n"
        + L"\n"
        + L"この設定はVCEEnc.confに保存され、\n"
        + L"バッチ処理ごとの変更はできません。"
        );
    fcgTTEx->SetToolTip(fcgCBMuxMinimize, L""
        + L"mux時のウィンドウを最小化で開始します。"
        );
    fcgTTEx->SetToolTip(fcgCXMuxPriority, L""
        + L"muxerのCPU優先度を指定します。\n"
        + L"AviutlSync で Aviutlの優先度と同じになります。"
        );
    //バッチファイル実行
    fcgTTEx->SetToolTip(fcgCBRunBatBefore, L""
        + L"エンコード開始前にバッチファイルを実行します。"
        );
    fcgTTEx->SetToolTip(fcgCBRunBatAfter, L""
        + L"エンコード終了後、バッチファイルを実行します。"
        );
    fcgTTEx->SetToolTip(fcgCBWaitForBatBefore, L""
        + L"バッチ処理開始後、バッチ処理が終了するまで待機します。"
        );
    fcgTTEx->SetToolTip(fcgCBWaitForBatAfter, L""
        + L"バッチ処理開始後、バッチ処理が終了するまで待機します。"
        );
    fcgTTEx->SetToolTip(fcgBTBatBeforePath, L""
        + L"エンコード終了後実行するバッチファイルを指定します。\n"
        + L"実際のバッチ実行時には新たに\"<バッチファイル名>_tmp.bat\"を作成、\n"
        + L"指定したバッチファイルの内容をコピーし、\n"
        + L"さらに特定文字列を置換して実行します。\n"
        + L"使用できる置換文字列はreadmeをご覧下さい。"
        );
    fcgTTEx->SetToolTip(fcgBTBatAfterPath, L""
        + L"エンコード終了後実行するバッチファイルを指定します。\n"
        + L"実際のバッチ実行時には新たに\"<バッチファイル名>_tmp.bat\"を作成、\n"
        + L"指定したバッチファイルの内容をコピーし、\n"
        + L"さらに特定文字列を置換して実行します。\n"
        + L"使用できる置換文字列はreadmeをご覧下さい。"
        );
    //上部ツールストリップ
    fcgTSBDelete->ToolTipText = L""
        + L"現在選択中のプロファイルを削除します。";

    fcgTSBOtherSettings->ToolTipText = L""
        + L"プロファイルの保存フォルダを変更します。";

    fcgTSBSave->ToolTipText = L""
        + L"現在の設定をプロファイルに上書き保存します。";

    fcgTSBSaveNew->ToolTipText = L""
        + L"現在の設定を新たなプロファイルに保存します。";

    //他
    fcgTTEx->SetToolTip(fcgTXCmd,         L""
        + L"x264に渡される予定のコマンドラインです。\n"
        + L"エンコード時には更に\n"
        + L"・「追加コマンド」の付加\n"
        + L"・\"auto\"な設定項目の反映\n"
        + L"・必要な情報の付加(--fps/-o/--input-res/--input-csp/--frames等)\n"
        + L"が行われます。\n"
        + L"\n"
        + L"このウィンドウはダブルクリックで拡大縮小できます。"
        );
    fcgTTEx->SetToolTip(fcgBTDefault,     L""
        + L"デフォルト設定をロードします。"
        );
}
System::Void frmConfig::ShowExehelp(String^ ExePath, String^ args) {
    if (!File::Exists(ExePath)) {
        MessageBox::Show(L"指定された実行ファイルが存在しません。", L"エラー", MessageBoxButtons::OK, MessageBoxIcon::Error);
    } else {
        char exe_path[MAX_PATH_LEN];
        char file_path[MAX_PATH_LEN];
        char cmd[MAX_CMD_LEN];
        GetCHARfromString(exe_path, sizeof(exe_path), ExePath);
        apply_appendix(file_path, _countof(file_path), exe_path, "_fullhelp.txt");
        File::Delete(String(file_path).ToString());
        array<String^>^ arg_list = args->Split(L';');
        for (int i = 0; i < arg_list->Length; i++) {
            if (i) {
                System::IO::StreamWriter^ sw;
                try {
                    sw = gcnew System::IO::StreamWriter(String(file_path).ToString(), true, System::Text::Encoding::GetEncoding("shift_jis"));
                    sw->WriteLine();
                    sw->WriteLine();
                } finally {
                    if (sw != nullptr) { sw->Close(); }
                }
            }
            GetCHARfromString(cmd, sizeof(cmd), arg_list[i]);
            if (get_exe_message_to_file(exe_path, cmd, file_path, AUO_PIPE_MUXED, 5) != RP_SUCCESS) {
                File::Delete(String(file_path).ToString());
                MessageBox::Show(L"helpの取得に失敗しました。", L"エラー", MessageBoxButtons::OK, MessageBoxIcon::Error);
                return;
            }
        }
        try {
            System::Diagnostics::Process::Start(String(file_path).ToString());
        } catch (...) {
            MessageBox::Show(L"helpを開く際に不明なエラーが発生しました。", L"エラー", MessageBoxButtons::OK, MessageBoxIcon::Error);
        }
    }
}

System::Void frmConfig::SetVidEncInfo(VidEncInfo info) {
    if (this->InvokeRequired) {
        this->Invoke(gcnew SetVidEncInfoDelegate(this, &frmConfig::SetVidEncInfo), info);
    } else {
        fcgpictureBoxVCEEnabled->Visible = encInfo.hwencAvail;
        fcgCXCodec_SelectedIndexChanged(nullptr, nullptr);
        fcgRebuildCmd(nullptr, nullptr);
    }
}

VidEncInfo frmConfig::GetVidEncInfo() {
    char exe_path[MAX_PATH_LEN];
    char mes[8192];

    VidEncInfo info;
    info.hwencAvail = false;

    if (File::Exists(LocalStg.vidEncPath)) {
        GetCHARfromString(exe_path, sizeof(exe_path), LocalStg.vidEncPath);
    } else {
        const auto defaultExePath = find_latest_videnc_for_frm();
        if (defaultExePath.length() > 0) {
            strcpy_s(exe_path, defaultExePath.c_str());
        }
    }
    if (get_exe_message(exe_path, "--check-hw", mes, _countof(mes), AUO_PIPE_MUXED) == RP_SUCCESS) {
        auto lines = String(mes).ToString()->Split(String(L"\r\n").ToString()->ToCharArray(), System::StringSplitOptions::RemoveEmptyEntries);
        for (int i = 0; i < lines->Length; i++) {
            if (lines[i]->Contains("VCE available")) {
                info.hwencAvail = true;
            }
        }
    }
    encInfo = info;
    SetVidEncInfo(info);
    return info;
}

System::Void frmConfig::GetVidEncInfoAsync() {
    if (!File::Exists(LocalStg.vidEncPath)) {
        auto defaultExePath = find_latest_videnc_for_frm();
        if (defaultExePath.length() == 0) {
            encInfo.hwencAvail = false;
            return;
        }
        String^ exePath = String(defaultExePath.c_str()).ToString();
        if (!File::Exists(exePath)) {
            encInfo.hwencAvail = false;
            return;
        }
    }
    if (taskEncInfo != nullptr && !taskEncInfo->IsCompleted) {
        taskEncInfoCancell->Cancel();
    }
    taskEncInfoCancell = gcnew CancellationTokenSource();
    taskEncInfo = Task<VidEncInfo>::Factory->StartNew(gcnew Func<VidEncInfo>(this, &frmConfig::GetVidEncInfo), taskEncInfoCancell->Token);
}

#pragma warning( pop )
