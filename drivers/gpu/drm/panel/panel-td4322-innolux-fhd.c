// SPDX-License-Identifier: GPL-2.0-only

#include <linux/delay.h>
#include <linux/gpio/consumer.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/regulator/consumer.h>

#include <drm/drm_mipi_dsi.h>
#include <drm/drm_modes.h>
#include <drm/drm_panel.h>

struct td4322_innolux_fhd {
	struct drm_panel panel;
	struct mipi_dsi_device *dsi;
	struct regulator_bulk_data supplies[4];
	struct gpio_desc *reset_gpio;
	bool prepared;
};

static inline
struct td4322_innolux_fhd *to_td4322_innolux_fhd(struct drm_panel *panel)
{
	return container_of(panel, struct td4322_innolux_fhd, panel);
}

#define dsi_dcs_write_seq(dsi, seq...) do {				\
		static const u8 d[] = { seq };				\
		int ret;						\
		ret = mipi_dsi_dcs_write_buffer(dsi, d, ARRAY_SIZE(d));	\
		if (ret < 0)						\
			return ret;					\
	} while (0)

static void td4322_innolux_fhd_reset(struct td4322_innolux_fhd *ctx)
{
	gpiod_set_value_cansleep(ctx->reset_gpio, 0);
	msleep(30);
	gpiod_set_value_cansleep(ctx->reset_gpio, 1);
	msleep(150);
}

static int td4322_innolux_fhd_on(struct td4322_innolux_fhd *ctx)
{
	struct mipi_dsi_device *dsi = ctx->dsi;
	struct device *dev = &dsi->dev;
	int ret;

	dsi->mode_flags |= MIPI_DSI_MODE_LPM;

	dsi_dcs_write_seq(dsi, 0x35);

	ret = mipi_dsi_dcs_set_display_on(dsi);
	if (ret < 0) {
		dev_err(dev, "Failed to set display on: %d\n", ret);
		return ret;
	}

	ret = mipi_dsi_dcs_exit_sleep_mode(dsi);
	if (ret < 0) {
		dev_err(dev, "Failed to exit sleep mode: %d\n", ret);
		return ret;
	}
	msleep(70);

	return 0;
}

static int td4322_innolux_fhd_off(struct td4322_innolux_fhd *ctx)
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
	msleep(120);

	return 0;
}

static int td4322_innolux_fhd_prepare(struct drm_panel *panel)
{
	struct td4322_innolux_fhd *ctx = to_td4322_innolux_fhd(panel);
	struct device *dev = &ctx->dsi->dev;
	int ret;

	if (ctx->prepared)
		return 0;

	ret = regulator_bulk_enable(ARRAY_SIZE(ctx->supplies), ctx->supplies);
	if (ret < 0) {
		dev_err(dev, "Failed to enable regulators: %d\n", ret);
		return ret;
	}

	td4322_innolux_fhd_reset(ctx);

	ret = td4322_innolux_fhd_on(ctx);
	if (ret < 0) {
		dev_err(dev, "Failed to initialize panel: %d\n", ret);
		gpiod_set_value_cansleep(ctx->reset_gpio, 0);
		regulator_bulk_disable(ARRAY_SIZE(ctx->supplies), ctx->supplies);
		return ret;
	}

	ctx->prepared = true;
	return 0;
}

static int td4322_innolux_fhd_unprepare(struct drm_panel *panel)
{
	struct td4322_innolux_fhd *ctx = to_td4322_innolux_fhd(panel);
	struct device *dev = &ctx->dsi->dev;
	int ret;

	if (!ctx->prepared)
		return 0;

	ret = td4322_innolux_fhd_off(ctx);
	if (ret < 0)
		dev_err(dev, "Failed to un-initialize panel: %d\n", ret);

	gpiod_set_value_cansleep(ctx->reset_gpio, 0);
	regulator_bulk_disable(ARRAY_SIZE(ctx->supplies), ctx->supplies);

	ctx->prepared = false;
	return 0;
}

