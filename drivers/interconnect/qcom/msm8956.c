// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2019 Linaro Ltd
 */

#define DEBUG

#include <dt-bindings/interconnect/qcom,msm8956.h>
#include <linux/clk.h>
#include <linux/device.h>
#include <linux/interconnect-provider.h>
#include <linux/io.h>
#include <linux/module.h>
#include <linux/of_device.h>
#include <linux/of_platform.h>
#include <linux/platform_device.h>
#include <linux/slab.h>

#include "smd-rpm.h"

#define RPM_BUS_MASTER_REQ	0x73616d62
#define RPM_BUS_SLAVE_REQ	0x766c7362

enum {	
	MSM8956_MASTER_MDP_PORT0	= 22,
	MSM8956_MASTER_MDP_PORT1	= 23,
	MSM8956_MASTER_GRAPHICS_3D	= 26,

	MSM8956_SLAVE_EBI_CH0		= 512,
	MSM8956_SLAVE_SYSTEM_IMEM	= 519,
	MSM8956_SLAVE_QDSS_STM		= 588,

	MSM8956_SNOC_MM_INT_0		= 10000,
	MSM8956_SNOC_INT_0		= 10004,

	MSM8956_MNOC_BIMC_MAS		= 10027,
	MSM8956_MNOC_BIMC_SLV		= 10028,
	MSM8956_MASTER_SNOC_PNOC	= 10041,
	MSM8956_SLAVE_SNOC_PNOC		= 10042,
	MSM8956_PCNOC_INT_2		= 10049,
};

#define to_qcom_provider(_provider) \
	container_of(_provider, struct qcom_icc_provider, provider)

static const struct clk_bulk_data bus_clocks[] = {
	{ .id = "bus" },
	{ .id = "bus_a" },
};

/**
 * struct qcom_icc_provider - Qualcomm specific interconnect provider
 * @provider: generic interconnect provider
 * @bus_clks: the clk_bulk_data table of bus clocks
 * @num_clks: the total number of clk_bulk_data entries
 */
struct qcom_icc_provider {
	struct icc_provider provider;
	struct clk_bulk_data *bus_clks;
	int num_clks;
};

/*
 * ID's used in RPM messages
 */
