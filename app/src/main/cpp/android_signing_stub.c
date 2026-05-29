// Stub for ncam-signing.c — replaces OpenSSL-dependent signing on Android.
// ncam-signing.c is excluded from the build via CMakeLists.txt.
// All callers in ncam.c check osi.is_verified and print osi.* fields;
// we return a safe "not verified" state so the daemon starts normally.

#ifdef __ANDROID__

#include "ncam/ncam-signing.h"
#include <string.h>
#include <stdlib.h>

struct o_sign_info osi;

bool init_signing_info(const char *binfile)
{
    memset(&osi, 0, sizeof(osi));
    osi.is_verified          = false;
    osi.cert_version         = 0;
    osi.cert_is_expired      = false;
    osi.cert_is_cacert       = false;
    osi.cert_is_valid_self   = false;
    osi.cert_is_valid_system = false;
    osi.binfile_exists       = false;
    osi.sign_digest_size     = 0;
    osi.hash_digest_size     = 0;
    osi.hash_size            = 0;

    // Provide non-NULL strings so callers can safely print them.
    osi.cert_serial      = strdup("n/a");
    osi.cert_fingerprint = strdup("n/a");
    osi.cert_subject     = strdup("Android build — signing disabled");
    osi.cert_issuer      = strdup("n/a");
    osi.pkey_type        = strdup("n/a");
    osi.hash_sha256      = strdup("n/a");
    osi.system_ca_file   = NULL;

    if (binfile)
        strncpy(osi.resolved_binfile, binfile, sizeof(osi.resolved_binfile) - 1);

    return true;  // return true so ncam.c does not call cs_exit_ncam()
}

#endif // __ANDROID__
