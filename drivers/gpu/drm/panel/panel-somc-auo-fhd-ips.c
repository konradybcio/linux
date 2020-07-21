/*
 * Copyright (C) 2019 AngeloGioacchino Del Regno <kholk11@gmail.com>
 *
 * from DT configurations on Sony Xperia Tone platform
 * AUO xxxx IPS LCD Panel Driver -- TODO: RETRIEVE PANEL MODEL!!!
 * SOMC PANEL ID = 4
 *
 * Copyright (c) 2016 Sony Mobile Communications Inc.
 * Parameters from dsi-panel-somc-synaptics-auo-1080p-cmd.dtsi
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/

#include <linux/backlight.h>
#include <linux/gpio/consumer.h>
#include <linux/module.h>
#include <linux/of_platform.h>
#include <linux/platform_device.h>
#include <linux/regulator/consumer.h>

#include <drm/drmP.h>
#include <drm/drm_crtc.h>
#include <drm/drm_mipi_dsi.h>
#include <drm/drm_panel.h>

#include <video/display_timing.h>
#include <video/videomode.h>

//#define MDSS_BUG_SOLVED

struct auo_fhd_ips_panel {
	struct drm_panel base;
	struct mipi_dsi_device *dsi;

	struct backlight_device *backlight;

	struct regulator *vddio_supply;
	struct regulator *avdd_supply;
	struct regulator *pvddio_supply;
	struct regulator *tvddio_supply;

	struct gpio_desc *pan_reset_gpio;
	struct gpio_desc *ts_reset_gpio;

	bool prepared;
	bool enabled;

	const struct drm_display_mode *mode;
};

static const u8 cmd_on_unk1[2] = {0xb0, 0x04};
static const u8 cmd_on_unk2[2] = {0xd6, 0x01};

static const u8 cmd_on_unk3[32] =
	{
		0xC1, 0x84, 0x00, 0x00, 0xFF, 0x47, 0x99, 0x80,
		0x39, 0xEB, 0xFF, 0xCF, 0x9A, 0x73, 0x8D, 0xFD,
		0xBF, 0xD6, 0x31, 0x2F, 0x89, 0xF1, 0x3F, 0x00,
		0x00, 0x40, 0x22, 0x82, 0x03, 0x08, 0x00, 0x01,
	};

static const u8 cmd_on_unk4[31] =
	{
		0xC7, 0x0F, 0x1F, 0x29, 0x34, 0x41, 0x4D, 0x55,
		0x61, 0x42, 0x49, 0x54, 0x60, 0x67, 0x6D, 0x79,
		0x0B, 0x1F, 0x29, 0x34, 0x41, 0x4D, 0x55, 0x61,
		0x42, 0x49, 0x54, 0x60, 0x67, 0x6D, 0x73,
	};

static const u8 cmd_on_unk5[19] =
	{
		0xC8, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFC, 0x00,
		0x00, 0x00, 0x00, 0x00, 0xFC, 0x00, 0x00, 0x00,
		0x00, 0x00, 0xFC,
	};

static const u8 cmd_off_unk1[29] =
	{
		0xD3, 0x13, 0x3B, 0xBB, 0xB3, 0xA5, 0x33, 0x33,
		0x33, 0x00, 0x80, 0xA1, 0xAA, 0x4F, 0x4F, 0x33,
		0x33, 0x33, 0xF7, 0xF2, 0x0F, 0x7D, 0x7C, 0xFF,
		0x0F, 0x99, 0x00, 0xFF, 0xFF,
	};

static const u8 cmd_off_unk2[4] = {0xD4, 0x00, 0x00, 0x00};

static inline struct auo_fhd_ips_panel *to_auo_fhd_ips(struct drm_panel *panel)
{
	return container_of(panel, struct auo_fhd_ips_panel, base);
}

static int auo_fhd_ips_panel_enable(struct drm_panel *panel)
{
	struct auo_fhd_ips_panel *auo_panel = to_auo_fhd_ips(panel);

	if (auo_panel->enabled)
		return 0;

	auo_panel->enabled = true;

	return 0;
}

static int auo_fhd_ips_panel_init(struct auo_fhd_ips_panel *auo_panel)
{
	struct device *dev = &auo_panel->dsi->dev;
	ssize_t wr_sz = 0;
	int rc = 0;

	auo_panel->dsi->mode_flags |= MIPI_DSI_MODE_LPM;

	wr_sz = mipi_dsi_generic_write(auo_panel->dsi,
					cmd_on_unk1, sizeof(cmd_on_unk1));
	if (wr_sz < 0)
		dev_err(dev, "Cannot send ON command 1: %ld\n", wr_sz);

	wr_sz = mipi_dsi_generic_write(auo_panel->dsi,
					cmd_on_unk2, sizeof(cmd_on_unk2));
	if (wr_sz < 0)
		dev_err(dev, "Cannot send ON command 2: %ld\n", wr_sz);

	wr_sz = mipi_dsi_generic_write(auo_panel->dsi,
					cmd_on_unk3, sizeof(cmd_on_unk3));
	if (wr_sz < 0)
		dev_err(dev, "Cannot send ON command 3: %ld\n", wr_sz);

	rc = mipi_dsi_dcs_set_tear_on(auo_panel->dsi, MIPI_DSI_DCS_TEAR_MODE_VBLANK);
	if (rc < 0) {
		dev_err(dev, "Cannot send mipi_dsi_set_tear_on: %d\n", rc);
		return rc;
	}

	rc = mipi_dsi_dcs_exit_sleep_mode(auo_panel->dsi);
	if (rc < 0) {
		dev_err(dev, "Cannot send exit sleep cmd: %d\n", rc);
		return rc;
	}

	wr_sz = mipi_dsi_generic_write(auo_panel->dsi,
					cmd_on_unk4, sizeof(cmd_on_unk4));
	if (wr_sz < 0)
		dev_err(dev, "Cannot send ON command 4: %ld\n", wr_sz);

	wr_sz = mipi_dsi_generic_write(auo_panel->dsi,
					cmd_on_unk5, sizeof(cmd_on_unk5));
	if (wr_sz < 0)
		dev_err(dev, "Cannot send ON command 5: %ld\n", wr_sz);


	msleep(120);

	return rc;
}

static int auo_fhd_ips_panel_on(struct auo_fhd_ips_panel *auo_panel)
{
	struct device *dev = &auo_panel->dsi->dev;
	int rc = 0;

	rc = mipi_dsi_dcs_set_display_on(auo_panel->dsi);
	if (rc < 0) {
		dev_err(dev, "Cannot send disp on cmd: %d\n", rc);
		return rc;
	}

	msleep(120);

	return rc;
}

static int auo_fhd_ips_panel_disable(struct drm_panel *panel)
{
	struct auo_fhd_ips_panel *auo_panel = to_auo_fhd_ips(panel);

	if (!auo_panel->enabled)
		return 0;

	auo_panel->enabled = false;

	return 0;
}

static int auo_fhd_ips_panel_off(struct auo_fhd_ips_panel *auo_panel)
{
	struct device *dev = &auo_panel->dsi->dev;
	ssize_t wr_sz = 0;
	int rc;

	auo_panel->dsi->mode_flags &= ~MIPI_DSI_MODE_LPM;

	rc = mipi_dsi_dcs_set_display_off(auo_panel->dsi);
	if (rc < 0)
		dev_err(dev, "Cannot set display off: %d\n", rc);

	wr_sz = mipi_dsi_generic_write(auo_panel->dsi,
					cmd_off_unk1, sizeof(cmd_off_unk1));
	if (wr_sz < 0)
		dev_err(dev, "Cannot send OFF command 1: %ld\n", wr_sz);

	wr_sz = mipi_dsi_generic_write(auo_panel->dsi,
					cmd_off_unk2, sizeof(cmd_off_unk2));
	if (wr_sz < 0)
		dev_err(dev, "Cannot send OFF command 2: %ld\n", wr_sz);

	rc = mipi_dsi_dcs_enter_sleep_mode(auo_panel->dsi);
	if (rc < 0)
		dev_err(dev, "Cannot enter sleep mode: %d\n", rc);

	msleep(100);

	return rc;
}

static int auo_fhd_ips_panel_unprepare(struct drm_panel *panel)
{
	struct auo_fhd_ips_panel *auo_panel = to_auo_fhd_ips(panel);
	int rc = 0;

	if (!auo_panel->prepared)
		return 0;
#ifdef MDSS_BUG_SOLVED
	if (auo_panel->ts_reset_gpio) {
		gpiod_set_value(auo_panel->ts_reset_gpio, 0);
		usleep_range(10000, 11000);
	}
#endif
	auo_fhd_ips_panel_off(auo_panel);

	/* TODO: LAB/IBB */
