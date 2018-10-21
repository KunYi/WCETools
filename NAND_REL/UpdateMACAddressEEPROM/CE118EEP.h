//#include "resource.h"
#include <windows.h>
#include "windev.h"
#include "ntddndis.h"
#include "nuiouser.h"

#define MAJOR_VER 0
#define MINOR_VER 1
#define DATECODE "12/02/2009"

// SCSRs

#define ID_REV							(0x50UL)
#define INT_CFG							(0x54UL)
#define INT_STS							(0x58UL)
#define INT_EN							(0x5CUL)
#define DMA_CFG							(0x60UL)
#define BYTE_TEST						(0x64UL)	// RO default 0x87654321
#define FIFO_INT						(0x68UL)
#define RX_CFG							(0x6CUL)
#define TX_CFG							(0x70UL)
#define HW_CFG							(0x74UL)
#define RX_DP_CTL						(0x78UL)
#define RX_FIFO_INF						(0x7CUL)
#define TX_FIFO_INF						(0x80UL)
#define PMT_CTRL						(0x84UL)
#define GPIO_CFG						(0x88UL)
#define GPT_CFG							(0x8CUL)
#define GPT_CNT							(0x90UL)
#define WORD_SWAP						(0x98UL)	// R/W Not Affected by SW Reset
#define FREE_RUN						(0x9CUL)	// RO
#define RX_DROP							(0xA0UL)	// R/WC
#define MAC_CSR_CMD						(0xA4UL)
#define MAC_CSR_DATA					(0xA8UL)	// R/W
#define DEFAULT_AFC_CFG					(0x006E374FUL)
#define AFC_CFG							(0xACUL)
#define E2P_CMD							(0xB0UL)
#define E2P_DATA						(0xB4UL)
#define TEST_REG_A						(0xC0UL)
#define LAN_REGISTER_EXTENT				(0x00000100UL)



// MAC CSRs
#define MAC_CR							(0x01UL)	// R/W
#define ADDRH							(0x02UL)	// R/W mask 0x0000FFFFUL
#define ADDRL							(0x03UL)	// R/W mask 0xFFFFFFFFUL
#define HASHH							(0x04UL)	// R/W
#define HASHL							(0x05UL)	// R/W
#define MII_ACC							(0x06UL)	// R/W
#define MII_DATA						(0x07UL)	// R/W mask 0x0000FFFFUL
#define FLOW							(0x08UL)	// R/W
#define VLAN1							(0x09UL)	// R/W mask 0x0000FFFFUL
#define VLAN2							(0x0AUL)	// R/W mask 0x0000FFFFUL
#define WUFF							(0x0BUL)	// WO
#define WUCSR							(0x0CUL)	// R/W




////////////////////////////////////
// PHY Definitions
////////////////////////////////////
#define LAN9118_PHY_ID					(0x00C0001CUL)
#define PHY_BSR							((DWORD)1U)
#define PHY_ID_1						((DWORD)2U)
#define PHY_ID_2						((DWORD)3U)
#define PHY_ANEG_ADV					((DWORD)4U)
#define PHY_ANEG_LPA					((DWORD)5U)
#define PHY_ANEG_EXP					((DWORD)6U)
#define PHY_MODE_CTRL_STS				((DWORD)17)		// Mode Control/Status Register
#define SPECIAL_CTRL_STS				((DWORD)27)
#define PHY_INT_SRC						((DWORD)29)
#define PHY_INT_MASK					((DWORD)30)
#define PHY_SPECIAL						((DWORD)31)


typedef struct _SCSR_Disp
{
	char name[50];
	DWORD offset;
}SCSR_Disp;


