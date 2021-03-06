// MoveMgr.cpp
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

#include    "stdafx.h"
#include    "Gp.h"
#include    "GamDoc.h"
#include    "GMisc.h"
#include    "PBoard.h"
#include    "PPieces.h"
#include    "Trays.h"
#include    "DrawObj.h"
#include    "GamState.h"

#include    "MoveMgr.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

/////////////////////////////////////////////////////////////////////
// Delay between submoves

const int stepDelay = 800;          // 800 milliseconds

/////////////////////////////////////////////////////////////////////
// CMoveRecord methods....

void CMoveRecord::Serialize(CArchive& ar)
{
    // The type code is stored seperately and are reconstituted
    // by the object's constructor.
    if (ar.IsStoring())
        ar << (short)m_nSeqNum;
    else
    {
        short sTmp;
        ar >> sTmp; m_nSeqNum = (int)sTmp;
    }
}

/////////////////////////////////////////////////////////////////////
// CBoardPieceMove methods....

CBoardPieceMove::CBoardPieceMove(BoardID nBrdSerNum, PieceID pid, CPoint pnt,
    PlacePos ePos)
{
    m_eType = mrecPMove;
    // ------- //
    m_nBrdNum = nBrdSerNum;
    m_pid = pid;
    m_ptCtr = pnt;
    m_ePos = ePos;
}

BOOL CBoardPieceMove::ValidatePieces(CGamDoc* pDoc)
{
#ifdef _DEBUG
    BOOL bUsed = pDoc->GetPieceTable()->IsPieceUsed(m_pid);
    if (!bUsed)
        TRACE1("CBoardPieceMove::ValidatePieces - Piece %u not in piece table.\n", value_preserving_cast<UINT>(static_cast<WORD>(m_pid)));
    return bUsed;
#else
    return pDoc->GetPieceTable()->IsPieceUsed(m_pid);
#endif
}

void CBoardPieceMove::DoMoveSetup(CGamDoc* pDoc, int nMoveWithinGroup)
{
    CPlayBoard* pPBoard;
    CTraySet* pTray;
    CPieceObj* pObj;

    if (pDoc->FindPieceCurrentLocation(m_pid, pTray, pPBoard, &pObj))
    {
        CRect rct = pObj->GetRect();
        CPoint ptCtr = GetMidRect(rct);
        pDoc->EnsureBoardLocationVisible(*pPBoard, ptCtr);
        CPlayBoard* pPBrdDest = pDoc->GetPBoardManager()->
            GetPBoardBySerial(m_nBrdNum);
        ASSERT(pPBrdDest);
        pDoc->IndicateBoardToBoardPieceMove(pPBoard, pPBrdDest, ptCtr, m_ptCtr,
            pObj->GetRect().Size());
    }
    else
        pDoc->SelectTrayItem(*pTray, m_pid);
}

void CBoardPieceMove::DoMove(CGamDoc* pDoc, int nMoveWithinGroup)
{
    CPlayBoard* pPBrdFrom;
    CTraySet* pTrayFrom;
    CPieceObj* pObj;

    CPlayBoard* pPBrdDest = pDoc->GetPBoardManager()->
        GetPBoardBySerial(m_nBrdNum);
    ASSERT(pPBrdDest);

    pDoc->EnsureBoardLocationVisible(*pPBrdDest, m_ptCtr);

    if (pDoc->FindPieceCurrentLocation(m_pid, pTrayFrom, pPBrdFrom, &pObj))
    {
        CRect rct = pObj->GetRect();
        CSize size = m_ptCtr - GetMidRect(rct);
        pDoc->PlaceObjectOnBoard(pPBrdDest, pObj, size, m_ePos);
    }
    else
    {
        pDoc->PlacePieceOnBoard(m_ptCtr, m_pid, pPBrdDest);
        VERIFY(pDoc->FindPieceOnBoard(m_pid, &pObj) != NULL);

        CRect rct = pObj->GetRect();
        CPoint ptCtr = GetMidRect(rct);
        pDoc->IndicateBoardPiece(pPBrdDest, ptCtr, rct.Size());
    }
    pDoc->SelectObjectOnBoard(*pPBrdDest, pObj);
}

void CBoardPieceMove::DoMoveCleanup(CGamDoc* pDoc, int nMoveWithinGroup)
{
}

void CBoardPieceMove::Serialize(CArchive& ar)
{
    CMoveRecord::Serialize(ar);
    if (ar.IsStoring())
    {
        ar << m_pid;
        ar << m_nBrdNum;
        ar << (short)m_ePos;
        ar << (short)m_ptCtr.x;
        ar << (short)m_ptCtr.y;
    }
    else
    {
        short sTmp;
        ar >> m_pid;
        ar >> m_nBrdNum;
        ar >> sTmp; m_ePos = (PlacePos)sTmp;
        ar >> sTmp; m_ptCtr.x = sTmp;
        ar >> sTmp; m_ptCtr.y = sTmp;
    }
}

#ifdef _DEBUG
void CBoardPieceMove::DumpToTextFile(CFile& file)
{
    char szBfr[256];
    wsprintf(szBfr, "    board = %d, pos = %d, pid = %d, @(%d, %d)\r\n",
        value_preserving_cast<int>(static_cast<WORD>(m_nBrdNum)), m_ePos, value_preserving_cast<int>(static_cast<WORD>(m_pid)), m_ptCtr.x, m_ptCtr.y);
    file.Write(szBfr, lstrlen(szBfr));
}
#endif

/////////////////////////////////////////////////////////////////////
// CTrayPieceMove methods....

CTrayPieceMove::CTrayPieceMove(size_t nTrayNum, PieceID pid, size_t nPos)
{
    m_eType = mrecTMove;
    // ------- //
    m_nTrayNum = nTrayNum;
    m_pid = pid;
    m_nPos = nPos;
}

BOOL CTrayPieceMove::ValidatePieces(CGamDoc* pDoc)
{
#ifdef _DEBUG
    BOOL bUsed = pDoc->GetPieceTable()->IsPieceUsed(m_pid);
    if (!bUsed)
        TRACE1("CTrayPieceMove::ValidatePieces - Piece %u not in piece table.\n", value_preserving_cast<UINT>(static_cast<WORD>(m_pid)));
    return bUsed;
#else
    return pDoc->GetPieceTable()->IsPieceUsed(m_pid);
#endif
}

void CTrayPieceMove::DoMoveSetup(CGamDoc* pDoc, int nMoveWithinGroup)
{
    CPlayBoard* pPBoard;
    CTraySet* pTray;
    CPieceObj* pObj;

    if (pDoc->FindPieceCurrentLocation(m_pid, pTray, pPBoard, &pObj))
    {
        CRect rct = pObj->GetRect();
        CPoint ptCtr = GetMidRect(rct);
        pDoc->EnsureBoardLocationVisible(*pPBoard, ptCtr);
        pDoc->IndicateBoardPiece(pPBoard, ptCtr, rct.Size());
    }
    else
        pDoc->SelectTrayItem(*pTray, m_pid);
}

void CTrayPieceMove::DoMove(CGamDoc* pDoc, int nMoveWithinGroup)
{
    CTraySet& pYGrp = pDoc->GetTrayManager()->GetTraySet(m_nTrayNum);
    pDoc->PlacePieceInTray(m_pid, pYGrp, m_nPos);
}

void CTrayPieceMove::DoMoveCleanup(CGamDoc* pDoc, int nMoveWithinGroup)
{
    CTraySet& pYGrp = pDoc->GetTrayManager()->GetTraySet(m_nTrayNum);
    pDoc->SelectTrayItem(pYGrp, m_pid);
}

void CTrayPieceMove::Serialize(CArchive& ar)
{
    CMoveRecord::Serialize(ar);
    if (ar.IsStoring())
    {
        ar << m_pid;
        ar << value_preserving_cast<int16_t>(m_nTrayNum);
        ASSERT(m_nPos == Invalid_v<size_t> ||
                m_nPos < size_t(int16_t(-1)));
        ar << (m_nPos == Invalid_v<size_t> ? int16_t(-1) : value_preserving_cast<int16_t>(m_nPos));
    }
    else
    {
        int16_t sTmp;
        ar >> m_pid;
        ar >> sTmp; m_nTrayNum = value_preserving_cast<size_t>(sTmp);
        ar >> sTmp; m_nPos = sTmp == int16_t(-1) ? Invalid_v<size_t> : value_preserving_cast<size_t>(sTmp);
    }
}