#define	ICBID_MASTER_APPSS_PROC 0
#define	ICBID_MASTER_MSS_PROC 1
#define	ICBID_MASTER_MNOC_BIMC 2
#define	ICBID_MASTER_SNOC_BIMC 3
#define	ICBID_MASTER_SNOC_BIMC_0 ICBID_MASTER_SNOC_BIMC
#define	ICBID_MASTER_CNOC_MNOC_MMSS_CFG 4
#define	ICBID_MASTER_CNOC_MNOC_CFG 5
#define	ICBID_MASTER_GFX3D 6
#define	ICBID_MASTER_JPEG 7
#define	ICBID_MASTER_MDP 8
#define	ICBID_MASTER_MDP0 ICBID_MASTER_MDP
#define	ICBID_MASTER_MDPS ICBID_MASTER_MDP
#define	ICBID_MASTER_VIDEO 9
#define	ICBID_MASTER_VIDEO_P0 ICBID_MASTER_VIDEO
#define	ICBID_MASTER_VIDEO_P1 10
#define	ICBID_MASTER_VFE 11
#define	ICBID_MASTER_VFE0 ICBID_MASTER_VFE
#define	ICBID_MASTER_CNOC_ONOC_CFG 12
#define	ICBID_MASTER_JPEG_OCMEM 13
#define	ICBID_MASTER_MDP_OCMEM 14
#define	ICBID_MASTER_VIDEO_P0_OCMEM 15
#define	ICBID_MASTER_VIDEO_P1_OCMEM 16
#define	ICBID_MASTER_VFE_OCMEM 17
#define	ICBID_MASTER_LPASS_AHB 18
#define	ICBID_MASTER_QDSS_BAM 19
#define	ICBID_MASTER_SNOC_CFG 20
#define	ICBID_MASTER_BIMC_SNOC 21
#define	ICBID_MASTER_BIMC_SNOC_0 ICBID_MASTER_BIMC_SNOC
#define	ICBID_MASTER_CNOC_SNOC 22
#define	ICBID_MASTER_CRYPTO 23
#define	ICBID_MASTER_CRYPTO_CORE0 ICBID_MASTER_CRYPTO
#define	ICBID_MASTER_CRYPTO_CORE1 24
#define	ICBID_MASTER_LPASS_PROC 25
#define	ICBID_MASTER_MSS 26
#define	ICBID_MASTER_MSS_NAV 27
#define	ICBID_MASTER_OCMEM_DMA 28
#define	ICBID_MASTER_PNOC_SNOC 29
#define	ICBID_MASTER_WCSS 30
#define	ICBID_MASTER_QDSS_ETR 31
#define	ICBID_MASTER_USB3 32
#define	ICBID_MASTER_USB3_0 ICBID_MASTER_USB3
#define	ICBID_MASTER_SDCC_1 33
#define	ICBID_MASTER_SDCC_3 34
#define	ICBID_MASTER_SDCC_2 35
#define	ICBID_MASTER_SDCC_4 36
#define	ICBID_MASTER_TSIF 37
#define	ICBID_MASTER_BAM_DMA 38
#define	ICBID_MASTER_BLSP_2 39
#define	ICBID_MASTER_USB_HSIC 40
#define	ICBID_MASTER_BLSP_1 41
#define	ICBID_MASTER_USB_HS 42
#define	ICBID_MASTER_USB_HS1 ICBID_MASTER_USB_HS
#define	ICBID_MASTER_PNOC_CFG 43
#define	ICBID_MASTER_SNOC_PNOC 44
#define	ICBID_MASTER_RPM_INST 45
#define	ICBID_MASTER_RPM_DATA 46
#define	ICBID_MASTER_RPM_SYS 47
#define	ICBID_MASTER_DEHR 48
#define	ICBID_MASTER_QDSS_DAP 49
#define	ICBID_MASTER_SPDM 50
#define	ICBID_MASTER_TIC 51
#define	ICBID_MASTER_SNOC_CNOC 52
#define	ICBID_MASTER_GFX3D_OCMEM 53
#define	ICBID_MASTER_GFX3D_GMEM ICBID_MASTER_GFX3D_OCMEM
#define	ICBID_MASTER_OVIRT_SNOC 54
#define	ICBID_MASTER_SNOC_OVIRT 55
#define	ICBID_MASTER_SNOC_GVIRT ICBID_MASTER_SNOC_OVIRT
#define	ICBID_MASTER_ONOC_OVIRT 56
#define	ICBID_MASTER_USB_HS2 57
#define	ICBID_MASTER_QPIC 58
#define	ICBID_MASTER_IPA 59
#define	ICBID_MASTER_DSI 60
#define	ICBID_MASTER_MDP1 61
#define	ICBID_MASTER_MDPE ICBID_MASTER_MDP1
#define	ICBID_MASTER_VPU_PROC 62
#define	ICBID_MASTER_VPU 63
#define	ICBID_MASTER_VPU0 ICBID_MASTER_VPU
#define	ICBID_MASTER_CRYPTO_CORE2 64
#define	ICBID_MASTER_PCIE_0 65
#define	ICBID_MASTER_PCIE_1 66
#define	ICBID_MASTER_SATA 67
#define	ICBID_MASTER_UFS 68
#define	ICBID_MASTER_USB3_1 69
#define	ICBID_MASTER_VIDEO_OCMEM 70
#define	ICBID_MASTER_VPU1 71
#define	ICBID_MASTER_VCAP 72
#define	ICBID_MASTER_EMAC 73
#define	ICBID_MASTER_BCAST 74
#define	ICBID_MASTER_MMSS_PROC 75
#define	ICBID_MASTER_SNOC_BIMC_1 76
#define	ICBID_MASTER_SNOC_PCNOC 77
#define	ICBID_MASTER_AUDIO 78
#define	ICBID_MASTER_MM_INT_0 79
#define	ICBID_MASTER_MM_INT_1 80
#define	ICBID_MASTER_MM_INT_2 81
#define	ICBID_MASTER_MM_INT_BIMC 82
#define	ICBID_MASTER_MSS_INT 83
#define	ICBID_MASTER_PCNOC_CFG 84
#define	ICBID_MASTER_PCNOC_INT_0 85
#define	ICBID_MASTER_PCNOC_INT_1 86
#define	ICBID_MASTER_PCNOC_M_0 87
#define	ICBID_MASTER_PCNOC_M_1 88
#define	ICBID_MASTER_PCNOC_S_0 89
#define	ICBID_MASTER_PCNOC_S_1 90
#define	ICBID_MASTER_PCNOC_S_2 91
#define	ICBID_MASTER_PCNOC_S_3 92
#define	ICBID_MASTER_PCNOC_S_4 93
#define	ICBID_MASTER_PCNOC_S_6 94
#define	ICBID_MASTER_PCNOC_S_7 95
#define	ICBID_MASTER_PCNOC_S_8 96
#define	ICBID_MASTER_PCNOC_S_9 97
#define	ICBID_MASTER_QDSS_INT 98
#define	ICBID_MASTER_SNOC_INT_0	99
#define	ICBID_MASTER_SNOC_INT_1 100
#define	ICBID_MASTER_SNOC_INT_BIMC 101
#define	ICBID_MASTER_TCU_0 102
#define	ICBID_MASTER_TCU_1 103
#define	ICBID_MASTER_BIMC_INT_0 104
#define	ICBID_MASTER_BIMC_INT_1 105
#define	ICBID_MASTER_CAMERA 106
#define	ICBID_MASTER_RICA 107
#define	ICBID_MASTER_SNOC_BIMC_2 108
#define	ICBID_MASTER_BIMC_SNOC_1 109
#define	ICBID_MASTER_A0NOC_SNOC 110
#define	ICBID_MASTER_A1NOC_SNOC 111
#define	ICBID_MASTER_A2NOC_SNOC 112
#define	ICBID_MASTER_PIMEM 113
#define	ICBID_MASTER_SNOC_VMEM 114
#define	ICBID_MASTER_CPP 115
#define	ICBID_MASTER_CNOC_A1NOC 116
#define	ICBID_MASTER_PNOC_A1NOC 117
#define	ICBID_MASTER_HMSS 118
#define	ICBID_MASTER_PCIE_2 119
#define	ICBID_MASTER_ROTATOR 120
#define	ICBID_MASTER_VENUS_VMEM 121
#define	ICBID_MASTER_DCC 122
#define	ICBID_MASTER_MCDMA 123
#define	ICBID_MASTER_PCNOC_INT_2 124
#define	ICBID_MASTER_PCNOC_INT_3 125
#define	ICBID_MASTER_PCNOC_INT_4 126
#define	ICBID_MASTER_PCNOC_INT_5 127
#define	ICBID_MASTER_PCNOC_INT_6 128
#define	ICBID_MASTER_PCNOC_S_5 129
#define	ICBID_MASTER_SENSORS_AHB 130
#define	ICBID_MASTER_SENSORS_PROC 131
#define	ICBID_MASTER_QSPI 132
#define	ICBID_MASTER_VFE1 133
#define	ICBID_MASTER_SNOC_INT_2 134
#define	ICBID_MASTER_SMMNOC_BIMC 135
#define	ICBID_MASTER_CRVIRT_A1NOC 136
#define	ICBID_MASTER_XM_USB_HS1 137
#define	ICBID_MASTER_XI_USB_HS1 138
#define	ICBID_MASTER_PCNOC_BIMC_1 139
#define	ICBID_MASTER_BIMC_PCNOC 140
#define	ICBID_MASTER_XI_HSIC 141
#define	ICBID_MASTER_SGMII  142
#define	ICBID_MASTER_SPMI_FETCHER 143
#define	ICBID_MASTER_GNOC_BIMC 144
#define	ICBID_MASTER_CRVIRT_A2NOC 145
#define	ICBID_MASTER_CNOC_A2NOC 146
#define	ICBID_MASTER_WLAN 147
#define	ICBID_MASTER_MSS_CE 148
#define	ICBID_MASTER_EMMC 149
#define	ICBID_MASTER_GNOC_SNOC 150
#define	ICBID_MASTER_CDSP_PROC 151