SCSR_Disp LanRegSet[] =	{"ID_REV      ", ID_REV,
						"INT_CFG    ", INT_CFG,
						"INT_STS     ", INT_STS,
						"INT_EN       ", INT_EN,
						"DMA_CFG      ", DMA_CFG,
						"BYTE_TEST      ", BYTE_TEST,
						"FIFO_INT ", FIFO_INT,
						"RX_CFG ", RX_CFG,
						"TX_CFG      ", TX_CFG,
						"HW_CFG ", HW_CFG,
						"RX_DP_CTL     ", RX_DP_CTL,
						"RX_FIFO_INF     ", RX_FIFO_INF,
						"TX_FIFO_INF     ", TX_FIFO_INF,
						"PMT_CTRL  ", PMT_CTRL,
						"GPIO_CFG ", GPIO_CFG,
						"GPT_CFG     ", GPT_CFG,
						"GPT_CNT     ", GPT_CNT,
						"WORD_SWAP     ", WORD_SWAP,
						"FREE_RUN    ", FREE_RUN,
						"RX_DROP    ", RX_DROP,
						"MAC_CSR_CMD   ", MAC_CSR_CMD,
						"MAC_CSR_DATA  ", MAC_CSR_DATA,
						"DEFAULT_AFC_CFG ", DEFAULT_AFC_CFG,
						"AFC_CFG",            AFC_CFG,
						"E2P_CMD",             	E2P_CMD,
						"E2P_DATA",            	E2P_DATA,
						"TEST_REG_A",          TEST_REG_A,
						"LAN_REGISTER_EXTENT",  LAN_REGISTER_EXTENT};


SCSR_Disp MacRegSet[] = {"MAC_CR    ", MAC_CR,
						"ADDRH     ", ADDRH,
						"ADDRL     ", ADDRL,
						"HASHH     ", HASHH,
						"HASHL     ", HASHL,
						"MII_ACC  ", MII_ACC,
						"MII_DATA  ", MII_DATA,
						"FLOW      ", FLOW,
						"VLAN1     ", VLAN1,
						"VLAN2     ", VLAN2,
						"WUFF      ", WUFF,
						"WUCSR     ", WUCSR};


SCSR_Disp PhyRegSet[] = {"LAN9118_PHY_ID           ", LAN9118_PHY_ID,
						"PHY_BSR            ", PHY_BSR,
						"PHY_ID_1            ", PHY_ID_1,
						"PHY_ID_2            ", PHY_ID_2,
						"PHY_ANEG_ADV            ", PHY_ANEG_ADV,
						"PHY_ANEG_LPA          ", PHY_ANEG_LPA,
						"PHY_ANEG_EXP            ", PHY_ANEG_EXP,
						"PHY_MODE_CTRL_STS ", PHY_MODE_CTRL_STS,
						"SPECIAL_CTRL_STS", SPECIAL_CTRL_STS,
						"PHY_INT_SRC  ", PHY_INT_SRC,
						"PHY_INT_MASK     ", PHY_INT_MASK,
						"PHY_SPECIAL  ", PHY_SPECIAL};


BOOL SendE2pCmd(HANDLE	hAdapter, PNDISUIO_QUERY_OID queryOID, DWORD cmd);
BOOL ReadE2p(HANDLE	hAdapter, PNDISUIO_QUERY_OID queryOID, WORD addr, BYTE *val);
BOOL WriteE2p( HANDLE	hAdapter, PNDISUIO_QUERY_OID queryOID, WORD addr, BYTE *val);
BOOL ReadBlockE2p( HANDLE	hAdapter, PNDISUIO_QUERY_OID queryOID, DWORD addr, DWORD length, BYTE *buf);
BOOL WriteBlockE2p( HANDLE	hAdapter, PNDISUIO_QUERY_OID queryOID, DWORD addr, DWORD length, BYTE *buf);
BOOL EraseE2p(HANDLE	hAdapter, PNDISUIO_QUERY_OID queryOID);
BOOL LanReadRegDW(HANDLE	hAdapter,  PNDISUIO_QUERY_OID queryOID, DWORD dwOffset, DWORD *dwVal);
BOOL LanWriteRegDW( HANDLE	hAdapter, PNDISUIO_QUERY_OID queryOID, DWORD dwOffset, DWORD *dwVal);
BOOL MacReadRegDW(HANDLE	hAdapter,  PNDISUIO_QUERY_OID queryOID, DWORD dwOffset, DWORD *dwVal);
BOOL MacWriteRegDW( HANDLE	hAdapter, PNDISUIO_QUERY_OID queryOID, DWORD dwOffset, DWORD *dwVal);
BOOL PhyReadRegW( HANDLE	hAdapter, PNDISUIO_QUERY_OID queryOID, DWORD dwOffset, DWORD *wVal);
BOOL PhyWriteRegW(HANDLE	hAdapter, PNDISUIO_QUERY_OID queryOID, DWORD dwOffset, DWORD *wVal);
BOOL GetDriverVersion(HANDLE	hAdapter, PNDISUIO_QUERY_OID queryOID, void *Buf);