#ifdef MDSS_BUG_SOLVED
	regulator_disable(auo_panel->avdd_supply);
	regulator_disable(auo_panel->vddio_supply);

	if (auo_panel->pan_reset_gpio) {
		gpiod_set_value(auo_panel->pan_reset_gpio, 0);
		usleep_range(10000, 11000);
	}

	if (auo_panel->pvddio_supply)
		regulator_disable(auo_panel->pvddio_supply);
#endif
	auo_panel->prepared = false;

	return rc;
}

static int auo_fhd_ips_panel_prepare(struct drm_panel *panel)
{
	struct auo_fhd_ips_panel *auo_panel = to_auo_fhd_ips(panel);
	struct device *dev = &auo_panel->dsi->dev;
	int rc;

	if (auo_panel->prepared)
		return 0;

	/* Power rail VDDIO => in-cell panel main */
	rc = regulator_enable(auo_panel->vddio_supply);
	if (rc < 0)
		return rc;

	msleep(80);

	/* Power rail AVDD => in-cell touch-controller main */
	rc = regulator_enable(auo_panel->avdd_supply);
	if (rc < 0)
		dev_err(dev, "Cannot enable AVDD: %d\n", rc);
	else
		usleep_range(1000, 1100);

	/* TODO: LAB/IBB */

	/* Enable the in-cell supply to panel */
	if (auo_panel->pvddio_supply)
		rc = regulator_enable(auo_panel->pvddio_supply);
	if (rc) {
		dev_err(dev, "Cannot enable pvddio: %d", rc);
		goto poweroff_s1;
	}
	usleep_range(1000, 1100);

	/* Enable the in-cell supply to touch-controller */
	rc = regulator_enable(auo_panel->tvddio_supply);
	if (rc) {
		dev_err(dev, "Cannot enable TVDDIO: %d", rc);
		goto poweroff_s2;
	}
	usleep_range(1000, 1100);

	if (auo_panel->ts_reset_gpio)
		gpiod_set_value(auo_panel->ts_reset_gpio, 1);

#ifdef MDSS_BUG_SOLVED
	if (auo_panel->pan_reset_gpio) {
		gpiod_set_value(auo_panel->pan_reset_gpio, 0);
		usleep_range(10000, 10000);
		gpiod_set_value(auo_panel->pan_reset_gpio, 1);
		usleep_range(10000, 11000);
	};
#endif

	rc = auo_fhd_ips_panel_init(auo_panel);
	if (rc < 0) {
		dev_err(dev, "Cannot initialize panel: %d\n", rc);
		goto poweroff_s2;
	}

	rc = auo_fhd_ips_panel_on(auo_panel);
	if (rc < 0) {
		dev_err(dev, "Cannot poweron panel: %d\n", rc);
		goto poweroff_s2;
	}

	auo_panel->prepared = true;

	return 0;

poweroff_s2:
	/* Disable it to avoid current/voltage spikes in the enable path */
	if (auo_panel->pvddio_supply)
		regulator_disable(auo_panel->pvddio_supply);
	// TODO: TVDDIO not disabled!

poweroff_s1:
	regulator_disable(auo_panel->avdd_supply);
	regulator_disable(auo_panel->vddio_supply);

	return rc;
}