#define	ICBID_SLAVE_EBI1 0
#define	ICBID_SLAVE_APPSS_L2 1
#define	ICBID_SLAVE_BIMC_SNOC 2
#define	ICBID_SLAVE_BIMC_SNOC_0 ICBID_SLAVE_BIMC_SNOC
#define	ICBID_SLAVE_CAMERA_CFG 3
#define	ICBID_SLAVE_DISPLAY_CFG 4
#define	ICBID_SLAVE_OCMEM_CFG 5
#define	ICBID_SLAVE_CPR_CFG 6
#define	ICBID_SLAVE_CPR_XPU_CFG 7
#define	ICBID_SLAVE_MISC_CFG 8
#define	ICBID_SLAVE_MISC_XPU_CFG 9
#define	ICBID_SLAVE_VENUS_CFG 10
#define	ICBID_SLAVE_GFX3D_CFG 11
#define	ICBID_SLAVE_MMSS_CLK_CFG 12
#define	ICBID_SLAVE_MMSS_CLK_XPU_CFG 13
#define	ICBID_SLAVE_MNOC_MPU_CFG 14
#define	ICBID_SLAVE_ONOC_MPU_CFG 15
#define	ICBID_SLAVE_MNOC_BIMC 16
#define	ICBID_SLAVE_SERVICE_MNOC 17
#define	ICBID_SLAVE_OCMEM 18
#define	ICBID_SLAVE_GMEM ICBID_SLAVE_OCMEM
#define	ICBID_SLAVE_SERVICE_ONOC 19
#define	ICBID_SLAVE_APPSS 20
#define	ICBID_SLAVE_LPASS 21
#define	ICBID_SLAVE_USB3 22
#define	ICBID_SLAVE_USB3_0 ICBID_SLAVE_USB3
#define	ICBID_SLAVE_WCSS 23
#define	ICBID_SLAVE_SNOC_BIMC 24
#define	ICBID_SLAVE_SNOC_BIMC_0 ICBID_SLAVE_SNOC_BIMC
#define	ICBID_SLAVE_SNOC_CNOC 25
#define	ICBID_SLAVE_IMEM 26
#define	ICBID_SLAVE_OCIMEM ICBID_SLAVE_IMEM
#define	ICBID_SLAVE_SNOC_OVIRT 27
#define	ICBID_SLAVE_SNOC_GVIRT ICBID_SLAVE_SNOC_OVIRT
#define	ICBID_SLAVE_SNOC_PNOC 28
#define	ICBID_SLAVE_SNOC_PCNOC ICBID_SLAVE_SNOC_PNOC
#define	ICBID_SLAVE_SERVICE_SNOC 29
#define	ICBID_SLAVE_QDSS_STM 30
#define	ICBID_SLAVE_SDCC_1 31
#define	ICBID_SLAVE_SDCC_3 32
#define	ICBID_SLAVE_SDCC_2 33
#define	ICBID_SLAVE_SDCC_4 34
#define	ICBID_SLAVE_TSIF 35
#define	ICBID_SLAVE_BAM_DMA 36
#define	ICBID_SLAVE_BLSP_2 37
#define	ICBID_SLAVE_USB_HSIC 38
#define	ICBID_SLAVE_BLSP_1 39
#define	ICBID_SLAVE_USB_HS 40
#define	ICBID_SLAVE_USB_HS1 ICBID_SLAVE_USB_HS
#define	ICBID_SLAVE_PDM 41
#define	ICBID_SLAVE_PERIPH_APU_CFG 42
#define	ICBID_SLAVE_PNOC_MPU_CFG 43
#define	ICBID_SLAVE_PRNG 44
#define	ICBID_SLAVE_PNOC_SNOC 45
#define	ICBID_SLAVE_PCNOC_SNOC ICBID_SLAVE_PNOC_SNOC
#define	ICBID_SLAVE_SERVICE_PNOC 46
#define	ICBID_SLAVE_CLK_CTL 47
#define	ICBID_SLAVE_CNOC_MSS 48
#define	ICBID_SLAVE_PCNOC_MSS ICBID_SLAVE_CNOC_MSS
#define	ICBID_SLAVE_SECURITY 49
#define	ICBID_SLAVE_TCSR 50
#define	ICBID_SLAVE_TLMM 51
#define	ICBID_SLAVE_CRYPTO_0_CFG 52
#define	ICBID_SLAVE_CRYPTO_1_CFG 53
#define	ICBID_SLAVE_IMEM_CFG 54
#define	ICBID_SLAVE_MESSAGE_RAM 55
#define	ICBID_SLAVE_BIMC_CFG 56
#define	ICBID_SLAVE_BOOT_ROM 57
#define	ICBID_SLAVE_CNOC_MNOC_MMSS_CFG 58
#define	ICBID_SLAVE_PMIC_ARB 59
#define	ICBID_SLAVE_SPDM_WRAPPER 60
#define	ICBID_SLAVE_DEHR_CFG 61
#define	ICBID_SLAVE_MPM 62
#define	ICBID_SLAVE_QDSS_CFG 63
#define	ICBID_SLAVE_RBCPR_CFG 64
#define	ICBID_SLAVE_RBCPR_CX_CFG ICBID_SLAVE_RBCPR_CFG
#define	ICBID_SLAVE_RBCPR_QDSS_APU_CFG 65
#define	ICBID_SLAVE_CNOC_MNOC_CFG 66
#define	ICBID_SLAVE_SNOC_MPU_CFG 67
#define	ICBID_SLAVE_CNOC_ONOC_CFG 68
#define	ICBID_SLAVE_PNOC_CFG 69
#define	ICBID_SLAVE_SNOC_CFG 70
#define	ICBID_SLAVE_EBI1_DLL_CFG 71
#define	ICBID_SLAVE_PHY_APU_CFG 72
#define	ICBID_SLAVE_EBI1_PHY_CFG 73
#define	ICBID_SLAVE_RPM 74
#define	ICBID_SLAVE_CNOC_SNOC 75
#define	ICBID_SLAVE_SERVICE_CNOC 76
#define	ICBID_SLAVE_OVIRT_SNOC 77
#define	ICBID_SLAVE_OVIRT_OCMEM 78
#define	ICBID_SLAVE_USB_HS2 79
#define	ICBID_SLAVE_QPIC 80
#define	ICBID_SLAVE_IPS_CFG 81
#define	ICBID_SLAVE_DSI_CFG 82
#define	ICBID_SLAVE_USB3_1 83
#define	ICBID_SLAVE_PCIE_0 84
#define	ICBID_SLAVE_PCIE_1 85
#define	ICBID_SLAVE_PSS_SMMU_CFG 86
#define	ICBID_SLAVE_CRYPTO_2_CFG 87
#define	ICBID_SLAVE_PCIE_0_CFG 88
#define	ICBID_SLAVE_PCIE_1_CFG 89
#define	ICBID_SLAVE_SATA_CFG 90
#define	ICBID_SLAVE_SPSS_GENI_IR 91
#define	ICBID_SLAVE_UFS_CFG 92
#define	ICBID_SLAVE_AVSYNC_CFG 93
#define	ICBID_SLAVE_VPU_CFG 94
#define	ICBID_SLAVE_USB_PHY_CFG 95
#define	ICBID_SLAVE_RBCPR_MX_CFG 96
#define	ICBID_SLAVE_PCIE_PARF 97
#define	ICBID_SLAVE_VCAP_CFG 98
#define	ICBID_SLAVE_EMAC_CFG 99
#define	ICBID_SLAVE_BCAST_CFG 100
#define	ICBID_SLAVE_KLM_CFG 101
#define	ICBID_SLAVE_DISPLAY_PWM 102
#define	ICBID_SLAVE_GENI 103
#define	ICBID_SLAVE_SNOC_BIMC_1 104
#define	ICBID_SLAVE_AUDIO 105
#define	ICBID_SLAVE_CATS_0 106
#define	ICBID_SLAVE_CATS_1 107
#define	ICBID_SLAVE_MM_INT_0 108
#define	ICBID_SLAVE_MM_INT_1 109
#define	ICBID_SLAVE_MM_INT_2 110
#define	ICBID_SLAVE_MM_INT_BIMC 111
#define	ICBID_SLAVE_MMU_MODEM_XPU_CFG 112
#define	ICBID_SLAVE_MSS_INT 113
#define	ICBID_SLAVE_PCNOC_INT_0 114
#define	ICBID_SLAVE_PCNOC_INT_1 115
#define	ICBID_SLAVE_PCNOC_M_0 116
#define	ICBID_SLAVE_PCNOC_M_1 117
#define	ICBID_SLAVE_PCNOC_S_0 118
#define	ICBID_SLAVE_PCNOC_S_1 119
#define	ICBID_SLAVE_PCNOC_S_2 120
#define	ICBID_SLAVE_PCNOC_S_3 121
#define	ICBID_SLAVE_PCNOC_S_4 122
#define	ICBID_SLAVE_PCNOC_S_6 123
#define	ICBID_SLAVE_PCNOC_S_7 124
#define	ICBID_SLAVE_PCNOC_S_8 125
#define	ICBID_SLAVE_PCNOC_S_9 126
#define	ICBID_SLAVE_PRNG_XPU_CFG 127
#define	ICBID_SLAVE_QDSS_INT 128
#define	ICBID_SLAVE_RPM_XPU_CFG 129
#define	ICBID_SLAVE_SNOC_INT_0 130
#define	ICBID_SLAVE_SNOC_INT_1 131
#define	ICBID_SLAVE_SNOC_INT_BIMC 132
#define	ICBID_SLAVE_TCU 133
#define	ICBID_SLAVE_BIMC_INT_0 134
#define	ICBID_SLAVE_BIMC_INT_1 135
#define	ICBID_SLAVE_RICA_CFG 136
#define	ICBID_SLAVE_SNOC_BIMC_2 137
#define	ICBID_SLAVE_BIMC_SNOC_1 138
#define	ICBID_SLAVE_PNOC_A1NOC 139
#define	ICBID_SLAVE_SNOC_VMEM 140
#define	ICBID_SLAVE_A0NOC_SNOC 141
#define	ICBID_SLAVE_A1NOC_SNOC 142
#define	ICBID_SLAVE_A2NOC_SNOC 143
#define	ICBID_SLAVE_A0NOC_CFG 144
#define	ICBID_SLAVE_A0NOC_MPU_CFG 145
#define	ICBID_SLAVE_A0NOC_SMMU_CFG 146
#define	ICBID_SLAVE_A1NOC_CFG 147
#define	ICBID_SLAVE_A1NOC_MPU_CFG 148
#define	ICBID_SLAVE_A1NOC_SMMU_CFG 149
#define	ICBID_SLAVE_A2NOC_CFG 150
#define	ICBID_SLAVE_A2NOC_MPU_CFG 151
#define	ICBID_SLAVE_A2NOC_SMMU_CFG 152
#define	ICBID_SLAVE_AHB2PHY 153
#define	ICBID_SLAVE_CAMERA_THROTTLE_CFG 154
#define	ICBID_SLAVE_DCC_CFG 155
#define	ICBID_SLAVE_DISPLAY_THROTTLE_CFG 156
#define	ICBID_SLAVE_DSA_CFG 157
#define	ICBID_SLAVE_DSA_MPU_CFG 158
#define	ICBID_SLAVE_SSC_MPU_CFG 159
#define	ICBID_SLAVE_HMSS_L3 160
#define	ICBID_SLAVE_LPASS_SMMU_CFG 161
#define	ICBID_SLAVE_MMAGIC_CFG 162
#define	ICBID_SLAVE_PCIE20_AHB2PHY 163
#define	ICBID_SLAVE_PCIE_2 164
#define	ICBID_SLAVE_PCIE_2_CFG 165
#define	ICBID_SLAVE_PIMEM 166
#define	ICBID_SLAVE_PIMEM_CFG 167
#define	ICBID_SLAVE_QDSS_RBCPR_APU_CFG 168
#define	ICBID_SLAVE_RBCPR_CX 169
#define	ICBID_SLAVE_RBCPR_MX 170
#define	ICBID_SLAVE_SMMU_CPP_CFG 171
#define	ICBID_SLAVE_SMMU_JPEG_CFG 172
#define	ICBID_SLAVE_SMMU_MDP_CFG 173
#define	ICBID_SLAVE_SMMU_ROTATOR_CFG 174
#define	ICBID_SLAVE_SMMU_VENUS_CFG 175
#define	ICBID_SLAVE_SMMU_VFE_CFG 176
#define	ICBID_SLAVE_SSC_CFG 177
#define	ICBID_SLAVE_VENUS_THROTTLE_CFG 178
#define	ICBID_SLAVE_VMEM 179
#define	ICBID_SLAVE_VMEM_CFG 180
#define	ICBID_SLAVE_QDSS_MPU_CFG 181
#define	ICBID_SLAVE_USB3_PHY_CFG 182
#define	ICBID_SLAVE_IPA_CFG 183
#define	ICBID_SLAVE_PCNOC_INT_2 184
#define	ICBID_SLAVE_PCNOC_INT_3 185
#define	ICBID_SLAVE_PCNOC_INT_4 186
#define	ICBID_SLAVE_PCNOC_INT_5 187
#define	ICBID_SLAVE_PCNOC_INT_6 188
#define	ICBID_SLAVE_PCNOC_S_5 189
#define	ICBID_SLAVE_QSPI 190
#define	ICBID_SLAVE_A1NOC_MS_MPU_CFG 191
#define	ICBID_SLAVE_A2NOC_MS_MPU_CFG 192
#define	ICBID_SLAVE_MODEM_Q6_SMMU_CFG 193
#define	ICBID_SLAVE_MSS_MPU_CFG 194
#define	ICBID_SLAVE_MSS_PROC_MS_MPU_CFG 195
#define	ICBID_SLAVE_SKL 196
#define	ICBID_SLAVE_SNOC_INT_2 197
#define	ICBID_SLAVE_SMMNOC_BIMC 198
#define	ICBID_SLAVE_CRVIRT_A1NOC 199
#define	ICBID_SLAVE_SGMII	 200
#define	ICBID_SLAVE_QHS4_APPS	 201
#define	ICBID_SLAVE_BIMC_PCNOC   202
#define	ICBID_SLAVE_PCNOC_BIMC_1 203
#define	ICBID_SLAVE_SPMI_FETCHER 204
#define	ICBID_SLAVE_MMSS_SMMU_CFG 205
#define	ICBID_SLAVE_WLAN 206
#define	ICBID_SLAVE_CRVIRT_A2NOC 207
#define	ICBID_SLAVE_CNOC_A2NOC 208
#define	ICBID_SLAVE_GLM 209
#define	ICBID_SLAVE_GNOC_BIMC 210
#define	ICBID_SLAVE_GNOC_SNOC 211
#define	ICBID_SLAVE_QM_CFG 212
#define	ICBID_SLAVE_TLMM_EAST 213
#define	ICBID_SLAVE_TLMM_NORTH 214
#define	ICBID_SLAVE_TLMM_WEST 215
#define	ICBID_SLAVE_LPASS_TCM	216
#define	ICBID_SLAVE_TLMM_SOUTH	217
#define	ICBID_SLAVE_TLMM_CENTER	218
#define	ICBID_SLAVE_MSS_NAV_CE_MPU_CFG	219
#define	ICBID_SLAVE_A2NOC_THROTTLE_CFG	220
#define	ICBID_SLAVE_CDSP	221
#define	ICBID_SLAVE_CDSP_SMMU_CFG	222
#define	ICBID_SLAVE_LPASS_MPU_CFG	223
#define	ICBID_SLAVE_CSI_PHY_CFG	224