#ifdef _DEBUG
void CTrayPieceMove::DumpToTextFile(CFile& file)
{
    char szBfr[256];
    wsprintf(szBfr, "    tray = %zu, nPos = %zu, pid = %u\r\n",
        m_nTrayNum, m_nPos, static_cast<WORD>(m_pid));
    file.Write(szBfr, lstrlen(szBfr));
}
#endif

/////////////////////////////////////////////////////////////////////
// CPieceSetSide methods....

BOOL CPieceSetSide::ValidatePieces(CGamDoc* pDoc)
{
#ifdef _DEBUG
    BOOL bUsed = pDoc->GetPieceTable()->IsPieceUsed(m_pid);
    if (!bUsed)
        TRACE1("CPieceSetSide::ValidatePieces - Piece %u not in piece table.\n", value_preserving_cast<UINT>(static_cast<WORD>(m_pid)));
    return bUsed;
#else
    return pDoc->GetPieceTable()->IsPieceUsed(m_pid);
#endif
}

BOOL CPieceSetSide::IsMoveHidden(CGamDoc* pDoc, int nMoveWithinGroup)
{
    CPlayBoard* pPBoard;
    CTraySet* pTray;
    CPieceObj* pObj;

    if (pDoc->GetPieceTable()->IsOwnedButNotByCurrentPlayer(m_pid, pDoc))
        return TRUE;

    if (pDoc->FindPieceCurrentLocation(m_pid, pTray, pPBoard, &pObj))
    {
        if (pPBoard->IsOwnedButNotByCurrentPlayer(pDoc))
            return TRUE;
    }
    else
    {
        if (pTray->IsOwnedButNotByCurrentPlayer(pDoc))
            return TRUE;
    }
    return FALSE;
}

void CPieceSetSide::DoMoveSetup(CGamDoc* pDoc, int nMoveWithinGroup)
{
    CPlayBoard* pPBoard;
    CTraySet* pTray;
    CPieceObj* pObj;

    if (pDoc->FindPieceCurrentLocation(m_pid, pTray, pPBoard, &pObj))
    {
        CRect rct = pObj->GetRect();
        CPoint ptCtr = GetMidRect(rct);
        pDoc->EnsureBoardLocationVisible(*pPBoard, ptCtr);
        pDoc->IndicateBoardPiece(pPBoard, ptCtr, rct.Size());
        pDoc->SelectObjectOnBoard(*pPBoard, pObj);
    }
    else
        pDoc->SelectTrayItem(*pTray, m_pid);
}

void CPieceSetSide::DoMove(CGamDoc* pDoc, int nMoveWithinGroup)
{
    CPlayBoard* pPBoard;
    CTraySet* pTray;
    CPieceObj* pObj;

    if (pDoc->FindPieceCurrentLocation(m_pid, pTray, pPBoard, &pObj))
        pDoc->InvertPlayingPieceOnBoard(*pObj, pPBoard);
    else
        pDoc->InvertPlayingPieceInTray(m_pid);
}

void CPieceSetSide::Serialize(CArchive& ar)
{
    CMoveRecord::Serialize(ar);
    if (ar.IsStoring())
    {
        ar << m_pid;
        ar << (BYTE)m_bTopUp;
    }
    else
    {
        ar >> m_pid;
        BYTE cTmp;
        ar >> cTmp; m_bTopUp = (BOOL)cTmp;
    }
}

#ifdef _DEBUG
void CPieceSetSide::DumpToTextFile(CFile& file)
{
    char szBfr[256];
    wsprintf(szBfr, "    Piece %d is set to %s visible.\r\n",
        value_preserving_cast<int>(static_cast<WORD>(m_pid)), m_bTopUp ? "top" : "bottom");
    file.Write(szBfr, lstrlen(szBfr));
}
#endif

/////////////////////////////////////////////////////////////////////
// CPieceSetFacing methods....

BOOL CPieceSetFacing::ValidatePieces(CGamDoc* pDoc)
{
#ifdef _DEBUG
    BOOL bUsed = pDoc->GetPieceTable()->IsPieceUsed(m_pid);
    if (!bUsed)
        TRACE1("CPieceSetFacing::ValidatePieces - Piece %u not in piece table.\n", value_preserving_cast<UINT>(static_cast<WORD>(m_pid)));
    return bUsed;
#else
    return pDoc->GetPieceTable()->IsPieceUsed(m_pid);
#endif
}

void CPieceSetFacing::DoMoveSetup(CGamDoc* pDoc, int nMoveWithinGroup)
{
    CPlayBoard* pPBoard;
    CTraySet* pTray;
    CPieceObj* pObj;

    if (pDoc->FindPieceCurrentLocation(m_pid, pTray, pPBoard, &pObj))
    {
        CRect rct = pObj->GetRect();
        CPoint ptCtr = GetMidRect(rct);
        pDoc->EnsureBoardLocationVisible(*pPBoard, ptCtr);
        pDoc->IndicateBoardPiece(pPBoard, ptCtr, rct.Size());
        pDoc->SelectObjectOnBoard(*pPBoard, pObj);
    }
    else
        pDoc->SelectTrayItem(*pTray, m_pid);
}

void CPieceSetFacing::DoMove(CGamDoc* pDoc, int nMoveWithinGroup)
{
    CPlayBoard* pPBoard;
    CTraySet* pTray;
    CPieceObj* pObj;

    if (pDoc->FindPieceCurrentLocation(m_pid, pTray, pPBoard, &pObj))
        pDoc->ChangePlayingPieceFacingOnBoard(*pObj, pPBoard, m_nFacingDegCW);
    else
        pDoc->ChangePlayingPieceFacingInTray(m_pid, m_nFacingDegCW);
}

void CPieceSetFacing::Serialize(CArchive& ar)
{
    CMoveRecord::Serialize(ar);
    if (ar.IsStoring())
    {
        ar << m_pid;
        ar << (WORD)m_nFacingDegCW;
    }
    else
    {
        WORD wTmp;
        ar >> m_pid;
        if (CGamDoc::GetLoadingVersion() < NumVersion(2, 90))   //Ver2.90
        {
            BYTE cTmp;
            ar >> cTmp;
            m_nFacingDegCW = 5 * (int)cTmp;
        }
        else
        {
            ar >> wTmp;
            m_nFacingDegCW = (int)wTmp;
        }
    }
}

#ifdef _DEBUG
void CPieceSetFacing::DumpToTextFile(CFile& file)
{
    char szBfr[256];
    wsprintf(szBfr, "    Piece %d is rotated %d degrees.\r\n", value_preserving_cast<int>(static_cast<WORD>(m_pid)), m_nFacingDegCW);
    file.Write(szBfr, lstrlen(szBfr));
}
#endif

/////////////////////////////////////////////////////////////////////
// CPieceSetFacing methods....

BOOL CPieceSetOwnership::ValidatePieces(CGamDoc* pDoc)
{
#ifdef _DEBUG
    BOOL bUsed = pDoc->GetPieceTable()->IsPieceUsed(m_pid);
    if (!bUsed)
        TRACE1("CPieceSetOwnership::ValidatePieces - Piece %u not in piece table.\n", value_preserving_cast<UINT>(static_cast<WORD>(m_pid)));
    return bUsed;
#else
    return pDoc->GetPieceTable()->IsPieceUsed(m_pid);
#endif
}

void CPieceSetOwnership::DoMove(CGamDoc* pDoc, int nMoveWithinGroup)
{
    CPlayBoard* pPBoard;
    CTraySet* pTray;
    CPieceObj* pObj;

    if (pDoc->FindPieceCurrentLocation(m_pid, pTray, pPBoard, &pObj))
    {
        CRect rct = pObj->GetRect();
        CPoint ptCtr = GetMidRect(rct);
        pDoc->EnsureBoardLocationVisible(*pPBoard, ptCtr);
        pDoc->IndicateBoardPiece(pPBoard, ptCtr, rct.Size());
        pDoc->SelectObjectOnBoard(*pPBoard, pObj);
    }
    else
        pDoc->SelectTrayItem(*pTray, m_pid);
    pDoc->SetPieceOwnership(m_pid, m_dwOwnerMask);
}

void CPieceSetOwnership::Serialize(CArchive& ar)
{
    CMoveRecord::Serialize(ar);
    if (ar.IsStoring())
    {
        ar << m_pid;
        ar << m_dwOwnerMask;
    }
    else
    {
        WORD wTmp;
        ar >> m_pid;
        if (CGamDoc::GetLoadingVersion() < NumVersion(3, 10))
        {
            ar >> wTmp;
            m_dwOwnerMask = UPGRADE_OWNER_MASK(wTmp);
        }
        else
            ar >> m_dwOwnerMask;
    }
}