static const struct drm_display_mode default_mode = {
	//.clock = 299013,
	.clock = 149506,
	.hdisplay = 1080,
	.hsync_start = 1080 + 56,
	.hsync_end = 1080 + 56 + 8,
	.htotal = 1080 + 56 + 8 + 8,
	.vdisplay = 1920,
	.vsync_start = 1920 + 227,
	.vsync_end = 1920 + 227 + 8,
	.vtotal = 1920 + 227 + 8 + 8,
	//.vrefresh = 120,
	.vrefresh = 60,
	//.flags = 0,
};

static int auo_fhd_ips_panel_get_modes(struct drm_panel *panel)
{
	struct drm_display_mode *mode;
	struct auo_fhd_ips_panel *auo_panel = to_auo_fhd_ips(panel);
	struct device *dev = &auo_panel->dsi->dev;

	mode = drm_mode_duplicate(panel->drm, &default_mode);
	if (!mode) {
		dev_err(dev, "failed to add mode %ux%ux@%u\n",
			default_mode.hdisplay, default_mode.vdisplay,
			default_mode.vrefresh);
		return -ENOMEM;
	}

	drm_mode_set_name(mode);

	drm_mode_probed_add(panel->connector, mode);

	panel->connector->display_info.width_mm = 61;
	panel->connector->display_info.height_mm = 110;

	return 1;
}

