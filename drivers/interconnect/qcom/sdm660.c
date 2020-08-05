// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2020, Konrad Dybcio
 */

#include <dt-bindings/interconnect/qcom,sdm660.h>
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
	SDM660_MASTER_IPA = 1,
	SDM660_MASTER_CNOC_A2NOC,
	SDM660_MASTER_SDCC_1,
	SDM660_MASTER_SDCC_2,
	SDM660_MASTER_BLSP_1,
	SDM660_MASTER_BLSP_2,
	SDM660_MASTER_UFS,
	SDM660_MASTER_USB_HS,
	SDM660_MASTER_USB3,
	SDM660_MASTER_CRYPTO_C0,
	SDM660_MASTER_GNOC_BIMC,
	SDM660_MASTER_OXILI,
	SDM660_MASTER_MNOC_BIMC,
	SDM660_MASTER_SNOC_BIMC,
	SDM660_MASTER_PIMEM,
	SDM660_MASTER_SNOC_CNOC,
	SDM660_MASTER_QDSS_DAP,
	SDM660_MASTER_APPS_PROC,
	SDM660_MASTER_CNOC_MNOC_MMSS_CFG,
	SDM660_MASTER_CNOC_MNOC_CFG,
	SDM660_MASTER_CPP,
	SDM660_MASTER_JPEG,
	SDM660_MASTER_MDP_P0,
	SDM660_MASTER_MDP_P1,
	SDM660_MASTER_VENUS,
	SDM660_MASTER_VFE,
	SDM660_MASTER_QDSS_ETR,
	SDM660_MASTER_QDSS_BAM,
	SDM660_MASTER_SNOC_CFG,
	SDM660_MASTER_BIMC_SNOC,
	SDM660_MASTER_A2NOC_SNOC,
	SDM660_MASTER_GNOC_SNOC,

	SDM660_SLAVE_A2NOC_SNOC,
	SDM660_SLAVE_EBI,
	SDM660_SLAVE_HMSS_L3,
	SDM660_SLAVE_BIMC_SNOC,
	SDM660_SLAVE_CNOC_A2NOC,
	SDM660_SLAVE_MPM,
	SDM660_SLAVE_PMIC_ARB,
	SDM660_SLAVE_TLMM_NORTH,
	SDM660_SLAVE_TCSR,
	SDM660_SLAVE_PIMEM_CFG,
	SDM660_SLAVE_IMEM_CFG,
	SDM660_SLAVE_MESSAGE_RAM,
	SDM660_SLAVE_GLM,
	SDM660_SLAVE_BIMC_CFG,
	SDM660_SLAVE_PRNG,
	SDM660_SLAVE_SPDM,
	SDM660_SLAVE_QDSS_CFG,
	SDM660_SLAVE_CNOC_MNOC_CFG,
	SDM660_SLAVE_SNOC_CFG,
	SDM660_SLAVE_QM_CFG,
	SDM660_SLAVE_CLK_CTL,
	SDM660_SLAVE_MSS_CFG,
	SDM660_SLAVE_TLMM_SOUTH,
	SDM660_SLAVE_UFS_CFG,
	SDM660_SLAVE_A2NOC_CFG,
	SDM660_SLAVE_A2NOC_SMMU_CFG,
	SDM660_SLAVE_GPUSS_CFG,
	SDM660_SLAVE_AHB2PHY,
	SDM660_SLAVE_BLSP_1,
	SDM660_SLAVE_SDCC_1,
	SDM660_SLAVE_SDCC_2,
	SDM660_SLAVE_TLMM_CENTER,
	SDM660_SLAVE_BLSP_2,
	SDM660_SLAVE_PDM,
	SDM660_SLAVE_CNOC_MNOC_MMSS_CFG,
	SDM660_SLAVE_USB_HS,
	SDM660_SLAVE_USB3_0,
	SDM660_SLAVE_SRVC_CNOC,
	SDM660_SLAVE_GNOC_BIMC,
	SDM660_SLAVE_GNOC_SNOC,
	SDM660_SLAVE_CAMERA_CFG,
	SDM660_SLAVE_CAMERA_THROTTLE_CFG,
	SDM660_SLAVE_MISC_CFG,
	SDM660_SLAVE_VENUS_THROTTLE_CFG,
	SDM660_SLAVE_VENUS_CFG,
	SDM660_SLAVE_MMSS_CLK_XPU_CFG,
	SDM660_SLAVE_MMSS_CLK_CFG,
	SDM660_SLAVE_MNOC_MPU_CFG,
	SDM660_SLAVE_DISPLAY_CFG,
	SDM660_SLAVE_CSI_PHY_CFG,
	SDM660_SLAVE_DISPLAY_THROTTLE_CFG,
	SDM660_SLAVE_SMMU_CFG,
	SDM660_SLAVE_MNOC_BIMC,
	SDM660_SLAVE_SRVC_MNOC,
	SDM660_SLAVE_HMSS,
	SDM660_SLAVE_LPASS,
	SDM660_SLAVE_WLAN,
	SDM660_SLAVE_CDSP,
	SDM660_SLAVE_IPA,
	SDM660_SLAVE_SNOC_BIMC,
	SDM660_SLAVE_SNOC_CNOC,
	SDM660_SLAVE_IMEM,
	SDM660_SLAVE_PIMEM,
	SDM660_SLAVE_QDSS_STM,
	SDM660_SLAVE_SRVC_SNOC,

	SDM660_A2NOC,
	SDM660_BIMC,
	SDM660_CNOC,
	SDM660_GNOC,
	SDM660_MNOC,
	SDM660_SNOC,
	SDM660_MNOC_AHB,
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

