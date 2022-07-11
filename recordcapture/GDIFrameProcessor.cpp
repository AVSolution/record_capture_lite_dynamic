#include "GDIFrameProcessor.h"
#include <Dwmapi.h>

namespace RL {
namespace RecordCapture {

    DUPL_RETURN GDIFrameProcessor::Init(std::shared_ptr<Thread_Data> data, const Monitor &monitor)
    {
        SelectedMonitor = monitor;
        auto Ret = DUPL_RETURN_SUCCESS;

        MonitorDC.DC = CreateDCA(Name(SelectedMonitor), NULL, NULL, NULL);
        CaptureDC.DC = CreateCompatibleDC(MonitorDC.DC);
        CaptureBMP.Bitmap = CreateCompatibleBitmap(MonitorDC.DC, Width(SelectedMonitor), Height(SelectedMonitor));
        NewImageBuffer = std::make_unique<unsigned char[]>(ImageBufferSize);
        if (!MonitorDC.DC || !CaptureDC.DC || !CaptureBMP.Bitmap) {
            return DUPL_RETURN::DUPL_RETURN_ERROR_EXPECTED;
        }

        Data = data;
        return Ret;
    }
    DUPL_RETURN GDIFrameProcessor::Init(std::shared_ptr<Thread_Data> data, const Window &selectedwindow)
    {
        // this is needed to fix AERO BitBlt capturing issues
		OSVERSIONINFOEX info;
		ZeroMemory(&info, sizeof(OSVERSIONINFOEX));
		info.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
		GetVersionExA((LPOSVERSIONINFO)&info);
		LogInstance()->rlog(IRecordLog::LOG_INFO, "Windows version: %u.%u\n", info.dwMajorVersion, info.dwMinorVersion);
		if (info.dwMajorVersion == 6 && info.dwMinorVersion == 1)
			isWin7 = true;

        ANIMATIONINFO str;
        str.cbSize = sizeof(str);
        str.iMinAnimate = 0;
        SystemParametersInfo(SPI_SETANIMATION, sizeof(str), (void *)&str, SPIF_UPDATEINIFILE | SPIF_SENDCHANGE);

        SelectedWindow = reinterpret_cast<HWND>(selectedwindow.Handle);
		//win7 add top_most attribute
		if (isWin7) {
			RECT rt;
			GetWindowRect(SelectedWindow, &rt);
			SetWindowPos(SelectedWindow, HWND_TOPMOST, rt.left, rt.top, rt.right - rt.left, rt.bottom - rt.top, SWP_NOSIZE | SWP_NOMOVE);
		}
        auto Ret = DUPL_RETURN_SUCCESS;
        NewImageBuffer = std::make_unique<unsigned char[]>(ImageBufferSize);
        MonitorDC.DC = GetWindowDC(SelectedWindow);
        CaptureDC.DC = CreateCompatibleDC(MonitorDC.DC);

        CaptureBMP.Bitmap = CreateCompatibleBitmap(MonitorDC.DC, selectedwindow.Size.x, selectedwindow.Size.y);

        if (!MonitorDC.DC || !CaptureDC.DC || !CaptureBMP.Bitmap) {
            return DUPL_RETURN::DUPL_RETURN_ERROR_EXPECTED;
        }

        Data = data;
        return Ret;
    }
    DUPL_RETURN GDIFrameProcessor::ProcessFrame(const Monitor &currentmonitorinfo)
    {

        auto Ret = DUPL_RETURN_SUCCESS;

        ImageRect ret;
        ret.left = ret.top = 0;
        ret.bottom = Height(SelectedMonitor);
        ret.right = Width(SelectedMonitor);

        // Selecting an object into the specified DC
        auto originalBmp = SelectObject(CaptureDC.DC, CaptureBMP.Bitmap);

        if (BitBlt(CaptureDC.DC, 0, 0, ret.right, ret.bottom, MonitorDC.DC, 0, 0, SRCCOPY | CAPTUREBLT) == FALSE) {
            // if the screen cannot be captured, return
            SelectObject(CaptureDC.DC, originalBmp);
            return DUPL_RETURN::DUPL_RETURN_ERROR_EXPECTED; // likely a permission issue
        }
        else {

            BITMAPINFOHEADER bi;
            memset(&bi, 0, sizeof(bi));

            bi.biSize = sizeof(BITMAPINFOHEADER);

            bi.biWidth = ret.right;
            bi.biHeight = -ret.bottom;
            bi.biPlanes = 1;
            bi.biBitCount = sizeof(ImageBGRA) * 8; // always 32 bits damnit!!!
            bi.biCompression = BI_RGB;
            bi.biSizeImage = ((ret.right * bi.biBitCount + 31) / (sizeof(ImageBGRA) * 8)) * sizeof(ImageBGRA)  * ret.bottom;
            GetDIBits(MonitorDC.DC, CaptureBMP.Bitmap, 0, (UINT)ret.bottom, NewImageBuffer.get(), (BITMAPINFO *)&bi, DIB_RGB_COLORS);
            SelectObject(CaptureDC.DC, originalBmp);
            ProcessCapture(Data->ScreenCaptureData, *this, currentmonitorinfo, NewImageBuffer.get(), Width(SelectedMonitor)* sizeof(ImageBGRA));
        }

        return Ret;
    }