#ifdef _DEBUG
void CPieceSetOwnership::DumpToTextFile(CFile& file)
{
    char szBfr[256];
    wsprintf(szBfr, "    Piece %d has ownership changed to 0x%X.\r\n",
        value_preserving_cast<int>(static_cast<WORD>(m_pid)), m_dwOwnerMask);
    file.Write(szBfr, lstrlen(szBfr));
}
#endif

/////////////////////////////////////////////////////////////////////
// CMarkerSetFacing methods....

CMarkerSetFacing::CMarkerSetFacing(ObjectID dwObjID, MarkID mid, int nFacingDegCW)
{
    m_eType = mrecMFacing;
    m_dwObjID = dwObjID;
    m_mid = mid;
    m_nFacingDegCW = nFacingDegCW;
}

void CMarkerSetFacing::DoMoveSetup(CGamDoc* pDoc, int nMoveWithinGroup)
{
    CDrawObj* pObj;
    CPlayBoard* pPBoard = pDoc->FindObjectOnBoard(m_dwObjID, &pObj);

    if (pPBoard != NULL)
    {
        CRect rct = pObj->GetRect();
        CPoint ptCtr = GetMidRect(rct);
        pDoc->EnsureBoardLocationVisible(*pPBoard, ptCtr);
        pDoc->IndicateBoardPiece(pPBoard, ptCtr, rct.Size());
        pDoc->SelectObjectOnBoard(*pPBoard, pObj);
    }
    else
        ASSERT(FALSE);          // SHOULDN'T HAPPEN
}

void CMarkerSetFacing::DoMove(CGamDoc* pDoc, int nMoveWithinGroup)
{
    CDrawObj* pObj;
    CPlayBoard* pPBoard = pDoc->FindObjectOnBoard(m_dwObjID, &pObj);

    if (pPBoard != NULL)
        pDoc->ChangeMarkerFacingOnBoard(*static_cast<CMarkObj*>(pObj), pPBoard, m_nFacingDegCW);
    else
        ASSERT(FALSE);          // SHOULDN'T HAPPEN
}

void CMarkerSetFacing::Serialize(CArchive& ar)              // VER2.0 is first time used
{
    CMoveRecord::Serialize(ar);
    if (ar.IsStoring())
    {
        ar << m_dwObjID;
        ar << m_mid;
        ar << value_preserving_cast<WORD>(m_nFacingDegCW);
    }
    else
    {
        ar >> m_dwObjID;
        WORD wTmp;
        ar >> m_mid;
        ar >> wTmp; m_nFacingDegCW = value_preserving_cast<int>(wTmp);
        if (CGamDoc::GetLoadingVersion() < NumVersion(2, 90))   //Ver2.90
            m_nFacingDegCW *= 5;                                // Convert old values to degrees
    }
}

#ifdef _DEBUG
void CMarkerSetFacing::DumpToTextFile(CFile& file)
{
    char szBfr[256];
    wsprintf(szBfr, "    Marker dwObjID = %lX, mid = %d is rotated %d degrees.\r\n",
        m_dwObjID, value_preserving_cast<int>(static_cast<WORD>(m_mid)), m_nFacingDegCW);
    file.Write(szBfr, lstrlen(szBfr));
}
#endif

/////////////////////////////////////////////////////////////////////
// CBoardMarkerMove methods....

CBoardMarkerMove::CBoardMarkerMove(BoardID nBrdSerNum, ObjectID dwObjID, MarkID mid,
    CPoint pnt, PlacePos ePos)
{
    m_eType = mrecMMove;
    // ------- //
    m_nBrdNum = nBrdSerNum;
    m_dwObjID = dwObjID;
    m_mid = mid;
    m_ptCtr = pnt;
    m_ePos = ePos;
}

void CBoardMarkerMove::DoMoveSetup(CGamDoc* pDoc, int nMoveWithinGroup)
{
    CDrawObj* pObj;
    CPlayBoard* pPBoard = pDoc->FindObjectOnBoard(m_dwObjID, &pObj);

    if (pPBoard != NULL)
    {
        CRect rct = pObj->GetRect();
        CPoint ptCtr = GetMidRect(rct);
        pDoc->EnsureBoardLocationVisible(*pPBoard, ptCtr);
        CPlayBoard* pPBrdDest = pDoc->GetPBoardManager()->
            GetPBoardBySerial(m_nBrdNum);
        ASSERT(pPBrdDest != NULL);
        pDoc->IndicateBoardToBoardPieceMove(pPBoard, pPBrdDest, ptCtr, m_ptCtr,
            pObj->GetRect().Size());
    }
    else
        pDoc->SelectMarkerPaletteItem(m_mid);
}

void CBoardMarkerMove::DoMove(CGamDoc* pDoc, int nMoveWithinGroup)
{
    CPlayBoard* pPBrdDest = pDoc->GetPBoardManager()->
        GetPBoardBySerial(m_nBrdNum);
    ASSERT(pPBrdDest != NULL);

    pDoc->EnsureBoardLocationVisible(*pPBrdDest, m_ptCtr);

    CDrawObj* pObj;
    CPlayBoard* pPBrdFrom = pDoc->FindObjectOnBoard(m_dwObjID, &pObj);

    if (pPBrdFrom != NULL)
    {
        CRect rct = pObj->GetRect();
        CSize size = m_ptCtr - GetMidRect(rct);
        pDoc->PlaceObjectOnBoard(pPBrdDest, pObj, size, m_ePos);
    }
    else
    {
        // Need to create since doesn't currently exist
        pObj = &pDoc->CreateMarkerObject(pPBrdDest, m_mid, m_ptCtr, m_dwObjID);

        CRect rct = pObj->GetRect();
        CPoint ptCtr = GetMidRect(rct);
        pDoc->IndicateBoardPiece(pPBrdDest, ptCtr, rct.Size());
    }
}

void CBoardMarkerMove::Serialize(CArchive& ar)
{
    CMoveRecord::Serialize(ar);
    if (ar.IsStoring())
    {
        ar << m_mid;
        ar << m_dwObjID;
        ar << m_nBrdNum;
        ar << (short)m_ePos;
        ar << (short)m_ptCtr.x;
        ar << (short)m_ptCtr.y;
    }
    else
    {
        short sTmp;
        ar >> m_mid;
        ar >> m_dwObjID;
        ar >> m_nBrdNum;
        ar >> sTmp; m_ePos = (PlacePos)sTmp;
        ar >> sTmp; m_ptCtr.x = sTmp;
        ar >> sTmp; m_ptCtr.y = sTmp;
    }
}

#ifdef _DEBUG
void CBoardMarkerMove::DumpToTextFile(CFile& file)
{
    char szBfr[256];
    wsprintf(szBfr, "    board = %d, pos = %d, dwObjID = %lX, mid = %d, @(%d, %d)\r\n",
        value_preserving_cast<int>(static_cast<WORD>(m_nBrdNum)), m_ePos, m_dwObjID, value_preserving_cast<int>(static_cast<WORD>(m_mid)), m_ptCtr.x, m_ptCtr.y);
    file.Write(szBfr, lstrlen(szBfr));
}
#endif

/////////////////////////////////////////////////////////////////////
// CObjectDelete methods....

CObjectDelete::CObjectDelete(ObjectID dwObjID)
{
    m_eType = mrecDelObj;
    m_dwObjID = dwObjID;
}

void CObjectDelete::DoMoveSetup(CGamDoc* pDoc, int nMoveWithinGroup)
{
    CDrawObj* pObj;
    CPlayBoard* pPBoard = pDoc->FindObjectOnBoard(m_dwObjID, &pObj);

    if (pPBoard != NULL)
    {
        CRect rct = pObj->GetRect();
        CPoint ptCtr = GetMidRect(rct);
        pDoc->EnsureBoardLocationVisible(*pPBoard, ptCtr);
        pDoc->IndicateBoardPiece(pPBoard, ptCtr, pObj->GetRect().Size());
    }
}

void CObjectDelete::DoMove(CGamDoc* pDoc, int nMoveWithinGroup)
{
    CDrawObj* pObj;
    CPlayBoard* pPBoard = pDoc->FindObjectOnBoard(m_dwObjID, &pObj);

    if (pPBoard != NULL)
    {
        std::vector<CB::not_null<CDrawObj*>> list;
        list.push_back(pObj);
        pDoc->DeleteObjectsInTable(list);
    }
}