#define SDM660_MAX_LINKS	34

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
	u16 links[SDM660_MAX_LINKS];
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

#define DEFINE_QNODE(_name, _id, _buswidth, _mas_rpm_id, _slv_rpm_id,	\
		     ...)						\
		static struct qcom_icc_node _name = {			\
		.name = #_name,						\
		.id = _id,						\
		.buswidth = _buswidth,					\
		.mas_rpm_id = _mas_rpm_id,				\
		.slv_rpm_id = _slv_rpm_id,				\
		.num_links = ARRAY_SIZE(((int[]){ __VA_ARGS__ })),	\
		.links = { __VA_ARGS__ },				\
	}

DEFINE_QNODE(mas_ipa, SDM660_MASTER_IPA, 8, 59, -1, SDM660_SLAVE_A2NOC_SNOC);
DEFINE_QNODE(mas_cnoc_a2noc, SDM660_MASTER_CNOC_A2NOC, 8, 146, -1, SDM660_SLAVE_A2NOC_SNOC);
DEFINE_QNODE(mas_sdcc_1, SDM660_MASTER_SDCC_1, 8, 33, -1, SDM660_SLAVE_A2NOC_SNOC);
DEFINE_QNODE(mas_sdcc_2, SDM660_MASTER_SDCC_2, 8, 34, -1, SDM660_SLAVE_A2NOC_SNOC);
DEFINE_QNODE(mas_blsp_1, SDM660_MASTER_BLSP_1, 4, 41, -1, SDM660_SLAVE_A2NOC_SNOC);
DEFINE_QNODE(mas_blsp_2, SDM660_MASTER_BLSP_2, 4, 39, -1, SDM660_SLAVE_A2NOC_SNOC);
DEFINE_QNODE(mas_ufs, SDM660_MASTER_UFS, 8, 68, -1, SDM660_SLAVE_A2NOC_SNOC);
DEFINE_QNODE(mas_usb_hs, SDM660_MASTER_USB_HS, 8, 42, -1, SDM660_SLAVE_A2NOC_SNOC);
DEFINE_QNODE(mas_usb3, SDM660_MASTER_USB3, 8, 32, -1, SDM660_SLAVE_A2NOC_SNOC);
DEFINE_QNODE(mas_crypto, SDM660_MASTER_CRYPTO_C0, 8, 23, -1, SDM660_SLAVE_A2NOC_SNOC);
DEFINE_QNODE(mas_gnoc_bimc, SDM660_MASTER_GNOC_BIMC, 4, 144, -1, SDM660_SLAVE_EBI);
DEFINE_QNODE(mas_oxili, SDM660_MASTER_OXILI, 4, 6, -1, SDM660_SLAVE_HMSS_L3, SDM660_SLAVE_EBI, SDM660_SLAVE_BIMC_SNOC);
DEFINE_QNODE(mas_mnoc_bimc, SDM660_MASTER_MNOC_BIMC, 4, 2, -1, SDM660_SLAVE_HMSS_L3, SDM660_SLAVE_EBI, SDM660_SLAVE_BIMC_SNOC);
DEFINE_QNODE(mas_snoc_bimc, SDM660_MASTER_SNOC_BIMC, 4, 3, -1, SDM660_SLAVE_HMSS_L3, SDM660_SLAVE_EBI);
DEFINE_QNODE(mas_pimem, SDM660_MASTER_PIMEM, 4, 113, -1, SDM660_SLAVE_HMSS_L3, SDM660_SLAVE_EBI);
DEFINE_QNODE(mas_snoc_cnoc, SDM660_MASTER_SNOC_CNOC, 8, 52, -1, SDM660_SLAVE_CLK_CTL, SDM660_SLAVE_QDSS_CFG, SDM660_SLAVE_QM_CFG, SDM660_SLAVE_SRVC_CNOC, SDM660_SLAVE_UFS_CFG, SDM660_SLAVE_TCSR, SDM660_SLAVE_A2NOC_SMMU_CFG, SDM660_SLAVE_SNOC_CFG, SDM660_SLAVE_TLMM_SOUTH, SDM660_SLAVE_MPM, SDM660_SLAVE_CNOC_MNOC_MMSS_CFG, SDM660_SLAVE_SDCC_2, SDM660_SLAVE_SDCC_1, SDM660_SLAVE_SPDM, SDM660_SLAVE_PMIC_ARB, SDM660_SLAVE_PRNG, SDM660_SLAVE_MSS_CFG, SDM660_SLAVE_GPUSS_CFG, SDM660_SLAVE_IMEM_CFG, SDM660_SLAVE_USB3_0, SDM660_SLAVE_A2NOC_CFG, SDM660_SLAVE_TLMM_NORTH, SDM660_SLAVE_USB_HS, SDM660_SLAVE_PDM, SDM660_SLAVE_TLMM_CENTER, SDM660_SLAVE_AHB2PHY, SDM660_SLAVE_BLSP_2, SDM660_SLAVE_BLSP_1, SDM660_SLAVE_PIMEM_CFG, SDM660_SLAVE_GLM, SDM660_SLAVE_MESSAGE_RAM, SDM660_SLAVE_BIMC_CFG, SDM660_SLAVE_CNOC_MNOC_CFG);
DEFINE_QNODE(mas_qdss_dap, SDM660_MASTER_QDSS_DAP, 8, 49, -1, SDM660_SLAVE_CLK_CTL, SDM660_SLAVE_QDSS_CFG, SDM660_SLAVE_QM_CFG, SDM660_SLAVE_SRVC_CNOC, SDM660_SLAVE_UFS_CFG, SDM660_SLAVE_TCSR, SDM660_SLAVE_A2NOC_SMMU_CFG, SDM660_SLAVE_SNOC_CFG, SDM660_SLAVE_TLMM_SOUTH, SDM660_SLAVE_MPM, SDM660_SLAVE_CNOC_MNOC_MMSS_CFG, SDM660_SLAVE_SDCC_2, SDM660_SLAVE_SDCC_1, SDM660_SLAVE_SPDM, SDM660_SLAVE_PMIC_ARB, SDM660_SLAVE_PRNG, SDM660_SLAVE_MSS_CFG, SDM660_SLAVE_GPUSS_CFG, SDM660_SLAVE_IMEM_CFG, SDM660_SLAVE_USB3_0, SDM660_SLAVE_A2NOC_CFG, SDM660_SLAVE_TLMM_NORTH, SDM660_SLAVE_USB_HS, SDM660_SLAVE_PDM, SDM660_SLAVE_TLMM_CENTER, SDM660_SLAVE_AHB2PHY, SDM660_SLAVE_BLSP_2, SDM660_SLAVE_BLSP_1, SDM660_SLAVE_PIMEM_CFG, SDM660_SLAVE_GLM, SDM660_SLAVE_MESSAGE_RAM, SDM660_SLAVE_CNOC_A2NOC, SDM660_SLAVE_BIMC_CFG, SDM660_SLAVE_CNOC_MNOC_CFG);
DEFINE_QNODE(mas_apss_proc, SDM660_MASTER_APPS_PROC, 16, 0, -1, SDM660_SLAVE_GNOC_SNOC, SDM660_SLAVE_GNOC_BIMC);
DEFINE_QNODE(mas_cnoc_mnoc_mmss_cfg, SDM660_MASTER_CNOC_MNOC_MMSS_CFG, 8, 4, -1, SDM660_SLAVE_VENUS_THROTTLE_CFG, SDM660_SLAVE_VENUS_CFG, SDM660_SLAVE_CAMERA_THROTTLE_CFG, SDM660_SLAVE_SMMU_CFG, SDM660_SLAVE_CAMERA_CFG, SDM660_SLAVE_CSI_PHY_CFG, SDM660_SLAVE_DISPLAY_THROTTLE_CFG, SDM660_SLAVE_DISPLAY_CFG, SDM660_SLAVE_MMSS_CLK_CFG, SDM660_SLAVE_MNOC_MPU_CFG, SDM660_SLAVE_MISC_CFG, SDM660_SLAVE_MMSS_CLK_XPU_CFG);
DEFINE_QNODE(mas_cnoc_mnoc_cfg, SDM660_MASTER_CNOC_MNOC_CFG, 4, 5, -1, SDM660_SLAVE_SRVC_MNOC);
DEFINE_QNODE(mas_cpp, SDM660_MASTER_CPP, 16, 115, -1, SDM660_SLAVE_MNOC_BIMC);
DEFINE_QNODE(mas_jpeg, SDM660_MASTER_JPEG, 16, 7, -1, SDM660_SLAVE_MNOC_BIMC);
DEFINE_QNODE(mas_mdp_p0, SDM660_MASTER_MDP_P0, 16, 8, -1, SDM660_SLAVE_MNOC_BIMC);
DEFINE_QNODE(mas_mdp_p1, SDM660_MASTER_MDP_P1, 16, 61, -1, SDM660_SLAVE_MNOC_BIMC);
DEFINE_QNODE(mas_venus, SDM660_MASTER_VENUS, 16, 9, -1, SDM660_SLAVE_MNOC_BIMC);
DEFINE_QNODE(mas_vfe, SDM660_MASTER_VFE, 16, 11, -1, SDM660_SLAVE_MNOC_BIMC);
DEFINE_QNODE(mas_qdss_etr, SDM660_MASTER_QDSS_ETR, 8, 31, -1, SDM660_SLAVE_PIMEM, SDM660_SLAVE_IMEM, SDM660_SLAVE_SNOC_CNOC, SDM660_SLAVE_SNOC_BIMC);
DEFINE_QNODE(mas_qdss_bam, SDM660_MASTER_QDSS_BAM, 4, 19, -1, SDM660_SLAVE_PIMEM, SDM660_SLAVE_IMEM, SDM660_SLAVE_SNOC_CNOC, SDM660_SLAVE_SNOC_BIMC);
DEFINE_QNODE(mas_snoc_cfg, SDM660_MASTER_SNOC_CFG, 4, 20, -1, SDM660_SLAVE_SRVC_SNOC);
DEFINE_QNODE(mas_bimc_snoc, SDM660_MASTER_BIMC_SNOC, 8, 21, -1, SDM660_SLAVE_PIMEM, SDM660_SLAVE_IPA, SDM660_SLAVE_QDSS_STM, SDM660_SLAVE_LPASS, SDM660_SLAVE_HMSS, SDM660_SLAVE_CDSP, SDM660_SLAVE_SNOC_CNOC, SDM660_SLAVE_WLAN, SDM660_SLAVE_IMEM);
DEFINE_QNODE(mas_a2noc_snoc, SDM660_MASTER_A2NOC_SNOC, 16, 112, -1, SDM660_SLAVE_PIMEM, SDM660_SLAVE_IPA, SDM660_SLAVE_QDSS_STM, SDM660_SLAVE_LPASS, SDM660_SLAVE_HMSS, SDM660_SLAVE_CDSP, SDM660_SLAVE_SNOC_CNOC, SDM660_SLAVE_WLAN, SDM660_SLAVE_IMEM);
DEFINE_QNODE(slv_a2noc_snoc, SDM660_SLAVE_A2NOC_SNOC, 16, -1, 143, SDM660_MASTER_A2NOC_SNOC);
DEFINE_QNODE(slv_ebi, SDM660_SLAVE_EBI, 4, -1, 0, 0);
DEFINE_QNODE(slv_hmss_l3, SDM660_SLAVE_HMSS_L3, 4, -1, 160, 0);
DEFINE_QNODE(slv_bimc_snoc, SDM660_SLAVE_BIMC_SNOC, 4, -1, 2, SDM660_MASTER_BIMC_SNOC);
DEFINE_QNODE(slv_cnoc_a2noc, SDM660_SLAVE_CNOC_A2NOC, 8, -1, 208, SDM660_MASTER_CNOC_A2NOC);
DEFINE_QNODE(slv_mpm, SDM660_SLAVE_MPM, 4, -1, 62, 0);
DEFINE_QNODE(slv_pmic_arb, SDM660_SLAVE_PMIC_ARB, 4, -1, 59, 0);
DEFINE_QNODE(slv_tlmm_north, SDM660_SLAVE_TLMM_NORTH, 8, -1, 214, 0);
DEFINE_QNODE(slv_tcsr, SDM660_SLAVE_TCSR, 4, -1, 50, 0);
DEFINE_QNODE(slv_pimem_cfg, SDM660_SLAVE_PIMEM_CFG, 4, -1, 167, 0);
DEFINE_QNODE(slv_imem_cfg, SDM660_SLAVE_IMEM_CFG, 4, -1, 54, 0);
DEFINE_QNODE(slv_message_ram, SDM660_SLAVE_MESSAGE_RAM, 4, -1, 55, 0);
DEFINE_QNODE(slv_glm, SDM660_SLAVE_GLM, 4, -1, 209, 0);
DEFINE_QNODE(slv_bimc_cfg, SDM660_SLAVE_BIMC_CFG, 4, -1, 56, 0);
DEFINE_QNODE(slv_prng, SDM660_SLAVE_PRNG, 4, -1, 44, 0);
DEFINE_QNODE(slv_spdm, SDM660_SLAVE_SPDM, 4, -1, 60, 0);
DEFINE_QNODE(slv_qdss_cfg, SDM660_SLAVE_QDSS_CFG, 4, -1, 63, 0);
DEFINE_QNODE(slv_cnoc_mnoc_cfg, SDM660_SLAVE_BLSP_1, 4, -1, 66, SDM660_MASTER_CNOC_MNOC_CFG);
DEFINE_QNODE(slv_snoc_cfg, SDM660_SLAVE_SNOC_CFG, 4, -1, 70, 0);
DEFINE_QNODE(slv_qm_cfg, SDM660_SLAVE_QM_CFG, 4, -1, 212, 0);
DEFINE_QNODE(slv_clk_ctl, SDM660_SLAVE_CLK_CTL, 4, -1, 47, 0);
DEFINE_QNODE(slv_mss_cfg, SDM660_SLAVE_MSS_CFG, 4, -1, 48, 0);
DEFINE_QNODE(slv_tlmm_south, SDM660_SLAVE_TLMM_SOUTH, 4, -1, 217, 0);
DEFINE_QNODE(slv_ufs_cfg, SDM660_SLAVE_UFS_CFG, 4, -1, 92, 0);
DEFINE_QNODE(slv_a2noc_cfg, SDM660_SLAVE_A2NOC_CFG, 4, -1, 150, 0);
DEFINE_QNODE(slv_a2noc_smmu_cfg, SDM660_SLAVE_A2NOC_SMMU_CFG, 8, -1, 152, 0);
DEFINE_QNODE(slv_gpuss_cfg, SDM660_SLAVE_GPUSS_CFG, 8, -1, 11, 0);
DEFINE_QNODE(slv_ahb2phy, SDM660_SLAVE_AHB2PHY, 4, -1, 163, 0);
DEFINE_QNODE(slv_blsp_1, SDM660_SLAVE_BLSP_1, 4, -1, 39, 0);
DEFINE_QNODE(slv_sdcc_1, SDM660_SLAVE_SDCC_1, 4, -1, 31, 0);
DEFINE_QNODE(slv_sdcc_2, SDM660_SLAVE_SDCC_2, 4, -1, 32, 0);
DEFINE_QNODE(slv_tlmm_center, SDM660_SLAVE_TLMM_CENTER, 4, -1, 218, 0);
DEFINE_QNODE(slv_blsp_2, SDM660_SLAVE_BLSP_2, 4, -1, 37, 0);
DEFINE_QNODE(slv_pdm, SDM660_SLAVE_PDM, 4, -1, 41, 0);
DEFINE_QNODE(slv_cnoc_mnoc_mmss_cfg, SDM660_SLAVE_CNOC_MNOC_MMSS_CFG, 8, -1, 58, SDM660_MASTER_CNOC_MNOC_MMSS_CFG);
DEFINE_QNODE(slv_usb_hs, SDM660_SLAVE_USB_HS, 4, -1, 40, 0);
DEFINE_QNODE(slv_usb3_0, SDM660_SLAVE_USB3_0, 4, -1, 22, 0);
DEFINE_QNODE(slv_srvc_cnoc, SDM660_SLAVE_SRVC_CNOC, 4, -1, 76, 0);
DEFINE_QNODE(slv_gnoc_bimc, SDM660_SLAVE_GNOC_BIMC, 16, -1, 210, SDM660_MASTER_GNOC_BIMC);
DEFINE_QNODE(slv_gnoc_snoc, SDM660_SLAVE_GNOC_SNOC, 8, -1, 211, SDM660_MASTER_GNOC_SNOC);
/* Downstream had this master defined within slaves.. :shrug: */
DEFINE_QNODE(mas_gnoc_snoc, SDM660_MASTER_GNOC_SNOC, 8, 150, -1, SDM660_SLAVE_PIMEM, SDM660_SLAVE_IPA, SDM660_SLAVE_QDSS_STM, SDM660_SLAVE_LPASS, SDM660_SLAVE_HMSS, SDM660_SLAVE_CDSP, SDM660_SLAVE_SNOC_CNOC, SDM660_SLAVE_WLAN, SDM660_SLAVE_IMEM);
DEFINE_QNODE(slv_camera_cfg, SDM660_SLAVE_CAMERA_CFG, 4, -1, 3, 0);
DEFINE_QNODE(slv_camera_throttle_cfg, SDM660_SLAVE_CAMERA_THROTTLE_CFG, 4, -1, 154, 0);
DEFINE_QNODE(slv_misc_cfg, SDM660_SLAVE_MISC_CFG, 4, -1, 8, 0);
DEFINE_QNODE(slv_venus_throttle_cfg, SDM660_SLAVE_VENUS_THROTTLE_CFG, 4, -1, 178, 0);
DEFINE_QNODE(slv_venus_cfg, SDM660_SLAVE_VENUS_CFG, 4, -1, 10, 0);
DEFINE_QNODE(slv_mmss_clk_xpu_cfg, SDM660_SLAVE_MMSS_CLK_XPU_CFG, 4, -1, 13, 0);
DEFINE_QNODE(slv_mmss_clk_cfg, SDM660_SLAVE_MMSS_CLK_CFG, 4, -1, 12, 0);
DEFINE_QNODE(slv_mnoc_mpu_cfg, SDM660_SLAVE_MNOC_MPU_CFG, 4, -1, 14, 0);
DEFINE_QNODE(slv_display_cfg, SDM660_SLAVE_DISPLAY_CFG, 4, -1, 4, 0);
DEFINE_QNODE(slv_csi_phy_cfg, SDM660_SLAVE_CSI_PHY_CFG, 4, -1, 224, 0);
DEFINE_QNODE(slv_display_throttle_cfg, SDM660_SLAVE_DISPLAY_THROTTLE_CFG, 4, -1, 156, 0);
DEFINE_QNODE(slv_smmu_cfg, SDM660_SLAVE_SMMU_CFG, 8, -1, 205, 0);
DEFINE_QNODE(slv_mnoc_bimc, SDM660_SLAVE_MNOC_BIMC, 16, -1, 16, SDM660_MASTER_MNOC_BIMC);
DEFINE_QNODE(slv_srvc_mnoc, SDM660_SLAVE_SRVC_MNOC, 8, -1, 17, 0);
DEFINE_QNODE(slv_hmss, SDM660_SLAVE_HMSS, 8, -1, 20, 0);
DEFINE_QNODE(slv_lpass, SDM660_SLAVE_LPASS, 4, -1, 21, 0);
DEFINE_QNODE(slv_wlan, SDM660_SLAVE_WLAN, 4, -1, 206, 0);
DEFINE_QNODE(slv_cdsp, SDM660_SLAVE_CDSP, 4, -1, 221, 0);
DEFINE_QNODE(slv_ipa, SDM660_SLAVE_IPA, 4, -1, 183, 0);
DEFINE_QNODE(slv_snoc_bimc, SDM660_SLAVE_SNOC_BIMC, 16, -1, 24, SDM660_MASTER_SNOC_BIMC);
DEFINE_QNODE(slv_snoc_cnoc, SDM660_SLAVE_SNOC_CNOC, 8, -1, 25, SDM660_MASTER_SNOC_CNOC);
DEFINE_QNODE(slv_imem, SDM660_SLAVE_IMEM, 8, -1, 26, 0);
DEFINE_QNODE(slv_pimem, SDM660_SLAVE_PIMEM, 8, -1, 166, 0);
DEFINE_QNODE(slv_qdss_stm, SDM660_SLAVE_QDSS_STM, 4, -1, 30, 0);
DEFINE_QNODE(slv_srvc_snoc, SDM660_SLAVE_SRVC_SNOC, 16, -1, 29, 0);