    DUPL_RETURN GDIFrameProcessor::ProcessFrame(Window &selectedwindow)
    {
        auto Ret = DUPL_RETURN_SUCCESS;
        auto windowrect = RL::RecordCapture::GetWindowRect(SelectedWindow);
        ImageRect ret;
        memset(&ret, 0, sizeof(ret));
        ret.bottom = windowrect.ClientRect.bottom;
        ret.left = windowrect.ClientRect.left;
        ret.right = windowrect.ClientRect.right;
        ret.top = windowrect.ClientRect.top;
        selectedwindow.Position.x = windowrect.ClientRect.left;
        selectedwindow.Position.y = windowrect.ClientRect.top;

        if (!IsWindow(SelectedWindow) || selectedwindow.Size.x != Width(ret) || selectedwindow.Size.y != Height(ret)) {
            return DUPL_RETURN::DUPL_RETURN_ERROR_EXPECTED; // window size changed. This will rebuild everything
        }

        // Selecting an object into the specified DC
        auto originalBmp = SelectObject(CaptureDC.DC, CaptureBMP.Bitmap);
        auto left = -windowrect.ClientBorder.left;
        auto top = -windowrect.ClientBorder.top;

        if (BitBlt(CaptureDC.DC, left, top, ret.right - ret.left, ret.bottom - ret.top, MonitorDC.DC, 0, 0, SRCCOPY) == FALSE) {
            // if the screen cannot be captured, return
            SelectObject(CaptureDC.DC, originalBmp);
            return DUPL_RETURN::DUPL_RETURN_ERROR_EXPECTED; // likely a permission issue
        }
        else {
			//add mouse
			CURSORINFO cursor = { sizeof(cursor) };
			GetCursorInfo(&cursor);
			if (cursor.flags == CURSOR_SHOWING) {
				RECT rect;
				GetWindowRect(SelectedWindow, &rect);
				ICONINFO info = { sizeof(info) };
				GetIconInfo(cursor.hCursor, &info);
				const int x = cursor.ptScreenPos.x - rect.left - info.xHotspot;
				const int y = cursor.ptScreenPos.y - rect.top - info.yHotspot;
				BITMAP bmpCursor = { 0 };
				GetObject(info.hbmColor, sizeof(bmpCursor), &bmpCursor);
				DrawIconEx(CaptureDC.DC, x, y, cursor.hCursor, bmpCursor.bmWidth, bmpCursor.bmHeight,
					0, NULL, DI_NORMAL);
			}

            BITMAPINFOHEADER bi;
            memset(&bi, 0, sizeof(bi));

            bi.biSize = sizeof(BITMAPINFOHEADER);

            bi.biWidth = Width(ret);
            bi.biHeight = -Height(ret);
            bi.biPlanes = 1;
            bi.biBitCount = sizeof(ImageBGRA) * 8; // always 32 bits damnit!!!
            bi.biCompression = BI_RGB;
            bi.biSizeImage = ((Width(ret) * bi.biBitCount + 31) / (sizeof(ImageBGRA) * 8)) * sizeof(ImageBGRA)  * Height(ret);
            GetDIBits(MonitorDC.DC, CaptureBMP.Bitmap, 0, (UINT)Height(ret), NewImageBuffer.get(), (BITMAPINFO *)&bi, DIB_RGB_COLORS);
            SelectObject(CaptureDC.DC, originalBmp);
            ProcessCapture(Data->WindowCaptureData, *this, selectedwindow, NewImageBuffer.get(), Width(selectedwindow)* sizeof(ImageBGRA));
        }

        return Ret;
    }
} // namespace RecordCapture
} // namespace RL