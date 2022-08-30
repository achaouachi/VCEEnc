﻿// -----------------------------------------------------------------------------------------
// x264guiEx/x265guiEx/svtAV1guiEx/ffmpegOut/QSVEnc/NVEnc/VCEEnc by rigaya
// -----------------------------------------------------------------------------------------
// The MIT License
//
// Copyright (c) 2010-2022 rigaya
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
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//
// --------------------------------------------------------------------------------------------

#ifndef _AUO_CONF_H_
#define _AUO_CONF_H_

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#include "auo.h"
#include "auo_version.h"

const int CONF_INITIALIZED = 1;

enum {
    TMP_DIR_OUTPUT = 0,
    TMP_DIR_SYSTEM = 1,
    TMP_DIR_CUSTOM = 2,
};

enum : DWORD {
    RUN_BAT_NONE           = 0x00,
    RUN_BAT_BEFORE_PROCESS = 0x01,
    RUN_BAT_AFTER_PROCESS  = 0x02,
    RUN_BAT_BEFORE_AUDIO   = 0x04,
    RUN_BAT_AFTER_AUDIO    = 0x08,
};

static inline int get_run_bat_idx(DWORD flag) {
    DWORD ret;
    _BitScanForward(&ret, flag);
    return (int)ret;
}

#if ENCODER_QSV
#include "qsv_util.h"
#include "qsv_prm.h"

static const char *const CONF_NAME_OLD_1 = "QSVEnc ConfigFile";
static const char *const CONF_NAME_OLD_2 = "QSVEnc ConfigFile v2";
static const char *const CONF_NAME_OLD_3 = "QSVEnc ConfigFile v3";
static const char *const CONF_NAME_OLD_4 = "QSVEnc ConfigFile v4";
static const char *const CONF_NAME_OLD_5 = "QSVEnc ConfigFile v5";
static const char *const CONF_NAME_OLD_6 = "QSVEnc ConfigFile v6";
static const char *const CONF_NAME       = CONF_NAME_OLD_6;
const int CONF_NAME_BLOCK_LEN            = 32;
const int CONF_BLOCK_MAX                 = 32;
const int CONF_BLOCK_COUNT               = 6; //最大 CONF_BLOCK_MAXまで
const int CONF_HEAD_SIZE                 = (3 + CONF_BLOCK_MAX) * sizeof(int) + CONF_BLOCK_MAX * sizeof(size_t) + CONF_NAME_BLOCK_LEN;
#elif ENCODER_NVENC
#include "NVEncParam.h"

static const char *const CONF_NAME_OLD_1 = "NVEnc ConfigFile";
static const char *const CONF_NAME_OLD_2 = "NVEnc ConfigFile v2";
static const char *const CONF_NAME_OLD_3 = "NVEnc ConfigFile v3";
static const char *const CONF_NAME_OLD_4 = "NVEnc ConfigFile v4";
static const char *const CONF_NAME       = CONF_NAME_OLD_4;
const int CONF_NAME_BLOCK_LEN            = 32;
const int CONF_BLOCK_MAX                 = 32;
const int CONF_BLOCK_COUNT               = 5; //最大 CONF_BLOCK_MAXまで
const int CONF_HEAD_SIZE                 = (3 + CONF_BLOCK_MAX) * sizeof(int) + CONF_BLOCK_MAX * sizeof(size_t) + CONF_NAME_BLOCK_LEN;
#elif ENCODER_VCEENC
#include "vce_param.h"

static const char *CONF_NAME          = "VCEEnc ConfigFile v3";
const int CONF_NAME_BLOCK_LEN         = 32;
const int CONF_BLOCK_MAX              = 32;
const int CONF_BLOCK_COUNT            = 5; //最大 CONF_BLOCK_MAXまで
const int CONF_HEAD_SIZE              = (3 + CONF_BLOCK_MAX) * sizeof(int) + CONF_BLOCK_MAX * sizeof(size_t) + CONF_NAME_BLOCK_LEN;
#else
static_assert(false);
#endif

static const char *const CONF_LAST_OUT = "前回出力.stg";

enum {
    CONF_ERROR_NONE = 0,
    CONF_ERROR_FILE_OPEN,
    CONF_ERROR_BLOCK_SIZE,
    CONF_ERROR_INVALID_FILENAME,
};

enum {
    AUDIO_DELAY_CUT_NONE         = 0, //音声エンコード遅延の削除を行わない
    AUDIO_DELAY_CUT_DELETE_AUDIO = 1, //音声エンコード遅延の削除を音声の先頭を削除することで行う
    AUDIO_DELAY_CUT_ADD_VIDEO    = 2, //音声エンコード遅延の削除を映像を先頭に追加することで行う
    AUDIO_DELAY_CUT_EDTS         = 3, //音声エンコード遅延の削除をedtsを用いて行う
};

static const char *const AUDIO_DELAY_CUT_MODE[] = {
    "補正なし",
    "音声カット",
    "----------",
    "edts",
    NULL
};

const int CMDEX_MAX_LEN = 2048;    //追加コマンドラインの最大長

#pragma pack(push, 1)
typedef struct CONF_VCE {
    int codec;
    int reserved[128];
#if ENCODER_QSV
    char reserved3[1024];
#endif
    char cmd[3072];
    char cmdex[512];
    char reserved2[512];
} CONF_VCE;

typedef struct CONF_VIDEO {
    BOOL afs;                      //自動フィールドシフトの使用
    BOOL auo_tcfile_out;           //auo側でタイムコードを出力する
#if ENCODER_QSV
    int  reserved[2];
#endif
    BOOL resize_enable;
    int resize_width;
    int resize_height;
} CONF_VIDEO;