static struct qcom_icc_node *sdm660_a2noc_nodes[] = {
	[MASTER_IPA] = &mas_ipa,
	[MASTER_CNOC_A2NOC] = &mas_cnoc_a2noc,
	[MASTER_SDCC_1] = &mas_sdcc_1,
	[MASTER_SDCC_2] = &mas_sdcc_2,
	[MASTER_BLSP_1] = &mas_blsp_1,
	[MASTER_BLSP_2] = &mas_blsp_2,
	[MASTER_UFS] = &mas_ufs,
	[MASTER_USB_HS] = &mas_usb_hs,
	[MASTER_USB3] = &mas_usb3,
	[MASTER_CRYPTO_C0] = &mas_crypto,
	[SLAVE_A2NOC_SNOC] = &slv_a2noc_snoc,
};

static struct qcom_icc_desc sdm660_a2noc = {
	.nodes = sdm660_a2noc_nodes,
	.num_nodes = ARRAY_SIZE(sdm660_a2noc_nodes),
};

static struct qcom_icc_node *sdm660_bimc_nodes[] = {
	[MASTER_GNOC_BIMC] = &mas_gnoc_bimc,
	[MASTER_OXILI] = &mas_oxili,
	[MASTER_MNOC_BIMC] = &mas_mnoc_bimc,
	[MASTER_SNOC_BIMC] = &mas_snoc_bimc,
	[MASTER_PIMEM] = &mas_pimem,
	[SLAVE_EBI] = &slv_ebi,
	[SLAVE_HMSS_L3] = &slv_hmss_l3,
	[SLAVE_BIMC_SNOC] = &slv_bimc_snoc,
};

