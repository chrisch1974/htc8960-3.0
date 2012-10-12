#include "msm_sensor.h"
#define SENSOR_NAME "s5k4e5yx"
#define PLATFORM_DRIVER_NAME "msm_camera_s5k4e5yx"
#define s5k4e5yx_obj s5k4e5yx_##obj

#define S5K4E5YX_REG_READ_MODE 0x0101
#define S5K4E5YX_READ_NORMAL_MODE 0x0000	/* without mirror/flip */
#define S5K4E5YX_READ_MIRROR 0x0001			/* with mirror */
#define S5K4E5YX_READ_FLIP 0x0002			/* with flip */
#define S5K4E5YX_READ_MIRROR_FLIP 0x0003	/* with mirror/flip */

DEFINE_MUTEX(s5k4e5yx_mut);
static struct msm_sensor_ctrl_t s5k4e5yx_s_ctrl;

static struct msm_camera_i2c_reg_conf s5k4e5yx_start_settings[] = {
	{0x0100, 0x01},
};

static struct msm_camera_i2c_reg_conf s5k4e5yx_stop_settings[] = {
	{0x0100, 0x00},
};

static struct msm_camera_i2c_reg_conf s5k4e5yx_groupon_settings[] = {
	{0x104, 0x01},
};

static struct msm_camera_i2c_reg_conf s5k4e5yx_groupoff_settings[] = {
	{0x104, 0x00},
};

static struct msm_camera_i2c_reg_conf s5k4e5yx_prev_settings[] = {
	{0x30BD, 0x00}, /*SEL_CCP[0]*/
	{0x3084, 0x15}, /*SYNC Mode*/
	{0x30BE, 0x1A}, /*M_PCLKDIV_AUTO[4], M_DIV_PCLK[3:0]*/
	{0x30C1, 0x01}, /*pack video enable [0]*/
	{0x30EE, 0x02}, /*DPHY enable [1]*/
	{0x3111, 0x86}, /*Embedded data off [5]*/
	{0x30E8, 0x07},	/*Non-Continuous mode*/

	{0x30E3, 0x38},	/*According to MCLK*/
	{0x30E4, 0x40},
	{0x3113, 0x70},
	{0x3114, 0x80},
	{0x3115, 0x7B},
	{0x3116, 0xC0},
	{0x30EE, 0x12},

	/* PLL setting ...*/
	{0x0305, 0x04},
	{0x0306, 0x00},
	{0x0307, 0x66},
	{0x30B5, 0x01},
	{0x30E2, 0x02}, /*num lanes[1:0] = 2*/
	{0x30F1, 0xA0},	/* DPHY Band Control*/

	{0x30BC, 0xA8},	/* [7]bit : DBLR enable, [6]~[3]bit : DBLR Div*/
									/* DBLR clock = Pll output/DBLR Div = 61.2Mhz*/

	{0x30BF, 0xAB}, /*outif_enable[7], data_type[5:0](2Bh = bayer 10bit)*/
	{0x30C0, 0xA0}, /*video_offset[7:4]*/
	{0x30C8, 0x06}, /*video_data_length*/
	{0x30C9, 0x5E},

	{0x3112, 0x00},	/*gain option sel off*/
	{0x3030, 0x07},	/*old shut mode*/

	/*--> This register are for user.*/
	/*+++++++++++++++++++++++++++++++*/
	/* Integration setting ...*/
	{0x0200, 0x05}, /* fine integration time*/
	{0x0201, 0x52},
	{0x0202, 0x03}, /* coarse integration time*/
	{0x0203, 0xd0},
	{0x0204, 0x00}, /* Analog Gain*/
	{0x0205, 0x20},
	{0x0340, 0x04}, /* Frame Length*/
	{0x0341, 0xFC},
	{0x0342, 0x0C}, /* Min Line Length(0x0AF0) @ 610Mclk*/
	{0x0343, 0x6F},

	/*Size Setting ...*/
	/* 1304x980*/
	{0x30A9, 0x02}, /*Horizontal Binning On*/
	{0x300E, 0x29}, /*Vertical Binning On*/
	{0x302B, 0x00},
	{0x3029, 0x74}, /* DBLR & PLA*/

	{0x0380, 0x00}, /*x_even_inc 1*/
	{0x0381, 0x01},
	{0x0382, 0x00}, /*x_odd_inc 1*/
	{0x0383, 0x01},
	{0x0384, 0x00}, /*y_even_inc 1*/
	{0x0385, 0x01},
	{0x0386, 0x00}, /*y_odd_inc 3*/
	{0x0387, 0x03},

