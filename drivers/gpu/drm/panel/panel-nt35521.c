// SPDX-License-Identifier: GPL-2.0-only
// Copyright (c) 2020 FIXME
// Generated with linux-mdss-dsi-panel-driver-generator from vendor device tree:
//   Copyright (c) 2013, The Linux Foundation. All rights reserved. (FIXME)

#include <linux/backlight.h>
#include <linux/delay.h>
#include <linux/gpio/consumer.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/regulator/consumer.h>

#include <drm/drm_mipi_dsi.h>
#include <drm/drm_modes.h>
#include <drm/drm_panel.h>

struct nt35521 {
	struct drm_panel panel;
	struct mipi_dsi_device *dsi;
	struct regulator_bulk_data supplies[2];
	struct gpio_desc *reset_gpio;
	struct gpio_desc *backlight_gpio;
	struct gpio_desc *enable_gpio;
	bool prepared;
};

static inline struct nt35521 *to_nt35521(struct drm_panel *panel)
{
	return container_of(panel, struct nt35521, panel);
}

#define dsi_generic_write_seq(dsi, seq...) do {				\
		static const u8 d[] = { seq };				\
		int ret;						\
		ret = mipi_dsi_generic_write(dsi, d, ARRAY_SIZE(d));	\
		if (ret < 0)						\
			return ret;					\
	} while (0)

static void nt35521_reset(struct nt35521 *ctx)
{
	gpiod_set_value_cansleep(ctx->reset_gpio, 1);
	msleep(20);
	gpiod_set_value_cansleep(ctx->reset_gpio, 0);
	usleep_range(1000, 2000);
	gpiod_set_value_cansleep(ctx->reset_gpio, 1);
	msleep(20);
}

