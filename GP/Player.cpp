// Player.cpp
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
#include    "Player.h"

int CPlayerManager::AddPlayer(LPCTSTR pszName)
{
    Player player(pszName);
    return Add(player);
}

Player& CPlayerManager::GetPlayerUsingMask(DWORD dwMask)
{
    ASSERT(dwMask != 0);
    int nPlayerNum = GetPlayerNumFromMask(dwMask);
    return ElementAt(nPlayerNum);
}

DWORD CPlayerManager::GetMaskFromPlayerNum(int nPlayerNumber)
{
    if (nPlayerNumber < 0)
        return 0;
	return (DWORD)1 << nPlayerNumber;
}

// Returns -1 if no bit set. Otherwise, returns number of
// the rightmost set bit.
int CPlayerManager::GetPlayerNumFromMask(DWORD dwMask)
{
    if (dwMask == 0) return -1;          // For a bit more speed
    for (int i = 0; i < MAX_OWNER_ACCOUNTS; i++)
    {
        if ((dwMask & 1) != 0)
            return i;
        dwMask >>= 1;
    }
    return -1;
}

void CPlayerManager::Serialize(CArchive& ar)
{
    if (ar.IsStoring())
    {
        ar << (WORD)GetSize();
        for (int i = 0; i < GetSize(); i++)
            GetAt(i).Serialize(ar);
    }
    else
    {
        RemoveAll();
        int nCount;
        WORD wTmp;

        ar >> wTmp; nCount = (int)wTmp;
        while (nCount--)
        {
            Player player;
            player.Serialize(ar);
            Add(player);
        }
    }
}

/////////////////////////////////////////////////////////////////////////////

void Player::Serialize(CArchive& ar)
{
    if (ar.IsStoring())
    {
        ar << m_seqNum;
        ar << m_strName;
    }
    else
    {
        ar >> m_seqNum;
        ar >> m_strName;
    }
}