	{0x0344, 0x00}, /*x_addr_start*/
	{0x0345, 0x00},
	{0x0346, 0x00}, /*y_addr_start*/
	{0x0347, 0x00},
	{0x0348, 0x0A}, /*x_addr_end*/
	{0x0349, 0x2F},
	{0x034A, 0x07}, /*y_addr_end*/
	{0x034B, 0xA7},

	{0x034C, 0x05}, /*x_output_size_High*/
	{0x034D, 0x18},
	{0x034E, 0x03}, /*y_output_size_High*/
	{0x034F, 0xD4},
};

static struct msm_camera_i2c_reg_conf s5k4e5yx_video_settings[] = {
	/*Timing configuration*/
	{0x0200, 0x05}, /* fine integration time */
	{0x0201, 0x52},
	{0x0202, 0x03}, /* coarse integration time */
	{0x0203, 0xd0},
	{0x0204, 0x00}, /* Analog Gain */
	{0x0205, 0x20},
	{0x0342, 0x0B}, /* Min Line Length(0x0AF0) @ 610Mclk	*/
	{0x0343, 0x18},
	{0x0340, 0x07}, /* Frame Length */
	{0x0341, 0xB4},
	/*Output Size (1640x1232)*/
	{0x30A9, 0x03}, /* Horizontal Binning Off */
	{0x300E, 0x28}, /* Vertical Binning Off */
	{0x0380, 0x00}, /* x_even_inc 1 */
	{0x0381, 0x01},
	{0x0382, 0x00}, /* x_odd_inc 1 */
	{0x0383, 0x01},
	{0x0384, 0x00}, /* y_even_inc 1 */
	{0x0385, 0x01},
	{0x0386, 0x00}, /* y_odd_inc 3 */
	{0x0387, 0x01},
	{0x0105, 0x01}, /* 20110408 Aaron*/
	{0x0344, 0x00}, /* x_addr_start */
	{0x0345, 0x00},
	{0x0346, 0x00}, /* y_addr_start */
	{0x0347, 0x00},
	{0x0348, 0x0A}, /* x_addr_end */
	{0x0349, 0x2F},
	{0x034A, 0x07}, /* y_addr_end */
	{0x034B, 0xA7},
	{0x034C, 0x0A}, /* x_output_size_High */
	{0x034D, 0x30},
	{0x034E, 0x07}, /* y_output_size_High */
	{0x034F, 0xA8},
	{0x30BD, 0x00}, /* SEL_CCP[0] */
	{0x3084, 0x15}, /* SYNC Mode */
	{0x30BE, 0x1A}, /* M_PCLKDIV_AUTO[4], M_DIV_PCLK[3:0] */
	{0x30C1, 0x01}, /* pack video enable [0] */
	{0x30EE, 0x02}, /* DPHY enable [1] */
	{0x3111, 0x86}, /* Embedded data off [5] */
	{0x30E8, 0x07}, /* Non-Continuous mode */
	{0x30E3, 0x38}, /* According to MCLK */
	{0x30E4, 0x40},
	{0x3113, 0x70},
	{0x3114, 0x80},
	{0x3115, 0x7B},
	{0x3116, 0xC0},
	{0x30EE, 0x12},
	{0x0305, 0x06},
	{0x0306, 0x00},
	{0x0307, 0x69},
	{0x30B5, 0x00},
	{0x30E2, 0x02}, /* num lanes[1:0] = 2 */
	{0x30F1, 0xD0}, /* DPHY Band Control */
	{0x30BC, 0xB0}, /* [7]bit : DBLR enable, [6]~[3]bit : DBLR Div */
	{0x30BF, 0xAB}, /* outif_enable[7], data_type[5:0](2Bh = bayer 10bit) */
	{0x30C0, 0x80}, /* video_offset[7:4] */
	{0x30C8, 0x0C}, /* video_data_length */
	{0x30C9, 0xBC},
	{0x3112, 0x00}, /* 20110408 Aaron*/
	{0x3030, 0x07},
};

static struct msm_camera_i2c_reg_conf s5k4e5yx_fast_video_settings[] = {
	{0x30BD, 0x00},	/*SEL_CCP[0]*/
	{0x3084, 0x15},	/*SYNC Mode*/
	{0x30BE, 0x1A},	/*M_PCLKDIV_AUTO[4], M_DIV_PCLK[3:0]*/
	{0x30C1, 0x01},	/*pack video enable [0]*/
	{0x30EE, 0x02},	/*DPHY enable [1]*/
	{0x3111, 0x86},	/*Embedded data off [5]*/
	{0x30E8, 0x0F},	/*Continuous mode*/