static const struct drm_display_mode td4322_innolux_fhd_mode = {
	.clock = (1080 + 96 + 20 + 48) * (1920 + 5 + 2 + 3) * 60 / 1000,
	.hdisplay = 1080,
	.hsync_start = 1080 + 96,
	.hsync_end = 1080 + 96 + 20,
	.htotal = 1080 + 96 + 20 + 48,
	.vdisplay = 1920,
	.vsync_start = 1920 + 5,
	.vsync_end = 1920 + 5 + 2,
	.vtotal = 1920 + 5 + 2 + 3,
	.vrefresh = 60,
	.width_mm = 74,
	.height_mm = 132,
};

static int td4322_innolux_fhd_get_modes(struct drm_panel *panel,
					struct drm_connector *connector)
{
	struct drm_display_mode *mode;

	mode = drm_mode_duplicate(connector->dev, &td4322_innolux_fhd_mode);
	if (!mode)
		return -ENOMEM;

	drm_mode_set_name(mode);

	mode->type = DRM_MODE_TYPE_DRIVER | DRM_MODE_TYPE_PREFERRED;
	connector->display_info.width_mm = mode->width_mm;
	connector->display_info.height_mm = mode->height_mm;
	drm_mode_probed_add(connector, mode);

	return 1;
}

static const struct drm_panel_funcs td4322_innolux_fhd_panel_funcs = {
	.prepare = td4322_innolux_fhd_prepare,
	.unprepare = td4322_innolux_fhd_unprepare,
	.get_modes = td4322_innolux_fhd_get_modes,
};

static int td4322_innolux_fhd_probe(struct mipi_dsi_device *dsi)
{
	struct device *dev = &dsi->dev;
	struct td4322_innolux_fhd *ctx;
	int ret;

	ctx = devm_kzalloc(dev, sizeof(*ctx), GFP_KERNEL);
	if (!ctx)
		return -ENOMEM;

	ctx->supplies[0].supply = "vdd";
	ctx->supplies[1].supply = "vddio";
	ctx->supplies[2].supply = "vsn";
	ctx->supplies[3].supply = "vsp";
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

	ctx->dsi = dsi;
	mipi_dsi_set_drvdata(dsi, ctx);

	dsi->lanes = 4;
	dsi->format = MIPI_DSI_FMT_RGB888;
	dsi->mode_flags = MIPI_DSI_MODE_VIDEO_BURST |
			  MIPI_DSI_CLOCK_NON_CONTINUOUS;

	drm_panel_init(&ctx->panel, dev, &td4322_innolux_fhd_panel_funcs,
		       DRM_MODE_CONNECTOR_DSI);

	ret = drm_panel_of_backlight(&ctx->panel);
	if (ret) {
		dev_err(dev, "Failed to get backlight: %d\n", ret);
		return ret;
	}

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

static int td4322_innolux_fhd_remove(struct mipi_dsi_device *dsi)
{
	struct td4322_innolux_fhd *ctx = mipi_dsi_get_drvdata(dsi);
	int ret;

	ret = mipi_dsi_detach(dsi);
	if (ret < 0)
		dev_err(&dsi->dev, "Failed to detach from DSI host: %d\n", ret);

	drm_panel_remove(&ctx->panel);

	return 0;
}

static const struct of_device_id td4322_innolux_fhd_of_match[] = {
	{ .compatible = "sony,discovery-td4322-innolux" },
	{ /* sentinel */ }
};
MODULE_DEVICE_TABLE(of, td4322_innolux_fhd_of_match);

static struct mipi_dsi_driver td4322_innolux_fhd_driver = {
	.probe = td4322_innolux_fhd_probe,
	.remove = td4322_innolux_fhd_remove,
	.driver = {
		.name = "panel-td4322-innolux-fhd",
		.of_match_table = td4322_innolux_fhd_of_match,
	},
};
module_mipi_dsi_driver(td4322_innolux_fhd_driver);

MODULE_AUTHOR("Konrad Dybcio <konradybcio@gmail.com>");
MODULE_DESCRIPTION("DRM driver for Sony TD4322 Innolux FHD panel");
MODULE_LICENSE("GPL v2");