#define MSM8956_MAX_LINKS	12

/**
 * struct qcom_icc_node - Qualcomm specific interconnect nodes
 * @name: the node name used in debugfs
 * @id: a unique node identifier
 * @links: an array of nodes where we can go next while traversing
 * @num_links: the total number of @links
 * @buswidth: width of the interconnect between a node and the bus (bytes)
 * @mas_rpm_id:	RPM id for devices that are bus masters
 * @slv_rpm_id:	RPM id for devices that are bus slaves
 * @rate: current bus clock rate in Hz
 */
struct qcom_icc_node {
	unsigned char *name;
	u16 id;
	u16 links[MSM8956_MAX_LINKS];
	u16 num_links;
	u16 buswidth;
	int mas_rpm_id;
	int slv_rpm_id;
	u64 rate;
};

struct qcom_icc_desc {
	struct qcom_icc_node **nodes;
	size_t num_nodes;
};

#define DEFINE_QNODE(_idx, _name, _id, _buswidth, _mas_rpm_id,		\
		_slv_rpm_id, ...)					\
	[_idx] = &(struct qcom_icc_node) {				\
		.name = #_name,						\
		.id = _id,						\
		.buswidth = _buswidth,					\
		.mas_rpm_id = _mas_rpm_id,				\
		.slv_rpm_id = _slv_rpm_id,				\
		.num_links = ARRAY_SIZE(((int[]){ __VA_ARGS__ })),	\
		.links = { __VA_ARGS__ },				\
	},

