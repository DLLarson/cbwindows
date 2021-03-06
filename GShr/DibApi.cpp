//  dibapi.cpp
//
//  Source file for Device-Independent Bitmap (DIB) API.  Provides
//  the following functions:
//
//  SaveDIB()           - Saves the specified dib in a file
//  ReadDIBFile()       - Loads a DIB from a file
//  CreateDIB()         - Create an empty DIB
//  CreateDIBPalette()  - Creates a palette from a DIB
//  FindDIBBits()       - Returns a pointer to the DIB bits
//  DIBWidth()          - Gets the width of the DIB
//  DIBHeight()         - Gets the height of the DIB
//  PaletteSize()       - Gets the size required to store the DIB's palette
//  DIBNumColors()      - Calculates the number of colors
//                        in the DIB's color table
//  CopyHandle()        - Makes a copy of the given global memory block
//
// This is a part of the Microsoft Foundation Classes C++ library.
// Copyright (C) 1992 Microsoft Corporation
// All rights reserved.
//
// This source code is only intended as a supplement to the
// Microsoft Foundation Classes Reference and Microsoft
// QuickHelp and/or WinHelp documentation provided with the library.
// See these sources for detailed information regarding the
// Microsoft Foundation Classes product.

#include    "stdafx.h"
#include    <io.h>
#include    <errno.h>
#include    "GdiTools.h"
#include    "DibApi.h"

#ifdef _DEBUG
    #define new DEBUG_NEW
#endif

/*
 * Dib Header Marker - used in writing DIBs to files
 */

#define DIB_HEADER_MARKER   ((WORD) ('M' << 8) | 'B')

///////////////////////////////////////////////////////////////////////

BOOL SaveDIB(HDIB hDib, CFile& file)
{
    BITMAPFILEHEADER bmfHdr; // Header for Bitmap file
    LPBITMAPINFOHEADER lpBI;   // Pointer to DIB info structure
    DWORD dwDIBSize;

    if (hDib == NULL)
        return FALSE;

    /*
     * Get a pointer to the DIB memory, the first of which contains
     * a BITMAPINFO structure
     */
    lpBI = (LPBITMAPINFOHEADER) GlobalLock((HGLOBAL)hDib);
    if (lpBI == NULL)
        return FALSE;

    if (!IS_WIN30_DIB(lpBI))
    {
        GlobalUnlock((HGLOBAL) hDib);
        return FALSE;       // It's an other-style DIB (save not supported)
    }

    /*
     * Fill in the fields of the file header
     */

    /* Fill in file type (first 2 bytes must be "BM" for a bitmap) */
    bmfHdr.bfType = DIB_HEADER_MARKER;  // "BM"

    // Calculating the size of the DIB is a bit tricky (if we want to
    // do it right).  The easiest way to do this is to call GlobalSize()
    // on our global handle, but since the size of our global memory may have
    // been padded a few bytes, we may end up writing out a few too
    // many bytes to the file (which may cause problems with some apps).
    //
    // So, instead let's calculate the size manually (if we can)
    //
    // First, find size of header plus size of color table.  Since the
    // first DWORD in both BITMAPINFOHEADER and BITMAPCOREHEADER conains
    // the size of the structure, let's use this.

    dwDIBSize = *(LPDWORD)lpBI + PaletteSize((LPSTR)lpBI);  // Partial Calculation

    // Now calculate the size of the image

    if ((lpBI->biCompression == BI_RLE8) || (lpBI->biCompression == BI_RLE4))
    {
        // It's an RLE bitmap, we can't calculate size, so trust the
        // biSizeImage field

        dwDIBSize += lpBI->biSizeImage;
    }
    else
    {
        DWORD dwBmBitsSize;  // Size of Bitmap Bits only

        // It's not RLE, so size is Width (DWORD aligned) * Height

        dwBmBitsSize = WIDTHBYTES((lpBI->biWidth)*((DWORD)lpBI->biBitCount)) * lpBI->biHeight;

        dwDIBSize += dwBmBitsSize;

        // Now, since we have calculated the correct size, why don't we
        // fill in the biSizeImage field (this will fix any .BMP files which
        // have this field incorrect).

        lpBI->biSizeImage = dwBmBitsSize;
    }


    // Calculate the file size by adding the DIB size to sizeof(BITMAPFILEHEADER)

    bmfHdr.bfSize = dwDIBSize + sizeof(BITMAPFILEHEADER);
    bmfHdr.bfReserved1 = 0;
    bmfHdr.bfReserved2 = 0;

    /*
     * Now, calculate the offset the actual bitmap bits will be in
     * the file -- It's the Bitmap file header plus the DIB header,
     * plus the size of the color table.
     */
    bmfHdr.bfOffBits = (DWORD)sizeof(BITMAPFILEHEADER) + lpBI->biSize
                       + PaletteSize((LPSTR)lpBI);
    TRY
    {
        // Write the file header
        file.Write((LPSTR)&bmfHdr, sizeof(BITMAPFILEHEADER));
        //
        // Write the DIB header and the bits
        //
        file.Write(lpBI, dwDIBSize);
    }
    CATCH (CFileException, e)
    {
        GlobalUnlock((HGLOBAL) hDib);
        THROW_LAST();
    }
    END_CATCH

    GlobalUnlock((HGLOBAL) hDib);
    return TRUE;
}