	{0x30E3, 0x38},	/*According to MCLK*/
	{0x30E4, 0x40},
	{0x3113, 0x70},
	{0x3114, 0x80},
	{0x3115, 0x7B},
	{0x3116, 0xC0},
	{0x30EE, 0x12},

	/* PLL setting ...	*/
	{0x0305, 0x06},
	{0x0306, 0x00},
	{0x0307, 0x69},
	{0x30B5, 0x00},
	{0x30E2, 0x02}, /*num lanes[1:0] = 2*/
	{0x30F1, 0xD0},	/*DPHY Band Control*/

	{0x30BC, 0xB0},	/*[7]bit : DBLR enable, [6]~[3]bit : DBLR Div*/
									/*DBLR clock = Pll output/DBLR Div = 70Mhz*/

	{0x30BF, 0xAB}, /*outif_enable[7], data_type[5:0](2Bh = bayer 10bit)*/
	{0x30C0, 0xA0}, /*video_offset[7:4]*/
	{0x30C8, 0x06}, /*video_data_length*/
	{0x30C9, 0x5E},

	{0x3112, 0x00},	/*gain option sel off*/
	{0x3030, 0x07},	/*old shut mode*/

	/*--> This register are for user.*/
	/*+++++++++++++++++++++++++++++++*/
	/* Integration setting ...*/
	{0x0200, 0x05}, /* fine integration time*/
	{0x0201, 0x52},
	{0x0202, 0x03},   /* coarse integration time*/
	{0x0203, 0xd0},
	{0x0204, 0x00},  /* Analog Gain*/
	{0x0205, 0x20},
	{0x0340, 0x03},   /* Frame Length*/
	{0x0341, 0xE0},
	{0x0342, 0x0B},   /* Min Line Length(0x0B18) @ 840Mclk*/
	{0x0343, 0x18},
	/*Size Setting ...*/
	/* 1304x980*/
	{0x30A9, 0x02}, /*Horizontal Binning On*/
	{0x300E, 0x29}, /*Vertical Binning On*/
	{0x302B, 0x00},
	{0x3029, 0x74}, /* DBLR & PLA*/

	{0x0380, 0x00}, /*x_even_inc 1*/
	{0x0381, 0x01},
	{0x0382, 0x00}, /*x_odd_inc 1*/
	{0x0383, 0x01},
	{0x0384, 0x00}, /*y_even_inc 1*/
	{0x0385, 0x01},
	{0x0386, 0x00}, /*y_odd_inc 3*/
	{0x0387, 0x03},

	{0x0344, 0x00}, /*x_addr_start*/
	{0x0345, 0x00},
	{0x0346, 0x00}, /*y_addr_start*/
	{0x0347, 0x00},
	{0x0348, 0x0A}, /*x_addr_end*/
	{0x0349, 0x2F},
	{0x034A, 0x07}, /*y_addr_end*/
	{0x034B, 0xA7},

	{0x034C, 0x05}, /*x_output_size_High*/
	{0x034D, 0x18},
	{0x034E, 0x03}, /*y_output_size_High*/
	{0x034F, 0xD4},

};

static struct msm_camera_i2c_reg_conf s5k4e5yx_snap_settings[] = {
	{0x30BD, 0x00}, /*SEL_CCP[0]*/
	{0x3084, 0x15}, /*SYNC Mode*/
	{0x30BE, 0x1A}, /*M_PCLKDIV_AUTO[4], M_DIV_PCLK[3:0]*/
	{0x30C1, 0x01}, /*pack video enable [0]*/
	{0x30EE, 0x02}, /*DPHY enable [1]*/
	{0x3111, 0x86}, /*Embedded data off [5]*/
	{0x30E8, 0x07},	/*Non-Continuous mode*/

	{0x30E3, 0x38},	/*According to MCLK*/
	{0x30E4, 0x40},
	{0x3113, 0x70},
	{0x3114, 0x80},
	{0x3115, 0x7B},
	{0x3116, 0xC0},
	{0x30EE, 0x12},

	/* PLL setting ...*/
	{0x0305, 0x06},
	{0x0306, 0x00},
	{0x0307, 0x69},
	{0x30B5, 0x00},
	{0x30E2, 0x02}, /*num lanes[1:0] = 2*/
	{0x30F1, 0xD0},	/* DPHY Band Control*/

	{0x30BC, 0xB0},	/* [7]bit : DBLR enable, [6]~[3]bit : DBLR Div*/
	/* DBLR clock = Pll output/DBLR Div = 70Mhz*/

