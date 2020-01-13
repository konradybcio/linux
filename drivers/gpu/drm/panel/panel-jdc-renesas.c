// SPDX-License-Identifier: GPL-2.0-only
// Copyright (c) 2013, The Linux Foundation. All rights reserved.

#include <drm/drm_mipi_dsi.h>
#include <drm/drm_modes.h>
#include <drm/drm_panel.h>
#include <linux/backlight.h>
#include <linux/delay.h>
#include <linux/gpio/consumer.h>
#include <linux/module.h>
#include <linux/regulator/consumer.h>

#include <linux/of_platform.h>
#include <linux/platform_device.h>

struct renesas_jdc {
	struct drm_panel panel;
	struct mipi_dsi_device *dsi;
	struct backlight_device *backlight;
	struct gpio_desc *reset_gpio;

	struct regulator_bulk_data supplies[2];

	bool prepared;
	bool enabled;
};

static inline struct renesas_jdc *to_renesas_jdc(struct drm_panel *panel)
{
	return container_of(panel, struct renesas_jdc, panel);
}

#define dsi_generic_write_seq(dsi, seq...) do {				\
		static const u8 d[] = { seq };				\
		int ret;						\
		ret = mipi_dsi_generic_write(dsi, d, ARRAY_SIZE(d));	\
		if (ret < 0)						\
			return ret;					\
	} while (0)

static void renesas_jdc_reset(struct renesas_jdc *ctx)
{
	gpiod_set_value_cansleep(ctx->reset_gpio, 1);
	msleep(20);
	gpiod_set_value_cansleep(ctx->reset_gpio, 0);
	msleep(200);
	gpiod_set_value_cansleep(ctx->reset_gpio, 1);
	msleep(20);
}

static int renesas_jdc_on(struct renesas_jdc *ctx)
{
	struct mipi_dsi_device *dsi = ctx->dsi;
	struct device *dev = &dsi->dev;
	int ret;

	dsi->mode_flags |= MIPI_DSI_MODE_LPM;

	dsi_generic_write_seq(dsi, 0xb0, 0x00);
	dsi_generic_write_seq(dsi, 0xd6, 0x01);
	dsi_generic_write_seq(dsi, 0xc7,
			      0x07, 0x08, 0x0b, 0x12, 0x22, 0x3d, 0x30, 0x3d,
			      0x52, 0x54, 0x68, 0x78, 0x07, 0x08, 0x0b, 0x12,
			      0x22, 0x3d, 0x30, 0x3d, 0x52, 0x54, 0x68, 0x78);
	dsi_generic_write_seq(dsi, 0xc8,
			      0x07, 0x12, 0x1a, 0x25, 0x32, 0x45, 0x33, 0x3f,
			      0x4f, 0x50, 0x5d, 0x78, 0x07, 0x12, 0x1a, 0x25,
			      0x32, 0x45, 0x33, 0x3f, 0x4f, 0x50, 0x5d, 0x78);
	dsi_generic_write_seq(dsi, 0xc9,
			      0x07, 0x21, 0x2b, 0x33, 0x3d, 0x4c, 0x35, 0x40,
			      0x51, 0x53, 0x5c, 0x78, 0x07, 0x21, 0x2b, 0x33,
			      0x3d, 0x4c, 0x35, 0x40, 0x51, 0x53, 0x5c, 0x78);

	ret = mipi_dsi_dcs_exit_sleep_mode(dsi);
	if (ret < 0) {
		dev_err(dev, "Failed to exit sleep mode: %d\n", ret);
		return ret;
	}

	ret = mipi_dsi_dcs_set_display_on(dsi);
	if (ret < 0) {
		dev_err(dev, "Failed to set display on: %d\n", ret);
		return ret;
	}

	return 0;
}