///////////////////////////////////////////////////////////////////////

HDIB ReadDIBFile(CFile& file)
{
    BITMAPFILEHEADER bmfHeader;
    DWORD dwBitsSize;
    HDIB hDIB;
    LPSTR pDIB;

    /*
     * get length of DIB in bytes for use when reading
     */

    dwBitsSize = (DWORD)file.GetLength();

    /*
     * Go read the DIB file header and check if it's valid.
     */
    if ((file.Read((LPSTR)&bmfHeader, sizeof(bmfHeader)) !=
         sizeof(bmfHeader)) || (bmfHeader.bfType != DIB_HEADER_MARKER))
    {
        return NULL;
    }
    /*
     * Allocate memory for DIB
     */
    hDIB = (HDIB) GlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT, dwBitsSize);
    if (hDIB == 0)
    {
        return NULL;
    }
    pDIB = (LPSTR) GlobalLock((HGLOBAL) hDIB);

    /*
     * Go read the bits.
     */
    if (file.Read(pDIB, dwBitsSize - sizeof(BITMAPFILEHEADER)) !=
        dwBitsSize - sizeof(BITMAPFILEHEADER))
    {
        GlobalUnlock((HGLOBAL) hDIB);
        GlobalFree((HGLOBAL) hDIB);
        return NULL;
    }
    GlobalUnlock((HGLOBAL) hDIB);
    return hDIB;
}

////////////////////////////////////////////////////////////////////

void AddDIBColorsToPaletteEntryTable(HDIB hDIB, LPPALETTEENTRY pLP,
                                            int nSize, BOOL bReducedPalette)
{
    LPSTR lpbi;              // pointer to packed-DIB
    LPBITMAPINFO lpbmi;      // pointer to BITMAPINFO structure (Win3.0)
    LPBITMAPCOREINFO lpbmc;  // pointer to BITMAPCOREINFO structure (old)

    /* if handle to DIB is invalid, return FALSE */

    ASSERT(hDIB != NULL);

    lpbi = (LPSTR) GlobalLock((HGLOBAL) hDIB);

    /* get pointer to BITMAPINFO (Win 3.0) */
    lpbmi = (LPBITMAPINFO)lpbi;

    /* get pointer to BITMAPCOREINFO (old 1.x) */
    lpbmc = (LPBITMAPCOREINFO)lpbi;

    /* get the number of colors in the DIB */
    WORD wNumColors = DIBNumColors(lpbi);
    BOOL bWinStyleDIB = IS_WIN30_DIB(lpbi);

    if (wNumColors != 0)
    {
        LPBYTE pFlags = NULL;
        if (bReducedPalette)
            pFlags = GetColorUseTable(lpbi);

        for (int i = 0; i < (int)wNumColors; i++)
        {
            if (bReducedPalette)
                if (pFlags[i] == 0) continue; // Don't store color (not used)
            PALETTEENTRY pe;
            if (bWinStyleDIB)
            {
                pe.peRed = lpbmi->bmiColors[i].rgbRed;
                pe.peGreen = lpbmi->bmiColors[i].rgbGreen;
                pe.peBlue = lpbmi->bmiColors[i].rgbBlue;
                pe.peFlags = 0;
            }
            else
            {
                pe.peRed = lpbmc->bmciColors[i].rgbtRed;
                pe.peGreen = lpbmc->bmciColors[i].rgbtGreen;
                pe.peBlue = lpbmc->bmciColors[i].rgbtBlue;
                pe.peFlags = 0;
            }
            AddEntryToPalette(pLP, nSize, pe);
        }
        if (pFlags) delete pFlags;
    }
    GlobalUnlock((HGLOBAL) hDIB);
}

