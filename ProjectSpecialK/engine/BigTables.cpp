unsigned int crcLut[256] =
{
	0x00000000, 0x77073096, 0xEE0E612C, 0x990951BA,	0x076DC419, 0x706AF48F, 0xE963A535, 0x9E6495A3,
	0x0EDB8832, 0x79DCB8A4, 0xE0D5E91E, 0x97D2D988,	0x09B64C2B, 0x7EB17CBD, 0xE7B82D07, 0x90BF1D91,
	0x1DB71064, 0x6AB020F2, 0xF3B97148, 0x84BE41DE,	0x1ADAD47D, 0x6DDDE4EB, 0xF4D4B551, 0x83D385C7,
	0x136C9856, 0x646BA8C0, 0xFD62F97A, 0x8A65C9EC,	0x14015C4F, 0x63066CD9, 0xFA0F3D63, 0x8D080DF5,
	0x3B6E20C8, 0x4C69105E, 0xD56041E4, 0xA2677172,	0x3C03E4D1, 0x4B04D447, 0xD20D85FD, 0xA50AB56B,
	0x35B5A8FA, 0x42B2986C, 0xDBBBC9D6, 0xACBCF940,	0x32D86CE3, 0x45DF5C75, 0xDCD60DCF, 0xABD13D59,
	0x26D930AC, 0x51DE003A, 0xC8D75180, 0xBFD06116,	0x21B4F4B5, 0x56B3C423, 0xCFBA9599, 0xB8BDA50F,
	0x2802B89E, 0x5F058808, 0xC60CD9B2, 0xB10BE924,	0x2F6F7C87, 0x58684C11, 0xC1611DAB, 0xB6662D3D,
	0x76DC4190, 0x01DB7106, 0x98D220BC, 0xEFD5102A,	0x71B18589, 0x06B6B51F, 0x9FBFE4A5, 0xE8B8D433,
	0x7807C9A2, 0x0F00F934, 0x9609A88E, 0xE10E9818,	0x7F6A0DBB, 0x086D3D2D, 0x91646C97, 0xE6635C01,
	0x6B6B51F4, 0x1C6C6162, 0x856530D8, 0xF262004E,	0x6C0695ED, 0x1B01A57B, 0x8208F4C1, 0xF50FC457,
	0x65B0D9C6, 0x12B7E950, 0x8BBEB8EA, 0xFCB9887C,	0x62DD1DDF, 0x15DA2D49, 0x8CD37CF3, 0xFBD44C65,
	0x4DB26158, 0x3AB551CE, 0xA3BC0074, 0xD4BB30E2,	0x4ADFA541, 0x3DD895D7, 0xA4D1C46D, 0xD3D6F4FB,
	0x4369E96A, 0x346ED9FC, 0xAD678846, 0xDA60B8D0,	0x44042D73, 0x33031DE5, 0xAA0A4C5F, 0xDD0D7CC9,
	0x5005713C, 0x270241AA, 0xBE0B1010, 0xC90C2086,	0x5768B525, 0x206F85B3, 0xB966D409, 0xCE61E49F,
	0x5EDEF90E, 0x29D9C998, 0xB0D09822, 0xC7D7A8B4,	0x59B33D17, 0x2EB40D81, 0xB7BD5C3B, 0xC0BA6CAD,
	0xEDB88320, 0x9ABFB3B6, 0x03B6E20C, 0x74B1D29A,	0xEAD54739, 0x9DD277AF, 0x04DB2615, 0x73DC1683,
	0xE3630B12, 0x94643B84, 0x0D6D6A3E, 0x7A6A5AA8,	0xE40ECF0B, 0x9309FF9D, 0x0A00AE27, 0x7D079EB1,
	0xF00F9344, 0x8708A3D2, 0x1E01F268, 0x6906C2FE,	0xF762575D, 0x806567CB, 0x196C3671, 0x6E6B06E7,
	0xFED41B76, 0x89D32BE0, 0x10DA7A5A, 0x67DD4ACC,	0xF9B9DF6F, 0x8EBEEFF9, 0x17B7BE43, 0x60B08ED5,
	0xD6D6A3E8, 0xA1D1937E, 0x38D8C2C4, 0x4FDFF252,	0xD1BB67F1, 0xA6BC5767, 0x3FB506DD, 0x48B2364B,
	0xD80D2BDA, 0xAF0A1B4C, 0x36034AF6, 0x41047A60,	0xDF60EFC3, 0xA867DF55, 0x316E8EEF, 0x4669BE79,
	0xCB61B38C, 0xBC66831A, 0x256FD2A0, 0x5268E236,	0xCC0C7795, 0xBB0B4703, 0x220216B9, 0x5505262F,
	0xC5BA3BBE, 0xB2BD0B28, 0x2BB45A92, 0x5CB36A04,	0xC2D7FFA7, 0xB5D0CF31, 0x2CD99E8B, 0x5BDEAE1D,
	0x9B64C2B0, 0xEC63F226, 0x756AA39C, 0x026D930A,	0x9C0906A9, 0xEB0E363F, 0x72076785, 0x05005713,
	0x95BF4A82, 0xE2B87A14, 0x7BB12BAE, 0x0CB61B38,	0x92D28E9B, 0xE5D5BE0D, 0x7CDCEFB7, 0x0BDBDF21,
	0x86D3D2D4, 0xF1D4E242, 0x68DDB3F8, 0x1FDA836E,	0x81BE16CD, 0xF6B9265B, 0x6FB077E1, 0x18B74777,
	0x88085AE6, 0xFF0F6A70, 0x66063BCA, 0x11010B5C,	0x8F659EFF, 0xF862AE69, 0x616BFFD3, 0x166CCF45,
	0xA00AE278, 0xD70DD2EE, 0x4E048354, 0x3903B3C2,	0xA7672661, 0xD06016F7, 0x4969474D, 0x3E6E77DB,
	0xAED16A4A, 0xD9D65ADC, 0x40DF0B66, 0x37D83BF0,	0xA9BCAE53, 0xDEBB9EC5, 0x47B2CF7F, 0x30B5FFE9,
	0xBDBDF21C, 0xCABAC28A, 0x53B39330, 0x24B4A3A6,	0xBAD03605, 0xCDD70693, 0x54DE5729, 0x23D967BF,
	0xB3667A2E, 0xC4614AB8, 0x5D681B02, 0x2A6F2B94,	0xB40BBE37, 0xC30C8EA1, 0x5A05DF1B, 0x2D02EF8D
};

