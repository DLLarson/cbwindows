// FrmDockMark.cpp - container window for the marker palette.
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
#include "FrmDockMark.h"
#include "PalMark.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////

BEGIN_MESSAGE_MAP(CDockMarkPalette, CDockablePane)
    ON_WM_DESTROY()
    ON_WM_SIZE()
	ON_WM_WINDOWPOSCHANGED()  // TEST INTERCEPT
END_MESSAGE_MAP()

IMPLEMENT_DYNAMIC(CDockMarkPalette, CDockablePane);

/////////////////////////////////////////////////////////////////////////////

CDockMarkPalette::CDockMarkPalette()
{
    m_pChildWnd = NULL;
}

CDockMarkPalette::~CDockMarkPalette()
{
}

/////////////////////////////////////////////////////////////////////////////

void CDockMarkPalette::SetChild(CMarkerPalette* pChildWnd)
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
// CDockMarkPalette message handlers

void CDockMarkPalette::OnSize(UINT nType, int cx, int cy)
{
    ATLTRACE2(traceAppMsg, 0, "CDockMarkPalette::OnSize(nType=%u, cx=%d, cy=%d) called...\n", nType, cx, cy);
    CDockablePane::OnSize(nType, cx, cy);
    CRect rct;
    GetClientRect(rct);
    if (m_pChildWnd != NULL)
        m_pChildWnd->MoveWindow(&rct);
    ATLTRACE2(traceAppMsg, 0, "...CDockMarkPalette::OnSize() returns nothing.\n");
}

void CDockMarkPalette::OnDestroy()
{
    m_pChildWnd = NULL;
    CDockablePane::OnDestroy();
}

/// TESTING INTERCEPTS...
CSize CDockMarkPalette::CalcFixedLayout(BOOL bStretch, BOOL bHorz)
{
    ATLTRACE2(traceAppMsg, 0, "CDockMarkPalette::CalcFixedLayout(bStretch=%u, bHorz=%d) called...\n", bStretch, bHorz);
    CSize ret = CDockablePane::CalcFixedLayout(bStretch, bHorz);
    ATLTRACE2(traceAppMsg, 0, "...CDockMarkPalette::CalcFixedLayout(...) returns CSize(%d, %d).\n", ret.cx, ret.cy);
    return ret;
    //return CDockablePane::CalcFixedLayout(bStretch, bHorz);
}

BOOL CDockMarkPalette::IsResizable() const
{
    ATLTRACE2(traceAppMsg, 0, "CDockMarkPalette::IsResizable() called...\n");
    BOOL bRet = CDockablePane::IsResizable();
    ATLTRACE2(traceAppMsg, 0, "...CDockMarkPalette::IsResizable() returns BOOL(%d)\n", bRet);
    return bRet;
    //return CDockablePane::IsResizable();
}

CSize CDockMarkPalette::CalcAvailableSize(CRect rectRequired)
{
    ATLTRACE2(traceAppMsg, 0, "CDockMarkPalette::CalcAvailableSize(rectRequired={%d, %d, %d, $d}) called...\n",
        rectRequired.top, rectRequired.left, rectRequired.bottom, rectRequired.right);
    CSize ret = CDockablePane::CalcAvailableSize(rectRequired);
    ATLTRACE2(traceAppMsg, 0, "...CDockMarkPalette::CalcAvailableSize(...) returns CSize(%d, %d).\n", ret.cx, ret.cy);
    return ret;
    //return CDockablePane::CalcAvailableSize(rectRequired);
}

CSize CDockMarkPalette::CalcSize(BOOL bVertDock)
{
    ATLTRACE2(traceAppMsg, 0, "CDockMarkPalette::CalcSize(bVertDock=%u) called...\n", bVertDock);
    CSize ret = CDockablePane::CalcSize(bVertDock);
    ATLTRACE2(traceAppMsg, 0, "...CDockMarkPalette::CalcSize(...) returns CSize(%d, %d).\n", ret.cx, ret.cy);
    return ret;
    //return CDockablePane::CalcSize(bVertDock);
}

void CDockMarkPalette::GetMinSize(CSize& size) const
{
    ATLTRACE2(traceAppMsg, 0, "CDockMarkPalette::GetMinSize(size=CSize(%d, %d)) called...\n", size.cx, size.cy);
    CDockablePane::GetMinSize(size);
    ATLTRACE2(traceAppMsg, 0, "...CDockMarkPalette::GetMinSize(...) returns CSize(%d, %d).\n", size.cx, size.cy);
}

void CDockMarkPalette::RecalcLayout()
{
    ATLTRACE2(traceAppMsg, 0, "CDockMarkPalette::RecalcLayout called...\n");
    CDockablePane::RecalcLayout();
    ATLTRACE2(traceAppMsg, 0, "...CDockMarkPalette::RecalcLayout returns.\n");
}

void CDockMarkPalette::AdjustLayout()
{
    ATLTRACE2(traceAppMsg, 0, "CDockMarkPalette::AdjustLayout called...\n");
    CDockablePane::AdjustLayout();
    ATLTRACE2(traceAppMsg, 0, "...CDockMarkPalette::AdjustLayout returns.\n");
}

void CDockMarkPalette::OnWindowPosChanged(WINDOWPOS FAR* lpwndpos)
{
    ATLTRACE2(traceAppMsg, 0, "CDockMarkPalette::OnWindowPosChanged(x=%d, y=%d, cx=%d, cy=%d) called...\n", 
        lpwndpos->x, lpwndpos->y, lpwndpos->cx, lpwndpos->cy);
    if (lpwndpos->cx > 200 || lpwndpos->cy < 100)
        TRACE0("SOMETHING BAD HAPPENED...\n");
    CDockablePane::OnWindowPosChanged(lpwndpos);
    ATLTRACE2(traceAppMsg, 0, "...CDockMarkPalette::OnWindowPosChanged returns.\n");
}

/// ...TESTING INTERCEPTS

