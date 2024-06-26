// DlgTilsz.h : header file
//
// Copyright (c) 1994-2024 By Dale L. Larson & William Su, All Rights Reserved.
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

/////////////////////////////////////////////////////////////////////////////
// CResizeTileDialog dialog

class CBoardManager;

class CResizeTileDialog : public wxDialog
{
// Construction
public:
    CResizeTileDialog(wxWindow* parent = &CB::GetMainWndWx());    // standard constructor

// Dialog Data
private:
    CB_XRC_BEGIN_CTRLS_DECL()
        RefPtr<wxTextCtrl> m_editHeight;
        RefPtr<wxTextCtrl> m_editWidth;
        RefPtr<wxStaticText> m_staticCurWd;
        RefPtr<wxStaticText> m_staticCurHt;
        RefPtr<wxChoice> m_comboBrdName;
        RefPtr<wxCheckBox> m_chkRescaleBMaps;
    CB_XRC_END_CTRLS_DECL()

public:
    bool    m_bRescaleBMaps;
    UINT    m_nWidth;
    UINT    m_nHeight;

    UINT    m_nHalfHeight;          // If 0, caller computes half size
    UINT    m_nHalfWidth;           // If 0, caller computes half size
    CBoardManager* m_pBMgr;

// Implementation
protected:
    bool TransferDataToWindow() override;
    void OnSelchangeBrdName(wxCommandEvent& event);
    void OnChangeTileWd(wxCommandEvent& event);
    void OnChangeTileHt(wxCommandEvent& event);
    bool TransferDataFromWindow() override;
#if 0
    afx_msg BOOL OnHelpInfo(HELPINFO* pHelpInfo);
    afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
#endif
    wxDECLARE_EVENT_TABLE();
};