static struct qcom_icc_node *msm8956_bimc_nodes[] = {
	DEFINE_QNODE(MASTER_SMMNOC_BIMC, mas_smmnoc_bimc, MSM8956_MNOC_BIMC_MAS, 16, ICBID_MASTER_SMMNOC_BIMC, -1, MSM8956_SLAVE_EBI_CH0)
	DEFINE_QNODE(SLAVE_EBI_CH0, slv_ebi, MSM8956_SLAVE_EBI_CH0, 16, -1, ICBID_SLAVE_EBI1)
};

static struct qcom_icc_desc msm8956_bimc = {
	.nodes = msm8956_bimc_nodes,
	.num_nodes = ARRAY_SIZE(msm8956_bimc_nodes),
};

static struct qcom_icc_node *msm8956_pcnoc_nodes[] = {
	DEFINE_QNODE(MASTER_SNOC_PCNOC, mas_snoc_pcnoc, MSM8956_MASTER_SNOC_PNOC, 8, ICBID_MASTER_SNOC_PCNOC, -1, MSM8956_PCNOC_INT_2)
	DEFINE_QNODE(PCNOC_INT_2, pcnoc_int_2, MSM8956_PCNOC_INT_2, 8, ICBID_MASTER_PCNOC_INT_2, ICBID_SLAVE_PCNOC_INT_2) // TODO Bunch of PCNOC_S
};