	{0x30BF, 0xAB}, /*outif_enable[7], data_type[5:0](2Bh = bayer 10bit)*/
	{0x30C0, 0x80}, /*video_offset[7:4]*/
	{0x30C8, 0x0C}, /*video_data_length*/
	{0x30C9, 0xBC},

	{0x3112, 0x00},	/*gain option sel off*/
	{0x3030, 0x07},	/*old shut mode*/

	/*--> This register are for user.*/
	/*+++++++++++++++++++++++++++++++*/
	/* Integration setting ...*/
	{0x0200, 0x05}, /* fine integration time*/
	{0x0201, 0x52},
	{0x0202, 0x03}, /* coarse integration time*/
	{0x0203, 0xd0},
	{0x0204, 0x00}, /* Analog Gain*/
	{0x0205, 0x20},
	{0x0340, 0x07}, /* Frame Length*/
	{0x0341, 0xB4},
	{0x0342, 0x0B}, /* Min Line Length(0x0B18) @ 840Mclk*/
	{0x0343, 0x18},

	/*Size Setting ...*/
	/* 2608x1960*/
	{0x30A9, 0x03}, /*Horizontal Binning Off*/
	{0x300E, 0x28}, /*Vertical Binning Off*/
	{0x302B, 0x01},
	{0x3029, 0x34}, /* DBLR & PLA*/

	{0x0380, 0x00}, /*x_even_inc 1*/
	{0x0381, 0x01},
	{0x0382, 0x00}, /*x_odd_inc 1*/
	{0x0383, 0x01},
	{0x0384, 0x00}, /*y_even_inc 1*/
	{0x0385, 0x01},
	{0x0386, 0x00}, /*y_odd_inc 3*/
	{0x0387, 0x01},

	{0x0344, 0x00}, /*x_addr_start*/
	{0x0345, 0x00},
	{0x0346, 0x00}, /*y_addr_start*/
	{0x0347, 0x00},
	{0x0348, 0x0A}, /*x_addr_end*/
	{0x0349, 0x2F},
	{0x034A, 0x07}, /*y_addr_end*/
	{0x034B, 0xA7},

	{0x034C, 0x0A}, /*x_output_size_High*/
	{0x034D, 0x30},
	{0x034E, 0x07}, /*y_output_size_High*/
	{0x034F, 0xA8},
};

static struct msm_camera_i2c_reg_conf s5k4e5yx_recommend_settings[] = {
	{0x3000, 0x05},
	{0x3001, 0x03},
	{0x3002, 0x08},
	{0x3003, 0x0A},
	{0x3004, 0x50},
	{0x3005, 0x0E},
	{0x3006, 0x5E},
	{0x3007, 0x00},
	{0x3008, 0x78},
	{0x3009, 0x78},
	{0x300A, 0x50},
	{0x300B, 0x08},
	{0x300C, 0x14},
	{0x300D, 0x00},
	{0x300F, 0x40},
	{0x301B, 0x77},

	/* CDS option setting ...	*/
	{0x3010, 0x00},
	{0x3011, 0x3A},

	{0x3012, 0x30},
	{0x3013, 0xA0},
	{0x3014, 0x00},
	{0x3015, 0x00},
	{0x3016, 0x52},
	{0x3017, 0x94},
	{0x3018, 0x70},

	{0x301D, 0xD4},

	{0x3021, 0x02},
	{0x3022, 0x24},
	{0x3024, 0x40},
	{0x3027, 0x08},
	/* Pixel option setting ...*/
	{0x301C, 0x06},
	{0x30D8, 0x3F},

	/* ADLC setting ...*/
	{0x3070, 0x5F},
	{0x3071, 0x00},
	{0x3080, 0x04},
	{0x3081, 0x38},
	{0x302E, 0x0B},

};

static struct v4l2_subdev_info s5k4e5yx_subdev_info[] = {
	{
	.code   = V4L2_MBUS_FMT_SBGGR10_1X10,
	.colorspace = V4L2_COLORSPACE_JPEG,
	.fmt    = 1,
	.order    = 0,
	},
	/* more can be supported, to be added later */
};

static struct msm_camera_i2c_conf_array s5k4e5yx_init_conf[] = {
	{&s5k4e5yx_recommend_settings[0],
	ARRAY_SIZE(s5k4e5yx_recommend_settings), 0, MSM_CAMERA_I2C_BYTE_DATA}
};

