/************************************************************************************
**                                                                                 **
**                               mcHF QRP Transceiver                              **
**                             K Atanassov - M0NKA 2014                            **
**                                                                                 **
**---------------------------------------------------------------------------------**
**                                                                                 **
**  File name:                                                                     **
**  Description:                                                                   **
**  Last Modified:                                                                 **
**  Licence:		For radio amateurs experimentation, non-commercial use only!   **
************************************************************************************/

//
// SI570 frequency shift table
//       0-100 degrees C range
//       shots from 20m band (56Mhz)
//
// This frequency compensation table referenced to 14.000 MHz by KA7OEI, 10/14 using
// a GPS reference in a temperature-controlled environment, extrapolated below 17C and above
// 70C.
//
// Temperature normalized to 43C, a temperature achieved very soon after power-up
// with both the Si570 and the temperature sensor thermally bonded.
//
// Note:  The exact frequency/temperature dependencies will likely vary from unit to unit
// for each Si570, but the values below appear to approximately follow typical AT-cut
// temperature-frequency curves.
//
short tcxo_table_20m[100] =
{
	-165	//	 0 C
	-162,	//	 1 C
	-160,	//	 2 C
	-157,	//	 3 C
	-155,	//	 4 C
	-152,	//	 5 C
	-150,	//	 6 C
	-148,	//	 7 C
	-144,	//	 8 C
	-142,	//	 9 C
	-136,	//	10 C
	-132,	//	11 C
	-130,	//	12 C
	-129,	//	13 C
	-126,	//	14 C
	-121,	//	15 C
	-116,	//	16 C
	-112,	//	17 C
	-108,	//	18 C
	-103,	//	19 C
	-99,	//	20 C
	-95,	//	21 C
	-90,	//	22 C
	-86,	//	23 C
	-81,	//	24 C
	-75,	//	25 C
	-70,	//	26 C
	-66,	//	27 C
	-61,	//	28 C
	-57,	//	29 C
	-53,	//	30 C
	-46,	//	31 C
	-42,	//	32 C
	-38,	//	33 C
	-32,	//	34 C
	-28,	//	35 C
	-25,	//	36 C
	-20,	//	37 C
	-17,	//	38 C
	-13,	//	39 C
	-9,	//	40 C
	-5,	//	41 C
	-2,	//	42 C
	1,	//	43 C
	4,	//	44 C
	7,	//	45 C
	10,	//	46 C
	12,	//	47 C
	15,	//	48 C
	16,	//	49 C
	18,	//	50 C
	20,	//	51 C
	21,	//	52 C
	22,	//	53 C
	23,	//	54 C
	23,	//	55 C
	24,	//	56 C
	23,	//	57 C
	23,	//	58 C
	22,	//	59 C
	21,	//	60 C
	20,	//	61 C
	18,	//	62 C
	16,	//	63 C
	14,	//	64 C
	11,	//	65 C
	7,	//	66 C
	3,	//	67 C
	-1,	//	68 C
	-5,	//	69 C
	-11,//	70 C
	-17,	//	71 C
	-24,	//	72 C
	-31,	//	73 C
	-38,	//	74 C
	-45,	//	75 C
	-52,	//	76 C
	-58,	//	77 C
	-67,	//	78 C
	-71,	//	79 C
	-77,	//	80 C
	-83,	//	81 C
	-89,	//	82 C
	-95,	//	83 C
	-102,	//	84 C
	-108,	//	85 C
	-115,	//	86 C
	-121,	//	87 C
	-127,	//	88 C
	-134,	//	89 C
	-141,	//	90 C
	-147,	//	91 C
	-153,	//	92 C
	-159,	//	93 C
	-166,	//	94 C
	-177,	//	95 C
	-184,	//	96 C
	-190,	//	97 C
	-197,	//	98 C
	-203	//	99 C
};