void CObjectDelete::Serialize(CArchive& ar)
{
    CMoveRecord::Serialize(ar);
    if (ar.IsStoring())
        ar << m_dwObjID;
    else
        ar >> m_dwObjID;
}

#ifdef _DEBUG
void CObjectDelete::DumpToTextFile(CFile& file)
{
    char szBfr[256];
    wsprintf(szBfr, "    dwObjID = %lX\r\n", m_dwObjID);
    file.Write(szBfr, lstrlen(szBfr));
}
#endif

/////////////////////////////////////////////////////////////////////
// CObjectSetText methods....

CObjectSetText::CObjectSetText(GameElement elem, LPCTSTR pszText)
{
    m_eType = mrecSetObjText;
    m_elem = elem;
    if (pszText != NULL)
        m_strObjText = pszText;
}

BOOL CObjectSetText::IsMoveHidden(CGamDoc* pDoc, int nMoveWithinGroup)
{
    CPlayBoard* pPBoard = NULL;
    CTraySet* pTray = NULL;
    CDrawObj* pObj = NULL;
    CPieceObj* pPObj = NULL;

    if (IsGameElementAPiece(m_elem))
    {
        PieceID pid = GetPieceIDFromElement(m_elem);
        if (pDoc->GetPieceTable()->IsOwnedButNotByCurrentPlayer(pid, pDoc))
            return TRUE;

        pDoc->FindPieceCurrentLocation(pid, pTray, pPBoard, &pPObj);
    }
    else
        pPBoard = pDoc->FindObjectOnBoard(static_cast<ObjectID>(m_elem), &pObj);
    if (pPBoard != NULL)
    {
        if (pPBoard->IsOwnedButNotByCurrentPlayer(pDoc))
            return TRUE;
    }
    else if (pTray != NULL)
    {
        if (pTray->IsOwnedButNotByCurrentPlayer(pDoc))
            return TRUE;
    }
    return FALSE;
}

void CObjectSetText::DoMove(CGamDoc* pDoc, int nMoveWithinGroup)
{
    CPlayBoard* pPBoard;
    CTraySet* pTray;
    CDrawObj* pObj;
    CPieceObj* pPObj;

    pDoc->SetGameElementString(m_elem,
        m_strObjText.IsEmpty() ? NULL : (LPCTSTR)m_strObjText);

    if (IsGameElementAPiece(m_elem))
    {
        if (pDoc->FindPieceCurrentLocation(GetPieceIDFromElement(m_elem),
                pTray, pPBoard, &pPObj))
            pObj = pPObj;
        else
            pDoc->SelectTrayItem(*pTray, GetPieceIDFromElement(m_elem), IDS_TIP_OBJTEXTCHG);
    }
    else
        pPBoard = pDoc->FindObjectOnBoard(static_cast<ObjectID>(m_elem), &pObj);

    if (pPBoard != NULL)
    {
        CRect rct = pObj->GetRect();
        CPoint ptCtr = GetMidRect(rct);
        pDoc->EnsureBoardLocationVisible(*pPBoard, ptCtr);
        pDoc->IndicateBoardPiece(pPBoard, ptCtr, pObj->GetRect().Size());

        // Show a balloon tip so person knows what happened
        if (nMoveWithinGroup == 0)
            pDoc->IndicateTextTipOnBoard(*pPBoard, ptCtr, IDS_TIP_OBJTEXTCHG);
    }
}

void CObjectSetText::Serialize(CArchive& ar)
{
    CMoveRecord::Serialize(ar);
    if (ar.IsStoring())
    {
        ar << m_elem;
        ar << m_strObjText;
    }
    else
    {
        ar >> m_elem;
        ar >> m_strObjText;
    }
}

#ifdef _DEBUG
void CObjectSetText::DumpToTextFile(CFile& file)
{
    char szBfr[256];
    wsprintf(szBfr, "    elem = %lX, text = \"%s\"\r\n", m_elem, (LPCTSTR)m_strObjText);
    file.Write(szBfr, lstrlen(szBfr));
}
#endif

/////////////////////////////////////////////////////////////////////
// CObjectLockdown methods....

CObjectLockdown::CObjectLockdown(GameElement elem, BOOL bLockState)
{
    m_eType = mrecLockObj;
    m_elem = elem;
    m_bLockState = bLockState;
}

BOOL CObjectLockdown::IsMoveHidden(CGamDoc* pDoc, int nMoveWithinGroup)
{
    CDrawObj* pObj;
    CPieceObj* pPObj;
    CPlayBoard* pPBoard;

    if (IsGameElementAPiece(m_elem))
    {
        PieceID pid = GetPieceIDFromElement(m_elem);
        if (pDoc->GetPieceTable()->IsOwnedButNotByCurrentPlayer(pid, pDoc))
            return TRUE;
        pPBoard = pDoc->FindPieceOnBoard(GetPieceIDFromElement(m_elem), &pPObj);
        pObj = pPObj;
    }
    else
        pPBoard = pDoc->FindObjectOnBoard(static_cast<ObjectID>(m_elem), &pObj);

    ASSERT(pObj != NULL);

    if (pPBoard != NULL && pPBoard->IsOwnedButNotByCurrentPlayer(pDoc))
        return TRUE;

    return FALSE;
}

void CObjectLockdown::DoMoveSetup(CGamDoc* pDoc, int nMoveWithinGroup)
{
    CDrawObj* pObj;
    CPieceObj* pPObj;
    CPlayBoard* pPBoard;

    if (IsGameElementAPiece(m_elem))
    {
        pPBoard = pDoc->FindPieceOnBoard(GetPieceIDFromElement(m_elem), &pPObj);
        pObj = pPObj;
    }
    else
        pPBoard = pDoc->FindObjectOnBoard(static_cast<ObjectID>(m_elem), &pObj);


    ASSERT(pObj != NULL);

    if (pObj != NULL)
        pObj->ModifyDObjFlags(dobjFlgLockDown, m_bLockState);

    if (pPBoard != NULL)
    {

        CRect rct = pObj->GetRect();
        CPoint ptCtr = GetMidRect(rct);
        pDoc->EnsureBoardLocationVisible(*pPBoard, ptCtr);
        pDoc->IndicateBoardPiece(pPBoard, ptCtr, pObj->GetRect().Size());
    }
}

void CObjectLockdown::DoMove(CGamDoc* pDoc, int nMoveWithinGroup)
{
    if (nMoveWithinGroup != 0)      // Only do this for the first record in group
        return;

    CDrawObj* pObj;
    CPieceObj* pPObj;
    CPlayBoard* pPBoard;

    if (IsGameElementAPiece(m_elem))
    {
        pPBoard = pDoc->FindPieceOnBoard(GetPieceIDFromElement(m_elem), &pPObj);
        pObj = pPObj;
    }
    else
        pPBoard = pDoc->FindObjectOnBoard(static_cast<ObjectID>(m_elem), &pObj);


    if (pPBoard != NULL)
    {

        CRect rct = pObj->GetRect();
        CPoint ptCtr = GetMidRect(rct);
        pDoc->EnsureBoardLocationVisible(*pPBoard, ptCtr);

        // Show a balloon tip so person knows what happened
        pDoc->IndicateTextTipOnBoard(*pPBoard, ptCtr,
            m_bLockState ? IDS_TIP_OBJLOCKED : IDS_TIP_OBJUNLOCKED);
    }
}

void CObjectLockdown::Serialize(CArchive& ar)
{
    CMoveRecord::Serialize(ar);
    if (ar.IsStoring())
    {
        ar << m_elem;
        ar << (WORD)m_bLockState;
    }
    else
    {
        WORD wTmp;
        ar >> m_elem;
        ar >> wTmp; m_bLockState = (BOOL)wTmp;
    }
}

#ifdef _DEBUG
void CObjectLockdown::DumpToTextFile(CFile& file)
{
    char szBfr[256];
    wsprintf(szBfr, "    elem = %lX, state = %d\r\n", m_elem, m_bLockState);
    file.Write(szBfr, lstrlen(szBfr));
}
#endif

/////////////////////////////////////////////////////////////////////
// CGameStateRcd methods....

void CGameStateRcd::DoMove(CGamDoc* pDoc, int nMoveWithinGroup)
{
    if (!m_pState->RestoreState())
        AfxMessageBox(IDS_ERR_FAILEDSTATECHG, MB_OK | MB_ICONEXCLAMATION);
    if (!pDoc->IsQuietPlayback())
        pDoc->UpdateAllViews(NULL, HINT_GAMESTATEUSED);
}