static int nt35521_on(struct nt35521 *ctx)
{
	struct mipi_dsi_device *dsi = ctx->dsi;
	struct device *dev = &dsi->dev;
	int ret;

	dsi->mode_flags |= MIPI_DSI_MODE_LPM;

	dsi_generic_write_seq(dsi, 0xf0, 0x55, 0xaa, 0x52, 0x08, 0x00);
	pr_err("HELLO DCS 1");
	dsi_generic_write_seq(dsi, 0xb1, 0x68, 0x21);
	pr_err("HELLO DCS 2");
	dsi_generic_write_seq(dsi, 0xb5, 0xc8);
	pr_err("HELLO DCS 3");
	dsi_generic_write_seq(dsi, 0xb6, 0x0f);
	pr_err("HELLO DCS 4");
	dsi_generic_write_seq(dsi, 0xb8, 0x00, 0x00, 0x0a, 0x00);
	pr_err("HELLO DCS 5");
	dsi_generic_write_seq(dsi, 0xb9, 0x00);
	pr_err("HELLO DCS 6");
	dsi_generic_write_seq(dsi, 0xba, 0x02);
	pr_err("HELLO DCS 7");
	dsi_generic_write_seq(dsi, 0xbb, 0x63, 0x63);
	pr_err("HELLO DCS 8");
	dsi_generic_write_seq(dsi, 0xbc, 0x00, 0x00);
	pr_err("HELLO DCS 9");
	dsi_generic_write_seq(dsi, 0xbd, 0x02, 0x7f, 0x0d, 0x0b, 0x00);
	pr_err("HELLO DCS 10");
	dsi_generic_write_seq(dsi, 0xcc,
			      0x41, 0x36, 0x87, 0x54, 0x46, 0x65, 0x10, 0x12,
			      0x14, 0x10, 0x12, 0x14, 0x40, 0x08, 0x15, 0x05);
				  pr_err("HELLO DCS 11");
	dsi_generic_write_seq(dsi, 0xd0, 0x00);
	pr_err("HELLO DCS 12");
	dsi_generic_write_seq(dsi, 0xd1,
			      0x00, 0x04, 0x08, 0x0c, 0x10, 0x14, 0x18, 0x1c,
			      0x20, 0x24, 0x28, 0x2c, 0x30, 0x34, 0x38, 0x3c);
				  pr_err("HELLO DCS 13");
	dsi_generic_write_seq(dsi, 0xd3, 0x00);
	pr_err("HELLO DCS 14");
	dsi_generic_write_seq(dsi, 0xd6, 0x44, 0x44);
	pr_err("HELLO DCS 15");
	dsi_generic_write_seq(dsi, 0xd7,
			      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			      0x00, 0x00, 0x00, 0x00);
				  pr_err("HELLO DCS 16");
	dsi_generic_write_seq(dsi, 0xd8,
			      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			      0x00, 0x00, 0x00, 0x00, 0x00);
				  pr_err("HELLO DCS 17");
	dsi_generic_write_seq(dsi, 0xd9, 0x03, 0x06);
	pr_err("HELLO DCS 18");
	dsi_generic_write_seq(dsi, 0xe5, 0x00, 0xff);
	pr_err("HELLO DCS 19");
	dsi_generic_write_seq(dsi, 0xe6, 0xf3, 0xec, 0xe7, 0xdf);
	pr_err("HELLO DCS 20");
	dsi_generic_write_seq(dsi, 0xe7,
			      0xf3, 0xd9, 0xcc, 0xcd, 0xb3, 0xa6, 0x99, 0x99,
			      0x99, 0x95);
				  pr_err("HELLO DCS 21");
	dsi_generic_write_seq(dsi, 0xe8,
			      0xf3, 0xd9, 0xcc, 0xcd, 0xb3, 0xa6, 0x99, 0x99,
			      0x99, 0x95);
				  pr_err("HELLO DCS 22");
	dsi_generic_write_seq(dsi, 0xe9, 0x00, 0x04);
	pr_err("HELLO DCS 23");
	dsi_generic_write_seq(dsi, 0xea, 0x00);
	pr_err("HELLO DCS 24");
	dsi_generic_write_seq(dsi, 0xee, 0x87, 0x78, 0x00, 0x00);
	pr_err("HELLO DCS 25");
	dsi_generic_write_seq(dsi, 0xef, 0x07, 0xff);
	dsi_generic_write_seq(dsi, 0xf0, 0x55, 0xaa, 0x52, 0x08, 0x01);
	dsi_generic_write_seq(dsi, 0xb0, 0x0d, 0x0d);
	dsi_generic_write_seq(dsi, 0xb1, 0x0d, 0x0d);
	dsi_generic_write_seq(dsi, 0xb3, 0x2d, 0x2d);
	dsi_generic_write_seq(dsi, 0xb4, 0x19, 0x19);
	dsi_generic_write_seq(dsi, 0xb5, 0x06, 0x06);
	dsi_generic_write_seq(dsi, 0xb6, 0x05, 0x05);
	dsi_generic_write_seq(dsi, 0xb7, 0x05, 0x05);
	dsi_generic_write_seq(dsi, 0xb8, 0x05, 0x05);
	dsi_generic_write_seq(dsi, 0xb9, 0x44, 0x44);
	dsi_generic_write_seq(dsi, 0xba, 0x36, 0x36);
	dsi_generic_write_seq(dsi, 0xbc, 0x50, 0x00);
	dsi_generic_write_seq(dsi, 0xbd, 0x50, 0x00);
	dsi_generic_write_seq(dsi, 0xbe, 0x39);
	dsi_generic_write_seq(dsi, 0xbf, 0x39);
	dsi_generic_write_seq(dsi, 0xc0, 0x0c);
	dsi_generic_write_seq(dsi, 0xc1, 0x00);
	dsi_generic_write_seq(dsi, 0xc2, 0x19, 0x19);
	dsi_generic_write_seq(dsi, 0xc3, 0x0a, 0x0a);
	dsi_generic_write_seq(dsi, 0xc4, 0x23, 0x23);
	dsi_generic_write_seq(dsi, 0xc7, 0x00, 0x80, 0x00);
	dsi_generic_write_seq(dsi, 0xc9, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
	dsi_generic_write_seq(dsi, 0xca, 0x01);
	dsi_generic_write_seq(dsi, 0xcb, 0x0b, 0x53);
	dsi_generic_write_seq(dsi, 0xcc, 0x00);
	dsi_generic_write_seq(dsi, 0xcd, 0x0b, 0x52, 0x53);
	dsi_generic_write_seq(dsi, 0xce, 0x44);
	dsi_generic_write_seq(dsi, 0xcf, 0x00, 0x50, 0x50);
	dsi_generic_write_seq(dsi, 0xd0, 0x50, 0x50);
	dsi_generic_write_seq(dsi, 0xd1, 0x50, 0x50);
	dsi_generic_write_seq(dsi, 0xd2, 0x39);
	dsi_generic_write_seq(dsi, 0xd3, 0x39);
	dsi_generic_write_seq(dsi, 0xf0, 0x55, 0xaa, 0x52, 0x08, 0x02);
	dsi_generic_write_seq(dsi, 0xb0,
			      0x00, 0xac, 0x00, 0xba, 0x00, 0xd9, 0x00, 0xed,
			      0x01, 0x01, 0x01, 0x1e, 0x01, 0x3a, 0x01, 0x62);
	dsi_generic_write_seq(dsi, 0xb1,
			      0x01, 0x85, 0x01, 0xb8, 0x01, 0xe4, 0x02, 0x27,
			      0x02, 0x5b, 0x02, 0x5d, 0x02, 0x8c, 0x02, 0xbe);
	dsi_generic_write_seq(dsi, 0xb2,
			      0x02, 0xdf, 0x03, 0x0c, 0x03, 0x2a, 0x03, 0x51,
			      0x03, 0x6d, 0x03, 0x8d, 0x03, 0xa4, 0x03, 0xbe);
	dsi_generic_write_seq(dsi, 0xb3, 0x03, 0xcc, 0x03, 0xcc);
	dsi_generic_write_seq(dsi, 0xb4,
			      0x00, 0xac, 0x00, 0xba, 0x00, 0xd9, 0x00, 0xed,
			      0x01, 0x01, 0x01, 0x1e, 0x01, 0x3a, 0x01, 0x62);
	dsi_generic_write_seq(dsi, 0xb5,
			      0x01, 0x85, 0x01, 0xb8, 0x01, 0xe4, 0x02, 0x27,
			      0x02, 0x5b, 0x02, 0x5d, 0x02, 0x8c, 0x02, 0xbe);
	dsi_generic_write_seq(dsi, 0xb6,
			      0x02, 0xdf, 0x03, 0x0c, 0x03, 0x2a, 0x03, 0x51,
			      0x03, 0x6d, 0x03, 0x8d, 0x03, 0xa4, 0x03, 0xbe);
	dsi_generic_write_seq(dsi, 0xb7, 0x03, 0xcc, 0x03, 0xcc);
	dsi_generic_write_seq(dsi, 0xb8,
			      0x00, 0xac, 0x00, 0xba, 0x00, 0xd9, 0x00, 0xed,
			      0x01, 0x01, 0x01, 0x1e, 0x01, 0x3a, 0x01, 0x62);
	dsi_generic_write_seq(dsi, 0xb9,
			      0x01, 0x85, 0x01, 0xb8, 0x01, 0xe4, 0x02, 0x27,
			      0x02, 0x5b, 0x02, 0x5d, 0x02, 0x8c, 0x02, 0xbe);
	dsi_generic_write_seq(dsi, 0xba,
			      0x02, 0xdf, 0x03, 0x0c, 0x03, 0x2a, 0x03, 0x51,
			      0x03, 0x6d, 0x03, 0x8d, 0x03, 0xa4, 0x03, 0xbe);
	dsi_generic_write_seq(dsi, 0xbb, 0x03, 0xcc, 0x03, 0xcc);
	dsi_generic_write_seq(dsi, 0xbc,
			      0x00, 0xac, 0x00, 0xba, 0x00, 0xd9, 0x00, 0xed,
			      0x01, 0x01, 0x01, 0x1e, 0x01, 0x3a, 0x01, 0x62);
	dsi_generic_write_seq(dsi, 0xbd,
			      0x01, 0x85, 0x01, 0xb8, 0x01, 0xe4, 0x02, 0x27,
			      0x02, 0x5b, 0x02, 0x5d, 0x02, 0x8c, 0x02, 0xbe);
	dsi_generic_write_seq(dsi, 0xbe,
			      0x02, 0xdf, 0x03, 0x0c, 0x03, 0x2a, 0x03, 0x51,
			      0x03, 0x6d, 0x03, 0x8d, 0x03, 0xa4, 0x03, 0xbe);
	dsi_generic_write_seq(dsi, 0xbf, 0x03, 0xcc, 0x03, 0xcc);
	dsi_generic_write_seq(dsi, 0xc0,
			      0x00, 0xac, 0x00, 0xba, 0x00, 0xd9, 0x00, 0xed,
			      0x01, 0x01, 0x01, 0x1e, 0x01, 0x3a, 0x01, 0x62);
	dsi_generic_write_seq(dsi, 0xc1,
			      0x01, 0x85, 0x01, 0xb8, 0x01, 0xe4, 0x02, 0x27,
			      0x02, 0x5b, 0x02, 0x5d, 0x02, 0x8c, 0x02, 0xbe);
	dsi_generic_write_seq(dsi, 0xc2,
			      0x02, 0xdf, 0x03, 0x0c, 0x03, 0x2a, 0x03, 0x51,
			      0x03, 0x6d, 0x03, 0x8d, 0x03, 0xa4, 0x03, 0xbe);
	dsi_generic_write_seq(dsi, 0xc3, 0x03, 0xcc, 0x03, 0xcc);
	dsi_generic_write_seq(dsi, 0xc4,
			      0x00, 0xac, 0x00, 0xba, 0x00, 0xd9, 0x00, 0xed,
			      0x01, 0x01, 0x01, 0x1e, 0x01, 0x3a, 0x01, 0x62);
	dsi_generic_write_seq(dsi, 0xc5,
			      0x01, 0x85, 0x01, 0xb8, 0x01, 0xe4, 0x02, 0x27,
			      0x02, 0x5b, 0x02, 0x5d, 0x02, 0x8c, 0x02, 0xbe);
	dsi_generic_write_seq(dsi, 0xc6,
			      0x02, 0xdf, 0x03, 0x0c, 0x03, 0x2a, 0x03, 0x51,
			      0x03, 0x6d, 0x03, 0x8d, 0x03, 0xa4, 0x03, 0xbe);
	dsi_generic_write_seq(dsi, 0xc7, 0x03, 0xcc, 0x03, 0xcc);
	dsi_generic_write_seq(dsi, 0xee, 0x00);
	dsi_generic_write_seq(dsi, 0xf0, 0x55, 0xaa, 0x52, 0x08, 0x03);
	dsi_generic_write_seq(dsi, 0xb0, 0x00, 0x00);
	dsi_generic_write_seq(dsi, 0xb1, 0x00, 0x00);
	dsi_generic_write_seq(dsi, 0xb2, 0x03, 0x00, 0x00, 0x00, 0x00);
	dsi_generic_write_seq(dsi, 0xb3, 0x03, 0x00, 0x00, 0x00, 0x00);
	dsi_generic_write_seq(dsi, 0xb4, 0x03, 0x00, 0x00, 0x00, 0x00);
	dsi_generic_write_seq(dsi, 0xb5, 0x03, 0x00, 0x00, 0x00, 0x00);
	dsi_generic_write_seq(dsi, 0xb6, 0x03, 0x00, 0x00, 0x00, 0x00);
	dsi_generic_write_seq(dsi, 0xb7, 0x03, 0x00, 0x00, 0x00, 0x00);
	dsi_generic_write_seq(dsi, 0xb8, 0x03, 0x00, 0x00, 0x00, 0x00);
	dsi_generic_write_seq(dsi, 0xb9, 0x03, 0x00, 0x00, 0x00, 0x00);
	dsi_generic_write_seq(dsi, 0xba, 0x35, 0x10, 0x00, 0x00, 0x00);
	dsi_generic_write_seq(dsi, 0xbb, 0x35, 0x10, 0x00, 0x00, 0x00);
	dsi_generic_write_seq(dsi, 0xbc, 0x35, 0x10, 0x00, 0x00, 0x00);
	dsi_generic_write_seq(dsi, 0xbd, 0x35, 0x10, 0x00, 0x00, 0x00);
	dsi_generic_write_seq(dsi, 0xc0, 0x00, 0x34, 0x00, 0x00);
	dsi_generic_write_seq(dsi, 0xc1, 0x00, 0x34, 0x00, 0x00);
	dsi_generic_write_seq(dsi, 0xc2, 0x00, 0x34, 0x00, 0x00);
	dsi_generic_write_seq(dsi, 0xc3, 0x00, 0x34, 0x00, 0x00);
	dsi_generic_write_seq(dsi, 0xc4, 0x40);
	dsi_generic_write_seq(dsi, 0xc5, 0x40);
	dsi_generic_write_seq(dsi, 0xc6, 0x40);
	dsi_generic_write_seq(dsi, 0xc7, 0x40);
	dsi_generic_write_seq(dsi, 0xef, 0x00);
	dsi_generic_write_seq(dsi, 0xf0, 0x55, 0xaa, 0x52, 0x08, 0x05);
	dsi_generic_write_seq(dsi, 0xb0, 0x1b, 0x10);
	dsi_generic_write_seq(dsi, 0xb1, 0x1b, 0x10);
	dsi_generic_write_seq(dsi, 0xb2, 0x1b, 0x10);
	dsi_generic_write_seq(dsi, 0xb3, 0x1b, 0x10);
	dsi_generic_write_seq(dsi, 0xb4, 0x1b, 0x10);
	dsi_generic_write_seq(dsi, 0xb5, 0x1b, 0x10);
	dsi_generic_write_seq(dsi, 0xb6, 0x1b, 0x10);
	dsi_generic_write_seq(dsi, 0xb7, 0x1b, 0x10);
	dsi_generic_write_seq(dsi, 0xb8, 0x00);
	dsi_generic_write_seq(dsi, 0xb9, 0x00);
	dsi_generic_write_seq(dsi, 0xba, 0x00);
	dsi_generic_write_seq(dsi, 0xbb, 0x00);
	dsi_generic_write_seq(dsi, 0xbc, 0x00);
	dsi_generic_write_seq(dsi, 0xbd, 0x03, 0x03, 0x03, 0x00, 0x01);
	dsi_generic_write_seq(dsi, 0xc0, 0x03);
	dsi_generic_write_seq(dsi, 0xc1, 0x05);
	dsi_generic_write_seq(dsi, 0xc2, 0x03);
	dsi_generic_write_seq(dsi, 0xc3, 0x05);
	dsi_generic_write_seq(dsi, 0xc4, 0x80);
	dsi_generic_write_seq(dsi, 0xc5, 0xa2);
	dsi_generic_write_seq(dsi, 0xc6, 0x80);
	dsi_generic_write_seq(dsi, 0xc7, 0xa2);
	dsi_generic_write_seq(dsi, 0xc8, 0x01, 0x20);
	dsi_generic_write_seq(dsi, 0xc9, 0x00, 0x20);
	dsi_generic_write_seq(dsi, 0xca, 0x01, 0x00);
	dsi_generic_write_seq(dsi, 0xcb, 0x00, 0x00);
	dsi_generic_write_seq(dsi, 0xcc, 0x00, 0x00, 0x01);
	dsi_generic_write_seq(dsi, 0xcd, 0x00, 0x00, 0x01);
	dsi_generic_write_seq(dsi, 0xce, 0x00, 0x00, 0x01);
	dsi_generic_write_seq(dsi, 0xcf, 0x00, 0x00, 0x01);
	dsi_generic_write_seq(dsi, 0xd0, 0x00);
	dsi_generic_write_seq(dsi, 0xd1, 0x03, 0x00, 0x00, 0x07, 0x10);
	dsi_generic_write_seq(dsi, 0xd2, 0x13, 0x00, 0x00, 0x07, 0x11);
	dsi_generic_write_seq(dsi, 0xd3, 0x23, 0x00, 0x00, 0x07, 0x10);
	dsi_generic_write_seq(dsi, 0xd4, 0x33, 0x00, 0x00, 0x07, 0x11);
	dsi_generic_write_seq(dsi, 0xe5, 0x06);
	dsi_generic_write_seq(dsi, 0xe6, 0x06);
	dsi_generic_write_seq(dsi, 0xe7, 0x06);
	dsi_generic_write_seq(dsi, 0xe8, 0x06);
	dsi_generic_write_seq(dsi, 0xe9, 0x06);
	dsi_generic_write_seq(dsi, 0xea, 0x06);
	dsi_generic_write_seq(dsi, 0xeb, 0x06);
	dsi_generic_write_seq(dsi, 0xec, 0x06);
	dsi_generic_write_seq(dsi, 0xed, 0x31);
	dsi_generic_write_seq(dsi, 0xf0, 0x55, 0xaa, 0x52, 0x08, 0x06);
	dsi_generic_write_seq(dsi, 0xb0, 0x10, 0x11);
	dsi_generic_write_seq(dsi, 0xb1, 0x12, 0x13);
	dsi_generic_write_seq(dsi, 0xb2, 0x08, 0x00);
	dsi_generic_write_seq(dsi, 0xb3, 0x2d, 0x2d);
	dsi_generic_write_seq(dsi, 0xb4, 0x2d, 0x34);
	dsi_generic_write_seq(dsi, 0xb5, 0x34, 0x2d);
	dsi_generic_write_seq(dsi, 0xb6, 0x2d, 0x34);
	dsi_generic_write_seq(dsi, 0xb7, 0x34, 0x34);
	dsi_generic_write_seq(dsi, 0xb8, 0x02, 0x0a);
	dsi_generic_write_seq(dsi, 0xb9, 0x00, 0x08);
	dsi_generic_write_seq(dsi, 0xba, 0x09, 0x01);
	dsi_generic_write_seq(dsi, 0xbb, 0x0b, 0x03);
	dsi_generic_write_seq(dsi, 0xbc, 0x34, 0x34);
	dsi_generic_write_seq(dsi, 0xbd, 0x34, 0x2d);
	dsi_generic_write_seq(dsi, 0xbe, 0x2d, 0x34);
	dsi_generic_write_seq(dsi, 0xbf, 0x34, 0x2d);
	dsi_generic_write_seq(dsi, 0xc0, 0x2d, 0x2d);
	dsi_generic_write_seq(dsi, 0xc1, 0x01, 0x09);
	dsi_generic_write_seq(dsi, 0xc2, 0x19, 0x18);
	dsi_generic_write_seq(dsi, 0xc3, 0x17, 0x16);
	dsi_generic_write_seq(dsi, 0xc4, 0x19, 0x18);
	dsi_generic_write_seq(dsi, 0xc5, 0x17, 0x16);
	dsi_generic_write_seq(dsi, 0xc6, 0x01, 0x09);
	dsi_generic_write_seq(dsi, 0xc7, 0x2d, 0x2d);
	dsi_generic_write_seq(dsi, 0xc8, 0x2d, 0x34);
	dsi_generic_write_seq(dsi, 0xc9, 0x34, 0x2d);
	dsi_generic_write_seq(dsi, 0xca, 0x2d, 0x34);
	dsi_generic_write_seq(dsi, 0xcb, 0x34, 0x34);
	dsi_generic_write_seq(dsi, 0xcc, 0x0b, 0x03);
	dsi_generic_write_seq(dsi, 0xcd, 0x09, 0x01);
	dsi_generic_write_seq(dsi, 0xce, 0x00, 0x08);
	dsi_generic_write_seq(dsi, 0xcf, 0x02, 0x0a);
	dsi_generic_write_seq(dsi, 0xd0, 0x34, 0x34);
	dsi_generic_write_seq(dsi, 0xd1, 0x34, 0x2d);
	dsi_generic_write_seq(dsi, 0xd2, 0x2d, 0x34);
	dsi_generic_write_seq(dsi, 0xd3, 0x34, 0x2d);
	dsi_generic_write_seq(dsi, 0xd4, 0x2d, 0x2d);
	dsi_generic_write_seq(dsi, 0xd5, 0x08, 0x00);
	dsi_generic_write_seq(dsi, 0xd6, 0x10, 0x11);
	dsi_generic_write_seq(dsi, 0xd7, 0x12, 0x13);
	dsi_generic_write_seq(dsi, 0xd8, 0x55, 0x55, 0x55, 0x55, 0x55);
	dsi_generic_write_seq(dsi, 0xd9, 0x55, 0x55, 0x55, 0x55, 0x55);
	dsi_generic_write_seq(dsi, 0xe5, 0x34, 0x34);
	dsi_generic_write_seq(dsi, 0xe6, 0x34, 0x34);
	dsi_generic_write_seq(dsi, 0xe7, 0x05);
	dsi_generic_write_seq(dsi, 0xf0, 0x55, 0xaa, 0x52, 0x00, 0x00);

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
	msleep(20);

	dsi_generic_write_seq(dsi, 0xf0, 0x55, 0xaa, 0x52, 0x08, 0x01);
	dsi_generic_write_seq(dsi, 0xf0, 0x55, 0xaa, 0x52, 0x00, 0x00);
	dsi_generic_write_seq(dsi, 0x53, 0x2c);

	return 0;
}

static int nt35521_off(struct nt35521 *ctx)
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

	ret = mipi_dsi_dcs_enter_sleep_mode(dsi);
	if (ret < 0) {
		dev_err(dev, "Failed to enter sleep mode: %d\n", ret);
		return ret;
	}

	return 0;
}