static struct qcom_icc_desc sdm660_bimc = {
	.nodes = sdm660_bimc_nodes,
	.num_nodes = ARRAY_SIZE(sdm660_bimc_nodes),
};

static struct qcom_icc_node *sdm660_cnoc_nodes[] = {
	[MASTER_SNOC_CNOC] = &mas_snoc_cnoc,
	[MASTER_QDSS_DAP] = &mas_qdss_dap,
	[SLAVE_CNOC_A2NOC] = &slv_cnoc_a2noc,
	[SLAVE_MPM] = &slv_mpm,
	[SLAVE_PMIC_ARB] = &slv_pmic_arb,
	[SLAVE_TLMM_NORTH] = &slv_tlmm_north,
	[SLAVE_TCSR] = &slv_tcsr,
	[SLAVE_PIMEM_CFG] = &slv_pimem_cfg,
	[SLAVE_IMEM_CFG] &slv_imem_cfg,
	[SLAVE_MESSAGE_RAM] = &slv_message_ram,
	[SLAVE_GLM] = &slv_glm,
	[SLAVE_BIMC_CFG] = &slv_bimc_cfg,
	[SLAVE_PRNG] = &slv_prng,
	[SLAVE_SPDM] = &slv_spdm,
	[SLAVE_QDSS_CFG] = &slv_qdss_cfg,
	[SLAVE_CNOC_MNOC_CFG] = &slv_cnoc_mnoc_cfg,
	[SLAVE_SNOC_CFG] = &slv_snoc_cfg,
	[SLAVE_QM_CFG] = &slv_qm_cfg,
	[SLAVE_CLK_CTL] = &slv_clk_ctl,
	[SLAVE_MSS_CFG] = &slv_mss_cfg,
	[SLAVE_TLMM_SOUTH] = &slv_tlmm_south,
	[SLAVE_UFS_CFG] = &slv_ufs_cfg,
	[SLAVE_A2NOC_CFG] = &slv_a2noc_cfg,
	[SLAVE_A2NOC_SMMU_CFG] = &slv_a2noc_smmu_cfg,
	[SLAVE_GPUSS_CFG] = &slv_gpuss_cfg,
	[SLAVE_AHB2PHY] = &slv_ahb2phy,
	[SLAVE_BLSP_1] = &slv_blsp_1,
	[SLAVE_SDCC_1] = &slv_sdcc_1,
	[SLAVE_SDCC_2] = &slv_sdcc_2,
	[SLAVE_TLMM_CENTER] = &slv_tlmm_center,
	[SLAVE_BLSP_2] = &slv_blsp_2,
	[SLAVE_PDM] = &slv_pdm,
	[SLAVE_CNOC_MNOC_MMSS_CFG] = &slv_cnoc_mnoc_mmss_cfg,
	[SLAVE_USB_HS] = &slv_usb_hs,
	[SLAVE_USB3_0] = &slv_usb3_0,
	[SLAVE_SRVC_CNOC] = &slv_srvc_cnoc,
};