void CGameStateRcd::Serialize(CArchive& ar)
{
    CMoveRecord::Serialize(ar);
    if (!ar.IsStoring())
    {
        m_pState = MakeOwner<CGameState>((CGamDoc*)ar.m_pDocument);
    }
    ASSERT(m_pState != NULL);
    m_pState->Serialize(ar);
}

#ifdef _DEBUG
void CGameStateRcd::DumpToTextFile(CFile& file)
{
    static char strMsg[] =
        "    To much to dump! (Trust me on this)\r\n";
    file.Write(strMsg, lstrlen(strMsg));
}
#endif

/////////////////////////////////////////////////////////////////////
// CMovePlotList

void CMovePlotList::SavePlotList(CDrawList* pDwg)
{
    m_tblPlot.RemoveAll();
    for (CDrawList::iterator pos = pDwg->begin(); pos != pDwg->end(); ++pos)
    {
        CDrawObj& pObj = **pos;
        if (pObj.GetType() == CDrawObj::drawLine)
        {
            CLine& pLObj = static_cast<CLine&>(pObj);
            m_tblPlot.Add((DWORD)MAKELONG(pLObj.m_ptBeg.x, pLObj.m_ptBeg.y));
            m_tblPlot.Add((DWORD)MAKELONG(pLObj.m_ptEnd.x, pLObj.m_ptEnd.y));
        }
    }
}

void CMovePlotList::DoMoveSetup(CGamDoc* pDoc, int nMoveWithinGroup)
{
    CPlayBoard* pPBrd = pDoc->GetPBoardManager()->GetPBoardBySerial(m_nBrdNum);
    ASSERT(pPBrd != NULL);
    pPBrd->SetPlotMoveMode(TRUE);
    for (int i = 0; i < m_tblPlot.GetSize(); i += 2)
    {
        CPoint ptA(m_tblPlot.GetAt(i));
        CPoint ptB(m_tblPlot.GetAt(i+1));
        pDoc->IndicateBoardPlotLine(pPBrd, ptA, ptB);
    }
}

void CMovePlotList::DoMoveCleanup(CGamDoc* pDoc, int nMoveWithinGroup)
{
    CPlayBoard* pPBrd = pDoc->GetPBoardManager()->GetPBoardBySerial(m_nBrdNum);
    ASSERT(pPBrd != NULL);
    pPBrd->SetPlotMoveMode(FALSE);
}

void CMovePlotList::Serialize(CArchive& ar)
{
    CMoveRecord::Serialize(ar);
    if (ar.IsStoring())
        ar << m_nBrdNum;
    else
    {
        ar >> m_nBrdNum;
    }
    m_tblPlot.Serialize(ar);
}

#ifdef _DEBUG
void CMovePlotList::DumpToTextFile(CFile& file)
{
    static char strMsg[] =
        "    To much to dump! (just plain lazy on this one)\r\n";
    file.Write(strMsg, lstrlen(strMsg));
}
#endif

/////////////////////////////////////////////////////////////////////

void CMessageRcd::DoMove(CGamDoc* pDoc, int nMoveWithinGroup)
{
    pDoc->MsgSetMessageText(m_strMsg);
}

void CMessageRcd::Serialize(CArchive& ar)
{
    CMoveRecord::Serialize(ar);
    if (ar.IsStoring())
        ar << m_strMsg;
    else
        ar >> m_strMsg;
}

#ifdef _DEBUG
void CMessageRcd::DumpToTextFile(CFile& file)
{
    file.Write(m_strMsg, lstrlen(m_strMsg));
    file.Write("\r\n", 2);
}
#endif

/////////////////////////////////////////////////////////////////////

CEventMessageRcd::CEventMessageRcd(CString strMsg,
    BoardID nBoard, int x, int y)
{
    m_eType = mrecEvtMsg;
    m_bIsBoardEvent = TRUE;
    m_nBoard = nBoard;
    m_x = x;
    m_y = y;
    m_strMsg = strMsg;
}

CEventMessageRcd::CEventMessageRcd(CString strMsg,
    size_t nTray, PieceID pieceID)
{
    m_eType = mrecEvtMsg;
    m_bIsBoardEvent = FALSE;
    m_nTray = nTray;
    m_pieceID = pieceID;
    m_strMsg = strMsg;
}

void CEventMessageRcd::DoMoveCleanup(CGamDoc* pDoc, int nMoveWithinGroup)
{
    if (m_bIsBoardEvent)        // Board event message
        pDoc->EventShowBoardNotification(m_nBoard, CPoint(m_x, m_y), m_strMsg);
    else                        // Tray event message
        pDoc->EventShowTrayNotification(m_nTray, m_pieceID, m_strMsg);
}

void CEventMessageRcd::Serialize(CArchive& ar)
{
    CMoveRecord::Serialize(ar);
    if (ar.IsStoring())
    {
        ar << (WORD)m_bIsBoardEvent;
        if (m_bIsBoardEvent)
        {
            ar << m_nBoard;
            ar << value_preserving_cast<DWORD>(m_x);
            ar << value_preserving_cast<DWORD>(m_y);
        }
        else
        {
            ar << value_preserving_cast<WORD>(m_nTray);
            ar << value_preserving_cast<DWORD>(static_cast<WORD>(m_pieceID));
            ar << (DWORD)0;
        }
        ar << m_strMsg;
    }
    else
    {
        WORD wTmp;
        DWORD dwTmp;
        ar >> wTmp; m_bIsBoardEvent = (int)wTmp;
        if (m_bIsBoardEvent)
        {
            ar >> m_nBoard;
            ar >> dwTmp; m_x = value_preserving_cast<int>(dwTmp);
            ar >> dwTmp; m_y = value_preserving_cast<int>(dwTmp);
        }
        else
        {
            ar >> wTmp; m_nTray = value_preserving_cast<size_t>(wTmp);
            ar >> dwTmp; m_pieceID = static_cast<PieceID>(dwTmp);
            ar >> dwTmp;
        }
        ar >> m_strMsg;
    }
}

#ifdef _DEBUG
void CEventMessageRcd::DumpToTextFile(CFile& file)
{
    char szBfr[256];
    if (m_bIsBoardEvent)
    {
        wsprintf(szBfr, "    BoardEvt = %s, nBoard = %d, (x, y) = (%d, %d)\r\n",
                 (LPCTSTR)(m_bIsBoardEvent ? "TRUE" : "FALSE"), value_preserving_cast<int>(static_cast<WORD>(m_nBoard)), m_x, m_y);
    }
    else
    {
        wsprintf(szBfr, "    BoardEvt = %s, nTray = %zu, pieceID = %d\r\n",
                 (LPCTSTR)(m_bIsBoardEvent ? "TRUE" : "FALSE"), m_nTray, value_preserving_cast<int>(static_cast<WORD>(m_pieceID)));
    }
    file.Write(szBfr, lstrlen(szBfr));
}
#endif

/////////////////////////////////////////////////////////////////////

void CCompoundMove::Serialize(CArchive& ar)
{
    CMoveRecord::Serialize(ar);
    if (ar.IsStoring())
        ar << (WORD)m_bGroupBegin;
    else
    {
        WORD wTmp;
        ar >> wTmp; m_bGroupBegin = (BOOL)wTmp;
    }
}

#ifdef _DEBUG
void CCompoundMove::DumpToTextFile(CFile& file)
{
    char szBfr[256];
    wsprintf(szBfr, "    %s Compound Move.\r\n", m_bGroupBegin ? "Begin" : "End");
    file.Write(szBfr, lstrlen(szBfr));
}
#endif

/////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////

CMoveList::CMoveList()
{
    m_nSeqNum = 0;
    m_nSkipCount = 0;
    m_bSkipKeepInd = FALSE;
    m_bCompoundMove = FALSE;
    m_nCompoundBaseIndex = Invalid_v<size_t>;
    m_nPlaybackLock = 0;
    m_pCompoundBaseBookMark = NULL;
    m_bCompoundSingleStep = FALSE;
    m_pStateSave = NULL;
}

CMoveList::~CMoveList()
{
    Clear();
}

/////////////////////////////////////////////////////////////////////
// This is a crappy way to clone a move list but it's easier
// than propagating virtual Clone() methods throughout the
// move list object structure. It's at times like these I wish I
// could just toss it all and start over in a decent language like
// C#. The whole reason for doing this is to patch over side effects
// of switching to in-memory histories.

