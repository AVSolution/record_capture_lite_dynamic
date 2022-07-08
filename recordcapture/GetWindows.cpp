#include "GDIHelpers.h"
#include "Capture.h"
#include "SCCommon.h"
#include <Psapi.h>
#include <algorithm>
#include <ShlObj.h>
#include <iostream>
using namespace std;

#define WIN32_LEAN_AND_MEAN // Exclude rarely-used stuff from Windows headers
#include <Windows.h>

namespace RL {
namespace RecordCapture {

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

#ifdef _WIN64
		HWND parentWnd = (HWND)GetWindowLong(hwnd, GWLP_HWNDPARENT);
#else
		HWND parentWnd = (HWND)GetWindowLong(hwnd, GWL_HWNDPARENT);
#endif
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
        //std::cout << "enum window name: " << w.Name <<" handle: "<<hwnd<< std::endl;
		LogInstance()->rlog(IRecordLog::LOG_INFO, "enum window name: %s,hwnd: %p", w.Name, hwnd);

        srch *s = (srch *)lParam;
        w.Handle = reinterpret_cast<size_t>(hwnd);
        auto windowrect = RL::RecordCapture::GetWindowRect(hwnd);
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

	std::string GetLocalAppDataPath()
	{
		char m_lpszDefaultDir[MAX_PATH];
		char szDocument[MAX_PATH] = { 0 };
		memset(m_lpszDefaultDir, 0, _MAX_PATH);

		LPITEMIDLIST pidl = NULL;
		SHGetSpecialFolderLocation(NULL, CSIDL_LOCAL_APPDATA, &pidl);
		if (pidl && SHGetPathFromIDList(pidl, szDocument))
		{
			GetShortPathName(szDocument, m_lpszDefaultDir, _MAX_PATH);
		}

		return m_lpszDefaultDir;
	}

} // namespace RecordCapture
} // namespace RL