static struct qcom_icc_desc sdm660_cnoc = {
	.nodes = sdm660_cnoc_nodes,
	.num_nodes = ARRAY_SIZE(sdm660_cnoc_nodes),
};

static struct qcom_icc_node *sdm660_gnoc_nodes[] = {
	[MASTER_APSS_PROC] &mas_apss_proc,
	[SLAVE_GNOC_BIMC] = &slv_gnoc_bimc,
	[SLAVE_GNOC_SNOC] = &slv_gnoc_snoc,

};

static struct qcom_icc_desc sdm660_gnoc = {
	.nodes = sdm660_gnoc_nodes,
	.num_nodes = ARRAY_SIZE(sdm660_gnoc_nodes),
};

static struct qcom_icc_node *sdm660_mnoc_nodes[] = {
	[MASTER_CPP] = &mas_cpp,
	[MASTER_JPEG] = &mas_jpeg,
	[MASTER_MDP_P0] = &mas_mdp_p0,
	[MASTER_MDP_P1] = &mas_mdp_p1,
	[MASTER_VENUS] = &mas_venus,
	[MASTER_VFE] = &mas_vfe,
	[SLAVE_MNOC_BIMC] = &slv_mnoc_bimc,
};

static struct qcom_icc_desc sdm660_mnoc = {
	.nodes = sdm660_mnoc_nodes,
	.num_nodes = ARRAY_SIZE(sdm660_mnoc_nodes),
};