OwnerPtr<CMoveList> CMoveList::CloneMoveList(CGamDoc* pDoc, CMoveList& pMoveList)
{
    CMemFile memFile;
    CArchive arStore(&memFile, CArchive::store);
    arStore.m_pDocument = pDoc;
    pMoveList.Serialize(arStore, FALSE);
    arStore.Close();

    memFile.SeekToBegin();
    CArchive arLoad(&memFile, CArchive::load);
    arLoad.m_pDocument = pDoc;
    OwnerPtr<CMoveList> pNewMoveList = MakeOwner<CMoveList>();
    pNewMoveList->Serialize(arLoad, FALSE);
    return pNewMoveList;
}

/////////////////////////////////////////////////////////////////////
// CMoveList methods....

BOOL CMoveList::ValidatePieces(CGamDoc* pDoc)
{
    ASSERT(m_nPlaybackLock == 0);
    if (m_nPlaybackLock != 0)
        return FALSE;

    iterator pos;
    for (pos = begin(); pos != end(); )
    {
        CMoveRecord& pRcd = GetNext(pos);
        if (!pRcd.ValidatePieces(pDoc))
            return FALSE;
    }
    return TRUE;
}

bool CMoveList::IsThisMovePossible(size_t nIndex)
{
    return nIndex < size() && nIndex != Invalid_v<size_t>;
}

bool CMoveList::IsWithinCompoundMove(size_t nIndex)
{
    if (nIndex == Invalid_v<size_t>)
        return false;

    iterator posPrev = FindIndex(nIndex);

    // Start looking on previous record.
    if (posPrev != end())
        GetPrev(posPrev);
    while (posPrev != end())
    {
        CMoveRecord& pRcd = GetPrev(posPrev);
        if (pRcd.GetType() == CMoveRecord::mrecCompoundMove)
            return static_cast<CCompoundMove&>(pRcd).IsGroupBegin();
    }
    return false;
}

// Returns the starting move index
size_t CMoveList::SetStartingState()
{
    iterator pos = ++begin();
    ASSERT(pos != end());
    CB::not_null<CMoveRecord*> temp = &GetAt(pos);
    size_t nStartIndex = size_t(2);

    if (temp->GetType() != CMoveRecord::mrecState)
    {
        iterator pos = begin();
        ASSERT(pos != end());
        temp = &GetAt(pos);
        ASSERT(temp->GetType() == CMoveRecord::mrecState); // This *HAS* to be TRUE
        nStartIndex = size_t(1);
    }
    CGameStateRcd& pRcd = static_cast<CGameStateRcd&>(*temp);
    pRcd.GetGameState().RestoreState();
    return nStartIndex;
}

// PushAndSetState saves the current state of the playback and
// sets the playback state to what is should be at the specified
// move index.

void CMoveList::PushAndSetState(CGamDoc* pDoc, size_t nIndex)
{
    ASSERT(m_pStateSave == NULL); // Only one push allowed
    m_pStateSave = new CGameState(pDoc);
    pDoc->FlushAllIndicators();
    m_pStateSave->SaveState();
    m_bQuietPlaybackSave = pDoc->IsQuietPlayback();
    pDoc->SetQuietPlayback(TRUE);

    size_t nCurIndex = SetStartingState();
    if (nCurIndex < nIndex)
    {
        while ((nCurIndex = DoMove(pDoc, nCurIndex)) < nIndex)
            pDoc->FlushAllIndicators();
    }
    pDoc->FlushAllIndicators();
}

// Undoes what PushAndSetState did.
void CMoveList::PopAndRestoreState(CGamDoc* pDoc)
{
    ASSERT(m_pStateSave != NULL); // Better be one!
    pDoc->SetLoadingVersion(NumVersion(fileGsnVerMajor, fileGsnVerMinor));
    m_pStateSave->RestoreState();
    delete m_pStateSave;
    m_pStateSave = NULL;
    pDoc->SetQuietPlayback(m_bQuietPlaybackSave);
}

size_t CMoveList::FindPreviousMove(CGamDoc* pDoc, size_t nIndex)
{
    iterator     posPrev;
    size_t       nCurIndex;

    BOOL bWithinCompoundMove = IsWithinCompoundMove(nIndex);
    if (bWithinCompoundMove && !m_bCompoundSingleStep)
    {
        // This case only happens if compound move single step was on
        // and then turned of while we were stepping WITHIN a
        // compound move group. In this case we simply locate the
        // starting record and return that record number.
        nCurIndex = nIndex;
        posPrev = FindIndex(nCurIndex);
        GetPrev(posPrev);           // Point to previous record.
        while (TRUE)
        {
            CMoveRecord& pRcd = GetPrev(posPrev);
            nCurIndex--;
            if (pRcd.GetType() == CMoveRecord::mrecCompoundMove)
                return nCurIndex;           // Found it.
        }
    }

CHECK_AGAIN:
    if (nIndex != Invalid_v<size_t>)
    {
        nCurIndex = nIndex - size_t(1);
        posPrev = FindIndex(nIndex);
        ASSERT(posPrev != end());
        if (posPrev == end())
            return size_t(0);
        CMoveRecord& pRcd = GetPrev(posPrev);// First GetPrev() gets current record
    }
    else
    {
        // We are past the end of the list. Last record is end
        // of previous move.
        posPrev = --end();
        nCurIndex = size() - size_t(1);
    }
    CB::not_null<CMoveRecord*> pRcd = &GetPrev(posPrev);

    // Another weird special case...If the record is an end of compound
    // move record and we are in single step mode, then step back one more
    // record.
    if (m_bCompoundSingleStep &&
        pRcd->GetType() == CMoveRecord::mrecCompoundMove &&
        !static_cast<CCompoundMove&>(*pRcd).IsGroupBegin())
    {
        pRcd = &GetPrev(posPrev);
        nCurIndex--;
    }

    // Use different search approach depending on whether or not the
    // previous record ended a compound move.

    ASSERT(nCurIndex >= 0);
    if (pRcd->GetType() == CMoveRecord::mrecCompoundMove &&
        !static_cast<CCompoundMove&>(*pRcd).IsGroupBegin() && !m_bCompoundSingleStep)
    {
        // Previous move ended a compound move. Search for
        // starting record of this compound move grouping.
        while (TRUE)
        {
            pRcd = &GetPrev(posPrev);
            nCurIndex--;
            if (pRcd->GetType() == CMoveRecord::mrecCompoundMove)
                break;          // Found it.
        }
    }
    else
    {
        // Search for starting record with this sequence number.
        int nSeqNum = pRcd->GetSeqNum();
        do
        {
            pRcd = &GetPrev(posPrev);
            nCurIndex--;
        }
        while (pRcd->GetSeqNum() == nSeqNum);
        nCurIndex++;
        if ((pRcd->GetType() == CMoveRecord::mrecCompoundMove && m_bCompoundSingleStep))
        {
            nIndex = nCurIndex;
            goto CHECK_AGAIN;       // Back another record
        }
        // If this move is hidden for this player, step back another move
        // and try again.
        PushAndSetState(pDoc, nCurIndex);   // Need to make sure game state is correct
        BOOL bMoveIsHidden = IsMoveHidden(pDoc, nCurIndex);
        PopAndRestoreState(pDoc);

        if (bMoveIsHidden)
        {
            nIndex = nCurIndex;
            goto CHECK_AGAIN;       // Back another record
        }
    }
    ASSERT(nCurIndex != Invalid_v<size_t>);
    return nCurIndex;
}

// Check for hidden operations. If the operation
// has all hidden portions, the entire move will be
// done in 'quiet' mode.

bool CMoveList::IsMoveHidden(CGamDoc* pDoc, size_t nIndex)
{
    iterator posFirst = FindIndex(nIndex);
    ASSERT(posFirst != end());
    if (posFirst == end())
        return false;

    int nGrp = INT_MIN;
    size_t nNextIndex = nIndex;

    iterator pos = posFirst;
    int nElementInGroup = 0;
    while (pos != end())
    {
        CMoveRecord& pRcd = GetNext(pos);
        if (nGrp == INT_MIN)
            nGrp = pRcd.GetSeqNum();
        if (nGrp != pRcd.GetSeqNum())
            break;
        nNextIndex++;                       // Determine for caller
        // For purposes of this scan certain records aren't considered
        // when determining whether a move is entirely hidden.
        if (pRcd.GetType() != CMoveRecord::mrecEvtMsg)
        {
            if (!pRcd.IsMoveHidden(pDoc, nElementInGroup))
                return FALSE;
        }
        nElementInGroup++;
    }
    return TRUE;
}

// This routine plays back the moves for the group at the
// specified index. Returns the index of the next move group.