static int renesas_jdc_off(struct renesas_jdc *ctx)
{
	struct mipi_dsi_device *dsi = ctx->dsi;
	struct device *dev = &dsi->dev;
	int ret;

	dsi->mode_flags &= ~MIPI_DSI_MODE_LPM;

	ret = mipi_dsi_dcs_set_display_off(dsi);
	if (ret < 0) {
		dev_err(dev, "Failed to set display off: %d\n", ret);
		return ret;
	}
	msleep(20);

	ret = mipi_dsi_dcs_enter_sleep_mode(dsi);
	if (ret < 0) {
		dev_err(dev, "Failed to enter sleep mode: %d\n", ret);
		return ret;
	}
	msleep(80);

	return 0;
}

static int renesas_jdc_prepare(struct drm_panel *panel)
{
	struct renesas_jdc *ctx = to_renesas_jdc(panel);
	struct device *dev = &ctx->dsi->dev;
	int ret;

	if (ctx->prepared)
		return 0;

	ret = regulator_bulk_enable(ARRAY_SIZE(ctx->supplies), ctx->supplies);
	if (ret < 0) {
		dev_err(dev, "Failed to enable regulators: %d\n", ret);
		return ret;
	}

	renesas_jdc_reset(ctx);

	ret = renesas_jdc_on(ctx);
	if (ret < 0) {
		dev_err(dev, "Failed to initialize panel: %d\n", ret);
		gpiod_set_value_cansleep(ctx->reset_gpio, 0);
		regulator_bulk_disable(ARRAY_SIZE(ctx->supplies), ctx->supplies);
		return ret;
	}

	ctx->prepared = true;
	return 0;
}

static int renesas_jdc_unprepare(struct drm_panel *panel)
{
	struct renesas_jdc *ctx = to_renesas_jdc(panel);
	struct device *dev = &ctx->dsi->dev;
	int ret;

	if (!ctx->prepared)
		return 0;

	ret = renesas_jdc_off(ctx);
	if (ret < 0)
		dev_err(dev, "Failed to un-initialize panel: %d\n", ret);

	gpiod_set_value_cansleep(ctx->reset_gpio, 0);

	regulator_bulk_disable(ARRAY_SIZE(ctx->supplies), ctx->supplies);

	ctx->prepared = false;
	return 0;
}

static int renesas_jdc_enable(struct drm_panel *panel)
{
	struct renesas_jdc *ctx = to_renesas_jdc(panel);
	int ret;

	if (ctx->enabled)
		return 0;

	ret = backlight_enable(ctx->backlight);
	if (ret < 0) {
		dev_err(&ctx->dsi->dev, "Failed to enable backlight: %d\n", ret);
		return ret;
	}

	ctx->enabled = true;
	return 0;
}

static int renesas_jdc_disable(struct drm_panel *panel)
{
	struct renesas_jdc *ctx = to_renesas_jdc(panel);
	int ret;

	if (!ctx->enabled)
		return 0;

	ret = backlight_disable(ctx->backlight);
	if (ret < 0) {
		dev_err(&ctx->dsi->dev, "Failed to disable backlight: %d\n", ret);
		return ret;
	}

	ctx->enabled = false;
	return 0;
}

static const struct drm_display_mode renesas_jdc_mode = {
	.clock = (1080 + 128 + 8 + 72) * (1920 + 8 + 4 + 4) * 60 / 1000,
	.hdisplay = 1080,
	.hsync_start = 1080 + 128,
	.hsync_end = 1080 + 128 + 8,
	.htotal = 1080 + 128 + 8 + 72,
	.vdisplay = 1920,
	.vsync_start = 1920 + 8,
	.vsync_end = 1920 + 8 + 4,
	.vtotal = 1920 + 8 + 4 + 4,
	.vrefresh = 60,
	.width_mm = 62,
	.height_mm = 110,
};

static int renesas_jdc_get_modes(struct drm_panel *panel)
{
	struct drm_display_mode *mode;

	mode = drm_mode_duplicate(panel->drm, &renesas_jdc_mode);
	if (!mode)
		return -ENOMEM;

	drm_mode_set_name(mode);

	mode->type = DRM_MODE_TYPE_DRIVER | DRM_MODE_TYPE_PREFERRED;
	panel->connector->display_info.width_mm = mode->width_mm;
	panel->connector->display_info.height_mm = mode->height_mm;
	drm_mode_probed_add(panel->connector, mode);

	return 1;
}