static struct qcom_icc_node *sdm660_mnoc_ahb_nodes[] = {
	[MASTER_CNOC_MNOC_MMSS_CFG] = &mas_cnoc_mnoc_mmss_cfg,
	[MASTER_CNOC_MNOC_CFG] = &mas_cnoc_mnoc_cfg,
	[SLAVE_CAMERA_CFG] = &slv_camera_cfg,
	[SLAVE_CAMERA_THROTTLE_CFG] = &slv_camera_throttle_cfg,
	[SLAVE_MISC_CFG] = &slv_misc_cfg,
	[SLAVE_VENUS_THROTTLE_CFG] = &slv_venus_throttle_cfg,
	[SLAVE_VENUS_CFG] = &slv_venus_cfg,
	[SLAVE_MMSS_CLK_XPU_CFG] = &slv_mmss_clk_xpu_cfg,
	[SLAVE_MMSS_CLK_CFG] = &slv_mmss_clk_cfg,
	[SLAVE_MNOC_MPU_CFG] = &slv_mnoc_mpu_cfg,
	[SLAVE_DISPLAY_CFG] = &slv_display_cfg,
	[SLAVE_CSI_PHY_CFG] = &slv_csi_phy_cfg,
	[SLAVE_DISPLAY_THROTTLE_CFG] = &slv_display_throttle_cfg,
	[SLAVE_SMMU_CFG] = &slv_smmu_cfg,
	[SLAVE_SRVC_MNOC] = &slv_srvc_mnoc,
};