static struct msm_camera_i2c_conf_array s5k4e5yx_confs[] = {
	{&s5k4e5yx_snap_settings[0],
	ARRAY_SIZE(s5k4e5yx_snap_settings), 0, MSM_CAMERA_I2C_BYTE_DATA},
	{&s5k4e5yx_prev_settings[0],
	ARRAY_SIZE(s5k4e5yx_prev_settings), 0, MSM_CAMERA_I2C_BYTE_DATA},
	{&s5k4e5yx_video_settings[0],
	ARRAY_SIZE(s5k4e5yx_video_settings), 0, MSM_CAMERA_I2C_BYTE_DATA},
	{&s5k4e5yx_fast_video_settings[0],
	ARRAY_SIZE(s5k4e5yx_fast_video_settings), 0, MSM_CAMERA_I2C_BYTE_DATA},
};

static struct msm_sensor_output_info_t s5k4e5yx_dimensions[] = {
	{
		.x_output = 0xA30,
		.y_output = 0x7A8,
		.line_length_pclk = 0xB18,
		.frame_length_lines = 0x7B4,
		.vt_pixel_clk = 168000000,
		.op_pixel_clk = 168000000,
		.binning_factor = 1,
	},
	{
		.x_output = 0x518,
		.y_output = 0x3D4,
		.line_length_pclk = 0xC6F,
		.frame_length_lines = 0x4FC,
		.vt_pixel_clk = 122400000,
		.op_pixel_clk = 122400000,
		.binning_factor = 1,
	},
	{
		.x_output = 0xA30,
		.y_output = 0x7A8,
		.line_length_pclk = 0xB18,
		.frame_length_lines = 0x7B4,
		.vt_pixel_clk = 168000000,
		.op_pixel_clk = 168000000,
		.binning_factor = 1,
	},
	{
		.x_output = 0x518,
		.y_output = 0x3D4,
		.line_length_pclk = 0xB18,
		.frame_length_lines = 0x3E0,
		.vt_pixel_clk = 168000000,
		.op_pixel_clk = 168000000,
		.binning_factor = 1,
	},
};

static struct msm_camera_csid_vc_cfg s5k4e5yx_cid_cfg[] = {
	{0, CSI_RAW10, CSI_DECODE_10BIT},
	{1, CSI_EMBED_DATA, CSI_DECODE_8BIT},
};

static struct msm_camera_csi2_params s5k4e5yx_csi_params = {
	.csid_params = {
		.lane_assign = 0xe4,
		.lane_cnt = 2,
		.lut_params = {
			.num_cid = 2,
			.vc_cfg = s5k4e5yx_cid_cfg,
		},
	},
	.csiphy_params = {
		.lane_cnt = 2,
		.settle_cnt = 20,
	},
};

static struct msm_camera_csi2_params *s5k4e5yx_csi_params_array[] = {
	&s5k4e5yx_csi_params,
	&s5k4e5yx_csi_params,
	&s5k4e5yx_csi_params,
	&s5k4e5yx_csi_params
};

static struct msm_sensor_output_reg_addr_t s5k4e5yx_reg_addr = {
	.x_output = 0x34C,
	.y_output = 0x34E,
	.line_length_pclk = 0x342,
	.frame_length_lines = 0x340,
};

static struct msm_sensor_id_info_t s5k4e5yx_id_info = {
	.sensor_id_reg_addr = 0x0,
	.sensor_id = 0x4E50,
};

static struct msm_sensor_exp_gain_info_t s5k4e5yx_exp_gain_info = {
	.coarse_int_time_addr = 0x202,
	.global_gain_addr = 0x204,
	.vert_offset = 4,
	.min_vert = 4, /* min coarse integration time */ /* HTC Angie 20111019 - Fix FPS */
};

static int s5k4e5yx_sensor_config(void __user *argp)
{
	return msm_sensor_config(&s5k4e5yx_s_ctrl, argp);
}

static int s5k4e5yx_sensor_open_init(const struct msm_camera_sensor_info *data)
{
	int rc;
	uint16_t value = 0;

	if (data->sensor_platform_info)
		s5k4e5yx_s_ctrl.mirror_flip = data->sensor_platform_info->mirror_flip;

	rc = msm_sensor_open_init(&s5k4e5yx_s_ctrl, data);
	if (rc < 0) {
		pr_err("%s failed to sensor open init\n", __func__);
		return rc;
	}

	/* Apply sensor mirror/flip */
	if (s5k4e5yx_s_ctrl.mirror_flip == CAMERA_SENSOR_MIRROR_FLIP)
		value = S5K4E5YX_READ_MIRROR_FLIP;
	else if (s5k4e5yx_s_ctrl.mirror_flip == CAMERA_SENSOR_MIRROR)
		value = S5K4E5YX_READ_MIRROR;
	else if (s5k4e5yx_s_ctrl.mirror_flip == CAMERA_SENSOR_FLIP)
		value = S5K4E5YX_READ_FLIP;
	else
		value = S5K4E5YX_READ_NORMAL_MODE;
	msm_camera_i2c_write(s5k4e5yx_s_ctrl.sensor_i2c_client,
		S5K4E5YX_REG_READ_MODE, value, MSM_CAMERA_I2C_BYTE_DATA);

	return rc;
}