static const struct drm_panel_funcs renesas_jdc_panel_funcs = {
	.disable = renesas_jdc_disable,
	.unprepare = renesas_jdc_unprepare,
	.prepare = renesas_jdc_prepare,
	.enable = renesas_jdc_enable,
	.get_modes = renesas_jdc_get_modes,
};

static int renesas_jdc_probe(struct mipi_dsi_device *dsi)
{
	struct device *dev = &dsi->dev;
	struct renesas_jdc *ctx;
	int ret;

	ctx = devm_kzalloc(dev, sizeof(*ctx), GFP_KERNEL);
	if (!ctx)
		return -ENOMEM;

	ctx->supplies[0].supply = "vdd";
	ctx->supplies[1].supply = "vddio";
	ret = devm_regulator_bulk_get(dev, ARRAY_SIZE(ctx->supplies),
				      ctx->supplies);
	if (ret < 0) {
		dev_err(dev, "Failed to get regulators: %d\n", ret);
		return ret;
	}

	ctx->reset_gpio = devm_gpiod_get(dev, "reset", GPIOD_OUT_LOW);
	if (IS_ERR(ctx->reset_gpio)) {
		ret = PTR_ERR(ctx->reset_gpio);
		dev_err(dev, "Failed to get reset-gpios: %d\n", ret);
		return ret;
	}

	ctx->backlight = devm_of_find_backlight(dev);
	if (IS_ERR(ctx->backlight)) {
		ret = PTR_ERR(ctx->backlight);
		dev_err(dev, "Failed to get backlight: %d\n", ret);
		return ret;
	}

	ctx->dsi = dsi;
	mipi_dsi_set_drvdata(dsi, ctx);

	dsi->lanes = 4;
	dsi->format = MIPI_DSI_FMT_RGB888;
	dsi->mode_flags = MIPI_DSI_MODE_VIDEO | MIPI_DSI_MODE_VIDEO_HSE |
			  MIPI_DSI_MODE_EOT_PACKET |
			  MIPI_DSI_CLOCK_NON_CONTINUOUS;

	drm_panel_init(&ctx->panel, dev, &renesas_jdc_panel_funcs,
		       DRM_MODE_CONNECTOR_DSI);

	ret = drm_panel_add(&ctx->panel);
	if (ret < 0) {
		dev_err(dev, "Failed to add panel: %d\n", ret);
		return ret;
	}

	ret = mipi_dsi_attach(dsi);
	if (ret < 0) {
		dev_err(dev, "Failed to attach to DSI host: %d\n", ret);
		return ret;
	}

	return 0;
}

static int renesas_jdc_remove(struct mipi_dsi_device *dsi)
{
	struct renesas_jdc *ctx = mipi_dsi_get_drvdata(dsi);
	int ret;

	ret = mipi_dsi_detach(dsi);
	if (ret < 0)
		dev_err(&dsi->dev, "Failed to detach from DSI host: %d\n", ret);

	drm_panel_remove(&ctx->panel);

	return 0;
}

static const struct of_device_id renesas_jdc_of_match[] = {
	{ .compatible = "jdc,renesas" }, // FIXME
	{ }
};
MODULE_DEVICE_TABLE(of, renesas_jdc_of_match);

static struct mipi_dsi_driver renesas_jdc_driver = {
	.probe = renesas_jdc_probe,
	.remove = renesas_jdc_remove,
	.driver = {
		.name = "panel-renesas-jdc",
		.of_match_table = renesas_jdc_of_match,
	},
};
module_mipi_dsi_driver(renesas_jdc_driver);

MODULE_AUTHOR("linux-mdss-dsi-panel-driver-generator <fix@me>");
MODULE_DESCRIPTION("DRM driver for jdc renesas 1080p video");
MODULE_LICENSE("GPL v2");