///////////////////////////////////////////////////////////////////////

HDIB CreateDIB(DWORD dwWidth, DWORD dwHeight, WORD wBitCount)
{
    BITMAPINFOHEADER bi;         // bitmap header
    LPBITMAPINFOHEADER lpbi;     // pointer to BITMAPINFOHEADER
    DWORD dwLen;                 // size of memory block
    HDIB hDIB;
    DWORD dwBytesPerLine;        // Number of bytes per scanline


    // Make sure bits per pixel is valid
    if (wBitCount <= 1)
        wBitCount = 1;
    else if (wBitCount <= 4)
        wBitCount = 4;
    else if (wBitCount <= 8)
        wBitCount = 8;
    else if (wBitCount <= 16)
        wBitCount = 16;
    else if (wBitCount <= 24)
        wBitCount = 24;
    else
        wBitCount = 4;  // set default value to 4 if parameter is bogus

    // initialize BITMAPINFOHEADER
    bi.biSize = sizeof(BITMAPINFOHEADER);
    bi.biWidth = dwWidth;         // fill in width from parameter
    bi.biHeight = dwHeight;       // fill in height from parameter
    bi.biPlanes = 1;              // must be 1
    bi.biBitCount = wBitCount;    // from parameter
    bi.biCompression = wBitCount == 16 ? BI_BITFIELDS : BI_RGB;
    bi.biSizeImage = 0;           // 0's here mean "default"
    bi.biXPelsPerMeter = 0;
    bi.biYPelsPerMeter = 0;
    bi.biClrUsed = 0;
    bi.biClrImportant = 0;

    // calculate size of memory block required to store the DIB.  This
    // block should be big enough to hold the BITMAPINFOHEADER, the color
    // table, and the bits

    dwBytesPerLine = WIDTHBYTES(wBitCount * dwWidth);
    dwLen = bi.biSize + PaletteSize((LPSTR)&bi) + (dwBytesPerLine * dwHeight);

    // alloc memory block to store our bitmap
    hDIB = (HDIB)GlobalAlloc(GHND, dwLen);

    // major bummer if we couldn't get memory block
    if (!hDIB)
        return NULL;

    // lock memory and get pointer to it
    lpbi = (LPBITMAPINFOHEADER)GlobalLock(hDIB);

    // use our bitmap info structure to fill in first part of
    // our DIB with the BITMAPINFOHEADER
    *lpbi = bi;

    InitColorTableMasksIfReqd((BITMAPINFO*)lpbi);

    // Since we don't know what the colortable and bits should contain,
    // just leave these blank.  Unlock the DIB and return the HDIB.

    GlobalUnlock(hDIB);

    /* return handle to the DIB */
    return hDIB;
}

///////////////////////////////////////////////////////////////////////

