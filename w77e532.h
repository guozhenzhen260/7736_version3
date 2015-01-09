#ifndef _W77E532_
#define _W77E532_
#define __REG52_H__
/*  BYTE Registers  */
sfr P0    = 0x80;
sfr P1    = 0x90;
sfr P2    = 0xA0;
sfr P3    = 0xB0;
sfr SCON1 = 0xC0;
sfr PSW   = 0xD0;
sfr ACC   = 0xE0;
sfr B     = 0xF0;
sfr SP    = 0x81;
sfr DPL   = 0x82;
sfr DPH   = 0x83;
sfr DPL1  = 0x84;
sfr DPH1  = 0x85;
sfr DPS   = 0x86;
sfr PCON  = 0x87;
sfr TCON  = 0x88;
sfr TMOD  = 0x89;
sfr TL0   = 0x8A;
sfr TL1   = 0x8B;
sfr TH0   = 0x8C;
sfr TH1   = 0x8D;
sfr CKCON = 0x8E;
sfr EXIF  = 0x91;
sfr P4CONA= 0x92;
sfr P4CONB= 0x93;
sfr P40AL = 0x94;
sfr P40AH = 0x95;
sfr P41AL = 0x96;
sfr P41AH = 0x97;
sfr SCON  = 0x98;
sfr SBUF  = 0x99;
sfr P42AL = 0x9A;
sfr P42AH = 0x9B;
sfr P43AL = 0x9C;
sfr P43AH = 0x9D;
sfr CHPCON= 0x9F;
sfr P4CSIN= 0xA2;
sfr P4    = 0xA5;
sfr SADDR = 0xA9;
sfr SADDR1= 0xAA;
sfr ROMCON= 0xAB;
sfr SFRAL = 0xAC;
sfr SFRAH = 0xAD; 
sfr SFDFD = 0xAE;
sfr SFRCN = 0xAF;
sfr SADEN = 0xB9;
sfr SADEN1= 0xBA;
sfr SBUF1 = 0xC1;

sfr EIE   = 0xE8;
sfr EIP   = 0xF8;
sfr IE    = 0xA8;
sfr IP    = 0xB8;

/*  8052 Extensions  */
sfr T2CON  = 0xC8;
sfr RCAP2L = 0xCA;
sfr RCAP2H = 0xCB;
sfr TL2    = 0xCC;
sfr TH2    = 0xCD;


/*  BIT Registers  */
/*  PSW  */
sbit CY    = PSW^7;
sbit AC    = PSW^6;
sbit F0    = PSW^5;
sbit RS1   = PSW^4;
sbit RS0   = PSW^3;
sbit OV    = PSW^2;
sbit P     = PSW^0; //8052 only

/*  TCON  */
sbit TF1   = TCON^7;
sbit TR1   = TCON^6;
sbit TF0   = TCON^5;
sbit TR0   = TCON^4;
sbit IE1   = TCON^3;
sbit IT1   = TCON^2;
sbit IE0   = TCON^1;
sbit IT0   = TCON^0;

/*  IE  */
sbit EA    = IE^7;
sbit ES1   = IE^6;
sbit ET2   = IE^5; //8052 only
sbit ES    = IE^4;
sbit ET1   = IE^3;
sbit EX1   = IE^2;
sbit ET0   = IE^1;
sbit EX0   = IE^0;

/* EIE */
sbit EX2   = EIE^0;
sbit EX3   = EIE^1;
sbit EX4   = EIE^2;
sbit EX5   = EIE^3;
sbit EWDI  = EIE^4;


/*  IP  */
sbit PT2   = IP^5;
sbit PS    = IP^4;
sbit PT1   = IP^3;
sbit PX1   = IP^2;
sbit PT0   = IP^1;
sbit PX0   = IP^0;

/*  P3  */
sbit RD    = P3^7;
sbit WR    = P3^6;
sbit T1    = P3^5;
sbit T0    = P3^4;
sbit INT1  = P3^3;
sbit INT0  = P3^2;
sbit TXD   = P3^1;
sbit RXD   = P3^0;

/*  SCON  */
sbit SM0   = SCON^7;
sbit SM1   = SCON^6;
sbit SM2   = SCON^5;
sbit REN   = SCON^4;
sbit TB8   = SCON^3;
sbit RB8   = SCON^2;
sbit TI    = SCON^1;
sbit RI    = SCON^0;

/*  P1  */
sbit T2EX  = P1^1; // 8052 only
sbit T2    = P1^0; // 8052 only
             
/*  T2CON  */
sbit TF2    = T2CON^7;
sbit EXF2   = T2CON^6;
sbit RCLK   = T2CON^5;
sbit TCLK   = T2CON^4;
sbit EXEN2  = T2CON^3;
sbit TR2    = T2CON^2;
sbit C_T2   = T2CON^1;
sbit CP_RL2 = T2CON^0;

/* SCON1 */            
sbit SM0_1   = SCON1^7;
sbit SM1_1   = SCON1^6;
sbit SM2_1   = SCON1^5;
sbit REN_1   = SCON1^4;
sbit TB8_1   = SCON1^3;
sbit RB8_1   = SCON1^2;
sbit TI_1    = SCON1^1;
sbit RI_1    = SCON1^0;

#endif