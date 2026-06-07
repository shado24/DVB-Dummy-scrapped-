#include <linux/module.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/media.h>
#include <media/dvbdev.h>
#include <media/demux.h>
#include <media/dvb_demux.h>
#include <media/dmxdev.h>
#include <media/dvb_frontend.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Test");
MODULE_DESCRIPTION("Modern Dummy DVB Driver with Frontend for Kernel 6.12+");

static short adapter_nr[] = {0};

static struct dvb_adapter dummy_adapter;
static struct dvb_demux *dummy_demux;
static struct dmxdev dummy_dmxdev;
static struct dvb_frontend *dummy_frontend;

// Fixed for Kernel 6.12+: Removed obsolete .type and fixed FE_CAN_RECOVER typo
static struct dvb_frontend_ops dummy_fe_ops = {
    .info = {
        .name = "Dummy Virtual Frontend",
        .frequency_min_hz = 470000000,
        .frequency_max_hz = 862000000,
        .frequency_stepsize_hz = 166667,
        .caps = FE_CAN_FEC_1_2 | FE_CAN_FEC_2_3 | FE_CAN_FEC_3_4 |
                FE_CAN_QAM_16 | FE_CAN_QAM_64 | FE_CAN_RECOVER
    },
};

static int __init dummy_dvb_init(void) {
    int ret;
    
    // 1. Allocate demux memory
    dummy_demux = kzalloc(sizeof(struct dvb_demux), GFP_KERNEL);
    if (!dummy_demux) return -ENOMEM;

    // 2. Register Adapter
    ret = dvb_register_adapter(&dummy_adapter, "Dummy DVB Adapter", THIS_MODULE, NULL, adapter_nr);
    if (ret < 0) {
        kfree(dummy_demux);
        return ret;
    }

    // 3. Register Frontend
    dummy_frontend = kzalloc(sizeof(struct dvb_frontend), GFP_KERNEL);
    if (!dummy_frontend) {
        ret = -ENOMEM;
        goto err_adapter;
    }
    
    dummy_frontend->ops = dummy_fe_ops;
    ret = dvb_register_frontend(&dummy_adapter, dummy_frontend);
    if (ret < 0) {
        kfree(dummy_frontend);
        goto err_adapter;
    }

    // Modern DVB API requires telling the frontend which stream type it accepts
    // We explicitly set DVB-T (Terrestrial Digital TV) here
    dummy_frontend->ops.delsys[0] = SYS_DVBT;

    // 4. Initialize Demux
    dummy_demux->dmx.capabilities = DMX_TS_FILTERING | DMX_SECTION_FILTERING;
    dummy_demux->filternum = 16;
    dummy_demux->feednum = 16;
    ret = dvb_dmx_init(dummy_demux);
    if (ret < 0) goto err_fe;

    // 5. Initialize Dmxdev
    dummy_dmxdev.filternum = 16;
    dummy_dmxdev.demux = &dummy_demux->dmx;
    dummy_dmxdev.capabilities = 0;
    ret = dvb_dmxdev_init(&dummy_dmxdev, &dummy_adapter);
    if (ret < 0) goto err_demux;

    pr_info("Dummy DVB adapter and Frontend registered successfully\n");
    return 0;

err_demux:
    dvb_dmx_release(dummy_demux);
err_fe:
    dvb_unregister_frontend(dummy_frontend);
    kfree(dummy_frontend);
err_adapter:
    dvb_unregister_adapter(&dummy_adapter);
    kfree(dummy_demux);
    return ret;
}

static void __exit dummy_dvb_exit(void) {
    dvb_dmxdev_release(&dummy_dmxdev);
    dvb_dmx_release(dummy_demux);
    if (dummy_frontend) {
        dvb_unregister_frontend(dummy_frontend);
        kfree(dummy_frontend);
    }
    dvb_unregister_adapter(&dummy_adapter);
    kfree(dummy_demux);
    pr_info("Dummy DVB adapter unregistered\n");
}

module_init(dummy_dvb_init);
module_exit(dummy_dvb_exit);