static const struct drm_panel_funcs auo_fhd_ips_panel_funcs = {
	.disable = auo_fhd_ips_panel_disable,
	.unprepare = auo_fhd_ips_panel_unprepare,
	.prepare = auo_fhd_ips_panel_prepare,
	.enable = auo_fhd_ips_panel_enable,
	.get_modes = auo_fhd_ips_panel_get_modes,
};

static const struct of_device_id auo_fhd_ips_of_match[] = {
	{ .compatible = "auo,syn-incell-fhd-ips-lcd", },
	{ }
};
MODULE_DEVICE_TABLE(of, auo_fhd_ips_of_match);

static int auo_fhd_ips_panel_add(struct auo_fhd_ips_panel *auo_panel)
{
	struct device *dev = &auo_panel->dsi->dev;
	int rc;

	auo_panel->mode = &default_mode;

	auo_panel->vddio_supply = devm_regulator_get(dev, "vddio");
	if (IS_ERR(auo_panel->vddio_supply)) {
		dev_err(dev, "cannot get vddio regulator: %ld\n",
			PTR_ERR(auo_panel->vddio_supply));
		return PTR_ERR(auo_panel->vddio_supply);
	}

	auo_panel->avdd_supply = devm_regulator_get_optional(dev, "avdd");
	if (IS_ERR(auo_panel->avdd_supply)) {
		dev_err(dev, "cannot get avdd regulator: %ld\n",
			PTR_ERR(auo_panel->avdd_supply));
		auo_panel->avdd_supply = NULL;
	}

	auo_panel->pvddio_supply = devm_regulator_get_optional(dev, "pvddio");
	if (IS_ERR(auo_panel->pvddio_supply)) {
		dev_err(dev, "cannot get pvddio regulator: %ld\n",
			PTR_ERR(auo_panel->pvddio_supply));
		auo_panel->pvddio_supply = NULL;
	}

	auo_panel->tvddio_supply = devm_regulator_get_optional(dev, "tvddio");
	if (IS_ERR(auo_panel->tvddio_supply)) {
		dev_err(dev, "cannot get tvddio regulator: %ld\n",
			PTR_ERR(auo_panel->tvddio_supply));
		auo_panel->tvddio_supply = NULL;
	}

	auo_panel->pan_reset_gpio = devm_gpiod_get(dev,
					"preset", GPIOD_ASIS);
	if (IS_ERR(auo_panel->pan_reset_gpio)) {
		dev_err(dev, "cannot get preset-gpio: %ld\n",
			PTR_ERR(auo_panel->pan_reset_gpio));
		auo_panel->pan_reset_gpio = NULL;
	}

	auo_panel->ts_reset_gpio = devm_gpiod_get(dev,
					"treset", GPIOD_ASIS);
	if (IS_ERR(auo_panel->ts_reset_gpio)) {
		dev_err(dev, "cannot get treset-gpio: %ld\n",
			PTR_ERR(auo_panel->ts_reset_gpio));
		auo_panel->ts_reset_gpio = NULL;
	}

	drm_panel_init(&auo_panel->base);
	auo_panel->base.funcs = &auo_fhd_ips_panel_funcs;
	auo_panel->base.dev = dev;

	rc = drm_panel_add(&auo_panel->base);
	if (rc < 0)
		pr_err("drm panel add failed\n");

	return rc;
}