size_t CMoveList::DoMove(CGamDoc* pDoc, size_t nIndex, BOOL bAutoStepHiddenMove /* = TRUE*/)
{
    ASSERT(m_nPlaybackLock == 0);
    if (m_nPlaybackLock != 0)
        return Invalid_v<size_t>;

    m_nPlaybackLock++;                  // Stop recursion

    size_t nNextIndex = nIndex;

    m_nSkipCount = 0;                   // Make sure we don't have any pent up skips

    do
    {
        // Check at the start of the loop. This causes the next move
        // to play normally if the current move is skipped.
        if (m_nSkipCount > 0)
        {
            m_nSkipCount--;
            if (!m_bSkipKeepInd)
                pDoc->FlushAllIndicators();
        }
        pDoc->FlushAllSelections();

        nNextIndex = nIndex;

        BOOL bCompoundMove = FALSE;
        BOOL bDoNextMove = FALSE;           // Set if hidden move was executed

        do
        {
            bDoNextMove = FALSE;

            if (nIndex >= size() || nIndex == Invalid_v<size_t>)
                break;

            iterator posFirst = FindIndex(nIndex);
            ASSERT(posFirst != end());
            if (posFirst == end())
                break;

            // First check for compound move record...
            CMoveRecord& pRcd = GetAt(posFirst);
            if (pRcd.GetType() == CMoveRecord::mrecCompoundMove)
            {
                GetNext(posFirst);              // Step past record
                nNextIndex++;                   // calc'ed for caller
                if (!static_cast<CCompoundMove&>(pRcd).IsGroupBegin() && !m_bCompoundSingleStep)
                    break;
                if (!m_bCompoundSingleStep)
                    bCompoundMove = TRUE;
            }

            iterator pos = posFirst;
            int  nGrp = INT_MIN;

            if (bCompoundMove)
                pDoc->FlushAllSelections();

            // Check for hidden operations. If the operation
            // has any hidden portions, the entire move will be
            // done in 'quiet' mode.

            BOOL bQuietModeSave = pDoc->IsQuietPlayback();

            if (!pDoc->IsQuietPlayback() && IsMoveHidden(pDoc, nIndex))
            {
                pDoc->SetQuietPlayback(TRUE);
                bDoNextMove = bAutoStepHiddenMove;
            }

            // Call setup routines

            pos = posFirst;
            int nElementInGroup = 0;
            while (pos != end())
            {
                CMoveRecord& pRcd = GetNext(pos);
                if (nGrp == INT_MIN)
                    nGrp = pRcd.GetSeqNum();
                if (nGrp != pRcd.GetSeqNum())
                    break;
                nNextIndex++;                   // Determine for caller
                BOOL bHidden = pRcd.IsMoveHidden(pDoc, nElementInGroup);
                BOOL bTmpQuietSave = pDoc->IsQuietPlayback();
                if (bHidden)
                    pDoc->SetQuietPlayback(TRUE);
                pRcd.DoMoveSetup(pDoc, nElementInGroup);
                if (bHidden)
                    pDoc->SetQuietPlayback(bTmpQuietSave);
                nElementInGroup++;
            }

            // Wait a moment.

            if (!pDoc->IsQuietPlayback())
                GetApp()->Delay(stepDelay, (BOOL*)&m_nSkipCount);

            // Do actual moves

            pos = posFirst;
            nElementInGroup = 0;
            while (pos != end())
            {
                CMoveRecord& pRcd = GetNext(pos);
                if (nGrp != pRcd.GetSeqNum())
                    break;
                BOOL bHidden = pRcd.IsMoveHidden(pDoc, nElementInGroup);
                BOOL bTmpQuietSave = pDoc->IsQuietPlayback();
                if (bHidden)
                    pDoc->SetQuietPlayback(TRUE);
                pRcd.DoMove(pDoc, nElementInGroup);
                if (bHidden)
                    pDoc->SetQuietPlayback(bTmpQuietSave);
                nElementInGroup++;
            }

            // Do move clean up

            pos = posFirst;
            nElementInGroup = 0;
            while (pos != end())
            {
                CMoveRecord& pRcd = GetNext(pos);
                if (nGrp != pRcd.GetSeqNum())
                    break;
                BOOL bHidden = pRcd.IsMoveHidden(pDoc, nElementInGroup);
                BOOL bTmpQuietSave = pDoc->IsQuietPlayback();
                if (bHidden)
                    pDoc->SetQuietPlayback(TRUE);
                pRcd.DoMoveCleanup(pDoc, nElementInGroup);
                if (bHidden)
                    pDoc->SetQuietPlayback(bTmpQuietSave);
                nElementInGroup++;
            }

            // Restore quite mode playback if mode was initially different.
            pDoc->SetQuietPlayback(bQuietModeSave);

            nIndex = nNextIndex >= size() ? Invalid_v<size_t> : nNextIndex;

            // Short delay between moves in compound move.
            if (bCompoundMove && !pDoc->IsQuietPlayback())
                GetApp()->Delay((2 * stepDelay) / 3, (BOOL*)&m_nSkipCount);

        } while (bCompoundMove || bDoNextMove);


        nNextIndex = nNextIndex >= size() ? Invalid_v<size_t> : nNextIndex;

        if (nNextIndex != Invalid_v<size_t> && m_bCompoundSingleStep)
        {
            // If the next record to be executed is a compound move
            // end record AND we are single stepping the compound
            // records, we need to step to the next record.
            iterator pos = FindIndex(nNextIndex);
            CMoveRecord& pRcd = GetAt(pos);
            if (pRcd.GetType() == CMoveRecord::mrecCompoundMove &&
                !static_cast<CCompoundMove&>(pRcd).IsGroupBegin())
            {
                nNextIndex++;
                nNextIndex = nNextIndex >= size() ? Invalid_v<size_t> : nNextIndex;
            }
        }
    } while (m_nSkipCount > 0);

    m_nPlaybackLock--;

    return nNextIndex;
}

CMoveList::iterator CMoveList::AppendMoveRecord(OwnerPtr<CMoveRecord> pRec)
{
    ASSERT(m_nPlaybackLock == 0);
    if (m_nPlaybackLock != 0)
        return end();

    pRec->SetSeqNum(m_nSeqNum);
    push_back(std::move(pRec));
    return --end();
}

CMoveList::iterator CMoveList::PrependMoveRecord(OwnerPtr<CMoveRecord> pRec,
    BOOL bSetSeqNum /* = TRUE */)
{
    ASSERT(m_nPlaybackLock == 0);
    if (m_nPlaybackLock != 0)
        return end();

    if (bSetSeqNum)
        pRec->SetSeqNum(m_nSeqNum);
    push_front(std::move(pRec));
    return begin();
}

void CMoveList::PurgeAfter(size_t nIndex)
{
    ASSERT(m_nPlaybackLock == 0);
    if (m_nPlaybackLock != 0)
        return;

    if (nIndex <= m_nCompoundBaseIndex && m_pCompoundBaseBookMark != NULL)
    {
        if (m_pCompoundBaseBookMark)
        {
            delete m_pCompoundBaseBookMark;
            m_pCompoundBaseBookMark = NULL;
        }
        m_bCompoundMove = FALSE;
        m_nCompoundBaseIndex = Invalid_v<size_t>;
    }

    iterator pos = FindIndex(nIndex);
    if (pos == end())
        return;             // Doesn't exist
    while (pos != end())
    {
        erase(pos++);
    }
}

void CMoveList::Clear()
{
    ASSERT(m_nPlaybackLock == 0);
    if (m_nPlaybackLock != 0)
        return;

    if (m_pCompoundBaseBookMark)
        delete m_pCompoundBaseBookMark;
    m_pCompoundBaseBookMark = NULL;
    m_bCompoundMove = FALSE;
    m_nCompoundBaseIndex = Invalid_v<size_t>;

    m_nSeqNum = 0;
    clear();
}

void CMoveList::BeginRecordingCompoundMove(CGamDoc* pDoc)
{
    ASSERT(m_nPlaybackLock == 0);
    if (m_nPlaybackLock != 0)
        return;

    if (m_bCompoundMove)
    {
        if (m_nCompoundBaseIndex == size() - size_t(1)) // Check if any moves recorded
            return;                             // Nope. Keep current marker record
        EndRecordingCompoundMove();             // Mark end of current block
    }
    ASSERT(!m_bCompoundMove);

    m_pCompoundBaseBookMark = new CGameState(pDoc);
    if (!m_pCompoundBaseBookMark->SaveState())
    {
        // Memory low warning?....
        delete m_pCompoundBaseBookMark;
        m_pCompoundBaseBookMark = NULL;
        return;
    }

    AssignNewMoveGroup();
    m_nCompoundBaseIndex = size();
    AppendMoveRecord(MakeOwner<CCompoundMove>(TRUE));
    m_bCompoundMove = TRUE;
}