static struct qcom_icc_desc sdm660_mnoc_ahb = {
	.nodes = sdm660_mnoc_ahb_nodes,
	.num_nodes = ARRAY_SIZE(sdm660_mnoc_ahb_nodes),
};

static struct qcom_icc_node *sdm660_snoc_nodes[] = {
	[MASTER_QDSS_ETR] = &mas_qdss_etr,
	[MASTER_QDSS_BAM] = &mas_qdss_bam,
	[MASTER_SNOC_CFG] = &mas_snoc_cfg,
	[MASTER_BIMC_SNOC] = &mas_bimc_snoc,
	[MASTER_A2NOC_SNOC] = &mas_a2noc_snoc,
	[MASTER_GNOC_SNOC] = &mas_gnoc_snoc,
	[SLAVE_HMSS] = &slv_hmss,
	[SLAVE_LPASS] = &slv_lpass,
	[SLAVE_WLAN] = &slv_wlan,
	[SLAVE_CDSP] = &slv_cdsp,
	[SLAVE_IPA] = &slv_ipa,
	[SLAVE_SNOC_BIMC] = &slv_snoc_bimc,
	[SLAVE_SNOC_CNOC] = &slv_snoc_cnoc,
	[SLAVE_IMEM] = &slv_imem,
	[SLAVE_PIMEM] = &slv_pimem,
	[SLAVE_QDSS_STM] = &slv_qdss_stm,
	[SLAVE_SRVC_SNOC] = &slv_srvc_snoc,
};

