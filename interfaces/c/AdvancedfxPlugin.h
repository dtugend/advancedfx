#ifndef ADVANCEDFX_PLUGIN_H
#define ADVANCEDFX_PLUGIN_H

#include "AdvancedfxTypes.h"

struct AdvancedfxGetPlugins_s {
    void (*AddRef)(void);
    void (*Release)(void);

    /**    
     * @remark Next() -> ADVANCEDFX_NULLPTR | AdvancedfxPlugin_s*
     */
    struct AdvancedfxPlugin_s * (*Next)(void);
};

struct AdvancedfxPlugin_s {
    void (*AddRef)(void);
    void (*Release)(void);
 
    /**
     * @remark Call("GetTitle", ADVANCEDFX_NULLPTR) -> ADVANCEDFX_NULLPTR | AdvancedfxCString_t
     * @remark Call("GetSemVer", ADVANCEDFX_NULLPTR) -> ADVANCEDFX_NULLPTR | AdvancedfxCString_t
     * @remark Call("GetAuthor", ADVANCEDFX_NULLPTR) -> ADVANCEDFX_NULLPTR | AdvancedfxCString_t
     * @remark Call("GetUrl", ADVANCEDFX_NULLPTR) -> ADVANCEDFX_NULLPTR | AdvancedfxCString_t
     * @remark Call("GetContexts", ADVANCEDFX_NULLPTR) -> ADVANCEDFX_NULLPTR | AdvancedfxGetContextsResult_s*
     */
    void * (*Call)(const char * context, void * args);
};

struct AdvancedfxGetContextsResult_s {
    void (*AddRef)(void);
    void (*Release)(void);

    /**    
     * @remark Next() -> ADVANCEDFX_NULLPTR | AdvancedfxCString_t
     */
    AdvancedfxCString_t (*Next)(void);
};

#endif
