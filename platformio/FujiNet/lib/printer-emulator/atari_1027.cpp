#include "atari_1027.h"
#include "../../include/debug.h"

void atari1027::pdf_handle_char(byte c)
{
    if (escMode)
    {
        // Atari 1027 escape codes:
        // CTRL-O - start underscoring        15
        // CTRL-N - stop underscoring         14  - note in T1027.BAS there is a case of 27 14
        // ESC CTRL-Y - start underscoring    27  25
        // ESC CTRL-Z - stop underscoring     27  26
        // ESC CTRL-W - start international   27  23
        // ESC CTRL-X - stop international    27  24
        switch (c)
        {
        case 25:
            uscoreFlag = true;
            break;
        case 26:
            uscoreFlag = false;
            break;
        case 23:
            intlFlag = true;
            break;
        case 24:
            intlFlag = false;
            break;
        default:
            break;
        }
        escMode = false;
    }
    else if (!intlFlag && c == 15)
        uscoreFlag = true;
    else if (!intlFlag && c == 14)
        uscoreFlag = false;
    else if (c == 27)
        escMode = true;
    else
    { // maybe printable character
        //printable characters for 1027 Standard Set + a few more >123 -- see mapping atari on ATASCII
        if (intlFlag && (c < 32 || c == 96 || c == 123))
        {
            bool valid = false;
            byte d = 0;

            if (c < 27)
            {
                d = intlchar[c];
                valid = true;
            }
            else if (c > 27 && c < 32)
            {
                // Codes 28-31 are arrows made from compound chars
                byte d1 = (byte)'|';
                switch (c)
                {
                case 28:
                    d = (byte)'^';
                    break;
                case 29:
                    d = (byte)'v';
                    d1 = (byte)'!';
                    break;
                case 30:
                    d = (byte)'<';
                    d1 = (byte)'-';
                    break;
                case 31:
                    d = (byte)'>';
                    d1 = (byte)'-';
                    break;
                default:
                    break;
                }
                _file.write(d1);
                _file.printf(")600("); // |^ -< -> !v
                valid = true;
            }
            else
                switch (c)
                {
                case 96:
                    d = byte(206); // use I with carot but really I with circle
                    valid = true;
                    break;
                case 123:
                    d = byte(196);
                    valid = true;
                    break;
                default:
                    valid = false;
                    break;
                }
            if (valid)
            {
                _file.write(d);
                if (uscoreFlag)
                    _file.printf(")600(_"); // close text string, backspace, start new text string, write _

                pdf_X += charWidth; // update x position
            }
        }
        else if (c > 31 && c < 128)
        {
            if (c == 123 || c == 125 || c == 127)
                c = ' ';
            if (c == '\\' || c == '(' || c == ')')
                _file.write('\\');
            _file.write(c);

            if (uscoreFlag)
                _file.printf(")600(_"); // close text string, backspace, start new text string, write _

            pdf_X += charWidth; // update x position
        }
    }
}

void atari1027::initPrinter(FS *filesystem)
{
    printer_emu::initPrinter(filesystem);

    shortname = "a1027";

    pageWidth = 612.0;
    pageHeight = 792.0;
    leftMargin = 66.0;
    bottomMargin = 0;
    printWidth = 480.0; // 6 2/3 inches
    lineHeight = 12.0;
    charWidth = 6; // 12cpi
    fontNumber = 1;
    fontSize = 10;

    // F1 : PrestigeEliteStd
    fontObjPos[0][0] = 66;    // FontDescriptor Reference
    fontObjPos[0][1] = 150;   // Widths Reference
    fontObjPos[0][2] = 199;   // FontDescriptor Object
    fontObjPos[0][3] = 424;   // FontFile Reference
    fontObjPos[0][4] = 443;   // FontFile Object
    fontObjPos[0][5] = 21694; // Widths Object
    fontObjPos[0][6] = 22738; // fragment length

    pdf_header();

    uscoreFlag = false;
    intlFlag = false;
    escMode = false;
}