/* Basic type definitions */
//typedef unsigned char   BYTE, *PBYTE;
//typedef unsigned short  WORD, *PWORD;
//typedef unsigned long   DWORD, *PDWORD;
//typedef unsigned __int64 DWORD64, *PDWORD64;


#define OID_NDIS_SMSC_LAN_READ_REGS					0xFFFF0001U
#define OID_NDIS_SMSC_MAC_READ_REGS					0xFFFF0002U
#define OID_NDIS_SMSC_PHY_READ_REGS					0xFFFF0003U
#define OID_NDIS_SMSC_LAN_WRITE_REGS				0xFFFF0004U
#define OID_NDIS_SMSC_MAC_WRITE_REGS				0xFFFF0005U
#define OID_NDIS_SMSC_PHY_WRITE_REGS				0xFFFF0006U
#define OID_NDIS_SMSC_DUMP_LAN_REGS					0xFFFF0007U
#define OID_NDIS_SMSC_DUMP_MAC_REGS					0xFFFF0008U
#define OID_NDIS_SMSC_DUMP_PHY_REGS					0xFFFF0009U
#define OID_NDIS_SMSC_DUMP_TX_STATS					0xFFFF000AU
#define OID_NDIS_SMSC_DUMP_RX_STATS					0xFFFF000BU
#define OID_NDIS_SMSC_SAVE_E2P_TO_FILE				0xFFFF000CU
#define OID_NDIS_SMSC_ERASE_E2P						0xFFFF000DU
#define OID_NDIS_SMSC_WRITE_FILE_TO_E2P				0xFFFF000EU
#define OID_NDIS_SMSC_SEND_E2P_COMMAND				0xFFFF000FU
#define OID_NDIS_SMSC_READ_E2P						0xFFFF0010U
#define OID_NDIS_SMSC_WRITE_E2P						0xFFFF0011U
#define OID_NDIS_SMSC_READ_E2P_BLOCK				0xFFFF0012U
#define OID_NDIS_SMSC_WRITE_E2P_BLOCK				0xFFFF0013U
#define OID_NDIS_SMSC_GET_E2P_SIZE					0xFFFF0014U
#define OID_NDIS_SMSC_GET_DRIVER_INFO				0xFFFF0015U
#define OID_NDIS_SMSC_ENABLE_DEBUG					0xFFFF0016U
#define OID_NDIS_SMSC_ENABLE_RESUME					0xFFFF0017U


// E2pCmdType:
#define	E2P_CMD_EWDS								0x10000000UL // Erase/Write Disable
#define	E2P_CMD_EWEN								0x20000000UL // Erase/Write Enable
#define	E2P_CMD_ERAL								0x60000000UL // Erase All
/*
typedef union _E2P_STRUCT
{
	BYTE E2pBuf[512];
	struct
	{
		BYTE Signature;
		BYTE MacAddr[6];
		BYTE FsPollInterval;
		BYTE HsPollInterval;
		BYTE ConfigFlag;
		WORD LangId;
		BYTE ManufacturerStrLen;
		BYTE ManufacturerStrOffset;
		BYTE ProductStrLen;
		BYTE ProductStrOffset;
		BYTE SerialNumberStrLen;
		BYTE SerialNumberStrOffset;
		BYTE ConfigStrLen;
		BYTE ConfigStrOffset;
		BYTE InterfaceStrLen;
		BYTE InterfaceStrOffset;
		BYTE HsDeviceDescLen;
		BYTE HsDeviceDescOffset;
		BYTE HsCfgIfDescLen;
		BYTE HsCfgIfDescOffset;
		BYTE FsDeviceDescLen;
		BYTE FsDeviceDescOffset;
		BYTE FsCfgIfDescLen;
		BYTE FsCfgIfDescOffset;
		BYTE Reserved[2];

		DEV_DESC DevDesc;
		CFG_DESC CfgDesc;
		IF_DESC  IfDesc;

		WORD UnicodeStr[222];
	}format;
}E2P_STRUCT, *PE2P_STRUCT;

typedef struct _OID_E2P_STRUCT
{
    DWORD            E2pSize;           // Do not change the first three fields of the
    E2P_STRUCT 			eeprom;
} OID_E2P_STRUCT, *POID_E2P_STRUCT;

typedef struct _DRIVER_VERSION {
	DWORD	Major;
	DWORD	Minor;
	DWORD	Build;
	DWORD	Qfe;
}DRIVER_VERSION, *PDRIVER_VERSION;
*/