static int nt35521_prepare(struct drm_panel *panel)
{
	struct nt35521 *ctx = to_nt35521(panel);
	struct device *dev = &ctx->dsi->dev;
	int ret;

	if (ctx->prepared)
		return 0;

	ret = regulator_bulk_enable(ARRAY_SIZE(ctx->supplies), ctx->supplies);
	if (ret < 0) {
		dev_err(dev, "Failed to enable regulators: %d\n", ret);
		return ret;
	}

	nt35521_reset(ctx);

	gpiod_set_value(ctx->enable_gpio, 1);
	msleep(20);

	ret = nt35521_on(ctx);
	if (ret < 0) {
		dev_err(dev, "Failed to initialize panel: %d\n", ret);
		gpiod_set_value_cansleep(ctx->reset_gpio, 0);
		regulator_bulk_disable(ARRAY_SIZE(ctx->supplies), ctx->supplies);
		return ret;
	}

	ctx->prepared = true;
	return 0;
}

static int nt35521_unprepare(struct drm_panel *panel)
{
	struct nt35521 *ctx = to_nt35521(panel);
	struct device *dev = &ctx->dsi->dev;
	int ret;

	if (!ctx->prepared)
		return 0;

	ret = nt35521_off(ctx);
	if (ret < 0)
		dev_err(dev, "Failed to un-initialize panel: %d\n", ret);

	gpiod_set_value_cansleep(ctx->reset_gpio, 0);
	regulator_bulk_disable(ARRAY_SIZE(ctx->supplies), ctx->supplies);

	ctx->prepared = false;
	return 0;
}