BOOL CreateDIBPalette(HDIB hDIB, CPalette* pPal, BOOL bReducedPalette)
{
    LPLOGPALETTE lpPal;      // pointer to a logical palette
    HANDLE hLogPal;          // handle to a logical palette
    HPALETTE hPal = NULL;    // handle to a palette
    int i;                   // loop index
    WORD wNumColors;         // number of colors in color table
    LPSTR lpbi;              // pointer to packed-DIB
    LPBITMAPINFO lpbmi;      // pointer to BITMAPINFO structure (Win3.0)
    LPBITMAPCOREINFO lpbmc;  // pointer to BITMAPCOREINFO structure (old)
    BOOL bWinStyleDIB;       // flag which signifies whether this is a Win3.0 DIB
    BOOL bResult = FALSE;

    /* if handle to DIB is invalid, return FALSE */

    if (hDIB == NULL)
        return FALSE;

    lpbi = (LPSTR) GlobalLock((HGLOBAL) hDIB);

    /* get pointer to BITMAPINFO (Win 3.0) */
    lpbmi = (LPBITMAPINFO)lpbi;

    /* get pointer to BITMAPCOREINFO (old 1.x) */
    lpbmc = (LPBITMAPCOREINFO)lpbi;

    /* get the number of colors in the DIB */
    wNumColors = DIBNumColors(lpbi);

    if (wNumColors != 0)
    {
        LPBYTE pFlags = NULL;
        if (bReducedPalette)
            pFlags = GetColorUseTable(lpbi);

        /* allocate memory block for logical palette */
        hLogPal = GlobalAlloc(GHND, sizeof(LOGPALETTE)+ sizeof(PALETTEENTRY) *
                              wNumColors);

        /* If not enough memory, clean up and return NULL */
        if (hLogPal == 0)
        {
            GlobalUnlock((HGLOBAL) hDIB);
            return FALSE;
        }

        lpPal = (LPLOGPALETTE) GlobalLock((HGLOBAL) hLogPal);

        /* set version and number of palette entries */
        lpPal->palVersion = PALVERSION;

        /* is this a Win 3.0 DIB? */
        bWinStyleDIB = IS_WIN30_DIB(lpbi);
        int n = 0;
        for (i = 0; i < (int)wNumColors; i++)
        {
            if (bReducedPalette)
                if (pFlags[i] == 0) continue; // Don't store color (not used)
            if (bWinStyleDIB)
            {
                lpPal->palPalEntry[n].peRed = lpbmi->bmiColors[i].rgbRed;
                lpPal->palPalEntry[n].peGreen = lpbmi->bmiColors[i].rgbGreen;
                lpPal->palPalEntry[n].peBlue = lpbmi->bmiColors[i].rgbBlue;
                lpPal->palPalEntry[n].peFlags = 0;
            }
            else
            {
                lpPal->palPalEntry[n].peRed = lpbmc->bmciColors[i].rgbtRed;
                lpPal->palPalEntry[n].peGreen = lpbmc->bmciColors[i].rgbtGreen;
                lpPal->palPalEntry[n].peBlue = lpbmc->bmciColors[i].rgbtBlue;
                lpPal->palPalEntry[n].peFlags = 0;
            }
            n++;
        }

        /* create the palette and get handle to it */
        lpPal->palNumEntries = (WORD)n;
        bResult = pPal->CreatePalette(lpPal);
        GlobalUnlock((HGLOBAL) hLogPal);
        GlobalFree((HGLOBAL) hLogPal);
        if (pFlags) delete pFlags;
    }

    GlobalUnlock((HGLOBAL) hDIB);

    return bResult;
}

///////////////////////////////////////////////////////////////////////

LPBYTE GetColorUseTable(LPSTR lpbi)
{
    WORD wNumColors = DIBNumColors(lpbi);
    ASSERT(wNumColors <= 256);
    LPBYTE pFlags = new BYTE[wNumColors];
    memset(pFlags, 0, wNumColors);
    if (wNumColors == 2)
    {
        pFlags[0] = pFlags[1] = 1;
        return pFlags;
    }

    int nWidth = (int)DIBWidth(lpbi);
    int nHeight = (int)DIBHeight(lpbi);

    for (int y = 0; y < nHeight; y++)
    {
        for (int x = 0; x < nWidth; x++)
        {
            BYTE bmapByte = *DibXY(lpbi,x,y);
            if (wNumColors == 256)
                pFlags[bmapByte] = 1;
            else if (wNumColors == 16)
            {
                int idx = (x & 0 ? bmapByte >> 4 : bmapByte) & 0x0F;
                pFlags[idx] = 1;
            }
        }
    }
    return pFlags;
}