static void auo_fhd_ips_panel_del(struct auo_fhd_ips_panel *auo_panel)
{
	if (auo_panel->base.dev)
		drm_panel_remove(&auo_panel->base);
}

static int auo_fhd_ips_panel_probe(struct mipi_dsi_device *dsi)
{
	struct auo_fhd_ips_panel *auo_panel;
	int rc;

	dsi->lanes = 4;
	dsi->format = MIPI_DSI_FMT_RGB888;
	dsi->mode_flags = MIPI_DSI_CLOCK_NON_CONTINUOUS;

	auo_panel = devm_kzalloc(&dsi->dev, sizeof(*auo_panel), GFP_KERNEL);
	if (!auo_panel)
		return -ENOMEM;

	mipi_dsi_set_drvdata(dsi, auo_panel);
	auo_panel->dsi = dsi;

	rc = auo_fhd_ips_panel_add(auo_panel);
	if (rc < 0)
		return rc;

	return mipi_dsi_attach(dsi);
}

static int auo_fhd_ips_panel_remove(struct mipi_dsi_device *dsi)
{
	struct auo_fhd_ips_panel *auo_panel = mipi_dsi_get_drvdata(dsi);
	struct device *dev = &auo_panel->dsi->dev;
	int ret;

	ret = auo_fhd_ips_panel_disable(&auo_panel->base);
	if (ret < 0)
		dev_err(dev, "failed to disable panel: %d\n", ret);

	ret = mipi_dsi_detach(dsi);
	if (ret < 0)
		dev_err(dev, "Cannot detach from DSI host: %d\n", ret);

	auo_fhd_ips_panel_del(auo_panel);

	return 0;
}

static void auo_fhd_ips_panel_shutdown(struct mipi_dsi_device *dsi)
{
	struct auo_fhd_ips_panel *auo_panel = mipi_dsi_get_drvdata(dsi);

	auo_fhd_ips_panel_disable(&auo_panel->base);
}

static struct mipi_dsi_driver auo_fhd_ips_panel_driver = {
	.driver = {
		.name = "panel-auo-fhd-ips",
		.of_match_table = auo_fhd_ips_of_match,
	},
	.probe = auo_fhd_ips_panel_probe,
	.remove = auo_fhd_ips_panel_remove,
	.shutdown = auo_fhd_ips_panel_shutdown,
};
module_mipi_dsi_driver(auo_fhd_ips_panel_driver);

MODULE_AUTHOR("AngeloGioacchino Del Regno <kholk11@gmail.com>");
MODULE_DESCRIPTION("auo FullHD IPS MIPI LCD");
MODULE_LICENSE("GPL v2");
