// DlgSvisi.h : header file
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
// CSetScaleVisibilityDialog dialog

class CSetScaleVisibilityDialog : public wxDialog
{
// Construction
public:
    CSetScaleVisibilityDialog(wxWindow* parent = &CB::GetMainWndWx());    // standard constructor

// Dialog Data
private:
    CB_XRC_BEGIN_CTRLS_DECL()
        RefPtr<wxCheckBox> m_chkNaturalScale;
        RefPtr<wxCheckBox> m_chkSmallScale;
        RefPtr<wxCheckBox> m_chkHalfScale;
        RefPtr<wxCheckBox> m_chkFullScale;
    CB_XRC_END_CTRLS_DECL()

public:
    bool    m_bFullScale;
    bool    m_bHalfScale;
    bool    m_bSmallScale;
    bool    m_bNaturalScale;

// Implementation
protected:
    bool TransferDataFromWindow() override;
#if 0
    afx_msg BOOL OnHelpInfo(HELPINFO* pHelpInfo);
    afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
#endif
    bool TransferDataToWindow() override;
    void OnClickHalf(wxCommandEvent& event);
    void OnClickHalf()
    {
        wxCommandEvent dummy;
        OnClickHalf(dummy);
    }
    void OnClickFull(wxCommandEvent& event);
    void OnClickFull()
    {
        wxCommandEvent dummy;
        OnClickFull(dummy);
    }
    void OnClickSmall(wxCommandEvent& event);
    void OnClickSmall()
    {
        wxCommandEvent dummy;
        OnClickSmall(dummy);
    }
    void OnClickNatural(wxCommandEvent& event);
    void OnClickNatural()
    {
        wxCommandEvent dummy;
        OnClickNatural(dummy);
    }
    wxDECLARE_EVENT_TABLE();
};