///////////////////////////////////////////////////////////////////////

BOOL IsColorNumInDIB(LPSTR lpbi, UINT iColor)
{
    int nWidth = (int)DIBWidth(lpbi);
    int nHeight = (int)DIBHeight(lpbi);

    for (int y = 0; y < nHeight; y++)
    {
        for (int x = 0; x < nWidth; x++)
            if (*DibXY(lpbi,x,y) == iColor) return TRUE;
    }
    return FALSE;
}

///////////////////////////////////////////////////////////////////////

LPBYTE DibXY(LPSTR lpbi, int x, int y)
{
    BYTE* pBits;
    DWORD ulWidthBytes;
    LPBITMAPINFOHEADER lpbmi = (LPBITMAPINFOHEADER)lpbi;

    pBits = (BYTE *)lpbi + (WORD)lpbmi->biSize + PaletteSize(lpbi);
    ulWidthBytes = DIBWIDTHBYTES(*lpbmi);
    pBits += (ulWidthBytes * (long)(lpbmi->biHeight - y - 1)) +
        (x * (int)lpbmi->biBitCount / 8);
    return(LPBYTE)pBits;
}

///////////////////////////////////////////////////////////////////////

LPSTR FindDIBBits(LPSTR lpbi)
{
    return(lpbi + *(LPDWORD)lpbi + PaletteSize(lpbi));
}

///////////////////////////////////////////////////////////////////////

DWORD DIBWidth(LPSTR lpDIB)
{
    LPBITMAPINFOHEADER lpbmi;  // pointer to a Win 3.0-style DIB
    LPBITMAPCOREHEADER lpbmc;  // pointer to an other-style DIB

    /* point to the header (whether Win 3.0 and old) */

    lpbmi = (LPBITMAPINFOHEADER)lpDIB;
    lpbmc = (LPBITMAPCOREHEADER)lpDIB;

    /* return the DIB width if it is a Win 3.0 DIB */
    if (IS_WIN30_DIB(lpDIB))
        return lpbmi->biWidth;
    else  /* it is an other-style DIB, so return its width */
        return(DWORD)lpbmc->bcWidth;
}

///////////////////////////////////////////////////////////////////////

DWORD DIBHeight(LPSTR lpDIB)
{
    LPBITMAPINFOHEADER lpbmi;  // pointer to a Win 3.0-style DIB
    LPBITMAPCOREHEADER lpbmc;  // pointer to an other-style DIB

    /* point to the header (whether old or Win 3.0 */

    lpbmi = (LPBITMAPINFOHEADER)lpDIB;
    lpbmc = (LPBITMAPCOREHEADER)lpDIB;

    /* return the DIB height if it is a Win 3.0 DIB */
    if (IS_WIN30_DIB(lpDIB))
        return lpbmi->biHeight;
    else  /* it is an other-style DIB, so return its height */
        return(DWORD)lpbmc->bcHeight;
}

///////////////////////////////////////////////////////////////////////

WORD PaletteSize(LPSTR lpbi)
{
    /* calculate the size required by the palette */
    if (IS_WIN30_DIB (lpbi))
        return (WORD)(DIBNumColors(lpbi) * sizeof(RGBQUAD));
    else
        return (WORD)(DIBNumColors(lpbi) * sizeof(RGBTRIPLE));
}

///////////////////////////////////////////////////////////////////////

