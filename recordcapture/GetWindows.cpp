#include "GDIHelpers.h"
#include "Capture.h"
#include "SCCommon.h"
#include <Psapi.h>
#include <algorithm>
#include <iostream>
using namespace std;

#define WIN32_LEAN_AND_MEAN // Exclude rarely-used stuff from Windows headers
#include <Windows.h>

namespace RL {
namespace recordcapture {

    struct srch {
        std::vector<Window> Found;
    };
    BOOL CALLBACK EnumWindowsProc(_In_ HWND hwnd, _In_ LPARAM lParam)
    {
        Window w = {};

        DWORD pid;

        if (!IsWindowVisible(hwnd))
            return true;

        if (!IsWindowEnabled(hwnd))
            return true;

        long lstyle = GetWindowLong(hwnd, GWL_STYLE);
        if ((lstyle & WS_CHILDWINDOW))
            return true;

        HWND parentWnd = (HWND)GetWindowLong(hwnd, GWL_HWNDPARENT);
        if (IsWindowEnabled(parentWnd))
            return true;

		if (IsWindowVisible(parentWnd))
            return true;

        GetWindowThreadProcessId(hwnd, &pid);
        w.Name[0] = '\n';
        if (pid != GetCurrentProcessId()) {
            auto textlen = GetWindowTextA(hwnd, w.Name, sizeof(w.Name));
        }
        if (!strlen(w.Name)) {
            return true;
        }
        std::cout << "enum window name: " << w.Name <<" handle: "<<hwnd<< std::endl;

        srch *s = (srch *)lParam;
        w.Handle = reinterpret_cast<size_t>(hwnd);
        auto windowrect = RL::recordcapture::GetWindowRect(hwnd);
        w.Position.x = windowrect.ClientRect.left;
        w.Position.y = windowrect.ClientRect.top;
        w.Size.x = windowrect.ClientRect.right - windowrect.ClientRect.left;
        w.Size.y = windowrect.ClientRect.bottom - windowrect.ClientRect.top;
        std::transform(std::begin(w.Name), std::end(w.Name), std::begin(w.Name), ::tolower);
        s->Found.push_back(w);
        return TRUE;
    }

    std::vector<Window> GetWindows()
    {
        srch s;
        EnumWindows(EnumWindowsProc, (LPARAM)&s);
        return s.Found;
    }

} // namespace recordcapture
} // namespace RL
