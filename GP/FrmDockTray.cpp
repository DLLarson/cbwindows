// FrmDockTray.cpp - container window for the tray palettes.
//
// Copyright (c) 1994-2020 By Dale L. Larson, All Rights Reserved.
//
// Permission is hereby granted, free of charge, to any person obtaining
// a copy of this software and associated documentation files (the
// "Software"), to deal in the Software without restriction, including
// without limitation the rights to use, copy, modify, merge, publish,
// distribute, sublicense, and/or sell copies of the Software, and to
// permit persons to whom the Software is furnished to do so, subject to
// the following conditions:
//
// The above copyright notice and this permission notice shall be
// included in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
// NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
// LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
// OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
// WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
//

#include "stdafx.h"
#include "Gp.h"
#include "FrmMain.h"
#include "FrmDockTray.h"
#include "PalTray.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////

BEGIN_MESSAGE_MAP(CDockTrayPalette, CDockablePane)
    ON_WM_DESTROY()
    ON_WM_SIZE()
    ON_WM_WINDOWPOSCHANGED()  // TEST INTERCEPT
END_MESSAGE_MAP()

IMPLEMENT_DYNAMIC(CDockTrayPalette, CDockablePane);

/////////////////////////////////////////////////////////////////////////////

CDockTrayPalette::CDockTrayPalette()
{
    m_pChildWnd = NULL;
}

CDockTrayPalette::~CDockTrayPalette()
{
}

/////////////////////////////////////////////////////////////////////////////

void CDockTrayPalette::SetChild(CTrayPalette* pChildWnd)
{
    if (m_pChildWnd == pChildWnd)
        return;

    if (m_pChildWnd != NULL)
    {
        m_pChildWnd->ShowWindow(SW_HIDE);
        m_pChildWnd->SetDockingFrame(NULL);
        Invalidate(TRUE);
    }
    // We need to set this field explicit rather than
    // using CDockablePane::SetChild() since this function
    // insists that the window be non-NULL even though it's
    // perfectly fine to be NULL!
    m_pChildWnd = pChildWnd;
    if (pChildWnd != NULL)
    {
        pChildWnd->SetDockingFrame(this);
        CRect rct;
        GetClientRect(rct);
        pChildWnd->MoveWindow(&rct);
        m_pChildWnd->ShowWindow(SW_SHOW);
    }
    else
        GetMainFrame()->ShowPane(this, FALSE, TRUE, FALSE);
}

/////////////////////////////////////////////////////////////////////////////
// CDockTrayPalette message handlers

void CDockTrayPalette::OnSize(UINT nType, int cx, int cy)
{
    CDockablePane::OnSize(nType, cx, cy);
    CRect rct;
    GetClientRect(rct);
    if (m_pChildWnd != NULL)
        m_pChildWnd->MoveWindow(&rct);
}

void CDockTrayPalette::OnDestroy()
{
    m_pChildWnd = NULL;
    CDockablePane::OnDestroy();
}

/// TESTING INTERCEPTS...
CSize CDockTrayPalette::CalcFixedLayout(BOOL bStretch, BOOL bHorz)
{
    ATLTRACE2(traceAppMsg, 0, "CDockTrayPalette%s::CalcFixedLayout(bStretch=%u, bHorz=%d) called...\n", (LPCSTR)m_trayID, bStretch, bHorz);
    CSize ret = CDockablePane::CalcFixedLayout(bStretch, bHorz);
    ATLTRACE2(traceAppMsg, 0, "...CDockTrayPalette%s::CalcFixedLayout(...) returns CSize(%d, %d).\n", (LPCSTR)m_trayID, ret.cx, ret.cy);
    return ret;
    //return CDockablePane::CalcFixedLayout(bStretch, bHorz);
}