WORD DIBNumColors(LPSTR lpbi)
{
    WORD wBitCount;  // DIB bit count

    /*  If this is a Windows-style DIB, the number of colors in the
     *  color table can be less than the number of bits per pixel
     *  allows for (i.e. lpbi->biClrUsed can be set to some value).
     *  If this is the case, return the appropriate value.
     */

    if (IS_WIN30_DIB(lpbi))
    {
        DWORD dwClrUsed;

        dwClrUsed = ((LPBITMAPINFOHEADER)lpbi)->biClrUsed;
        if (dwClrUsed != 0)
            return(WORD)dwClrUsed;
    }

    /*  Calculate the number of colors in the color table based on
     *  the number of bits per pixel for the DIB.
     */
    if (IS_WIN30_DIB(lpbi))
        wBitCount = ((LPBITMAPINFOHEADER)lpbi)->biBitCount;
    else
        wBitCount = ((LPBITMAPCOREHEADER)lpbi)->bcBitCount;

    /* return number of colors based on bits per pixel */
    switch (wBitCount)
    {
        case 1:
            return 2;

        case 4:
            return 16;

        case 8:
            return 256;

        case 16:
            return 3;       // Special: Used only for the pixel bit masks

        default:
            return 0;
    }
}

///////////////////////////////////////////////////////////////////////

HBITMAP DIBToBitmap (HANDLE hDIB, HPALETTE hPal)
{
    LPSTR    lpDIBHdr, lpDIBBits;
    HBITMAP  hBitmap;
    HDC      hDC;
    HPALETTE hOldPal = NULL;

    if (!hDIB)
        return NULL;

    lpDIBHdr = (LPSTR)GlobalLock(hDIB);
    lpDIBBits = FindDIBBits(lpDIBHdr);
    hDC = GetDC(NULL);

    if (!hDC)
    {
        GlobalUnlock(hDIB);
        return NULL;
    }

    if (hPal)
        hOldPal = SelectPalette(hDC, hPal, FALSE);

    RealizePalette (hDC);

    hBitmap = CreateDIBitmap(hDC, (LPBITMAPINFOHEADER) lpDIBHdr, CBM_INIT,
        lpDIBBits, (LPBITMAPINFO)lpDIBHdr, DIB_RGB_COLORS);

    if (!hBitmap)
        return NULL;

    if (hOldPal)
        SelectPalette(hDC, hOldPal, FALSE);

    ReleaseDC (NULL, hDC);
    GlobalUnlock(hDIB);

    return hBitmap;
}

///////////////////////////////////////////////////////////////////////