static const struct drm_display_mode nt35521_mode = {
	.clock = (720 + 44 + 11 + 55) * (1280 + 14 + 1 + 15) * 60 / 1000,
	.hdisplay = 720,
	.hsync_start = 720 + 44,
	.hsync_end = 720 + 44 + 11,
	.htotal = 720 + 44 + 11 + 55,
	.vdisplay = 1280,
	.vsync_start = 1280 + 14,
	.vsync_end = 1280 + 14 + 1,
	.vtotal = 1280 + 14 + 1 + 15,
	.vrefresh = 60,
	.width_mm = 62,
	.height_mm = 111,
};

static int nt35521_get_modes(struct drm_panel *panel,
			     struct drm_connector *connector)
{
	struct drm_display_mode *mode;

	mode = drm_mode_duplicate(connector->dev, &nt35521_mode);
	if (!mode)
		return -ENOMEM;

	drm_mode_set_name(mode);

	mode->type = DRM_MODE_TYPE_DRIVER | DRM_MODE_TYPE_PREFERRED;
	connector->display_info.width_mm = mode->width_mm;
	connector->display_info.height_mm = mode->height_mm;
	drm_mode_probed_add(connector, mode);

	return 1;
}

static const struct drm_panel_funcs nt35521_panel_funcs = {
	.prepare = nt35521_prepare,
	.unprepare = nt35521_unprepare,
	.get_modes = nt35521_get_modes,
};