void CMoveList::CancelRecordingCompoundMove(CGamDoc* pDoc)
{
    ASSERT(m_nPlaybackLock == 0);
    if (m_nPlaybackLock != 0)
        return;

    if (!m_bCompoundMove)
        return;                                 // Nothing to get rid of

    pDoc->FlushAllIndicators();

    ASSERT(m_pCompoundBaseBookMark != NULL);
    pDoc->SetLoadingVersion(NumVersion(fileGsnVerMajor, fileGsnVerMinor));
    if (!m_pCompoundBaseBookMark->RestoreState())
    {
        // Memory error message should be here
        delete m_pCompoundBaseBookMark;
        m_pCompoundBaseBookMark = NULL;
        return;
    }
    if (m_pCompoundBaseBookMark)
    {
        delete m_pCompoundBaseBookMark;
        m_pCompoundBaseBookMark = NULL;
    }
    m_bCompoundMove = FALSE;
    PurgeAfter(m_nCompoundBaseIndex);
    m_nCompoundBaseIndex = Invalid_v<size_t>;
    pDoc->UpdateAllViews(NULL, HINT_GAMESTATEUSED);
}

void CMoveList::EndRecordingCompoundMove()
{
    ASSERT(m_nPlaybackLock == 0);
    if (m_nPlaybackLock != 0)
        return;

    ASSERT(m_bCompoundMove);
    if (!m_bCompoundMove)
        return;                                 // Nothing to mark the end of

    AssignNewMoveGroup();
    if (m_nCompoundBaseIndex < size() - size_t(1))  // Check if any moves recorded
    {
        AppendMoveRecord(MakeOwner<CCompoundMove>(FALSE));
        m_bCompoundMove = FALSE;
        m_nCompoundBaseIndex = Invalid_v<size_t>;

        if (m_pCompoundBaseBookMark)
        {
            delete m_pCompoundBaseBookMark;
            m_pCompoundBaseBookMark = NULL;
        }
    }
    else
    {
        // Just make believe the compound move never occurred.
        PurgeAfter(m_nCompoundBaseIndex);
    }
}

void CMoveList::Serialize(CArchive& ar, BOOL bSaveUndo)
{
    ASSERT(m_nPlaybackLock == 0);

    if (ar.IsStoring())
    {
        ASSERT(m_pStateSave == NULL); // Should never save with this active!

        ar << (short)m_nSeqNum;
        ar << (WORD)m_bCompoundMove;
        ar << value_preserving_cast<DWORD>(m_nCompoundBaseIndex);
        ar << (BYTE)(m_pCompoundBaseBookMark != NULL ? 1 : 0);
        if (m_pCompoundBaseBookMark)
            m_pCompoundBaseBookMark->Serialize(ar);

        ar << value_preserving_cast<WORD>(size());
        iterator pos;
        for (pos = begin(); pos != end(); )
        {
            CMoveRecord& pRcd = GetNext(pos);
            ar << (short)pRcd.GetType();
            pRcd.Serialize(ar);
        }
    }
    else
    {
        Clear();
        WORD wCount;
        short sTmp;

        ar >> sTmp; m_nSeqNum = (int)sTmp;

        if (CGamDoc::GetLoadingVersion() >= NumVersion(0, 60))
        {
            BYTE  cTmp;
            WORD  wTmp;
            DWORD dwTmp;
            ar >> wTmp; m_bCompoundMove = (BOOL)wTmp;
            ar >> dwTmp; m_nCompoundBaseIndex = value_preserving_cast<size_t>(dwTmp);
            ar >> cTmp;             // Check for a bookmark
            if (cTmp)
            {
                ASSERT(m_pCompoundBaseBookMark == NULL);
                if (m_pCompoundBaseBookMark)
                    delete m_pCompoundBaseBookMark;
                m_pCompoundBaseBookMark = new CGameState((CGamDoc*)ar.m_pDocument);
                m_pCompoundBaseBookMark->Serialize(ar);
            }
        }

        ar >> wCount;
        for (WORD i = 0; i < wCount; i++)
        {
            OwnerOrNullPtr<CMoveRecord> pRcd;
            short sType;
            ar >> sType;
            switch ((CMoveRecord::RcdType)sType)
            {
                case CMoveRecord::mrecState:
                    pRcd = MakeOwner<CGameStateRcd>();
                    break;
                case CMoveRecord::mrecPMove:
                    pRcd = MakeOwner<CBoardPieceMove>();
                    break;
                case CMoveRecord::mrecTMove:
                    pRcd = MakeOwner<CTrayPieceMove>();
                    break;
                case CMoveRecord::mrecPSide:
                    pRcd = MakeOwner<CPieceSetSide>();
                    break;
                case CMoveRecord::mrecPFacing:
                    pRcd = MakeOwner<CPieceSetFacing>();
                    break;
                case CMoveRecord::mrecMMove:
                    pRcd = MakeOwner<CBoardMarkerMove>();
                    break;
                case CMoveRecord::mrecMPlot:
                    pRcd = MakeOwner<CMovePlotList>();
                    break;
                case CMoveRecord::mrecMsg:
                    pRcd = MakeOwner<CMessageRcd>();
                    break;
                case CMoveRecord::mrecDelObj:
                    pRcd = MakeOwner<CObjectDelete>();
                    break;
                case CMoveRecord::mrecCompoundMove:
                    pRcd = MakeOwner<CCompoundMove>();
                    break;
                case CMoveRecord::mrecMFacing:
                    pRcd = MakeOwner<CMarkerSetFacing>();
                    break;
                case CMoveRecord::mrecSetObjText:
                    pRcd = MakeOwner<CObjectSetText>();
                    break;
                case CMoveRecord::mrecLockObj:
                    pRcd = MakeOwner<CObjectLockdown>();
                    break;
                case CMoveRecord::mrecEvtMsg:
                    pRcd = MakeOwner<CEventMessageRcd>();
                    break;
                case CMoveRecord::mrecPOwner:
                    pRcd = MakeOwner<CPieceSetOwnership>();
                    break;
                default:
                    ASSERT(FALSE);
                    AfxThrowArchiveException(CArchiveException::badClass);
            }
            pRcd->Serialize(ar);
            push_back(std::move(pRcd));
            BYTE cUndoFlag;
            if (CGamDoc::GetLoadingVersion() < NumVersion(2, 0))
                ar >> cUndoFlag;        // Eat UNDO flag info (never used)
        }
    }
}

#ifdef _DEBUG

static char *tblTypes[CMoveRecord::mrecMax] =
{
    "UnKnown", "GameStateSnapshot", "PieceToBoardMove", "PieceToTrayMove",
    "PieceSetSide", "MarkerToBoardMove", "MovePlotTrack", "PlayerMessage",
    "DeleteObject", "PieceFacing", "CompoundMove", "MarkerFacing",
    "SetObjectText", "LockObject", "EventMessage", "SetPieceOwnership"
};

void CMoveList::DumpToTextFile(CFile& file)
{
    char szBfr[256];
    wsprintf(szBfr, "Current Move Group: %d\r\n", m_nSeqNum);
    file.Write(szBfr, lstrlen(szBfr));
    wsprintf(szBfr, "Number of move records: %zu\r\n", size());
    file.Write(szBfr, lstrlen(szBfr));

    iterator pos;
    int nIndex = 0;
    for (pos = begin(); pos != end(); )
    {
        CMoveRecord& pRcd = GetNext(pos);
        CMoveRecord::RcdType eType = pRcd.GetType();
        ASSERT(eType >= 0 && eType < CMoveRecord::mrecMax);
        wsprintf(szBfr, "[Index=%04d; Seq=%04d: %s]\r\n", nIndex, pRcd.GetSeqNum(),
            (LPCSTR)tblTypes[eType]);
        file.Write(szBfr, lstrlen(szBfr));
        pRcd.DumpToTextFile(file);
        nIndex++;
    }
}
#endif

CMoveList::iterator CMoveList::FindIndex(size_t nIndex)
{
    if (nIndex >= size())
    {
        ASSERT(!"out of bounds");
        return end();
    }
    iterator retval = begin();
    for (size_t i = size_t(0) ; i < nIndex ; ++i)
    {
        ++retval;
    }
    return retval;
}