HANDLE BitmapToDIB(HBITMAP hBitmap, HPALETTE hPal, int nBPP)
{
    HANDLE hDIB = NULL;
    if (!hBitmap)
        return NULL;
    // If the target format is 16 bits per pixel we avoid the use
    // of GetDIBits() since it forces 555 format and we desire 565
    // format.
    if (nBPP == 16)
    {
        HDC hMemDCScreen = NULL;
        HDC hMemDCSrc = NULL;
        HDC hMemDCSect = NULL;
        HBITMAP hBmapSect = NULL;
        HBITMAP hPrvBmapSrc = NULL;
        HBITMAP hPrvBmapSect = NULL;
        HPALETTE hPrvPal = NULL;
        LPVOID pBits = NULL;

        // If the bitmap is already a DIB section of the proper format
        // we can just directly make a DIB.
        DIBSECTION dibSect;
        if (GetObject(hBitmap, sizeof(dibSect), (LPVOID)&dibSect))
        {
            if (dibSect.dsBmih.biBitCount == 16 && dibSect.dsBitfields[0] == 0xF800)
                return ConvertDIBSectionToDIB(hBitmap);
        }

        BITMAP bmapSrc;                 // Source bitmap
        if (!GetObject(hBitmap, sizeof(bmapSrc), (LPVOID)&bmapSrc))
            return NULL;

        // To force the proper 16bit bitmap format we'll use
        // DIB sections. A DIB section of the desired format is
        // created and the bitmap is Blited into it. Then the
        // section is converted to a DIB.

        BYTE bmapInfo[sizeof(BITMAPINFOHEADER) + 3 * sizeof(RGBQUAD)];
        BITMAPINFO* pbmi = (BITMAPINFO*)&bmapInfo;      // Convenience pointer
        InitBitmapInfoHeader((BITMAPINFOHEADER*)pbmi,
            bmapSrc.bmWidth, bmapSrc.bmHeight, 16);
        InitColorTableMasksIfReqd(pbmi);

        // We need a reference DC for palette bitmaps
        hMemDCScreen = GetDC(NULL);
        hMemDCSrc = CreateCompatibleDC(hMemDCScreen);
        ReleaseDC(NULL, hMemDCSrc);

        if (hPal)
        {
            hPrvPal = SelectPalette(hMemDCSrc, hPal, FALSE);
            RealizePalette(hMemDCSrc);
        }

        hBmapSect = CreateDIBSection(hMemDCSrc, pbmi, DIB_RGB_COLORS, &pBits, NULL, 0);
        if (hBmapSect != NULL)
        {
            hMemDCSect = CreateCompatibleDC(hMemDCScreen);
            hPrvBmapSect = (HBITMAP)SelectObject(hMemDCSect, hBmapSect);

            hPrvBmapSrc = (HBITMAP)SelectObject(hMemDCSrc, hBitmap);

            BitBlt(hMemDCSect, 0, 0, bmapSrc.bmWidth, bmapSrc.bmHeight,
                hMemDCSrc, 0, 0, SRCCOPY);

            SelectObject(hMemDCSect, hPrvBmapSect);
            SelectObject(hMemDCSrc, hPrvBmapSrc);

            DeleteDC(hMemDCSect);
        }

        if (hPrvPal)
            SelectPalette(hMemDCSrc, hPrvPal, FALSE);
        DeleteDC(hMemDCSrc);

        // Now that we have the converted bitmap create the
        // actual DIB.
        hDIB = ConvertDIBSectionToDIB(hBmapSect);
        DeleteObject(hBmapSect);
    }
    else
    {
        BITMAP             Bitmap;
        BITMAPINFOHEADER   bmInfoHdr;
        LPBITMAPINFOHEADER lpbmInfoHdr;
        LPSTR              lpBits;
        HDC                hMemDC;
        HPALETTE           hOldPal = NULL;

        // Do some setup -- make sure the Bitmap passed in is valid,
        // get info on the bitmap (like its height, width, etc.),
        // then setup a BITMAPINFOHEADER.

        if (!hBitmap)
            return NULL;

        if (!GetObject(hBitmap, sizeof(Bitmap), (LPSTR)&Bitmap))
            return NULL;

        // Changed to allow forces bits per pixel.
        InitBitmapInfoHeader(&bmInfoHdr, Bitmap.bmWidth, Bitmap.bmHeight,
            nBPP == 0 ? Bitmap.bmPlanes * Bitmap.bmBitsPixel : nBPP);

        // Now allocate memory for the DIB.  Then, set the BITMAPINFOHEADER
        // into this memory, and find out where the bitmap bits go.

        hDIB = GlobalAlloc(GHND, sizeof(BITMAPINFOHEADER) +
            PaletteSize((LPSTR)&bmInfoHdr) + bmInfoHdr.biSizeImage);

        if (!hDIB)
            return NULL;

        lpbmInfoHdr  = (LPBITMAPINFOHEADER)GlobalLock(hDIB);
        *lpbmInfoHdr = bmInfoHdr;

        InitColorTableMasksIfReqd((LPBITMAPINFO)lpbmInfoHdr);

        lpBits = FindDIBBits((LPSTR)lpbmInfoHdr);

        // Now, we need a DC to hold our bitmap.  If the app passed us
        //  a palette, it should be selected into the DC.

        hMemDC = GetDC(NULL);

        if (hPal)
        {
            hOldPal = SelectPalette(hMemDC, hPal, FALSE);
            RealizePalette(hMemDC);
        }
        // We're finally ready to get the DIB.  Call the driver and let
        // it party on our bitmap.  It will fill in the color table,
        // and bitmap bits of our global memory block.
        if (!GetDIBits(hMemDC, hBitmap, 0, Bitmap.bmHeight, lpBits,
            (LPBITMAPINFO)lpbmInfoHdr, DIB_RGB_COLORS))
        {
            GlobalUnlock (hDIB);
            GlobalFree (hDIB);
            hDIB = NULL;
        }
        else
            GlobalUnlock (hDIB);

        // Finally, clean up and return.
        if (hOldPal)
            SelectPalette(hMemDC, hOldPal, FALSE);

        ReleaseDC(NULL, hMemDC);
    }
    return hDIB;
}