static int nt35521_bl_update_status(struct backlight_device *bl)
{
	struct mipi_dsi_device *dsi = bl_get_data(bl);
	struct nt35521 *ctx = mipi_dsi_get_drvdata(dsi);
	u16 brightness = bl->props.brightness;
	int ret;

	if (bl->props.power != FB_BLANK_UNBLANK ||
	    bl->props.fb_blank != FB_BLANK_UNBLANK ||
	    bl->props.state & (BL_CORE_SUSPENDED | BL_CORE_FBBLANK))
		brightness = 0;

	gpiod_set_value_cansleep(ctx->backlight_gpio, !!brightness);

	dsi->mode_flags &= ~MIPI_DSI_MODE_LPM;

	ret = mipi_dsi_dcs_set_display_brightness(dsi, brightness);
	if (ret < 0)
		return ret;

	dsi->mode_flags |= MIPI_DSI_MODE_LPM;

	return 0;
}

// TODO: Check if /sys/class/backlight/.../actual_brightness actually returns
// correct values. If not, remove this function.
static int nt35521_bl_get_brightness(struct backlight_device *bl)
{
	struct mipi_dsi_device *dsi = bl_get_data(bl);
	u16 brightness = bl->props.brightness;
	int ret;

	dsi->mode_flags &= ~MIPI_DSI_MODE_LPM;

	ret = mipi_dsi_dcs_get_display_brightness(dsi, &brightness);
	if (ret < 0)
		return ret;

	dsi->mode_flags |= MIPI_DSI_MODE_LPM;

	return brightness & 0xff;
}

