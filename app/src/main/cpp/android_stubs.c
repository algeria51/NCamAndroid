#ifdef __ANDROID__

#include "android_compat.h"
#include "globals.h"

// Cardreader stubs — required by ncam-reader.c even when WITH_CARDREADER=0
// because readers.h declares them unconditionally.

#ifndef WITH_CARDREADER

const struct s_cardreader cardreader_internal_sci  = {0};
const struct s_cardreader cardreader_mouse         = {0};
const struct s_cardreader cardreader_stinger       = {0};
const struct s_cardreader cardreader_db2com        = {0};
const struct s_cardreader cardreader_mp35          = {0};
const struct s_cardreader cardreader_sc8in1        = {0};
const struct s_cardreader cardreader_smargo        = {0};
const struct s_cardreader cardreader_smartreader   = {0};
const struct s_cardreader cardreader_pcsc          = {0};
const struct s_cardreader cardreader_stapi         = {0};
const struct s_cardreader cardreader_gxapi         = {0};
const struct s_cardreader cardreader_drecas        = {0};

// icc_async stubs
int ICC_Async_GetStatus(struct s_reader *r, int *s) { (void)r;(void)s; return 0; }
int ICC_Async_Activate(struct s_reader *r, void *a, uint16_t f) { (void)r;(void)a;(void)f; return 0; }
int ICC_Async_CardWrite(struct s_reader *r,unsigned char*i,uint16_t il,unsigned char*o,uint16_t*ol) {
    (void)r;(void)i;(void)il;(void)o;(void)ol; return 0;
}
int ICC_Async_SetTimings(struct s_reader *r, void *p) { (void)r;(void)p; return 0; }
int ICC_Async_Transmit(struct s_reader *r, uint32_t l, uint32_t e, unsigned char *d,
                       uint32_t *rl, uint32_t *el) {
    (void)r;(void)l;(void)e;(void)d;(void)rl;(void)el; return 0;
}
void cardreader_init_locks(void) {}
int  cardreader_init(struct s_reader *r) { (void)r; return 0; }
void cardreader_close(struct s_reader *r) { (void)r; }
int  cardreader_do_reset(struct s_reader *r, void *a, int *b, uint16_t *c) {
    (void)r;(void)a;(void)b;(void)c; return 0;
}
int  cardreader_get_card_info(struct s_reader *r) { (void)r; return 0; }
int  cardreader_process_ecm(struct s_reader *r, struct s_client *c, struct ecm_request_t *e) {
    (void)r;(void)c;(void)e; return 0;
}
int  cardreader_do_emm(struct s_reader *r, struct emm_packet_t *e) { (void)r;(void)e; return 0; }

#endif // !WITH_CARDREADER

#endif // __ANDROID__