///////////////////////////////////////////////////////////////////////

HANDLE ConvertDIBSectionToDIB(HBITMAP hDibSect)
{
    DIBSECTION dibSect;
    if (!GetObject(hDibSect, sizeof(dibSect), (LPVOID)&dibSect))
        return NULL;

    ASSERT(dibSect.dsBmih.biBitCount == 16);            // Only support this
    HANDLE hDIB = GlobalAlloc(GHND, sizeof(BITMAPINFOHEADER) +
        3 * sizeof(RGBQUAD) + dibSect.dsBmih.biSizeImage);

    if (hDIB == NULL)
        return NULL;

    BITMAPINFOHEADER* pbmInfoHdr = (BITMAPINFOHEADER*)GlobalLock(hDIB);
    DWORD* pdwMasks = (DWORD*)((LPSTR)pbmInfoHdr + sizeof(BITMAPINFOHEADER));

    *pbmInfoHdr = dibSect.dsBmih;

    pdwMasks[0] = dibSect.dsBitfields[0];
    pdwMasks[1] = dibSect.dsBitfields[1];
    pdwMasks[2] = dibSect.dsBitfields[2];

    LPSTR pBits = FindDIBBits((LPSTR)pbmInfoHdr);

    memcpy(pBits, dibSect.dsBm.bmBits, dibSect.dsBmih.biSizeImage);

    GlobalUnlock(hDIB);

    return hDIB;
}

///////////////////////////////////////////////////////////////////////

void InitBitmapInfoHeader(LPBITMAPINFOHEADER lpBmInfoHdr, DWORD dwWidth,
    DWORD dwHeight, int nBPP)
{
    memset(lpBmInfoHdr, 0, sizeof(BITMAPINFOHEADER));

    lpBmInfoHdr->biSize      = sizeof(BITMAPINFOHEADER);
    lpBmInfoHdr->biWidth     = dwWidth;
    lpBmInfoHdr->biHeight    = dwHeight;
    lpBmInfoHdr->biPlanes    = 1;

    if (nBPP <= 1)
        nBPP = 1;
    else if (nBPP <= 4)
        nBPP = 4;
    else if (nBPP <= 8)
        nBPP = 8;
    else if (nBPP <= 16)
    {
        nBPP = 16;
        lpBmInfoHdr->biCompression = BI_BITFIELDS;
    }
    else
        nBPP = 24;

    lpBmInfoHdr->biBitCount  = nBPP;
    lpBmInfoHdr->biSizeImage = WIDTHBYTES(dwWidth * nBPP) * dwHeight;
}

///////////////////////////////////////////////////////////////////////

void InitColorTableMasksIfReqd(LPBITMAPINFO lpBmInfo)
{
    if (lpBmInfo->bmiHeader.biBitCount == 16)
    {
        DWORD* pdwMasks = (DWORD*)&lpBmInfo->bmiColors[0];
        pdwMasks[0] = 0x0000F800;
        pdwMasks[1] = 0x000007E0;
        pdwMasks[2] = 0x0000001F;
    }
}

//////////////////////////////////////////////////////////////////////////
//// Clipboard support

HANDLE CopyHandle (HANDLE h)
{
    BYTE*       lpCopy;
    BYTE*       lp;
    HANDLE      hCopy;
    DWORD       dwLen;

    if (h == NULL)
        return NULL;

    dwLen = GlobalSize((HGLOBAL) h);

    if ((hCopy = (HANDLE) GlobalAlloc (GHND, dwLen)) != NULL)
    {
        lpCopy = (BYTE *) GlobalLock((HGLOBAL) hCopy);
        lp     = (BYTE *) GlobalLock((HGLOBAL) h);

        while (dwLen--)
            *lpCopy++ = *lp++;

        GlobalUnlock((HGLOBAL) hCopy);
        GlobalUnlock((HGLOBAL) h);
    }

    return hCopy;
}