static struct qcom_icc_desc msm8956_pcnoc = {
	.nodes = msm8956_pcnoc_nodes,
	.num_nodes = ARRAY_SIZE(msm8956_pcnoc_nodes),
};

static struct qcom_icc_node *msm8956_snoc_nodes[] = {
	DEFINE_QNODE(SNOC_INT_0, snoc_int_0, MSM8956_SNOC_INT_0, 8, ICBID_MASTER_SNOC_INT_0, ICBID_SLAVE_SNOC_INT_0, MSM8956_SLAVE_QDSS_STM, MSM8956_SLAVE_SYSTEM_IMEM, MSM8956_SLAVE_SNOC_PNOC)
	DEFINE_QNODE(SLAVE_SNOC_PCNOC, slv_snoc_pcnoc, MSM8956_SLAVE_SNOC_PNOC, 8, -1, ICBID_SLAVE_SNOC_PCNOC, MSM8956_MASTER_SNOC_PNOC)
	DEFINE_QNODE(SLAVE_QDSS_STM, slv_qdss_stm, MSM8956_SLAVE_QDSS_STM, 4, -1, ICBID_SLAVE_QDSS_STM)
	DEFINE_QNODE(SLAVE_SYSTEM_IMEM, slv_imem, MSM8956_SLAVE_SYSTEM_IMEM, 8, -1, ICBID_SLAVE_IMEM)

	DEFINE_QNODE(MASTER_OXILI, mas_oxili, MSM8956_MASTER_GRAPHICS_3D, 16, ICBID_MASTER_GFX3D, -1, MSM8956_MNOC_BIMC_SLV, MSM8956_SNOC_MM_INT_0)
	DEFINE_QNODE(SLAVE_SMMNOC_BIMC, slv_smmnoc_bimc, MSM8956_MNOC_BIMC_SLV, 16, -1, ICBID_SLAVE_SMMNOC_BIMC, MSM8956_MNOC_BIMC_MAS)
	
	DEFINE_QNODE(MASTER_MDP_PORT0, mas_mdp0, MSM8956_MASTER_MDP_PORT0, 16, ICBID_MASTER_MDP0, -1, MSM8956_MNOC_BIMC_SLV, MSM8956_SNOC_MM_INT_0)
	DEFINE_QNODE(MASTER_MDP_PORT1, mas_mdp1, MSM8956_MASTER_MDP_PORT1, 16, ICBID_MASTER_MDP1, -1, MSM8956_MNOC_BIMC_SLV, MSM8956_SNOC_MM_INT_0)

	DEFINE_QNODE(SNOC_MM_INT_0, mm_int_0, MSM8956_SNOC_MM_INT_0, 16, ICBID_MASTER_MM_INT_0, ICBID_SLAVE_MM_INT_0, MSM8956_SNOC_INT_0)
};

static struct qcom_icc_desc msm8956_snoc = {
	.nodes = msm8956_snoc_nodes,
	.num_nodes = ARRAY_SIZE(msm8956_snoc_nodes),
};

static struct qcom_icc_node *msm8956_snoc_mm_nodes[] = {
};

static struct qcom_icc_desc msm8956_snoc_mm = {
	.nodes = msm8956_snoc_mm_nodes,
	.num_nodes = ARRAY_SIZE(msm8956_snoc_mm_nodes),
};