static struct qcom_icc_desc sdm660_snoc = {
	.nodes = sdm660_snoc_nodes,
	.num_nodes = ARRAY_SIZE(sdm660_snoc_nodes),
};

static int qcom_icc_set(struct icc_node *src, struct icc_node *dst)
{
	struct qcom_icc_provider *qp;
	struct qcom_icc_node *qn;
	struct icc_provider *provider;
	struct icc_node *n;
	u64 sum_bw;
	u64 max_peak_bw;
	u64 rate;
	u32 agg_avg = 0;
	u32 agg_peak = 0;
	int ret, i;

	qn = src->data;
	provider = src->provider;
	qp = to_qcom_provider(provider);

	list_for_each_entry(n, &provider->nodes, node_list)
		provider->aggregate(n, 0, n->avg_bw, n->peak_bw,
				    &agg_avg, &agg_peak);

	sum_bw = icc_units_to_bps(agg_avg);
	max_peak_bw = icc_units_to_bps(agg_peak);

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
	if (!qcom_icc_rpm_smd_available())
		return -EPROBE_DEFER;

	desc = of_device_get_match_data(dev);
	if (!desc)
		return -EINVAL;

	qnodes = desc->nodes;
	num_nodes = desc->num_nodes;

	qp = devm_kzalloc(dev, sizeof(*qp), GFP_KERNEL);
	if (!qp)
		return -ENOMEM;

	data = devm_kzalloc(dev, struct_size(data, nodes, num_nodes),
			    GFP_KERNEL);
	if (!data)
		return -ENOMEM;

	qp->bus_clks = devm_kmemdup(dev, bus_clocks, sizeof(bus_clocks),
				    GFP_KERNEL);
	if (!qp->bus_clks)
		return -ENOMEM;

	qp->num_clks = ARRAY_SIZE(bus_clocks);
	ret = devm_clk_bulk_get(dev, qp->num_clks, qp->bus_clks);
	if (ret)
		return ret;

	ret = clk_bulk_prepare_enable(qp->num_clks, qp->bus_clks);
	if (ret)
		return ret;

	provider = &qp->provider;
	INIT_LIST_HEAD(&provider->nodes);
	provider->dev = dev;
	provider->set = qcom_icc_set;
	provider->aggregate = icc_std_aggregate;
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

	return 0;
err:
	icc_nodes_remove(provider);
	clk_bulk_disable_unprepare(qp->num_clks, qp->bus_clks);
	icc_provider_del(provider);

	return ret;
}

static int qnoc_remove(struct platform_device *pdev)
{
	struct qcom_icc_provider *qp = platform_get_drvdata(pdev);

	icc_nodes_remove(&qp->provider);
	clk_bulk_disable_unprepare(qp->num_clks, qp->bus_clks);
	return icc_provider_del(&qp->provider);
}

static const struct of_device_id sdm660_noc_of_match[] = {
	{ .compatible = "qcom,sdm660-a2noc", .data = &sdm660_a2noc },
	{ .compatible = "qcom,sdm660-bimc", .data = &sdm660_bimc },
	{ .compatible = "qcom,sdm660-cnoc", .data = &sdm660_cnoc },
	{ .compatible = "qcom,sdm660-gnoc", .data = &sdm660_gnoc },
	{ .compatible = "qcom,sdm660-mnoc", .data = &sdm660_mnoc },
	{ .compatible = "qcom,sdm660-mnoc_ahb", .data = &sdm660_mnoc_ahb },
	{ .compatible = "qcom,sdm660-snoc", .data = &sdm660_snoc },
	{ },
};
MODULE_DEVICE_TABLE(of, sdm660_noc_of_match);

static struct platform_driver sdm660_noc_driver = {
	.probe = qnoc_probe,
	.remove = qnoc_remove,
	.driver = {
		.name = "qnoc-sdm660",
		.of_match_table = sdm660_noc_of_match,
	},
};
module_platform_driver(sdm660_noc_driver);
MODULE_DESCRIPTION("Qualcomm sdm660 NoC driver");
MODULE_LICENSE("GPL v2");
