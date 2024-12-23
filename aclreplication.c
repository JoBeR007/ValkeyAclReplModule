#include "valkeymodule.h"
#include "string.h"

static ValkeyModuleCtx *moduleContext = NULL;
static ValkeyModuleCommandFilter *filter;
static long long noself = 1;

void ACLReplication_CommandFilter(ValkeyModuleCommandFilterCtx *filter);

int ValkeyModule_OnLoad(ValkeyModuleCtx *ctx, ValkeyModuleString **argv, int argc) {
    if (ValkeyModule_Init(ctx, "aclrepl", 1, VALKEYMODULE_APIVER_1) == VALKEYMODULE_ERR)
        return VALKEYMODULE_ERR;

    moduleContext = ctx;

    if ((filter = ValkeyModule_RegisterCommandFilter(ctx, ACLReplication_CommandFilter,
                                                     noself ? VALKEYMODULE_CMDFILTER_NOSELF : 0)) == NULL) {
        return VALKEYMODULE_ERR;
    }

    // TODO: ACL LIST -> REPLICATE existing

    ValkeyModule_Log(ctx, "notice", "ACL Replication Module loaded.");
    return VALKEYMODULE_OK;
}

void ACLReplication_CommandFilter(ValkeyModuleCommandFilterCtx *filter) {
    size_t argc = ValkeyModule_CommandFilterArgsCount(filter);
    if (argc < 1) {
        return;
    }

    ValkeyModuleString *cmd = ValkeyModule_CommandFilterArgGet(filter, 0);
    size_t cmdlen;
    const char *cmdname = ValkeyModule_StringPtrLen(cmd, &cmdlen);

    if (strcasecmp(cmdname, "ACL") == 0 && argc >= 2) {
        ValkeyModuleString *subcmd_str = ValkeyModule_CommandFilterArgGet(filter, 1);
        size_t subcmdlen;
        const char *subcmd = ValkeyModule_StringPtrLen(subcmd_str, &subcmdlen);

        const char *acl_modify_subcommands[] = {
            "SETUSER",
            "DELUSER",
            NULL};

        for (int i = 0; acl_modify_subcommands[i] != NULL; i++) {
            if (strcasecmp(subcmd, acl_modify_subcommands[i]) == 0) {
                size_t replicate_argc = argc - 1;
                ValkeyModuleString **replicate_argv = malloc(replicate_argc * sizeof(ValkeyModuleString *));

                for (size_t j = 1; j < replicate_argc + 1; j++) {
                    replicate_argv[j - 1] = ValkeyModule_CommandFilterArgGet(filter, j);
                }

                if (ValkeyModule_Replicate(ValkeyModule_GetDetachedThreadSafeContext(moduleContext),
                                           "ACL",
                                           "v",
                                           replicate_argv,
                                           replicate_argc) == VALKEYMODULE_ERR) {
                    ValkeyModule_Log(moduleContext, "warning", "Failed to replicate ACL command");
                }
                free(replicate_argv);
                break;
            }
        }
    }
}