static int qcom_icc_aggregate(struct icc_node *node, u32 avg_bw, u32 peak_bw,
			      u32 *agg_avg, u32 *agg_peak)
{
	*agg_avg += avg_bw;
	*agg_peak = max(*agg_peak, peak_bw);

	return 0;
}

static int qcom_icc_set(struct icc_node *src, struct icc_node *dst)
{
	struct qcom_icc_provider *qp;
	struct qcom_icc_node *qn, *qn_dest;
	struct icc_provider *provider;
	struct icc_node *n;
	u64 sum_bw;
	u64 max_peak_bw;
	u64 rate;
	u32 agg_avg = 0;
	u32 agg_peak = 0;
	int ret, i;

	return 0;

	if (dst != NULL)
		qcom_icc_set(dst, NULL);

	qn = src->data;
	provider = src->provider;
	qp = to_qcom_provider(provider);

	if (dst) {
		qn_dest = dst->data;
		dev_info(provider->dev, "%s for %s->%s\n", __func__, qn->name, qn_dest->name);
	}

	list_for_each_entry(n, &provider->nodes, node_list)
		qcom_icc_aggregate(n, n->avg_bw, n->peak_bw,
				   &agg_avg, &agg_peak);

	sum_bw = icc_units_to_bps(agg_avg);
	max_peak_bw = icc_units_to_bps(agg_peak);

	dev_info(provider->dev, "bw: sum: %llu, max: %llu\n", sum_bw, max_peak_bw);

	/* send bandwidth request message to the RPM processor */
	if (qn->mas_rpm_id != -1) {
		ret = qcom_icc_rpm_smd_send(QCOM_SMD_RPM_ACTIVE_STATE,
					    RPM_BUS_MASTER_REQ,
					    qn->mas_rpm_id,
					    sum_bw);
		if (ret) {
			pr_err("qcom_icc_rpm_smd_send mas %d error %d\n",
			       qn->mas_rpm_id, ret);
			return ret;
		}
	}

	if (qn->slv_rpm_id != -1) {
		ret = qcom_icc_rpm_smd_send(QCOM_SMD_RPM_ACTIVE_STATE,
					    RPM_BUS_SLAVE_REQ,
					    qn->slv_rpm_id,
					    sum_bw);
		if (ret) {
			pr_err("qcom_icc_rpm_smd_send slv error %d\n",
			       ret);
			return ret;
		}
	}

	rate = max(sum_bw, max_peak_bw);

	do_div(rate, qn->buswidth);

	if (qn->rate == rate)
		return 0;

	for (i = 0; i < qp->num_clks; i++) {
		ret = clk_set_rate(qp->bus_clks[i].clk, rate);
		if (ret) {
			pr_err("%s clk_set_rate error: %d\n",
			       qp->bus_clks[i].id, ret);
			return ret;
		}
	}

	qn->rate = rate;

	return 0;
}