#ifndef _WIN32
//Left value is FROM, right value is TO.
unsigned short caseFolding[2378] = {
	0x0041, 0x0061, //latin capital letter a
	0x0042, 0x0062, //latin capital letter b
	0x0043, 0x0063, //latin capital letter c
	0x0044, 0x0064, //latin capital letter d
	0x0045, 0x0065, //latin capital letter e
	0x0046, 0x0066, //latin capital letter f
	0x0047, 0x0067, //latin capital letter g
	0x0048, 0x0068, //latin capital letter h
	0x0049, 0x0069, //latin capital letter i
	0x004A, 0x006A, //latin capital letter j
	0x004B, 0x006B, //latin capital letter k
	0x004C, 0x006C, //latin capital letter l
	0x004D, 0x006D, //latin capital letter m
	0x004E, 0x006E, //latin capital letter n
	0x004F, 0x006F, //latin capital letter o
	0x0050, 0x0070, //latin capital letter p
	0x0051, 0x0071, //latin capital letter q
	0x0052, 0x0072, //latin capital letter r
	0x0053, 0x0073, //latin capital letter s
	0x0054, 0x0074, //latin capital letter t
	0x0055, 0x0075, //latin capital letter u
	0x0056, 0x0076, //latin capital letter v
	0x0057, 0x0077, //latin capital letter w
	0x0058, 0x0078, //latin capital letter x
	0x0059, 0x0079, //latin capital letter y
	0x005A, 0x007A, //latin capital letter z
	0x00B5, 0x03BC, //micro sign
	0x00C0, 0x00E0, //latin capital letter a with grave
	0x00C1, 0x00E1, //latin capital letter a with acute
	0x00C2, 0x00E2, //latin capital letter a with circumflex
	0x00C3, 0x00E3, //latin capital letter a with tilde
	0x00C4, 0x00E4, //latin capital letter a with diaeresis
	0x00C5, 0x00E5, //latin capital letter a with ring above
	0x00C6, 0x00E6, //latin capital letter ae
	0x00C7, 0x00E7, //latin capital letter c with cedilla
	0x00C8, 0x00E8, //latin capital letter e with grave
	0x00C9, 0x00E9, //latin capital letter e with acute
	0x00CA, 0x00EA, //latin capital letter e with circumflex
	0x00CB, 0x00EB, //latin capital letter e with diaeresis
	0x00CC, 0x00EC, //latin capital letter i with grave
	0x00CD, 0x00ED, //latin capital letter i with acute
	0x00CE, 0x00EE, //latin capital letter i with circumflex
	0x00CF, 0x00EF, //latin capital letter i with diaeresis
	0x00D0, 0x00F0, //latin capital letter eth
	0x00D1, 0x00F1, //latin capital letter n with tilde
	0x00D2, 0x00F2, //latin capital letter o with grave
	0x00D3, 0x00F3, //latin capital letter o with acute
	0x00D4, 0x00F4, //latin capital letter o with circumflex
	0x00D5, 0x00F5, //latin capital letter o with tilde
	0x00D6, 0x00F6, //latin capital letter o with diaeresis
	0x00D8, 0x00F8, //latin capital letter o with stroke
	0x00D9, 0x00F9, //latin capital letter u with grave
	0x00DA, 0x00FA, //latin capital letter u with acute
	0x00DB, 0x00FB, //latin capital letter u with circumflex
	0x00DC, 0x00FC, //latin capital letter u with diaeresis
	0x00DD, 0x00FD, //latin capital letter y with acute
	0x00DE, 0x00FE, //latin capital letter thorn
	0x0100, 0x0101, //latin capital letter a with macron
	0x0102, 0x0103, //latin capital letter a with breve
	0x0104, 0x0105, //latin capital letter a with ogonek
	0x0106, 0x0107, //latin capital letter c with acute
	0x0108, 0x0109, //latin capital letter c with circumflex
	0x010A, 0x010B, //latin capital letter c with dot above
	0x010C, 0x010D, //latin capital letter c with caron
	0x010E, 0x010F, //latin capital letter d with caron
	0x0110, 0x0111, //latin capital letter d with stroke
	0x0112, 0x0113, //latin capital letter e with macron
	0x0114, 0x0115, //latin capital letter e with breve
	0x0116, 0x0117, //latin capital letter e with dot above
	0x0118, 0x0119, //latin capital letter e with ogonek
	0x011A, 0x011B, //latin capital letter e with caron
	0x011C, 0x011D, //latin capital letter g with circumflex
	0x011E, 0x011F, //latin capital letter g with breve
	0x0120, 0x0121, //latin capital letter g with dot above
	0x0122, 0x0123, //latin capital letter g with cedilla
	0x0124, 0x0125, //latin capital letter h with circumflex
	0x0126, 0x0127, //latin capital letter h with stroke
	0x0128, 0x0129, //latin capital letter i with tilde
	0x012A, 0x012B, //latin capital letter i with macron
	0x012C, 0x012D, //latin capital letter i with breve
	0x012E, 0x012F, //latin capital letter i with ogonek
	0x0132, 0x0133, //latin capital ligature ij
	0x0134, 0x0135, //latin capital letter j with circumflex
	0x0136, 0x0137, //latin capital letter k with cedilla
	0x0139, 0x013A, //latin capital letter l with acute
	0x013B, 0x013C, //latin capital letter l with cedilla
	0x013D, 0x013E, //latin capital letter l with caron
	0x013F, 0x0140, //latin capital letter l with middle dot
	0x0141, 0x0142, //latin capital letter l with stroke
	0x0143, 0x0144, //latin capital letter n with acute
	0x0145, 0x0146, //latin capital letter n with cedilla
	0x0147, 0x0148, //latin capital letter n with caron
	0x014A, 0x014B, //latin capital letter eng
	0x014C, 0x014D, //latin capital letter o with macron
	0x014E, 0x014F, //latin capital letter o with breve
	0x0150, 0x0151, //latin capital letter o with double acute
	0x0152, 0x0153, //latin capital ligature oe
	0x0154, 0x0155, //latin capital letter r with acute
	0x0156, 0x0157, //latin capital letter r with cedilla
	0x0158, 0x0159, //latin capital letter r with caron
	0x015A, 0x015B, //latin capital letter s with acute
	0x015C, 0x015D, //latin capital letter s with circumflex
	0x015E, 0x015F, //latin capital letter s with cedilla
	0x0160, 0x0161, //latin capital letter s with caron
	0x0162, 0x0163, //latin capital letter t with cedilla
	0x0164, 0x0165, //latin capital letter t with caron
	0x0166, 0x0167, //latin capital letter t with stroke
	0x0168, 0x0169, //latin capital letter u with tilde
	0x016A, 0x016B, //latin capital letter u with macron
	0x016C, 0x016D, //latin capital letter u with breve
	0x016E, 0x016F, //latin capital letter u with ring above
	0x0170, 0x0171, //latin capital letter u with double acute
	0x0172, 0x0173, //latin capital letter u with ogonek
	0x0174, 0x0175, //latin capital letter w with circumflex
	0x0176, 0x0177, //latin capital letter y with circumflex
	0x0178, 0x00FF, //latin capital letter y with diaeresis
	0x0179, 0x017A, //latin capital letter z with acute
	0x017B, 0x017C, //latin capital letter z with dot above
	0x017D, 0x017E, //latin capital letter z with caron
	0x017F, 0x0073, //latin small letter long s
	0x0181, 0x0253, //latin capital letter b with hook
	0x0182, 0x0183, //latin capital letter b with topbar
	0x0184, 0x0185, //latin capital letter tone six
	0x0186, 0x0254, //latin capital letter open o
	0x0187, 0x0188, //latin capital letter c with hook
	0x0189, 0x0256, //latin capital letter african d
	0x018A, 0x0257, //latin capital letter d with hook
	0x018B, 0x018C, //latin capital letter d with topbar
	0x018E, 0x01DD, //latin capital letter reversed e
	0x018F, 0x0259, //latin capital letter schwa
	0x0190, 0x025B, //latin capital letter open e
	0x0191, 0x0192, //latin capital letter f with hook
	0x0193, 0x0260, //latin capital letter g with hook
	0x0194, 0x0263, //latin capital letter gamma
	0x0196, 0x0269, //latin capital letter iota
	0x0197, 0x0268, //latin capital letter i with stroke
	0x0198, 0x0199, //latin capital letter k with hook
	0x019C, 0x026F, //latin capital letter turned m
	0x019D, 0x0272, //latin capital letter n with left hook
	0x019F, 0x0275, //latin capital letter o with middle tilde
	0x01A0, 0x01A1, //latin capital letter o with horn
	0x01A2, 0x01A3, //latin capital letter oi
	0x01A4, 0x01A5, //latin capital letter p with hook
	0x01A6, 0x0280, //latin letter yr
	0x01A7, 0x01A8, //latin capital letter tone two
	0x01A9, 0x0283, //latin capital letter esh
	0x01AC, 0x01AD, //latin capital letter t with hook
	0x01AE, 0x0288, //latin capital letter t with retroflex hook
	0x01AF, 0x01B0, //latin capital letter u with horn
	0x01B1, 0x028A, //latin capital letter upsilon
	0x01B2, 0x028B, //latin capital letter v with hook
	0x01B3, 0x01B4, //latin capital letter y with hook
	0x01B5, 0x01B6, //latin capital letter z with stroke
	0x01B7, 0x0292, //latin capital letter ezh
	0x01B8, 0x01B9, //latin capital letter ezh reversed
	0x01BC, 0x01BD, //latin capital letter tone five
	0x01C4, 0x01C6, //latin capital letter dz with caron
	0x01C5, 0x01C6, //latin capital letter d with small letter z with caron
	0x01C7, 0x01C9, //latin capital letter lj
	0x01C8, 0x01C9, //latin capital letter l with small letter j
	0x01CA, 0x01CC, //latin capital letter nj
	0x01CB, 0x01CC, //latin capital letter n with small letter j
	0x01CD, 0x01CE, //latin capital letter a with caron
	0x01CF, 0x01D0, //latin capital letter i with caron
	0x01D1, 0x01D2, //latin capital letter o with caron
	0x01D3, 0x01D4, //latin capital letter u with caron
	0x01D5, 0x01D6, //latin capital letter u with diaeresis and macron
	0x01D7, 0x01D8, //latin capital letter u with diaeresis and acute
	0x01D9, 0x01DA, //latin capital letter u with diaeresis and caron
	0x01DB, 0x01DC, //latin capital letter u with diaeresis and grave
	0x01DE, 0x01DF, //latin capital letter a with diaeresis and macron
	0x01E0, 0x01E1, //latin capital letter a with dot above and macron
	0x01E2, 0x01E3, //latin capital letter ae with macron
	0x01E4, 0x01E5, //latin capital letter g with stroke
	0x01E6, 0x01E7, //latin capital letter g with caron
	0x01E8, 0x01E9, //latin capital letter k with caron
	0x01EA, 0x01EB, //latin capital letter o with ogonek
	0x01EC, 0x01ED, //latin capital letter o with ogonek and macron
	0x01EE, 0x01EF, //latin capital letter ezh with caron
	0x01F1, 0x01F3, //latin capital letter dz
	0x01F2, 0x01F3, //latin capital letter d with small letter z
	0x01F4, 0x01F5, //latin capital letter g with acute
	0x01F6, 0x0195, //latin capital letter hwair
	0x01F7, 0x01BF, //latin capital letter wynn
	0x01F8, 0x01F9, //latin capital letter n with grave
	0x01FA, 0x01FB, //latin capital letter a with ring above and acute
	0x01FC, 0x01FD, //latin capital letter ae with acute
	0x01FE, 0x01FF, //latin capital letter o with stroke and acute
	0x0200, 0x0201, //latin capital letter a with double grave
	0x0202, 0x0203, //latin capital letter a with inverted breve
	0x0204, 0x0205, //latin capital letter e with double grave
	0x0206, 0x0207, //latin capital letter e with inverted breve
	0x0208, 0x0209, //latin capital letter i with double grave
	0x020A, 0x020B, //latin capital letter i with inverted breve
	0x020C, 0x020D, //latin capital letter o with double grave
	0x020E, 0x020F, //latin capital letter o with inverted breve
	0x0210, 0x0211, //latin capital letter r with double grave
	0x0212, 0x0213, //latin capital letter r with inverted breve
	0x0214, 0x0215, //latin capital letter u with double grave
	0x0216, 0x0217, //latin capital letter u with inverted breve
	0x0218, 0x0219, //latin capital letter s with comma below
	0x021A, 0x021B, //latin capital letter t with comma below
	0x021C, 0x021D, //latin capital letter yogh
	0x021E, 0x021F, //latin capital letter h with caron
	0x0220, 0x019E, //latin capital letter n with long right leg
	0x0222, 0x0223, //latin capital letter ou
	0x0224, 0x0225, //latin capital letter z with hook
	0x0226, 0x0227, //latin capital letter a with dot above
	0x0228, 0x0229, //latin capital letter e with cedilla
	0x022A, 0x022B, //latin capital letter o with diaeresis and macron
	0x022C, 0x022D, //latin capital letter o with tilde and macron
	0x022E, 0x022F, //latin capital letter o with dot above
	0x0230, 0x0231, //latin capital letter o with dot above and macron
	0x0232, 0x0233, //latin capital letter y with macron
	0x023A, 0x2C65, //latin capital letter a with stroke
	0x023B, 0x023C, //latin capital letter c with stroke
	0x023D, 0x019A, //latin capital letter l with bar
	0x023E, 0x2C66, //latin capital letter t with diagonal stroke
	0x0241, 0x0242, //latin capital letter glottal stop
	0x0243, 0x0180, //latin capital letter b with stroke
	0x0244, 0x0289, //latin capital letter u bar
	0x0245, 0x028C, //latin capital letter turned v
	0x0246, 0x0247, //latin capital letter e with stroke
	0x0248, 0x0249, //latin capital letter j with stroke
	0x024A, 0x024B, //latin capital letter small q with hook tail
	0x024C, 0x024D, //latin capital letter r with stroke
	0x024E, 0x024F, //latin capital letter y with stroke
	0x0345, 0x03B9, //combining greek ypogegrammeni
	0x0370, 0x0371, //greek capital letter heta
	0x0372, 0x0373, //greek capital letter archaic sampi
	0x0376, 0x0377, //greek capital letter pamphylian digamma
	0x037F, 0x03F3, //greek capital letter yot
	0x0386, 0x03AC, //greek capital letter alpha with tonos
	0x0388, 0x03AD, //greek capital letter epsilon with tonos
	0x0389, 0x03AE, //greek capital letter eta with tonos
	0x038A, 0x03AF, //greek capital letter iota with tonos
	0x038C, 0x03CC, //greek capital letter omicron with tonos
	0x038E, 0x03CD, //greek capital letter upsilon with tonos
	0x038F, 0x03CE, //greek capital letter omega with tonos
	0x0391, 0x03B1, //greek capital letter alpha
	0x0392, 0x03B2, //greek capital letter beta
	0x0393, 0x03B3, //greek capital letter gamma
	0x0394, 0x03B4, //greek capital letter delta
	0x0395, 0x03B5, //greek capital letter epsilon
	0x0396, 0x03B6, //greek capital letter zeta
	0x0397, 0x03B7, //greek capital letter eta
	0x0398, 0x03B8, //greek capital letter theta
	0x0399, 0x03B9, //greek capital letter iota
	0x039A, 0x03BA, //greek capital letter kappa
	0x039B, 0x03BB, //greek capital letter lamda
	0x039C, 0x03BC, //greek capital letter mu
	0x039D, 0x03BD, //greek capital letter nu
	0x039E, 0x03BE, //greek capital letter xi
	0x039F, 0x03BF, //greek capital letter omicron
	0x03A0, 0x03C0, //greek capital letter pi
	0x03A1, 0x03C1, //greek capital letter rho
	0x03A3, 0x03C3, //greek capital letter sigma
	0x03A4, 0x03C4, //greek capital letter tau
	0x03A5, 0x03C5, //greek capital letter upsilon
	0x03A6, 0x03C6, //greek capital letter phi
	0x03A7, 0x03C7, //greek capital letter chi
	0x03A8, 0x03C8, //greek capital letter psi
	0x03A9, 0x03C9, //greek capital letter omega
	0x03AA, 0x03CA, //greek capital letter iota with dialytika
	0x03AB, 0x03CB, //greek capital letter upsilon with dialytika
	0x03C2, 0x03C3, //greek small letter final sigma
	0x03CF, 0x03D7, //greek capital kai symbol
	0x03D0, 0x03B2, //greek beta symbol
	0x03D1, 0x03B8, //greek theta symbol
	0x03D5, 0x03C6, //greek phi symbol
	0x03D6, 0x03C0, //greek pi symbol
	0x03D8, 0x03D9, //greek letter archaic koppa
	0x03DA, 0x03DB, //greek letter stigma
	0x03DC, 0x03DD, //greek letter digamma
	0x03DE, 0x03DF, //greek letter koppa
	0x03E0, 0x03E1, //greek letter sampi
	0x03E2, 0x03E3, //coptic capital letter shei
	0x03E4, 0x03E5, //coptic capital letter fei
	0x03E6, 0x03E7, //coptic capital letter khei
	0x03E8, 0x03E9, //coptic capital letter hori
	0x03EA, 0x03EB, //coptic capital letter gangia
	0x03EC, 0x03ED, //coptic capital letter shima
	0x03EE, 0x03EF, //coptic capital letter dei
	0x03F0, 0x03BA, //greek kappa symbol
	0x03F1, 0x03C1, //greek rho symbol
	0x03F4, 0x03B8, //greek capital theta symbol
	0x03F5, 0x03B5, //greek lunate epsilon symbol
	0x03F7, 0x03F8, //greek capital letter sho
	0x03F9, 0x03F2, //greek capital lunate sigma symbol
	0x03FA, 0x03FB, //greek capital letter san
	0x03FD, 0x037B, //greek capital reversed lunate sigma symbol
	0x03FE, 0x037C, //greek capital dotted lunate sigma symbol
	0x03FF, 0x037D, //greek capital reversed dotted lunate sigma symbol
	0x0400, 0x0450, //cyrillic capital letter ie with grave
	0x0401, 0x0451, //cyrillic capital letter io
	0x0402, 0x0452, //cyrillic capital letter dje
	0x0403, 0x0453, //cyrillic capital letter gje
	0x0404, 0x0454, //cyrillic capital letter ukrainian ie
	0x0405, 0x0455, //cyrillic capital letter dze
	0x0406, 0x0456, //cyrillic capital letter byelorussian-ukrainian i
	0x0407, 0x0457, //cyrillic capital letter yi
	0x0408, 0x0458, //cyrillic capital letter je
	0x0409, 0x0459, //cyrillic capital letter lje
	0x040A, 0x045A, //cyrillic capital letter nje
	0x040B, 0x045B, //cyrillic capital letter tshe
	0x040C, 0x045C, //cyrillic capital letter kje
	0x040D, 0x045D, //cyrillic capital letter i with grave
	0x040E, 0x045E, //cyrillic capital letter short u
	0x040F, 0x045F, //cyrillic capital letter dzhe
	0x0410, 0x0430, //cyrillic capital letter a
	0x0411, 0x0431, //cyrillic capital letter be
	0x0412, 0x0432, //cyrillic capital letter ve
	0x0413, 0x0433, //cyrillic capital letter ghe
	0x0414, 0x0434, //cyrillic capital letter de
	0x0415, 0x0435, //cyrillic capital letter ie
	0x0416, 0x0436, //cyrillic capital letter zhe
	0x0417, 0x0437, //cyrillic capital letter ze
	0x0418, 0x0438, //cyrillic capital letter i
	0x0419, 0x0439, //cyrillic capital letter short i
	0x041A, 0x043A, //cyrillic capital letter ka
	0x041B, 0x043B, //cyrillic capital letter el
	0x041C, 0x043C, //cyrillic capital letter em
	0x041D, 0x043D, //cyrillic capital letter en
	0x041E, 0x043E, //cyrillic capital letter o
	0x041F, 0x043F, //cyrillic capital letter pe
	0x0420, 0x0440, //cyrillic capital letter er
	0x0421, 0x0441, //cyrillic capital letter es
	0x0422, 0x0442, //cyrillic capital letter te
	0x0423, 0x0443, //cyrillic capital letter u
	0x0424, 0x0444, //cyrillic capital letter ef
	0x0425, 0x0445, //cyrillic capital letter ha
	0x0426, 0x0446, //cyrillic capital letter tse
	0x0427, 0x0447, //cyrillic capital letter che
	0x0428, 0x0448, //cyrillic capital letter sha
	0x0429, 0x0449, //cyrillic capital letter shcha
	0x042A, 0x044A, //cyrillic capital letter hard sign
	0x042B, 0x044B, //cyrillic capital letter yeru
	0x042C, 0x044C, //cyrillic capital letter soft sign
	0x042D, 0x044D, //cyrillic capital letter e
	0x042E, 0x044E, //cyrillic capital letter yu
	0x042F, 0x044F, //cyrillic capital letter ya
	0x0460, 0x0461, //cyrillic capital letter omega
	0x0462, 0x0463, //cyrillic capital letter yat
	0x0464, 0x0465, //cyrillic capital letter iotified e
	0x0466, 0x0467, //cyrillic capital letter little yus
	0x0468, 0x0469, //cyrillic capital letter iotified little yus
	0x046A, 0x046B, //cyrillic capital letter big yus
	0x046C, 0x046D, //cyrillic capital letter iotified big yus
	0x046E, 0x046F, //cyrillic capital letter ksi
	0x0470, 0x0471, //cyrillic capital letter psi
	0x0472, 0x0473, //cyrillic capital letter fita
	0x0474, 0x0475, //cyrillic capital letter izhitsa
	0x0476, 0x0477, //cyrillic capital letter izhitsa with double grave accent
	0x0478, 0x0479, //cyrillic capital letter uk
	0x047A, 0x047B, //cyrillic capital letter round omega
	0x047C, 0x047D, //cyrillic capital letter omega with titlo
	0x047E, 0x047F, //cyrillic capital letter ot
	0x0480, 0x0481, //cyrillic capital letter koppa
	0x048A, 0x048B, //cyrillic capital letter short i with tail
	0x048C, 0x048D, //cyrillic capital letter semisoft sign
	0x048E, 0x048F, //cyrillic capital letter er with tick
	0x0490, 0x0491, //cyrillic capital letter ghe with upturn
	0x0492, 0x0493, //cyrillic capital letter ghe with stroke
	0x0494, 0x0495, //cyrillic capital letter ghe with middle hook
	0x0496, 0x0497, //cyrillic capital letter zhe with descender
	0x0498, 0x0499, //cyrillic capital letter ze with descender
	0x049A, 0x049B, //cyrillic capital letter ka with descender
	0x049C, 0x049D, //cyrillic capital letter ka with vertical stroke
	0x049E, 0x049F, //cyrillic capital letter ka with stroke
	0x04A0, 0x04A1, //cyrillic capital letter bashkir ka
	0x04A2, 0x04A3, //cyrillic capital letter en with descender
	0x04A4, 0x04A5, //cyrillic capital ligature en ghe
	0x04A6, 0x04A7, //cyrillic capital letter pe with middle hook
	0x04A8, 0x04A9, //cyrillic capital letter abkhasian ha
	0x04AA, 0x04AB, //cyrillic capital letter es with descender
	0x04AC, 0x04AD, //cyrillic capital letter te with descender
	0x04AE, 0x04AF, //cyrillic capital letter straight u
	0x04B0, 0x04B1, //cyrillic capital letter straight u with stroke
	0x04B2, 0x04B3, //cyrillic capital letter ha with descender
	0x04B4, 0x04B5, //cyrillic capital ligature te tse
	0x04B6, 0x04B7, //cyrillic capital letter che with descender
	0x04B8, 0x04B9, //cyrillic capital letter che with vertical stroke
	0x04BA, 0x04BB, //cyrillic capital letter shha
	0x04BC, 0x04BD, //cyrillic capital letter abkhasian che
	0x04BE, 0x04BF, //cyrillic capital letter abkhasian che with descender
	0x04C0, 0x04CF, //cyrillic letter palochka
	0x04C1, 0x04C2, //cyrillic capital letter zhe with breve
	0x04C3, 0x04C4, //cyrillic capital letter ka with hook
	0x04C5, 0x04C6, //cyrillic capital letter el with tail
	0x04C7, 0x04C8, //cyrillic capital letter en with hook
	0x04C9, 0x04CA, //cyrillic capital letter en with tail
	0x04CB, 0x04CC, //cyrillic capital letter khakassian che
	0x04CD, 0x04CE, //cyrillic capital letter em with tail
	0x04D0, 0x04D1, //cyrillic capital letter a with breve
	0x04D2, 0x04D3, //cyrillic capital letter a with diaeresis
	0x04D4, 0x04D5, //cyrillic capital ligature a ie
	0x04D6, 0x04D7, //cyrillic capital letter ie with breve
	0x04D8, 0x04D9, //cyrillic capital letter schwa
	0x04DA, 0x04DB, //cyrillic capital letter schwa with diaeresis
	0x04DC, 0x04DD, //cyrillic capital letter zhe with diaeresis
	0x04DE, 0x04DF, //cyrillic capital letter ze with diaeresis
	0x04E0, 0x04E1, //cyrillic capital letter abkhasian dze
	0x04E2, 0x04E3, //cyrillic capital letter i with macron
	0x04E4, 0x04E5, //cyrillic capital letter i with diaeresis
	0x04E6, 0x04E7, //cyrillic capital letter o with diaeresis
	0x04E8, 0x04E9, //cyrillic capital letter barred o
	0x04EA, 0x04EB, //cyrillic capital letter barred o with diaeresis
	0x04EC, 0x04ED, //cyrillic capital letter e with diaeresis
	0x04EE, 0x04EF, //cyrillic capital letter u with macron
	0x04F0, 0x04F1, //cyrillic capital letter u with diaeresis
	0x04F2, 0x04F3, //cyrillic capital letter u with double acute
	0x04F4, 0x04F5, //cyrillic capital letter che with diaeresis
	0x04F6, 0x04F7, //cyrillic capital letter ghe with descender
	0x04F8, 0x04F9, //cyrillic capital letter yeru with diaeresis
	0x04FA, 0x04FB, //cyrillic capital letter ghe with stroke and hook
	0x04FC, 0x04FD, //cyrillic capital letter ha with hook
	0x04FE, 0x04FF, //cyrillic capital letter ha with stroke
	0x0500, 0x0501, //cyrillic capital letter komi de
	0x0502, 0x0503, //cyrillic capital letter komi dje
	0x0504, 0x0505, //cyrillic capital letter komi zje
	0x0506, 0x0507, //cyrillic capital letter komi dzje
	0x0508, 0x0509, //cyrillic capital letter komi lje
	0x050A, 0x050B, //cyrillic capital letter komi nje
	0x050C, 0x050D, //cyrillic capital letter komi sje
	0x050E, 0x050F, //cyrillic capital letter komi tje
	0x0510, 0x0511, //cyrillic capital letter reversed ze
	0x0512, 0x0513, //cyrillic capital letter el with hook
	0x0514, 0x0515, //cyrillic capital letter lha
	0x0516, 0x0517, //cyrillic capital letter rha
	0x0518, 0x0519, //cyrillic capital letter yae
	0x051A, 0x051B, //cyrillic capital letter qa
	0x051C, 0x051D, //cyrillic capital letter we
	0x051E, 0x051F, //cyrillic capital letter aleut ka
	0x0520, 0x0521, //cyrillic capital letter el with middle hook
	0x0522, 0x0523, //cyrillic capital letter en with middle hook
	0x0524, 0x0525, //cyrillic capital letter pe with descender
	0x0526, 0x0527, //cyrillic capital letter shha with descender
	0x0528, 0x0529, //cyrillic capital letter en with left hook
	0x052A, 0x052B, //cyrillic capital letter dzzhe
	0x052C, 0x052D, //cyrillic capital letter dche
	0x052E, 0x052F, //cyrillic capital letter el with descender
	0x0531, 0x0561, //armenian capital letter ayb
	0x0532, 0x0562, //armenian capital letter ben
	0x0533, 0x0563, //armenian capital letter gim
	0x0534, 0x0564, //armenian capital letter da
	0x0535, 0x0565, //armenian capital letter ech
	0x0536, 0x0566, //armenian capital letter za
	0x0537, 0x0567, //armenian capital letter eh
	0x0538, 0x0568, //armenian capital letter et
	0x0539, 0x0569, //armenian capital letter to
	0x053A, 0x056A, //armenian capital letter zhe
	0x053B, 0x056B, //armenian capital letter ini
	0x053C, 0x056C, //armenian capital letter liwn
	0x053D, 0x056D, //armenian capital letter xeh
	0x053E, 0x056E, //armenian capital letter ca
	0x053F, 0x056F, //armenian capital letter ken
	0x0540, 0x0570, //armenian capital letter ho
	0x0541, 0x0571, //armenian capital letter ja
	0x0542, 0x0572, //armenian capital letter ghad
	0x0543, 0x0573, //armenian capital letter cheh
	0x0544, 0x0574, //armenian capital letter men
	0x0545, 0x0575, //armenian capital letter yi
	0x0546, 0x0576, //armenian capital letter now
	0x0547, 0x0577, //armenian capital letter sha
	0x0548, 0x0578, //armenian capital letter vo
	0x0549, 0x0579, //armenian capital letter cha
	0x054A, 0x057A, //armenian capital letter peh
	0x054B, 0x057B, //armenian capital letter jheh
	0x054C, 0x057C, //armenian capital letter ra
	0x054D, 0x057D, //armenian capital letter seh
	0x054E, 0x057E, //armenian capital letter vew
	0x054F, 0x057F, //armenian capital letter tiwn
	0x0550, 0x0580, //armenian capital letter reh
	0x0551, 0x0581, //armenian capital letter co
	0x0552, 0x0582, //armenian capital letter yiwn
	0x0553, 0x0583, //armenian capital letter piwr
	0x0554, 0x0584, //armenian capital letter keh
	0x0555, 0x0585, //armenian capital letter oh
	0x0556, 0x0586, //armenian capital letter feh
	0x10A0, 0x2D00, //georgian capital letter an
	0x10A1, 0x2D01, //georgian capital letter ban
	0x10A2, 0x2D02, //georgian capital letter gan
	0x10A3, 0x2D03, //georgian capital letter don
	0x10A4, 0x2D04, //georgian capital letter en
	0x10A5, 0x2D05, //georgian capital letter vin
	0x10A6, 0x2D06, //georgian capital letter zen
	0x10A7, 0x2D07, //georgian capital letter tan
	0x10A8, 0x2D08, //georgian capital letter in
	0x10A9, 0x2D09, //georgian capital letter kan
	0x10AA, 0x2D0A, //georgian capital letter las
	0x10AB, 0x2D0B, //georgian capital letter man
	0x10AC, 0x2D0C, //georgian capital letter nar
	0x10AD, 0x2D0D, //georgian capital letter on
	0x10AE, 0x2D0E, //georgian capital letter par
	0x10AF, 0x2D0F, //georgian capital letter zhar
	0x10B0, 0x2D10, //georgian capital letter rae
	0x10B1, 0x2D11, //georgian capital letter san
	0x10B2, 0x2D12, //georgian capital letter tar
	0x10B3, 0x2D13, //georgian capital letter un
	0x10B4, 0x2D14, //georgian capital letter phar
	0x10B5, 0x2D15, //georgian capital letter khar
	0x10B6, 0x2D16, //georgian capital letter ghan
	0x10B7, 0x2D17, //georgian capital letter qar
	0x10B8, 0x2D18, //georgian capital letter shin
	0x10B9, 0x2D19, //georgian capital letter chin
	0x10BA, 0x2D1A, //georgian capital letter can
	0x10BB, 0x2D1B, //georgian capital letter jil
	0x10BC, 0x2D1C, //georgian capital letter cil
	0x10BD, 0x2D1D, //georgian capital letter char
	0x10BE, 0x2D1E, //georgian capital letter xan
	0x10BF, 0x2D1F, //georgian capital letter jhan
	0x10C0, 0x2D20, //georgian capital letter hae
	0x10C1, 0x2D21, //georgian capital letter he
	0x10C2, 0x2D22, //georgian capital letter hie
	0x10C3, 0x2D23, //georgian capital letter we
	0x10C4, 0x2D24, //georgian capital letter har
	0x10C5, 0x2D25, //georgian capital letter hoe
	0x10C7, 0x2D27, //georgian capital letter yn
	0x10CD, 0x2D2D, //georgian capital letter aen
	0x13F8, 0x13F0, //cherokee small letter ye
	0x13F9, 0x13F1, //cherokee small letter yi
	0x13FA, 0x13F2, //cherokee small letter yo
	0x13FB, 0x13F3, //cherokee small letter yu
	0x13FC, 0x13F4, //cherokee small letter yv
	0x13FD, 0x13F5, //cherokee small letter mv
	0x1C80, 0x0432, //cyrillic small letter rounded ve
	0x1C81, 0x0434, //cyrillic small letter long-legged de
	0x1C82, 0x043E, //cyrillic small letter narrow o
	0x1C83, 0x0441, //cyrillic small letter wide es
	0x1C84, 0x0442, //cyrillic small letter tall te
	0x1C85, 0x0442, //cyrillic small letter three-legged te
	0x1C86, 0x044A, //cyrillic small letter tall hard sign
	0x1C87, 0x0463, //cyrillic small letter tall yat
	0x1C88, 0xA64B, //cyrillic small letter unblended uk
	0x1C90, 0x10D0, //georgian mtavruli capital letter an
	0x1C91, 0x10D1, //georgian mtavruli capital letter ban
	0x1C92, 0x10D2, //georgian mtavruli capital letter gan
	0x1C93, 0x10D3, //georgian mtavruli capital letter don
	0x1C94, 0x10D4, //georgian mtavruli capital letter en
	0x1C95, 0x10D5, //georgian mtavruli capital letter vin
	0x1C96, 0x10D6, //georgian mtavruli capital letter zen
	0x1C97, 0x10D7, //georgian mtavruli capital letter tan
	0x1C98, 0x10D8, //georgian mtavruli capital letter in
	0x1C99, 0x10D9, //georgian mtavruli capital letter kan
	0x1C9A, 0x10DA, //georgian mtavruli capital letter las
	0x1C9B, 0x10DB, //georgian mtavruli capital letter man
	0x1C9C, 0x10DC, //georgian mtavruli capital letter nar
	0x1C9D, 0x10DD, //georgian mtavruli capital letter on
	0x1C9E, 0x10DE, //georgian mtavruli capital letter par
	0x1C9F, 0x10DF, //georgian mtavruli capital letter zhar
	0x1CA0, 0x10E0, //georgian mtavruli capital letter rae
	0x1CA1, 0x10E1, //georgian mtavruli capital letter san
	0x1CA2, 0x10E2, //georgian mtavruli capital letter tar
	0x1CA3, 0x10E3, //georgian mtavruli capital letter un
	0x1CA4, 0x10E4, //georgian mtavruli capital letter phar
	0x1CA5, 0x10E5, //georgian mtavruli capital letter khar
	0x1CA6, 0x10E6, //georgian mtavruli capital letter ghan
	0x1CA7, 0x10E7, //georgian mtavruli capital letter qar
	0x1CA8, 0x10E8, //georgian mtavruli capital letter shin
	0x1CA9, 0x10E9, //georgian mtavruli capital letter chin
	0x1CAA, 0x10EA, //georgian mtavruli capital letter can
	0x1CAB, 0x10EB, //georgian mtavruli capital letter jil
	0x1CAC, 0x10EC, //georgian mtavruli capital letter cil
	0x1CAD, 0x10ED, //georgian mtavruli capital letter char
	0x1CAE, 0x10EE, //georgian mtavruli capital letter xan
	0x1CAF, 0x10EF, //georgian mtavruli capital letter jhan
	0x1CB0, 0x10F0, //georgian mtavruli capital letter hae
	0x1CB1, 0x10F1, //georgian mtavruli capital letter he
	0x1CB2, 0x10F2, //georgian mtavruli capital letter hie
	0x1CB3, 0x10F3, //georgian mtavruli capital letter we
	0x1CB4, 0x10F4, //georgian mtavruli capital letter har
	0x1CB5, 0x10F5, //georgian mtavruli capital letter hoe
	0x1CB6, 0x10F6, //georgian mtavruli capital letter fi
	0x1CB7, 0x10F7, //georgian mtavruli capital letter yn
	0x1CB8, 0x10F8, //georgian mtavruli capital letter elifi
	0x1CB9, 0x10F9, //georgian mtavruli capital letter turned gan
	0x1CBA, 0x10FA, //georgian mtavruli capital letter ain
	0x1CBD, 0x10FD, //georgian mtavruli capital letter aen
	0x1CBE, 0x10FE, //georgian mtavruli capital letter hard sign
	0x1CBF, 0x10FF, //georgian mtavruli capital letter labial sign
	0x1E00, 0x1E01, //latin capital letter a with ring below
	0x1E02, 0x1E03, //latin capital letter b with dot above
	0x1E04, 0x1E05, //latin capital letter b with dot below
	0x1E06, 0x1E07, //latin capital letter b with line below
	0x1E08, 0x1E09, //latin capital letter c with cedilla and acute
	0x1E0A, 0x1E0B, //latin capital letter d with dot above
	0x1E0C, 0x1E0D, //latin capital letter d with dot below
	0x1E0E, 0x1E0F, //latin capital letter d with line below
	0x1E10, 0x1E11, //latin capital letter d with cedilla
	0x1E12, 0x1E13, //latin capital letter d with circumflex below
	0x1E14, 0x1E15, //latin capital letter e with macron and grave
	0x1E16, 0x1E17, //latin capital letter e with macron and acute
	0x1E18, 0x1E19, //latin capital letter e with circumflex below
	0x1E1A, 0x1E1B, //latin capital letter e with tilde below
	0x1E1C, 0x1E1D, //latin capital letter e with cedilla and breve
	0x1E1E, 0x1E1F, //latin capital letter f with dot above
	0x1E20, 0x1E21, //latin capital letter g with macron
	0x1E22, 0x1E23, //latin capital letter h with dot above
	0x1E24, 0x1E25, //latin capital letter h with dot below
	0x1E26, 0x1E27, //latin capital letter h with diaeresis
	0x1E28, 0x1E29, //latin capital letter h with cedilla
	0x1E2A, 0x1E2B, //latin capital letter h with breve below
	0x1E2C, 0x1E2D, //latin capital letter i with tilde below
	0x1E2E, 0x1E2F, //latin capital letter i with diaeresis and acute
	0x1E30, 0x1E31, //latin capital letter k with acute
	0x1E32, 0x1E33, //latin capital letter k with dot below
	0x1E34, 0x1E35, //latin capital letter k with line below
	0x1E36, 0x1E37, //latin capital letter l with dot below
	0x1E38, 0x1E39, //latin capital letter l with dot below and macron
	0x1E3A, 0x1E3B, //latin capital letter l with line below
	0x1E3C, 0x1E3D, //latin capital letter l with circumflex below
	0x1E3E, 0x1E3F, //latin capital letter m with acute
	0x1E40, 0x1E41, //latin capital letter m with dot above
	0x1E42, 0x1E43, //latin capital letter m with dot below
	0x1E44, 0x1E45, //latin capital letter n with dot above
	0x1E46, 0x1E47, //latin capital letter n with dot below
	0x1E48, 0x1E49, //latin capital letter n with line below
	0x1E4A, 0x1E4B, //latin capital letter n with circumflex below
	0x1E4C, 0x1E4D, //latin capital letter o with tilde and acute
	0x1E4E, 0x1E4F, //latin capital letter o with tilde and diaeresis
	0x1E50, 0x1E51, //latin capital letter o with macron and grave
	0x1E52, 0x1E53, //latin capital letter o with macron and acute
	0x1E54, 0x1E55, //latin capital letter p with acute
	0x1E56, 0x1E57, //latin capital letter p with dot above
	0x1E58, 0x1E59, //latin capital letter r with dot above
	0x1E5A, 0x1E5B, //latin capital letter r with dot below
	0x1E5C, 0x1E5D, //latin capital letter r with dot below and macron
	0x1E5E, 0x1E5F, //latin capital letter r with line below
	0x1E60, 0x1E61, //latin capital letter s with dot above
	0x1E62, 0x1E63, //latin capital letter s with dot below
	0x1E64, 0x1E65, //latin capital letter s with acute and dot above
	0x1E66, 0x1E67, //latin capital letter s with caron and dot above
	0x1E68, 0x1E69, //latin capital letter s with dot below and dot above
	0x1E6A, 0x1E6B, //latin capital letter t with dot above
	0x1E6C, 0x1E6D, //latin capital letter t with dot below
	0x1E6E, 0x1E6F, //latin capital letter t with line below
	0x1E70, 0x1E71, //latin capital letter t with circumflex below
	0x1E72, 0x1E73, //latin capital letter u with diaeresis below
	0x1E74, 0x1E75, //latin capital letter u with tilde below
	0x1E76, 0x1E77, //latin capital letter u with circumflex below
	0x1E78, 0x1E79, //latin capital letter u with tilde and acute
	0x1E7A, 0x1E7B, //latin capital letter u with macron and diaeresis
	0x1E7C, 0x1E7D, //latin capital letter v with tilde
	0x1E7E, 0x1E7F, //latin capital letter v with dot below
	0x1E80, 0x1E81, //latin capital letter w with grave
	0x1E82, 0x1E83, //latin capital letter w with acute
	0x1E84, 0x1E85, //latin capital letter w with diaeresis
	0x1E86, 0x1E87, //latin capital letter w with dot above
	0x1E88, 0x1E89, //latin capital letter w with dot below
	0x1E8A, 0x1E8B, //latin capital letter x with dot above
	0x1E8C, 0x1E8D, //latin capital letter x with diaeresis
	0x1E8E, 0x1E8F, //latin capital letter y with dot above
	0x1E90, 0x1E91, //latin capital letter z with circumflex
	0x1E92, 0x1E93, //latin capital letter z with dot below
	0x1E94, 0x1E95, //latin capital letter z with line below
	0x1E9B, 0x1E61, //latin small letter long s with dot above
	0x1E9E, 0x00DF, //latin capital letter sharp s
	0x1EA0, 0x1EA1, //latin capital letter a with dot below
	0x1EA2, 0x1EA3, //latin capital letter a with hook above
	0x1EA4, 0x1EA5, //latin capital letter a with circumflex and acute
	0x1EA6, 0x1EA7, //latin capital letter a with circumflex and grave
	0x1EA8, 0x1EA9, //latin capital letter a with circumflex and hook above
	0x1EAA, 0x1EAB, //latin capital letter a with circumflex and tilde
	0x1EAC, 0x1EAD, //latin capital letter a with circumflex and dot below
	0x1EAE, 0x1EAF, //latin capital letter a with breve and acute
	0x1EB0, 0x1EB1, //latin capital letter a with breve and grave
	0x1EB2, 0x1EB3, //latin capital letter a with breve and hook above
	0x1EB4, 0x1EB5, //latin capital letter a with breve and tilde
	0x1EB6, 0x1EB7, //latin capital letter a with breve and dot below
	0x1EB8, 0x1EB9, //latin capital letter e with dot below
	0x1EBA, 0x1EBB, //latin capital letter e with hook above
	0x1EBC, 0x1EBD, //latin capital letter e with tilde
	0x1EBE, 0x1EBF, //latin capital letter e with circumflex and acute
	0x1EC0, 0x1EC1, //latin capital letter e with circumflex and grave
	0x1EC2, 0x1EC3, //latin capital letter e with circumflex and hook above
	0x1EC4, 0x1EC5, //latin capital letter e with circumflex and tilde
	0x1EC6, 0x1EC7, //latin capital letter e with circumflex and dot below
	0x1EC8, 0x1EC9, //latin capital letter i with hook above
	0x1ECA, 0x1ECB, //latin capital letter i with dot below
	0x1ECC, 0x1ECD, //latin capital letter o with dot below
	0x1ECE, 0x1ECF, //latin capital letter o with hook above
	0x1ED0, 0x1ED1, //latin capital letter o with circumflex and acute
	0x1ED2, 0x1ED3, //latin capital letter o with circumflex and grave
	0x1ED4, 0x1ED5, //latin capital letter o with circumflex and hook above
	0x1ED6, 0x1ED7, //latin capital letter o with circumflex and tilde
	0x1ED8, 0x1ED9, //latin capital letter o with circumflex and dot below
	0x1EDA, 0x1EDB, //latin capital letter o with horn and acute
	0x1EDC, 0x1EDD, //latin capital letter o with horn and grave
	0x1EDE, 0x1EDF, //latin capital letter o with horn and hook above
	0x1EE0, 0x1EE1, //latin capital letter o with horn and tilde
	0x1EE2, 0x1EE3, //latin capital letter o with horn and dot below
	0x1EE4, 0x1EE5, //latin capital letter u with dot below
	0x1EE6, 0x1EE7, //latin capital letter u with hook above
	0x1EE8, 0x1EE9, //latin capital letter u with horn and acute
	0x1EEA, 0x1EEB, //latin capital letter u with horn and grave
	0x1EEC, 0x1EED, //latin capital letter u with horn and hook above
	0x1EEE, 0x1EEF, //latin capital letter u with horn and tilde
	0x1EF0, 0x1EF1, //latin capital letter u with horn and dot below
	0x1EF2, 0x1EF3, //latin capital letter y with grave
	0x1EF4, 0x1EF5, //latin capital letter y with dot below
	0x1EF6, 0x1EF7, //latin capital letter y with hook above
	0x1EF8, 0x1EF9, //latin capital letter y with tilde
	0x1EFA, 0x1EFB, //latin capital letter middle-welsh ll
	0x1EFC, 0x1EFD, //latin capital letter middle-welsh v
	0x1EFE, 0x1EFF, //latin capital letter y with loop
	0x1F08, 0x1F00, //greek capital letter alpha with psili
	0x1F09, 0x1F01, //greek capital letter alpha with dasia
	0x1F0A, 0x1F02, //greek capital letter alpha with psili and varia
	0x1F0B, 0x1F03, //greek capital letter alpha with dasia and varia
	0x1F0C, 0x1F04, //greek capital letter alpha with psili and oxia
	0x1F0D, 0x1F05, //greek capital letter alpha with dasia and oxia
	0x1F0E, 0x1F06, //greek capital letter alpha with psili and perispomeni
	0x1F0F, 0x1F07, //greek capital letter alpha with dasia and perispomeni
	0x1F18, 0x1F10, //greek capital letter epsilon with psili
	0x1F19, 0x1F11, //greek capital letter epsilon with dasia
	0x1F1A, 0x1F12, //greek capital letter epsilon with psili and varia
	0x1F1B, 0x1F13, //greek capital letter epsilon with dasia and varia
	0x1F1C, 0x1F14, //greek capital letter epsilon with psili and oxia
	0x1F1D, 0x1F15, //greek capital letter epsilon with dasia and oxia
	0x1F28, 0x1F20, //greek capital letter eta with psili
	0x1F29, 0x1F21, //greek capital letter eta with dasia
	0x1F2A, 0x1F22, //greek capital letter eta with psili and varia
	0x1F2B, 0x1F23, //greek capital letter eta with dasia and varia
	0x1F2C, 0x1F24, //greek capital letter eta with psili and oxia
	0x1F2D, 0x1F25, //greek capital letter eta with dasia and oxia
	0x1F2E, 0x1F26, //greek capital letter eta with psili and perispomeni
	0x1F2F, 0x1F27, //greek capital letter eta with dasia and perispomeni
	0x1F38, 0x1F30, //greek capital letter iota with psili
	0x1F39, 0x1F31, //greek capital letter iota with dasia
	0x1F3A, 0x1F32, //greek capital letter iota with psili and varia
	0x1F3B, 0x1F33, //greek capital letter iota with dasia and varia
	0x1F3C, 0x1F34, //greek capital letter iota with psili and oxia
	0x1F3D, 0x1F35, //greek capital letter iota with dasia and oxia
	0x1F3E, 0x1F36, //greek capital letter iota with psili and perispomeni
	0x1F3F, 0x1F37, //greek capital letter iota with dasia and perispomeni
	0x1F48, 0x1F40, //greek capital letter omicron with psili
	0x1F49, 0x1F41, //greek capital letter omicron with dasia
	0x1F4A, 0x1F42, //greek capital letter omicron with psili and varia
	0x1F4B, 0x1F43, //greek capital letter omicron with dasia and varia
	0x1F4C, 0x1F44, //greek capital letter omicron with psili and oxia
	0x1F4D, 0x1F45, //greek capital letter omicron with dasia and oxia
	0x1F59, 0x1F51, //greek capital letter upsilon with dasia
	0x1F5B, 0x1F53, //greek capital letter upsilon with dasia and varia
	0x1F5D, 0x1F55, //greek capital letter upsilon with dasia and oxia
	0x1F5F, 0x1F57, //greek capital letter upsilon with dasia and perispomeni
	0x1F68, 0x1F60, //greek capital letter omega with psili
	0x1F69, 0x1F61, //greek capital letter omega with dasia
	0x1F6A, 0x1F62, //greek capital letter omega with psili and varia
	0x1F6B, 0x1F63, //greek capital letter omega with dasia and varia
	0x1F6C, 0x1F64, //greek capital letter omega with psili and oxia
	0x1F6D, 0x1F65, //greek capital letter omega with dasia and oxia
	0x1F6E, 0x1F66, //greek capital letter omega with psili and perispomeni
	0x1F6F, 0x1F67, //greek capital letter omega with dasia and perispomeni
	0x1F88, 0x1F80, //greek capital letter alpha with psili and prosgegrammeni
	0x1F89, 0x1F81, //greek capital letter alpha with dasia and prosgegrammeni
	0x1F8A, 0x1F82, //greek capital letter alpha with psili and varia and prosgegrammeni
	0x1F8B, 0x1F83, //greek capital letter alpha with dasia and varia and prosgegrammeni
	0x1F8C, 0x1F84, //greek capital letter alpha with psili and oxia and prosgegrammeni
	0x1F8D, 0x1F85, //greek capital letter alpha with dasia and oxia and prosgegrammeni
	0x1F8E, 0x1F86, //greek capital letter alpha with psili and perispomeni and prosgegrammeni
	0x1F8F, 0x1F87, //greek capital letter alpha with dasia and perispomeni and prosgegrammeni
	0x1F98, 0x1F90, //greek capital letter eta with psili and prosgegrammeni
	0x1F99, 0x1F91, //greek capital letter eta with dasia and prosgegrammeni
	0x1F9A, 0x1F92, //greek capital letter eta with psili and varia and prosgegrammeni
	0x1F9B, 0x1F93, //greek capital letter eta with dasia and varia and prosgegrammeni
	0x1F9C, 0x1F94, //greek capital letter eta with psili and oxia and prosgegrammeni
	0x1F9D, 0x1F95, //greek capital letter eta with dasia and oxia and prosgegrammeni
	0x1F9E, 0x1F96, //greek capital letter eta with psili and perispomeni and prosgegrammeni
	0x1F9F, 0x1F97, //greek capital letter eta with dasia and perispomeni and prosgegrammeni
	0x1FA8, 0x1FA0, //greek capital letter omega with psili and prosgegrammeni
	0x1FA9, 0x1FA1, //greek capital letter omega with dasia and prosgegrammeni
	0x1FAA, 0x1FA2, //greek capital letter omega with psili and varia and prosgegrammeni
	0x1FAB, 0x1FA3, //greek capital letter omega with dasia and varia and prosgegrammeni
	0x1FAC, 0x1FA4, //greek capital letter omega with psili and oxia and prosgegrammeni
	0x1FAD, 0x1FA5, //greek capital letter omega with dasia and oxia and prosgegrammeni
	0x1FAE, 0x1FA6, //greek capital letter omega with psili and perispomeni and prosgegrammeni
	0x1FAF, 0x1FA7, //greek capital letter omega with dasia and perispomeni and prosgegrammeni
	0x1FB8, 0x1FB0, //greek capital letter alpha with vrachy
	0x1FB9, 0x1FB1, //greek capital letter alpha with macron
	0x1FBA, 0x1F70, //greek capital letter alpha with varia
	0x1FBB, 0x1F71, //greek capital letter alpha with oxia
	0x1FBC, 0x1FB3, //greek capital letter alpha with prosgegrammeni
	0x1FBE, 0x03B9, //greek prosgegrammeni
	0x1FC8, 0x1F72, //greek capital letter epsilon with varia
	0x1FC9, 0x1F73, //greek capital letter epsilon with oxia
	0x1FCA, 0x1F74, //greek capital letter eta with varia
	0x1FCB, 0x1F75, //greek capital letter eta with oxia
	0x1FCC, 0x1FC3, //greek capital letter eta with prosgegrammeni
	0x1FD8, 0x1FD0, //greek capital letter iota with vrachy
	0x1FD9, 0x1FD1, //greek capital letter iota with macron
	0x1FDA, 0x1F76, //greek capital letter iota with varia
	0x1FDB, 0x1F77, //greek capital letter iota with oxia
	0x1FE8, 0x1FE0, //greek capital letter upsilon with vrachy
	0x1FE9, 0x1FE1, //greek capital letter upsilon with macron
	0x1FEA, 0x1F7A, //greek capital letter upsilon with varia
	0x1FEB, 0x1F7B, //greek capital letter upsilon with oxia
	0x1FEC, 0x1FE5, //greek capital letter rho with dasia
	0x1FF8, 0x1F78, //greek capital letter omicron with varia
	0x1FF9, 0x1F79, //greek capital letter omicron with oxia
	0x1FFA, 0x1F7C, //greek capital letter omega with varia
	0x1FFB, 0x1F7D, //greek capital letter omega with oxia
	0x1FFC, 0x1FF3, //greek capital letter omega with prosgegrammeni
	0x2126, 0x03C9, //ohm sign
	0x212A, 0x006B, //kelvin sign
	0x212B, 0x00E5, //angstrom sign
	0x2132, 0x214E, //turned capital f
	0x2160, 0x2170, //roman numeral one
	0x2161, 0x2171, //roman numeral two
	0x2162, 0x2172, //roman numeral three
	0x2163, 0x2173, //roman numeral four
	0x2164, 0x2174, //roman numeral five
	0x2165, 0x2175, //roman numeral six
	0x2166, 0x2176, //roman numeral seven
	0x2167, 0x2177, //roman numeral eight
	0x2168, 0x2178, //roman numeral nine
	0x2169, 0x2179, //roman numeral ten
	0x216A, 0x217A, //roman numeral eleven
	0x216B, 0x217B, //roman numeral twelve
	0x216C, 0x217C, //roman numeral fifty
	0x216D, 0x217D, //roman numeral one hundred
	0x216E, 0x217E, //roman numeral five hundred
	0x216F, 0x217F, //roman numeral one thousand
	0x2183, 0x2184, //roman numeral reversed one hundred
	0x24B6, 0x24D0, //circled latin capital letter a
	0x24B7, 0x24D1, //circled latin capital letter b
	0x24B8, 0x24D2, //circled latin capital letter c
	0x24B9, 0x24D3, //circled latin capital letter d
	0x24BA, 0x24D4, //circled latin capital letter e
	0x24BB, 0x24D5, //circled latin capital letter f
	0x24BC, 0x24D6, //circled latin capital letter g
	0x24BD, 0x24D7, //circled latin capital letter h
	0x24BE, 0x24D8, //circled latin capital letter i
	0x24BF, 0x24D9, //circled latin capital letter j
	0x24C0, 0x24DA, //circled latin capital letter k
	0x24C1, 0x24DB, //circled latin capital letter l
	0x24C2, 0x24DC, //circled latin capital letter m
	0x24C3, 0x24DD, //circled latin capital letter n
	0x24C4, 0x24DE, //circled latin capital letter o
	0x24C5, 0x24DF, //circled latin capital letter p
	0x24C6, 0x24E0, //circled latin capital letter q
	0x24C7, 0x24E1, //circled latin capital letter r
	0x24C8, 0x24E2, //circled latin capital letter s
	0x24C9, 0x24E3, //circled latin capital letter t
	0x24CA, 0x24E4, //circled latin capital letter u
	0x24CB, 0x24E5, //circled latin capital letter v
	0x24CC, 0x24E6, //circled latin capital letter w
	0x24CD, 0x24E7, //circled latin capital letter x
	0x24CE, 0x24E8, //circled latin capital letter y
	0x24CF, 0x24E9, //circled latin capital letter z
	0x2C00, 0x2C30, //glagolitic capital letter azu
	0x2C01, 0x2C31, //glagolitic capital letter buky
	0x2C02, 0x2C32, //glagolitic capital letter vede
	0x2C03, 0x2C33, //glagolitic capital letter glagoli
	0x2C04, 0x2C34, //glagolitic capital letter dobro
	0x2C05, 0x2C35, //glagolitic capital letter yestu
	0x2C06, 0x2C36, //glagolitic capital letter zhivete
	0x2C07, 0x2C37, //glagolitic capital letter dzelo
	0x2C08, 0x2C38, //glagolitic capital letter zemlja
	0x2C09, 0x2C39, //glagolitic capital letter izhe
	0x2C0A, 0x2C3A, //glagolitic capital letter initial izhe
	0x2C0B, 0x2C3B, //glagolitic capital letter i
	0x2C0C, 0x2C3C, //glagolitic capital letter djervi
	0x2C0D, 0x2C3D, //glagolitic capital letter kako
	0x2C0E, 0x2C3E, //glagolitic capital letter ljudije
	0x2C0F, 0x2C3F, //glagolitic capital letter myslite
	0x2C10, 0x2C40, //glagolitic capital letter nashi
	0x2C11, 0x2C41, //glagolitic capital letter onu
	0x2C12, 0x2C42, //glagolitic capital letter pokoji
	0x2C13, 0x2C43, //glagolitic capital letter ritsi
	0x2C14, 0x2C44, //glagolitic capital letter slovo
	0x2C15, 0x2C45, //glagolitic capital letter tvrido
	0x2C16, 0x2C46, //glagolitic capital letter uku
	0x2C17, 0x2C47, //glagolitic capital letter fritu
	0x2C18, 0x2C48, //glagolitic capital letter heru
	0x2C19, 0x2C49, //glagolitic capital letter otu
	0x2C1A, 0x2C4A, //glagolitic capital letter pe
	0x2C1B, 0x2C4B, //glagolitic capital letter shta
	0x2C1C, 0x2C4C, //glagolitic capital letter tsi
	0x2C1D, 0x2C4D, //glagolitic capital letter chrivi
	0x2C1E, 0x2C4E, //glagolitic capital letter sha
	0x2C1F, 0x2C4F, //glagolitic capital letter yeru
	0x2C20, 0x2C50, //glagolitic capital letter yeri
	0x2C21, 0x2C51, //glagolitic capital letter yati
	0x2C22, 0x2C52, //glagolitic capital letter spidery ha
	0x2C23, 0x2C53, //glagolitic capital letter yu
	0x2C24, 0x2C54, //glagolitic capital letter small yus
	0x2C25, 0x2C55, //glagolitic capital letter small yus with tail
	0x2C26, 0x2C56, //glagolitic capital letter yo
	0x2C27, 0x2C57, //glagolitic capital letter iotated small yus
	0x2C28, 0x2C58, //glagolitic capital letter big yus
	0x2C29, 0x2C59, //glagolitic capital letter iotated big yus
	0x2C2A, 0x2C5A, //glagolitic capital letter fita
	0x2C2B, 0x2C5B, //glagolitic capital letter izhitsa
	0x2C2C, 0x2C5C, //glagolitic capital letter shtapic
	0x2C2D, 0x2C5D, //glagolitic capital letter trokutasti a
	0x2C2E, 0x2C5E, //glagolitic capital letter latinate myslite
	0x2C60, 0x2C61, //latin capital letter l with double bar
	0x2C62, 0x026B, //latin capital letter l with middle tilde
	0x2C63, 0x1D7D, //latin capital letter p with stroke
	0x2C64, 0x027D, //latin capital letter r with tail
	0x2C67, 0x2C68, //latin capital letter h with descender
	0x2C69, 0x2C6A, //latin capital letter k with descender
	0x2C6B, 0x2C6C, //latin capital letter z with descender
	0x2C6D, 0x0251, //latin capital letter alpha
	0x2C6E, 0x0271, //latin capital letter m with hook
	0x2C6F, 0x0250, //latin capital letter turned a
	0x2C70, 0x0252, //latin capital letter turned alpha
	0x2C72, 0x2C73, //latin capital letter w with hook
	0x2C75, 0x2C76, //latin capital letter half h
	0x2C7E, 0x023F, //latin capital letter s with swash tail
	0x2C7F, 0x0240, //latin capital letter z with swash tail
	0x2C80, 0x2C81, //coptic capital letter alfa
	0x2C82, 0x2C83, //coptic capital letter vida
	0x2C84, 0x2C85, //coptic capital letter gamma
	0x2C86, 0x2C87, //coptic capital letter dalda
	0x2C88, 0x2C89, //coptic capital letter eie
	0x2C8A, 0x2C8B, //coptic capital letter sou
	0x2C8C, 0x2C8D, //coptic capital letter zata
	0x2C8E, 0x2C8F, //coptic capital letter hate
	0x2C90, 0x2C91, //coptic capital letter thethe
	0x2C92, 0x2C93, //coptic capital letter iauda
	0x2C94, 0x2C95, //coptic capital letter kapa
	0x2C96, 0x2C97, //coptic capital letter laula
	0x2C98, 0x2C99, //coptic capital letter mi
	0x2C9A, 0x2C9B, //coptic capital letter ni
	0x2C9C, 0x2C9D, //coptic capital letter ksi
	0x2C9E, 0x2C9F, //coptic capital letter o
	0x2CA0, 0x2CA1, //coptic capital letter pi
	0x2CA2, 0x2CA3, //coptic capital letter ro
	0x2CA4, 0x2CA5, //coptic capital letter sima
	0x2CA6, 0x2CA7, //coptic capital letter tau
	0x2CA8, 0x2CA9, //coptic capital letter ua
	0x2CAA, 0x2CAB, //coptic capital letter fi
	0x2CAC, 0x2CAD, //coptic capital letter khi
	0x2CAE, 0x2CAF, //coptic capital letter psi
	0x2CB0, 0x2CB1, //coptic capital letter oou
	0x2CB2, 0x2CB3, //coptic capital letter dialect-p alef
	0x2CB4, 0x2CB5, //coptic capital letter old coptic ain
	0x2CB6, 0x2CB7, //coptic capital letter cryptogrammic eie
	0x2CB8, 0x2CB9, //coptic capital letter dialect-p kapa
	0x2CBA, 0x2CBB, //coptic capital letter dialect-p ni
	0x2CBC, 0x2CBD, //coptic capital letter cryptogrammic ni
	0x2CBE, 0x2CBF, //coptic capital letter old coptic oou
	0x2CC0, 0x2CC1, //coptic capital letter sampi
	0x2CC2, 0x2CC3, //coptic capital letter crossed shei
	0x2CC4, 0x2CC5, //coptic capital letter old coptic shei
	0x2CC6, 0x2CC7, //coptic capital letter old coptic esh
	0x2CC8, 0x2CC9, //coptic capital letter akhmimic khei
	0x2CCA, 0x2CCB, //coptic capital letter dialect-p hori
	0x2CCC, 0x2CCD, //coptic capital letter old coptic hori
	0x2CCE, 0x2CCF, //coptic capital letter old coptic ha
	0x2CD0, 0x2CD1, //coptic capital letter l-shaped ha
	0x2CD2, 0x2CD3, //coptic capital letter old coptic hei
	0x2CD4, 0x2CD5, //coptic capital letter old coptic hat
	0x2CD6, 0x2CD7, //coptic capital letter old coptic gangia
	0x2CD8, 0x2CD9, //coptic capital letter old coptic dja
	0x2CDA, 0x2CDB, //coptic capital letter old coptic shima
	0x2CDC, 0x2CDD, //coptic capital letter old nubian shima
	0x2CDE, 0x2CDF, //coptic capital letter old nubian ngi
	0x2CE0, 0x2CE1, //coptic capital letter old nubian nyi
	0x2CE2, 0x2CE3, //coptic capital letter old nubian wau
	0x2CEB, 0x2CEC, //coptic capital letter cryptogrammic shei
	0x2CED, 0x2CEE, //coptic capital letter cryptogrammic gangia
	0x2CF2, 0x2CF3, //coptic capital letter bohairic khei
	0xA640, 0xA641, //cyrillic capital letter zemlya
	0xA642, 0xA643, //cyrillic capital letter dzelo
	0xA644, 0xA645, //cyrillic capital letter reversed dze
	0xA646, 0xA647, //cyrillic capital letter iota
	0xA648, 0xA649, //cyrillic capital letter djerv
	0xA64A, 0xA64B, //cyrillic capital letter monograph uk
	0xA64C, 0xA64D, //cyrillic capital letter broad omega
	0xA64E, 0xA64F, //cyrillic capital letter neutral yer
	0xA650, 0xA651, //cyrillic capital letter yeru with back yer
	0xA652, 0xA653, //cyrillic capital letter iotified yat
	0xA654, 0xA655, //cyrillic capital letter reversed yu
	0xA656, 0xA657, //cyrillic capital letter iotified a
	0xA658, 0xA659, //cyrillic capital letter closed little yus
	0xA65A, 0xA65B, //cyrillic capital letter blended yus
	0xA65C, 0xA65D, //cyrillic capital letter iotified closed little yus
	0xA65E, 0xA65F, //cyrillic capital letter yn
	0xA660, 0xA661, //cyrillic capital letter reversed tse
	0xA662, 0xA663, //cyrillic capital letter soft de
	0xA664, 0xA665, //cyrillic capital letter soft el
	0xA666, 0xA667, //cyrillic capital letter soft em
	0xA668, 0xA669, //cyrillic capital letter monocular o
	0xA66A, 0xA66B, //cyrillic capital letter binocular o
	0xA66C, 0xA66D, //cyrillic capital letter double monocular o
	0xA680, 0xA681, //cyrillic capital letter dwe
	0xA682, 0xA683, //cyrillic capital letter dzwe
	0xA684, 0xA685, //cyrillic capital letter zhwe
	0xA686, 0xA687, //cyrillic capital letter cche
	0xA688, 0xA689, //cyrillic capital letter dzze
	0xA68A, 0xA68B, //cyrillic capital letter te with middle hook
	0xA68C, 0xA68D, //cyrillic capital letter twe
	0xA68E, 0xA68F, //cyrillic capital letter tswe
	0xA690, 0xA691, //cyrillic capital letter tsse
	0xA692, 0xA693, //cyrillic capital letter tche
	0xA694, 0xA695, //cyrillic capital letter hwe
	0xA696, 0xA697, //cyrillic capital letter shwe
	0xA698, 0xA699, //cyrillic capital letter double o
	0xA69A, 0xA69B, //cyrillic capital letter crossed o
	0xA722, 0xA723, //latin capital letter egyptological alef
	0xA724, 0xA725, //latin capital letter egyptological ain
	0xA726, 0xA727, //latin capital letter heng
	0xA728, 0xA729, //latin capital letter tz
	0xA72A, 0xA72B, //latin capital letter tresillo
	0xA72C, 0xA72D, //latin capital letter cuatrillo
	0xA72E, 0xA72F, //latin capital letter cuatrillo with comma
	0xA732, 0xA733, //latin capital letter aa
	0xA734, 0xA735, //latin capital letter ao
	0xA736, 0xA737, //latin capital letter au
	0xA738, 0xA739, //latin capital letter av
	0xA73A, 0xA73B, //latin capital letter av with horizontal bar
	0xA73C, 0xA73D, //latin capital letter ay
	0xA73E, 0xA73F, //latin capital letter reversed c with dot
	0xA740, 0xA741, //latin capital letter k with stroke
	0xA742, 0xA743, //latin capital letter k with diagonal stroke
	0xA744, 0xA745, //latin capital letter k with stroke and diagonal stroke
	0xA746, 0xA747, //latin capital letter broken l
	0xA748, 0xA749, //latin capital letter l with high stroke
	0xA74A, 0xA74B, //latin capital letter o with long stroke overlay
	0xA74C, 0xA74D, //latin capital letter o with loop
	0xA74E, 0xA74F, //latin capital letter oo
	0xA750, 0xA751, //latin capital letter p with stroke through descender
	0xA752, 0xA753, //latin capital letter p with flourish
	0xA754, 0xA755, //latin capital letter p with squirrel tail
	0xA756, 0xA757, //latin capital letter q with stroke through descender
	0xA758, 0xA759, //latin capital letter q with diagonal stroke
	0xA75A, 0xA75B, //latin capital letter r rotunda
	0xA75C, 0xA75D, //latin capital letter rum rotunda
	0xA75E, 0xA75F, //latin capital letter v with diagonal stroke
	0xA760, 0xA761, //latin capital letter vy
	0xA762, 0xA763, //latin capital letter visigothic z
	0xA764, 0xA765, //latin capital letter thorn with stroke
	0xA766, 0xA767, //latin capital letter thorn with stroke through descender
	0xA768, 0xA769, //latin capital letter vend
	0xA76A, 0xA76B, //latin capital letter et
	0xA76C, 0xA76D, //latin capital letter is
	0xA76E, 0xA76F, //latin capital letter con
	0xA779, 0xA77A, //latin capital letter insular d
	0xA77B, 0xA77C, //latin capital letter insular f
	0xA77D, 0x1D79, //latin capital letter insular g
	0xA77E, 0xA77F, //latin capital letter turned insular g
	0xA780, 0xA781, //latin capital letter turned l
	0xA782, 0xA783, //latin capital letter insular r
	0xA784, 0xA785, //latin capital letter insular s
	0xA786, 0xA787, //latin capital letter insular t
	0xA78B, 0xA78C, //latin capital letter saltillo
	0xA78D, 0x0265, //latin capital letter turned h
	0xA790, 0xA791, //latin capital letter n with descender
	0xA792, 0xA793, //latin capital letter c with bar
	0xA796, 0xA797, //latin capital letter b with flourish
	0xA798, 0xA799, //latin capital letter f with stroke
	0xA79A, 0xA79B, //latin capital letter volapuk ae
	0xA79C, 0xA79D, //latin capital letter volapuk oe
	0xA79E, 0xA79F, //latin capital letter volapuk ue
	0xA7A0, 0xA7A1, //latin capital letter g with oblique stroke
	0xA7A2, 0xA7A3, //latin capital letter k with oblique stroke
	0xA7A4, 0xA7A5, //latin capital letter n with oblique stroke
	0xA7A6, 0xA7A7, //latin capital letter r with oblique stroke
	0xA7A8, 0xA7A9, //latin capital letter s with oblique stroke
	0xA7AA, 0x0266, //latin capital letter h with hook
	0xA7AB, 0x025C, //latin capital letter reversed open e
	0xA7AC, 0x0261, //latin capital letter script g
	0xA7AD, 0x026C, //latin capital letter l with belt
	0xA7AE, 0x026A, //latin capital letter small capital i
	0xA7B0, 0x029E, //latin capital letter turned k
	0xA7B1, 0x0287, //latin capital letter turned t
	0xA7B2, 0x029D, //latin capital letter j with crossed-tail
	0xA7B3, 0xAB53, //latin capital letter chi
	0xA7B4, 0xA7B5, //latin capital letter beta
	0xA7B6, 0xA7B7, //latin capital letter omega
	0xA7B8, 0xA7B9, //latin capital letter u with stroke
	0xA7BA, 0xA7BB, //latin capital letter glottal a
	0xA7BC, 0xA7BD, //latin capital letter glottal i
	0xA7BE, 0xA7BF, //latin capital letter glottal u
	0xA7C2, 0xA7C3, //latin capital letter anglicana w
	0xA7C4, 0xA794, //latin capital letter c with palatal hook
	0xA7C5, 0x0282, //latin capital letter s with hook
	0xA7C6, 0x1D8E, //latin capital letter z with palatal hook
	0xA7C7, 0xA7C8, //latin capital letter d with short stroke overlay
	0xA7C9, 0xA7CA, //latin capital letter s with short stroke overlay
	0xA7F5, 0xA7F6, //latin capital letter reversed half h
	0xAB70, 0x13A0, //cherokee small letter a
	0xAB71, 0x13A1, //cherokee small letter e
	0xAB72, 0x13A2, //cherokee small letter i
	0xAB73, 0x13A3, //cherokee small letter o
	0xAB74, 0x13A4, //cherokee small letter u
	0xAB75, 0x13A5, //cherokee small letter v
	0xAB76, 0x13A6, //cherokee small letter ga
	0xAB77, 0x13A7, //cherokee small letter ka
	0xAB78, 0x13A8, //cherokee small letter ge
	0xAB79, 0x13A9, //cherokee small letter gi
	0xAB7A, 0x13AA, //cherokee small letter go
	0xAB7B, 0x13AB, //cherokee small letter gu
	0xAB7C, 0x13AC, //cherokee small letter gv
	0xAB7D, 0x13AD, //cherokee small letter ha
	0xAB7E, 0x13AE, //cherokee small letter he
	0xAB7F, 0x13AF, //cherokee small letter hi
	0xAB80, 0x13B0, //cherokee small letter ho
	0xAB81, 0x13B1, //cherokee small letter hu
	0xAB82, 0x13B2, //cherokee small letter hv
	0xAB83, 0x13B3, //cherokee small letter la
	0xAB84, 0x13B4, //cherokee small letter le
	0xAB85, 0x13B5, //cherokee small letter li
	0xAB86, 0x13B6, //cherokee small letter lo
	0xAB87, 0x13B7, //cherokee small letter lu
	0xAB88, 0x13B8, //cherokee small letter lv
	0xAB89, 0x13B9, //cherokee small letter ma
	0xAB8A, 0x13BA, //cherokee small letter me
	0xAB8B, 0x13BB, //cherokee small letter mi
	0xAB8C, 0x13BC, //cherokee small letter mo
	0xAB8D, 0x13BD, //cherokee small letter mu
	0xAB8E, 0x13BE, //cherokee small letter na
	0xAB8F, 0x13BF, //cherokee small letter hna
	0xAB90, 0x13C0, //cherokee small letter nah
	0xAB91, 0x13C1, //cherokee small letter ne
	0xAB92, 0x13C2, //cherokee small letter ni
	0xAB93, 0x13C3, //cherokee small letter no
	0xAB94, 0x13C4, //cherokee small letter nu
	0xAB95, 0x13C5, //cherokee small letter nv
	0xAB96, 0x13C6, //cherokee small letter qua
	0xAB97, 0x13C7, //cherokee small letter que
	0xAB98, 0x13C8, //cherokee small letter qui
	0xAB99, 0x13C9, //cherokee small letter quo
	0xAB9A, 0x13CA, //cherokee small letter quu
	0xAB9B, 0x13CB, //cherokee small letter quv
	0xAB9C, 0x13CC, //cherokee small letter sa
	0xAB9D, 0x13CD, //cherokee small letter s
	0xAB9E, 0x13CE, //cherokee small letter se
	0xAB9F, 0x13CF, //cherokee small letter si
	0xABA0, 0x13D0, //cherokee small letter so
	0xABA1, 0x13D1, //cherokee small letter su
	0xABA2, 0x13D2, //cherokee small letter sv
	0xABA3, 0x13D3, //cherokee small letter da
	0xABA4, 0x13D4, //cherokee small letter ta
	0xABA5, 0x13D5, //cherokee small letter de
	0xABA6, 0x13D6, //cherokee small letter te
	0xABA7, 0x13D7, //cherokee small letter di
	0xABA8, 0x13D8, //cherokee small letter ti
	0xABA9, 0x13D9, //cherokee small letter do
	0xABAA, 0x13DA, //cherokee small letter du
	0xABAB, 0x13DB, //cherokee small letter dv
	0xABAC, 0x13DC, //cherokee small letter dla
	0xABAD, 0x13DD, //cherokee small letter tla
	0xABAE, 0x13DE, //cherokee small letter tle
	0xABAF, 0x13DF, //cherokee small letter tli
	0xABB0, 0x13E0, //cherokee small letter tlo
	0xABB1, 0x13E1, //cherokee small letter tlu
	0xABB2, 0x13E2, //cherokee small letter tlv
	0xABB3, 0x13E3, //cherokee small letter tsa
	0xABB4, 0x13E4, //cherokee small letter tse
	0xABB5, 0x13E5, //cherokee small letter tsi
	0xABB6, 0x13E6, //cherokee small letter tso
	0xABB7, 0x13E7, //cherokee small letter tsu
	0xABB8, 0x13E8, //cherokee small letter tsv
	0xABB9, 0x13E9, //cherokee small letter wa
	0xABBA, 0x13EA, //cherokee small letter we
	0xABBB, 0x13EB, //cherokee small letter wi
	0xABBC, 0x13EC, //cherokee small letter wo
	0xABBD, 0x13ED, //cherokee small letter wu
	0xABBE, 0x13EE, //cherokee small letter wv
	0xABBF, 0x13EF, //cherokee small letter ya
	0xFF21, 0xFF41, //fullwidth latin capital letter a
	0xFF22, 0xFF42, //fullwidth latin capital letter b
	0xFF23, 0xFF43, //fullwidth latin capital letter c
	0xFF24, 0xFF44, //fullwidth latin capital letter d
	0xFF25, 0xFF45, //fullwidth latin capital letter e
	0xFF26, 0xFF46, //fullwidth latin capital letter f
	0xFF27, 0xFF47, //fullwidth latin capital letter g
	0xFF28, 0xFF48, //fullwidth latin capital letter h
	0xFF29, 0xFF49, //fullwidth latin capital letter i
	0xFF2A, 0xFF4A, //fullwidth latin capital letter j
	0xFF2B, 0xFF4B, //fullwidth latin capital letter k
	0xFF2C, 0xFF4C, //fullwidth latin capital letter l
	0xFF2D, 0xFF4D, //fullwidth latin capital letter m
	0xFF2E, 0xFF4E, //fullwidth latin capital letter n
	0xFF2F, 0xFF4F, //fullwidth latin capital letter o
	0xFF30, 0xFF50, //fullwidth latin capital letter p
	0xFF31, 0xFF51, //fullwidth latin capital letter q
	0xFF32, 0xFF52, //fullwidth latin capital letter r
	0xFF33, 0xFF53, //fullwidth latin capital letter s
	0xFF34, 0xFF54, //fullwidth latin capital letter t
	0xFF35, 0xFF55, //fullwidth latin capital letter u
	0xFF36, 0xFF56, //fullwidth latin capital letter v
	0xFF37, 0xFF57, //fullwidth latin capital letter w
	0xFF38, 0xFF58, //fullwidth latin capital letter x
	0xFF39, 0xFF59, //fullwidth latin capital letter y
	0xFF3A, 0xFF5A, //fullwidth latin capital letter z
};
#endif