static int s5k4e5yx_sensor_release(void)
{
	int	rc;
	rc = msm_sensor_release(&s5k4e5yx_s_ctrl);

	return rc;
}

static const char *s5k4e5yxVendor = "samsung";
static const char *s5k4e5yxNAME = "s5k4e5yx";
static const char *s5k4e5yxSize = "5M";

static ssize_t sensor_vendor_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	ssize_t ret = 0;

	sprintf(buf, "%s %s %s\n", s5k4e5yxVendor, s5k4e5yxNAME, s5k4e5yxSize);
	ret = strlen(buf) + 1;

	return ret;
}

static DEVICE_ATTR(sensor, 0444, sensor_vendor_show, NULL);

static struct kobject *android_s5k4e5yx;

static int s5k4e5yx_sysfs_init(void)
{
	int ret ;
	pr_info("s5k4e5yx:kobject creat and add\n");
	android_s5k4e5yx = kobject_create_and_add("android_camera", NULL);
	if (android_s5k4e5yx == NULL) {
		pr_info("s5k4e5yx_sysfs_init: subsystem_register " \
		"failed\n");
		ret = -ENOMEM;
		return ret ;
	}
	pr_info("s5k4e5yx:sysfs_create_file\n");
	ret = sysfs_create_file(android_s5k4e5yx, &dev_attr_sensor.attr);
	if (ret) {
		pr_info("s5k4e5yx_sysfs_init: sysfs_create_file " \
		"failed\n");
		kobject_del(android_s5k4e5yx);
	}

	return 0 ;
}

static const struct i2c_device_id s5k4e5yx_i2c_id[] = {
	{SENSOR_NAME, (kernel_ulong_t)&s5k4e5yx_s_ctrl},
	{ }
};

static struct i2c_driver s5k4e5yx_i2c_driver = {
	.id_table = s5k4e5yx_i2c_id,
	.probe  = msm_sensor_i2c_probe,
	.driver = {
		.name = SENSOR_NAME,
	},
};

static struct msm_camera_i2c_client s5k4e5yx_sensor_i2c_client = {
	.addr_type = MSM_CAMERA_I2C_WORD_ADDR,
};

static int s5k4e5yx_sensor_v4l2_probe(const struct msm_camera_sensor_info *info,
	struct v4l2_subdev *sdev, struct msm_sensor_ctrl *s)
{
	int rc = -EINVAL;

	rc = msm_sensor_v4l2_probe(&s5k4e5yx_s_ctrl, info, sdev, s);

	return rc;
}

int32_t s5k4e5yx_power_up(const struct msm_camera_sensor_info *sdata)
{
	int rc;
	pr_info("%s\n", __func__);

	rc = msm_camio_clk_enable(CAMIO_CAM_MCLK_CLK);
	if (rc < 0) {
		pr_err("%s: msm_camio_sensor_clk_on failed:%d\n",
		 __func__, rc);
		return rc;
	}

	if (sdata->camera_power_on == NULL) {
		pr_err("sensor platform_data didnt register\n");
		return -EIO;
	}

	rc = sdata->camera_power_on();
	if (rc < 0) {
		pr_err("%s failed to enable power\n", __func__);
		return rc;
	}
	return msm_sensor_power_up(sdata);
}

int32_t s5k4e5yx_power_down(const struct msm_camera_sensor_info *sdata)
{
	int rc;
	pr_info("%s\n", __func__);

	if (sdata->camera_power_off == NULL) {
		pr_err("sensor platform_data didnt register\n");
		return -EIO;
	}

	rc = sdata->camera_power_off();
	if (rc < 0) {
		pr_err("%s failed to disable power\n", __func__);
		return rc;
	}
	rc = msm_sensor_power_down(sdata);

	msm_camio_clk_disable(CAMIO_CAM_MCLK_CLK);

	return rc;
}

static int s5k4e5yx_probe(struct platform_device *pdev)
{
	int	rc = 0;

	pr_info("%s\n", __func__);

	rc = msm_sensor_register(pdev, s5k4e5yx_sensor_v4l2_probe);
	if(rc >= 0)
		s5k4e5yx_sysfs_init();
	return rc;
}