static int qnoc_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	const struct qcom_icc_desc *desc;
	struct icc_onecell_data *data;
	struct icc_provider *provider;
	struct qcom_icc_node **qnodes;
	struct qcom_icc_provider *qp;
	struct icc_node *node;
	size_t num_nodes, i;
	int ret;

	/* wait for the RPM proxy */
	if (!qcom_icc_rpm_smd_available()) {
		dev_err(dev, "rpm smd not available yet!\n");
		return -EPROBE_DEFER;
	} else {
		dev_err(dev, "rpm smd available!\n");
	}

	desc = of_device_get_match_data(dev);
	dev_err(dev, "match data %px\n", desc);
	if (!desc)
		return -EINVAL;

	qnodes = desc->nodes;
	num_nodes = desc->num_nodes;

	qp = devm_kzalloc(dev, sizeof(*qp), GFP_KERNEL);
	if (!qp)
		return -ENOMEM;

	data = devm_kcalloc(dev, num_nodes, sizeof(*node), GFP_KERNEL);
	if (!data)
		return -ENOMEM;

	qp->bus_clks = devm_kmemdup(dev, bus_clocks, sizeof(bus_clocks),
				    GFP_KERNEL);
	if (!qp->bus_clks)
		return -ENOMEM;

	qp->num_clks = ARRAY_SIZE(bus_clocks);
	ret = devm_clk_bulk_get(dev, qp->num_clks, qp->bus_clks);
	dev_err(dev, "clk bulk: %d\n", ret);
	if (ret)
		return ret;

	dev_err(dev, "passed first part\n");

	ret = clk_bulk_prepare_enable(qp->num_clks, qp->bus_clks);
	if (ret)
		return ret;

	provider = &qp->provider;
	INIT_LIST_HEAD(&provider->nodes);
	provider->dev = dev;
	provider->set = qcom_icc_set;
	provider->aggregate = qcom_icc_aggregate;
	provider->xlate = of_icc_xlate_onecell;
	provider->data = data;

	ret = icc_provider_add(provider);
	if (ret) {
		dev_err(dev, "error adding interconnect provider: %d\n", ret);
		clk_bulk_disable_unprepare(qp->num_clks, qp->bus_clks);
		return ret;
	}

	for (i = 0; i < num_nodes; i++) {
		size_t j;

		node = icc_node_create(qnodes[i]->id);
		if (IS_ERR(node)) {
			ret = PTR_ERR(node);
			goto err;
		}

		node->name = qnodes[i]->name;
		node->data = qnodes[i];
		icc_node_add(node, provider);

		dev_dbg(dev, "registered node %s\n", node->name);

		/* populate links */
		for (j = 0; j < qnodes[i]->num_links; j++)
			icc_link_create(node, qnodes[i]->links[j]);

		data->nodes[i] = node;
	}
	data->num_nodes = num_nodes;

	platform_set_drvdata(pdev, qp);

	{
		int id;
		u64 rate;

		ret |= qcom_icc_rpm_smd_send(QCOM_SMD_RPM_ACTIVE_STATE, RPM_BUS_MASTER_REQ, 134, 721438000);
		ret |= qcom_icc_rpm_smd_send(QCOM_SMD_RPM_ACTIVE_STATE, RPM_BUS_MASTER_REQ, 29, 721438000);
		ret |= qcom_icc_rpm_smd_send(QCOM_SMD_RPM_ACTIVE_STATE, RPM_BUS_MASTER_REQ, 33, 261438000);
		ret |= qcom_icc_rpm_smd_send(QCOM_SMD_RPM_ACTIVE_STATE, RPM_BUS_MASTER_REQ, 34, 400000000);
		ret |= qcom_icc_rpm_smd_send(QCOM_SMD_RPM_ACTIVE_STATE, RPM_BUS_MASTER_REQ, 35, 1046000);
		ret |= qcom_icc_rpm_smd_send(QCOM_SMD_RPM_ACTIVE_STATE, RPM_BUS_MASTER_REQ, 41, 500000);
		ret |= qcom_icc_rpm_smd_send(QCOM_SMD_RPM_ACTIVE_STATE, RPM_BUS_MASTER_REQ, 42, 60000000);
		ret |= qcom_icc_rpm_smd_send(QCOM_SMD_RPM_ACTIVE_STATE, RPM_BUS_MASTER_REQ, 86, 661438000);
		ret |= qcom_icc_rpm_smd_send(QCOM_SMD_RPM_ACTIVE_STATE, RPM_BUS_MASTER_REQ, 88, 60000000);
		ret |= qcom_icc_rpm_smd_send(QCOM_SMD_RPM_ACTIVE_STATE, RPM_BUS_SLAVE_REQ, 0, 9581438000);
		ret |= qcom_icc_rpm_smd_send(QCOM_SMD_RPM_ACTIVE_STATE, RPM_BUS_SLAVE_REQ, 115, 661438000);
		ret |= qcom_icc_rpm_smd_send(QCOM_SMD_RPM_ACTIVE_STATE, RPM_BUS_SLAVE_REQ, 117, 60000000);
		ret |= qcom_icc_rpm_smd_send(QCOM_SMD_RPM_ACTIVE_STATE, RPM_BUS_SLAVE_REQ, 197, 721438000);
		ret |= qcom_icc_rpm_smd_send(QCOM_SMD_RPM_ACTIVE_STATE, RPM_BUS_SLAVE_REQ, 45, 721438000);

		dev_err(dev, "manually hacking rates: %d\n", ret);


		rate = 9581438000;
		dev_err(dev, "Setting clocks to %llu\n", rate);

		for(id=0;id<qp->num_clks;++id)
			clk_set_rate(qp->bus_clks[id].clk, rate);
	}

#if 0
	{
		int id, r;
		u32 val = 400000000;

		dev_notice(dev, "Starting test!\n");
		/*clk_set_rate(qp->bus_clk, val);
		clk_set_rate(qp->bus_a_clk, val);*/
		for(id=0;id<qp->num_clks;++id)
			clk_set_rate(qp->bus_clks[id].clk, val);

		for (id=0;id<1000;id++) {
			r = qcom_icc_rpm_smd_send(QCOM_SMD_RPM_ACTIVE_STATE, RPM_BUS_MASTER_REQ, id, val);
			if (!r)
				pr_info("*** BMAS %d\n", id);

			r = qcom_icc_rpm_smd_send(QCOM_SMD_RPM_ACTIVE_STATE, RPM_BUS_SLAVE_REQ, id, val);
			if (!r)
				pr_info("*** BSLV %d\n", id);

			//r = qcom_icc_rpm_smd_send(QCOM_SMD_RPM_SLEEP_STATE, RPM_BUS_MASTER_REQ, id, val);
			//r = qcom_icc_rpm_smd_send(QCOM_SMD_RPM_SLEEP_STATE, RPM_BUS_SLAVE_REQ, id, val);
		}
	}
#endif

	return 0;
err:
	list_for_each_entry(node, &provider->nodes, node_list) {
		icc_node_del(node);
		icc_node_destroy(node->id);
	}
	clk_bulk_disable_unprepare(qp->num_clks, qp->bus_clks);
	icc_provider_del(provider);

	return ret;
}

static int qnoc_remove(struct platform_device *pdev)
{
	struct qcom_icc_provider *qp = platform_get_drvdata(pdev);
	struct icc_provider *provider = &qp->provider;
	struct icc_node *n;

	list_for_each_entry(n, &provider->nodes, node_list) {
		icc_node_del(n);
		icc_node_destroy(n->id);
	}
	clk_bulk_disable_unprepare(qp->num_clks, qp->bus_clks);

	return icc_provider_del(provider);
}

static const struct of_device_id msm8956_noc_of_match[] = {
	{ .compatible = "qcom,msm8956-bimc", .data = &msm8956_bimc },
	{ .compatible = "qcom,msm8956-pcnoc", .data = &msm8956_pcnoc },
	{ .compatible = "qcom,msm8956-snoc", .data = &msm8956_snoc },
	{ },
};
MODULE_DEVICE_TABLE(of, msm8956_noc_of_match);

static struct platform_driver msm8956_noc_driver = {
	.probe = qnoc_probe,
	.remove = qnoc_remove,
	.driver = {
		.name = "qnoc-msm8956",
		.of_match_table = msm8956_noc_of_match,
	},
};
module_platform_driver(msm8956_noc_driver);
MODULE_DESCRIPTION("Qualcomm MSM8956 NoC driver");
MODULE_LICENSE("GPL v2");
