#pragma once
#include "capture.h"
#include "SCCommon.h"
#include <memory>
#include "GDIHelpers.h"

namespace RL {
    namespace RecordCapture {

        class GDIFrameProcessor : public BaseFrameProcessor {
            HDCWrapper MonitorDC;
            HDCWrapper CaptureDC;
            HBITMAPWrapper CaptureBMP;
            Monitor SelectedMonitor;
            HWND SelectedWindow;
			bool isWin7{ false };
            std::unique_ptr<unsigned char[]> NewImageBuffer;
			

           // std::shared_ptr<Thread_Data> Data;
        public:
            void Pause() {
				if (IsWindowVisible(SelectedWindow) || isWin7) {
					RECT rt;
					GetWindowRect(SelectedWindow, &rt);
					int res = SetWindowPos(SelectedWindow, HWND_NOTOPMOST, rt.left, rt.top, rt.right - rt.left, rt.bottom - rt.top, SWP_NOSIZE | SWP_NOMOVE);
					LogInstance()->rlog(IRecordLog::LOG_INFO, "pause capture. win7 restore window attribute notopmost\n");
				}
			}
            void Resume() {}
            DUPL_RETURN Init(std::shared_ptr<Thread_Data> data, const Monitor& monitor);
            DUPL_RETURN ProcessFrame(const Monitor& currentmonitorinfo);
            DUPL_RETURN Init(std::shared_ptr<Thread_Data> data, const Window& selectedwindow);
            DUPL_RETURN ProcessFrame(Window& selectedwindow);
        };
    }
}