BOOL CDockTrayPalette::IsResizable() const
{
    ATLTRACE2(traceAppMsg, 0, "CDockTrayPalette%s::IsResizable() called...\n", (LPCSTR)m_trayID);
    BOOL bRet = CDockablePane::IsResizable();
    ATLTRACE2(traceAppMsg, 0, "...CDockTrayPalette%s::IsResizable() returns BOOL(%d)\n", (LPCSTR)m_trayID, bRet);
    return bRet;
    //return CDockablePane::IsResizable();
}

CSize CDockTrayPalette::CalcAvailableSize(CRect rectRequired)
{
    ATLTRACE2(traceAppMsg, 0, "CDockTrayPalette%s::CalcAvailableSize(rectRequired={%d, %d, %d, $d}) called...\n", (LPCSTR)m_trayID,
        rectRequired.top, rectRequired.left, rectRequired.bottom, rectRequired.right);
    CSize ret = CDockablePane::CalcAvailableSize(rectRequired);
    ATLTRACE2(traceAppMsg, 0, "...CDockTrayPalette%s::CalcAvailableSize(...) returns CSize(%d, %d).\n", (LPCSTR)m_trayID, ret.cx, ret.cy);
    return ret;
    //return CDockablePane::CalcAvailableSize(rectRequired);
}

CSize CDockTrayPalette::CalcSize(BOOL bVertDock)
{
    ATLTRACE2(traceAppMsg, 0, "CDockTrayPalette%s::CalcSize(bVertDock=%u) called...\n", (LPCSTR)m_trayID, bVertDock);
    CSize ret = CDockablePane::CalcSize(bVertDock);
    ATLTRACE2(traceAppMsg, 0, "...CDockTrayPalette%s::CalcSize(...) returns CSize(%d, %d).\n", (LPCSTR)m_trayID, ret.cx, ret.cy);
    return ret;
    //return CDockablePane::CalcSize(bVertDock);
}

void CDockTrayPalette::GetMinSize(CSize& size) const
{
    ATLTRACE2(traceAppMsg, 0, "CDockTrayPalette%s::GetMinSize(size=CSize(%d, %d)) called...\n", (LPCSTR)m_trayID, size.cx, size.cy);
    CDockablePane::GetMinSize(size);
    ATLTRACE2(traceAppMsg, 0, "...CDockTrayPalette%s::GetMinSize(...) returns CSize(%d, %d).\n", (LPCSTR)m_trayID, size.cx, size.cy);
}

void CDockTrayPalette::RecalcLayout()
{
    ATLTRACE2(traceAppMsg, 0, "CDockTrayPalette%s::RecalcLayout called...\n", (LPCSTR)m_trayID);
    CDockablePane::RecalcLayout();
    ATLTRACE2(traceAppMsg, 0, "...CDockTrayPalette%s::RecalcLayout returns.\n", (LPCSTR)m_trayID);
}

void CDockTrayPalette::AdjustLayout()
{
    ATLTRACE2(traceAppMsg, 0, "CDockTrayPalette%s::AdjustLayout called...\n", (LPCSTR)m_trayID);
    CDockablePane::AdjustLayout();
    ATLTRACE2(traceAppMsg, 0, "...CDockTrayPalette%s::AdjustLayout returns.\n", (LPCSTR)m_trayID);
}

void CDockTrayPalette::OnWindowPosChanged(WINDOWPOS FAR* lpwndpos)
{
    ATLTRACE2(traceAppMsg, 0, "CDockTrayPalette%s::OnWindowPosChanged(x=%d, y=%d, cx=%d, cy=%d) called...\n", (LPCSTR)m_trayID,
        lpwndpos->x, lpwndpos->y, lpwndpos->cx, lpwndpos->cy);
    if (lpwndpos->cx > 200 || lpwndpos->cy < 100)
        TRACE0("SOMETHING BAD HAPPENED..");
    CDockablePane::OnWindowPosChanged(lpwndpos);
    ATLTRACE2(traceAppMsg, 0, "...CDockTrayPalette%s::OnWindowPosChanged returns.\n", (LPCSTR)m_trayID);
}

/// ...TESTING INTERCEPTS