struct platform_driver s5k4e5yx_driver = {
	.probe = s5k4e5yx_probe,
	.driver = {
		.name = PLATFORM_DRIVER_NAME,
		.owner = THIS_MODULE,
	},
};

static int __init msm_sensor_init_module(void)
{
	pr_info("%s\n", __func__);
	return platform_driver_register(&s5k4e5yx_driver);
}

static struct v4l2_subdev_core_ops s5k4e5yx_subdev_core_ops;
static struct v4l2_subdev_video_ops s5k4e5yx_subdev_video_ops = {
	.enum_mbus_fmt = msm_sensor_v4l2_enum_fmt,
};

static struct v4l2_subdev_ops s5k4e5yx_subdev_ops = {
	.core = &s5k4e5yx_subdev_core_ops,
	.video  = &s5k4e5yx_subdev_video_ops,
};

/* HTC_START*/
static int s5k4e5yx_read_fuseid(struct sensor_cfg_data *cdata,
	struct msm_sensor_ctrl_t *s_ctrl)
{
	int32_t  rc;
	unsigned short i, R1, R2, R3;
	unsigned short  OTP[10] = {0};

	struct msm_camera_i2c_client *s5k4e5yx_msm_camera_i2c_client = s_ctrl->sensor_i2c_client;

	pr_info("%s: sensor OTP information:\n", __func__);

	rc = msm_camera_i2c_write_b(s5k4e5yx_msm_camera_i2c_client, 0x30F9, 0x0E);
	if (rc < 0)
		pr_err("%s: i2c_write_b 0x30F9 fail\n", __func__);

	rc = msm_camera_i2c_write_b(s5k4e5yx_msm_camera_i2c_client, 0x30FA, 0x0A);
	if (rc < 0)
		pr_err("%s: i2c_write_b 0x30FA fail\n", __func__);

	rc = msm_camera_i2c_write_b(s5k4e5yx_msm_camera_i2c_client, 0x30FB, 0x71);
	if (rc < 0)
		pr_err("%s: i2c_write_b 0x30FB fail\n", __func__);

	rc = msm_camera_i2c_write_b(s5k4e5yx_msm_camera_i2c_client, 0x30FB, 0x70);
	if (rc < 0)
		pr_err("%s: i2c_write_b 0x30FB fail\n", __func__);

	mdelay(4);

	for (i = 0; i < 10; i++) {
		rc = msm_camera_i2c_write_b(s5k4e5yx_msm_camera_i2c_client, 0x310C, i);
		if (rc < 0)
			pr_err("%s: i2c_write_b 0x310C fail\n", __func__);
		rc = msm_camera_i2c_read_b(s5k4e5yx_msm_camera_i2c_client, 0x310F, &R1);
		if (rc < 0)
			pr_err("%s: i2c_read_b 0x310F fail\n", __func__);
		rc = msm_camera_i2c_read_b(s5k4e5yx_msm_camera_i2c_client, 0x310E, &R2);
		if (rc < 0)
			pr_err("%s: i2c_read_b 0x310E fail\n", __func__);
		rc = msm_camera_i2c_read_b(s5k4e5yx_msm_camera_i2c_client, 0x310D, &R3);
			if (rc < 0)
			pr_err("%s: i2c_read_b 0x310D fail\n", __func__);

		if ((R3&0x0F) != 0)
			OTP[i] = (short)(R3&0x0F);
		else if ((R2&0x0F) != 0)
			OTP[i] = (short)(R2&0x0F);
		else if ((R2>>4) != 0)
			OTP[i] = (short)(R2>>4);
		else if ((R1&0x0F) != 0)
			OTP[i] = (short)(R1&0x0F);
		else
			OTP[i] = (short)(R1>>4);

	}
	pr_info("%s: VenderID=%x,LensID=%x,SensorID=%x%x\n", __func__,
		OTP[0], OTP[1], OTP[2], OTP[3]);
	pr_info("%s: ModuleFuseID= %x%x%x%x%x%x\n", __func__,
		OTP[4], OTP[5], OTP[6], OTP[7], OTP[8], OTP[9]);

    cdata->cfg.fuse.fuse_id_word1 = 0;
    cdata->cfg.fuse.fuse_id_word2 = 0;
	cdata->cfg.fuse.fuse_id_word3 = (OTP[0]);
	cdata->cfg.fuse.fuse_id_word4 =
		(OTP[4]<<20) |
		(OTP[5]<<16) |
		(OTP[6]<<12) |
		(OTP[7]<<8) |
		(OTP[8]<<4) |
		(OTP[9]);

