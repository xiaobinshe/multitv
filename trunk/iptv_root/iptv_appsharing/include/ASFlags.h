#pragma once
#ifndef __ASFLAGS__
#define __ASFLAGS__

enum flModes
{
    AS_NULL = 0,
    AS_CAPTURING,
    AS_RECEIVING
};

enum flCaptureMode
{
	AS_GDI = 0,		// Indexes based on iptv_video_capture output
	AS_DRIVER = 1
};

enum flASNotifies
{
    AS_NOTIFY_NULL = 0,
    AS_NOTIFY_KMMSG,
    AS_NOTIFY_MOUSEMOVE,
    AS_NOTIFY_LBUTTONUP,
    AS_NOTIFY_LBUTTONDOWN,
    AS_NOTIFY_CAPTURE_ERROR,
    AS_NOTIFY_CAPTURE_START,
    AS_NOTIFY_CAPTURE_STOP,
    AS_NOTIFY_RECEIVE_ERROR,
    AS_NOTIFY_RECEIVE_START,
    AS_NOTIFY_RECEIVE_STOP,
	AS_NOTIFY_RECEIVE_NEW_FRAME
};

enum flASErrors
{
    AS_ERROR_NULL = 0,
    AS_ERROR_CAP_CREATEDC,
    AS_ERROR_CAP_ATTACHMIRROR,
    AS_ERROR_CAP_GETDEVICENAME,
    AS_ERROR_CAP_EXPESCAPE,
    AS_ERROR_CAP_MAPMIRROR

};

enum flASFrameFormat
{
	AS_FRAME_FORMAT_NULL = 0,	 // Null
	AS_FRAME_FORMAT_RGB,		 // RGB  8BPP
	AS_FRAME_FORMAT_ARGB,		 // RGBA 8BPP
	AS_FRAME_FORMAT_BMP,		 // Bitmap file structure
	AS_FRAME_FORMAT_DIB_BGR,	 // 24-bit DIB
	AS_FRAME_FORMAT_DIB_GBRA,	 // 32-bit DIB
	AS_FRAME_FORMAT_DIB_GBR565,  // 16-bit DIB
	AS_FRAME_FORMAT_DIB_BGR565,	 // 16-bit DIB
	AS_FRAME_FORMAT_DIB_BGR555,  // 16-bit DIB
};
#endif //__ASFLAGS__