typedef struct {
    int  encoder;             //使用する音声エンコーダ
    int  enc_mode;            //使用する音声エンコーダの設定
    int  bitrate;             //ビットレート指定モード
    BOOL use_2pass;           //音声2passエンコードを行う
    BOOL use_wav;             //パイプを使用せず、wavを出力してエンコードを行う
    BOOL faw_check;           //FAWCheckを行う
    int  priority;            //音声エンコーダのCPU優先度(インデックス)
    BOOL minimized;           //音声エンコーダを最小化で実行
    int  aud_temp_dir;        //音声専用一時フォルダ
    int  audio_encode_timing; //音声を先にエンコード
    int  delay_cut;           //エンコード遅延の削除
} CONF_AUDIO_BASE; //音声用設定

typedef struct CONF_AUDIO {
    CONF_AUDIO_BASE ext;
    CONF_AUDIO_BASE in;
    BOOL use_internal;
} CONF_AUDIO; //音声用設定

typedef struct CONF_MUX {
    BOOL disable_mp4ext;  //mp4出力時、外部muxerを使用する
    BOOL disable_mkvext;  //mkv出力時、外部muxerを使用する
    int  mp4_mode;        //mp4 外部muxer用追加コマンドの設定
    int  mkv_mode;        //mkv 外部muxer用追加コマンドの設定
    BOOL minimized;       //muxを最小化で実行
    int  priority;        //mux優先度(インデックス)
    int  mp4_temp_dir;    //mp4box用一時ディレクトリ
    BOOL apple_mode;      //Apple用モード(mp4系専用)
    BOOL disable_mpgext;  //mpg出力時、外部muxerを使用する
    int  mpg_mode;        //mpg 外部muxer用追加コマンドの設定
    BOOL use_internal;    //内蔵muxerの使用
    int  internal_mode;   //内蔵muxer用のオプション
} CONF_MUX; //muxer用設定

typedef struct CONF_OTHER {
    //BOOL disable_guicmd;         //GUIによるコマンドライン生成を停止(CLIモード)
    int   temp_dir;               //一時ディレクトリ
    BOOL  out_audio_only;         //音声のみ出力
    char  notes[128];             //メモ
    DWORD run_bat;                //バッチファイルを実行するかどうか (RUN_BAT_xxx)
    DWORD dont_wait_bat_fin;      //バッチファイルの処理終了待機をするかどうか (RUN_BAT_xxx)
    union {
        char batfiles[4][512];        //バッチファイルのパス
        struct {
            char before_process[512]; //エンコ前バッチファイルのパス
            char after_process[512];  //エンコ後バッチファイルのパス
            char before_audio[512];   //音声エンコ前バッチファイルのパス
            char after_audio[512];    //音声エンコ後バッチファイルのパス
        } batfile;
    };
} CONF_OTHER;

typedef struct CONF_GUIEX {
    char        conf_name[CONF_NAME_BLOCK_LEN];  //保存時に使用
    int         size_all;                        //保存時: CONF_GUIEXの全サイズ / 設定中、エンコ中: CONF_INITIALIZED
    int         head_size;                       //ヘッダ部分の全サイズ
    int         block_count;                     //ヘッダ部を除いた設定のブロック数
    int         block_size[CONF_BLOCK_MAX];      //各ブロックのサイズ
    size_t      block_head_p[CONF_BLOCK_MAX];    //各ブロックのポインタ位置
    CONF_VCE    enc;                             //vceについての設定
    CONF_VIDEO  vid;                             //その他動画についての設定
    CONF_AUDIO  aud;                             //音声についての設定
    CONF_MUX    mux;                             //muxについての設定
    CONF_OTHER  oth;                             //その他の設定
} CONF_GUIEX;
#pragma pack(pop)

class guiEx_config {
private:
    static const size_t conf_block_pointer[CONF_BLOCK_COUNT];
    static const int conf_block_data[CONF_BLOCK_COUNT];
#if ENCODER_QSV
    static void *convert_qsvstgv1_to_stgv3(void *_conf, int size);
    static void *convert_qsvstgv2_to_stgv3(void *_conf);
    static void *convert_qsvstgv3_to_stgv4(void *_conf);
    static void *convert_qsvstgv4_to_stgv5(void *_conf);
    static void *convert_qsvstgv5_to_stgv6(void *_conf);
#elif ENCODER_NVENC
    static int  stgv3_block_size();
    static void convert_nvencstg_to_nvencstgv4(CONF_GUIEX *conf, const void *dat);
    static void convert_nvencstgv2_to_nvencstgv3(void *dat);
    static void convert_nvencstgv2_to_nvencstgv4(CONF_GUIEX *conf, const void *dat);
    static void convert_nvencstgv3_to_nvencstgv4(CONF_GUIEX *conf, const void *dat);
#endif
public:
    guiEx_config();
    static void write_conf_header(CONF_GUIEX *conf);
    static int  adjust_conf_size(CONF_GUIEX *conf_buf, void *old_data, int old_size);
    static int  load_guiEx_conf(CONF_GUIEX *conf, const char *stg_file);       //設定をstgファイルから読み込み
    static int  save_guiEx_conf(const CONF_GUIEX *conf, const char *stg_file); //設定をstgファイルとして保存
};

void init_CONF_GUIEX(CONF_GUIEX *conf, BOOL use_highbit); //初期化し、デフォルトを設定

//出力ファイルの拡張子フィルタを作成
//filterがNULLならauoのOUTPUT_PLUGIN_TABLE用のフィルタを書き換える
void make_file_filter(char *filter, size_t nSize, int default_index);

void overwrite_aviutl_ini_file_filter(int idx);
void overwrite_aviutl_ini_name();

#endif //_AUO_CONF_H_