static const struct backlight_ops nt35521_bl_ops = {
	.update_status = nt35521_bl_update_status,
	.get_brightness = nt35521_bl_get_brightness,
};

static struct backlight_device *
nt35521_create_backlight(struct mipi_dsi_device *dsi)
{
	struct device *dev = &dsi->dev;
	struct backlight_properties props = {
		.type = BACKLIGHT_RAW,
		.brightness = 255,
		.max_brightness = 255,
	};

	return devm_backlight_device_register(dev, dev_name(dev), dev, dsi,
					      &nt35521_bl_ops, &props);
}

static int nt35521_probe(struct mipi_dsi_device *dsi)
{
	struct device *dev = &dsi->dev;
	struct nt35521 *ctx;
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

	ctx->enable_gpio = devm_gpiod_get(dev, "enable", GPIOD_OUT_HIGH);
    if (IS_ERR(ctx->enable_gpio)) {
		ret = PTR_ERR(ctx->enable_gpio);
		dev_err(dev, "Failed to get enable-gpios: %d\n", ret);
		return ret;
    }


	ctx->backlight_gpio = devm_gpiod_get(dev, "backlight", GPIOD_OUT_LOW);
	if (IS_ERR(ctx->backlight_gpio)) {
		ret = PTR_ERR(ctx->backlight_gpio);
		dev_err(dev, "Failed to get backlight-gpios: %d\n", ret);
		return ret;
	}

	ctx->dsi = dsi;
	mipi_dsi_set_drvdata(dsi, ctx);

	dsi->lanes = 4;
	dsi->format = MIPI_DSI_FMT_RGB888;
	dsi->mode_flags = MIPI_DSI_MODE_VIDEO | MIPI_DSI_MODE_VIDEO_BURST |
			  MIPI_DSI_MODE_VIDEO_HSE | MIPI_DSI_MODE_EOT_PACKET |
			  MIPI_DSI_CLOCK_NON_CONTINUOUS;

	drm_panel_init(&ctx->panel, dev, &nt35521_panel_funcs,
		       DRM_MODE_CONNECTOR_DSI);

	ctx->panel.backlight = nt35521_create_backlight(dsi);
	if (IS_ERR(ctx->panel.backlight)) {
		ret = PTR_ERR(ctx->panel.backlight);
		dev_err(dev, "Failed to create backlight: %d\n", ret);
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

static int nt35521_remove(struct mipi_dsi_device *dsi)
{
	struct nt35521 *ctx = mipi_dsi_get_drvdata(dsi);
	int ret;

	ret = mipi_dsi_detach(dsi);
	if (ret < 0)
		dev_err(&dsi->dev, "Failed to detach from DSI host: %d\n", ret);

	drm_panel_remove(&ctx->panel);

	return 0;
}

static const struct of_device_id nt35521_of_match[] = {
	{ .compatible = "mdss,nt35521" }, // FIXME
	{ /* sentinel */ }
};
MODULE_DEVICE_TABLE(of, nt35521_of_match);

static struct mipi_dsi_driver nt35521_driver = {
	.probe = nt35521_probe,
	.remove = nt35521_remove,
	.driver = {
		.name = "panel-nt35521",
		.of_match_table = nt35521_of_match,
	},
};
module_mipi_dsi_driver(nt35521_driver);

MODULE_AUTHOR("linux-mdss-dsi-panel-driver-generator <fix@me>"); // FIXME
MODULE_DESCRIPTION("DRM driver for nt35521 720p video mode dsi panel");
MODULE_LICENSE("GPL v2");
