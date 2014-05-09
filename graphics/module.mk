MODULE := graphics

MODULE_OBJS := \
	conversion.o \
	cursorman.o \
	font.o \
	fontman.o \
	fonts/bdf.o \
	fonts/consolefont.o \
	fonts/newfont_big.o \
	fonts/newfont.o \
	fonts/ttf.o \
	fonts/winfont.o \
	maccursor.o \
	nine_patch.o \
	primitives.o \
	scaler.o \
	scalerplugin.o \
	scaler/thumbnail_intern.o \
	scaler/normal.o \
	sjis.o \
	surface.o \
	thumbnail.o \
	transform_struct.o \
	transform_tools.o \
	transparent_surface.o \
	VectorRenderer.o \
	VectorRendererSpec.o \
	wincursor.o \
	yuv_to_rgb.o

ifdef USE_ASPECT
MODULE_OBJS += \
	scaler/aspect.o
endif

ifdef USE_SCALERS
MODULE_OBJS += \
	scaler/dotmatrix.o \
	scaler/sai.o \
	scaler/pm.o \
	scaler/downscaler.o \
	scaler/scale2x.o \
	scaler/scale3x.o \
	scaler/scalebit.o \
	scaler/tv.o

ifdef USE_ARM_SCALER_ASM
MODULE_OBJS += \
	scaler/downscalerARM.o \
	scaler/scale2xARM.o \
	scaler/Normal2xARM.o
endif

ifdef USE_HQ_SCALERS
MODULE_OBJS += \
	scaler/hq.o

ifdef USE_NASM
MODULE_OBJS += \
	scaler/hq2x_i386.o \
	scaler/hq3x_i386.o
endif

endif

ifdef USE_EDGE_SCALERS
MODULE_OBJS += \
	scaler/edge.o
endif

endif

# Include common rules
include $(srcdir)/rules.mk