	pr_info("s5k4e5yx: fuse->fuse_id_word1:%d\n",
		cdata->cfg.fuse.fuse_id_word1);
	pr_info("s5k4e5yx: fuse->fuse_id_word2:%d\n",
		cdata->cfg.fuse.fuse_id_word2);
	pr_info("s5k4e5yx: fuse->fuse_id_word3:0x%08x\n",
		cdata->cfg.fuse.fuse_id_word3);
	pr_info("s5k4e5yx: fuse->fuse_id_word4:0x%08x\n",
		cdata->cfg.fuse.fuse_id_word4);
	return 0;
}
/* HTC_END*/


static struct msm_sensor_fn_t s5k4e5yx_func_tbl = {
	.sensor_start_stream = msm_sensor_start_stream,
	.sensor_stop_stream = msm_sensor_stop_stream,
	.sensor_group_hold_on = msm_sensor_group_hold_on,
	.sensor_group_hold_off = msm_sensor_group_hold_off,
	.sensor_set_fps = msm_sensor_set_fps,
	.sensor_write_exp_gain = msm_sensor_write_exp_gain1,
	.sensor_write_snapshot_exp_gain = msm_sensor_write_exp_gain1,
	.sensor_setting = msm_sensor_setting,
	.sensor_set_sensor_mode = msm_sensor_set_sensor_mode,
	.sensor_mode_init = msm_sensor_mode_init,
	.sensor_get_output_info = msm_sensor_get_output_info,
	.sensor_config = s5k4e5yx_sensor_config,
	.sensor_open_init = s5k4e5yx_sensor_open_init,
	.sensor_release = s5k4e5yx_sensor_release,
	.sensor_power_up = s5k4e5yx_power_up,
	.sensor_power_down = s5k4e5yx_power_down,
	.sensor_probe = msm_sensor_probe,
	.sensor_i2c_read_fuseid = s5k4e5yx_read_fuseid,/*HTC*/
};

static struct msm_sensor_reg_t s5k4e5yx_regs = {
	.default_data_type = MSM_CAMERA_I2C_BYTE_DATA,
	.start_stream_conf = s5k4e5yx_start_settings,
	.start_stream_conf_size = ARRAY_SIZE(s5k4e5yx_start_settings),
	.stop_stream_conf = s5k4e5yx_stop_settings,
	.stop_stream_conf_size = ARRAY_SIZE(s5k4e5yx_stop_settings),
	.group_hold_on_conf = s5k4e5yx_groupon_settings,
	.group_hold_on_conf_size = ARRAY_SIZE(s5k4e5yx_groupon_settings),
	.group_hold_off_conf = s5k4e5yx_groupoff_settings,
	.group_hold_off_conf_size =
		ARRAY_SIZE(s5k4e5yx_groupoff_settings),
	.init_settings = &s5k4e5yx_init_conf[0],
	.init_size = ARRAY_SIZE(s5k4e5yx_init_conf),
	.mode_settings = &s5k4e5yx_confs[0],
	.output_settings = &s5k4e5yx_dimensions[0],
	.num_conf = ARRAY_SIZE(s5k4e5yx_confs),
};

static struct msm_sensor_ctrl_t s5k4e5yx_s_ctrl = {
	.msm_sensor_reg = &s5k4e5yx_regs,
	.sensor_i2c_client = &s5k4e5yx_sensor_i2c_client,
	.sensor_i2c_addr = 0x20,
	.mirror_flip = CAMERA_SENSOR_NONE,
	.sensor_output_reg_addr = &s5k4e5yx_reg_addr,
	.sensor_id_info = &s5k4e5yx_id_info,
	.sensor_exp_gain_info = &s5k4e5yx_exp_gain_info,
	.cam_mode = MSM_SENSOR_MODE_INVALID,
	.csi_params = &s5k4e5yx_csi_params_array[0],
	.msm_sensor_mutex = &s5k4e5yx_mut,
	.sensor_i2c_driver = &s5k4e5yx_i2c_driver,
	.sensor_v4l2_subdev_info = s5k4e5yx_subdev_info,
	.sensor_v4l2_subdev_info_size = ARRAY_SIZE(s5k4e5yx_subdev_info),
	.sensor_v4l2_subdev_ops = &s5k4e5yx_subdev_ops,
	.func_tbl = &s5k4e5yx_func_tbl,
};

module_init(msm_sensor_init_module);
MODULE_DESCRIPTION("Samsung 5 MP Bayer sensor driver");
MODULE_LICENSE("GPL v2");
